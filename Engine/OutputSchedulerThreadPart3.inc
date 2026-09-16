void
RenderEngine::renderFrameRange(bool isBlocking,
                               bool enableRenderStats,
                               int firstFrame,
                               int lastFrame,
                               int frameStep,
                               const std::vector<ViewIdx>& viewsToRender,
                               RenderDirectionEnum forward)
{
    setPlaybackAutoRestartEnabled(true);

    {
        QMutexLocker k(&_imp->schedulerCreationLock);
        if (!_imp->scheduler) {
            _imp->scheduler = createScheduler( _imp->output.lock() );
        }
    }

    _imp->scheduler->renderFrameRange(isBlocking, enableRenderStats, firstFrame, lastFrame, frameStep, viewsToRender, forward);
}

void
RenderEngine::renderFromCurrentFrame(bool enableRenderStats,
                                     const std::vector<ViewIdx>& viewsToRender,
                                     RenderDirectionEnum forward)
{
    setPlaybackAutoRestartEnabled(true);

    {
        QMutexLocker k(&_imp->schedulerCreationLock);
        if (!_imp->scheduler) {
            _imp->scheduler = createScheduler( _imp->output.lock() );
        }
    }

    _imp->scheduler->renderFromCurrentFrame(enableRenderStats, viewsToRender, forward);
}

void
RenderEngine::onCurrentFrameRenderRequestPosted()
{
    assert( QThread::currentThread() == qApp->thread() );

    //Okay we are at the end of the event loop, concatenate all similar events
    RenderEnginePrivate::RefreshRequest r;
    bool rSet = false;
    while ( !_imp->refreshQueue.empty() ) {
        const RenderEnginePrivate::RefreshRequest& queueBegin = _imp->refreshQueue.front();
        if (!rSet) {
            rSet = true;
        } else {
            if ( (queueBegin.enableAbort == r.enableAbort) && (queueBegin.enableStats == r.enableStats) ) {
                _imp->refreshQueue.pop_front();
                continue;
            }
        }
        r = queueBegin;
        renderCurrentFrameInternal(r.enableStats, r.enableAbort);
        _imp->refreshQueue.pop_front();
    }
}

void
RenderEngine::renderCurrentFrameInternal(bool enableRenderStats,
                                         bool canAbort)
{
    assert( QThread::currentThread() == qApp->thread() );

    ViewerInstance* isViewer = dynamic_cast<ViewerInstance*>( _imp->output.lock().get() );
    if (!isViewer) {
        qDebug() << "RenderEngine::renderCurrentFrame for a writer is unsupported";

        return;
    }


    ///If the scheduler is already doing playback, continue it
    if (_imp->scheduler) {
        bool working = _imp->scheduler->isWorking();
        if (working) {
            _imp->scheduler->abortThreadedTask();
        }
        if ( working || isPlaybackAutoRestartEnabled() ) {
            RenderDirectionEnum lastDirection;
            std::vector<ViewIdx> lastViews;
            _imp->scheduler->getLastRunArgs(&lastDirection, &lastViews);
            _imp->scheduler->renderFromCurrentFrame( enableRenderStats, lastViews,  lastDirection);

            return;
        }
    }


    {
        QMutexLocker k(&_imp->schedulerCreationLock);
        if (!_imp->scheduler) {
            _imp->scheduler = createScheduler( _imp->output.lock() );
        }
    }

    if (!_imp->currentFrameScheduler) {
        _imp->currentFrameScheduler = new ViewerCurrentFrameRequestScheduler(isViewer);
    }

    _imp->currentFrameScheduler->renderCurrentFrame(enableRenderStats, canAbort);
}

void
RenderEngine::renderCurrentFrame(bool enableRenderStats,
                                 bool canAbort)
{
    assert( QThread::currentThread() == qApp->thread() );
    RenderEnginePrivate::RefreshRequest r;
    r.enableStats = enableRenderStats;
    r.enableAbort = canAbort;
    _imp->refreshQueue.push_back(r);
    Q_EMIT currentFrameRenderRequestPosted();
}

void
RenderEngine::setPlaybackAutoRestartEnabled(bool enabled)
{
    QMutexLocker k(&_imp->canAutoRestartPlaybackMutex);

    _imp->canAutoRestartPlayback = enabled;
}

bool
RenderEngine::isPlaybackAutoRestartEnabled() const
{
    QMutexLocker k(&_imp->canAutoRestartPlaybackMutex);

    return _imp->canAutoRestartPlayback;
}

