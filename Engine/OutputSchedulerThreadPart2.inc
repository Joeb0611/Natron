void
RenderThreadTask::run()
{
#ifdef DEBUG
    boost_adaptbx::floating_point::exception_trapping trap(boost_adaptbx::floating_point::exception_trapping::division_by_zero |
                                                           boost_adaptbx::floating_point::exception_trapping::invalid |
                                                           boost_adaptbx::floating_point::exception_trapping::overflow);
#endif
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    notifyIsRunning(true);

    for (;; ) {
        bool enableRenderStats;
        std::vector<ViewIdx> viewsToRender;
        int time = _imp->scheduler->pickFrameToRender(this, &enableRenderStats, &viewsToRender);

        if ( mustQuit() ) {
            break;
        }

#ifdef TRACE_SCHEDULER
        qDebug() << "Parallel Render Thread: Picking frame to render: " << time;
#endif
        renderFrame(time, viewsToRender, enableRenderStats);

        appPTR->getAppTLS()->cleanupTLSForThread();

        if ( mustQuit() ) {
            break;
        }
    }

    {
        QMutexLocker l(&_imp->mustQuitMutex);
        _imp->hasQuit = true;
    }
    notifyIsRunning(false);
    _imp->scheduler->notifyThreadAboutToQuit(this);
#else // NATRON_PLAYBACK_USES_THREAD_POOL
    renderFrame(_imp->time, _imp->viewsToRender, _imp->useRenderStats);
    _imp->scheduler->notifyThreadAboutToQuit(this);
#endif
}

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
bool
RenderThreadTask::hasQuit() const
{
    QMutexLocker l(&_imp->mustQuitMutex);

    return _imp->hasQuit;
}

void
RenderThreadTask::scheduleForRemoval()
{
    QMutexLocker l(&_imp->mustQuitMutex);

    assert(!_imp->mustQuit);
    _imp->mustQuit = true;
}

bool
RenderThreadTask::mustQuit() const
{
    QMutexLocker l(&_imp->mustQuitMutex);

    return _imp->mustQuit;
}

void
RenderThreadTask::notifyIsRunning(bool running)
{
    {
        QMutexLocker l(&_imp->runningMutex);
        if (_imp->running == running) {
            return;
        }
        _imp->running = running;
    }

    appPTR->fetchAndAddNRunningThreads(running ? 1 : -1);
}

#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//////////////////////// DefaultScheduler ////////////


DefaultScheduler::DefaultScheduler(RenderEngine* engine,
                                   const OutputEffectInstancePtr& effect)
    : OutputSchedulerThread(engine, effect, eProcessFrameBySchedulerThread)
    , _effect(effect)
    , _currentTimeMutex()
    , _currentTime(0)
{
    engine->setPlaybackMode(ePlaybackModeOnce);
}

DefaultScheduler::~DefaultScheduler()
{
}

class DefaultRenderFrameRunnable
    : public RenderThreadTask
{
public:


#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    DefaultRenderFrameRunnable(const OutputEffectInstancePtr& writer,
                               OutputSchedulerThread* scheduler)
        : RenderThreadTask(writer, scheduler)
    {
    }

#else
    DefaultRenderFrameRunnable(const OutputEffectInstancePtr& writer,
                               OutputSchedulerThread* scheduler,
                               const int time,
                               const bool useRenderStats,
                               const std::vector<int>& viewsToRender)
        : RenderThreadTask(writer, scheduler, time, useRenderStats, viewsToRender)
    {
    }

#endif


    virtual ~DefaultRenderFrameRunnable()
    {
    }

