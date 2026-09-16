// Engine/EffectInstancePart3.inc — included from EffectInstance.h (class body).
// Split so GitHub Contents/MCP writes stay under the 100k cap.

    /**
     * @brief Called whenever a param changes. It calls the virtual
     * portion paramChangedByUser(...) and brackets the call by a begin/end if it was
     * not done already.
     **/
    virtual bool knobChanged(KnobI* /*k*/,
                             ValueChangedReasonEnum /*reason*/,
                             ViewSpec /*view*/,
                             double /*time*/,
                             bool /*originatedFromMainThread*/)
    {
        return false;
    }

    virtual StatusEnum beginSequenceRender(double /*first*/,
                                           double /*last*/,
                                           double /*step*/,
                                           bool /*interactive*/,
                                           const RenderScale & /*scale*/,
                                           bool /*isSequentialRender*/,
                                           bool /*isRenderResponseToUserInteraction*/,
                                           bool /*draftMode*/,
                                           ViewIdx /*view*/,
                                           bool /*isOpenGLRender*/,
                                           const EffectInstance::OpenGLContextEffectDataPtr& /*glContextData*/)
    {
        return eStatusOK;
    }

    virtual StatusEnum endSequenceRender(double /*first*/,
                                         double /*last*/,
                                         double /*step*/,
                                         bool /*interactive*/,
                                         const RenderScale & /*scale*/,
                                         bool /*isSequentialRender*/,
                                         bool /*isRenderResponseToUserInteraction*/,
                                         bool /*draftMode*/,
                                         ViewIdx /*view*/,
                                         bool /*isOpenGLRender*/,
                                         const EffectInstance::OpenGLContextEffectDataPtr& /*glContextData*/)
    {
        return eStatusOK;
    }

