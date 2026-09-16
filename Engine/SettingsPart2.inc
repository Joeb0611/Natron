
void
Settings::initializeKnobsNodeGraphColors()
{
    _nodegraphColorsTab = AppManager::createKnob<KnobPage>( this, tr("Node Graph") );
    _appearanceTab->addKnob(_nodegraphColorsTab);

    _usePluginIconsInNodeGraph = AppManager::createKnob<KnobBool>( this, tr("Display plug-in icon on node-graph") );
    _usePluginIconsInNodeGraph->setName("usePluginIcons");
    _usePluginIconsInNodeGraph->setHintToolTip( tr("When checked, each node that has a plug-in icon will display it in the node-graph. "
                                                   "Changing this option will not affect already existing nodes, unless a restart of Natron is made.") );
    _usePluginIconsInNodeGraph->setAddNewLine(false);
    _nodegraphColorsTab->addKnob(_usePluginIconsInNodeGraph);

    _useAntiAliasing = AppManager::createKnob<KnobBool>( this, tr("Anti-Aliasing") );
    _useAntiAliasing->setName("antiAliasing");
    _useAntiAliasing->setHintToolTip( tr("When checked, the node graph will be painted using anti-aliasing. Unchecking it may increase performance. "
                                         " Changing this requires a restart of Natron") );
    _nodegraphColorsTab->addKnob(_useAntiAliasing);


    _defaultNodeColor = AppManager::createKnob<KnobColor>(this, tr("Default node color"), 3);
    _defaultNodeColor->setName("defaultNodeColor");
    _defaultNodeColor->setSimplified(true);
    _defaultNodeColor->setHintToolTip( tr("The default color used for newly created nodes.") );

    _nodegraphColorsTab->addKnob(_defaultNodeColor);


    _defaultBackdropColor =  AppManager::createKnob<KnobColor>(this, tr("Default backdrop color"), 3);
    _defaultBackdropColor->setName("backdropColor");
    _defaultBackdropColor->setSimplified(true);
    _defaultBackdropColor->setHintToolTip( tr("The default color used for newly created backdrop nodes.") );
    _nodegraphColorsTab->addKnob(_defaultBackdropColor);

    _defaultReaderColor =  AppManager::createKnob<KnobColor>(this, tr(PLUGIN_GROUP_IMAGE_READERS), 3);
    _defaultReaderColor->setName("readerColor");
    _defaultReaderColor->setSimplified(true);
    _defaultReaderColor->setHintToolTip( tr("The color used for newly created Reader nodes.") );
    _nodegraphColorsTab->addKnob(_defaultReaderColor);

    _defaultWriterColor =  AppManager::createKnob<KnobColor>(this, tr(PLUGIN_GROUP_IMAGE_WRITERS), 3);
    _defaultWriterColor->setName("writerColor");
    _defaultWriterColor->setSimplified(true);
    _defaultWriterColor->setHintToolTip( tr("The color used for newly created Writer nodes.") );
    _nodegraphColorsTab->addKnob(_defaultWriterColor);

    _defaultGeneratorColor =  AppManager::createKnob<KnobColor>(this, tr("Generators"), 3);
    _defaultGeneratorColor->setName("generatorColor");
    _defaultGeneratorColor->setSimplified(true);
    _defaultGeneratorColor->setHintToolTip( tr("The color used for newly created Generator nodes.") );
    _nodegraphColorsTab->addKnob(_defaultGeneratorColor);

    _defaultColorGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Color group"), 3);
    _defaultColorGroupColor->setName("colorNodesColor");
    _defaultColorGroupColor->setSimplified(true);
    _defaultColorGroupColor->setHintToolTip( tr("The color used for newly created Color nodes.") );
    _nodegraphColorsTab->addKnob(_defaultColorGroupColor);

    _defaultFilterGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Filter group"), 3);
    _defaultFilterGroupColor->setName("filterNodesColor");
    _defaultFilterGroupColor->setSimplified(true);
    _defaultFilterGroupColor->setHintToolTip( tr("The color used for newly created Filter nodes.") );
    _nodegraphColorsTab->addKnob(_defaultFilterGroupColor);

    _defaultTransformGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Transform group"), 3);
    _defaultTransformGroupColor->setName("transformNodesColor");
    _defaultTransformGroupColor->setSimplified(true);
    _defaultTransformGroupColor->setHintToolTip( tr("The color used for newly created Transform nodes.") );
    _nodegraphColorsTab->addKnob(_defaultTransformGroupColor);

    _defaultTimeGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Time group"), 3);
    _defaultTimeGroupColor->setName("timeNodesColor");
    _defaultTimeGroupColor->setSimplified(true);
    _defaultTimeGroupColor->setHintToolTip( tr("The color used for newly created Time nodes.") );
    _nodegraphColorsTab->addKnob(_defaultTimeGroupColor);

    _defaultDrawGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Draw group"), 3);
    _defaultDrawGroupColor->setName("drawNodesColor");
    _defaultDrawGroupColor->setSimplified(true);
    _defaultDrawGroupColor->setHintToolTip( tr("The color used for newly created Draw nodes.") );
    _nodegraphColorsTab->addKnob(_defaultDrawGroupColor);

    _defaultKeyerGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Keyer group"), 3);
    _defaultKeyerGroupColor->setName("keyerNodesColor");
    _defaultKeyerGroupColor->setSimplified(true);
    _defaultKeyerGroupColor->setHintToolTip( tr("The color used for newly created Keyer nodes.") );
    _nodegraphColorsTab->addKnob(_defaultKeyerGroupColor);

    _defaultChannelGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Channel group"), 3);
    _defaultChannelGroupColor->setName("channelNodesColor");
    _defaultChannelGroupColor->setSimplified(true);
    _defaultChannelGroupColor->setHintToolTip( tr("The color used for newly created Channel nodes.") );
    _nodegraphColorsTab->addKnob(_defaultChannelGroupColor);

    _defaultMergeGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Merge group"), 3);
    _defaultMergeGroupColor->setName("defaultMergeColor");
    _defaultMergeGroupColor->setSimplified(true);
    _defaultMergeGroupColor->setHintToolTip( tr("The color used for newly created Merge nodes.") );
    _nodegraphColorsTab->addKnob(_defaultMergeGroupColor);

    _defaultViewsGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Views group"), 3);
    _defaultViewsGroupColor->setName("defaultViewsColor");
    _defaultViewsGroupColor->setSimplified(true);
    _defaultViewsGroupColor->setHintToolTip( tr("The color used for newly created Views nodes.") );
    _nodegraphColorsTab->addKnob(_defaultViewsGroupColor);

    _defaultDeepGroupColor =  AppManager::createKnob<KnobColor>(this, tr("Deep group"), 3);
    _defaultDeepGroupColor->setName("defaultDeepColor");
    _defaultDeepGroupColor->setSimplified(true);
    _defaultDeepGroupColor->setHintToolTip( tr("The color used for newly created Deep nodes.") );
    _nodegraphColorsTab->addKnob(_defaultDeepGroupColor);
} // Settings::initializeKnobsNodeGraphColors

