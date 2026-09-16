
GenericSchedulerThread::ThreadStateEnum
OutputSchedulerThread::threadLoopOnce(const GenericThreadStartArgsPtr &inArgs)
{
    OutputSchedulerThreadStartArgsPtr args = std::dynamic_pointer_cast<OutputSchedulerThreadStartArgs>(inArgs);

    assert(args);
    _imp->runArgs = args;

    ThreadStateEnum state = eThreadStateActive;
    int expectedTimeToRenderPreviousIteration = std::numeric_limits<int>::min();

    // This is the number of time this thread was woken up by a render thread because a frame was available for processing,
    // but this is not the frame this thread expects to render. If it reaches a certain amount, we detected a stall and abort.
    int nbIterationsWithoutProcessing = 0;
    int nbEmptyBufferStallSlices = 0;

    startRender();

    for (;; ) {
        ///When set to true, we don't sleep in the bufEmptyCondition but in the startCondition instead, indicating
        ///we finished a render
        bool renderFinished = false;

        {
            ///_imp->renderFinished might be set when in FFA scheduling policy
            QMutexLocker l(&_imp->renderFinishedMutex);
            if (_imp->renderFinished) {
                renderFinished = true;
            }
        }
        bool bufferEmpty;
        {
            QMutexLocker l(&_imp->bufMutex);
            bufferEmpty = _imp->buf.empty();
        }
        int expectedTimeToRender;


        while (!bufferEmpty) {
            state = resolveState();
            if ( (state == eThreadStateAborted) || (state == eThreadStateStopped) ) {
                ///Do not wait in the buf wait condition and go directly into the stopEngine()
                renderFinished = true;
                break;
            }


            {
                QMutexLocker k(&_imp->framesToRenderMutex);
                expectedTimeToRender = _imp->expectFrameToRender;
            }

#ifdef TRACE_SCHEDULER
            if ( (expectedTimeToRenderPreviousIteration == std::numeric_limits<int>::min()) || (expectedTimeToRenderPreviousIteration != expectedTimeToRender) ) {
                qDebug() << "Scheduler Thread: waiting for " << expectedTimeToRender << " to be rendered...";
            }
#endif
            if (expectedTimeToRenderPreviousIteration == expectedTimeToRender) {
                ++nbIterationsWithoutProcessing;
                if (nbIterationsWithoutProcessing >= NATRON_SCHEDULER_ABORT_AFTER_X_UNSUCCESSFUL_ITERATIONS) {
#ifdef TRACE_SCHEDULER
                    qDebug() << "Scheduler Thread: Detected stall after " << NATRON_SCHEDULER_ABORT_AFTER_X_UNSUCCESSFUL_ITERATIONS
                             << "unsuccessful iterations";
#endif
                    renderFinished = true;
                    break;
                }
            } else {
                nbIterationsWithoutProcessing = 0;
            }
            OutputSchedulerThreadExecMTArgsPtr framesToRender = std::make_shared<OutputSchedulerThreadExecMTArgs>();
            {
                QMutexLocker l(&_imp->bufMutex);
                _imp->getFromBufferAndErase(expectedTimeToRender, framesToRender->frames);
            }

            ///The expected frame is not yet ready, go to sleep again
            if ( framesToRender->frames.empty() ) {
                expectedTimeToRenderPreviousIteration = expectedTimeToRender;
                break;
            }

#ifdef TRACE_SCHEDULER
            qDebug() << "Scheduler Thread: received frame to process" << expectedTimeToRender;
#endif

            int nextFrameToRender = -1;
            RenderDirectionEnum newDirection = eRenderDirectionForward;

            if (!renderFinished) {
                ///////////
                /////Refresh frame range if needed (for viewers)


                int firstFrame, lastFrame;
                getFrameRangeToRender(firstFrame, lastFrame);


                RenderDirectionEnum timelineDirection;
                int frameStep;

                ///Refresh the firstframe/lastFrame as they might have changed on the timeline
                args->firstFrame = firstFrame;
                args->lastFrame = lastFrame;


                timelineDirection = args->processTimelineDirection;
                frameStep = args->frameStep;


                ///////////
                ///Determine if we finished rendering or if we should just increment/decrement the timeline
                ///or just loop/bounce
                PlaybackModeEnum pMode = _imp->engine->getPlaybackMode();
                if ( (firstFrame == lastFrame) && (pMode == ePlaybackModeOnce) ) {
                    renderFinished = true;
                    newDirection = eRenderDirectionForward;
                } else {
                    renderFinished = !OutputSchedulerThreadPrivate::getNextFrameInSequence(pMode, timelineDirection,
                                                                                           expectedTimeToRender, firstFrame,
                                                                                           lastFrame, frameStep, &nextFrameToRender, &newDirection);
                }

                if (newDirection != timelineDirection) {
                    args->processTimelineDirection = newDirection;
                }

                if (!renderFinished) {
                    {
                        QMutexLocker k(&_imp->framesToRenderMutex);
                        _imp->expectFrameToRender = nextFrameToRender;
                    }

#ifndef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
                    ///////////
                    /////If we were analysing the CPU activity, now set the appropriate number of threads to render.
                    int newNThreads, lastNThreads;
                    adjustNumberOfThreads(&newNThreads, &lastNThreads);

                    ///////////
                    /////Append render requests for the render threads

                    ///Simple heuristic to limit the size of the internal buffer.
                    ///If the buffer grows too much, we will keep shared ptr to images, hence keep them in RAM which
                    ///can lead to RAM issue for the end user.
                    ///We can end up in this situation for very simple graphs where the rendering of the output node (the writer or viewer)
                    ///is much slower than things upstream, hence the buffer grows quickly, and fills up the RAM.
                    bool bufferFull;
                    {
                        QMutexLocker k(&_imp->bufMutex);
                        int nbThreadsHardware = appPTR->getHardwareIdealThreadCount();
                        bufferFull = isBufferFull(_imp->buf.size(), nbThreadsHardware);
                    }
                    if (!bufferFull) {
                        pushFramesToRender(newNThreads);
                    }
#else
                    startTasksFromLastStartedFrame();
#endif

#endif
                }
            } // if (!renderFinished) {

            if (_imp->timer.playState == ePlayStateRunning) {
                _imp->timer.waitUntilNextFrameIsDue(); // timer synchronizing with the requested fps
            }


            state = resolveState();
            if ( (state == eThreadStateAborted) || (state == eThreadStateStopped) ) {
                // Do not wait in the buf wait condition and go directly into the stopEngine()
                renderFinished = true;
                break;
            }

            if (_imp->mode == eProcessFrameBySchedulerThread) {
                processFrame(framesToRender->frames);
            } else {
                requestExecutionOnMainThread(framesToRender);
            }

            expectedTimeToRenderPreviousIteration = expectedTimeToRender;

#ifdef TRACE_SCHEDULER
            QString pushDirectionStr = newDirection == eRenderDirectionForward ? QLatin1String("Forward") : QLatin1String("Backward");
            qDebug() << "Scheduler Thread: Frame " << expectedTimeToRender << " processed, setting expectedTimeToRender to " << nextFrameToRender
                     << ", new process direction is " << pushDirectionStr;
#endif
            if (!renderFinished) {
                ///Timeline might have changed if another thread moved the playhead
                int timelineCurrentTime = timelineGetTime();
                if (timelineCurrentTime != expectedTimeToRender) {
                    timelineGoTo(timelineCurrentTime);
                } else {
                    timelineGoTo(nextFrameToRender);
                }
            }

            ////////////
            /////At this point the frame has been processed by the output device

            assert( !framesToRender->frames.empty() );
            {
                const BufferedFrame& frame = framesToRender->frames.front();
                std::vector<ViewIdx> views(1);
                views[0] = frame.view;
                notifyFrameRendered(expectedTimeToRender, frame.view, views, frame.stats, eSchedulingPolicyOrdered);
            }

            ///////////
            /// End of the loop, refresh bufferEmpty
            {
                QMutexLocker l(&_imp->bufMutex);
                bufferEmpty = _imp->buf.empty();
            }
        } // while(!bufferEmpty)

        if (state == eThreadStateActive) {
            state = resolveState();
        }

        if ( (state == eThreadStateAborted) || (state == eThreadStateStopped) ) {
            renderFinished = true;
        }

        if (!renderFinished) {
            state = resolveState();
            if ( (state == eThreadStateAborted) || (state == eThreadStateStopped) ) {
                renderFinished = true;
            }
        }

        if (!renderFinished) {
            assert(state == eThreadStateActive);
            QMutexLocker bufLocker (&_imp->bufMutex);
            // Wait here for more frames to be rendered, we will be woken up once appendToBuffer(...) is called.
            // Timed wait: abort and silent stalls (#248) must not pin this thread at CPU-zero forever.
            if (_imp->buf.empty()) {
                if (!_imp->bufEmptyCondition.wait(bufLocker.mutex(), NATRON_SCHEDULER_WAIT_SLICE_MS)) {
                    ++nbEmptyBufferStallSlices;
                    if (nbEmptyBufferStallSlices >= NATRON_SCHEDULER_EMPTY_BUFFER_STALL_SLICES) {
#ifdef TRACE_SCHEDULER
                        qDebug() << "Scheduler Thread: empty-buffer stall after"
                                 << NATRON_SCHEDULER_EMPTY_BUFFER_STALL_SLICES << "timeouts";
#endif
                        renderFinished = true;
                    }
                } else {
                    nbEmptyBufferStallSlices = 0;
                }
            }
        } else {
            if ( !_imp->engine->isPlaybackAutoRestartEnabled() ) {
                //Move the timeline to the last rendered frame to keep it in sync with what is displayed
                timelineGoTo( getLastRenderedTime() );
            }
            break;
        }
    } // for (;;)

    stopRender();

    return state;
} // OutputSchedulerThread::threadLoopOnce