private:


    virtual void renderFrame(int time,
                             const std::vector<ViewIdx>& viewsToRender,
                             bool enableRenderStats)
    {
        OutputEffectInstancePtr output = _imp->output.lock();

        if (!output) {
            _imp->scheduler->notifyRenderFailure("");

            return;
        }

        AbortableThread* isAbortableThread = dynamic_cast<AbortableThread*>( QThread::currentThread() );

        ///Even if enableRenderStats is false, we at least profile the time spent rendering the frame when rendering with a Write node.
        ///Though we don't enable render stats for sequential renders (e.g: WriteFFMPEG) since this is 1 file.
        RenderStatsPtr stats = std::make_shared<RenderStats>(enableRenderStats);
        NodePtr outputNode = output->getNode();
        std::string cb = outputNode->getBeforeFrameRenderCallback();
        if ( !cb.empty() ) {
            std::vector<std::string> args;
            std::string error;
            try {
                NATRON_PYTHON_NAMESPACE::getFunctionArguments(cb, &error, &args);
            } catch (const std::exception& e) {
                output->getApp()->appendToScriptEditor( std::string("Failed to get signature of beforeFrameRendered callback: ")
                                                        + e.what() );

                return;
            }

            if ( !error.empty() ) {
                output->getApp()->appendToScriptEditor("Failed to get signature of beforeFrameRendered callback: " + error);

                return;
            }

            std::string signatureError;
            signatureError.append("The before frame render callback supports the following signature(s):\n");
            signatureError.append("- callback(frame, thisNode, app)");
            if ( (args.size() != 3) || (args[0] != "frame") || (args[1] != "thisNode") || (args[2] != "app") ) {
                output->getApp()->appendToScriptEditor("Wrong signature of beforeFrameRendered callback: " + signatureError);

                return;
            }

            std::stringstream ss;
            std::string appStr = outputNode->getApp()->getAppIDString();
            std::string outputNodeName = appStr + "." + outputNode->getFullyQualifiedName();
            ss << cb << "(" << time << ", " << outputNodeName << ", " << appStr << ")";
            std::string script = ss.str();
            try {
                _imp->scheduler->runCallbackWithVariables( QString::fromUtf8( script.c_str() ) );
            } catch (const std::exception &e) {
                _imp->scheduler->notifyRenderFailure( e.what() );

                return;
            }
        }

        try {
            ////Writers always render at scale 1.
            const int mipmapLevel = 0;
            const RenderScale scale = RenderScale::fromMipmapLevel(mipmapLevel);
            RectD rod;
            bool isProjectFormat;

            // Do not catch exceptions: if an exception occurs here it is probably fatal, since
            // it comes from Natron itself. All exceptions from plugins are already caught
            // by the HostSupport library.
            EffectInstancePtr activeInputToRender;
            //if (renderDirectly) {
            activeInputToRender = output;
            WriteNode* isWriteNode = dynamic_cast<WriteNode*>( output.get() );
            if (isWriteNode) {
                NodePtr embeddedWriter = isWriteNode->getEmbeddedWriter();
                if (embeddedWriter) {
                    activeInputToRender = embeddedWriter->getEffectInstance();
                }
            }
            assert(activeInputToRender);
            NodePtr activeInputNode = activeInputToRender->getNode();
            U64 activeInputToRenderHash = isWriteNode ? isWriteNode->getHash() : activeInputToRender->getHash();
            const double par = activeInputToRender->getAspectRatio(-1);
            const bool isRenderDueToRenderInteraction = false;
            const bool isSequentialRender = true;

            for (std::size_t view = 0; view < viewsToRender.size(); ++view) {
                StatusEnum stat = activeInputToRender->getRegionOfDefinition_public(activeInputToRenderHash, time, scale, viewsToRender[view], &rod, &isProjectFormat);
                if (stat == eStatusFailed) {
                    _imp->scheduler->notifyRenderFailure("Error caught while rendering");

                    return;
                }
                std::list<ImagePlaneDesc> components;
                ImageBitDepthEnum imageDepth;

                //Use needed components to figure out what we need to render
                EffectInstance::ComponentsNeededMap neededComps;
                std::list<ImagePlaneDesc> passThroughPlanes;
                bool processAll;
                double ptTime;
                int ptView;
                std::bitset<4> processChannels;
                int ptInput;
                activeInputToRender->getComponentsNeededAndProduced_public(activeInputToRenderHash,time, viewsToRender[view], &neededComps, &passThroughPlanes, &processAll, &ptTime, &ptView, &processChannels, &ptInput);


                //Retrieve bitdepth only
                imageDepth = activeInputToRender->getBitDepth(-1);
                components.clear();

                EffectInstance::ComponentsNeededMap::iterator foundOutput = neededComps.find(-1);
                if ( foundOutput != neededComps.end() ) {
                    for (std::list<ImagePlaneDesc>::const_iterator it2 = foundOutput->second.begin(); it2 != foundOutput->second.end(); ++it2) {
                        components.push_back(*it2);
                    }
                }
                const RectI renderWindow = rod.toPixelEnclosing(scale, par);

                AbortableRenderInfoPtr abortInfo = AbortableRenderInfo::create(true, 0);
                if (isAbortableThread) {
                    isAbortableThread->setAbortInfo(isRenderDueToRenderInteraction, abortInfo, activeInputToRender);
                }

                ParallelRenderArgsSetter frameRenderArgs(time,
                                                         viewsToRender[view],
                                                         isRenderDueToRenderInteraction,  // is this render due to user interaction ?
                                                         isSequentialRender,
                                                         abortInfo, //abortInfo
                                                         activeInputNode, // viewer requester
                                                         0, //texture index
                                                         output->getApp()->getTimeLine().get(),
                                                         NodePtr(),
                                                         false,
                                                         false,
                                                         stats);

                {
                    FrameRequestMap request;
                    stat = EffectInstance::computeRequestPass(time, viewsToRender[view], mipmapLevel, rod, activeInputNode, request);
                    if (stat == eStatusFailed) {
                        _imp->scheduler->notifyRenderFailure("Error caught while rendering");

                        return;
                    }
                    frameRenderArgs.updateNodesRequest(request);
                }
                RenderingFlagSetter flagIsRendering( activeInputToRender->getNode() );
                std::map<ImagePlaneDesc, ImagePtr> planes;
                std::unique_ptr<EffectInstance::RenderRoIArgs> renderArgs( new EffectInstance::RenderRoIArgs(time, //< the time at which to render
                                                                                                               scale, //< the scale at which to render
                                                                                                               mipmapLevel, //< the mipmap level (redundant with the scale)
                                                                                                               viewsToRender[view], //< the view to render
                                                                                                               false,
                                                                                                               renderWindow, //< the region of interest (in pixel coordinates)
                                                                                                               rod, // < any precomputed rod ? in canonical coordinates
                                                                                                               components,
                                                                                                               imageDepth,
                                                                                                               false,
                                                                                                               activeInputToRender.get(),
                                                                                                               eStorageModeRAM,
                                                                                                               time) );
                EffectInstance::RenderRoIRetCode retCode;
                retCode = activeInputToRender->renderRoI(*renderArgs, &planes);
                if (retCode != EffectInstance::eRenderRoIRetCodeOk) {
                    if (retCode == EffectInstance::eRenderRoIRetCodeAborted) {
                        _imp->scheduler->notifyRenderFailure("Render aborted");
                    } else {
                        _imp->scheduler->notifyRenderFailure("Error caught while rendering");
                    }

                    return;
                }

                ///If we need sequential rendering, pass the image to the output scheduler that will ensure the sequential ordering
                /*if (!renderDirectly) {
                    for (std::map<ImagePlaneDesc,ImagePtr>::iterator it = planes.begin(); it != planes.end(); ++it) {
                        _imp->scheduler->appendToBuffer(time, viewsToRender[view], stats, std::dynamic_pointer_cast<BufferableObject>(it->second));
                    }
                   } else {*/
                _imp->scheduler->notifyFrameRendered(time, viewsToRender[view], viewsToRender, stats, eSchedulingPolicyFFA);
                //}
            }
        } catch (const std::exception& e) {
            _imp->scheduler->notifyRenderFailure( std::string("Error while rendering: ") + e.what() );
        }
    } // renderFrame
};

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
RenderThreadTask*
DefaultScheduler::createRunnable()
{
    return new DefaultRenderFrameRunnable(_effect.lock(), this);
}