public:

    /**
     * @brief Push a new undo command to the undo/redo stack associated to this node.
     * The stack takes ownership of the shared pointer, so you should not hold a strong reference to the passed pointer.
     * If no undo/redo stack is present, the command will just be redone once then destroyed.
     **/
    void pushUndoCommand(const UndoCommandPtr& command);

    // Same as the version above, do NOT dereference command after this call as it will be destroyed already, usually this call is
    // made as such: pushUndoCommand(new MyCommand(...))
    void pushUndoCommand(UndoCommand* command);

    /**
     * @brief Set the cursor to be one of the default cursor.
     * @returns True if it successfully set the cursor, false otherwise.
     * Note: this can only be called during an overlay interact action.
     **/
    bool setCurrentCursor(CursorEnum defaultCursor);
    bool setCurrentCursor(const QString& customCursorFilePath);

    ///Doesn't do anything, instead we overridden onKnobValueChanged_public
    virtual bool onKnobValueChanged(KnobI* k,
                                    ValueChangedReasonEnum reason,
                                    double time,
                                    ViewSpec view,
                                    bool originatedFromMainThread) OVERRIDE FINAL;
    StatusEnum beginSequenceRender_public(double first, double last,
                                          double step, bool interactive, const RenderScale & scale,
                                          bool isSequentialRender, bool isRenderResponseToUserInteraction,
                                          bool draftMode,
                                          ViewIdx view,
                                          bool isOpenGLRender,
                                          const EffectInstance::OpenGLContextEffectDataPtr& glContextData);
    StatusEnum endSequenceRender_public(double first, double last,
                                        double step, bool interactive, const RenderScale & scale,
                                        bool isSequentialRender, bool isRenderResponseToUserInteraction,
                                        bool draftMode,
                                        ViewIdx view,
                                        bool isOpenGLRender,
                                        const EffectInstance::OpenGLContextEffectDataPtr& glContextData);

    virtual bool canHandleRenderScaleForOverlays() const { return true; }


    void drawOverlay_public(double time, const RenderScale & renderScale, ViewIdx view);

    bool onOverlayPenDown_public(double time, const RenderScale & renderScale, ViewIdx view, const QPointF & viewportPos, const QPointF & pos, double pressure, double timestamp, PenType pen) WARN_UNUSED_RETURN;

    bool onOverlayPenMotion_public(double time, const RenderScale & renderScale, ViewIdx view, const QPointF & viewportPos, const QPointF & pos, double pressure, double timestamp) WARN_UNUSED_RETURN;

    bool onOverlayPenDoubleClicked_public(double time, const RenderScale & renderScale, ViewIdx view, const QPointF & viewportPos, const QPointF & pos) WARN_UNUSED_RETURN;

    bool onOverlayPenUp_public(double time, const RenderScale & renderScale, ViewIdx view, const QPointF & viewportPos, const QPointF & pos, double pressure, double timestamp) WARN_UNUSED_RETURN;

    bool onOverlayKeyDown_public(double time, const RenderScale & renderScale, ViewIdx view, Key key, KeyboardModifiers modifiers) WARN_UNUSED_RETURN;

    bool onOverlayKeyUp_public(double time, const RenderScale & renderScale, ViewIdx view, Key key, KeyboardModifiers modifiers) WARN_UNUSED_RETURN;

    bool onOverlayKeyRepeat_public(double time, const RenderScale & renderScale, ViewIdx view, Key key, KeyboardModifiers modifiers) WARN_UNUSED_RETURN;

    bool onOverlayFocusGained_public(double time, const RenderScale & renderScale, ViewIdx view) WARN_UNUSED_RETURN;

    bool onOverlayFocusLost_public(double time, const RenderScale & renderScale, ViewIdx view) WARN_UNUSED_RETURN;

    void setInteractColourPicker_public(const OfxRGBAColourD& color, bool setColor, bool hasColor);

    virtual bool isDoingInteractAction() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void onInteractViewportSelectionCleared() {}

    virtual void onInteractViewportSelectionUpdated(const RectD& /*rectangle*/,
                                                    bool /*onRelease*/) {}

    void setDoingInteractAction(bool doing);

    /* @brief Overlay support:
     * Just overload this function in your operator.
     * No need to include any OpenGL related header.
     * The coordinate space is  defined by the displayWindow
     * (i.e: (0,0) = bottomLeft and  width() and height() being
     * respectively the width and height of the frame.)
     */
    virtual bool hasOverlay() const
    {
        return false;
    }

    virtual void redrawOverlayInteract();

    /**
     * @brief Flag that the overlays should be redrawn when this knob changes.
     **/
    void addOverlaySlaveParam(const KnobIPtr& knob);

    bool isOverlaySlaveParam(const KnobI* knob) const;


    /**
     * @brief Reimplement to control how the host adds the RGBA checkboxes.
     * @returns True if you want the host to add the RGBA checkboxes, false otherwise.
     **/
    virtual bool isHostChannelSelectorSupported(bool* defaultR,
                                                bool* defaultG,
                                                bool* defaultB,
                                                bool* defaultA) const
    {
        *defaultR = true;
        *defaultG = true;
        *defaultB = true;
        *defaultA = true;

        return true;
    }

    /**
     * @brief Reimplement to activate host masking
     * Note that in this case this is expected that getNInputs returns the number of inputs *with* the mask.
     * The function getInputLabel should also return the appropriate label for the mask.
     * The function isInputMask should also return true for this mask index.
     * The mask will be the last input, i.e its index will be getNInputs() - 1.
     **/
    virtual bool isHostMaskingEnabled() const
    {
        return false;
    }

    /**
     * @brief Reimplement to activate host mixing
     **/
    virtual bool isHostMixingEnabled() const
    {
        return false;
    }

    virtual void onKnobsAboutToBeLoaded(const NodeSerializationPtr& /*serialization*/) {}

    virtual void onKnobsLoaded() {}

    /**
     * @brief Called after all knobs have been loaded and the nod ehas been created
     **/
    virtual void onEffectCreated(bool /*mayCreateFileDialog*/,
                                 const CreateNodeArgs& /*args*/) {}

    virtual void onKnobSlaved(const KnobIPtr& slave, const KnobIPtr& master,
                              int dimension,
                              bool isSlave) OVERRIDE FINAL;

    /**
     * @brief This function calls the implementation specific attachOpenGLContext()
     **/
    StatusEnum attachOpenGLContext_public(const OSGLContextPtr& glContext, EffectInstance::OpenGLContextEffectDataPtr* data);

    /**
     * @brief This function calls the implementation specific dettachOpenGLContext()
     **/
    StatusEnum dettachOpenGLContext_public(const OSGLContextPtr& glContext, const EffectInstance::OpenGLContextEffectDataPtr& data);

    /**
     * @brief Called for plug-ins that support concurrent OpenGL renders when the effect is about to be destroyed to release all contexts data.
     **/
    void dettachAllOpenGLContexts();

    /**
     * @brief Must return whether the plug-in handles concurrent OpenGL renders or not.
     * By default the OpenFX OpenGL render suite cannot support concurrent OpenGL renders, but version 2 specified by Natron allows for this.
     **/
    virtual bool supportsConcurrentOpenGLRenders() const { return true; }


    void clearRenderInstances();