void
OutputSchedulerThread::onAbortRequested(bool /*keepOldestRender*/)
{
    ///We make sure the render-threads don't wait for the main-thread to process a frame
    ///This function (abortRendering) was probably called from a user event that was posted earlier in the
    ///event-loop, we just flag that the next event that will process the frame should NOT process it by
    ///resetting the processRunning flag
    // Flag directly all threads that they are aborted, this enables each thread to have a shorter code-path
    // when checking for abortion and will generally abort faster
    {
        QMutexLocker l(&_imp->renderThreadsMutex);
        for (RenderThreads::iterator it = _imp->renderThreads.begin(); it != _imp->renderThreads.end(); ++it) {
            AbortableThread* isAbortableThread = dynamic_cast<AbortableThread*>(it->thread);
            if (isAbortableThread) {
                bool userInteraction;
                AbortableRenderInfoPtr abortInfo;
                EffectInstancePtr treeRoot;
                isAbortableThread->getAbortInfo(&userInteraction, &abortInfo, &treeRoot);
                if (abortInfo) {
                    abortInfo->setAborted();
                }
            }
        }
    }

    ///If the scheduler is asleep waiting for the buffer to be filling up, we post a fake request
    ///that will not be processed anyway because the first thing it does is checking for abort
    {
        QMutexLocker l2(&_imp->bufMutex);
        _imp->bufEmptyCondition.wakeAll();
    }
    {
        QMutexLocker framesLocker(&_imp->framesToRenderMutex);
        _imp->framesToRenderNotEmptyCond.wakeAll();
    }
    {
        QMutexLocker l(&_imp->renderThreadsMutex);
        _imp->allRenderThreadsInactiveCond.wakeAll();
    }
}