void
Settings::initializeKnobsScriptEditorColors()
{
    _scriptEditorColorsTab = AppManager::createKnob<KnobPage>( this, tr("Script Editor") );
    _scriptEditorColorsTab->setParentKnob(_appearanceTab);

    _scriptEditorFontChoice = AppManager::createKnob<KnobChoice>( this, tr("Font") );
    _scriptEditorFontChoice->setHintToolTip( tr("List of all fonts available on your system") );
    _scriptEditorFontChoice->setName("scriptEditorFont");
    _scriptEditorColorsTab->addKnob(_scriptEditorFontChoice);

    _scriptEditorFontSize = AppManager::createKnob<KnobInt>( this, tr("Font Size") );
    _scriptEditorFontSize->setHintToolTip( tr("The font size") );
    _scriptEditorFontSize->setName("scriptEditorFontSize");
    _scriptEditorColorsTab->addKnob(_scriptEditorFontSize);

    _curLineColor = AppManager::createKnob<KnobColor>(this, tr("Current Line Color"), 3);
    _curLineColor->setName("currentLineColor");
    _curLineColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_curLineColor);

    _keywordColor = AppManager::createKnob<KnobColor>(this, tr("Keyword Color"), 3);
    _keywordColor->setName("keywordColor");
    _keywordColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_keywordColor);

    _operatorColor = AppManager::createKnob<KnobColor>(this, tr("Operator Color"), 3);
    _operatorColor->setName("operatorColor");
    _operatorColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_operatorColor);

    _braceColor = AppManager::createKnob<KnobColor>(this, tr("Brace Color"), 3);
    _braceColor->setName("braceColor");
    _braceColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_braceColor);

    _defClassColor = AppManager::createKnob<KnobColor>(this, tr("Class Def Color"), 3);
    _defClassColor->setName("classDefColor");
    _defClassColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_defClassColor);

    _stringsColor = AppManager::createKnob<KnobColor>(this, tr("Strings Color"), 3);
    _stringsColor->setName("stringsColor");
    _stringsColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_stringsColor);

    _commentsColor = AppManager::createKnob<KnobColor>(this, tr("Comments Color"), 3);
    _commentsColor->setName("commentsColor");
    _commentsColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_commentsColor);

    _selfColor = AppManager::createKnob<KnobColor>(this, tr("Self Color"), 3);
    _selfColor->setName("selfColor");
    _selfColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_selfColor);

    _numbersColor = AppManager::createKnob<KnobColor>(this, tr("Numbers Color"), 3);
    _numbersColor->setName("numbersColor");
    _numbersColor->setSimplified(true);
    _scriptEditorColorsTab->addKnob(_numbersColor);
} // Settings::initializeKnobsScriptEditorColors