protected:


    /**
     * @brief Plug-ins that are flagged eRenderSafetyInstanceSafe or lower can implement this function to make a copy
     * of the effect that will be used to render. The copy should be as fast as possible, meaning any clip or parameter should
     * share pointers (since internally these classes are thread-safe) and ensure that any private data member is copied.
     **/
    virtual EffectInstancePtr createRenderClone() { return EffectInstancePtr(); }


    /**
    * @brief Must be implemented to evaluate a value change
    * made to a knob(e.g: force a new render).
    * @param knob[in] The knob whose value changed.
    **/
    virtual void evaluate(bool isSignificant, bool refreshMetadatas) OVERRIDE;

private:

    EffectInstancePtr getOrCreateRenderInstance();


    void releaseRenderInstance(const EffectInstancePtr& instance);

    /**
     * @brief This function must initialize all OpenGL context related data such as shaders, LUTs, etc...
     * This function will be called once per context. The function dettachOpenGLContext() will be called
     * on the value returned by this function to deinitialize all context related data.
     *
     * If the function supportsConcurrentOpenGLRenders() returns false, each call to attachOpenGLContext must be followed by
     * a call to dettachOpenGLContext before attaching a DIFFERENT context, meaning the plug-in thread-safety is instance safe at most.
     *
     * Possible return status code:
     * eStatusOK , the action was trapped and all was well
     * eStatusReplyDefault , the action was ignored, but all was well anyway
     * eStatusOutOfMemory , in which case this may be called again after a memory purge
     * eStatusFailed , something went wrong, but no error code appropriate, the plugin should to post a message if possible and the host should not attempt to run the plugin in OpenGL render mode.
     **/
    virtual StatusEnum attachOpenGLContext(EffectInstance::OpenGLContextEffectDataPtr* /*data*/) { return eStatusReplyDefault; }

    /**
     * @brief This function must free all OpenGL context related data that were allocated previously in a call to attachOpenGLContext().
     * Possible return status code:
     * eStatusOK , the action was trapped and all was well
     * eStatusReplyDefault , the action was ignored, but all was well anyway
     * eStatusOutOfMemory , in which case this may be called again after a memory purge
     * eStatusFailed , something went wrong, but no error code appropriate, the plugin should to post a message if possible and the host should not attempt to run the plugin in OpenGL render mode.
     **/
    virtual StatusEnum dettachOpenGLContext(const OpenGLContextEffectDataPtr& /*data*/) { return eStatusReplyDefault; }



public:

    void getComponentsNeededAndProduced_public(U64 hash,
                                               double time, ViewIdx view,
                                               EffectInstance::ComponentsNeededMap* comps,
                                               std::list<ImagePlaneDesc>* passThroughPlanes,
                                               bool* processAllRequested,
                                               double* passThroughTime,
                                               int* passThroughView,
                                               std::bitset<4> *processChannels,
                                               int* passThroughInput);


    void getAvailableLayers(double time, ViewIdx view, int inputNb, std::list<ImagePlaneDesc>* availableLayers) ;

    const std::vector<std::string>& getUserPlanes() const;

private:

    void getComponentsNeededDefault(double time, ViewIdx view,
                                    EffectInstance::ComponentsNeededMap* comps,
                                    std::list<ImagePlaneDesc>* passThroughPlanes,
                                    bool* processAllRequested,
                                    double* passThroughTime,
                                    int* passThroughView,
                                    std::bitset<4> *processChannels,
                                    int* passThroughInput);