void
OutputSchedulerThread::executeOnMainThread(const GenericThreadExecOnMainThreadArgsPtr& inArgs)
{
    OutputSchedulerThreadExecMTArgs* args = dynamic_cast<OutputSchedulerThreadExecMTArgs*>( inArgs.get() );

    assert(args);
    processFrame(args->frames);
}

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
void
OutputSchedulerThread::adjustNumberOfThreads(int* newNThreads,
                                             int *lastNThreads)
{
    ///////////
    /////If we were analysing the CPU activity, now set the appropriate number of threads to render.
    int optimalNThreads;

    ///How many parallel renders the user wants
    int userSettingParallelThreads = appPTR->getCurrentSettings()->getNumberOfParallelRenders();

    ///How many threads are running in the application
    int runningThreads = appPTR->getNRunningThreads() + QThreadPool::globalInstance()->activeThreadCount();

    ///How many current threads are used by THIS renderer
    int currentParallelRenders = getNRenderThreads();

    *lastNThreads = currentParallelRenders;

    if (userSettingParallelThreads == 0) {
        ///User wants it to be automatically computed, do a simple heuristic: launch as many parallel renders
        ///as there are cores
        optimalNThreads = appPTR->getHardwareIdealThreadCount();
    } else {
        optimalNThreads = userSettingParallelThreads;
    }
    optimalNThreads = std::max(1, optimalNThreads);


    if ( ( (runningThreads < optimalNThreads) && (currentParallelRenders < optimalNThreads) ) || (currentParallelRenders == 0) ) {
        ////////
        ///Launch 1 thread
        QMutexLocker l(&_imp->renderThreadsMutex);

        _imp->appendRunnable( createRunnable() );
        *newNThreads = currentParallelRenders +  1;
    } else if ( (runningThreads > optimalNThreads) && (currentParallelRenders > optimalNThreads) ) {
        ////////
        ///Stop 1 thread
        stopRenderThreads(1);
        *newNThreads = currentParallelRenders - 1;
    } else {
        /////////
        ///Keep the current count
        *newNThreads = std::max(1, currentParallelRenders);
    }
}

