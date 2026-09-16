bool
Settings::onKnobValueChanged(KnobI* k,
                             ValueChangedReasonEnum reason,
                             double /*time*/,
                             ViewSpec /*view*/,
                             bool /*originatedFromMainThread*/)
{
    Q_EMIT settingChanged(k);
    bool ret = true;

    if ( k == _maxViewerDiskCacheGB.get() ) {
        if (!_restoringSettings) {
            appPTR->setApplicationsCachesMaximumViewerDiskSpace( getMaximumViewerDiskCacheSize() );
        }
    } else if ( k == _maxDiskCacheNodeGB.get() ) {
        if (!_restoringSettings) {
            appPTR->setApplicationsCachesMaximumDiskSpace( getMaximumDiskCacheNodeSize() );
        }
    } else if ( k == _maxRAMPercent.get() ) {
        if (!_restoringSettings) {
            appPTR->setApplicationsCachesMaximumMemoryPercent( getRamMaximumPercent() );
        }
        setCachingLabels();
    } else if ( k == _diskCachePath.get() ) {
        QString path = QString::fromUtf8(_diskCachePath->getValue().c_str());
        qputenv(NATRON_DISK_CACHE_PATH_ENV_VAR, path.toUtf8());
        appPTR->refreshDiskCacheLocation();
    } else if ( k == _wipeDiskCache.get() ) {
        appPTR->wipeAndCreateDiskCacheStructure();
    } else if ( k == _numberOfThreads.get() ) {
        int nbThreads = getNumberOfThreads();
        appPTR->setNThreadsToRender(nbThreads);
        if (nbThreads == -1) {
            QThreadPool::globalInstance()->setMaxThreadCount(1);
            appPTR->abortAnyProcessing();
        } else if (nbThreads == 0) {
            // See https://github.com/NatronGitHub/Natron/issues/554
            // min(num_cores, RAM/3.5Gb)
            int maxThread = getSystemTotalRAM() / ( (3ULL * 1024ULL + 512ULL) * 1024ULL * 1024ULL );
            maxThread = std::min( std::max(1, maxThread), QThread::idealThreadCount() );
            QThreadPool::globalInstance()->setMaxThreadCount(maxThread);
        } else {
            QThreadPool::globalInstance()->setMaxThreadCount(nbThreads);
        }
    } else if ( k == _nThreadsPerEffect.get() ) {
        appPTR->setNThreadsPerEffect( getNumberOfThreadsPerEffect() );
    } else if ( k == _ocioConfigKnob.get() ) {
        if (_ocioConfigKnob->getActiveEntry().id == NATRON_CUSTOM_OCIO_CONFIG_NAME) {
            _customOcioConfigFile->setAllDimensionsEnabled(true);
        } else {
            _customOcioConfigFile->setAllDimensionsEnabled(false);
        }
        tryLoadOpenColorIOConfig();
    } else if ( k == _useThreadPool.get() ) {
        bool useTP = _useThreadPool->getValue();
        appPTR->setUseThreadPool(useTP);
    } else if ( k == _customOcioConfigFile.get() ) {
        if ( _customOcioConfigFile->isEnabled(0) ) {
            tryLoadOpenColorIOConfig();
            bool warnOcioChanged = _warnOcioConfigKnobChanged->getValue();
            if ( warnOcioChanged && appPTR->getTopLevelInstance() ) {
                bool stopAsking = false;
                Dialogs::warningDialog(tr("OCIO config changed").toStdString(),
                                       tr("The OpenColorIO config change requires a restart of %1 to be effective.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString(), &stopAsking);
                if (stopAsking) {
                    _warnOcioConfigKnobChanged->setValue(false);
                }
            }
        }
    } else if ( k == _maxUndoRedoNodeGraph.get() ) {
        appPTR->setUndoRedoStackLimit( _maxUndoRedoNodeGraph->getValue() );
    } else if ( k == _maxPanelsOpened.get() ) {
        appPTR->onMaxPanelsOpenedChanged( _maxPanelsOpened->getValue() );
    } else if ( k == _queueRenders.get() ) {
        appPTR->onQueueRendersChanged( _queueRenders->getValue() );
    } else if ( ( k == _checkerboardTileSize.get() ) || ( k == _checkerboardColor1.get() ) || ( k == _checkerboardColor2.get() ) ) {
        appPTR->onCheckerboardSettingsChanged();
    } else if ( k == _powerOf2Tiling.get() && !_restoringSettings) {
        appPTR->onViewerTileCacheSizeChanged();
    } else if ( k == _texturesMode.get() &&  !_restoringSettings) {
         appPTR->onViewerTileCacheSizeChanged();
    } else if ( ( k == _hideOptionalInputsAutomatically.get() ) && !_restoringSettings && (reason == eValueChangedReasonUserEdited) ) {
        appPTR->toggleAutoHideGraphInputs();
    } else if ( k == _autoProxyWhenScrubbingTimeline.get() ) {
        _autoProxyLevel->setSecret( !_autoProxyWhenScrubbingTimeline->getValue() );
    } else if ( !_restoringSettings &&
                ( ( k == _sunkenColor.get() ) ||
                  ( k == _baseColor.get() ) ||
                  ( k == _raisedColor.get() ) ||
                  ( k == _selectionColor.get() ) ||
                  ( k == _textColor.get() ) ||
                  ( k == _altTextColor.get() ) ||
                  ( k == _timelinePlayheadColor.get() ) ||
                  ( k == _timelineBoundsColor.get() ) ||
                  ( k == _timelineBGColor.get() ) ||
                  ( k == _interpolatedColor.get() ) ||
                  ( k == _keyframeColor.get() ) ||
                  ( k == _trackerKeyframeColor.get() ) ||
                  ( k == _cachedFrameColor.get() ) ||
                  ( k == _diskCachedFrameColor.get() ) ||
                  ( k == _curveEditorBGColor.get() ) ||
                  ( k == _gridColor.get() ) ||
                  ( k == _curveEditorScaleColor.get() ) ||
                  ( k == _dopeSheetEditorBackgroundColor.get() ) ||
                  ( k == _dopeSheetEditorRootSectionBackgroundColor.get() ) ||
                  ( k == _dopeSheetEditorKnobSectionBackgroundColor.get() ) ||
                  ( k == _dopeSheetEditorScaleColor.get() ) ||
                  ( k == _dopeSheetEditorGridColor.get() ) ||
                  ( k == _keywordColor.get() ) ||
                  ( k == _operatorColor.get() ) ||
                  ( k == _curLineColor.get() ) ||
                  ( k == _braceColor.get() ) ||
                  ( k == _defClassColor.get() ) ||
                  ( k == _stringsColor.get() ) ||
                  ( k == _commentsColor.get() ) ||
                  ( k == _selfColor.get() ) ||
                  ( k == _sliderColor.get() ) ||
                  ( k == _numbersColor.get() ) ) ) {
        appPTR->reloadStylesheets();
    } else if ( k == _qssFile.get() ) {
        appPTR->reloadStylesheets();
    } else if ( k == _hostName.get() ) {
        std::string hostName = _hostName->getActiveEntry().id;
        bool isCustom = hostName == NATRON_CUSTOM_HOST_NAME_ENTRY;
        _customHostName->setSecret(!isCustom);
#ifdef NATRON_USE_BREAKPAD
    } else if ( ( k == _testCrashReportButton.get() ) && (reason == eValueChangedReasonUserEdited) ) {
        StandardButtonEnum reply = Dialogs::questionDialog( tr("Crash Test").toStdString(),
                                                            tr("You are about to make %1 crash to test the reporting system.\n"
                                                               "Do you really want to crash?").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString(), false,
                                                            StandardButtons(eStandardButtonYes | eStandardButtonNo) );
        if (reply == eStandardButtonYes) {
            crash_application();
        }
#endif
    } else if ( ( k == _scriptEditorFontChoice.get() ) || ( k == _scriptEditorFontSize.get() ) ) {
        appPTR->reloadScriptEditorFonts();
    } else if ( k == _pluginUseImageCopyForSource.get() ) {
        appPTR->setPluginsUseInputImageCopyToRender( _pluginUseImageCopyForSource->getValue() );
    } else if ( k == _enableOpenGL.get() ) {
        appPTR->refreshOpenGLRenderingFlagOnAllInstances();
        if (!_restoringSettings) {
            appPTR->clearPluginsLoadedCache();
        }
    } else {
        ret = false;
    }
    if (ret) {
        if ( ( ( k == _hostName.get() ) || ( k == _customHostName.get() ) ) && !_restoringSettings ) {
            Dialogs::warningDialog( tr("Host-name change").toStdString(), tr("Changing this requires a restart of %1 and clearing the OpenFX plug-ins load cache from the Cache menu.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString() );
        }
    }

    return ret;
} // onKnobValueChanged

////////////////////////////////////////////////////////
// "Viewers" pane

ImageBitDepthEnum
Settings::getViewersBitDepth() const
{
    if (!appPTR->isTextureFloatSupported()) {
        return eImageBitDepthByte;
    }
    int v = _texturesMode->getValue();

    if (v == 0) {
        return eImageBitDepthByte;
    } else if (v == 1) {
        return eImageBitDepthFloat;
    } else {
        return eImageBitDepthByte;
    }
}

int
Settings::getViewerTilesPowerOf2() const
{
    return _powerOf2Tiling->getValue();
}

int
Settings::getCheckerboardTileSize() const
{
    return _checkerboardTileSize->getValue();
}

void
Settings::getCheckerboardColor1(double* r,
                                double* g,
                                double* b,
                                double* a) const
{
    *r = _checkerboardColor1->getValue(0);
    *g = _checkerboardColor1->getValue(1);
    *b = _checkerboardColor1->getValue(2);
    *a = _checkerboardColor1->getValue(3);
}

void
Settings::getCheckerboardColor2(double* r,
                                double* g,
                                double* b,
                                double* a) const
{
    *r = _checkerboardColor2->getValue(0);
    *g = _checkerboardColor2->getValue(1);
    *b = _checkerboardColor2->getValue(2);
    *a = _checkerboardColor2->getValue(3);
}

bool
Settings::isAutoWipeEnabled() const
{
    return _autoWipe->getValue();
}

bool
Settings::isAutoProxyEnabled() const
{
    return _autoProxyWhenScrubbingTimeline->getValue();
}

unsigned int
Settings::getAutoProxyMipmapLevel() const
{
    return (unsigned int)_autoProxyLevel->getValue() + 1;
}

int
Settings::getMaxOpenedNodesViewerContext() const
{
    return _maximumNodeViewerUIOpened->getValue();
}

bool
Settings::viewerNumberKeys() const
{
    return _viewerNumberKeys->getValue();
}

bool
Settings::viewerOverlaysPath() const
{
    return _viewerOverlaysPath->getValue();
}

///////////////////////////////////////////////////////
// "Caching" pane

bool
Settings::isAggressiveCachingEnabled() const
{
    return _aggressiveCaching->getValue();
}

double
Settings::getRamMaximumPercent() const
{
    return (double)_maxRAMPercent->getValue() / 100.;
}

U64
Settings::getMaximumViewerDiskCacheSize() const
{
    return (U64)( _maxViewerDiskCacheGB->getValue() ) * 1024 * 1024 * 1024;
}

U64
Settings::getMaximumDiskCacheNodeSize() const
{
    return (U64)( _maxDiskCacheNodeGB->getValue() ) * 1024 * 1024 * 1024;
}

U64
Settings::getMinimumFreeDiskBytes() const
{
    return (U64)( _minFreeDiskGiB->getValue() ) * 1024ULL * 1024ULL * 1024ULL;
}

void
Settings::setMinimumFreeDiskGiBForTests(int gib)
{
    if (gib > _minFreeDiskGiB->getMaximum()) {
        _minFreeDiskGiB->setMaximum(gib);
    }
    _minFreeDiskGiB->setValue(gib);
}

///////////////////////////////////////////////////

double
Settings::getUnreachableRamPercent() const
{
    return (double)_unreachableRAMPercent->getValue() / 100.;
}

bool
Settings::getColorPickerLinear() const
{
    return _linearPickers->getValue();
}

int
Settings::getNumberOfThreadsPerEffect() const
{
    return _nThreadsPerEffect->getValue();
}

int
Settings::getNumberOfThreads() const
{
    return _numberOfThreads->getValue();
}

void
Settings::setNumberOfThreads(int threadsNb)
{
    _numberOfThreads->setValue(threadsNb);
}

bool
Settings::isAutoPreviewOnForNewProjects() const
{
    return _autoPreviewEnabledForNewProjects->getValue();
}

#ifdef NATRON_DOCUMENTATION_ONLINE
int
Settings::getDocumentationSource() const
{
    return _documentationSource->getValue();
}
#endif

int
Settings::getServerPort() const
{
    return _wwwServerPort->getValue();
}

void
Settings::setServerPort(int port) const
{
    _wwwServerPort->setValue(port);
}

bool
Settings::isAutoScrollEnabled() const
{
    return _autoScroll->getValue();
}