void
RenderEngine::quitEngine(bool allowRestarts)
{
    if (_imp->scheduler) {
        _imp->scheduler->quitThread(allowRestarts);
    }

    if (_imp->currentFrameScheduler) {
        _imp->currentFrameScheduler->quitThread(allowRestarts);
    }
}

void
RenderEngine::waitForEngineToQuit_not_main_thread()
{
    if (_imp->scheduler) {
        _imp->scheduler->waitForThreadToQuit_not_main_thread();
    }

    if (_imp->currentFrameScheduler) {
        _imp->currentFrameScheduler->waitForThreadToQuit_not_main_thread();
    }
}

void
RenderEngine::waitForEngineToQuit_main_thread(bool allowRestart)
{
    assert( QThread::currentThread() == qApp->thread() );
    assert(!_imp->engineWatcher);
    _imp->engineWatcher.reset( new RenderEngineWatcher(this) );
    QObject::connect( _imp->engineWatcher.get(), SIGNAL(taskFinished(int,GenericWatcherCallerArgsPtr)), this, SLOT(onWatcherEngineQuitEmitted()) );
    _imp->engineWatcher->scheduleBlockingTask(allowRestart ? RenderEngineWatcher::eBlockingTaskWaitForQuitAllowRestart : RenderEngineWatcher::eBlockingTaskWaitForQuitDisallowRestart);
}

void
RenderEngine::waitForEngineToQuit_enforce_blocking()
{
    if (_imp->scheduler) {
        _imp->scheduler->waitForThreadToQuit_enforce_blocking();
    }

    if (_imp->currentFrameScheduler) {
        _imp->currentFrameScheduler->waitForThreadToQuit_enforce_blocking();
    }
}

bool
RenderEngine::abortRenderingInternal(bool keepOldestRender)
{
    bool ret = false;

    if (_imp->currentFrameScheduler) {
        ret |= _imp->currentFrameScheduler->abortThreadedTask(keepOldestRender);
    }

    if ( _imp->scheduler && _imp->scheduler->isWorking() ) {
        //If any playback active, abort it
        ret |= _imp->scheduler->abortThreadedTask(keepOldestRender);
    }

    return ret;
}

bool
RenderEngine::abortRenderingNoRestart(bool keepOldestRender)
{
    if ( abortRenderingInternal(keepOldestRender) ) {
        setPlaybackAutoRestartEnabled(false);

        return true;
    }

    return false;
}

bool
RenderEngine::abortRenderingAutoRestart()
{
    if ( abortRenderingInternal(true) ) {
        return true;
    }

    return false;
}

void
RenderEngine::waitForAbortToComplete_not_main_thread()
{
    if (_imp->currentFrameScheduler) {
        _imp->currentFrameScheduler->waitForAbortToComplete_not_main_thread();
    }
    if (_imp->scheduler) {
        _imp->scheduler->waitForAbortToComplete_not_main_thread();
    }
}

void
RenderEngine::waitForAbortToComplete_enforce_blocking()
{
    if (_imp->scheduler) {
        _imp->scheduler->waitForAbortToComplete_enforce_blocking();
    }

    if (_imp->currentFrameScheduler) {
        _imp->currentFrameScheduler->waitForAbortToComplete_enforce_blocking();
    }
}

void
RenderEngine::onWatcherEngineAbortedEmitted()
{
    assert(_imp->engineWatcher);
    if (!_imp->engineWatcher) {
        return;
    }
    _imp->engineWatcher.reset();
    Q_EMIT engineAborted();
}

void
RenderEngine::onWatcherEngineQuitEmitted()
{
    assert(_imp->engineWatcher);
    if (!_imp->engineWatcher) {
        return;
    }
    _imp->engineWatcher.reset();
    Q_EMIT engineQuit();
}

void
RenderEngine::waitForAbortToComplete_main_thread()
{
    assert( QThread::currentThread() == qApp->thread() );
    assert(!_imp->engineWatcher);
    _imp->engineWatcher.reset( new RenderEngineWatcher(this) );
    QObject::connect( _imp->engineWatcher.get(), SIGNAL(taskFinished(int,GenericWatcherCallerArgsPtr)), this, SLOT(onWatcherEngineAbortedEmitted()) );
    _imp->engineWatcher->scheduleBlockingTask(RenderEngineWatcher::eBlockingTaskWaitForAbort);
}

bool
RenderEngine::isSequentialRenderBeingAborted() const
{
    if (!_imp->scheduler) {
        return false;
    }

    return _imp->scheduler->isBeingAborted();
}

