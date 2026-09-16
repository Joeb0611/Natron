/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: abort wakes all scheduler waiters; no CPU-zero hang (#248).
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "OutputSchedulerThread.h"

#include <iostream>
#include <set>
#include <limits>
#include <list>
#include <algorithm> // min, max
#include <cassert>
#include <stdexcept>
#include <sstream> // stringstream

#include <QMetaType>
#include <QMutex>
#include <QWaitCondition>
#include <QCoreApplication>
#include <QString>
#include <QThreadPool>
#include <QDebug>
#include <QTextStream>
#include <QRunnable>

#include "Global/MathUtils.h"
#ifdef DEBUG
#include "Global/FloatingPointExceptions.h"
#endif
#include "Engine/AbortableRenderInfo.h"
#include "Engine/AppManager.h"
#include "Engine/AppInstance.h"
#include "Engine/EffectInstance.h"
#include "Engine/Image.h"
#include "Engine/KnobFile.h"
#include "Engine/Node.h"
#include "Engine/OpenGLViewerI.h"
#include "Engine/GenericSchedulerThreadWatcher.h"
#include "Engine/Project.h"
#include "Engine/RenderStats.h"
#include "Engine/RotoContext.h"
#include "Engine/Settings.h"
#include "Engine/Timer.h"
#include "Engine/TimeLine.h"
#include "Engine/TLSHolder.h"
#include "Engine/UpdateViewerParams.h"
#include "Engine/ViewIdx.h"
#include "Engine/ViewerInstance.h"
#include "Engine/WriteNode.h"

#ifdef DEBUG
//#define TRACE_SCHEDULER
//#define TRACE_CURRENT_FRAME_SCHEDULER
#endif

#define NATRON_FPS_REFRESH_RATE_SECONDS 1.5

/*
   When defined, parallel frame renders are spawned from a timer so that the frames
   appear to be rendered all at the same speed.
   When undefined each time a frame is computed a new thread will be spawned
   until we reach the maximum allowed parallel frame renders.
 */
//#define NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER

#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
#define NATRON_SCHEDULER_THREADS_SPAWN_DEFAULT_TIMEOUT_MS 500
#endif

#define NATRON_SCHEDULER_ABORT_AFTER_X_UNSUCCESSFUL_ITERATIONS 5000
#define NATRON_SCHEDULER_WAIT_SLICE_MS 250
#define NATRON_SCHEDULER_EMPTY_BUFFER_STALL_SLICES 40 // 10s of empty-buffer timeouts => stall

NATRON_NAMESPACE_ENTER


///Sort the frames by time and then by view

struct BufferedFrameKey
{
    int time;
};

struct BufferedFrameCompare_less
{
    bool operator()(const BufferedFrameKey& lhs,
                    const BufferedFrameKey& rhs) const
    {
        return lhs.time < rhs.time;
    }
};

typedef std::multimap<BufferedFrameKey, BufferedFrame, BufferedFrameCompare_less> FrameBuffer;


NATRON_NAMESPACE_ANONYMOUS_ENTER

class MetaTypesRegistration
{
public:
    inline MetaTypesRegistration()
    {
        qRegisterMetaType<BufferedFrames>("BufferedFrames");
        qRegisterMetaType<BufferableObjectPtrList>("BufferableObjectPtrList");
    }
};

NATRON_NAMESPACE_ANONYMOUS_EXIT


static MetaTypesRegistration registration;
struct RenderThread
{
    RenderThreadTask* thread;
    bool active;
};

typedef std::list<RenderThread> RenderThreads;


struct ProducedFrame
{
    BufferableObjectPtrList frames;
    U64 age;
    RenderStatsPtr stats;
};

struct ProducedFrameCompareAgeLess
{
    bool operator() (const ProducedFrame& lhs,
                     const ProducedFrame& rhs) const
    {
        return lhs.age < rhs.age;
    }
};

typedef std::set<ProducedFrame, ProducedFrameCompareAgeLess> ProducedFrameSet;

class OutputSchedulerThreadExecMTArgs
    : public GenericThreadExecOnMainThreadArgs
{
public:

    BufferedFrames frames;

    OutputSchedulerThreadExecMTArgs()
        : GenericThreadExecOnMainThreadArgs()
    {}

    virtual ~OutputSchedulerThreadExecMTArgs() {}
};

typedef std::shared_ptr<OutputSchedulerThreadExecMTArgs> OutputSchedulerThreadExecMTArgsPtr;

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
static bool
isBufferFull(int nbBufferedElement,
             int hardwardIdealThreadCount)
{
    return nbBufferedElement >= hardwardIdealThreadCount * 3;
}

#endif

struct OutputSchedulerThreadPrivate
{
    FrameBuffer buf; //the frames rendered by the worker threads that needs to be rendered in order by the output device
    QWaitCondition bufEmptyCondition;
    mutable QMutex bufMutex;

    //doesn't need any protection since it never changes and is set in the constructor
    OutputSchedulerThread::ProcessFrameModeEnum mode; //is the frame to be processed on the main-thread (i.e OpenGL rendering) or on the scheduler thread
    Timer timer; // Timer regulating the engine execution. It is controlled by the GUI and MT-safe.
    std::unique_ptr<TimeLapse> renderTimer; // Timer used to report stats when rendering

    ///When the render threads are not using the appendToBuffer API, the scheduler has no way to know the rendering is finished
    ///but to count the number of frames rendered via notifyFrameRended which is called by the render thread.
    mutable QMutex renderFinishedMutex;
    U64 nFramesRendered;
    bool renderFinished; //< set to true when nFramesRendered = runArgs->lastFrame - runArgs->firstFrame + 1

    // Pointer to the args used in threadLoopOnce(), only usable from the scheduler thread
    OutputSchedulerThreadStartArgsWPtr runArgs;
    mutable QMutex lastRunArgsMutex;
    std::vector<ViewIdx> lastPlaybackViewsToRender;
    RenderDirectionEnum lastPlaybackRenderDirection;

    ///Worker threads
    mutable QMutex renderThreadsMutex;
    RenderThreads renderThreads;
    QWaitCondition allRenderThreadsInactiveCond; // wait condition to make sure all render threads are asleep

#ifdef NATRON_PLAYBACK_USES_THREAD_POOL
    QThreadPool* threadPool;
#else

    QWaitCondition allRenderThreadsQuitCond; //to make sure all render threads have quit
    std::list<int> framesToRender;

    ///Render threads wait in this condition and the scheduler wake them when it needs to render some frames
    QWaitCondition framesToRenderNotEmptyCond;

#endif

    ///Work queue filled by the scheduler thread when in playback/render on disk
    QMutex framesToRenderMutex; // protects framesToRender & currentFrameRequests

    ///index of the last frame pushed (framesToRender.back())
    ///we store this because when we call pushFramesToRender we need to know what was the last frame that was queued
    ///Protected by framesToRenderMutex
    int lastFramePushedIndex;
    int expectFrameToRender;
    OutputEffectInstanceWPtr outputEffect; //< The effect used as output device
    RenderEngine* engine;

#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
    QTimer threadSpawnsTimer;
    QMutex lastRecordedFPSMutex;
    double lastRecordedFPS;

#endif

    QMutex bufferedOutputMutex;
    int lastBufferedOutputSize;


    OutputSchedulerThreadPrivate(RenderEngine* engine,
                                 const OutputEffectInstancePtr& effect,
                                 OutputSchedulerThread::ProcessFrameModeEnum mode)
        : buf()
        , bufEmptyCondition()
        , bufMutex()
        , mode(mode)
        , timer()
        , renderTimer()
        , renderFinishedMutex()
        , nFramesRendered(0)
        , renderFinished(false)
        , runArgs()
        , lastRunArgsMutex()
        , lastPlaybackViewsToRender()
        , lastPlaybackRenderDirection(eRenderDirectionForward)
        , renderThreadsMutex()
        , renderThreads()
        , allRenderThreadsInactiveCond()
#ifdef NATRON_PLAYBACK_USES_THREAD_POOL
        , threadPool( QThreadPool::globalInstance() )
#else
        , allRenderThreadsQuitCond()
        , framesToRender()
        , framesToRenderNotEmptyCond()
#endif
        , framesToRenderMutex()
        , lastFramePushedIndex(0)
        , expectFrameToRender(0)
        , outputEffect(effect)
        , engine(engine)
#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
        , threadSpawnsTimer()
        , lastRecordedFPSMutex()
        , lastRecordedFPS(0.)
#endif
        , bufferedOutputMutex()
        , lastBufferedOutputSize(0)
    {
    }

    void appendBufferedFrame(double time,
                             ViewIdx view,
                             const RenderStatsPtr& stats,
                             const BufferableObjectPtr& image)
    {
        ///Private, shouldn't lock
        assert( !bufMutex.tryLock() );
#ifdef TRACE_SCHEDULER
        QString idStr;
        if (image) {
            idStr = QString::fromUtf8("ID: ") + QString::number( image->getUniqueID() );
        }
        qDebug() << "Parallel Render Thread: Rendered Frame:" << time << " View:" << (int)view << idStr;
#endif
        BufferedFrameKey key;
        BufferedFrame value;
        value.time = key.time = time;
        value.view = view;
        value.frame = image;
        value.stats = stats;
        buf.insert( std::make_pair(key, value) );
    }

    struct ViewUniqueIDPair
    {
        int view;
        int uniqueId;
    };

    struct ViewUniqueIDPairCompareLess
    {
        bool operator() (const ViewUniqueIDPair& lhs,
                         const ViewUniqueIDPair& rhs) const
        {
            if (lhs.view < rhs.view) {
                return true;
            } else if (lhs.view > rhs.view) {
                return false;
            } else {
                if (lhs.uniqueId < rhs.uniqueId) {
                    return true;
                } else if (lhs.uniqueId > rhs.uniqueId) {
                    return false;
                } else {
                    return false;
                }
            }
        }
    };

    typedef std::set<ViewUniqueIDPair, ViewUniqueIDPairCompareLess> ViewUniqueIDSet;

    void getFromBufferAndErase(double time,
                               BufferedFrames& frames)
    {
        ///Private, shouldn't lock
        assert( !bufMutex.tryLock() );

        /*
           Note that the frame buffer does not hold any particular ordering and just contains all the frames as they
           were received by render threads.
           In the buffer, for any particular given time there can be:
           - Multiple views
           - Multiple "unique ID" (corresponds to viewer input A or B)

           Also since we are rendering ahead, we can have a buffered frame at time 23,
           and also another frame at time 23, each of which could have multiple unique IDs and so on

           To retrieve what we need to render, we extract at least one view and unique ID for this particular time
         */

        ViewUniqueIDSet uniqueIdsRetrieved;
        BufferedFrameKey key;
        key.time = time;
        std::pair<FrameBuffer::iterator, FrameBuffer::iterator> range = buf.equal_range(key);
        std::list<std::pair<BufferedFrameKey, BufferedFrame> > toKeep;
        for (FrameBuffer::iterator it = range.first; it != range.second; ++it) {
            bool keepInBuf = true;
            if (it->second.frame) {
                ViewUniqueIDPair p;
                p.view = (int)it->second.view;
                p.uniqueId = it->second.frame->getUniqueID();
                std::pair<ViewUniqueIDSet::iterator, bool> alreadyRetrievedIndex = uniqueIdsRetrieved.insert(p);
                if (alreadyRetrievedIndex.second) {
                    frames.push_back(it->second);
                    keepInBuf = false;
                }
            }


            if (keepInBuf) {
                toKeep.push_back(*it);
            }
        }
        if ( range.first != buf.end() ) {
            buf.erase(range.first, range.second);
            buf.insert( toKeep.begin(), toKeep.end() );
        }
    }

    void appendRunnable(RenderThreadTask* runnable)
    {
        assert( !renderThreadsMutex.tryLock() );
        RenderThread r;
        r.thread = runnable;
        r.active = true;
        renderThreads.push_back(r);
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        runnable->start();
#else
        threadPool->start(runnable);
#endif
    }

    RenderThreads::iterator getRunnableIterator(RenderThreadTask* runnable)
    {
        ///Private shouldn't lock
        assert( !renderThreadsMutex.tryLock() );
        for (RenderThreads::iterator it = renderThreads.begin(); it != renderThreads.end(); ++it) {
            if (it->thread == runnable) {
                return it;
            }
        }

        return renderThreads.end();
    }

    int getNBufferedFrames() const
    {
        QMutexLocker l(&bufMutex);

        return buf.size();
    }

    static bool getNextFrameInSequence(PlaybackModeEnum pMode,
                                       RenderDirectionEnum direction,
                                       int frame,
                                       int firstFrame,
                                       int lastFrame,
                                       unsigned int frameStep,
                                       int* nextFrame,
                                       RenderDirectionEnum* newDirection);
    static void getNearestInSequence(RenderDirectionEnum direction,
                                     int frame,
                                     int firstFrame,
                                     int lastFrame,
                                     int* nextFrame);


    void waitForRenderThreadsToBeDone()
    {
        assert( !renderThreadsMutex.tryLock() );
        int stallSlices = 0;
        while (renderThreads.size() > 0
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
               /*
                  When not using the thread pool we use the same threads for computing several frames.
                  When using the thread-pool tasks are actually just removed from renderThreads when they are finisehd
                */
               && getNActiveRenderThreads() > 0
#endif
               ) {
            if (!allRenderThreadsInactiveCond.wait(&renderThreadsMutex, NATRON_SCHEDULER_WAIT_SLICE_MS)) {
                ++stallSlices;
                if (engine && engine->isSequentialRenderBeingAborted()) {
                    break;
                }
                if (stallSlices >= NATRON_SCHEDULER_EMPTY_BUFFER_STALL_SLICES) {
                    break;
                }
            } else {
                stallSlices = 0;
            }
        }
    }

    int getNActiveRenderThreads() const
    {
        ///Private shouldn't lock
        assert( !renderThreadsMutex.tryLock() );

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        int ret = 0;
        for (RenderThreads::const_iterator it = renderThreads.begin(); it != renderThreads.end(); ++it) {
            if (it->active) {
                ++ret;
            }
        }

        return ret;
#else

        /*
           When not using the thread pool we use the same threads for computing several frames.
           When using the thread-pool tasks are actually just removed from renderThreads when they are finisehd
         */
        return (int)renderThreads.size();
#endif
    }

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    void removeQuitRenderThreadsInternal()
    {
        for (;; ) {
            bool hasRemoved = false;
            for (RenderThreads::iterator it = renderThreads.begin(); it != renderThreads.end(); ++it) {
                if ( it->thread->hasQuit() ) {
                    it->thread->deleteLater();
                    renderThreads.erase(it);
                    hasRemoved = true;
                    break;
                }
            }

            if (!hasRemoved) {
                break;
            }
        }
    }

#endif

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    void removeAllQuitRenderThreads()
    {
        ///Private shouldn't lock
        assert( !renderThreadsMutex.tryLock() );

        removeQuitRenderThreadsInternal();

        ///Wake-up the main-thread if it was waiting for all threads to quit
        allRenderThreadsQuitCond.wakeOne();
    }

#endif

    void waitForRenderThreadsToQuit()
    {
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        RenderThreads threads;
        {
            QMutexLocker l(&renderThreadsMutex);
            threads = renderThreads;
        }

        for (RenderThreads::iterator it = threads.begin(); it != threads.end(); ++it) {
            it->thread->wait();
        }
        {
            QMutexLocker l(&renderThreadsMutex);

            removeQuitRenderThreadsInternal();
            assert( renderThreads.empty() );
        }
#else
        /*
           We don't need the threads to actually quit, just need the runnables to be done
         */
        QMutexLocker l(&renderThreadsMutex);
        waitForRenderThreadsToBeDone();
#endif
    }
};

OutputSchedulerThread::OutputSchedulerThread(RenderEngine* engine,
                                             const OutputEffectInstancePtr& effect,
                                             ProcessFrameModeEnum mode)
    : GenericSchedulerThread()
    , _imp( new OutputSchedulerThreadPrivate(engine, effect, mode) )
{
    QObject::connect( &_imp->timer, SIGNAL(fpsChanged(double,double)), _imp->engine, SIGNAL(fpsChanged(double,double)) );


#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
    QObject::connect( &_imp->threadSpawnsTimer, SIGNAL(timeout()), this, SLOT(onThreadSpawnsTimerTriggered()) );
#endif

    setThreadName("Scheduler thread");
}

OutputSchedulerThread::~OutputSchedulerThread()
{
    ///Wake-up all threads and tell them that they must quit
    stopRenderThreads(0);


    ///Make sure they are all gone, there will be a deadlock here if that's not the case.
    _imp->waitForRenderThreadsToQuit();
}

bool
OutputSchedulerThreadPrivate::getNextFrameInSequence(PlaybackModeEnum pMode,
                                                     RenderDirectionEnum direction,
                                                     int frame,
                                                     int firstFrame,
                                                     int lastFrame,
                                                     unsigned int frameStep,
                                                     int* nextFrame,
                                                     RenderDirectionEnum* newDirection)
{
    assert(frameStep >= 1);
    *newDirection = direction;
    if (firstFrame == lastFrame) {
        *nextFrame = firstFrame;

        return true;
    }
    if (frame <= firstFrame) {
        switch (pMode) {
        case ePlaybackModeLoop:
            if (direction == eRenderDirectionForward) {
                *nextFrame = firstFrame + frameStep;
            } else {
                *nextFrame  = lastFrame - frameStep;
            }
            break;
        case ePlaybackModeBounce:
            if (direction == eRenderDirectionForward) {
                *newDirection = eRenderDirectionBackward;
                *nextFrame  = lastFrame - frameStep;
            } else {
                *newDirection = eRenderDirectionForward;
                *nextFrame  = firstFrame + frameStep;
            }
            break;
        case ePlaybackModeOnce:
        default:
            if (direction == eRenderDirectionForward) {
                *nextFrame = firstFrame + frameStep;
                break;
            } else {
                return false;
            }
        }
    } else if (frame >= lastFrame) {
        switch (pMode) {
        case ePlaybackModeLoop:
            if (direction == eRenderDirectionForward) {
                *nextFrame = firstFrame;
            } else {
                *nextFrame = lastFrame - frameStep;
            }
            break;
        case ePlaybackModeBounce:
            if (direction == eRenderDirectionForward) {
                *newDirection = eRenderDirectionBackward;
                *nextFrame = lastFrame - frameStep;
            } else {
                *newDirection = eRenderDirectionForward;
                *nextFrame = firstFrame + frameStep;
            }
            break;
        case ePlaybackModeOnce:
        default:
            if (direction == eRenderDirectionForward) {
                return false;
            } else {
                *nextFrame = lastFrame - frameStep;
                break;
            }
        }
    } else {
        if (direction == eRenderDirectionForward) {
            *nextFrame = frame + frameStep;
        } else {
            *nextFrame = frame - frameStep;
        }
    }

    return true;
} // OutputSchedulerThreadPrivate::getNextFrameInSequence

void
OutputSchedulerThreadPrivate::getNearestInSequence(RenderDirectionEnum direction,
                                                   int frame,
                                                   int firstFrame,
                                                   int lastFrame,
                                                   int* nextFrame)
{
    if ( (frame >= firstFrame) && (frame <= lastFrame) ) {
        *nextFrame = frame;
    } else if (frame < firstFrame) {
        if (direction == eRenderDirectionForward) {
            *nextFrame = firstFrame;
        } else {
            *nextFrame = lastFrame;
        }
    } else { // frame > lastFrame
        if (direction == eRenderDirectionForward) {
            *nextFrame = lastFrame;
        } else {
            *nextFrame = firstFrame;
        }
    }
}

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
void
OutputSchedulerThread::pushFramesToRender(int startingFrame,
                                          int nThreads)
{
    QMutexLocker l(&_imp->framesToRenderMutex);

    _imp->lastFramePushedIndex = startingFrame;

    pushFramesToRenderInternal(startingFrame, nThreads);
}

void
OutputSchedulerThread::pushFramesToRenderInternal(int startingFrame,
                                                  int nThreads)
{
    // QMutexLocker l(&_imp->framesToRenderMutex); already locked (check below)
    assert( !_imp->framesToRenderMutex.tryLock() );

    ///Make sure at least 1 frame is pushed
    if (nThreads <= 0) {
        nThreads = 1;
    }

    RenderDirectionEnum direction;
    int firstFrame, lastFrame, frameStep;
    OutputSchedulerThreadStartArgsPtr runArgs = _imp->runArgs.lock();
    assert(runArgs);
    direction = runArgs->pushTimelineDirection;
    firstFrame = runArgs->firstFrame;
    lastFrame = runArgs->lastFrame;
    frameStep = runArgs->frameStep;

    PlaybackModeEnum pMode = _imp->engine->getPlaybackMode();
    RenderDirectionEnum newDirection = direction;
    if (firstFrame == lastFrame) {
        _imp->framesToRender.push_back(startingFrame);
#ifdef TRACE_SCHEDULER
        qDebug() << "Scheduler Thread: Pushing frame to render: " << startingFrame;
#endif
        _imp->lastFramePushedIndex = startingFrame;
    } else {
        ///Push 2x the count of threads to be sure no one will be waiting
        while ( (int)_imp->framesToRender.size() < nThreads * 2 ) {
            _imp->framesToRender.push_back(startingFrame);
#ifdef TRACE_SCHEDULER
            QString pushDirectionStr = newDirection == eRenderDirectionForward ? QLatin1String("Forward") : QLatin1String("Backward");
            qDebug() << "Scheduler Thread:  Pushing frame to render: " << startingFrame << ", new push direction is " << pushDirectionStr;
#endif
            _imp->lastFramePushedIndex = startingFrame;
            runArgs->pushTimelineDirection = newDirection;

            if ( !OutputSchedulerThreadPrivate::getNextFrameInSequence(pMode, newDirection, startingFrame,
                                                                       firstFrame, lastFrame, frameStep, &startingFrame, &newDirection) ) {
                break;
            }
        }
    }


    ///Wake up render threads to notify them there's work to do
    _imp->framesToRenderNotEmptyCond.wakeAll();
}

void
OutputSchedulerThread::pushAllFrameRange()
{
    QMutexLocker l(&_imp->framesToRenderMutex);
    RenderDirectionEnum direction;
    int firstFrame, lastFrame, frameStep;
    OutputSchedulerThreadStartArgsPtr runArgs = _imp->runArgs.lock();

    assert(runArgs);
    direction = runArgs->pushTimelineDirection;
    firstFrame = runArgs->firstFrame;
    lastFrame = runArgs->lastFrame;
    frameStep = runArgs->frameStep;


    if (direction == eRenderDirectionForward) {
        for (int i = firstFrame; i <= lastFrame; i += frameStep) {
#ifdef TRACE_SCHEDULER
            qDebug() << "Scheduler Thread: Pushing frame to render: " << i;
#endif
            _imp->framesToRender.push_back(i);
        }
    } else {
        for (int i = lastFrame; i >= firstFrame; i -= frameStep) {
#ifdef TRACE_SCHEDULER
            qDebug() << "Scheduler Thread: Pushing frame to render: " << i;
#endif
            _imp->framesToRender.push_back(i);
        }
    }
    ///Wake up render threads to notify them there's work to do
    _imp->framesToRenderNotEmptyCond.wakeAll();
}

void
OutputSchedulerThread::pushFramesToRender(int nThreads)
{
    QMutexLocker l(&_imp->framesToRenderMutex);
    RenderDirectionEnum direction;
    int firstFrame, lastFrame, frameStep;
    OutputSchedulerThreadStartArgsPtr runArgs = _imp->runArgs.lock();

    assert(runArgs);
    direction = runArgs->pushTimelineDirection;
    firstFrame = runArgs->firstFrame;
    lastFrame = runArgs->lastFrame;
    frameStep = runArgs->frameStep;

    PlaybackModeEnum pMode = _imp->engine->getPlaybackMode();
    int frame = _imp->lastFramePushedIndex;

    if ( (firstFrame == lastFrame) && (frame == firstFrame) ) {
        return;
    }
    RenderDirectionEnum newDirection = direction;
    ///If startingTime is already taken into account in the framesToRender, push new frames from the last one in the stack instead
    bool canContinue = OutputSchedulerThreadPrivate::getNextFrameInSequence(pMode, direction, frame,
                                                                            firstFrame, lastFrame, frameStep, &frame, &newDirection);

    if ( canContinue && (direction != newDirection) ) {
        runArgs->pushTimelineDirection = newDirection;
    }
    if (canContinue) {
        pushFramesToRenderInternal(frame, nThreads);
    } else {
        ///Still wake up threads that may still sleep
        _imp->framesToRenderNotEmptyCond.wakeAll();
    }
}

int
OutputSchedulerThread::pickFrameToRender(RenderThreadTask* thread,
                                         bool* enableRenderStats,
                                         std::vector<ViewIdx>* viewsToRender)
{
    ///Flag the thread as inactive
    {
        QMutexLocker l(&_imp->renderThreadsMutex);
        RenderThreads::iterator found = _imp->getRunnableIterator(thread);
        assert( found != _imp->renderThreads.end() );
        found->active = false;

        ///Wake up the scheduler if it is waiting for all threads do be inactive
        _imp->allRenderThreadsInactiveCond.wakeOne();
    }


    bool gotFrame = false;
    int frame = -1;
    {
        QMutexLocker l(&_imp->framesToRenderMutex);
        while ( _imp->framesToRender.empty() && !thread->mustQuit() && !_imp->engine->isSequentialRenderBeingAborted() ) {
            ///Notify that we're no longer doing work
            thread->notifyIsRunning(false);

            _imp->framesToRenderNotEmptyCond.wait(l.mutex(), NATRON_SCHEDULER_WAIT_SLICE_MS);
        }

        if ( !_imp->framesToRender.empty() ) {
            ///Notify that we're running for good, will do nothing if flagged already running
            thread->notifyIsRunning(true);

            frame = _imp->framesToRender.front();

            _imp->framesToRender.pop_front();

            gotFrame = true;
        }
    }

    // thread is quitting, make sure we notified the application it is no longer running
    if (!gotFrame) {
        thread->notifyIsRunning(false);

        *enableRenderStats = false;

        return -1;
    } else {
        ///Flag the thread as active
        {
            QMutexLocker l(&_imp->renderThreadsMutex);
            RenderThreads::iterator found = _imp->getRunnableIterator(thread);
            assert( found != _imp->renderThreads.end() );
            found->active = true;
        }

        OutputSchedulerThreadStartArgsPtr args = _imp->runArgs.lock();
        *enableRenderStats = args->enableRenderStats;
        *viewsToRender = args->viewsToRender;

        return frame;
    }
} // OutputSchedulerThread::pickFrameToRender

#else // NATRON_PLAYBACK_USES_THREAD_POOL

void
OutputSchedulerThread::startTasksFromLastStartedFrame()
{
    int frame;
    bool canContinue;

    {
        QMutexLocker l(&_imp->framesToRenderMutex);
        RenderDirectionEnum direction;
        int firstFrame, lastFrame, frameStep;
        {
            QMutexLocker l(&_imp->runArgsMutex);
            direction = _imp->livingRunArgs.timelineDirection;
            firstFrame = _imp->livingRunArgs.firstFrame;
            lastFrame = _imp->livingRunArgs.lastFrame;
            frameStep = _imp->livingRunArgs.frameStep;
        }
        PlaybackModeEnum pMode = _imp->engine->getPlaybackMode();

        frame = _imp->lastFramePushedIndex;
        if ( (firstFrame == lastFrame) && (frame == firstFrame) ) {
            return;
        }
        RenderDirectionEnum newDirection = direction;
        ///If startingTime is already taken into account in the framesToRender, push new frames from the last one in the stack instead
        canContinue = OutputSchedulerThreadPrivate::getNextFrameInSequence(pMode, direction, frame,
                                                                           firstFrame, lastFrame, frameStep, &frame, &newDirection);
        if (newDirection != direction) {
            QMutexLocker l(&_imp->runArgsMutex);
            _imp->livingRunArgs.timelineDirection = newDirection;
        }
    }

    if (canContinue) {
        QMutexLocker l(&_imp->renderThreadsMutex);
        startTasks(frame);
    }
}

void
OutputSchedulerThread::startTasks(int startingFrame)
{
    int maxThreads = _imp->threadPool->maxThreadCount();
    int activeThreads = _imp->getNActiveRenderThreads();

    //This thread is from the thread pool so do not count it as it is probably done anyway
    if (QThread::currentThread() != this) {
        activeThreads -= 1;
    }


    int nFrames;
#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
    //We check every now and then if we need to start new threads
    {
        int nbAvailableThreads = maxThreads  - activeThreads;
        if (nbAvailableThreads <= 0) {
            return;
        }
        nFrames = 1;
    }
#else
    //Start one more thread until we use all the thread pool.
    //We leave some CPU available so that the multi-thread suite can take advantage of it
    nFrames = MathUtils::clamp(maxThreads - activeThreads, 1, 1);
#endif


    RenderDirectionEnum direction;
    int firstFrame, lastFrame, frameStep;
    bool useStats;
    std::vector<int> viewsToRender;
    {
        QMutexLocker l(&_imp->runArgsMutex);
        direction = _imp->livingRunArgs.timelineDirection;
        firstFrame = _imp->livingRunArgs.firstFrame;
        lastFrame = _imp->livingRunArgs.lastFrame;
        frameStep = _imp->livingRunArgs.frameStep;
        useStats = _imp->livingRunArgs.enableRenderStats;
        viewsToRender = _imp->livingRunArgs.viewsToRender;
    }
    PlaybackModeEnum pMode = _imp->engine->getPlaybackMode();
    if (firstFrame == lastFrame) {
        RenderThreadTask* task = createRunnable(startingFrame, useStats, viewsToRender);
        _imp->appendRunnable(task);

        QMutexLocker k(&_imp->framesToRenderMutex);
        _imp->lastFramePushedIndex = startingFrame;
    } else {
        int frame = startingFrame;
        RenderDirectionEnum newDirection = direction;
        for (int i = 0; i < nFrames; ++i) {
            RenderThreadTask* task = createRunnable(frame, useStats, viewsToRender);
            _imp->appendRunnable(task);


            {
                QMutexLocker k(&_imp->framesToRenderMutex);
                _imp->lastFramePushedIndex = frame;
            }

            if ( !OutputSchedulerThreadPrivate::getNextFrameInSequence(pMode, direction, frame,
                                                                       firstFrame, lastFrame, frameStep, &frame, &newDirection) ) {
                break;
            }
        }
        if (newDirection != direction) {
            QMutexLocker l(&_imp->runArgsMutex);
            _imp->livingRunArgs.timelineDirection = newDirection;
        }
    }
} // OutputSchedulerThread::startTasks

#endif //NATRON_PLAYBACK_USES_THREAD_POOL


void
OutputSchedulerThread::onThreadSpawnsTimerTriggered()
{
#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER

#ifdef NATRON_PLAYBACK_USES_THREAD_POOL
    startTasksFromLastStartedFrame();
#else
    ///////////
    /////If we were analysing the CPU activity, now set the appropriate number of threads to render.
    int newNThreads, lastNThreads;
    adjustNumberOfThreads(&newNThreads, &lastNThreads);

    ///////////
    /////Append render requests for the render threads
    pushFramesToRender(newNThreads);
#endif

#endif
}

void
OutputSchedulerThread::notifyThreadAboutToQuit(RenderThreadTask* thread)
{
    QMutexLocker l(&_imp->renderThreadsMutex);
    RenderThreads::iterator found = _imp->getRunnableIterator(thread);

    if ( found != _imp->renderThreads.end() ) {
        found->active = false;
#ifdef NATRON_PLAYBACK_USES_THREAD_POOL
        _imp->renderThreads.erase(found);
#endif
        _imp->allRenderThreadsInactiveCond.wakeOne();

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        _imp->allRenderThreadsQuitCond.wakeOne();
#endif
    }
}

void
OutputSchedulerThread::startRender()
{
    if ( isFPSRegulationNeeded() ) {
        _imp->timer.playState = ePlayStateRunning;
    }

    // Start measuring
    _imp->renderTimer.reset(new TimeLapse);

    ///We will push frame to renders starting at startingFrame.
    ///They will be in the range determined by firstFrame-lastFrame
    int startingFrame;
    int firstFrame, lastFrame;
    int frameStep;
    bool forward;
    OutputSchedulerThreadStartArgsPtr args = _imp->runArgs.lock();
    ///Copy the last requested run args

    firstFrame = args->firstFrame;
    lastFrame = args->lastFrame;
    frameStep = args->frameStep;
    startingFrame = timelineGetTime();
    forward = args->pushTimelineDirection == eRenderDirectionForward;


    aboutToStartRender();

    ///Notify everyone that the render is started
    _imp->engine->s_renderStarted(forward);

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    int nThreads;
    {
        QMutexLocker l(&_imp->renderThreadsMutex);
        _imp->removeAllQuitRenderThreads();
        nThreads = (int)_imp->renderThreads.size();
    }

    ///Start with one thread if it doesn't exist
    if (nThreads == 0) {
        int lastNThreads;
        adjustNumberOfThreads(&nThreads, &lastNThreads);
    }
#endif

    QMutexLocker l(&_imp->renderThreadsMutex);


    ///If the output effect is sequential (only WriteFFMPEG for now)
    EffectInstancePtr effect = _imp->outputEffect.lock();
    WriteNode* isWriteNode = dynamic_cast<WriteNode*>( effect.get() );
    if (isWriteNode) {
        NodePtr embeddedWriter = isWriteNode->getEmbeddedWriter();
        if (embeddedWriter) {
            effect = embeddedWriter->getEffectInstance();
        }
    }
    SequentialPreferenceEnum pref = effect->getSequentialPreference();
    if ( (pref == eSequentialPreferenceOnlySequential) || (pref == eSequentialPreferencePreferSequential) ) {
        if (effect->beginSequenceRender_public( firstFrame, lastFrame,
                                                frameStep,
                                                false,
                                                RenderScale::identity, true,
                                                true,
                                                false,
                                                ViewIdx(0),
                                                false /*useOpenGL*/,
                                                EffectInstance::OpenGLContextEffectDataPtr() ) == eStatusFailed) {
            l.unlock();


            _imp->engine->abortRenderingNoRestart();

            return;
        }
    }

    {
        QMutexLocker k(&_imp->framesToRenderMutex);
        _imp->expectFrameToRender = startingFrame;
    }
    SchedulingPolicyEnum policy = getSchedulingPolicy();
    if (policy == eSchedulingPolicyFFA) {
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        ///push all frame range and let the threads deal with it
        pushAllFrameRange();
#endif
    } else {
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        ///Push as many frames as there are threads
        pushFramesToRender(startingFrame, nThreads);
#endif
    }


#ifdef NATRON_PLAYBACK_USES_THREAD_POOL
    startTasks(startingFrame);
#endif

#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
    QMutexLocker k(&_imp->lastRecordedFPSMutex);
    double timeoutMS = _imp->lastRecordedFPS == 0. ? NATRON_SCHEDULER_THREADS_SPAWN_DEFAULT_TIMEOUT_MS : (1. / _imp->lastRecordedFPS) * 1000;
    _imp->threadSpawnsTimer.start(timeoutMS);
#endif
} // OutputSchedulerThread::startRender

void
OutputSchedulerThread::stopRender()
{
    _imp->timer.playState = ePlayStatePause;

#ifdef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER
    QMutexLocker k(&_imp->lastRecordedFPSMutex);
    _imp->lastRecordedFPS = _imp->timer.getActualFrameRate();
    _imp->threadSpawnsTimer.stop();
#endif

    ///Wait for all render threads to be done

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    ///Clear the work queue
    {
        QMutexLocker framesLocker (&_imp->framesToRenderMutex);
        _imp->framesToRender.clear();
    }
#endif

    ///Remove all current threads so the new render doesn't have many threads concurrently trying to do the same thing at the same time
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    stopRenderThreads(0);
#endif
    _imp->waitForRenderThreadsToQuit();

    ///If the output effect is sequential (only WriteFFMPEG for now)
    EffectInstancePtr effect = _imp->outputEffect.lock();
    WriteNode* isWriteNode = dynamic_cast<WriteNode*>( effect.get() );
    if (isWriteNode) {
        NodePtr embeddedWriter = isWriteNode->getEmbeddedWriter();
        if (embeddedWriter) {
            effect = embeddedWriter->getEffectInstance();
        }
    }
    SequentialPreferenceEnum pref = effect->getSequentialPreference();
    if ( (pref == eSequentialPreferenceOnlySequential) || (pref == eSequentialPreferencePreferSequential) ) {
        int firstFrame, lastFrame;
        OutputSchedulerThreadStartArgsPtr args = _imp->runArgs.lock();
        firstFrame = args->firstFrame;
        lastFrame = args->lastFrame;

        ignore_result( effect->endSequenceRender_public( firstFrame, lastFrame,
                                                         1,
                                                         !appPTR->isBackground(),
                                                         RenderScale::identity, true,
                                                         !appPTR->isBackground(),
                                                         false,
                                                         ViewIdx(0),
                                                         false /*use OpenGL render*/,
                                                         EffectInstance::OpenGLContextEffectDataPtr() ) );
    }


    bool wasAborted = isBeingAborted();


    ///Notify everyone that the render is finished
    _imp->engine->s_renderFinished(wasAborted ? 1 : 0);

    onRenderStopped(wasAborted);

    // When playing once disable auto-restart
    if (!wasAborted && _imp->engine->getPlaybackMode() == ePlaybackModeOnce) {
        _imp->engine->setPlaybackAutoRestartEnabled(false);
    }


    {
        QMutexLocker k(&_imp->bufMutex);
        _imp->buf.clear();
    }

    _imp->renderTimer.reset();
} // OutputSchedulerThread::stopRender

#include "OutputSchedulerThreadPart1.inc"
#include "OutputSchedulerThreadPart2.inc"
#include "OutputSchedulerThreadPart3.inc"
