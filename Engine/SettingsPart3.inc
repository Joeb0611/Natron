
void
Settings::setDefaultValues()
{
    beginChanges();

    _natronSettingsExist->setDefaultValue(false);

    // General
    _checkForUpdates->setDefaultValue(false);
#ifdef NATRON_USE_BREAKPAD
    _enableCrashReports->setDefaultValue(true);
#endif
    _autoSaveUnSavedProjects->setDefaultValue(true);
    _autoSaveDelay->setDefaultValue(5, 0);
    _saveVersions->setDefaultValue(1);
    _hostName->setDefaultValue(0);
    _customHostName->setDefaultValue(NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB "." NATRON_APPLICATION_NAME);

    // General/Threading
    _numberOfThreads->setDefaultValue(0, 0);
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    _numberOfParallelRenders->setDefaultValue(0, 0);
#endif
    _useThreadPool->setDefaultValue(true);
    _nThreadsPerEffect->setDefaultValue(0);
    _renderInSeparateProcess->setDefaultValue(false, 0);
    _queueRenders->setDefaultValue(false);

    // General/Rendering
    _convertNaNValues->setDefaultValue(true);
    _pluginUseImageCopyForSource->setDefaultValue(false);
    _activateRGBSupport->setDefaultValue(true);
    _activateTransformConcatenationSupport->setDefaultValue(true);

    // General/GPU rendering
    //_openglRendererString
    _nOpenGLContexts->setDefaultValue(2);
#if NATRON_VERSION_MAJOR < 2 || (NATRON_VERSION_MAJOR == 2 && NATRON_VERSION_MINOR < 2)
    _enableOpenGL->setDefaultValue((int)eEnableOpenGLDisabled);
#else
    _enableOpenGL->setDefaultValue((int)eEnableOpenGLDisabledIfBackground);
#endif

    // General/Projects setup
    _firstReadSetProjectFormat->setDefaultValue(true);
    _autoPreviewEnabledForNewProjects->setDefaultValue(true, 0);
    _fixPathsOnProjectPathChanged->setDefaultValue(true);
    //_enableMappingFromDriveLettersToUNCShareNames

    // General/Documentation
    _wwwServerPort->setDefaultValue(0);
#ifdef NATRON_DOCUMENTATION_ONLINE
    _documentationSource->setDefaultValue(0);
#endif

    // General/User Interface
    _notifyOnFileChange->setDefaultValue(true);
#ifdef NATRON_ENABLE_IO_META_NODES
    //_filedialogForWriters
#endif
    _renderOnEditingFinished->setDefaultValue(false);
    _linearPickers->setDefaultValue(true, 0);
    _maxPanelsOpened->setDefaultValue(10, 0);
    _useCursorPositionIncrements->setDefaultValue(true);
    //_defaultLayoutFile
    _loadProjectsWorkspace->setDefaultValue(false);
#ifdef Q_OS_WIN
    _enableConsoleWindow->setDefaultValue(false);
#endif

    // Color-Management
    //_ocioConfigKnob
    _warnOcioConfigKnobChanged->setDefaultValue(true);
    _ocioStartupCheck->setDefaultValue(true);
    //_customOcioConfigFile

    // Caching
    _aggressiveCaching->setDefaultValue(false);
    _maxRAMPercent->setDefaultValue(50, 0);
    _unreachableRAMPercent->setDefaultValue(20); // see https://github.com/NatronGitHub/Natron/issues/486
    _maxViewerDiskCacheGB->setDefaultValue(5, 0);
    _maxDiskCacheNodeGB->setDefaultValue(10, 0);
    _minFreeDiskGiB->setDefaultValue(1, 0);
    //_diskCachePath
    setCachingLabels();

    // Viewer
    _texturesMode->setDefaultValue(0, 0);
    _powerOf2Tiling->setDefaultValue(8, 0);
    _checkerboardTileSize->setDefaultValue(5);
    _checkerboardColor1->setDefaultValue(0.5, 0);
    _checkerboardColor1->setDefaultValue(0.5, 1);
    _checkerboardColor1->setDefaultValue(0.5, 2);
    _checkerboardColor1->setDefaultValue(0.5, 3);
    _checkerboardColor2->setDefaultValue(0., 0);
    _checkerboardColor2->setDefaultValue(0., 1);
    _checkerboardColor2->setDefaultValue(0., 2);
    _checkerboardColor2->setDefaultValue(0., 3);
    _autoWipe->setDefaultValue(true);
    _autoProxyWhenScrubbingTimeline->setDefaultValue(true);
    _autoProxyLevel->setDefaultValue(1);
    _maximumNodeViewerUIOpened->setDefaultValue(2);
    _viewerNumberKeys->setDefaultValue(true);
    _viewerOverlaysPath->setDefaultValue(true);

    // Nodegraph
    _autoScroll->setDefaultValue(false);
    _autoTurbo->setDefaultValue(false);
    _snapNodesToConnections->setDefaultValue(true);
    _useBWIcons->setDefaultValue(false);
    _maxUndoRedoNodeGraph->setDefaultValue(20, 0);
    _disconnectedArrowLength->setDefaultValue(30);
    _hideOptionalInputsAutomatically->setDefaultValue(true);
    _useInputAForMergeAutoConnect->setDefaultValue(true);
    _usePluginIconsInNodeGraph->setDefaultValue(true);
    _useAntiAliasing->setDefaultValue(true);

    // Plugins
    _extraPluginPaths->setDefaultValue("", 0);
    _useStdOFXPluginsLocation->setDefaultValue(true);
    //_templatesPluginPaths
    _preferBundledPlugins->setDefaultValue(true);
    _loadBundledPlugins->setDefaultValue(true);

    // Python
    //_onProjectCreated;
    //_defaultOnProjectLoaded;
    //_defaultOnProjectSave;
    //_defaultOnProjectClose;
    //_defaultOnNodeCreated;
    //_defaultOnNodeDelete;
    //_loadPyPlugsFromPythonScript;
    _echoVariableDeclarationToPython->setDefaultValue(false);

    // Appearance
    _systemFontChoice->setDefaultValue(0);
    _fontSize->setDefaultValue(NATRON_FONT_SIZE_DEFAULT);
    //_qssFile
    //_defaultAppearanceVersion

    // Appearance/Main Window
    _sunkenColor->setDefaultValue(0.12, 0);
    _sunkenColor->setDefaultValue(0.12, 1);
    _sunkenColor->setDefaultValue(0.12, 2);
    _baseColor->setDefaultValue(0.19, 0);
    _baseColor->setDefaultValue(0.19, 1);
    _baseColor->setDefaultValue(0.19, 2);
    _raisedColor->setDefaultValue(0.28, 0);
    _raisedColor->setDefaultValue(0.28, 1);
    _raisedColor->setDefaultValue(0.28, 2);
    _selectionColor->setDefaultValue(0.95, 0);
    _selectionColor->setDefaultValue(0.54, 1);
    _selectionColor->setDefaultValue(0., 2);
    _textColor->setDefaultValue(0.78, 0);
    _textColor->setDefaultValue(0.78, 1);
    _textColor->setDefaultValue(0.78, 2);
    _altTextColor->setDefaultValue(0.6, 0);
    _altTextColor->setDefaultValue(0.6, 1);
    _altTextColor->setDefaultValue(0.6, 2);
    _timelinePlayheadColor->setDefaultValue(0.95, 0);
    _timelinePlayheadColor->setDefaultValue(0.54, 1);
    _timelinePlayheadColor->setDefaultValue(0., 2);
    _timelineBGColor->setDefaultValue(0, 0);
    _timelineBGColor->setDefaultValue(0, 1);
    _timelineBGColor->setDefaultValue(0., 2);
    _timelineBoundsColor->setDefaultValue(0.81, 0);
    _timelineBoundsColor->setDefaultValue(0.27, 1);
    _timelineBoundsColor->setDefaultValue(0.02, 2);
    _interpolatedColor->setDefaultValue(0.34, 0);
    _interpolatedColor->setDefaultValue(0.46, 1);
    _interpolatedColor->setDefaultValue(0.6, 2);
    _keyframeColor->setDefaultValue(0.08, 0);
    _keyframeColor->setDefaultValue(0.38, 1);
    _keyframeColor->setDefaultValue(0.97, 2);
    _trackerKeyframeColor->setDefaultValue(0.7, 0);
    _trackerKeyframeColor->setDefaultValue(0.78, 1);
    _trackerKeyframeColor->setDefaultValue(0.39, 2);
    _exprColor->setDefaultValue(0.7, 0);
    _exprColor->setDefaultValue(0.78, 1);
    _exprColor->setDefaultValue(0.39, 2);
    _cachedFrameColor->setDefaultValue(0.56, 0);
    _cachedFrameColor->setDefaultValue(0.79, 1);
    _cachedFrameColor->setDefaultValue(0.4, 2);
    _diskCachedFrameColor->setDefaultValue(0.27, 0);
    _diskCachedFrameColor->setDefaultValue(0.38, 1);
    _diskCachedFrameColor->setDefaultValue(0.25, 2);
    _sliderColor->setDefaultValue(0.33, 0);
    _sliderColor->setDefaultValue(0.45, 1);
    _sliderColor->setDefaultValue(0.44, 2);

    // Apprance/Curve Editor
    _curveEditorBGColor->setDefaultValue(0., 0);
    _curveEditorBGColor->setDefaultValue(0., 1);
    _curveEditorBGColor->setDefaultValue(0., 2);
    _gridColor->setDefaultValue(0.46, 0);
    _gridColor->setDefaultValue(0.84, 1);
    _gridColor->setDefaultValue(0.35, 2);
    _curveEditorScaleColor->setDefaultValue(0.26, 0);
    _curveEditorScaleColor->setDefaultValue(0.48, 1);
    _curveEditorScaleColor->setDefaultValue(0.2, 2);

    // Appearance/Dope Sheet
    _dopeSheetEditorBackgroundColor->setDefaultValue(0.208, 0);
    _dopeSheetEditorBackgroundColor->setDefaultValue(0.208, 1);
    _dopeSheetEditorBackgroundColor->setDefaultValue(0.208, 2);
    _dopeSheetEditorRootSectionBackgroundColor->setDefaultValue(0.204, 0);
    _dopeSheetEditorRootSectionBackgroundColor->setDefaultValue(0.204, 1);
    _dopeSheetEditorRootSectionBackgroundColor->setDefaultValue(0.204, 2);
    _dopeSheetEditorRootSectionBackgroundColor->setDefaultValue(0.2, 3);
    _dopeSheetEditorKnobSectionBackgroundColor->setDefaultValue(0.443, 0);
    _dopeSheetEditorKnobSectionBackgroundColor->setDefaultValue(0.443, 1);
    _dopeSheetEditorKnobSectionBackgroundColor->setDefaultValue(0.443, 2);
    _dopeSheetEditorKnobSectionBackgroundColor->setDefaultValue(0.2, 3);
    _dopeSheetEditorScaleColor->setDefaultValue(0.714, 0);
    _dopeSheetEditorScaleColor->setDefaultValue(0.718, 1);
    _dopeSheetEditorScaleColor->setDefaultValue(0.714, 2);
    _dopeSheetEditorGridColor->setDefaultValue(0.714, 0);
    _dopeSheetEditorGridColor->setDefaultValue(0.714, 1);
    _dopeSheetEditorGridColor->setDefaultValue(0.714, 2);

    // Appearance/Script Editor
    _curLineColor->setDefaultValue(0.35, 0);
    _curLineColor->setDefaultValue(0.35, 1);
    _curLineColor->setDefaultValue(0.35, 2);
    _keywordColor->setDefaultValue(0.7, 0);
    _keywordColor->setDefaultValue(0.7, 1);
    _keywordColor->setDefaultValue(0., 2);
    _operatorColor->setDefaultValue(0.78, 0);
    _operatorColor->setDefaultValue(0.78, 1);
    _operatorColor->setDefaultValue(0.78, 2);
    _braceColor->setDefaultValue(0.85, 0);
    _braceColor->setDefaultValue(0.85, 1);
    _braceColor->setDefaultValue(0.85, 2);
    _defClassColor->setDefaultValue(0.7, 0);
    _defClassColor->setDefaultValue(0.7, 1);
    _defClassColor->setDefaultValue(0., 2);
    _stringsColor->setDefaultValue(0.8, 0);
    _stringsColor->setDefaultValue(0.2, 1);
    _stringsColor->setDefaultValue(0., 2);
    _commentsColor->setDefaultValue(0.25, 0);
    _commentsColor->setDefaultValue(0.6, 1);
    _commentsColor->setDefaultValue(0.25, 2);
    _selfColor->setDefaultValue(0.7, 0);
    _selfColor->setDefaultValue(0.7, 1);
    _selfColor->setDefaultValue(0., 2);
    _numbersColor->setDefaultValue(0.25, 0);
    _numbersColor->setDefaultValue(0.8, 1);
    _numbersColor->setDefaultValue(0.9, 2);
    _scriptEditorFontChoice->setDefaultValue(0);
    _scriptEditorFontSize->setDefaultValue(NATRON_FONT_SIZE_DEFAULT);


    // Appearance/Node Graph
    _defaultNodeColor->setDefaultValue(0.7, 0);
    _defaultNodeColor->setDefaultValue(0.7, 1);
    _defaultNodeColor->setDefaultValue(0.7, 2);
    _defaultBackdropColor->setDefaultValue(0.45, 0);
    _defaultBackdropColor->setDefaultValue(0.45, 1);
    _defaultBackdropColor->setDefaultValue(0.45, 2);
    _defaultGeneratorColor->setDefaultValue(0.3, 0);
    _defaultGeneratorColor->setDefaultValue(0.5, 1);
    _defaultGeneratorColor->setDefaultValue(0.2, 2);
    _defaultReaderColor->setDefaultValue(0.7, 0);
    _defaultReaderColor->setDefaultValue(0.7, 1);
    _defaultReaderColor->setDefaultValue(0.7, 2);
    _defaultWriterColor->setDefaultValue(0.75, 0);
    _defaultWriterColor->setDefaultValue(0.75, 1);
    _defaultWriterColor->setDefaultValue(0., 2);
    _defaultColorGroupColor->setDefaultValue(0.48, 0);
    _defaultColorGroupColor->setDefaultValue(0.66, 1);
    _defaultColorGroupColor->setDefaultValue(1., 2);
    _defaultFilterGroupColor->setDefaultValue(0.8, 0);
    _defaultFilterGroupColor->setDefaultValue(0.5, 1);
    _defaultFilterGroupColor->setDefaultValue(0.3, 2);
    _defaultTransformGroupColor->setDefaultValue(0.7, 0);
    _defaultTransformGroupColor->setDefaultValue(0.3, 1);
    _defaultTransformGroupColor->setDefaultValue(0.1, 2);
    _defaultTimeGroupColor->setDefaultValue(0.7, 0);
    _defaultTimeGroupColor->setDefaultValue(0.65, 1);
    _defaultTimeGroupColor->setDefaultValue(0.35, 2);
    _defaultDrawGroupColor->setDefaultValue(0.75, 0);
    _defaultDrawGroupColor->setDefaultValue(0.75, 1);
    _defaultDrawGroupColor->setDefaultValue(0.75, 2);
    _defaultKeyerGroupColor->setDefaultValue(0., 0);
    _defaultKeyerGroupColor->setDefaultValue(1, 1);
    _defaultKeyerGroupColor->setDefaultValue(0., 2);
    _defaultChannelGroupColor->setDefaultValue(0.6, 0);
    _defaultChannelGroupColor->setDefaultValue(0.24, 1);
    _defaultChannelGroupColor->setDefaultValue(0.39, 2);
    _defaultMergeGroupColor->setDefaultValue(0.3, 0);
    _defaultMergeGroupColor->setDefaultValue(0.37, 1);
    _defaultMergeGroupColor->setDefaultValue(0.776, 2);
    _defaultViewsGroupColor->setDefaultValue(0.5, 0);
    _defaultViewsGroupColor->setDefaultValue(0.9, 1);
    _defaultViewsGroupColor->setDefaultValue(0.7, 2);
    _defaultDeepGroupColor->setDefaultValue(0., 0);
    _defaultDeepGroupColor->setDefaultValue(0., 1);
    _defaultDeepGroupColor->setDefaultValue(0.38, 2);


    endChanges();
} // setDefaultValues