bool
RenderEngine::hasThreadsAlive() const
{
    bool schedulerRunning = false;

    if (_imp->scheduler) {
        schedulerRunning = _imp->scheduler->isRunning();
    }
    bool currentFrameSchedulerRunning = false;
    if (_imp->currentFrameScheduler) {
        currentFrameSchedulerRunning = _imp->currentFrameScheduler->isRunning();
    }

    return schedulerRunning || currentFrameSchedulerRunning;
}

bool
RenderEngine::hasThreadsWorking() const
{
    bool working = false;

    if (_imp->scheduler) {
        working |= _imp->scheduler->isWorking();
    }

    if (!working && _imp->currentFrameScheduler) {
        working |= _imp->currentFrameScheduler->isWorking();
    }

    return working;
}

bool
RenderEngine::isDoingSequentialRender() const
{
    return _imp->scheduler ? _imp->scheduler->isWorking() : false;
}

void
RenderEngine::setPlaybackMode(int mode)
{
    QMutexLocker l(&_imp->pbModeMutex);

    _imp->pbMode = (PlaybackModeEnum)mode;
}

PlaybackModeEnum
RenderEngine::getPlaybackMode() const
{
    QMutexLocker l(&_imp->pbModeMutex);

    return _imp->pbMode;
}

void
RenderEngine::setDesiredFPS(double d)
{
    {
        QMutexLocker k(&_imp->schedulerCreationLock);
        if (!_imp->scheduler) {
            _imp->scheduler = createScheduler( _imp->output.lock() );
        }
    }
    _imp->scheduler->setDesiredFPS(d);
}

double
RenderEngine::getDesiredFPS() const
{
    return _imp->scheduler ? _imp->scheduler->getDesiredFPS() : 24;
}

void
RenderEngine::notifyFrameProduced(const BufferableObjectPtrList& frames,
                                  const RenderStatsPtr& stats,
                                  const ViewerCurrentFrameRequestSchedulerStartArgsPtr& request)
{
    _imp->currentFrameScheduler->notifyFrameProduced(frames, stats, request);
}

OutputSchedulerThread*
ViewerRenderEngine::createScheduler(const OutputEffectInstancePtr& effect)
{
    return new ViewerDisplayScheduler( this, std::dynamic_pointer_cast<ViewerInstance>(effect) );
}

////////////////////////ViewerCurrentFrameRequestScheduler////////////////////////
class CurrentFrameFunctorArgs
    : public GenericThreadStartArgs
{
public:

    ViewIdx view;
    int time;
    RenderStatsPtr stats;
    ViewerInstance* viewer;
    U64 viewerHash;
    ViewerCurrentFrameRequestSchedulerStartArgsPtr request;
    ViewerCurrentFrameRequestSchedulerPrivate* scheduler;
    bool canAbort;
    NodePtr isRotoPaintRequest;
    RotoStrokeItemWPtr strokeItem;
    ViewerArgsPtr args[2];
    bool isRotoNeatRender;

    CurrentFrameFunctorArgs()
        : GenericThreadStartArgs()
        , view(0)
        , time(0)
        , stats()
        , viewer(0)
        , viewerHash(0)
        , request()
        , scheduler(0)
        , canAbort(true)
        , isRotoPaintRequest()
        , strokeItem()
        , args()
        , isRotoNeatRender(false)
    {
    }

    CurrentFrameFunctorArgs(ViewIdx view,
                            int time,
                            const RenderStatsPtr& stats,
                            ViewerInstance* viewer,
                            U64 viewerHash,
                            ViewerCurrentFrameRequestSchedulerPrivate* scheduler,
                            bool canAbort,
                            const NodePtr& isRotoPaintRequest,
                            const RotoStrokeItemPtr& strokeItem,
                            bool isRotoNeatRender)
        : GenericThreadStartArgs()
        , view(view)
        , time(time)
        , stats(stats)
        , viewer(viewer)
        , viewerHash(viewerHash)
        , request()
        , scheduler(scheduler)
        , canAbort(canAbort)
        , isRotoPaintRequest(isRotoPaintRequest)
        , strokeItem(strokeItem)
        , args()
        , isRotoNeatRender(isRotoNeatRender)
    {
        if (isRotoPaintRequest && isRotoNeatRender) {
            isRotoPaintRequest->getRotoContext()->setIsDoingNeatRender(true);
        }
    }

    ~CurrentFrameFunctorArgs()
    {
        if (isRotoPaintRequest && isRotoNeatRender) {
            isRotoPaintRequest->getRotoContext()->setIsDoingNeatRender(false);
        }
    }
};

