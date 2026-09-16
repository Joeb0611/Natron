// Engine/EffectInstancePart1.inc — included from EffectInstance.h (class body).
// Split so GitHub Contents/MCP writes stay under the 100k cap.
public:
    typedef std::map<int, std::list<ImagePtr> > InputImagesMap;
    typedef std::map<int, std::list<ImagePlaneDesc> > ComponentsNeededMap;
    typedef std::shared_ptr<ComponentsNeededMap> ComponentsNeededMapPtr;

    struct RenderRoIArgs
    {
        // Developer note: the fields were reordered to optimize packing.
        // see http://www.catb.org/esr/structure-packing/

        double time; //< the time at which to render
        RenderScale scale; //< the scale at which to render
        unsigned int mipmapLevel; //< the mipmap level (redundant with the scale, stored here to avoid refetching it everytimes)
        ViewIdx view; //< the view to render
        RectI roi; //< the renderWindow (in pixel coordinates) , watch out OpenFX action getRegionsOfInterest expects canonical coords!
        RectD preComputedRoD; //<  pre-computed region of definition in canonical coordinates for this effect to speed-up the call to renderRoi
        std::list<ImagePlaneDesc> components; //< the requested image components (per plane)

        ///When called from getImage() the calling node  will have already computed input images, hence the image of this node
        ///might already be in this list
        EffectInstance::InputImagesMap inputImagesList;
        const EffectInstance* caller;
        ImageBitDepthEnum bitdepth; //< the requested bit depth
        bool byPassCache;
        bool calledFromGetImage;

        // Request what kind of storage we need images to be in return of renderRoI
        StorageModeEnum returnStorage;

        // Set to false if you don't want the node to render using the GPU at all
        bool allowGPURendering;

        // the time that was passed to the original renderRoI call of the caller node
        double callerRenderTime;

        RenderRoIArgs()
            : time(0)
            , scale()
            , mipmapLevel(0)
            , view(0)
            , roi()
            , preComputedRoD()
            , components()
            , inputImagesList()
            , caller(0)
            , bitdepth(eImageBitDepthFloat)
            , byPassCache(false)
            , calledFromGetImage(false)
            , returnStorage(eStorageModeRAM)
            , allowGPURendering(true)
            , callerRenderTime(0.)
        {
        }

        RenderRoIArgs( double time_,
                       const RenderScale & scale_,
                       unsigned int mipmapLevel_,
                       ViewIdx view_,
                       bool byPassCache_,
                       const RectI & roi_,
                       const RectD & preComputedRoD_,
                       const std::list<ImagePlaneDesc> & components_,
                       ImageBitDepthEnum bitdepth_,
                       bool calledFromGetImage,
                       const EffectInstance* caller,
                       StorageModeEnum returnStorage,
                       double callerRenderTime,
                       const EffectInstance::InputImagesMap & inputImages = EffectInstance::InputImagesMap() )
            : time(time_)
            , scale(scale_)
            , mipmapLevel(mipmapLevel_)
            , view(view_)
            , roi(roi_)
            , preComputedRoD(preComputedRoD_)
            , components(components_)
            , inputImagesList(inputImages)
            , caller(caller)
            , bitdepth(bitdepth_)
            , byPassCache(byPassCache_)
            , calledFromGetImage(calledFromGetImage)
            , returnStorage(returnStorage)
            , allowGPURendering(true)
            , callerRenderTime(callerRenderTime)
        {
        }
    };

    enum SupportsEnum
    {
        eSupportsMaybe = -1,
        eSupportsNo = 0,
        eSupportsYes = 1
    };

public:
    // TODO: enable_shared_from_this
    // constructors should be privatized in any class that derives from std::enable_shared_from_this<>

    /**
     * @brief Constructor used once for each node created. Its purpose is to create the "live instance".
     * You shouldn't do any heavy processing here nor lengthy initialization as the constructor is often
     * called just to be able to call a few virtuals functions.
     * The constructor is always called by the main thread of the application.
     **/
    explicit EffectInstance(NodePtr node);

protected:
    EffectInstance(const EffectInstance& other);