void
Settings::warnChangedKnobs(const std::vector<KnobI*>& knobs)
{
    bool didFontWarn = false;
    bool didOCIOWarn = false;
    bool didOFXCacheWarn = false;

    for (U32 i = 0; i < knobs.size(); ++i) {
        if ( ( ( knobs[i] == _fontSize.get() ) ||
               ( knobs[i] == _systemFontChoice.get() ) )
             && !didFontWarn ) {
            didFontWarn = true;
            Dialogs::warningDialog( tr("Font change").toStdString(),
                                    tr("Changing the font requires a restart of %1.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString() );
        } else if ( ( ( knobs[i] == _ocioConfigKnob.get() ) ||
                      ( knobs[i] == _customOcioConfigFile.get() ) )
                    && !didOCIOWarn ) {
            didOCIOWarn = true;
            bool warnOcioChanged = _warnOcioConfigKnobChanged->getValue();
            if (warnOcioChanged) {
                bool stopAsking = false;
                Dialogs::warningDialog(tr("OCIO config changed").toStdString(),
                                       tr("The OpenColorIO config change requires a restart of %1 to be effective.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString(), &stopAsking);
                if (stopAsking) {
                    _warnOcioConfigKnobChanged->setValue(false);
                    saveSetting( _warnOcioConfigKnobChanged.get() );
                }
            }
        } else if ( knobs[i] == _texturesMode.get() ) {
            AppInstanceVec apps = appPTR->getAppInstances();
            for (AppInstanceVec::iterator it = apps.begin(); it != apps.end(); ++it) {
                std::list<ViewerInstance*> allViewers;
                (*it)->getProject()->getViewers(&allViewers);
                for (std::list<ViewerInstance*>::iterator it = allViewers.begin(); it != allViewers.end(); ++it) {
                    (*it)->renderCurrentFrame(true);
                }
            }
        } else if ( ( ( knobs[i] == _loadBundledPlugins.get() ) ||
                      ( knobs[i] == _preferBundledPlugins.get() ) ||
                      ( knobs[i] == _useStdOFXPluginsLocation.get() ) ||
                      ( knobs[i] == _extraPluginPaths.get() ) )
                    && !didOFXCacheWarn ) {
            didOFXCacheWarn = true;
            appPTR->clearPluginsLoadedCache(); // clear the cache for next restart
            Dialogs::warningDialog( tr("OpenFX plugins path changed").toStdString(),
                                    tr("The OpenFX plugins path change requires a restart of %1 to be effective.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString() );
        }
    }
} // Settings::warnChangedKnobs

void
Settings::saveAllSettings()
{
    if ( !_saveSettings->getValue() ) {
        return;
    }
    const KnobsVec &knobs = getKnobs();
    std::vector<KnobI*> k( knobs.size() );

    for (U32 i = 0; i < knobs.size(); ++i) {
        k[i] = knobs[i].get();
    }
    saveSettings(k, false, true);
}

void
Settings::restorePluginSettings()
{
    const PluginsMap& plugins = appPTR->getPluginsList();
    QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );

    for (PluginsMap::const_iterator it = plugins.begin(); it != plugins.end(); ++it) {
        if ( it->first.empty() ) {
            continue;
        }
        assert(it->second.size() > 0);

        for (PluginVersionsOrdered::const_reverse_iterator itver = it->second.rbegin(); itver != it->second.rend(); ++itver) {
            Plugin* plugin  = *itver;
            assert(plugin);

            if ( plugin->getIsForInternalUseOnly() ) {
                continue;
            }


            {
                QString pluginIDKey = plugin->getPluginID() + QString::fromUtf8("_") + QString::number( plugin->getMajorVersion() ) + QString::fromUtf8("_") + QString::number( plugin->getMinorVersion() );
                QString enabledKey = pluginIDKey + QString::fromUtf8("_enabled");
                if ( settings.contains(enabledKey) ) {
                    bool enabled = settings.value(enabledKey).toBool();
                    plugin->setActivated(enabled);
                } else {
                    settings.setValue( enabledKey, plugin->isActivated() );
                }

                QString rsKey = pluginIDKey + QString::fromUtf8("_rs");
                if ( settings.contains(rsKey) ) {
                    bool renderScaleEnabled = settings.value(rsKey).toBool();
                    plugin->setRenderScaleEnabled(renderScaleEnabled);
                } else {
                    settings.setValue( rsKey, plugin->isRenderScaleEnabled() );
                }

                QString mtKey = pluginIDKey + QString::fromUtf8("_mt");
                if ( settings.contains(mtKey) ) {
                    bool multiThreadingEnabled = settings.value(mtKey).toBool();
                    plugin->setMultiThreadingEnabled(multiThreadingEnabled);
                } else {
                    settings.setValue( mtKey, plugin->isMultiThreadingEnabled() );
                }

                QString glKey = pluginIDKey + QString::fromUtf8("_gl");
                if (settings.contains(glKey)) {
                    bool openglEnabled = settings.value(glKey).toBool();
                    plugin->setOpenGLEnabled(openglEnabled);
                } else {
                    settings.setValue(glKey, plugin->isOpenGLEnabled());
                }

            }
        }
    }
} // Settings::restorePluginSettings

void
Settings::savePluginsSettings()
{
    const PluginsMap& plugins = appPTR->getPluginsList();
    QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );

    for (PluginsMap::const_iterator it = plugins.begin(); it != plugins.end(); ++it) {
        assert(it->second.size() > 0);

        for (PluginVersionsOrdered::const_reverse_iterator itver = it->second.rbegin(); itver != it->second.rend(); ++itver) {
            Plugin* plugin  = *itver;
            assert(plugin);

            QString pluginID = plugin->getPluginID() + QString::fromUtf8("_") + QString::number( plugin->getMajorVersion() ) + QString::fromUtf8("_") + QString::number( plugin->getMinorVersion() );
            QString enabledKey = pluginID + QString::fromUtf8("_enabled");
            settings.setValue( enabledKey, plugin->isActivated() );

            QString rsKey = pluginID + QString::fromUtf8("_rs");
            settings.setValue( rsKey, plugin->isRenderScaleEnabled() );

            QString mtKey = pluginID + QString::fromUtf8("_mt");
            settings.setValue(mtKey, plugin->isMultiThreadingEnabled());

            QString glKey = pluginID + QString::fromUtf8("_gl");
            settings.setValue(glKey, plugin->isOpenGLEnabled());

        }
    }
}

void
Settings::setSaveSettings(bool v)
{
    _saveSettings->setValue(v);
}

bool
Settings::getSaveSettings() const
{
    return _saveSettings->getValue();
}

void
Settings::saveSettings(const std::vector<KnobI*>& knobs,
                       bool doWarnings,
                       bool pluginSettings)
{
    if ( !_saveSettings->getValue() ) {
        return;
    }
    if (pluginSettings) {
        savePluginsSettings();
    }
    std::vector<KnobI*> changedKnobs;
    QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );

    settings.setValue(QString::fromUtf8(kQSettingsSoftwareMajorVersionSettingName), NATRON_VERSION_MAJOR);
    for (U32 i = 0; i < knobs.size(); ++i) {
        KnobStringBase* isString = dynamic_cast<KnobStringBase*>(knobs[i]);
        KnobIntBase* isInt = dynamic_cast<KnobIntBase*>(knobs[i]);
        KnobChoice* isChoice = dynamic_cast<KnobChoice*>(knobs[i]);
        KnobDoubleBase* isDouble = dynamic_cast<KnobDoubleBase*>(knobs[i]);
        KnobBoolBase* isBool = dynamic_cast<KnobBoolBase*>(knobs[i]);

        const std::string& name = knobs[i]->getName();
        for (int j = 0; j < knobs[i]->getDimension(); ++j) {
            QString dimensionName;
            if (knobs[i]->getDimension() > 1) {
                dimensionName =  QString::fromUtf8( name.c_str() ) + QLatin1Char('.') + QString::fromUtf8( knobs[i]->getDimensionName(j).c_str() );
            } else {
                dimensionName = QString::fromUtf8( name.c_str() );
            }
            try {
                if (isString) {
                    QString old = settings.value(dimensionName).toString();
                    QString newValue = QString::fromUtf8( isString->getValue(j).c_str() );
                    if (old != newValue) {
                        changedKnobs.push_back(knobs[i]);
                    }
                    settings.setValue( dimensionName, QVariant(newValue) );
                } else if (isInt) {
                    if (isChoice) {
                        ///For choices,serialize the choice name instead
                        int newIndex = isChoice->getValue(j);
                        const std::vector<ChoiceOption> entries = isChoice->getEntries_mt_safe();
                        if ( newIndex < (int)entries.size() ) {
                            QString oldValue = settings.value(dimensionName).toString();
                            QString newValue = QString::fromUtf8( entries[newIndex].id.c_str());
                            if (oldValue != newValue) {
                                changedKnobs.push_back(knobs[i]);
                            }
                            settings.setValue( dimensionName, QVariant(newValue) );
                        }
                    } else {
                        int newValue = isInt->getValue(j);
                        int oldValue = settings.value( dimensionName, QVariant(std::numeric_limits<int>::min()) ).toInt();
                        if (newValue != oldValue) {
                            changedKnobs.push_back(knobs[i]);
                        }
                        settings.setValue( dimensionName, QVariant(newValue) );
                    }
                } else if (isDouble) {
                    double newValue = isDouble->getValue(j);
                    double oldValue = settings.value( dimensionName, QVariant(std::numeric_limits<int>::min()) ).toDouble();
                    if (newValue != oldValue) {
                        changedKnobs.push_back(knobs[i]);
                    }
                    settings.setValue( dimensionName, QVariant(newValue) );
                } else if (isBool) {
                    bool newValue = isBool->getValue(j);
                    bool oldValue = settings.value(dimensionName).toBool();
                    if (newValue != oldValue) {
                        changedKnobs.push_back(knobs[i]);
                    }
                    settings.setValue( dimensionName, QVariant(newValue) );
                } else {
                    assert(false);
                }
            } catch (std::logic_error&) {
                // ignore
            }
        } // for (int j = 0; j < knobs[i]->getDimension(); ++j) {
    } // for (U32 i = 0; i < knobs.size(); ++i) {

    if (doWarnings) {
        warnChangedKnobs(changedKnobs);
    }
} // saveSettings

void
Settings::restoreKnobsFromSettings(const std::vector<KnobI*>& knobs)
{
    QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );

    for (U32 i = 0; i < knobs.size(); ++i) {
        KnobStringBase* isString = dynamic_cast<KnobStringBase*>(knobs[i]);
        KnobIntBase* isInt = dynamic_cast<KnobIntBase*>(knobs[i]);
        KnobChoice* isChoice = dynamic_cast<KnobChoice*>(knobs[i]);
        KnobDoubleBase* isDouble = dynamic_cast<KnobDoubleBase*>(knobs[i]);
        KnobBoolBase* isBool = dynamic_cast<KnobBoolBase*>(knobs[i]);

        const std::string& name = knobs[i]->getName();

        for (int j = 0; j < knobs[i]->getDimension(); ++j) {
            std::string dimensionName = knobs[i]->getDimension() > 1 ? name + '.' + knobs[i]->getDimensionName(j) : name;
            QString qDimName = QString::fromUtf8( dimensionName.c_str() );

            if ( settings.contains(qDimName) ) {
                if (isString) {
                    isString->setValue(settings.value(qDimName).toString().toStdString(), ViewSpec::all(), j);
                } else if (isInt) {
                    if (isChoice) {
                        ///For choices,serialize the choice name instead
                        std::string value = settings.value(qDimName).toString().toStdString();
                        const std::vector<ChoiceOption> entries = isChoice->getEntries_mt_safe();
                        int found = -1;

                        for (U32 k = 0; k < entries.size(); ++k) {
                            if (entries[k].id == value) {
                                found = (int)k;
                                break;
                            }
                        }

                        if (found >= 0) {
                            isChoice->setValue(found, ViewSpec::all(), j);
                        }
                    } else {
                        isInt->setValue(settings.value(qDimName).toInt(), ViewSpec::all(), j);
                    }
                } else if (isDouble) {
                    isDouble->setValue(settings.value(qDimName).toDouble(), ViewSpec::all(), j);
                } else if (isBool) {
                    isBool->setValue(settings.value(qDimName).toBool(), ViewSpec::all(), j);
                } else {
                    assert(false);
                }
            }
        }
    }
} // Settings::restoreKnobsFromSettings

void
Settings::restoreKnobsFromSettings(const KnobsVec& knobs)
{
    std::vector<KnobI*> k( knobs.size() );

    for (U32 i = 0; i < knobs.size(); ++i) {
        k[i] = knobs[i].get();
    }
    restoreKnobsFromSettings(k);
}

void
Settings::restoreSettings(bool useDefault)
{
    _restoringSettings = true;

    // call restoreKnobsFromSettings only if --no-settings is not part of the command-line arguments
    if (!useDefault) {
        const KnobsVec& knobs = getKnobs();
        restoreKnobsFromSettings(knobs);
    }

    if (!_ocioRestored) {
        ///Load even though there's no settings!
        tryLoadOpenColorIOConfig();
    }

    // Restore opengl renderer
    {
        std::vector<ChoiceOption> availableRenderers = _availableOpenGLRenderers->getEntries_mt_safe();
        QString missingGLError;
        bool hasGL = appPTR->hasOpenGLForRequirements(eOpenGLRequirementsTypeRendering, &missingGLError);

        if ( availableRenderers.empty() || !hasGL) {
            if (missingGLError.isEmpty()) {
                _openglRendererString->setValue( tr("OpenGL rendering disabled: No device meeting %1 requirements could be found.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdString() );
            } else {
                _openglRendererString->setValue(missingGLError.toStdString());
            }
        }
        int curIndex = _availableOpenGLRenderers->getValue();
        if ( (curIndex >= 0) && ( curIndex < (int)availableRenderers.size() ) ) {
            const std::list<OpenGLRendererInfo>& renderers = appPTR->getOpenGLRenderers();
            int i = 0;
            for (std::list<OpenGLRendererInfo>::const_iterator it = renderers.begin(); it != renderers.end(); ++it, ++i) {
                if (i == curIndex) {
                    QString maxMemoryString = it->maxMemBytes == 0 ? tr("Unknown") : printAsRAM(it->maxMemBytes);
                    QString curRenderer = (QString::fromUtf8("<p><h2>") +
                                           tr("OpenGL Renderer Infos:") +
                                           QString::fromUtf8("</h2></p><p><b>") +
                                           tr("Vendor:") +
                                           QString::fromUtf8("</b> %1</p><p><b>").arg( QString::fromUtf8( it->vendorName.c_str() ) ) +
                                           tr("Renderer:") +
                                           QString::fromUtf8("</b> %1</p><p><b>").arg( QString::fromUtf8( it->rendererName.c_str() ) ) +
                                           tr("OpenGL Version:") +
                                           QString::fromUtf8("</b> %1</p><p><b>").arg( QString::fromUtf8( it->glVersionString.c_str() ) ) +
                                           tr("GLSL Version:") +
                                           QString::fromUtf8("</b> %1</p><p><b>").arg( QString::fromUtf8( it->glslVersionString.c_str() ) ) +
                                           tr("Max. Memory:") +
                                           QString::fromUtf8("</b> %1</p><p><b>").arg(maxMemoryString) +
                                           tr("Max. Texture Size (px):") +
                                           QString::fromUtf8("</b> %5</p<").arg(it->maxTextureSize));
                    _openglRendererString->setValue( curRenderer.toStdString() );
                    break;
                }
            }
        }
    }

    if (!appPTR->isTextureFloatSupported()) {
        if (_texturesMode) {
            _texturesMode->setSecret(true);
        }
    }

    _settingsExisted = false;
    try {
        _settingsExisted = _natronSettingsExist->getValue();

        if (!_settingsExisted && !useDefault) {
            _natronSettingsExist->setValue(true);
            saveSetting( _natronSettingsExist.get() );
        }

        int appearanceVersion = _defaultAppearanceVersion->getValue();
        if ( _settingsExisted && (appearanceVersion < NATRON_DEFAULT_APPEARANCE_VERSION) ) {
            _defaultAppearanceOutdated = true;
            _defaultAppearanceVersion->setValue(NATRON_DEFAULT_APPEARANCE_VERSION);
            saveSetting( _defaultAppearanceVersion.get() );
        }

        appPTR->setNThreadsPerEffect( getNumberOfThreadsPerEffect() );
        appPTR->setNThreadsToRender( getNumberOfThreads() );
        appPTR->setUseThreadPool( _useThreadPool->getValue() );
        appPTR->setPluginsUseInputImageCopyToRender( _pluginUseImageCopyForSource->getValue() );
    } catch (std::logic_error&) {
        // ignore
    }

    _restoringSettings = false;
} // restoreSettings

bool
Settings::tryLoadOpenColorIOConfig()
{
    // the default value is the environment variable "OCIO"
    QString configFile = QFile::decodeName( qgetenv(NATRON_OCIO_ENV_VAR_NAME) );

    // OCIO environment variable overrides everything, then try the custom config...
    if ( configFile.isEmpty() && _customOcioConfigFile->isEnabled(0) ) {
        ///try to load from the file
        std::string file;
        try {
            file = _customOcioConfigFile->getValue();
        } catch (...) {
            // ignore exceptions
        }
        if ( file.empty() ) {
            return false;
        }
        configFile = QString::fromUtf8( file.c_str() );
    }
    if ( !configFile.isEmpty() ) {
        if ( !QFile::exists(configFile) )  {
            Dialogs::errorDialog( "OpenColorIO", tr("%1: No such file.").arg(configFile).toStdString() );

            return false;
        }
    } else {
        // ... and finally try the setting from the choice menu.
        try {
            ///try to load from the combobox
            QString activeEntryText  = QString::fromUtf8( _ocioConfigKnob->getActiveEntry().id.c_str() );
            QString configFileName = QString( activeEntryText + QString::fromUtf8(".ocio") );
            QStringList defaultConfigsPaths = getDefaultOcioConfigPaths();
            Q_FOREACH(const QString &defaultConfigsDirStr, defaultConfigsPaths) {
                QDir defaultConfigsDir(defaultConfigsDirStr);

                if ( !defaultConfigsDir.exists() ) {
                    qDebug() << "Attempt to read an OpenColorIO configuration but the configuration directory"
                             << defaultConfigsDirStr << "does not exist.";
                    continue;
                }
                ///try to open the .ocio config file first in the defaultConfigsDir
                ///if we can't find it, try to look in a subdirectory with the name of the config for the file config.ocio
                if ( !defaultConfigsDir.exists(configFileName) ) {
                    QDir subDir(defaultConfigsDirStr + QDir::separator() + activeEntryText);
                    if ( !subDir.exists() ) {
                        Dialogs::errorDialog( "OpenColorIO", tr("%1: No such file or directory.").arg( subDir.absoluteFilePath( QString::fromUtf8("config.ocio") ) ).toStdString() );

                        return false;
                    }
                    if ( !subDir.exists( QString::fromUtf8("config.ocio") ) ) {
                        Dialogs::errorDialog( "OpenColorIO", tr("%1: No such file or directory.").arg( subDir.absoluteFilePath( QString::fromUtf8("config.ocio") ) ).toStdString() );

                        return false;
                    }
                    configFile = subDir.absoluteFilePath( QString::fromUtf8("config.ocio") );
                } else {
                    configFile = defaultConfigsDir.absoluteFilePath(configFileName);
                }
            }
        } catch (...) {
            // ignore exceptions
        }

        if ( configFile.isEmpty() ) {
            return false;
        }
    }
    _ocioRestored = true;
#ifdef DEBUG
    qDebug() << "setting OCIO=" << configFile;
#endif
    std::string stdConfigFile = configFile.toStdString();
#if 0 //def __NATRON_WIN32__ // commented out in https://github.com/NatronGitHub/Natron/commit/3445d671f15fbd97bca164b53ceb41cef47c61c3
    _wputenv_s(L"OCIO", StrUtils::utf8_to_utf16(stdConfigFile).c_str());
#else
    qputenv( NATRON_OCIO_ENV_VAR_NAME, stdConfigFile.c_str() );
#endif

    std::string configPath = SequenceParsing::removePath(stdConfigFile);
    if ( !configPath.empty() && (configPath[configPath.size() - 1] == '/') ) {
        configPath.erase(configPath.size() - 1, 1);
    }
    appPTR->onOCIOConfigPathChanged(configPath);

    return true;
} // tryLoadOpenColorIOConfig

#ifdef NATRON_USE_BREAKPAD
inline
void
crash_application()
{
    std::cerr << "CRASHING APPLICATION NOW UPON USER REQUEST!" << std::endl;
    volatile int* a = (int*)(NULL);

    // coverity[var_deref_op]
    *a = 1;
}