typedef std::shared_ptr<CurrentFrameFunctorArgs> CurrentFrameFunctorArgsPtr;

class RenderCurrentFrameFunctorRunnable;
struct ViewerCurrentFrameRequestSchedulerPrivate
{
    ViewerInstance* viewer;
    QThreadPool* threadPool;
    QMutex producedFramesMutex;
    ProducedFrameSet producedFrames;
    QWaitCondition producedFramesNotEmpty;


    /**
     * Single thread used by the ViewerCurrentFrameRequestScheduler when the global thread pool has reached its maximum
     * activity to keep the renders responsive even if the thread pool is choking.
     **/
    ViewerCurrentFrameRequestRendererBackup backupThread;
    mutable QMutex currentFrameRenderTasksMutex;
    QWaitCondition currentFrameRenderTasksCond;
    std::list<RenderCurrentFrameFunctorRunnable*> currentFrameRenderTasks;

    // Used to attribute an age to each renderCurrentFrameRequest
    U64 ageCounter;

    ViewerCurrentFrameRequestSchedulerPrivate(ViewerInstance* viewer)
        : viewer(viewer)
        , threadPool( QThreadPool::globalInstance() )
        , producedFramesMutex()
        , producedFrames()
        , producedFramesNotEmpty()
        , backupThread()
        , currentFrameRenderTasksCond()
        , currentFrameRenderTasks()
        , ageCounter(0)
    {
    }

    void appendRunnableTask(RenderCurrentFrameFunctorRunnable* task)
    {
        {
            QMutexLocker k(&currentFrameRenderTasksMutex);
            currentFrameRenderTasks.push_back(task);
        }
    }

    void removeRunnableTask(RenderCurrentFrameFunctorRunnable* task)
    {
        {
            QMutexLocker k(&currentFrameRenderTasksMutex);
            for (std::list<RenderCurrentFrameFunctorRunnable*>::iterator it = currentFrameRenderTasks.begin();
                 it != currentFrameRenderTasks.end(); ++it) {
                if (*it == task) {
                    currentFrameRenderTasks.erase(it);

                    currentFrameRenderTasksCond.wakeAll();
                    break;
                }
            }
        }
    }

    void waitForRunnableTasks()
    {
        QMutexLocker k(&currentFrameRenderTasksMutex);

        while ( !currentFrameRenderTasks.empty() ) {
            currentFrameRenderTasksCond.wait(k.mutex());
        }
    }

    void notifyFrameProduced(const BufferableObjectPtrList& frames,
                             const RenderStatsPtr& stats,
                             U64 age)
    {
        QMutexLocker k(&producedFramesMutex);
        ProducedFrame p;

        p.frames = frames;
        p.age = age;
        p.stats = stats;
        producedFrames.insert(p);
        producedFramesNotEmpty.wakeOne();
    }

    void processProducedFrame(const RenderStatsPtr& stats, const BufferableObjectPtrList& frames);
};