#endif // ifndef NATRON_PLAYBACK_USES_THREAD_POOL

void
OutputSchedulerThread::notifyFrameRendered(int frame,
                                           ViewIdx viewIndex,
                                           const std::vector<ViewIdx>& viewsToRender,
                                           const RenderStatsPtr& stats,
                                           SchedulingPolicyEnum policy)
{
    assert(viewsToRender.size() > 0);

    bool isLastView = viewIndex == viewsToRender[viewsToRender.size() - 1] || viewIndex == -1;

    // Report render stats if desired
    OutputEffectInstancePtr effect = _imp->outputEffect.lock();
    if (stats) {
        double timeSpentForFrame;
        std::map<NodePtr, NodeRenderStats > statResults = stats->getStats(&timeSpentForFrame);
        if ( !statResults.empty() ) {
            effect->reportStats(frame, viewIndex, timeSpentForFrame, statResults);
        }
    }


    bool isBackground = appPTR->isBackground();
    OutputSchedulerThreadStartArgsPtr runArgs = _imp->runArgs.lock();
    assert(runArgs);

    // If FFA all parallel renders call render on the Writer in their own thread,
    // otherwise the OutputSchedulerThread thread calls the render of the Writer.
    U64 nbTotalFrames;
    U64 nbFramesRendered;
    //bool renderingIsFinished = false;
    if (policy == eSchedulingPolicyFFA) {
        QMutexLocker l(&_imp->renderFinishedMutex);
        if (isLastView) {
            ++_imp->nFramesRendered;
        }
        nbTotalFrames = std::ceil( (double)(runArgs->lastFrame - runArgs->firstFrame + 1) / runArgs->frameStep );
        nbFramesRendered = _imp->nFramesRendered;


        if (_imp->nFramesRendered == nbTotalFrames) {
            _imp->renderFinished = true;
            l.unlock();

            // Notify the scheduler rendering is finished by append a fake frame to the buffer
            {
                QMutexLocker bufLocker (&_imp->bufMutex);
                _imp->appendBufferedFrame( 0, viewIndex, RenderStatsPtr(), BufferableObjectPtr() );
                _imp->bufEmptyCondition.wakeOne();
            }
        } else {
            l.unlock();

#ifndef NATRON_SCHEDULER_SPAWN_THREADS_WITH_TIMER

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
            ///////////
            /////Now set the appropriate number of threads to render.
            int newNThreads, nbCurParallelRenders;
            adjustNumberOfThreads(&newNThreads, &nbCurParallelRenders);
#else
            startTasksFromLastStartedFrame();
#endif

#endif
        }
        //renderingIsFinished = _imp->renderFinished;
    } else {
        nbTotalFrames = std::floor( (double)(runArgs->lastFrame - runArgs->firstFrame + 1) / runArgs->frameStep );
        if (runArgs->processTimelineDirection == eRenderDirectionForward) {
            nbFramesRendered = (frame - runArgs->firstFrame) / runArgs->frameStep;
        } else {
            nbFramesRendered = (runArgs->lastFrame - frame) / runArgs->frameStep;
        }
    } // if (policy == eSchedulingPolicyFFA) {

    double fractionDone = 0.;
    assert(nbTotalFrames > 0);
    if (nbTotalFrames != 0) {
        QMutexLocker k(&_imp->renderFinishedMutex);
        fractionDone = (double)_imp->nFramesRendered / nbTotalFrames;
    }
    assert(_imp->renderTimer);
    double timeSpentSinceStartSec = _imp->renderTimer->getTimeSinceCreation();
    double estimatedFps = (double)nbFramesRendered / timeSpentSinceStartSec;
    // total estimated time is: timeSpentSinceStartSec / fractionDone
    // remaining time is thus:
    double timeRemaining = (nbTotalFrames <= 0 || _imp->nFramesRendered <= 0) ? -1. : timeSpentSinceStartSec / fractionDone - timeSpentSinceStartSec;

    // If running in background, notify to the pipe that we rendered a frame
    if (isBackground) {
        QString longMessage;
        QTextStream ts(&longMessage);
        QString frameStr = QString::number(frame);
        QString fpsStr = QString::number(estimatedFps, 'f', 1);
        QString percentageStr = QString::number(fractionDone * 100, 'f', 1);
        QString timeRemainingStr = timeRemaining < 0 ? tr("unknown") : Timer::printAsTime(timeRemaining, true);

        longMessage = (tr("%1 ==> Frame: %2, Progress: %3%, %4 Fps, Time Remaining: %5")
                       .arg(QString::fromUtf8(effect->getScriptName_mt_safe().c_str()))
                       .arg(frameStr)
                       .arg(percentageStr)
                       .arg(fpsStr)
                       .arg(timeRemainingStr));

        QString shortMessage = QString::fromUtf8(kFrameRenderedStringShort) + frameStr + QString::fromUtf8(kProgressChangedStringShort) + QString::number(fractionDone);
        {
            QMutexLocker l(&_imp->bufferedOutputMutex);
            std::string toPrint = longMessage.toStdString();
            if ( (_imp->lastBufferedOutputSize != 0) && ( _imp->lastBufferedOutputSize > longMessage.size() ) ) {
                int nSpacesToAppend = _imp->lastBufferedOutputSize - longMessage.size();
                toPrint.append(nSpacesToAppend, ' ');
            }
            //std::cout << '\r';
            std::cout << toPrint;
            std::cout << std::endl;

            /*std::cout.flush();
               if (renderingIsFinished) {
                std::cout << std::endl;
               }*/
            _imp->lastBufferedOutputSize = longMessage.size();
        }

        appPTR->writeToOutputPipe(longMessage, shortMessage, false);
    }

    // Notify we rendered a frame
    if (isLastView) {
        _imp->engine->s_frameRendered(frame, fractionDone);
    }

    // Call Python after frame ranedered callback
    if ( isLastView && effect->isWriter() ) {
        std::string cb = effect->getNode()->getAfterFrameRenderCallback();
        if ( !cb.empty() ) {
            std::vector<std::string> args;
            std::string error;
            try {
                NATRON_PYTHON_NAMESPACE::getFunctionArguments(cb, &error, &args);
            } catch (const std::exception& e) {
                effect->getApp()->appendToScriptEditor( std::string("Failed to get signature of onFrameRendered callback: ")
                                                        + e.what() );

                return;
            }

            if ( !error.empty() ) {
                effect->getApp()->appendToScriptEditor("Failed to get signature of onFrameRendered callback: " + error);

                return;
            }

            std::string signatureError;
            signatureError.append("The after frame render callback supports the following signature(s):\n");
            signatureError.append("- callback(frame, thisNode, app)");
            if ( (args.size() != 3) || (args[0] != "frame") || (args[1] != "thisNode") || (args[2] != "app") ) {
                effect->getApp()->appendToScriptEditor("Wrong signature of onFrameRendered callback: " + signatureError);

                return;
            }

            std::stringstream ss;
            std::string appStr = effect->getApp()->getAppIDString();
            std::string outputNodeName = appStr + "." + effect->getNode()->getFullyQualifiedName();
            ss << cb << "(" << frame << ", ";
            ss << outputNodeName << ", " << appStr << ")";
            std::string script = ss.str();
            try {
                runCallbackWithVariables( QString::fromUtf8( script.c_str() ) );
            } catch (const std::exception& e) {
                notifyRenderFailure( e.what() );

                return;
            }
        }
    }
} // OutputSchedulerThread::notifyFrameRendered

