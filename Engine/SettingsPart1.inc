
void
Settings::initializeKnobsUserInterface()
{
    _uiPage = AppManager::createKnob<KnobPage>( this, tr("User Interface") );
    _uiPage->setName("userInterfacePage");

    _notifyOnFileChange = AppManager::createKnob<KnobBool>( this, tr("Warn when a file changes externally") );
    _notifyOnFileChange->setName("warnOnExternalChange");
    _notifyOnFileChange->setHintToolTip( tr("When checked, if a file read from a file parameter changes externally, a warning will be displayed "
                                            "on the viewer. Turning this off will suspend the notification system.") );
    _uiPage->addKnob(_notifyOnFileChange);

#ifdef NATRON_ENABLE_IO_META_NODES
    _filedialogForWriters = AppManager::createKnob<KnobBool>( this, tr("Prompt with file dialog when creating Write node") );
    _filedialogForWriters->setName("writeUseDialog");
    _filedialogForWriters->setDefaultValue(true);
    _filedialogForWriters->setHintToolTip( tr("When checked, opens-up a file dialog when creating a Write node") );
    _uiPage->addKnob(_filedialogForWriters);
#endif


    _renderOnEditingFinished = AppManager::createKnob<KnobBool>( this, tr("Refresh viewer only when editing is finished") );
    _renderOnEditingFinished->setName("renderOnEditingFinished");
    _renderOnEditingFinished->setHintToolTip( tr("When checked, the viewer triggers a new render only when mouse is released when editing parameters, curves "
                                                 " or the timeline. This setting doesn't apply to roto splines editing.") );
    _uiPage->addKnob(_renderOnEditingFinished);


    _linearPickers = AppManager::createKnob<KnobBool>( this, tr("Linear color pickers") );
    _linearPickers->setName("linearPickers");
    _linearPickers->setHintToolTip( tr("When activated, all colors picked from the color parameters are linearized "
                                       "before being fetched. Otherwise they are in the same colorspace "
                                       "as the viewer they were picked from.") );
    _uiPage->addKnob(_linearPickers);

    _maxPanelsOpened = AppManager::createKnob<KnobInt>( this, tr("Maximum number of open settings panels (0=\"unlimited\")") );
    _maxPanelsOpened->setName("maxPanels");
    _maxPanelsOpened->setHintToolTip( tr("This property holds the maximum number of settings panels that can be "
                                         "held by the properties dock at the same time. "
                                         "The special value of 0 indicates there can be an unlimited number of panels opened.") );
    _maxPanelsOpened->disableSlider();
    _maxPanelsOpened->setMinimum(0);
    _maxPanelsOpened->setMaximum(99);
    _uiPage->addKnob(_maxPanelsOpened);

    _useCursorPositionIncrements = AppManager::createKnob<KnobBool>( this, tr("Value increments based on cursor position") );
    _useCursorPositionIncrements->setName("cursorPositionAwareFields");
    _useCursorPositionIncrements->setHintToolTip( tr("When enabled, incrementing the value fields of parameters with the "
                                                     "mouse wheel or with arrow keys will increment the digits on the right "
                                                     "of the cursor. \n"
                                                     "When disabled, the value fields are incremented given what the plug-in "
                                                     "decided it should be. You can alter this increment by holding "
                                                     "Shift (x10) or Control (/10) while incrementing.") );
    _uiPage->addKnob(_useCursorPositionIncrements);

    _defaultLayoutFile = AppManager::createKnob<KnobFile>( this, tr("Default layout file") );
    _defaultLayoutFile->setName("defaultLayout");
    _defaultLayoutFile->setHintToolTip( tr("When set, %1 uses the given layout file "
                                           "as default layout for new projects. You can export/import a layout to/from a file "
                                           "from the Layout menu. If empty, the default application layout is used.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _uiPage->addKnob(_defaultLayoutFile);

    _loadProjectsWorkspace = AppManager::createKnob<KnobBool>( this, tr("Load workspace embedded within projects") );
    _loadProjectsWorkspace->setName("loadProjectWorkspace");
    _loadProjectsWorkspace->setHintToolTip( tr("When checked, when loading a project, the workspace (windows layout) will also be loaded, otherwise it "
                                               "will use your current layout.") );
    _uiPage->addKnob(_loadProjectsWorkspace);

#ifdef Q_OS_WIN
    _enableConsoleWindow = AppManager::createKnob<KnobBool>( this, tr("Enable console window") );
    _enableConsoleWindow->setName("enableConsoleWindow");
    _enableConsoleWindow->setHintToolTip( tr("When checked show console window on Windows.") );
    _uiPage->addKnob(_enableConsoleWindow);
#endif
} // Settings::initializeKnobsUserInterface

void
Settings::initializeKnobsColorManagement()
{
    _ocioTab = AppManager::createKnob<KnobPage>( this, tr("Color Management") );
    _ocioConfigKnob = AppManager::createKnob<KnobChoice>( this, tr("OpenColorIO configuration") );
    _ocioConfigKnob->setName("ocioConfig");

    std::vector<ChoiceOption> configs;
    int defaultIndex = 0;
    QStringList defaultOcioConfigsPaths = getDefaultOcioConfigPaths();
    Q_FOREACH(const QString &defaultOcioConfigsDir, defaultOcioConfigsPaths) {
        QDir ocioConfigsDir(defaultOcioConfigsDir);

        if ( ocioConfigsDir.exists() ) {
            QStringList entries = ocioConfigsDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
            for (int j = 0; j < entries.size(); ++j) {
                if ( entries[j] == QString::fromUtf8(NATRON_DEFAULT_OCIO_CONFIG_NAME) ) {
                    defaultIndex = j;
                }
                configs.push_back(ChoiceOption( entries[j].toStdString() ));
            }

            break; //if we found 1 OpenColorIO-Configs directory, skip the next
        }
    }
    configs.push_back(ChoiceOption(NATRON_CUSTOM_OCIO_CONFIG_NAME));
    _ocioConfigKnob->populateChoices(configs);
    _ocioConfigKnob->setDefaultValue(defaultIndex, 0);
    _ocioConfigKnob->setHintToolTip( tr("Select the OpenColorIO configuration you would like to use globally for all "
                                        "operators and plugins that use OpenColorIO, by setting the \"OCIO\" "
                                        "environment variable. Only nodes created after changing this parameter will take "
                                        "it into account, and it is better to restart the application after changing it. "
                                        "When \"%1\" is selected, the "
                                        "\"Custom OpenColorIO config file\" parameter is used.").arg( QString::fromUtf8(NATRON_CUSTOM_OCIO_CONFIG_NAME) ) );

    _ocioTab->addKnob(_ocioConfigKnob);

    _customOcioConfigFile = AppManager::createKnob<KnobFile>( this, tr("Custom OpenColorIO configuration file") );
    _customOcioConfigFile->setName("ocioCustomConfigFile");

    if (_ocioConfigKnob->getNumEntries() == 1) {
        _customOcioConfigFile->setDefaultAllDimensionsEnabled(true);
    } else {
        _customOcioConfigFile->setDefaultAllDimensionsEnabled(false);
    }

    _customOcioConfigFile->setHintToolTip( tr("OpenColorIO configuration file (config.ocio) to use when \"%1\" "
                                              "is selected as the OpenColorIO config.").arg( QString::fromUtf8(NATRON_CUSTOM_OCIO_CONFIG_NAME) ) );
    _ocioTab->addKnob(_customOcioConfigFile);

    _warnOcioConfigKnobChanged = AppManager::createKnob<KnobBool>( this, tr("Warn on OpenColorIO config change") );
    _warnOcioConfigKnobChanged->setName("warnOCIOChanged");
    _warnOcioConfigKnobChanged->setHintToolTip( tr("Show a warning dialog when changing the OpenColorIO config to remember that a restart is required.") );
    _ocioTab->addKnob(_warnOcioConfigKnobChanged);

    _ocioStartupCheck = AppManager::createKnob<KnobBool>( this, tr("Warn on startup if OpenColorIO config is not the default") );
    _ocioStartupCheck->setName("startupCheckOCIO");
    _ocioTab->addKnob(_ocioStartupCheck);
} // Settings::initializeKnobsColorManagement

void
Settings::initializeKnobsAppearance()
{
    //////////////APPEARANCE TAB/////////////////
    _appearanceTab = AppManager::createKnob<KnobPage>( this, tr("Appearance") );

    _defaultAppearanceVersion = AppManager::createKnob<KnobInt>( this, tr("Appearance version") );
    _defaultAppearanceVersion->setName("appearanceVersion");
    _defaultAppearanceVersion->setSecretByDefault(true);
    _appearanceTab->addKnob(_defaultAppearanceVersion);

    _systemFontChoice = AppManager::createKnob<KnobChoice>( this, tr("Font") );
    _systemFontChoice->setHintToolTip( tr("List of all fonts available on your system") );
    _systemFontChoice->setName("systemFont");
    _systemFontChoice->setAddNewLine(false);
    _appearanceTab->addKnob(_systemFontChoice);

    _fontSize = AppManager::createKnob<KnobInt>( this, tr("Font size") );
    _fontSize->setName("fontSize");
    _appearanceTab->addKnob(_fontSize);

    _qssFile = AppManager::createKnob<KnobFile>( this, tr("Stylesheet file (.qss)") );
    _qssFile->setName("stylesheetFile");
    _qssFile->setHintToolTip( tr("When pointing to a valid .qss file, the stylesheet of the application will be set according to this file instead of the default "
                                 "stylesheet. You can adapt the default stylesheet that can be found in your distribution of %1.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _appearanceTab->addKnob(_qssFile);
} // Settings::initializeKnobsAppearance

void
Settings::initializeKnobsGuiColors()
{
    _guiColorsTab = AppManager::createKnob<KnobPage>( this, tr("Main Window") );
    _appearanceTab->addKnob(_guiColorsTab);

    _useBWIcons = AppManager::createKnob<KnobBool>( this, tr("Use black & white toolbutton icons") );
    _useBWIcons->setName("useBwIcons");
    _useBWIcons->setHintToolTip( tr("When checked, the tools icons in the left toolbar are greyscale. Changing this takes "
                                    "effect upon the next launch of the application.") );
    _guiColorsTab->addKnob(_useBWIcons);


    _sunkenColor =  AppManager::createKnob<KnobColor>(this, tr("Sunken"), 3);
    _sunkenColor->setName("sunken");
    _sunkenColor->setSimplified(true);
    _guiColorsTab->addKnob(_sunkenColor);

    _baseColor =  AppManager::createKnob<KnobColor>(this, tr("Base"), 3);
    _baseColor->setName("base");
    _baseColor->setSimplified(true);
    _guiColorsTab->addKnob(_baseColor);

    _raisedColor =  AppManager::createKnob<KnobColor>(this, tr("Raised"), 3);
    _raisedColor->setName("raised");
    _raisedColor->setSimplified(true);
    _guiColorsTab->addKnob(_raisedColor);

    _selectionColor =  AppManager::createKnob<KnobColor>(this, tr("Selection"), 3);
    _selectionColor->setName("selection");
    _selectionColor->setSimplified(true);
    _guiColorsTab->addKnob(_selectionColor);

    _textColor =  AppManager::createKnob<KnobColor>(this, tr("Text"), 3);
    _textColor->setName("text");
    _textColor->setSimplified(true);
    _guiColorsTab->addKnob(_textColor);

    _altTextColor =  AppManager::createKnob<KnobColor>(this, tr("Unmodified text"), 3);
    _altTextColor->setName("unmodifiedText");
    _altTextColor->setSimplified(true);
    _guiColorsTab->addKnob(_altTextColor);

    _timelinePlayheadColor =  AppManager::createKnob<KnobColor>(this, tr("Timeline playhead"), 3);
    _timelinePlayheadColor->setName("timelinePlayhead");
    _timelinePlayheadColor->setSimplified(true);
    _guiColorsTab->addKnob(_timelinePlayheadColor);


    _timelineBGColor =  AppManager::createKnob<KnobColor>(this, tr("Timeline background"), 3);
    _timelineBGColor->setName("timelineBG");
    _timelineBGColor->setSimplified(true);
    _guiColorsTab->addKnob(_timelineBGColor);

    _timelineBoundsColor =  AppManager::createKnob<KnobColor>(this, tr("Timeline bounds"), 3);
    _timelineBoundsColor->setName("timelineBound");
    _timelineBoundsColor->setSimplified(true);
    _guiColorsTab->addKnob(_timelineBoundsColor);

    _cachedFrameColor =  AppManager::createKnob<KnobColor>(this, tr("Cached frame"), 3);
    _cachedFrameColor->setName("cachedFrame");
    _cachedFrameColor->setSimplified(true);
    _guiColorsTab->addKnob(_cachedFrameColor);

    _diskCachedFrameColor =  AppManager::createKnob<KnobColor>(this, tr("Disk cached frame"), 3);
    _diskCachedFrameColor->setName("diskCachedFrame");
    _diskCachedFrameColor->setSimplified(true);
    _guiColorsTab->addKnob(_diskCachedFrameColor);

    _interpolatedColor =  AppManager::createKnob<KnobColor>(this, tr("Interpolated value"), 3);
    _interpolatedColor->setName("interpValue");
    _interpolatedColor->setSimplified(true);
    _guiColorsTab->addKnob(_interpolatedColor);

    _keyframeColor =  AppManager::createKnob<KnobColor>(this, tr("Keyframe"), 3);
    _keyframeColor->setName("keyframe");
    _keyframeColor->setSimplified(true);
    _guiColorsTab->addKnob(_keyframeColor);

    _trackerKeyframeColor =  AppManager::createKnob<KnobColor>(this, tr("Track User Keyframes"), 3);
    _trackerKeyframeColor->setName("trackUserKeyframe");
    _trackerKeyframeColor->setSimplified(true);
    _guiColorsTab->addKnob(_trackerKeyframeColor);

    _exprColor =  AppManager::createKnob<KnobColor>(this, tr("Expression"), 3);
    _exprColor->setName("exprColor");
    _exprColor->setSimplified(true);
    _guiColorsTab->addKnob(_exprColor);

    _sliderColor =  AppManager::createKnob<KnobColor>(this, tr("Slider"), 3);
    _sliderColor->setName("slider");
    _sliderColor->setSimplified(true);
    _guiColorsTab->addKnob(_sliderColor);
} // Settings::initializeKnobsGuiColors

void
Settings::initializeKnobsCurveEditorColors()
{
    _curveEditorColorsTab = AppManager::createKnob<KnobPage>( this, tr("Curve Editor") );
    _appearanceTab->addKnob(_curveEditorColorsTab);

    _curveEditorBGColor =  AppManager::createKnob<KnobColor>(this, tr("Background color"), 3);
    _curveEditorBGColor->setName("curveEditorBG");
    _curveEditorBGColor->setSimplified(true);
    _curveEditorColorsTab->addKnob(_curveEditorBGColor);

    _gridColor =  AppManager::createKnob<KnobColor>(this, tr("Grid color"), 3);
    _gridColor->setName("curveditorGrid");
    _gridColor->setSimplified(true);
    _curveEditorColorsTab->addKnob(_gridColor);

    _curveEditorScaleColor =  AppManager::createKnob<KnobColor>(this, tr("Scale color"), 3);
    _curveEditorScaleColor->setName("curveeditorScale");
    _curveEditorScaleColor->setSimplified(true);
    _curveEditorColorsTab->addKnob(_curveEditorScaleColor);
}

void
Settings::initializeKnobsDopeSheetColors()
{
    _dopeSheetEditorColorsTab = AppManager::createKnob<KnobPage>( this, tr("Dope Sheet") );
    _appearanceTab->addKnob(_dopeSheetEditorColorsTab);

    _dopeSheetEditorBackgroundColor = AppManager::createKnob<KnobColor>(this, tr("Sheet background color"), 3);
    _dopeSheetEditorBackgroundColor->setName("dopesheetBackground");
    _dopeSheetEditorBackgroundColor->setSimplified(true);
    _dopeSheetEditorColorsTab->addKnob(_dopeSheetEditorBackgroundColor);

    _dopeSheetEditorRootSectionBackgroundColor = AppManager::createKnob<KnobColor>(this, tr("Root section background color"), 4);
    _dopeSheetEditorRootSectionBackgroundColor->setName("dopesheetRootSectionBackground");
    _dopeSheetEditorRootSectionBackgroundColor->setSimplified(true);
    _dopeSheetEditorColorsTab->addKnob(_dopeSheetEditorRootSectionBackgroundColor);

    _dopeSheetEditorKnobSectionBackgroundColor = AppManager::createKnob<KnobColor>(this, tr("Knob section background color"), 4);
    _dopeSheetEditorKnobSectionBackgroundColor->setName("dopesheetKnobSectionBackground");
    _dopeSheetEditorKnobSectionBackgroundColor->setSimplified(true);
    _dopeSheetEditorColorsTab->addKnob(_dopeSheetEditorKnobSectionBackgroundColor);

    _dopeSheetEditorScaleColor = AppManager::createKnob<KnobColor>(this, tr("Sheet scale color"), 3);
    _dopeSheetEditorScaleColor->setName("dopesheetScale");
    _dopeSheetEditorScaleColor->setSimplified(true);
    _dopeSheetEditorColorsTab->addKnob(_dopeSheetEditorScaleColor);

    _dopeSheetEditorGridColor = AppManager::createKnob<KnobColor>(this, tr("Sheet grid color"), 3);
    _dopeSheetEditorGridColor->setName("dopesheetGrid");
    _dopeSheetEditorGridColor->setSimplified(true);
    _dopeSheetEditorColorsTab->addKnob(_dopeSheetEditorGridColor);
}