#else
RenderThreadTask*
DefaultScheduler::createRunnable(int frame,
                                 bool useRenderStarts,
                                 const std::vector<int>& viewsToRender)
{
    return new DefaultRenderFrameRunnable(_effect.lock(), this, frame, useRenderStarts, viewsToRender);
}

#endif


/**
 * @brief Called whenever there are images available to process in the buffer.
 * Once processed, the frame will be removed from the buffer.
 *
 * According to the ProcessFrameModeEnum given to the scheduler this function will be called either by the scheduler thread (this)
 * or by the application's main-thread (typically to do OpenGL rendering).
 **/
void
DefaultScheduler::processFrame(const BufferedFrames& frames)
{
    assert( !frames.empty() );
    //Only consider the first frame, we shouldn't have multiple view here anyway.
    const BufferedFrame& frame = frames.front();

    OutputEffectInstancePtr effect = _effect.lock();
    U64 hash = effect->getHash();
    bool isProjectFormat;
    std::list<ImagePlaneDesc> components;

    {
        ImagePlaneDesc metadataPlane, metadataPairedPlane;
        effect->getMetadataComponents(-1, &metadataPlane, &metadataPairedPlane);
        if (metadataPlane.getNumComponents() > 0) {
            components.push_back(metadataPlane);
        }
    }

    ImageBitDepthEnum imageDepth = effect->getBitDepth(-1);
    const double par = effect->getAspectRatio(-1);
    const bool isRenderDueToRenderInteraction = false;
    const bool isSequentialRender = true;

    for (BufferedFrames::const_iterator it = frames.begin(); it != frames.end(); ++it) {
        AbortableRenderInfoPtr abortInfo = AbortableRenderInfo::create(true, 0);

        setAbortInfo(isRenderDueToRenderInteraction, abortInfo, effect);

        ParallelRenderArgsSetter frameRenderArgs(it->time,
                                                 it->view,
                                                 isRenderDueToRenderInteraction,  // is this render due to user interaction ?
                                                 isSequentialRender, // is this sequential ?
                                                 abortInfo, //abortInfo
                                                 effect->getNode(), //tree root
                                                 0, //texture index
                                                 effect->getApp()->getTimeLine().get(),
                                                 NodePtr(),
                                                 false,
                                                 false,
                                                 it->stats);
        RectD rod;
        ///Writers render to scale 1 always
        const RenderScale scale;
        ignore_result( effect->getRegionOfDefinition_public(hash, it->time, scale, it->view, &rod, &isProjectFormat) );
        const RectI roi = rod.toPixelEnclosing(0, par);


        RenderingFlagSetter flagIsRendering( effect->getNode() );
        ImagePtr inputImage = std::dynamic_pointer_cast<Image>(it->frame);
        assert(inputImage);

        EffectInstance::InputImagesMap inputImages;
        inputImages[0].push_back(inputImage);
        std::unique_ptr<EffectInstance::RenderRoIArgs> renderArgs( new EffectInstance::RenderRoIArgs(frame.time,
                                                                                                       scale, 0,
                                                                                                       it->view,
                                                                                                       true, // for writers, always by-pass cache for the write node only @see renderRoiInternal
                                                                                                       roi,
                                                                                                       rod,
                                                                                                       components,
                                                                                                       imageDepth,
                                                                                                       false,
                                                                                                       effect.get(),
                                                                                                       eStorageModeRAM,
                                                                                                       frame.time,
                                                                                                       inputImages) );
        try {
            std::map<ImagePlaneDesc, ImagePtr> planes;
            EffectInstance::RenderRoIRetCode retCode;
            retCode = effect->renderRoI(*renderArgs, &planes);
            if (retCode != EffectInstance::eRenderRoIRetCodeOk) {
                notifyRenderFailure("");
            }
        } catch (const std::exception& e) {
            notifyRenderFailure( e.what() );
        }
    }
} // DefaultScheduler::processFrame