class RenderCurrentFrameFunctorRunnable
    : public QRunnable
{
    CurrentFrameFunctorArgsPtr _args;

public:

    RenderCurrentFrameFunctorRunnable(const CurrentFrameFunctorArgsPtr& args)
        : _args(args)
    {
    }

    virtual ~RenderCurrentFrameFunctorRunnable()
    {
    }

    virtual void run() OVERRIDE FINAL
    {
#ifdef DEBUG
        boost_adaptbx::floating_point::exception_trapping trap(boost_adaptbx::floating_point::exception_trapping::division_by_zero |
                                                               boost_adaptbx::floating_point::exception_trapping::invalid |
                                                               boost_adaptbx::floating_point::exception_trapping::overflow);
#endif
        ///The viewer always uses the scheduler thread to regulate the output rate, @see ViewerInstance::renderViewer_internal
        ///it calls appendToBuffer by itself
        ViewerInstance::ViewerRenderRetCode stat = ViewerInstance::eViewerRenderRetCodeFail;
        BufferableObjectPtrList ret;

        try {
            if (!_args->isRotoPaintRequest || _args->isRotoNeatRender) {
                stat = _args->viewer->renderViewer(_args->view, QThread::currentThread() == qApp->thread(), false, _args->viewerHash, _args->canAbort,
                                                   NodePtr(), true, _args->args, _args->request, _args->stats);
            } else {
                stat = _args->viewer->getViewerArgsAndRenderViewer(_args->time, _args->canAbort, _args->view, _args->viewerHash, _args->isRotoPaintRequest, _args->strokeItem.lock(), _args->stats, &_args->args[0], &_args->args[1]);
            }
        } catch (...) {
            stat = ViewerInstance::eViewerRenderRetCodeFail;
        }

        if (stat == ViewerInstance::eViewerRenderRetCodeFail) {
            ///Don't report any error message otherwise we will flood the viewer with irrelevant messages such as
            ///"Render failed", instead we let the plug-in that failed post an error message which will be more helpful.
            _args->viewer->disconnectViewer();
            ret.clear();
        } else {
            for (int i = 0; i < 2; ++i) {
                if (_args->args[i] && _args->args[i]->params) {
                    if (_args->args[i]->params->tiles.size() > 0) {
                        ret.push_back(_args->args[i]->params);
                    }
                }
            }
        }

        if (_args->request) {
#ifdef DEBUG
            for (BufferableObjectPtrList::iterator it = ret.begin(); it != ret.end(); ++it) {
                UpdateViewerParams* isParams = dynamic_cast<UpdateViewerParams*>( it->get() );
                assert(isParams);
                assert( !isParams->tiles.empty() );
                for (std::list<UpdateViewerParams::CachedTile>::iterator it2 = isParams->tiles.begin(); it2 != isParams->tiles.end(); ++it2) {
                    assert(it2->ramBuffer);
                }
            }
#endif
            _args->scheduler->notifyFrameProduced(ret, _args->stats, _args->request->age);
        } else {
            assert( QThread::currentThread() == qApp->thread() );
            _args->scheduler->processProducedFrame(_args->stats, ret);
        }

        ///This thread is done, clean-up its TLS
        appPTR->getAppTLS()->cleanupTLSForThread();


        _args->scheduler->removeRunnableTask(this);
    } // run
};


class ViewerCurrentFrameRequestSchedulerExecOnMT
    : public GenericThreadExecOnMainThreadArgs
{
public:

    RenderStatsPtr stats;
    BufferableObjectPtrList frames;

    ViewerCurrentFrameRequestSchedulerExecOnMT()
        : GenericThreadExecOnMainThreadArgs()
    {
    }

    virtual ~ViewerCurrentFrameRequestSchedulerExecOnMT()
    {
    }
};

typedef std::shared_ptr<ViewerCurrentFrameRequestSchedulerExecOnMT> ViewerCurrentFrameRequestSchedulerExecOnMTPtr;

ViewerCurrentFrameRequestScheduler::ViewerCurrentFrameRequestScheduler(ViewerInstance* viewer)
    : GenericSchedulerThread()
    , _imp( new ViewerCurrentFrameRequestSchedulerPrivate(viewer) )
{
    setThreadName("ViewerCurrentFrameRequestScheduler");
}

ViewerCurrentFrameRequestScheduler::~ViewerCurrentFrameRequestScheduler()
{
    // Should've been stopped before anyway
    if (_imp->backupThread.quitThread(false)) {
        _imp->backupThread.waitForAbortToComplete_enforce_blocking();
    }
}

GenericSchedulerThread::TaskQueueBehaviorEnum
ViewerCurrentFrameRequestScheduler::tasksQueueBehaviour() const
{
    return eTaskQueueBehaviorProcessInOrder;
}