public:


    /**
     * @brief Check if Transform effects concatenation is possible on the current node and node upstream.
     **/
    void tryConcatenateTransforms(double time,
                                  bool draftRender,
                                  ViewIdx view,
                                  const RenderScale & scale,
                                  InputMatrixMap* inputTransforms);


    static void transformInputRois(const EffectInstance* self,
                                   const InputMatrixMapPtr& inputTransforms,
                                   double par,
                                   const RenderScale & scale,
                                   RoIMap* inputRois,
                                   std::map<int, EffectInstancePtr>* reroutesMap);
    struct RenderArgs
    {
        RectD rod; //!< the effect's RoD in CANONICAL coordinates
        RoIMap regionOfInterestResults; //< the input RoI's in CANONICAL coordinates
        RectI renderWindowPixel; //< the current renderWindow in PIXEL coordinates
        double time; //< the time to render
        ViewIdx view; //< the view to render
        bool validArgs; //< are the args valid ?
        bool isIdentity;
        double identityTime;
        EffectInstancePtr identityInput;
        EffectInstance::InputImagesMap inputImages;
        std::map<ImagePlaneDesc, PlaneToRender> outputPlanes;

        //This is set only when the plug-in has set ePassThroughRenderAllRequestedPlanes
        ImagePlaneDesc outputPlaneBeingRendered;
        ComponentsNeededMapPtr  compsNeeded;
        double firstFrame, lastFrame;
        InputMatrixMapPtr transformRedirections;
        bool isDoingOpenGLRender;

        RenderArgs();

        RenderArgs(const RenderArgs & o);

        RenderArgs& operator=(const RenderArgs& other); // copy assignment
    };

    //these are per-node thread-local data
    struct EffectTLSData
    {
        //Used to count the begin/endRenderAction recursion
        int beginEndRenderCount;


        ///Used to count the recursion in the function calls
        /* The image effect actions which may trigger a recursive action call on a single instance are...

           kOfxActionBeginInstanceChanged
           kOfxActionInstanceChanged
           kOfxActionEndInstanceChanged
           The interact actions which may trigger a recursive action to be called on the associated plugin instance are...

           kOfxInteractActionGainFocus
           kOfxInteractActionKeyDown
           kOfxInteractActionKeyRepeat
           kOfxInteractActionKeyUp
           kOfxInteractActionLoseFocus
           kOfxInteractActionPenDown
           kOfxInteractActionPenMotion
           kOfxInteractActionPenUp

           The image effect actions which may be called recursively are...

           kOfxActionBeginInstanceChanged
           kOfxActionInstanceChanged
           kOfxActionEndInstanceChanged
           kOfxImageEffectActionGetClipPreferences
           The interact actions which may be called recursively are...

           kOfxInteractActionDraw

         */
        int actionRecursionLevel;
#ifdef DEBUG
        std::list<bool> canSetValue;
#endif

        ///Recursive because it may be set recursively in such situation:
        ///knobChanged : set ParallelRenderArgs TLS for analysis
        ///timelineGoTo calls getRenderviewerArgs on the MT which overrides this TLS
        std::list<ParallelRenderArgsPtr> frameArgs;
        EffectInstance::RenderArgs currentRenderArgs;

        std::vector<std::string> userPlaneStrings;

        EffectTLSData()
            : beginEndRenderCount(0)
            , actionRecursionLevel(0)
#ifdef DEBUG
            , canSetValue()
#endif
            , frameArgs()
            , currentRenderArgs()
            , userPlaneStrings()
        {
        }
    };

    typedef std::shared_ptr<EffectTLSData> EffectTLSDataPtr;

protected:

    /**
     * @brief Equivalent to assert(actionsRecursionLevel == 0).
     * In release mode an exception is thrown instead.
     * This should be called in all actions except in the following recursive actions...
     *
     * kOfxActionBeginInstanceChanged
     * kOfxActionInstanceChanged
     * kOfxActionEndInstanceChanged
     * kOfxImageEffectActionGetClipPreferences
     * kOfxInteractActionDraw
     *
     * We also allow recursion in some other actions such as getRegionOfDefinition or isIdentity
     **/
    virtual void assertActionIsNotRecursive() const OVERRIDE FINAL;

    /**
     * @brief Should be called in the beginning of an action.
     * Right after assertActionIsNotRecursive() for non recursive actions.
     **/
    virtual void incrementRecursionLevel() OVERRIDE FINAL;

    /**
     * @brief Should be called at the end of an action.
     **/
    virtual void decrementRecursionLevel() OVERRIDE FINAL;

public:

    virtual int getRecursionLevel() const OVERRIDE FINAL WARN_UNUSED_RETURN;