void
DefaultScheduler::timelineStepOne(RenderDirectionEnum direction)
{
    QMutexLocker k(&_currentTimeMutex);

    if (direction == eRenderDirectionForward) {
        ++_currentTime;
    } else {
        --_currentTime;
    }
}

void
DefaultScheduler::timelineGoTo(int time)
{
    QMutexLocker k(&_currentTimeMutex);

    _currentTime =  time;
}

int
DefaultScheduler::timelineGetTime() const
{
    QMutexLocker k(&_currentTimeMutex);

    return _currentTime;
}

void
DefaultScheduler::getFrameRangeToRender(int& first,
                                        int& last) const
{
    OutputSchedulerThreadStartArgsPtr args = getCurrentRunArgs();

    first = args->firstFrame;
    last = args->lastFrame;
}

void
DefaultScheduler::handleRenderFailure(const std::string& errorMessage)
{
    if ( appPTR->isBackground() ) {
        std::cerr << errorMessage << std::endl;
    }
}

SchedulingPolicyEnum
DefaultScheduler::getSchedulingPolicy() const
{
    /*SequentialPreferenceEnum sequentiallity = _effect.lock()->getSequentialPreference();
       if (sequentiallity == eSequentialPreferenceNotSequential) {*/
    return eSchedulingPolicyFFA;
    /*} else {
        return eSchedulingPolicyOrdered;
       }*/
}

