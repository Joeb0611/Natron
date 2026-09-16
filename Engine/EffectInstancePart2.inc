// Engine/EffectInstancePart2.inc — included from EffectInstance.h (class body).
// Split so GitHub Contents/MCP writes stay under the 100k cap.

    virtual EffectInstance::PassThroughEnum isPassThroughForNonRenderedPlanes() const
    {
        return ePassThroughPassThroughNonRenderedPlanes;
    }

    virtual bool isViewAware() const
    {
        return false;
    }

    enum ViewInvarianceLevel
    {
        eViewInvarianceAllViewsVariant,
        eViewInvarianceOnlyPassThroughPlanesVariant,
        eViewInvarianceAllViewsInvariant,
    };

    virtual ViewInvarianceLevel isViewInvariant() const
    {
        return eViewInvarianceAllViewsVariant;
    }

    class OpenGLContextEffectData
    {
        // True if we did not unlock the context mutex in attachOpenGLContext()
        bool hasTakenLock;

public:

        OpenGLContextEffectData()
            : hasTakenLock(false)
        {
        }

        void setHasTakenLock(bool b)
        {
            hasTakenLock = b;
        }

        bool getHasTakenLock() const
        {
            return hasTakenLock;
        }

        virtual ~OpenGLContextEffectData()
        {
        }
    };

    typedef std::shared_ptr<OpenGLContextEffectData> OpenGLContextEffectDataPtr;
    typedef std::map<OSGLContextWPtr, OpenGLContextEffectDataPtr, std::owner_less<OSGLContextWPtr>> OpenGLContextEffectsMap;


    struct RenderActionArgs
    {
        double time;
        RenderScale originalScale;
        RenderScale mappedScale;
        RectI roi;
        std::list<std::pair<ImagePlaneDesc, ImagePtr> > outputPlanes;
        EffectInstance::InputImagesMap inputImages;
        ViewIdx view;
        bool isSequentialRender;
        bool isRenderResponseToUserInteraction;
        bool byPassCache;
        bool draftMode;
        bool useOpenGL;
        EffectInstance::OpenGLContextEffectDataPtr glContextData;
        std::bitset<4> processChannels;
    };

protected:
    /**
     * @brief Must fill the image 'output' for the region of interest 'roi' at the given time and
     * at the given scale.
     * Pre-condition: render() has been called for all inputs so the portion of the image contained
     * in output corresponding to the roi is valid.
     * Note that this function can be called concurrently for the same output image but with different
     * rois, depending on the threading-affinity of the plug-in.
     **/
    virtual StatusEnum render(const RenderActionArgs & /*args*/) WARN_UNUSED_RETURN
    {
        return eStatusOK;
    }

    virtual StatusEnum getTransform(double /*time*/,
                                    const RenderScale & /*renderScale*/,
                                    bool /*draftRender*/,
                                    ViewIdx /*view*/,
                                    EffectInstancePtr* /*inputToTransform*/,
                                    Transform::Matrix3x3* /*transform*/) WARN_UNUSED_RETURN
    {
        return eStatusReplyDefault;
    }

public:


    StatusEnum render_public(const RenderActionArgs & args) WARN_UNUSED_RETURN;
    StatusEnum getTransform_public(double time,
                                   const RenderScale & renderScale,
                                   bool draftRender,
                                   ViewIdx view,
                                   EffectInstancePtr* inputToTransform,
                                   Transform::Matrix3x3* transform) WARN_UNUSED_RETURN;

protected:
/**
 * @brief Can be overloaded to indicates whether the effect is an identity, i.e it doesn't produce
 * any change in output.
 * @param time The time of interest
 * @param scale The scale of interest
 * @param rod The image region of definition, in canonical coordinates
 * @param view The view we 're interested in
 * @param inputTime[out] the input time to which this plugin is identity of
 * @param inputNb[out] the input number of the effect that is identity of.
 * The special value of -2 indicates that the plugin is identity of itself at another time
 **/
    virtual bool isIdentity(double /*time*/,
                            const RenderScale & /*scale*/,
                            const RectI & /*roi*/,
                            ViewIdx /*view*/,
                            double* /*inputTime*/,
                            ViewIdx* /*inputView*/,
                            int* /*inputNb*/) WARN_UNUSED_RETURN
    {
        return false;
    }