protected:

    virtual void refreshExtraStateAfterTimeChanged(bool isPlayback, double time)  OVERRIDE;

    /**
     * @brief Must be implemented to initialize any knob using the
     * KnobFactory.
     **/
    virtual void initializeKnobs() OVERRIDE
    {
    };


    /**
     * @brief Returns a map of the components produced by this effect and the components needed by the inputs of this effect.
     * The output is mapped against -1. For all components not produced and if this effect is passthrough, it should use the
     * passThroughInput to fetch the components needed.
     **/
    virtual void getComponentsNeededAndProduced(double time, ViewIdx view,
                                                EffectInstance::ComponentsNeededMap* comps,
                                                double* passThroughTime,
                                                int* passThroughView,
                                                int* passThroughInput);


    virtual void setInteractColourPicker(const OfxRGBAColourD& /*color*/, bool /*setColor*/, bool /*hasColor*/)
    {

    }

    virtual bool shouldPreferPluginOverlayOverHostOverlay() const
    {
        return true;
    }

    virtual bool shouldDrawHostOverlay() const
    {
        return true;
    }

    virtual void drawOverlay(double /*time*/,
                             const RenderScale & /*renderScale*/,
                             ViewIdx /*view*/)
    {
    }

    virtual bool onOverlayPenDown(double /*time*/,
                                  const RenderScale & /*renderScale*/,
                                  ViewIdx /*view*/,
                                  const QPointF & /*viewportPos*/,
                                  const QPointF & /*pos*/,
                                  double /*pressure*/,
                                  double /*timestamp*/,
                                  PenType /*pen*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayPenDoubleClicked(double /*time*/,
                                           const RenderScale & /*renderScale*/,
                                           ViewIdx /*view*/,
                                           const QPointF & /*viewportPos*/,
                                           const QPointF & /*pos*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayPenMotion(double /*time*/,
                                    const RenderScale & /*renderScale*/,
                                    ViewIdx /*view*/,
                                    const QPointF & /*viewportPos*/,
                                    const QPointF & /*pos*/,
                                    double /*pressure*/,
                                    double /*timestamp*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayPenUp(double /*time*/,
                                const RenderScale & /*renderScale*/,
                                ViewIdx /*view*/,
                                const QPointF & /*viewportPos*/,
                                const QPointF & /*pos*/,
                                double /*pressure*/,
                                double /*timestamp*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayKeyDown(double /*time*/,
                                  const RenderScale & /*renderScale*/,
                                  ViewIdx /*view*/,
                                  Key /*key*/,
                                  KeyboardModifiers /*modifiers*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayKeyUp(double /*time*/,
                                const RenderScale & /*renderScale*/,
                                ViewIdx /*view*/,
                                Key /*key*/,
                                KeyboardModifiers /*modifiers*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayKeyRepeat(double /*time*/,
                                    const RenderScale & /*renderScale*/,
                                    ViewIdx /*view*/,
                                    Key /*key*/,
                                    KeyboardModifiers /*modifiers*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayFocusGained(double /*time*/,
                                      const RenderScale & /*renderScale*/,
                                      ViewIdx /*view*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool onOverlayFocusLost(double /*time*/,
                                    const RenderScale & /*renderScale*/,
                                    ViewIdx /*view*/) WARN_UNUSED_RETURN
    {
        return false;
    }

    NodeWPtr _node; //< the node holding this effect