void
DefaultScheduler::aboutToStartRender()
{
    OutputSchedulerThreadStartArgsPtr args = getCurrentRunArgs();
    OutputEffectInstancePtr effect = _effect.lock();

    {
        QMutexLocker k(&_currentTimeMutex);
        if (args->pushTimelineDirection == eRenderDirectionForward) {
            _currentTime  = args->firstFrame;
        } else {
            _currentTime  = args->lastFrame;
        }
    }
    bool isBackGround = appPTR->isBackground();

    if (!isBackGround) {
        effect->setKnobsFrozen(true);
    } else {
        QString longText = QString::fromUtf8( effect->getScriptName_mt_safe().c_str() ) + tr(" ==> Rendering started");
        appPTR->writeToOutputPipe(longText, QString::fromUtf8(kRenderingStartedShort), true);
    }

    // Activate the internal writer node for a write node
    WriteNode* isWriter = dynamic_cast<WriteNode*>( effect.get() );
    if (isWriter) {
        isWriter->onSequenceRenderStarted();
    }

    std::string cb = effect->getNode()->getBeforeRenderCallback();
    if ( !cb.empty() ) {
        std::vector<std::string> args;
        std::string error;
        try {
            NATRON_PYTHON_NAMESPACE::getFunctionArguments(cb, &error, &args);
        } catch (const std::exception& e) {
            effect->getApp()->appendToScriptEditor( std::string("Failed to get signature of beforeRender callback: ")
                                                    + e.what() );

            return;
        }

        if ( !error.empty() ) {
            effect->getApp()->appendToScriptEditor("Failed to get signature of beforeRender callback: " + error);

            return;
        }

        std::string signatureError;
        signatureError.append("The beforeRender callback supports the following signature(s):\n");
        signatureError.append("- callback(thisNode, app)");
        if ( (args.size() != 2) || (args[0] != "thisNode") || (args[1] != "app") ) {
            effect->getApp()->appendToScriptEditor("Failed to run beforeRender callback: " + signatureError);

            return;
        }


        std::stringstream ss;
        std::string appStr = effect->getApp()->getAppIDString();
        std::string outputNodeName = appStr + "." + effect->getNode()->getFullyQualifiedName();
        ss << cb << "(" << outputNodeName << ", " << appStr << ")";
        std::string script = ss.str();
        try {
            runCallbackWithVariables( QString::fromUtf8( script.c_str() ) );
        } catch (const std::exception &e) {
            notifyRenderFailure( e.what() );
        }
    }
} // DefaultScheduler::aboutToStartRender