void
OutputSchedulerThread::appendToBuffer_internal(double time,
                                               ViewIdx view,
                                               const RenderStatsPtr& stats,
                                               const BufferableObjectPtr& frame,
                                               bool wakeThread)
{
    if ( QThread::currentThread() == qApp->thread() ) {
        ///Single-threaded , call directly the function
        if (frame) {
            BufferedFrame b;
            b.time = time;
            b.view = view;
            b.frame = frame;
            BufferedFrames frames;
            frames.push_back(b);
            processFrame(frames);
        }
    } else {
        ///Called by the scheduler thread when an image is rendered

        QMutexLocker l(&_imp->bufMutex);
        _imp->appendBufferedFrame(time, view, stats, frame);
        if (wakeThread) {
            ///Wake up the scheduler thread that an image is available if it is asleep so it can process it.
            _imp->bufEmptyCondition.wakeOne();
        }
    }
}

void
OutputSchedulerThread::appendToBuffer(double time,
                                      ViewIdx view,
                                      const RenderStatsPtr& stats,
                                      const BufferableObjectPtr& image)
{
    appendToBuffer_internal(time, view, stats, image, true);
}

void
OutputSchedulerThread::appendToBuffer(double time,
                                      ViewIdx view,
                                      const RenderStatsPtr& stats,
                                      const BufferableObjectPtrList& frames)
{
    if ( frames.empty() ) {
        return;
    }
    BufferableObjectPtrList::const_iterator next = frames.begin();
    if ( next != frames.end() ) {
        ++next;
    }
    for (BufferableObjectPtrList::const_iterator it = frames.begin(); it != frames.end(); ++it) {
        if ( next != frames.end() ) {
            appendToBuffer_internal(time, view, stats, *it, false);
            ++next;
        } else {
            appendToBuffer_internal(time, view, stats, *it, true);
        }
    }
}