GenericSchedulerThread::ThreadStateEnum
ViewerCurrentFrameRequestScheduler::threadLoopOnce(const GenericThreadStartArgsPtr &inArgs)
{
    ThreadStateEnum state = eThreadStateActive;
    ViewerCurrentFrameRequestSchedulerStartArgsPtr args = std::dynamic_pointer_cast<ViewerCurrentFrameRequestSchedulerStartArgs>(inArgs);

    assert(args);

#ifdef TRACE_CURRENT_FRAME_SCHEDULER
    qDebug() << getThreadName().c_str() << "Thread loop once, waiting for" << args->age << "to be produced";
#endif

    ///Wait for the work to be done
    ViewerCurrentFrameRequestSchedulerExecOnMTPtr mtArgs = std::make_shared<ViewerCurrentFrameRequestSchedulerExecOnMT>();
    {
        QMutexLocker k(&_imp->producedFramesMutex);
        ProducedFrameSet::iterator found = _imp->producedFrames.end();
        for (ProducedFrameSet::iterator it = _imp->producedFrames.begin(); it != _imp->producedFrames.end(); ++it) {
            if (it->age == args->age) {
                found = it;
                break;
            }
        }

        while ( found == _imp->producedFrames.end() ) {
            state = resolveState();
            if ( (state == eThreadStateStopped) || (state == eThreadStateAborted) ) {
                //_imp->producedFrames.clear();
                break;
            }
            _imp->producedFramesNotEmpty.wait(k.mutex());

            for (ProducedFrameSet::iterator it = _imp->producedFrames.begin(); it != _imp->producedFrames.end(); ++it) {
                if (it->age == args->age) {
                    found = it;
                    break;
                }
            }
        }

        if ( found != _imp->producedFrames.end() ) {
#ifdef TRACE_CURRENT_FRAME_SCHEDULER
            qDebug() << getThreadName().c_str() << "Found" << args->age << "produced";
#endif

            mtArgs->frames = found->frames;
            mtArgs->stats = found->stats;

            // Erase from the produced frames all renders that are older that the age we want to render
            // since they are no longer going to be used.
            // Only do this if the render had frames it is not just a redraw
            //if (!found->frames.empty()) {
            //    ++found;
            //    _imp->producedFrames.erase(_imp->producedFrames.begin(), found);
            //} else {
            ++found;
            _imp->producedFrames.erase(_imp->producedFrames.begin(), found);
            //}
        } else {
#ifdef TRACE_CURRENT_FRAME_SCHEDULER
            qDebug() << getThreadName().c_str() << "Got aborted, skip waiting for" << args->age;
#endif
        }
    } // QMutexLocker k(&_imp->producedQueueMutex);


    if (state == eThreadStateActive) {
        state = resolveState();
    }
    // Do not uncomment the second part: if we don't update on the viewer aborted tasks then user will never even see valid images that were fully rendered
    if (state == eThreadStateStopped /*|| (state == eThreadStateAborted)*/ ) {
        return state;
    }

    requestExecutionOnMainThread(mtArgs);

#ifdef TRACE_CURRENT_FRAME_SCHEDULER
    qDebug() << getThreadName().c_str() << "Frame processed" << args->age;
#endif

    return state;
} // ViewerCurrentFrameRequestScheduler::threadLoopOnce

void
ViewerCurrentFrameRequestScheduler::executeOnMainThread(const GenericThreadExecOnMainThreadArgsPtr& inArgs)
{
    ViewerCurrentFrameRequestSchedulerExecOnMT* args = dynamic_cast<ViewerCurrentFrameRequestSchedulerExecOnMT*>( inArgs.get() );

    assert(args);
    if (args) {
        _imp->processProducedFrame(args->stats, args->frames);
    }
}

void
ViewerCurrentFrameRequestSchedulerPrivate::processProducedFrame(const RenderStatsPtr& stats,
                                                                const BufferableObjectPtrList& frames)
{
    assert( QThread::currentThread() == qApp->thread() );

    if ( !frames.empty() ) {
        viewer->aboutToUpdateTextures();
    }

    //bool hasDoneSomething = false;
    for (BufferableObjectPtrList::const_iterator it2 = frames.begin(); it2 != frames.end(); ++it2) {
        assert(*it2);
        UpdateViewerParamsPtr params = std::dynamic_pointer_cast<UpdateViewerParams>(*it2);
        assert(params);
        if ( params && (params->tiles.size() >= 1) ) {
            if (stats) {
                double timeSpent;
                std::map<NodePtr, NodeRenderStats > ret = stats->getStats(&timeSpent);
                viewer->reportStats(0, ViewIdx(0), timeSpent, ret);
            }

            viewer->updateViewer(params);
        }
    }


    ///At least redraw the viewer, we might be here when the user removed a node upstream of the viewer.
    viewer->redrawViewer();
}

void
ViewerCurrentFrameRequestScheduler::onAbortRequested(bool keepOldestRender)
{
#ifdef TRACE_CURRENT_FRAME_SCHEDULER
    qDebug() << getThreadName().c_str() << "Received abort request";
#endif
    //This will make all processing nodes that call the abort() function return true
    //This function marks all active renders of the viewer as aborted (except the oldest one)
    //and each node actually check if the render has been aborted in EffectInstance::Implementation::aborted()
    _imp->viewer->markAllOnGoingRendersAsAborted(keepOldestRender);
    _imp->backupThread.abortThreadedTask();
}

void
ViewerCurrentFrameRequestScheduler::onQuitRequested(bool allowRestarts)
{
    _imp->backupThread.quitThread(allowRestarts);
}

void
ViewerCurrentFrameRequestScheduler::onWaitForThreadToQuit()
{
    _imp->waitForRunnableTasks();
    _imp->backupThread.waitForThreadToQuit_enforce_blocking();
}