void
Settings::initializeKnobsViewers()
{
    _viewersTab = AppManager::createKnob<KnobPage>( this, tr("Viewer") );

    _texturesMode = AppManager::createKnob<KnobChoice>( this, tr("Viewer textures bit depth") );
    _texturesMode->setName("texturesBitDepth");
    std::vector<ChoiceOption> textureModes;
    textureModes.push_back(ChoiceOption("8u",
                                        tr("8-bit").toStdString(),
                                        tr("Post-processing done by the viewer (such as colorspace conversion) is done "
                                           "by the CPU. The size of cached textures is thus smaller.").toStdString() ));

    //textureModes.push_back("16bits half-float");
    //helpStringsTextureModes.push_back("Not available yet. Similar to 32bits fp.");
    textureModes.push_back(ChoiceOption("32f",
                                        tr("32-bit floating-point").toStdString(),
                                        tr("Post-processing done by the viewer (such as colorspace conversion) is done "
                                           "by the GPU, using GLSL. The size of cached textures is thus larger.").toStdString()));
    _texturesMode->populateChoices(textureModes);


    _texturesMode->setHintToolTip( tr("Bit depth of the viewer textures used for rendering. "
                                      "Hover each option with the mouse for a detailed description.") );
    _viewersTab->addKnob(_texturesMode);

    _powerOf2Tiling = AppManager::createKnob<KnobInt>( this, tr("Viewer tile size is 2 to the power of...") );
    _powerOf2Tiling->setName("viewerTiling");
    _powerOf2Tiling->setHintToolTip( tr("The dimension of the viewer tiles is 2^n by 2^n (i.e. 256 by 256 pixels for n=8). "
                                        "A high value means that the viewer renders large tiles, so that "
                                        "rendering is done less often, but on larger areas.") );
    _powerOf2Tiling->disableSlider();
    _powerOf2Tiling->setMinimum(4);
    _powerOf2Tiling->setDisplayMinimum(4);
    _powerOf2Tiling->setMaximum(9);
    _powerOf2Tiling->setDisplayMaximum(9);

    _viewersTab->addKnob(_powerOf2Tiling);

    _checkerboardTileSize = AppManager::createKnob<KnobInt>( this, tr("Checkerboard tile size (pixels)") );
    _checkerboardTileSize->setName("checkerboardTileSize");
    _checkerboardTileSize->setMinimum(1);
    _checkerboardTileSize->setHintToolTip( tr("The size (in screen pixels) of one tile of the checkerboard.") );
    _viewersTab->addKnob(_checkerboardTileSize);

    _checkerboardColor1 = AppManager::createKnob<KnobColor>(this, tr("Checkerboard color 1"), 4);
    _checkerboardColor1->setName("checkerboardColor1");
    _checkerboardColor1->setHintToolTip( tr("The first color used by the checkerboard.") );
    _viewersTab->addKnob(_checkerboardColor1);

    _checkerboardColor2 = AppManager::createKnob<KnobColor>(this, tr("Checkerboard color 2"), 4);
    _checkerboardColor2->setName("checkerboardColor2");
    _checkerboardColor2->setHintToolTip( tr("The second color used by the checkerboard.") );
    _viewersTab->addKnob(_checkerboardColor2);

    _autoWipe = AppManager::createKnob<KnobBool>( this, tr("Automatically enable wipe") );
    _autoWipe->setName("autoWipeForViewer");
    _autoWipe->setHintToolTip( tr("When checked, the wipe tool of the viewer will be automatically enabled "
                                  "when the mouse is hovering the viewer and changing an input of a viewer." ) );
    _viewersTab->addKnob(_autoWipe);


    _autoProxyWhenScrubbingTimeline = AppManager::createKnob<KnobBool>( this, tr("Automatically enable proxy when scrubbing the timeline") );
    _autoProxyWhenScrubbingTimeline->setName("autoProxyScrubbing");
    _autoProxyWhenScrubbingTimeline->setHintToolTip( tr("When checked, the proxy mode will be at least at the level "
                                                        "indicated by the auto-proxy parameter.") );
    _autoProxyWhenScrubbingTimeline->setAddNewLine(false);
    _viewersTab->addKnob(_autoProxyWhenScrubbingTimeline);


    _autoProxyLevel = AppManager::createKnob<KnobChoice>( this, tr("Auto-proxy level") );
    _autoProxyLevel->setName("autoProxyLevel");
    std::vector<ChoiceOption> autoProxyChoices;
    autoProxyChoices.push_back(ChoiceOption("2", "",""));
    autoProxyChoices.push_back(ChoiceOption("4", "",""));
    autoProxyChoices.push_back(ChoiceOption("8", "",""));
    autoProxyChoices.push_back(ChoiceOption("16", "",""));
    autoProxyChoices.push_back(ChoiceOption("32", "",""));
    _autoProxyLevel->populateChoices(autoProxyChoices);

    _viewersTab->addKnob(_autoProxyLevel);

    _maximumNodeViewerUIOpened = AppManager::createKnob<KnobInt>( this, tr("Max. opened node viewer interface") );
    _maximumNodeViewerUIOpened->setName("maxNodeUiOpened");
    _maximumNodeViewerUIOpened->setMinimum(1);
    _maximumNodeViewerUIOpened->disableSlider();
    _maximumNodeViewerUIOpened->setHintToolTip( tr("Controls the maximum amount of nodes that can have their interface showing up at the same time in the viewer") );
    _viewersTab->addKnob(_maximumNodeViewerUIOpened);

    _viewerNumberKeys = AppManager::createKnob<KnobBool>( this, tr("Use number keys for the viewer") );
    _viewerNumberKeys->setName("viewerNumberKeys");
    _viewerNumberKeys->setHintToolTip( tr("When enabled, the row of number keys on the keyboard "
                                    "is used for switching input (<key> connects input to A side, "
                                    "<shift-key> connects input to B side), even if the corresponding "
                                    "character in the current keyboard layout is not a number.\n"
                                    "This may have to be disabled when using a remote display connection "
                                    "to Linux from a different OS.") );
    _viewersTab->addKnob(_viewerNumberKeys);

    _viewerOverlaysPath = AppManager::createKnob<KnobBool>( this, tr("Only display overlays for the viewer render path") );
    _viewerOverlaysPath->setName("viewerOverlaysPath");
    _viewerOverlaysPath->setHintToolTip( tr("When disabled, overlays for all the non-minimized open "
                                            "properties panels are displayed. When enabled, overlays are "
                                            "displayed only for the render path for the current viewer "
                                            "inputs.") );
    _viewersTab->addKnob(_viewerOverlaysPath);
} // Settings::initializeKnobsViewers