void
OutputSchedulerThread::setDesiredFPS(double d)
{
    _imp->timer.setDesiredFrameRate(d);
}

double
OutputSchedulerThread::getDesiredFPS() const
{
    return _imp->timer.getDesiredFrameRate();
}

void
OutputSchedulerThread::getLastRunArgs(RenderDirectionEnum* direction,
                                      std::vector<ViewIdx>* viewsToRender) const
{
    QMutexLocker k(&_imp->lastRunArgsMutex);

    *direction = _imp->lastPlaybackRenderDirection;
    *viewsToRender = _imp->lastPlaybackViewsToRender;
}

void
OutputSchedulerThread::renderFrameRange(bool isBlocking,
                                        bool enableRenderStats,
                                        int firstFrame,
                                        int lastFrame,
                                        int frameStep,
                                        const std::vector<ViewIdx>& viewsToRender,
                                        RenderDirectionEnum direction)
{
    {
        QMutexLocker k(&_imp->lastRunArgsMutex);
        _imp->lastPlaybackRenderDirection = direction;
        _imp->lastPlaybackViewsToRender = viewsToRender;
    }
    if (direction == eRenderDirectionForward) {
        timelineGoTo(firstFrame);
    } else {
        timelineGoTo(lastFrame);
    }

    OutputSchedulerThreadStartArgsPtr args = std::make_shared<OutputSchedulerThreadStartArgs>(isBlocking, enableRenderStats, firstFrame, lastFrame, frameStep, viewsToRender, direction);

    {
        QMutexLocker k(&_imp->renderFinishedMutex);
        _imp->nFramesRendered = 0;
        _imp->renderFinished = false;
    }

    startTask(args);
}