void
ViewerCurrentFrameRequestScheduler::onWaitForAbortCompleted()
{
    _imp->waitForRunnableTasks();
    _imp->backupThread.waitForAbortToComplete_enforce_blocking();
}

void
ViewerCurrentFrameRequestScheduler::notifyFrameProduced(const BufferableObjectPtrList& frames,
                                                        const RenderStatsPtr& stats,
                                                        const ViewerCurrentFrameRequestSchedulerStartArgsPtr& request)
{
    _imp->notifyFrameProduced(frames, stats,  request->age);
}

void
ViewerCurrentFrameRequestScheduler::renderCurrentFrame(bool enableRenderStats,
                                                       bool canAbort)
{
    int frame = _imp->viewer->getTimeline()->currentFrame();
    int viewsCount = _imp->viewer->getRenderViewsCount();
    ViewIdx view = viewsCount > 0 ? _imp->viewer->getViewerCurrentView() : ViewIdx(0);
    U64 viewerHash = _imp->viewer->getHash();
    ViewerInstance::ViewerRenderRetCode status[2] = {
        ViewerInstance::eViewerRenderRetCodeFail, ViewerInstance::eViewerRenderRetCodeFail
    };

    // Do not render viewer that are not made visible by the user
    if ( !_imp->viewer->isViewerUIVisible() ) {
        return;
    }

    RenderStatsPtr stats;
    if (enableRenderStats) {
        stats.reset( new RenderStats(enableRenderStats) );
    }

    bool isTracking = _imp->viewer->isDoingPartialUpdates();
    NodePtr rotoPaintNode;
    RotoStrokeItemPtr curStroke;
    bool isDrawing = false;
    _imp->viewer->getApp()->getActiveRotoDrawingStroke(&rotoPaintNode, &curStroke, &isDrawing);

    bool rotoUse1Thread = false;
    bool isRotoNeatRender = false;
    if (!isDrawing) {
        isRotoNeatRender = rotoPaintNode ? rotoPaintNode->getRotoContext()->mustDoNeatRender() : false;
        if (rotoPaintNode && isRotoNeatRender) {
            rotoUse1Thread = true;
        } else {
            rotoPaintNode.reset();
            curStroke.reset();
        }
    } else {
        assert(rotoPaintNode);
        rotoUse1Thread = true;
    }

    ViewerArgsPtr args[2];
    if (!rotoPaintNode || isRotoNeatRender) {
        bool clearTexture[2] = {false, false};

        for (int i = 0; i < 2; ++i) {
            args[i] = std::make_shared<ViewerArgs>();
            status[i] = _imp->viewer->getRenderViewerArgsAndCheckCache_public( frame, false, view, i, viewerHash, canAbort, rotoPaintNode, stats, args[i].get() );

            clearTexture[i] = status[i] == ViewerInstance::eViewerRenderRetCodeFail || status[i] == ViewerInstance::eViewerRenderRetCodeBlack;
            if (clearTexture[i] || args[i]->params->isViewerPaused) {
                //Just clear the viewer, nothing to do
                args[i]->params.reset();
            }

            if ( (status[i] == ViewerInstance::eViewerRenderRetCodeRedraw) && args[i]->params ) {
                //We must redraw (re-render) don't hold a pointer to the cached frame
                args[i]->params->tiles.clear();
            }
        }

        if (status[0] == ViewerInstance::eViewerRenderRetCodeFail && status[1] == ViewerInstance::eViewerRenderRetCodeFail) {
            _imp->viewer->disconnectViewer();
            return;
        }
        if (clearTexture[0]) {
            _imp->viewer->disconnectTexture(0, status[0] == ViewerInstance::eViewerRenderRetCodeFail);
        }
        if (clearTexture[0]) {
            _imp->viewer->disconnectTexture(1, status[1] == ViewerInstance::eViewerRenderRetCodeFail);
        }

        if ( (status[0] == ViewerInstance::eViewerRenderRetCodeRedraw) && (status[1] == ViewerInstance::eViewerRenderRetCodeRedraw) ) {
            _imp->viewer->redrawViewer();

            return;
        }

        bool hasTextureCached = false;
        for (int i = 0; i < 2; ++i) {
            if ( args[i]->params && (args[i]->params->nbCachedTile > 0) && ( args[i]->params->nbCachedTile == (int)args[i]->params->tiles.size() ) ) {
                hasTextureCached = true;
                break;
            }
        }
        if (hasTextureCached) {
            _imp->viewer->aboutToUpdateTextures();
        }

        for (int i = 0; i < 2; ++i) {
            if ( args[i]->params && (args[i]->params->nbCachedTile > 0) && ( args[i]->params->nbCachedTile == (int)args[i]->params->tiles.size() ) ) {
                /*
                   The texture was cached
                 */
                if ( stats && (i == 0) ) {
                    double timeSpent;
                    std::map<NodePtr, NodeRenderStats > statResults = stats->getStats(&timeSpent);
                    _imp->viewer->reportStats(frame, view, timeSpent, statResults);
                }
                _imp->viewer->updateViewer(args[i]->params);
                args[i].reset();
            }
        }
        if ( (!args[0] && !args[1]) ||
             ( !args[0] && ( status[0] == ViewerInstance::eViewerRenderRetCodeRender) && args[1] && ( status[1] == ViewerInstance::eViewerRenderRetCodeFail) ) ||
             ( !args[1] && ( status[1] == ViewerInstance::eViewerRenderRetCodeRender) && args[0] && ( status[0] == ViewerInstance::eViewerRenderRetCodeFail) ) ) {
            _imp->viewer->redrawViewer();

            return;
        }
    }
#ifdef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    CurrentFrameFunctorArgsPtr functorArgs( new CurrentFrameFunctorArgs(view,
                                                                                        frame,
                                                                                        stats,
                                                                                        _imp->viewer,
                                                                                        viewerHash,
                                                                                        _imp.get(),
                                                                                        canAbort,
                                                                                        rotoPaintNode,
                                                                                        curStroke,
                                                                                        isRotoNeatRender) );