public:


    bool isIdentity_public(bool useIdentityCache, // only set to true when calling for the whole image (not for a subrect)
                           U64 hash,
                           double time,
                           const RenderScale & scale,
                           const RectI & renderWindow,
                           ViewIdx view,
                           double* inputTime,
                           ViewIdx* inputView,
                           int* inputNb) WARN_UNUSED_RETURN;

    /**
     * @brief Indicates how many simultaneous renders the plugin can deal with.
     * RenderSafetyEnum::eRenderSafetyUnsafe - indicating that only a single 'render' call can be made at any time among all instances,
     * RenderSafetyEnum::eRenderSafetyInstanceSafe - indicating that any instance can have a single 'render' call at any one time,
     * RenderSafetyEnum::eRenderSafetyFullySafe - indicating that any instance of a plugin can have multiple renders running simultaneously
     * RenderSafetyEnum::eRenderSafetyFullySafeFrame - Same as eRenderSafetyFullySafe but the plug-in also flagged  kOfxImageEffectPluginPropHostFrameThreading to true.
     **/
    virtual RenderSafetyEnum renderThreadSafety() const WARN_UNUSED_RETURN = 0;

    /*@brief The derived class should query this to abort any long process
       in the engine function.*/
    bool aborted() const WARN_UNUSED_RETURN;

    /**
     * @brief Test hooks for abort/trimap wait (#248). Hold a trimap mark so another
     * thread can wait, then abort must unwind instead of hanging at CPU-zero.
     **/
    void markImageAsBeingRenderedForTests(const ImagePtr & img, const RectI& roi);
    bool waitForImageBeingRenderedElsewhereForTests(const RectI & roi, const ImagePtr & img);
    void unmarkImageAsBeingRenderedForTests(const ImagePtr & img, bool renderFailed);


    /** @brief Returns the image computed by the input 'inputNb' at the given time and scale for the given view.
     * @param dontUpscale If the image is retrieved is downscaled but the plug-in doesn't support the user of
     * downscaled images by default we upscale the image. If dontUpscale is true then we don't do this upscaling.
     *
     * @param roiPixel If non NULL will be set to the render window used to render the image, that is, either the
     * region of interest of this effect on the input effect we want to render or the optionalBounds if set, but
     * converted to pixel coordinates
     */
    ImagePtr getImage(int inputNb,
                                      const double time,
                                      const RenderScale & scale,
                                      const ViewIdx view,
                                      const RectD *optionalBounds, //!< optional region in canonical coordinates
                                      const ImagePlaneDesc* layer, //< if set, fetch this specific layer, otherwise use what's in the clip pref
                                      const bool mapToClipPrefs,
                                      const bool dontUpscale,
                                      const StorageModeEnum returnStorage,
                                      const ImageBitDepthEnum* textureDepth,
                                      RectI* roiPixel,
                                      Transform::Matrix3x3Ptr* transform = 0) WARN_UNUSED_RETURN;
    virtual void aboutToRestoreDefaultValues() OVERRIDE FINAL;
    virtual bool shouldCacheOutput(bool isFrameVaryingOrAnimated, double time, ViewIdx view, int visitsCount) const;

    /**
     * @brief Can be derived to get the region that the plugin is capable of filling.
     * This is meaningful for plugins that generate images or transform images.
     * By default it returns in rod the union of all inputs RoD and eStatusReplyDefault is returned.
     * @param isProjectFormat[out] If set to true, then rod is taken to be equal to the current project format.
     * In case of failure the plugin should return eStatusFailed.
     * @returns eStatusOK, eStatusReplyDefault, or eStatusFailed. rod is set except if return value is eStatusOK or eStatusReplyDefault.
     **/
    virtual StatusEnum getRegionOfDefinition(U64 hash, double time, const RenderScale & scale, ViewIdx view, RectD* rod) WARN_UNUSED_RETURN;