void
Settings::initializeKnobsNodeGraph()
{
    /////////// Nodegraph tab
    _nodegraphTab = AppManager::createKnob<KnobPage>( this, tr("Nodegraph") );

    _autoScroll = AppManager::createKnob<KnobBool>( this, tr("Auto Scroll") );
    _autoScroll->setName("autoScroll");
    _autoScroll->setHintToolTip( tr("When checked the node graph will auto scroll if you move a node outside the current graph view.") );
    _nodegraphTab->addKnob(_autoScroll);

    _autoTurbo = AppManager::createKnob<KnobBool>( this, tr("Auto-turbo") );
    _autoTurbo->setName("autoTurbo");
    _autoTurbo->setHintToolTip( tr("When checked the Turbo-mode will be enabled automatically when playback is started and disabled "
                                   "when finished.") );
    _nodegraphTab->addKnob(_autoTurbo);

    _snapNodesToConnections = AppManager::createKnob<KnobBool>( this, tr("Snap to node") );
    _snapNodesToConnections->setName("enableSnapToNode");
    _snapNodesToConnections->setHintToolTip( tr("When moving nodes on the node graph, snap to positions where they are lined up "
                                                "with the inputs and output nodes.") );
    _nodegraphTab->addKnob(_snapNodesToConnections);


    _maxUndoRedoNodeGraph = AppManager::createKnob<KnobInt>( this, tr("Maximum undo/redo for the node graph") );
    _maxUndoRedoNodeGraph->setName("maxUndoRedo");
    _maxUndoRedoNodeGraph->disableSlider();
    _maxUndoRedoNodeGraph->setMinimum(0);
    _maxUndoRedoNodeGraph->setHintToolTip( tr("Set the maximum of events related to the node graph %1 "
                                              "remembers. Past this limit, older events will be deleted forever, "
                                              "allowing to re-use the RAM for other purposes. \n"
                                              "Changing this value will clear the undo/redo stack.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _nodegraphTab->addKnob(_maxUndoRedoNodeGraph);


    _disconnectedArrowLength = AppManager::createKnob<KnobInt>( this, tr("Disconnected arrow length") );
    _disconnectedArrowLength->setName("disconnectedArrowLength");
    _disconnectedArrowLength->setHintToolTip( tr("The size of a disconnected node input arrow in pixels.") );
    _disconnectedArrowLength->disableSlider();

    _nodegraphTab->addKnob(_disconnectedArrowLength);

    _hideOptionalInputsAutomatically = AppManager::createKnob<KnobBool>( this, tr("Auto hide masks inputs") );
    _hideOptionalInputsAutomatically->setName("autoHideInputs");
    _hideOptionalInputsAutomatically->setHintToolTip( tr("When checked, any diconnected mask input of a node in the nodegraph "
                                                         "will be visible only when the mouse is hovering the node or when it is "
                                                         "selected.") );
    _nodegraphTab->addKnob(_hideOptionalInputsAutomatically);

    _useInputAForMergeAutoConnect = AppManager::createKnob<KnobBool>( this, tr("Merge node connect to A input") );
    _useInputAForMergeAutoConnect->setName("mergeConnectToA");
    _useInputAForMergeAutoConnect->setHintToolTip( tr("If checked, upon creation of a new Merge node, or any other node with inputs named "
                                                      "A and B, input A is be preferred "
                                                      "for auto-connection. When the node is disabled, B is always output, whether this is checked or not.") );
    _nodegraphTab->addKnob(_useInputAForMergeAutoConnect);
} // Settings::initializeKnobsNodeGraph

void
Settings::initializeKnobsCaching()
{
    /////////// Caching tab
    _cachingTab = AppManager::createKnob<KnobPage>( this, tr("Caching") );

    _aggressiveCaching = AppManager::createKnob<KnobBool>( this, tr("Aggressive caching") );
    _aggressiveCaching->setName("aggressiveCaching");
    _aggressiveCaching->setHintToolTip( tr("When checked, %1 will cache the output of all images "
                                           "rendered by all nodes, regardless of their \"Force caching\" parameter. When enabling this option "
                                           "you need to have at least 8GiB of RAM, and 16GiB is recommended.\n"
                                           "If not checked, %1 will only cache the  nodes "
                                           "which have multiple outputs, or their parameter \"Force caching\" checked or if one of its "
                                           "output has its settings panel opened.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _cachingTab->addKnob(_aggressiveCaching);

    _maxRAMPercent = AppManager::createKnob<KnobInt>( this, tr("Maximum amount of RAM memory used for caching (% of total RAM)") );
    _maxRAMPercent->setName("maxRAMPercent");
    _maxRAMPercent->disableSlider();
    _maxRAMPercent->setMinimum(0);
    _maxRAMPercent->setMaximum(100);
    QString ramHint( tr("This setting indicates the percentage of the total RAM which can be used by the memory caches. "
                        "This system has %1 of RAM.").arg( printAsRAM( getSystemTotalRAM() ) ) );
    if ( isApplication32Bits() && (getSystemTotalRAM() > 4ULL * 1024ULL * 1024ULL * 1024ULL) ) {
        ramHint.append( QString::fromUtf8("\n") );
        ramHint.append( tr("The version of %1 you are running is 32 bits, which means the available RAM "
                           "is limited to 4GiB. The amount of RAM used for caching is 4GiB * MaxRamPercent.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    }

    _maxRAMPercent->setHintToolTip(ramHint);
    _maxRAMPercent->setAddNewLine(false);
    _cachingTab->addKnob(_maxRAMPercent);

    _maxRAMLabel = AppManager::createKnob<KnobString>( this, std::string() );
    _maxRAMLabel->setName("maxRamLabel");
    _maxRAMLabel->setIsPersistent(false);
    _maxRAMLabel->setAsLabel();
    _cachingTab->addKnob(_maxRAMLabel);


    _unreachableRAMPercent = AppManager::createKnob<KnobInt>( this, tr("System RAM to keep free (% of total RAM)") );
    _unreachableRAMPercent->setName("unreachableRAMPercent");
    _unreachableRAMPercent->disableSlider();
    _unreachableRAMPercent->setMinimum(0);
    _unreachableRAMPercent->setMaximum(90);
    _unreachableRAMPercent->setHintToolTip(tr("This determines how much RAM should be kept free for other applications "
                                              "running on the same system. "
                                              "When this limit is reached, the caches start recycling memory instead of growing. "
                                              //"A reasonable value should be set for it allowing the caches to stay in physical RAM " // users don't understand what swap is
                                              //"and avoid being swapped-out on disk. "
                                              "This value should reflect the amount of memory "
                                              "you want to keep available on your computer for other usage. "
                                              "A low value may result in a massive slowdown and high disk usage.")
                                           );
    _unreachableRAMPercent->setAddNewLine(false);
    _cachingTab->addKnob(_unreachableRAMPercent);
    _unreachableRAMLabel = AppManager::createKnob<KnobString>( this, std::string() );
    _unreachableRAMLabel->setName("unreachableRAMLabel");
    _unreachableRAMLabel->setIsPersistent(false);
    _unreachableRAMLabel->setAsLabel();
    _cachingTab->addKnob(_unreachableRAMLabel);

    _maxViewerDiskCacheGB = AppManager::createKnob<KnobInt>( this, tr("Maximum playback disk cache size (GiB)") );
    _maxViewerDiskCacheGB->setName("maxViewerDiskCache");
    _maxViewerDiskCacheGB->disableSlider();
    _maxViewerDiskCacheGB->setMinimum(0);
    _maxViewerDiskCacheGB->setMaximum(100);
    _maxViewerDiskCacheGB->setHintToolTip( tr("The maximum size that may be used by the playback cache on disk (in GiB)") );
    _cachingTab->addKnob(_maxViewerDiskCacheGB);

    _maxDiskCacheNodeGB = AppManager::createKnob<KnobInt>( this, tr("Maximum DiskCache node disk usage (GiB)") );
    _maxDiskCacheNodeGB->setName("maxDiskCacheNode");
    _maxDiskCacheNodeGB->disableSlider();
    _maxDiskCacheNodeGB->setMinimum(0);
    _maxDiskCacheNodeGB->setMaximum(100);
    _maxDiskCacheNodeGB->setHintToolTip( tr("The maximum size that may be used by the DiskCache node on disk (in GiB)") );
    _cachingTab->addKnob(_maxDiskCacheNodeGB);

    _minFreeDiskGiB = AppManager::createKnob<KnobInt>( this, tr("Minimum free disk to keep (GiB)") );
    _minFreeDiskGiB->setName("minFreeDiskGiB");
    _minFreeDiskGiB->disableSlider();
    _minFreeDiskGiB->setMinimum(0);
    _minFreeDiskGiB->setMaximum(1000000);
    _minFreeDiskGiB->setHintToolTip( tr("Refuse to grow the disk cache if doing so would leave less than this much free space. "
                                        "Prevents filling the volume and crashing on ENOSPC.") );
    _cachingTab->addKnob(_minFreeDiskGiB);


    _diskCachePath = AppManager::createKnob<KnobPath>( this, tr("Disk cache path") );
    _diskCachePath->setName("diskCachePath");
    _diskCachePath->setMultiPath(false);

    QString defaultLocation = StandardPaths::writableLocation(StandardPaths::eStandardLocationCache);
    QString diskCacheTt( tr("WARNING: Changing this parameter requires a restart of the application. \n"
                            "This points to the location where %1 on-disk caches will be. "
                            "This variable should point to your fastest disk. This parameter can be "
                            "overridden by the value of the environment variable %2.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).arg( QString::fromUtf8(NATRON_DISK_CACHE_PATH_ENV_VAR) ) );

    QString diskCacheTt2( tr("If the parameter is left empty or the location set is invalid, "
                             "the default location will be used. The default location is: %1").arg(defaultLocation) );

    _diskCachePath->setHintToolTip( diskCacheTt + QLatin1Char('\n') + diskCacheTt2 );
    _cachingTab->addKnob(_diskCachePath);

    _wipeDiskCache = AppManager::createKnob<KnobButton>( this, tr("Wipe Disk Cache") );
    _wipeDiskCache->setHintToolTip( tr("Cleans-up all caches, deleting all folders that may contain cached data. "
                                       "This is provided in case %1 lost track of cached images "
                                       "for some reason.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _cachingTab->addKnob(_wipeDiskCache);
} // Settings::initializeKnobsCaching

void
Settings::initializeKnobsPlugins()
{
    _pluginsTab = AppManager::createKnob<KnobPage>( this, tr("Plug-ins") );
    _pluginsTab->setName("plugins");

#if defined(__linux__) || defined(__FreeBSD__)
    std::string searchPath("/usr/OFX/Plugins");
#elif defined(__APPLE__)
    std::string searchPath("/Library/OFX/Plugins");
#elif defined(WINDOWS)

    std::wstring basePath = std::wstring( OFX::Host::PluginCache::getStdOFXPluginPath() );
    basePath.append( std::wstring(L" and C:\\Program Files\\Common Files\\OFX\\Plugins") );
    std::string searchPath = StrUtils::utf16_to_utf8(basePath);

#endif

    _loadBundledPlugins = AppManager::createKnob<KnobBool>( this, tr("Use bundled plug-ins") );
    _loadBundledPlugins->setName("useBundledPlugins");
    _loadBundledPlugins->setHintToolTip( tr("When checked, %1 also uses the plug-ins bundled "
                                            "with the binary distribution.\n"
                                            "When unchecked, only system-wide plug-ins found in are loaded (more information can be "
                                            "found in the help for the \"Extra plug-ins search paths\" setting).").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _pluginsTab->addKnob(_loadBundledPlugins);

    _preferBundledPlugins = AppManager::createKnob<KnobBool>( this, tr("Prefer bundled plug-ins over system-wide plug-ins") );
    _preferBundledPlugins->setName("preferBundledPlugins");
    _preferBundledPlugins->setHintToolTip( tr("When checked, and if \"Use bundled plug-ins\" is also checked, plug-ins bundled with the %1 binary distribution will take precedence over system-wide plug-ins "
                                              "if they have the same internal ID.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _pluginsTab->addKnob(_preferBundledPlugins);

    _useStdOFXPluginsLocation = AppManager::createKnob<KnobBool>( this, tr("Enable default OpenFX plugins location") );
    _useStdOFXPluginsLocation->setName("useStdOFXPluginsLocation");
    _useStdOFXPluginsLocation->setHintToolTip( tr("When checked, %1 also uses the OpenFX plug-ins found in the default location (%2).").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).arg( QString::fromUtf8( searchPath.c_str() ) ) );
    _pluginsTab->addKnob(_useStdOFXPluginsLocation);

    _extraPluginPaths = AppManager::createKnob<KnobPath>( this, tr("OpenFX plug-ins search path") );
    _extraPluginPaths->setName("extraPluginsSearchPaths");
    _extraPluginPaths->setHintToolTip( tr("Extra search paths where %1 should scan for OpenFX plug-ins. "
                                          "Extra plug-ins search paths can also be specified using the OFX_PLUGIN_PATH environment variable.\n"
                                          "The priority order for system-wide plug-ins, from high to low, is:\n"
                                          "- plugins bundled with the binary distribution of %1 (if \"Prefer bundled plug-ins over "
                                          "system-wide plug-ins\" is checked)\n"
                                          "- plug-ins found in OFX_PLUGIN_PATH\n"
                                          "- plug-ins found in %2 (if \"Enable default OpenFX plug-ins location\" is checked)\n"
                                          "- plugins bundled with the binary distribution of %1 (if \"Prefer bundled plug-ins over "
                                          "system-wide plug-ins\" is not checked)\n"
                                          "Any change will take effect on the next launch of %1.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).arg( QString::fromUtf8( searchPath.c_str() ) ) );
    _extraPluginPaths->setMultiPath(true);
    _pluginsTab->addKnob(_extraPluginPaths);


    _templatesPluginPaths = AppManager::createKnob<KnobPath>( this, tr("PyPlugs search path") );
    _templatesPluginPaths->setName("groupPluginsSearchPath");
    _templatesPluginPaths->setHintToolTip( tr("Search path where %1 should scan for Python group scripts (PyPlugs). "
                                              "The search paths for groups can also be specified using the "
                                              "NATRON_PLUGIN_PATH environment variable.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _templatesPluginPaths->setMultiPath(true);
    _pluginsTab->addKnob(_templatesPluginPaths);

} // Settings::initializeKnobsPlugins

void
Settings::initializeKnobsPython()
{
    _pythonPage = AppManager::createKnob<KnobPage>( this, tr("Python") );


    _onProjectCreated = AppManager::createKnob<KnobString>( this, tr("After project created") );
    _onProjectCreated->setName("afterProjectCreated");
    _onProjectCreated->setHintToolTip( tr("Callback called once a new project is created (this is never called "
                                          "when \"After project loaded\" is called.)\n"
                                          "The signature of the callback is: callback(app) where:\n"
                                          "- app: points to the current application instance\n") );
    _pythonPage->addKnob(_onProjectCreated);


    _defaultOnProjectLoaded = AppManager::createKnob<KnobString>( this, tr("Default after project loaded") );
    _defaultOnProjectLoaded->setName("defOnProjectLoaded");
    _defaultOnProjectLoaded->setHintToolTip( tr("The default afterProjectLoad callback that will be set for new projects.") );
    _pythonPage->addKnob(_defaultOnProjectLoaded);

    _defaultOnProjectSave = AppManager::createKnob<KnobString>( this, tr("Default before project save") );
    _defaultOnProjectSave->setName("defOnProjectSave");
    _defaultOnProjectSave->setHintToolTip( tr("The default beforeProjectSave callback that will be set for new projects.") );
    _pythonPage->addKnob(_defaultOnProjectSave);


    _defaultOnProjectClose = AppManager::createKnob<KnobString>( this, tr("Default before project close") );
    _defaultOnProjectClose->setName("defOnProjectClose");
    _defaultOnProjectClose->setHintToolTip( tr("The default beforeProjectClose callback that will be set for new projects.") );
    _pythonPage->addKnob(_defaultOnProjectClose);


    _defaultOnNodeCreated = AppManager::createKnob<KnobString>( this, tr("Default after node created") );
    _defaultOnNodeCreated->setName("defOnNodeCreated");
    _defaultOnNodeCreated->setHintToolTip( tr("The default afterNodeCreated callback that will be set for new projects.") );
    _pythonPage->addKnob(_defaultOnNodeCreated);


    _defaultOnNodeDelete = AppManager::createKnob<KnobString>( this, tr("Default before node removal") );
    _defaultOnNodeDelete->setName("defOnNodeDelete");
    _defaultOnNodeDelete->setHintToolTip( tr("The default beforeNodeRemoval callback that will be set for new projects.") );
    _pythonPage->addKnob(_defaultOnNodeDelete);

    _loadPyPlugsFromPythonScript = AppManager::createKnob<KnobBool>( this, tr("Load PyPlugs in projects from .py if possible") );
    _loadPyPlugsFromPythonScript->setName("loadFromPyFile");
    _loadPyPlugsFromPythonScript->setHintToolTip( tr("When checked, if a project contains a PyPlug, it will try to first load the PyPlug "
                                                     "from the .py file. If the version of the PyPlug has changed Natron will ask you "
                                                     "whether you want to upgrade to the new version of the PyPlug in your project. "
                                                     "If the .py file is not found, it will fallback to the same behavior "
                                                     "as when this option is unchecked. When unchecked the PyPlug will load as a regular group "
                                                     "with the information embedded in the project file.") );
    _loadPyPlugsFromPythonScript->setDefaultValue(true);
    _pythonPage->addKnob(_loadPyPlugsFromPythonScript);

    _echoVariableDeclarationToPython = AppManager::createKnob<KnobBool>( this, tr("Print auto-declared variables in the Script Editor") );
    _echoVariableDeclarationToPython->setName("printAutoDeclaredVars");
    _echoVariableDeclarationToPython->setHintToolTip( tr("When checked, %1 will print in the Script Editor all variables that are "
                                                         "automatically declared, such as the app variable or node attributes.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _pythonPage->addKnob(_echoVariableDeclarationToPython);
} // initializeKnobs

void
Settings::setCachingLabels()
{
    int maxTotalRam = _maxRAMPercent->getValue();
    U64 systemTotalRam = getSystemTotalRAM();
    U64 maxRAM = (U64)( ( (double)maxTotalRam / 100. ) * systemTotalRam );

    _maxRAMLabel->setValue( printAsRAM(maxRAM).toStdString() );
    _unreachableRAMLabel->setValue( printAsRAM( (double)systemTotalRam * ( (double)_unreachableRAMPercent->getValue() / 100. ) ).toStdString() );
}