void
OutputSchedulerThread::renderFromCurrentFrame(bool enableRenderStats,
                                              const std::vector<ViewIdx>& viewsToRender,
                                              RenderDirectionEnum timelineDirection)
{
    {
        QMutexLocker k(&_imp->lastRunArgsMutex);
        _imp->lastPlaybackRenderDirection = timelineDirection;
        _imp->lastPlaybackViewsToRender = viewsToRender;
    }
    int firstFrame, lastFrame;

    getFrameRangeToRender(firstFrame, lastFrame);

    ///Make sure current frame is in the frame range
    int currentTime = timelineGetTime();
    OutputSchedulerThreadPrivate::getNearestInSequence(timelineDirection, currentTime, firstFrame, lastFrame, &currentTime);
    OutputSchedulerThreadStartArgsPtr args = std::make_shared<OutputSchedulerThreadStartArgs>(false, enableRenderStats, firstFrame, lastFrame, 1, viewsToRender, timelineDirection);
    startTask(args);
}

void
OutputSchedulerThread::notifyRenderFailure(const std::string& errorMessage)
{
    ///Abort all ongoing rendering
    OutputSchedulerThreadStartArgsPtr args = _imp->runArgs.lock();

    assert(args);


    ///Handle failure: for viewers we make it black and don't display the error message which is irrelevant
    handleRenderFailure(errorMessage);

    _imp->engine->abortRenderingNoRestart();

    if (args->isBlocking) {
        waitForAbortToComplete_enforce_blocking();
    }
}

OutputSchedulerThreadStartArgsPtr
OutputSchedulerThread::getCurrentRunArgs() const
{
    return _imp->runArgs.lock();
}

int
OutputSchedulerThread::getNRenderThreads() const
{
    QMutexLocker l(&_imp->renderThreadsMutex);

    return (int)_imp->renderThreads.size();
}

int
OutputSchedulerThread::getNActiveRenderThreads() const
{
    QMutexLocker l(&_imp->renderThreadsMutex);

    return _imp->getNActiveRenderThreads();
}