protected:


    virtual void calcDefaultRegionOfDefinition(U64 hash, double time, const RenderScale & scale, ViewIdx view, RectD *rod);

    /**
     * @brief If the instance rod is infinite, returns the union of all connected inputs. If there's no input this returns the
     * project format.
     * @returns true if the rod is set to the project format.
     **/
    bool ifInfiniteApplyHeuristic(U64 hash,
                                  double time,
                                  const RenderScale & scale,
                                  ViewIdx view,
                                  RectD* rod); //!< input/output

    /**
     * @brief Can be derived to indicate for each input node what is the region of interest
     * of the node at time 'time' and render scale 'scale' given a render window.
     * For example a blur plugin would specify what it needs
     * from inputs in order to do a blur taking into account the size of the blurring kernel.
     * By default, it returns renderWindow for each input.
     **/
    virtual void getRegionsOfInterest(double time,
                                      const RenderScale & scale,
                                      const RectD & outputRoD,   //!< the RoD of the effect, in canonical coordinates
                                      const RectD & renderWindow,   //!< the region to be rendered in the output image, in Canonical Coordinates
                                      ViewIdx view,
                                      RoIMap* ret);

    /**
     * @brief Can be derived to indicate for each input node what is the frame range(s) (which can be discontinuous)
     * that this effects need in order to render the frame at the given time.
     **/
    virtual FramesNeededMap getFramesNeeded(double time, ViewIdx view) WARN_UNUSED_RETURN;


    /**
     * @brief Can be derived to get the frame range wherein the plugin is capable of producing frames.
     * By default it merges the frame range of the inputs.
     * In case of failure the plugin should return eStatusFailed.
     **/
    virtual void getFrameRange(double *first, double *last);

    /**
     * @brief Returns true if the value of renderScale is supported for the given the value of supportsRS.
     **/
    static bool isSupportedRenderScale(SupportsEnum supportsRS, const RenderScale renderScale)
    {
        return (supportsRS != eSupportsNo) || (renderScale == RenderScale::identity);
    }

public:

    StatusEnum getRegionOfDefinition_public(U64 hash,
                                            double time,
                                            const RenderScale & scale,
                                            ViewIdx view,
                                            RectD* rod,
                                            bool* isProjectFormat) WARN_UNUSED_RETURN;

    StatusEnum getRegionOfDefinitionFromCache(U64 hash,
                                              double time,
                                              const RenderScale & scale,
                                              ViewIdx view,
                                              RectD* rod,
                                              bool* isProjectFormat) WARN_UNUSED_RETURN;

public:


    void getRegionsOfInterest_public(double time,
                                     const RenderScale & scale,
                                     const RectD & outputRoD,
                                     const RectD & renderWindow,   //!< the region to be rendered in the output image, in Canonical Coordinates
                                     ViewIdx view,
                                     RoIMap* ret);

    FramesNeededMap getFramesNeeded_public(U64 hash, double time, ViewIdx view, unsigned int mipmapLevel) WARN_UNUSED_RETURN;

    void getFrameRange_public(U64 hash, double *first, double *last, bool bypasscache = false);

    /**
     * @brief Override to initialize the overlay interact. It is called only on the
     * live instance.
     **/
    virtual void initializeOverlayInteract()
    {
    }

    void setCurrentViewportForOverlays_public(OverlaySupport* viewport);
    OverlaySupport* getCurrentViewportForOverlays() const;

protected:

    virtual void setCurrentViewportForOverlays(OverlaySupport* /*viewport*/)
    {
    }