void
DefaultScheduler::onRenderStopped(bool aborted)
{
    OutputEffectInstancePtr effect = _effect.lock();
    bool isBackGround = appPTR->isBackground();

    if (!isBackGround) {
        effect->setKnobsFrozen(false);
    }

    {
        QString longText = QString::fromUtf8( effect->getScriptName_mt_safe().c_str() ) + tr(" ==> Rendering finished");
        appPTR->writeToOutputPipe(longText, QString::fromUtf8(kRenderingFinishedStringShort), true);
    }

    effect->notifyRenderFinished();

    std::string cb = effect->getNode()->getAfterRenderCallback();
    if ( !cb.empty() ) {
        std::vector<std::string> args;
        std::string error;
        try {
            NATRON_PYTHON_NAMESPACE::getFunctionArguments(cb, &error, &args);
        } catch (const std::exception& e) {
            effect->getApp()->appendToScriptEditor( std::string("Failed to get signature of afterRender callback: ")
                                                    + e.what() );

            return;
        }

        if ( !error.empty() ) {
            effect->getApp()->appendToScriptEditor("Failed to get signature of afterRender callback: " + error);

            return;
        }

        std::string signatureError;
        signatureError.append("The after render callback supports the following signature(s):\n");
        signatureError.append("- callback(aborted, thisNode, app)");
        if ( (args.size() != 3) || (args[0] != "aborted") || (args[1] != "thisNode") || (args[2] != "app") ) {
            effect->getApp()->appendToScriptEditor("Wrong signature of afterRender callback: " + signatureError);

            return;
        }


        std::stringstream ss;
        std::string appStr = effect->getApp()->getAppIDString();
        std::string outputNodeName = appStr + "." + effect->getNode()->getFullyQualifiedName();
        ss << cb << "(";
        if (aborted) {
            ss << "True, ";
        } else {
            ss << "False, ";
        }
        ss << outputNodeName << ", " << appStr << ")";
        std::string script = ss.str();
        try {
            runCallbackWithVariables( QString::fromUtf8( script.c_str() ) );
        } catch (...) {
            //Ignore expcetions in callback since the render is finished anyway
        }
    }
} // DefaultScheduler::onRenderStopped

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//////////////////////// ViewerDisplayScheduler ////////////


ViewerDisplayScheduler::ViewerDisplayScheduler(RenderEngine* engine,
                                               const ViewerInstancePtr& viewer)
    : OutputSchedulerThread(engine, viewer, eProcessFrameByMainThread) //< OpenGL rendering is done on the main-thread
    , _viewer(viewer)
{
}

ViewerDisplayScheduler::~ViewerDisplayScheduler()
{
}

/**
 * @brief Called whenever there are images available to process in the buffer.
 * Once processed, the frame will be removed from the buffer.
 *
 * According to the ProcessFrameModeEnum given to the scheduler this function will be called either by the scheduler thread (this)
 * or by the application's main-thread (typically to do OpenGL rendering).
 **/
void
ViewerDisplayScheduler::processFrame(const BufferedFrames& frames)
{
    ViewerInstancePtr viewer = _viewer.lock();

    if ( !frames.empty() ) {
        viewer->aboutToUpdateTextures();
    }
    if ( !frames.empty() ) {
        for (BufferedFrames::const_iterator it = frames.begin(); it != frames.end(); ++it) {
            UpdateViewerParamsPtr params = std::dynamic_pointer_cast<UpdateViewerParams>(it->frame);
            assert(params);
            viewer->updateViewer(params);
        }
        viewer->redrawViewerNow();
    } else {
        viewer->redrawViewer();
    }
}

void
ViewerDisplayScheduler::timelineStepOne(RenderDirectionEnum direction)
{
    ViewerInstancePtr viewer = _viewer.lock();

    if (direction == eRenderDirectionForward) {
        viewer->getTimeline()->incrementCurrentFrame();
    } else {
        viewer->getTimeline()->decrementCurrentFrame();
    }
}

void
ViewerDisplayScheduler::timelineGoTo(int time)
{
    ViewerInstancePtr viewer = _viewer.lock();

    viewer->getTimeline()->seekFrame(time, false, 0, eTimelineChangeReasonPlaybackSeek);
}

int
ViewerDisplayScheduler::timelineGetTime() const
{
    return _viewer.lock()->getTimeline()->currentFrame();
}

void
ViewerDisplayScheduler::getFrameRangeToRender(int &first,
                                              int &last) const
{
    ViewerInstancePtr viewer = _viewer.lock();
    ViewerInstance* leadViewer = viewer->getApp()->getLastViewerUsingTimeline();
    ViewerInstance* v = leadViewer ? leadViewer : viewer.get();

    assert(v);
    v->getTimelineBounds(&first, &last);
}