void
OutputSchedulerThread::stopRenderThreads(int nThreadsToStop)
{
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    {
        ///First flag the number of threads to stop
        QMutexLocker l(&_imp->renderThreadsMutex);
        int i = 0;
        for (RenderThreads::iterator it = _imp->renderThreads.begin();
             it != _imp->renderThreads.end() && (i < nThreadsToStop || nThreadsToStop == 0); ++it) {
            if ( !it->thread->mustQuit() ) {
                it->thread->scheduleForRemoval();
                ++i;
            }
        }

        ///Clean-up remaining zombie threads that are no longer useful
        _imp->removeAllQuitRenderThreads();
    }


    ///Wake-up all threads to make sure that they are notified that they must quit
    {
        QMutexLocker framesLocker(&_imp->framesToRenderMutex);
        _imp->framesToRenderNotEmptyCond.wakeAll();
    }

#else
    Q_UNUSED(nThreadsToStop);
    QMutexLocker l(&_imp->renderThreadsMutex);
    _imp->waitForRenderThreadsToBeDone();
#endif
}

RenderEngine*
OutputSchedulerThread::getEngine() const
{
    return _imp->engine;
}

void
OutputSchedulerThread::runCallbackWithVariables(const QString& callback)
{
    if ( !callback.isEmpty() ) {
        OutputEffectInstancePtr effect = _imp->outputEffect.lock();
        QString script = callback;
        std::string appID = effect->getApp()->getAppIDString();
        std::string nodeName = effect->getNode()->getFullyQualifiedName();
        std::string nodeFullName = appID + "." + nodeName;
        script.append( QString::fromUtf8( nodeFullName.c_str() ) );
        script.append( QLatin1Char(',') );
        script.append( QString::fromUtf8( appID.c_str() ) );
        script.append( QString::fromUtf8(")\n") );

        std::string err, output;
        if ( !NATRON_PYTHON_NAMESPACE::interpretPythonScript(callback.toStdString(), &err, &output) ) {
            effect->getApp()->appendToScriptEditor("Failed to run callback: " + err);
            throw std::runtime_error(err);
        } else if ( !output.empty() ) {
            effect->getApp()->appendToScriptEditor(output);
        }
    }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//////////////////////// RenderThreadTask ////////////

struct RenderThreadTaskPrivate
{
    OutputSchedulerThread* scheduler;
    OutputEffectInstanceWPtr output;

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    QMutex mustQuitMutex;
    bool mustQuit;
    bool hasQuit;
    QMutex runningMutex;
    bool running;
#else
    int time;
    bool useRenderStats;
    std::vector<int> viewsToRender;
#endif


    RenderThreadTaskPrivate(const OutputEffectInstancePtr& output,
                            OutputSchedulerThread* scheduler
                            #ifdef NATRON_PLAYBACK_USES_THREAD_POOL
                            ,
                            const int time,
                            const bool useRenderStats,
                            const std::vector<int>& viewsToRender
                            #endif
                            )
        : scheduler(scheduler)
        , output(output)
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
        , mustQuitMutex()
        , mustQuit(false)
        , hasQuit(false)
        , runningMutex()
        , running(false)
#else
        , time(time)
        , useRenderStats(useRenderStats)
        , viewsToRender(viewsToRender)
#endif
    {
    }
};


#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
RenderThreadTask::RenderThreadTask(const OutputEffectInstancePtr& output,
                                   OutputSchedulerThread* scheduler)
    : QThread()
    , AbortableThread(this)
    , _imp( new RenderThreadTaskPrivate(output, scheduler) )
{
    setThreadName("Parallel render thread");
}

#else
RenderThreadTask::RenderThreadTask(const OutputEffectInstancePtr& output,
                                   OutputSchedulerThread* scheduler,
                                   const int time,
                                   const bool useRenderStats,
                                   const std::vector<int>& viewsToRender)
    : QRunnable()
    , _imp( new RenderThreadTaskPrivate(output, scheduler, time, useRenderStats, viewsToRender) )
{
}

#endif
RenderThreadTask::~RenderThreadTask()
{
}