public:


    /**
     * @brief Overload this and return true if your operator should display a preview image by default.
     **/
    virtual bool makePreviewByDefault() const WARN_UNUSED_RETURN
    {
        return isReader();
    }

    /**
     * @brief Called on generator effects upon creation if they have an image input file field.
     **/
    void openImageFileKnob();


    /**
     * @brief Used to bracket a series of call to onKnobValueChanged(...) in case many complex changes are done
     * at once. If not called, onKnobValueChanged() will call automatically bracket its call be a begin/end
     * but this can lead to worse performance. You can overload this to make all changes to params at once.
     **/
    virtual void beginKnobsValuesChanged(ValueChangedReasonEnum /*reason*/) OVERRIDE
    {
    }

    /**
     * @brief Used to bracket a series of call to onKnobValueChanged(...) in case many complex changes are done
     * at once. If not called, onKnobValueChanged() will call automatically bracket its call be a begin/end
     * but this can lead to worse performance. You can overload this to make all changes to params at once.
     **/
    virtual void endKnobsValuesChanged(ValueChangedReasonEnum /*reason*/) OVERRIDE
    {
    }

    /**
     * @brief Can be overloaded to clear any cache the plugin might be
     * handling on his side.
     **/
    virtual void purgeCaches()
    {
    };
    virtual void clearLastRenderedImage();

    void clearActionsCache();

    /**
     * @brief Use this function to post a transient message to the user. It will be displayed using
     * a dialog. The message can be of 4 types...
     * INFORMATION_MESSAGE : you just want to inform the user about something.
     * eMessageTypeWarning : you want to inform the user that something important happened.
     * eMessageTypeError : you want to inform the user an error occurred.
     * eMessageTypeQuestion : you want to ask the user about something.
     * The function will return true always except for a message of type eMessageTypeQuestion, in which
     * case the function may return false if the user pressed the 'No' button.
     * @param content The message you want to pass.
     **/
    bool message(MessageTypeEnum type, const std::string & content) const;

    /**
     * @brief Use this function to post a persistent message to the user. It will be displayed on the
     * node's graphical interface and on any connected viewer. The message can be of 3 types...
     * INFORMATION_MESSAGE : you just want to inform the user about something.
     * eMessageTypeWarning : you want to inform the user that something important happened.
     * eMessageTypeError : you want to inform the user an error occurred.
     * @param content The message you want to pass.
     **/
    void setPersistentMessage(MessageTypeEnum type, const std::string & content);

    /**
     * @brief Test if a persistent message is set.
     **/
    bool hasPersistentMessage();

    /**
     * @brief Clears any message posted previously by setPersistentMessage.
     **/
    void clearPersistentMessage(bool recurse);

    /**
     * @brief Does this effect supports tiling ?
     * http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#kOfxImageEffectPropSupportsTiles
     * If a clip or plugin does not support tiled images, then the host should supply
     * full RoD images to the effect whenever it fetches one.
     **/
    virtual bool supportsTiles() const
    {
        return false;
    }

    /**
     * @brief Does this effect supports multiresolution ?
     * http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#kOfxImageEffectPropSupportsMultiResolution
     * Multiple resolution images mean...
     * input and output images can be of any size
     * input and output images can be offset from the origin
     **/
    virtual bool supportsMultiResolution() const
    {
        return true;
    }

    /**
     * @brief Does this effect supports rendering at a different scale than 1 ?
     * There is no OFX property for this purpose. The only solution found for OFX is that if a render
     * or isIdentity with renderscale != 1 fails, the host retries with renderscale = 1 (and upscaled images).
     * If the renderScale support was not set, this throws an exception.
     **/
    bool supportsRenderScale() const;

    SupportsEnum supportsRenderScaleMaybe() const;

    /// should be set during effect initialization, but may also be set by the first getRegionOfDefinition with scale != 1 that succeeds
    void setSupportsRenderScaleMaybe(EffectInstance::SupportsEnum s) const;

    virtual bool supportsRenderQuality() const { return false; }

    /**
     * @brief Does this effect can support multiple clips PAR ?
     * http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#kOfxImageEffectPropSupportsMultipleClipPARs
     * If a plugin does not accept clips of differing PARs, then the host must resample all images fed to that effect to agree with the output's PAR.
     * If a plugin does accept clips of differing PARs, it will need to specify the output clip's PAR in the kOfxImageEffectActionGetClipPreferences action.
     **/
    virtual bool supportsMultipleClipPARs() const
    {
        return false;
    }

    virtual bool supportsMultipleClipDepths() const
    {
        return false;
    }

    virtual bool supportsMultipleClipFPSs() const
    {
        return false;
    }

    virtual bool doesTemporalClipAccess() const
    {
        return false;
    }

    virtual PluginOpenGLRenderSupport supportsOpenGLRender() const
    {
        return ePluginOpenGLRenderSupportNone;
    }

    /**
     * @brief If this effect is a writer then the file path corresponding to the output images path will be fed
     * with the content of pattern.
     **/
    void setOutputFilesForWriter(const std::string & pattern);

    /**
     * @brief Constructs a new memory holder, with nBytes allocated. If the allocation failed, bad_alloc is thrown
     **/
    PluginMemoryPtr newMemoryInstance(size_t nBytes) WARN_UNUSED_RETURN;

    /// used to count the memory used by a plugin
    /// Don't call these, they're called by PluginMemory automatically
    void registerPluginMemory(size_t nBytes);
    void unregisterPluginMemory(size_t nBytes);

    void addPluginMemoryPointer(const PluginMemoryPtr& mem);
    void removePluginMemoryPointer(const PluginMemory* mem);

    void clearPluginMemoryChunks();

    /**
     * @brief Called right away when the user first opens the settings panel of the node.
     * This is called after each params had its default value set.
     **/
    virtual void beginEditKnobs()
    {
    }

    /**
     * @brief Called everytimes an input connection is changed
     **/
    virtual void onInputChanged(int inputNo);

    /**
     * @brief If the plug-in calls timelineGoTo and we're during a render/instance changed action,
     * then all the knobs will retrieve the current time as being the one in the last render args thread-storage.
     * This function is here to update the last render args thread storage.
     **/
    void updateThreadLocalRenderTime(double time);

    bool isDuringPaintStrokeCreationThreadLocal() const;

    struct PlaneToRender
    {
        //Points to the fullscale image if render scale is not supported by the plug-in, or downscaleImage otherwise
        ImagePtr fullscaleImage;

        //Points to the image to be rendered
        ImagePtr downscaleImage;

        //Points to the image that the plug-in can render (either fullScale or downscale)
        ImagePtr renderMappedImage;

        //Points to a temporary image that the plug-in will render
        ImagePtr tmpImage;

        /*
           In the event where the fullScaleImage is in the cache but we must resize it to render a portion unallocated yet and
           if the render is issues directly from getImage() we swap image in cache instead of taking the write lock of fullScaleImage
         */
        ImagePtr cacheSwapImage;
        void* originalCachedImage;

        /**
         * This is set to true if this plane is allocated with allocateImagePlaneAndSetInThreadLocalStorage()
         **/
        bool isAllocatedOnTheFly;

        PlaneToRender()
            : fullscaleImage()
            , downscaleImage()
            , renderMappedImage()
            , tmpImage()
            , cacheSwapImage()
            , originalCachedImage(0)
            , isAllocatedOnTheFly(false)
        {
        }
    };

    struct RectToRender
    {
        EffectInstancePtr identityInput;
        RectI rect;
        RoIMap inputRois;
        EffectInstance::InputImagesMap imgs;
        double identityTime;
        bool isIdentity;
        ViewIdx identityView;
    };

    struct ImagePlanesToRender
    {
        std::list<RectToRender> rectsToRender;
        std::map<ImagePlaneDesc, PlaneToRender> planes;
        std::map<int, ImagePremultiplicationEnum> inputPremult;
        ImagePremultiplicationEnum outputPremult;
        bool useOpenGL;
        EffectInstance::OpenGLContextEffectDataPtr glContextData;

        ImagePlanesToRender()
            : rectsToRender()
            , planes()
            , inputPremult()
            , outputPremult(eImagePremultiplicationPremultiplied)
            , useOpenGL(false)
            , glContextData()
        {
        }
    };

    typedef std::shared_ptr<ImagePlanesToRender> ImagePlanesToRenderPtr;

    /**
     * @brief If the caller thread is currently rendering an image, it will return a pointer to it
     * otherwise it will return NULL.
     * This function also returns the current renderWindow that is being rendered on that image
     * To be called exclusively on a render thread.
     *
     * WARNING: This call isexpensive and this function should not be called many times.
     **/
    bool getThreadLocalRenderedPlanes(std::map<ImagePlaneDesc, EffectInstance::PlaneToRender >*  planes,
                                      ImagePlaneDesc* planeBeingRendered,
                                      RectI* renderWindow) const;

    bool getThreadLocalNeededComponents(ComponentsNeededMapPtr* neededComps) const;

    /**
     * @brief Called when the associated node's hash has changed.
     * This is always called on the main-thread.
     **/
    void onNodeHashChanged(U64 hash);


    virtual void initializeData()
    {
    }