class ViewerRenderFrameRunnable
    : public RenderThreadTask
{
    ViewerInstanceWPtr _viewer;

public:

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    ViewerRenderFrameRunnable(const ViewerInstancePtr& viewer,
                              OutputSchedulerThread* scheduler)
        : RenderThreadTask(viewer, scheduler)
        , _viewer(viewer)
    {
    }

#else
    ViewerRenderFrameRunnable(const ViewerInstancePtr& viewer,
                              OutputSchedulerThread* scheduler,
                              const int frame,
                              const bool useRenderStarts,
                              const std::vector<int>& viewsToRender)
        : RenderThreadTask(viewer, scheduler, frame, useRenderStarts, viewsToRender)
        , _viewer(viewer)
    {
    }

#endif


    virtual ~ViewerRenderFrameRunnable()
    {
    }

private:

    virtual void renderFrame(int time,
                             const std::vector<ViewIdx>& viewsToRender,
                             bool enableRenderStats)
    {
        RenderStatsPtr stats;

        if (enableRenderStats) {
            stats = std::make_shared<RenderStats>(enableRenderStats);
        }
        ///The viewer always uses the scheduler thread to regulate the output rate, @see ViewerInstance::renderViewer_internal
        ///it calls appendToBuffer by itself
        ViewerInstance::ViewerRenderRetCode stat = ViewerInstance::eViewerRenderRetCodeRedraw;

        //Viewer can only render 1 view for now
        assert(viewsToRender.size() == 1);
        ViewIdx view = viewsToRender.front();
        ViewerInstancePtr viewer = _viewer.lock();
        U64 viewerHash = viewer->getHash();
        ViewerArgsPtr args[2];
        ViewerInstance::ViewerRenderRetCode status[2] = {
            ViewerInstance::eViewerRenderRetCodeFail, ViewerInstance::eViewerRenderRetCodeFail
        };
        bool clearTexture[2] = { false, false };
        BufferableObjectPtrList toAppend;

        for (int i = 0; i < 2; ++i) {
            args[i] = std::make_shared<ViewerArgs>();
            status[i] = viewer->getRenderViewerArgsAndCheckCache_public( time, true, view, i, viewerHash, true, NodePtr(), stats, args[i].get() );
            clearTexture[i] = status[i] == ViewerInstance::eViewerRenderRetCodeFail || status[i] == ViewerInstance::eViewerRenderRetCodeBlack;
            if (status[i] == ViewerInstance::eViewerRenderRetCodeFail) {
                //Just clear the viewer, nothing to do
                args[i]->params.reset();
                args[i].reset();
            } else if (status[i] == ViewerInstance::eViewerRenderRetCodeBlack) {
                if (args[i]->params) {
                    args[i]->params->tiles.clear();
                    toAppend.push_back(args[i]->params);
                    args[i]->params.reset();
                }
                args[i].reset();
            }
        }

        if ( (status[0] == ViewerInstance::eViewerRenderRetCodeFail) && (status[1] == ViewerInstance::eViewerRenderRetCodeFail) ) {
            viewer->disconnectViewer();
            return;
        }

        if (clearTexture[0]) {
            viewer->disconnectTexture(0, status[0] == ViewerInstance::eViewerRenderRetCodeFail);
        }
        if (clearTexture[0]) {
            viewer->disconnectTexture(1, status[1] == ViewerInstance::eViewerRenderRetCodeFail);
        }

        if ( ( (status[0] == ViewerInstance::eViewerRenderRetCodeRedraw) && !args[0]->params->isViewerPaused ) &&
                    ( ( status[1] == ViewerInstance::eViewerRenderRetCodeRedraw) && !args[1]->params->isViewerPaused ) ) {
            return;
        } else {
            for (int i = 0; i < 2; ++i) {
                if (args[i] && args[i]->params) {
                    if ( ( (args[i]->params->nbCachedTile > 0) && ( args[i]->params->nbCachedTile == (int)args[i]->params->tiles.size() ) ) || args[i]->params->isViewerPaused ) {
                        toAppend.push_back(args[i]->params);
                        args[i].reset();
                    }
                }
            }
        }


        if ( ( args[0] && (status[0] != ViewerInstance::eViewerRenderRetCodeFail) ) || ( args[1] && (status[1] != ViewerInstance::eViewerRenderRetCodeFail) ) ) {
            try {
                stat = viewer->renderViewer(view, false, true, viewerHash, true, NodePtr(), true,  args, ViewerCurrentFrameRequestSchedulerStartArgsPtr(), stats);
            } catch (...) {
                stat = ViewerInstance::eViewerRenderRetCodeFail;
            }
        }
        if (stat == ViewerInstance::eViewerRenderRetCodeFail) {
            ///Don't report any error message otherwise we will flood the viewer with irrelevant messages such as
            ///"Render failed", instead we let the plug-in that failed post an error message which will be more helpful.
            _imp->scheduler->notifyRenderFailure( std::string() );
        } else {
            for (int i = 0; i < 2; ++i) {
                if (args[i] && args[i]->params) {
                    toAppend.push_back(args[i]->params);
                    args[i].reset();
                }
            }
        }
        _imp->scheduler->appendToBuffer(time, view, stats, toAppend);
    } // renderFrame
};