public:

    // dtor
    virtual ~EffectInstance();


    virtual bool canHandleEvaluateOnChangeInOtherThread() const OVERRIDE
    {
        return true;
    }

    /**
     * @brief Returns true once the effect has been fully initialized and is ready to have its actions called apart from
     * the createInstanceAction
     **/
    virtual bool isEffectCreated() const
    {
        return true;
    }

    /**
     * @brief Returns a pointer to the node holding this effect.
     **/
    NodePtr getNode() const WARN_UNUSED_RETURN
    {
        return _node.lock();
    }

    /**
     * @brief Returns the "real" hash of the node synchronized with the gui state
     **/
    U64 getHash() const WARN_UNUSED_RETURN;

    /**
     * @brief Returns the hash the node had at the start of renderRoI. This will return the same value
     * at any time during the same render call.
     * @returns This function returns true if case of success, false otherwise.
     **/
    U64 getRenderHash() const WARN_UNUSED_RETURN;

    U64 getKnobsAge() const WARN_UNUSED_RETURN;

    /**
     * @brief Set the knobs age of this node to be 'age'. Note that this can be called
     * for 2 reasons:
     * - loading a project
     * - If this node is a clone and the master node changed its hash.
     **/
    void setKnobsAge(U64 age);

    /**
     * @brief Forwarded to the node's name
     **/
    const std::string & getScriptName() const WARN_UNUSED_RETURN;
    virtual std::string getScriptName_mt_safe() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual std::string getFullyQualifiedName() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void onScriptNameChanged(const std::string& /*fullyQualifiedName*/) {}

    /**
     * @brief Returns the node output format
     **/
    RectI getOutputFormat() const;

    /**
     * @brief Forwarded to the node's render views count
     **/
    int getRenderViewsCount() const WARN_UNUSED_RETURN;

    /**
     * @brief Returns input n. It might be NULL if the input is not connected.
     * MT-Safe
     **/
    EffectInstancePtr getInput(int n) const WARN_UNUSED_RETURN;

    /**
     * @brief Forwarded to the node holding the effect
     **/
    bool hasOutputConnected() const WARN_UNUSED_RETURN;

    /**
     * @brief Must return the plugin's major version.
     **/
    virtual int getMajorVersion() const WARN_UNUSED_RETURN = 0;

    /**
     * @brief Must return the plugin's minor version.
     **/
    virtual int getMinorVersion() const WARN_UNUSED_RETURN = 0;

    /**
     * @brief Is this node an input node ? An input node means
     * it has no input.
     **/
    virtual bool isGenerator() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Is the node a reader ?
     **/
    virtual bool isReader() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Basically returns true for PLUGINID_OFX_READFFMPEG
     **/
    virtual bool isVideoReader() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Is the node a writer ?
     **/
    virtual bool isWriter() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Basically returns true for PLUGINID_OFX_WRITEFFMPEG
     **/
    virtual bool isVideoWriter() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Is this node an output node ? An output node means
     * it has no output.
     **/
    virtual bool isOutput() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Returns true if this node is a tracker
     **/
    virtual bool isTrackerNodePlugin() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Returns true if this is our tracking node.
     **/
    virtual bool isBuiltinTrackerNode() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Returns true if this node is a tracker
     **/
    virtual bool isRotoPaintNode() const WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual bool isPaintingOverItselfEnabled() const WARN_UNUSED_RETURN;

    /**
     * @brief Returns true if the node is capable of processing data from its input
     **/
    virtual bool isFilter() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief Is this node an OpenFX node?
     **/
    virtual bool isOpenFX() const WARN_UNUSED_RETURN
    {
        return false;
    }

    /**
     * @brief How many input can we have at most. (i.e: how many input arrows)
     * This function should be MT-safe and should never change the value returned.
     **/
    virtual int getNInputs() const WARN_UNUSED_RETURN = 0;

    /**
     * @brief Is inputNb optional ? In which case the render can be made without it.
     **/
    virtual bool isInputOptional(int inputNb) const WARN_UNUSED_RETURN = 0;

    /**
     * @brief Is inputNb a mask ? In which case the effect will have an additional mask parameter.
     **/
    virtual bool isInputMask(int /*inputNb*/) const WARN_UNUSED_RETURN
    {
        return false;
    };

    /**
     * @brief Is the input a roto brush ?
     **/
    virtual bool isInputRotoBrush(int /*inputNb*/) const WARN_UNUSED_RETURN
    {
        return false;
    }

    virtual int getRotoBrushInputIndex() const WARN_UNUSED_RETURN
    {
        return -1;
    }

    virtual bool getMakeSettingsPanel() const { return true; }


    virtual bool getCreateChannelSelectorKnob() const;

    /**
     * @brief Returns the index of the channel to use to produce the mask and the components.
     * None = -1
     * R = 0
     * G = 1
     * B = 2
     * A = 3
     **/
    int getMaskChannel(int inputNb, const std::list<ImagePlaneDesc>& availableLayers, ImagePlaneDesc* comps) const;

    /**
     * @brief Returns whether masking is enabled or not
     **/
    bool isMaskEnabled(int inputNb) const;

    /**
     * @brief Routine called after the creation of an effect. This function must
     * fill for the given input what image components we can feed it with.
     * This function is also called to specify what image components this effect can output.
     * In that case inputNb equals -1.
     **/
    virtual void addAcceptedComponents(int inputNb, std::list<ImagePlaneDesc>* comps) = 0;
    virtual void addSupportedBitDepth(std::list<ImageBitDepthEnum>* depths) const = 0;

    /**
     * @brief Must return the deepest bit depth that this plug-in can support.
     * If 32 float is supported then return eImageBitDepthFloat, otherwise
     * return eImageBitDepthShort if 16 bits is supported, and as a last resort, return
     * eImageBitDepthByte. At least one must be returned.
     **/
    ImageBitDepthEnum getBestSupportedBitDepth() const;

    bool isSupportedBitDepth(ImageBitDepthEnum depth) const;

    /**
     * @brief Returns true if the given input supports the given components. If inputNb equals -1
     * then this function will check whether the effect can produce the given components.
     **/
    bool isSupportedComponent(int inputNb, const ImagePlaneDesc & comp) const;

    /**
     * @brief Returns the most appropriate components that can be supported by the inputNb.
     * If inputNb equals -1 then this function will check the output components.
     **/
    ImagePlaneDesc findClosestSupportedComponents(int inputNb, const ImagePlaneDesc & comp) const WARN_UNUSED_RETURN;

    /**
     * @brief Can be derived to give a more meaningful label to the input 'inputNb'
     **/
    virtual std::string getInputLabel(int inputNb) const WARN_UNUSED_RETURN;

    /**
     * @brief Return a string indicating the purpose of the given input. It is used for the user documentation.
     **/
    virtual std::string getInputHint(int inputNb) const WARN_UNUSED_RETURN;

    /**
     * @brief Must be implemented to give the plugin internal id(i.e: net.sf.openfx:invertPlugin)
     **/
    virtual std::string getPluginID() const WARN_UNUSED_RETURN = 0;

    /**
     * @brief Must be implemented to give the plugin a label that will be used by the graphical
     * user interface.
     **/
    virtual std::string getPluginLabel() const WARN_UNUSED_RETURN = 0;

    /**
     * @brief The grouping of the plug-in. For instance Views/Stereo/MyStuff
     * Each string being one level of the grouping. The first one being the name
     * of one group that will appear in the user interface.
     **/
    virtual void getPluginGrouping(std::list<std::string>* grouping) const = 0;

    /**
     * @brief Must be implemented to give a description of the effect that this node does. This is typically
     * what you'll see displayed when the user clicks the '?' button on the node's panel in the user interface.
     **/
    virtual std::string getPluginDescription() const WARN_UNUSED_RETURN = 0;

    /**
     * @brief Returns whether the plugin  description is written in markdown or not
     **/
    virtual bool isPluginDescriptionInMarkdown() const
    {
        return false;
    }

    /**
     * @brief Must returns the shortcuts that are going to be used for this plug-in. Each shortcut
     * will be added to the shortcut editor and must have an ID and a description label.
     * Make sure that within the same plug-in there are no conflicting shortcuts.
     * Each shortcut ID can then be set to KnobButton used to indicate they have a shortcut.
     **/
    virtual void getPluginShortcuts(std::list<PluginActionShortcut>* /*shortcuts*/) {}


    /**
     * @bried Returns the effect render order preferences:
     * eSequentialPreferenceNotSequential: The effect does not need to be run in a sequential order
     * eSequentialPreferenceOnlySequential: The effect can only be run in a sequential order (i.e like the background render would do)
     * eSequentialPreferencePreferSequential: This indicates that the effect would work better by rendering sequential. This is merely
     * a hint to Natron but for now we just consider it as eSequentialPreferenceNotSequential.
     **/
    virtual SequentialPreferenceEnum getSequentialPreference() const
    {
        return eSequentialPreferenceNotSequential;
    }

    enum RenderRoIRetCode
    {
        eRenderRoIRetCodeOk = 0,
        eRenderRoIRetCodeAborted,
        eRenderRoIRetCodeFailed
    };

    /**
     * @brief Renders the image planes at the given time,scale and for the given view & render window.
     * This returns a list of all planes requested in the args.
     * @param args See the definition of the class for comments on each argument.
     * The return code indicates whether the render succeeded or failed. Note that this function may succeed
     * and return 0 plane if the RoI does not intersect the RoD of the effect.
     **/
    RenderRoIRetCode renderRoI(const RenderRoIArgs & args,
                               std::map<ImagePlaneDesc, ImagePtr>* outputPlanes) WARN_UNUSED_RETURN;


    void getImageFromCacheAndConvertIfNeeded(bool useCache,
                                             StorageModeEnum storage,
                                             StorageModeEnum returnStorage,
                                             const ImageKey & key,
                                             unsigned int mipmapLevel,
                                             const RectI* boundsParam,
                                             const RectD* rodParam,
                                             const RectI& roi,
                                             ImageBitDepthEnum bitdepth,
                                             const ImagePlaneDesc & components,
                                             const EffectInstance::InputImagesMap & inputImages,
                                             const RenderStatsPtr & stats,
                                             const OSGLContextAttacherPtr& glContextAttacher,
                                             ImagePtr* image);

    /**
     * @brief Converts the given OpenGL texture to a RAM-stored image. The resulting image will be cached.
     * This function is SLOW as it makes use of glReadPixels.
     * The OpenGL context should have been made current prior to calling this function.
     **/
    ImagePtr convertOpenGLTextureToCachedRAMImage(const ImagePtr& image);

    /**
     * @brief Converts the given RAM-stored image to an OpenGL texture.
     * THe resulting texture will not be cached and will destroyed when the shared pointer is released.
     * The OpenGL context should have been made current prior to calling this function.
     **/
    ImagePtr convertRAMImageToOpenGLTexture(const ImagePtr& image);


    /**
     * @brief This function is to be called by getImage() when the plug-ins renders more planes than the ones suggested
     * by the render action. We allocate those extra planes and cache them so they were not rendered for nothing.
     * Note that the plug-ins may call this only while in the render action, and there must be other planes to render.
     **/
    ImagePtr allocateImagePlaneAndSetInThreadLocalStorage(const ImagePlaneDesc & plane);


    class NotifyRenderingStarted_RAII
    {
        Node* _node;
        bool _didEmit;
        bool _didGroupEmit;

public:

        NotifyRenderingStarted_RAII(Node* node);

        ~NotifyRenderingStarted_RAII();
    };
    typedef std::shared_ptr<NotifyRenderingStarted_RAII> NotifyRenderingStarted_RAIIPtr;

    class NotifyInputNRenderingStarted_RAII
    {
        Node* _node;
        int _inputNumber;
        bool _didEmit;

public:

        NotifyInputNRenderingStarted_RAII(Node* node,
                                          int inputNumber);

        ~NotifyInputNRenderingStarted_RAII();
    };
    typedef std::shared_ptr<NotifyInputNRenderingStarted_RAII> NotifyInputNRenderingStarted_RAIIPtr;



    /**
     * @brief Sets render preferences for the rendering of a frame for the current thread.
     * This is thread-local storage. This is NOT local to a call to renderRoI
     **/
    void setParallelRenderArgsTLS(double time,
                                  ViewIdx view,
                                  bool isRenderUserInteraction,
                                  bool isSequential,
                                  U64 nodeHash,
                                  const AbortableRenderInfoPtr& abortInfo,
                                  const NodePtr & treeRoot,
                                  int visitsCount,
                                  const NodeFrameRequestPtr & nodeRequest,
                                  const OSGLContextPtr& glContext,
                                  int textureIndex,
                                  const TimeLine* timeline,
                                  bool isAnalysis,
                                  bool isDuringPaintStrokeCreation,
                                  const NodesList & rotoPaintNodes,
                                  RenderSafetyEnum currentThreadSafety,
                                  PluginOpenGLRenderSupport currentOpenGLSupport,
                                  bool doNanHandling,
                                  bool draftMode,
                                  const RenderStatsPtr & stats);

    void setDuringPaintStrokeCreationThreadLocal(bool duringPaintStroke);

    void setNodeRequestThreadLocal(const NodeFrameRequestPtr & nodeRequest);

    void setParallelRenderArgsTLS(const ParallelRenderArgsPtr & args);

    /**
     *@returns whether the effect was flagged with canSetValue = true or false
     **/
    void invalidateParallelRenderArgsTLS();

    ParallelRenderArgsPtr getParallelRenderArgsTLS() const;

    //Implem in ParallelRenderArgs.cpp
    static StatusEnum getInputsRoIsFunctor(bool useTransforms,
                                           double time,
                                           ViewIdx view,
                                           unsigned originalMipmapLevel,
                                           const NodePtr & node,
                                           const NodePtr& callerNode,
                                           const NodePtr & treeRoot,
                                           const RectD & canonicalRenderWindow,
                                           FrameRequestMap & requests);

    /**
     * @brief Visit recursively the compositing tree and computes required information about region of interests for each node and
     * for each frame/view pair. This helps to call render a single time per frame/view pair for a node.
     * Implem is in ParallelRenderArgs.cpp
     **/
    static StatusEnum computeRequestPass(double time,
                                         ViewIdx view,
                                         unsigned int mipmapLevel,
                                         const RectD & renderWindow,
                                         const NodePtr & treeRoot,
                                         FrameRequestMap & request);

    // Implem is in ParallelRenderArgs.cpp
    static EffectInstance::RenderRoIRetCode treeRecurseFunctor(bool isRenderFunctor,
                                                               const NodePtr & node,
                                                               const FramesNeededMap & framesNeeded,
                                                               const RoIMap & inputRois,
                                                               const InputMatrixMapPtr & reroutesMap,
                                                               bool useTransforms,         // roi functor specific
                                                               StorageModeEnum renderStorageMode, // The storage of the image returned by the current Render
                                                               unsigned int originalMipmapLevel,         // roi functor specific
                                                               double time,
                                                               ViewIdx view,
                                                               const NodePtr & treeRoot,
                                                               FrameRequestMap* requests,          // roi functor specific
                                                               EffectInstance::InputImagesMap* inputImages,         // render functor specific
                                                               const EffectInstance::ComponentsNeededMap* neededComps,         // render functor specific
                                                               bool useScaleOneInputs,         // render functor specific
                                                               bool byPassCache);         // render functor specific


    /**
     * @brief Don't override this one, override onKnobValueChanged instead.
     **/
    virtual bool onKnobValueChanged_public(KnobI* k, ValueChangedReasonEnum reason, double time, ViewSpec view, bool originatedFromMainThread) OVERRIDE FINAL;

    /**
     * @brief Returns a pointer to the first non disabled upstream node.
     * When cycling through the tree, we prefer non optional inputs and we span inputs
     * from last to first.
     * If this not is not disabled, it will return a pointer to this.
     **/
    EffectInstancePtr getNearestNonDisabled() const;

    /**
     * @brief Same as getNearestNonDisabled() except that it returns the *last* disabled node before the nearest non disabled node.
     * @param inputNb[out] The inputNb of the node that is non disabled.
     **/
    EffectInstancePtr getNearestNonDisabledPrevious(int* inputNb);

    /**
     * @brief Same as getNearestNonDisabled except that it looks for the nearest non identity node.
     * This function calls the action isIdentity and getRegionOfDefinition and can be expensive!
     **/
    EffectInstancePtr getNearestNonIdentity(double time);

    /**
     * @brief This is purely for the OfxEffectInstance derived class, but passed here for the sake of abstraction
     **/
    bool refreshMetadata_public(bool recurse);

    virtual void onChannelsSelectorRefreshed() {}

    void setDefaultMetadata();