#ifdef DEBUG
    void checkCanSetValueAndWarn() const;
#endif

protected:


#ifdef DEBUG

    /*
       Debug helper to track plug-in that do setValue calls that are forbidden

       http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#SettingParams
       Officially, setValue calls are allowed during the following actions:

       The Create Instance Action
       The The Begin Instance Changed Action
       The The Instance Changed Action
       The The End Instance Changed Action
       The The Sync Private Data Action


     */

    void setCanSetValue(bool can);

    void invalidateCanSetValueFlag();


    bool isDuringActionThatCanSetValue() const;

    class CanSetSetValueFlag_RAII
    {
        EffectInstance* effect;

public:

        CanSetSetValueFlag_RAII(EffectInstance* effect,
                                bool canSetValue)
            : effect(effect)
        {
            effect->setCanSetValue(canSetValue);
        }

        ~CanSetSetValueFlag_RAII()
        {
            effect->invalidateCanSetValueFlag();
        }
    };

    bool checkCanSetValue() const
    {
        return isDuringActionThatCanSetValue();
    }

#define SET_CAN_SET_VALUE(canSetValue) EffectInstance::CanSetSetValueFlag_RAII canSetValueSetter(this, canSetValue)
#else
#define SET_CAN_SET_VALUE(canSetValue) ( (void)0 )
#endif // DEBUG