#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
RenderThreadTask*
ViewerDisplayScheduler::createRunnable()
{
    return new ViewerRenderFrameRunnable(_viewer.lock(), this);
}

#else
RenderThreadTask*
ViewerDisplayScheduler::createRunnable(int frame,
                                       bool useRenderStarts,
                                       const std::vector<int>& viewsToRender)
{
    return new ViewerRenderFrameRunnable(_viewer.lock(), this, frame, useRenderStarts, viewsToRender);
}

#endif


void
ViewerDisplayScheduler::handleRenderFailure(const std::string& /*errorMessage*/)
{
    _viewer.lock()->disconnectViewer();
}

void
ViewerDisplayScheduler::onRenderStopped(bool /*/aborted*/)
{
    ///Refresh all previews in the tree
    ViewerInstancePtr viewer = _viewer.lock();

    viewer->getApp()->refreshAllPreviews();

    if ( !viewer->getApp() || viewer->getApp()->isGuiFrozen() ) {
        getEngine()->s_refreshAllKnobs();
    }
}

int
ViewerDisplayScheduler::getLastRenderedTime() const
{
    return _viewer.lock()->getLastRenderedTime();
}

////////////////////////// RenderEngine

struct RenderEnginePrivate
{
    QMutex schedulerCreationLock;
    OutputSchedulerThread* scheduler;

    //If true then a current frame render can start playback, protected by abortedRequestedMutex
    bool canAutoRestartPlayback;
    QMutex canAutoRestartPlaybackMutex; // protects abortRequested
    OutputEffectInstanceWPtr output;
    mutable QMutex pbModeMutex;
    PlaybackModeEnum pbMode;
    ViewerCurrentFrameRequestScheduler* currentFrameScheduler;

    // Only used on the main-thread
    std::unique_ptr<RenderEngineWatcher> engineWatcher;
    struct RefreshRequest
    {
        bool enableStats;
        bool enableAbort;
    };

    /*
       This queue tracks all calls made to renderCurrentFrame() and attempts to concatenate the calls
       once the event loop fires the signal currentFrameRenderRequestPosted()
       This is only accessed on the main thread
     */
    std::list<RefreshRequest> refreshQueue;

    RenderEnginePrivate(const OutputEffectInstancePtr& output)
        : schedulerCreationLock()
        , scheduler(0)
        , canAutoRestartPlayback(false)
        , canAutoRestartPlaybackMutex()
        , output(output)
        , pbModeMutex()
        , pbMode(ePlaybackModeLoop)
        , currentFrameScheduler(0)
        , refreshQueue()
    {
    }
};

RenderEngine::RenderEngine(const OutputEffectInstancePtr& output)
    : _imp( new RenderEnginePrivate(output) )
{
    QObject::connect(this, SIGNAL(currentFrameRenderRequestPosted()), this, SLOT(onCurrentFrameRenderRequestPosted()), Qt::QueuedConnection);
}

RenderEngine::~RenderEngine()
{
    delete _imp->currentFrameScheduler;
    _imp->currentFrameScheduler = 0;
    delete _imp->scheduler;
    _imp->scheduler = 0;
}

OutputSchedulerThread*
RenderEngine::createScheduler(const OutputEffectInstancePtr& effect)
{
    return new DefaultScheduler(this, effect);
}

OutputEffectInstancePtr
RenderEngine::getOutput() const
{
    return _imp->output.lock();
}