protected:

    bool refreshMetadata_internal();

    bool setMetadataInternal(const NodeMetadata& metadata);

    virtual void onMetadataRefreshed(const NodeMetadata& /*metadata*/) {}

    bool refreshMetadata_recursive(std::list<Node*> & markedNodes);

    friend class ClipPreferencesRunning_RAII;
    void setClipPreferencesRunning(bool running);

public:


    ///////////////////////Metadatas related////////////////////////

    /**
     * @brief Returns the preferred metadata to render with
     * This should only be called to compute the clip preferences, call the appropriate
     * getters to get the actual values.
     * The default implementation gives reasonable values appropriate to the context of the node (the inputs)
     * and the values reported by the supported components/bitdepth
     *
     * This should not be reimplemented except for OpenFX which already has its default specification
     * for clip Preferences, see setDefaultClipPreferences()
     * Returns eStatusOK on success, eStatusFailed on failure.
     *
     **/
    StatusEnum getPreferredMetadata_public(NodeMetadata& metadata);

protected:

    virtual StatusEnum getPreferredMetadata(NodeMetadata& /*metadata*/) { return eStatusOK; }

private:

    StatusEnum getDefaultMetadata(NodeMetadata& metadata);

public:



    /**
     * @brief Returns whether the effect is frame-varying (i.e: a Reader with different images in the sequence)
     **/
    bool isFrameVarying() const;

    /**
     * @brief Returns whether the current node and/or the tree upstream is frame varying or animated.
     * It is frame varying/animated if at least one of the node is animated/varying
     **/
    bool isFrameVaryingOrAnimated_Recursive() const;

    /**
     * @brief Returns the preferred output frame rate to render with
     **/
    double getFrameRate() const;

    /**
     * @brief Returns the preferred premultiplication flag for the output image
     **/
    ImagePremultiplicationEnum getPremult() const;

    /**
     * @brief If true, the plug-in knows how to render frames at non integer times. If false
     * this is the hint indicating that the plug-ins can only render integer frame times (such as a Reader)
     **/
    bool canRenderContinuously() const;

    /**
     * @brief Returns the field ordering of images produced by this plug-in
     **/
    ImageFieldingOrderEnum getFieldingOrder() const;

    /**
     * @brief Returns the pixel aspect ratio, depth and components for the given input.
     * If inputNb equals -1 then this function will check the output components.
     **/
    double getAspectRatio(int inputNb) const;
    void getMetadataComponents(int inputNb, ImagePlaneDesc* plane, ImagePlaneDesc* pairedPlane) const;
    int getMetadataNComps(int inputNb) const;

    ImageBitDepthEnum getBitDepth(int inputNb) const;


    ///////////////////////End Metadatas related////////////////////////


    virtual void lock(const std::shared_ptr<Image> & entry) OVERRIDE FINAL;
    virtual bool tryLock(const std::shared_ptr<Image> & entry) OVERRIDE FINAL;
    virtual void unlock(const std::shared_ptr<Image> & entry) OVERRIDE FINAL;
    virtual bool canSetValue() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void abortAnyEvaluation(bool keepOldestRender = true) OVERRIDE FINAL;
    virtual double getCurrentTime() const OVERRIDE WARN_UNUSED_RETURN;
    virtual ViewIdx getCurrentView() const OVERRIDE WARN_UNUSED_RETURN;
    virtual bool getCanTransform() const
    {
        return false;
    }

    RenderScale getOverlayInteractRenderScale() const;

    SequenceTime getFrameRenderArgsCurrentTime() const;

    ViewIdx getFrameRenderArgsCurrentView() const;

    virtual bool getInputsHoldingTransform(std::list<int>* /*inputs*/) const
    {
        return false;
    }

    bool getThreadLocalRegionsOfInterests(RoIMap & roiMap) const;

    OSGLContextPtr getThreadLocalOpenGLContext() const;

    void getThreadLocalInputImages(InputImagesMap* images) const;

    void addThreadLocalInputImageTempPointer(int inputNb, const ImagePtr & img);

    bool getThreadLocalRotoPaintTreeNodes(NodesList* nodes) const;


    virtual bool isMultiPlanar() const
    {
        return false;
    }

    enum PassThroughEnum
    {
        ePassThroughBlockNonRenderedPlanes,
        ePassThroughPassThroughNonRenderedPlanes,
        ePassThroughRenderAllRequestedPlanes
    };