private:

    class Implementation;
    std::unique_ptr<Implementation> _imp; // PIMPL: hide implementation details

    friend class ReadNode;
    friend class WriteNode;
    friend class ImageBitMapMarker_RAII;

    enum RenderRoIStatusEnum
    {
        eRenderRoIStatusImageAlreadyRendered = 0, // there was nothing left to render
        eRenderRoIStatusImageRendered, // we rendered what was missing
        eRenderRoIStatusRenderFailed, // render failed
        eRenderRoIStatusRenderOutOfGPUMemory, // The render failed because the GPU did not have enough memory
    };


    /**
     * @brief The internal of renderRoI, mainly it calls render and handles the thread safety of the effect.
     * @param time The time at which to render
     * @param scale The scale at which to render
     * @param mipmapLevel Redundant with scale
     * @param view The view on which to render
     * @param renderWindow The rectangle to render of the image, in pixel coordinates
     * @param cachedImgParams The parameters of the image to render as they are in the cache.
     * @param image This is the \"full-scale\" image, if the effect does support the render scale, then
     * image and downscaledImage are pointing to the SAME image.
     * @param downscaledImage If the effect doesn't support the render scale, then this is a pointer to the
     * downscaled image. If the effect doesn't support render scale then it will render in the \"image\" parameter
     * and then downscale the results into downscaledImage.
     * @param isSequentialRender True when the render is sequential
     * @param isRenderMadeInResponseToUserInteraction True when the render is made due to user interaction
     * @param byPassCache Cache look-ups have been already handled by renderRoI(...) but we pass it here because
     * we need to call renderRoI() on the input effects with this parameter too.
     * @param nodeHash The hash of the node used to render. This might no longer be equal to the value returned by
     * getHash() because the user might have changed something in the project (parameters...links..)
     * @param channelForAlpha This is passed here because
     * we need to remember it later when converting the mask
     * which channel we wanted for the alpha channel.
     * @param renderFullScaleThenDownscale means that rendering should be done at full resolution and then
     * downscaled, because the plugin does not support render scale.
     * @returns True if the render call succeeded, false otherwise.
     **/
    static RenderRoIStatusEnum renderRoIInternal(EffectInstance* self,
                                                 double time,
                                                 const ParallelRenderArgsPtr & frameArgs,
                                                 RenderSafetyEnum safety,
                                                 unsigned int mipmapLevel,
                                                 ViewIdx view,
                                                 const RectD & rod, //!< rod in canonical coordinates
                                                 const double par,
                                                 const ImagePlanesToRenderPtr & planes,
                                                 bool isSequentialRender,
                                                 bool isRenderMadeInResponseToUserInteraction,
                                                 U64 nodeHash,
                                                 bool renderFullScaleThenDownscale,
                                                 bool byPassCache,
                                                 ImageBitDepthEnum outputClipPrefDepth,
                                                 const ImagePlaneDesc& outputClipPrefsComps,
                                                 const ComponentsNeededMapPtr & compsNeeded,
                                                 std::bitset<4> processChannels);


    /// \\returns false if rendering was aborted
    RenderRoIRetCode renderInputImagesForRoI(const FrameViewRequest* request,
                                             bool useTransforms,
                                             StorageModeEnum renderStorageMode,
                                             double time,
                                             ViewIdx view,
                                             const RectD & rod,
                                             const RectD & canonicalRenderWindow,
                                             const InputMatrixMapPtr & transformMatrix,
                                             unsigned int mipmapLevel,
                                             const RenderScale & renderMappedScale,
                                             bool useScaleOneInputImages,
                                             bool byPassCache,
                                             const FramesNeededMap & framesNeeded,
                                             const EffectInstance::ComponentsNeededMap & compsNeeded,
                                             EffectInstance::InputImagesMap *inputImages,
                                             RoIMap* inputsRoI);

    static ImagePtr convertPlanesFormatsIfNeeded(const AppInstancePtr& app,
                                                                 const ImagePtr& inputImage,
                                                                 const RectI& roi,
                                                                 const ImagePlaneDesc& targetComponents,
                                                                 ImageBitDepthEnum targetDepth,
                                                                 bool useAlpha0ForRGBToRGBAConversion,
                                                                 ImagePremultiplicationEnum outputPremult,
                                                                 int channelForAlpha);


    /**
     * @brief Called by getImage when the thread-storage was not set by the caller thread (mostly because this is a thread that is not
     * a thread controlled by Natron).
     **/
    // TODO: shouldn't this be documented a bit more? (parameters?)
    bool retrieveGetImageDataUponFailure(const double time,
                                         const ViewIdx view,
                                         const RenderScale & scale,
                                         const RectD* optionalBoundsParam,
                                         U64* nodeHash_p,
                                         bool* isIdentity_p,
                                         EffectInstancePtr* identityInput_p,
                                         bool* duringPaintStroke_p,
                                         RectD* rod_p,
                                         RoIMap* inputRois_p, //!< output, only set if optionalBoundsParam != NULL
                                         RectD* optionalBounds_p); //!< output, only set if optionalBoundsParam != NULL


    bool allocateImagePlane(const ImageKey & key,
                            const RectD & rod,
                            const RectI & downscaleImageBounds,
                            const RectI & fullScaleImageBounds,
                            bool isProjectFormat,
                            const ImagePlaneDesc & components,
                            ImageBitDepthEnum depth,
                            ImagePremultiplicationEnum premult,
                            ImageFieldingOrderEnum fielding,
                            double par,
                            unsigned int mipmapLevel,
                            bool renderFullScaleThenDownscale,
                            StorageModeEnum storage,
                            bool createInCache,
                            ImagePtr* fullScaleImage,
                            ImagePtr* downscaleImage);


    virtual void onSignificantEvaluateAboutToBeCalled(KnobI* knob) OVERRIDE FINAL;
    virtual void onAllKnobsSlaved(bool isSlave, KnobHolder* master) OVERRIDE FINAL;
    enum RenderingFunctorRetEnum
    {
        eRenderingFunctorRetFailed, //< must stop rendering
        eRenderingFunctorRetOK, //< ok, move on
        eRenderingFunctorRetAborted, // we were aborted
        eRenderingFunctorRetOutOfGPUMemory
    };


    /**
     * @brief Returns the index of the input if inputEffect is a valid input connected to this effect, otherwise returns -1.
     **/
    int getInputNumber(const EffectInstance* inputEffect) const;
};