#else
    CurrentFrameFunctorArgsPtr functorArgs = std::make_shared<CurrentFrameFunctorArgs>(view,
                                                                                                         frame,
                                                                                                         stats,
                                                                                                         _imp->viewer,
                                                                                                         viewerHash,
                                                                                                         _imp.get(),
                                                                                                         canAbort,
                                                                                                         rotoPaintNode,
                                                                                                         curStroke,
                                                                                                         isRotoNeatRender);
#endif
    functorArgs->args[0] = args[0];
    functorArgs->args[1] = args[1];

    if (appPTR->getCurrentSettings()->getNumberOfThreads() == -1) {
        RenderCurrentFrameFunctorRunnable task(functorArgs);
        task.run();
    } else {
        // Identify this render request with an age
        ViewerCurrentFrameRequestSchedulerStartArgsPtr request = std::make_shared<ViewerCurrentFrameRequestSchedulerStartArgs>();
        request->age = _imp->ageCounter;

        // If we reached the max amount of age, reset to 0... should never happen anyway
        if ( _imp->ageCounter >= std::numeric_limits<U64>::max() ) {
            _imp->ageCounter = 0;
        } else {
            ++_imp->ageCounter;
        }

        startTask(request);
        functorArgs->request = request;

        /*
         * Let at least 1 free thread in the thread-pool to allow the renderer to use the thread pool if we use the thread-pool
         */
        int maxThreads = _imp->threadPool->maxThreadCount();

        //When painting, limit the number of threads to 1 to be sure strokes are painted in the right order
        if (rotoUse1Thread || isTracking) {
            maxThreads = 1;
        }
        if ( (maxThreads == 1) || (_imp->threadPool->activeThreadCount() >= maxThreads - 1) ) {
            _imp->backupThread.startTask(functorArgs);
        } else {
            RenderCurrentFrameFunctorRunnable* task = new RenderCurrentFrameFunctorRunnable(functorArgs);
            _imp->appendRunnableTask(task);
            _imp->threadPool->start(task);
        }
    }
} // ViewerCurrentFrameRequestScheduler::renderCurrentFrame

ViewerCurrentFrameRequestRendererBackup::ViewerCurrentFrameRequestRendererBackup()
    : GenericSchedulerThread()
{
    setThreadName("ViewerCurrentFrameRequestRendererBackup");
}

ViewerCurrentFrameRequestRendererBackup::~ViewerCurrentFrameRequestRendererBackup()
{
}

GenericSchedulerThread::ThreadStateEnum
ViewerCurrentFrameRequestRendererBackup::threadLoopOnce(const GenericThreadStartArgsPtr& inArgs)
{
    CurrentFrameFunctorArgsPtr args = std::dynamic_pointer_cast<CurrentFrameFunctorArgs>(inArgs);

    assert(args);
    RenderCurrentFrameFunctorRunnable task(args);
    task.run();

    return eThreadStateActive;
}

NATRON_NAMESPACE_EXIT
