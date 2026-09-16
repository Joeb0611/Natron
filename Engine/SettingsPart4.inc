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

QString
Settings::makeHTMLDocumentation(bool genHTML) const
{
    QString ret;
    QString markdown;
    QTextStream ts(&ret);
    QTextStream ms(&markdown);

    ms << ( QString::fromUtf8("<!--") + tr("Do not edit this file! It is generated automatically by %1 itself.").arg ( QString::fromUtf8( NATRON_APPLICATION_NAME) ) + QString::fromUtf8(" -->\n\n") );
    ms << tr("Preferences") << "\n==========\n\n";

    const KnobsVec& knobs = getKnobs_mt_safe();
    for (KnobsVec::const_iterator it = knobs.begin(); it != knobs.end(); ++it) {
        if ( (*it)->getDefaultIsSecret() ) {
            continue;
        }
        //QString knobScriptName = QString::fromUtf8( (*it)->getName().c_str() );
        QString knobLabel = convertFromPlainTextToMarkdown( QString::fromStdString( (*it)->getLabel() ), genHTML, false );
        QString knobHint = convertFromPlainTextToMarkdown( QString::fromStdString( (*it)->getHintToolTip() ), genHTML, false );
        KnobPage* isPage = dynamic_cast<KnobPage*>( it->get() );
        KnobSeparator* isSep = dynamic_cast<KnobSeparator*>( it->get() );
        if (isPage) {
            if (isPage->getParentKnob()) {
                ms << "### " << knobLabel << "\n\n";
            } else {
                ms << knobLabel << "\n----------\n\n";
            }
        } else if (isSep) {
            ms << "**" << knobLabel << "**\n\n";
        } else if ( !knobLabel.isEmpty() && !knobHint.isEmpty() ) {
            if ( ( knobLabel != QString::fromUtf8("Enabled") ) && ( knobLabel != QString::fromUtf8("Zoom support") ) ) {
                ms << "**" << knobLabel << "**\n\n";
                ms << knobHint << "\n\n";
            }
        }
    }

    if (genHTML) {
        QString prefs_head =
        QString::fromUtf8(
        "<!DOCTYPE html>\n"
        "<!--[if IE 8]><html class=\"no-js lt-ie9\" lang=\"en\" > <![endif]-->\n"
        "<!--[if gt IE 8]><!--> <html class=\"no-js\" lang=\"en\" > <!--<![endif]-->\n"
        "<head>\n"
        "<meta charset=\"utf-8\">\n"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "<title>Preferences &mdash; NATRON_DOCUMENTATION</title>\n"
        "<script type=\"text/javascript\" src=\"_static/js/modernizr.min.js\"></script>\n"
        "<script type=\"text/javascript\" id=\"documentation_options\" data-url_root=\"./\" src=\"_static/documentation_options.js\"></script>\n"
        "<script src=\"_static/jquery.js\"></script>\n"
        "<script src=\"_static/underscore.js\"></script>\n"
        "<script src=\"_static/doctools.js\"></script>\n"
        "<script src=\"_static/language_data.js\"></script>\n"
        "<script async=\"async\" src=\"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/latest.js?config=TeX-AMS-MML_HTMLorMML\"></script>\n"
        "<script type=\"text/javascript\" src=\"_static/js/theme.js\"></script>\n"
        "<link rel=\"stylesheet\" href=\"_static/css/theme.css\" type=\"text/css\" />\n"
        "<link rel=\"stylesheet\" href=\"_static/pygments.css\" type=\"text/css\" />\n"
        "<link rel=\"stylesheet\" href=\"_static/theme_overrides.css\" type=\"text/css\" />\n"
        "<link rel=\"index\" title=\"Index\" href=\"genindex.html\" />\n"
        "<link rel=\"search\" title=\"Search\" href=\"search.html\" />\n"
        "<link rel=\"next\" title=\"Environment Variables\" href=\"_environment.html\" />\n"
        "<link rel=\"prev\" title=\"Reference Guide\" href=\"_group.html\" />\n"
        "</head>\n"
        "<body class=\"wy-body-for-nav\">\n"
        "<div class=\"wy-grid-for-nav\">\n"
        "<nav data-toggle=\"wy-nav-shift\" class=\"wy-nav-side\">\n"
        "<div class=\"wy-side-scroll\">\n"
        "<div class=\"wy-side-nav-search\" >\n"
        "<a href=\"index.html\" class=\"icon icon-home\"> Natron\n"
        "<img src=\"_static/logo.png\" class=\"logo\" alt=\"Logo\"/>\n"
        "</a>\n"
        "<div class=\"version\">\n"
        "2.3\n"
        "</div>\n"
        "<div role=\"search\">\n"
        "<form id=\"rtd-search-form\" class=\"wy-form\" action=\"search.html\" method=\"get\">\n"
        "<input type=\"text\" name=\"q\" placeholder=\"Search docs\" />\n"
        "<input type=\"hidden\" name=\"check_keywords\" value=\"yes\" />\n"
        "<input type=\"hidden\" name=\"area\" value=\"default\" />\n"
        "</form>\n"
        "</div>\n"
        "</div>\n"
        "<div class=\"wy-menu wy-menu-vertical\" data-spy=\"affix\" role=\"navigation\" aria-label=\"main navigation\">\n"
        "<ul class=\"current\">\n"
        "<li class=\"toctree-l1\"><a class=\"reference internal\" href=\"guide/index.html\">User Guide</a></li>\n"
        "<li class=\"toctree-l1 current\"><a class=\"reference internal\" href=\"_group.html\">Reference Guide</a><ul class=\"current\">\n"
        "<li class=\"toctree-l2 current\"><a class=\"current reference internal\" href=\"#\">Preferences</a><ul>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#general\">General</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#threading\">Threading</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#rendering\">Rendering</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#gpu-rendering\">GPU Rendering</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#project-setup\">Project Setup</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#documentation\">Documentation</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#user-interface\">User Interface</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#color-management\">Color Management</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#caching\">Caching</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#viewer\">Viewer</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#nodegraph\">Nodegraph</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#plug-ins\">Plug-ins</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#python\">Python</a></li>\n"
        "<li class=\"toctree-l3\"><a class=\"reference internal\" href=\"#appearance\">Appearance</a><ul>\n"
        "<li class=\"toctree-l4\"><a class=\"reference internal\" href=\"#main-window\">Main Window</a></li>\n"
        "<li class=\"toctree-l4\"><a class=\"reference internal\" href=\"#curve-editor\">Curve Editor</a></li>\n"
        "<li class=\"toctree-l4\"><a class=\"reference internal\" href=\"#dope-sheet\">Dope Sheet</a></li>\n"
        "<li class=\"toctree-l4\"><a class=\"reference internal\" href=\"#node-graph\">Node Graph</a></li>\n"
        "<li class=\"toctree-l4\"><a class=\"reference internal\" href=\"#script-editor\">Script Editor</a></li>\n"
        "</ul>\n"
        "</li>\n"
        "</ul>\n"
        "</li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_environment.html\">Environment Variables</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupImage.html\">Image nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupDraw.html\">Draw nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupTime.html\">Time nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupChannel.html\">Channel nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupColor.html\">Color nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupFilter.html\">Filter nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupKeyer.html\">Keyer nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupMerge.html\">Merge nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupTransform.html\">Transform nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupViews.html\">Views nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupOther.html\">Other nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupGMIC.html\">GMIC nodes</a></li>\n"
        "<li class=\"toctree-l2\"><a class=\"reference internal\" href=\"_groupExtra.html\">Extra nodes</a></li>\n"
        "</ul>\n"
        "</li>\n"
        "<li class=\"toctree-l1\"><a class=\"reference internal\" href=\"devel/index.html\">Developers Guide</a></li>\n"
        "</ul>\n"
        "</div>\n"
        "</div>\n"
        "</nav>\n"
        "<section data-toggle=\"wy-nav-shift\" class=\"wy-nav-content-wrap\">\n"
        "<nav class=\"wy-nav-top\" aria-label=\"top navigation\">\n"
        "<i data-toggle=\"wy-nav-top\" class=\"fa fa-bars\"></i>\n"
        "<a href=\"index.html\">Natron</a>\n"
        "</nav>\n"
        "<div class=\"wy-nav-content\">\n"
        "<div class=\"rst-content\">\n"
        "<div role=\"navigation\" aria-label=\"breadcrumbs navigation\">\n"
        "<ul class=\"wy-breadcrumbs\">\n"
        "<li><a href=\"index.html\">Docs</a> &raquo;</li>\n"
        "<li><a href=\"_group.html\">Reference Guide</a> &raquo;</li>\n"
        "<li>Preferences</li>\n"
        "<li class=\"wy-breadcrumbs-aside\">\n"
        "<a href=\"_sources/_prefs.rst.txt\" rel=\"nofollow\"> View page source</a>\n"
        "</li>\n"
        "</ul>\n"
        "<hr/>\n"
        "</div>\n"
        "<div role=\"main\" class=\"document\" itemscope=\"itemscope\" itemtype=\"http://schema.org/Article\">\n"
        "<div itemprop=\"articleBody\">\n"
        "<div class=\"section\" id=\"preferences\">\n");

        QString prefs_foot =
        QString::fromUtf8(
        "</div>\n"
        "</div>\n"
        "</div>\n"
        "<footer>\n"
        "<div class=\"rst-footer-buttons\" role=\"navigation\" aria-label=\"footer navigation\">\n"
        "<a href=\"_environment.html\" class=\"btn btn-neutral float-right\" title=\"Environment Variables\" accesskey=\"n\" rel=\"next\">Next <span class=\"fa fa-arrow-circle-right\"></span></a>\n"
        "<a href=\"_group.html\" class=\"btn btn-neutral float-left\" title=\"Reference Guide\" accesskey=\"p\" rel=\"prev\"><span class=\"fa fa-arrow-circle-left\"></span> Previous</a>\n"
        "</div>\n"
        "<hr/>\n"
        "<div role=\"contentinfo\">\n"
        "<p>\n"
        "&copy; Copyright 2013-2023 The Natron documentation authors, licensed under CC BY-SA 4.0\n"
        "</p>\n"
        "</div>\n"
        "Built with <a href=\"http://sphinx-doc.org/\">Sphinx</a> using a <a href=\"https://github.com/rtfd/sphinx_rtd_theme\">theme</a> provided by <a href=\"https://readthedocs.org\">Read the Docs</a>.\n"
        "</footer>\n"
        "</div>\n"
        "</div>\n"
        "</section>\n"
        "</div>\n"
        "<script type=\"text/javascript\">\n"
        "jQuery(function () {\n"
        "SphinxRtdTheme.Navigation.enable(true);\n"
        "});\n"
        "</script>\n"
        "</body>\n"
        "</html>\n");

        ts << prefs_head;
        QString html = Markdown::convert2html(markdown);
        ts << Markdown::fixSettingsHTML(html);
        ts << prefs_foot;
    } else {
        ts << markdown;
    }

    return ret;
} // Settings::makeHTMLDocumentation

void
Settings::populateSystemFonts(const QSettings& settings,
                              const std::vector<std::string>& fonts)
{
    std::vector<ChoiceOption> options(fonts.size());
    for (std::size_t i = 0; i < fonts.size(); ++i) {
        options[i].id = fonts[i];
    }
    _systemFontChoice->populateChoices(options);
    _scriptEditorFontChoice->populateChoices(options);

    for (U32 i = 0; i < fonts.size(); ++i) {
        if (fonts[i] == NATRON_FONT) {
            _systemFontChoice->setDefaultValue(i);
        }
        if (fonts[i] == NATRON_SCRIPT_FONT) {
            _scriptEditorFontChoice->setDefaultValue(i);
        }
    }
    ///Now restore properly the system font choice
    {
        QString name = QString::fromUtf8( _systemFontChoice->getName().c_str() );
        if ( settings.contains(name) ) {
            std::string value = settings.value(name).toString().toStdString();
            for (U32 i = 0; i < fonts.size(); ++i) {
                if (fonts[i] == value) {
                    _systemFontChoice->setValue(i);
                    break;
                }
            }
        }
    }
    {
        QString name = QString::fromUtf8( _scriptEditorFontChoice->getName().c_str() );
        if ( settings.contains(name) ) {
            std::string value = settings.value(name).toString().toStdString();
            for (U32 i = 0; i < fonts.size(); ++i) {
                if (fonts[i] == value) {
                    _scriptEditorFontChoice->setValue(i);
                    break;
                }
            }
        }
    }
}

void
Settings::getOpenFXPluginsSearchPaths(std::list<std::string>* paths) const
{
    assert(paths);
    try {
        _extraPluginPaths->getPaths(paths);
    } catch (std::logic_error&) {
        paths->clear();
    }
}

bool
Settings::getUseStdOFXPluginsLocation() const
{
    return _useStdOFXPluginsLocation->getValue();
}

void
Settings::restoreDefault()
{
    QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );

    if ( !QFile::remove( settings.fileName() ) ) {
        qDebug() << "Failed to remove settings ( " << settings.fileName() << " ).";
    }

    beginChanges();
    const KnobsVec & knobs = getKnobs();
    for (U32 i = 0; i < knobs.size(); ++i) {
        for (int j = 0; j < knobs[i]->getDimension(); ++j) {
            knobs[i]->resetToDefaultValue(j);
        }
    }
    setCachingLabels();
    endChanges();
}

bool
Settings::isRenderInSeparatedProcessEnabled() const
{
    return _renderInSeparateProcess->getValue();
}

int
Settings::getMaximumUndoRedoNodeGraph() const
{
    return _maxUndoRedoNodeGraph->getValue();
}

int
Settings::getAutoSaveDelayMS() const
{
    return _autoSaveDelay->getValue() * 1000;
}

bool
Settings::isAutoSaveEnabledForUnsavedProjects() const
{
    return _autoSaveUnSavedProjects->getValue();
}

int
Settings::saveVersions() const
{
    return _saveVersions->getValue();
}


bool
Settings::isSnapToNodeEnabled() const
{
    return _snapNodesToConnections->getValue();
}

bool
Settings::isCheckForUpdatesEnabled() const
{
    return _checkForUpdates->getValue();
}

void
Settings::setCheckUpdatesEnabled(bool enabled)
{
    _checkForUpdates->setValue(enabled);
    saveSetting( _checkForUpdates.get() );
}

#ifdef NATRON_USE_BREAKPAD
bool
Settings::isCrashReportingEnabled() const
{
    return _enableCrashReports->getValue();
}
#endif

int
Settings::getMaxPanelsOpened() const
{
    return _maxPanelsOpened->getValue();
}

void
Settings::setMaxPanelsOpened(int maxPanels)
{
    _maxPanelsOpened->setValue(maxPanels);
    saveSetting( _maxPanelsOpened.get() );
}

bool
Settings::loadBundledPlugins() const
{
    return _loadBundledPlugins->getValue();
}

bool
Settings::preferBundledPlugins() const
{
    return _preferBundledPlugins->getValue();
}

void
Settings::getDefaultNodeColor(float *r,
                              float *g,
                              float *b) const
{
    *r = _defaultNodeColor->getValue(0);
    *g = _defaultNodeColor->getValue(1);
    *b = _defaultNodeColor->getValue(2);
}

void
Settings::getDefaultBackdropColor(float *r,
                                  float *g,
                                  float *b) const
{
    *r = _defaultBackdropColor->getValue(0);
    *g = _defaultBackdropColor->getValue(1);
    *b = _defaultBackdropColor->getValue(2);
}

void
Settings::getGeneratorColor(float *r,
                            float *g,
                            float *b) const
{
    *r = _defaultGeneratorColor->getValue(0);
    *g = _defaultGeneratorColor->getValue(1);
    *b = _defaultGeneratorColor->getValue(2);
}

void
Settings::getReaderColor(float *r,
                         float *g,
                         float *b) const
{
    *r = _defaultReaderColor->getValue(0);
    *g = _defaultReaderColor->getValue(1);
    *b = _defaultReaderColor->getValue(2);
}

void
Settings::getWriterColor(float *r,
                         float *g,
                         float *b) const
{
    *r = _defaultWriterColor->getValue(0);
    *g = _defaultWriterColor->getValue(1);
    *b = _defaultWriterColor->getValue(2);
}

void
Settings::getColorGroupColor(float *r,
                             float *g,
                             float *b) const
{
    *r = _defaultColorGroupColor->getValue(0);
    *g = _defaultColorGroupColor->getValue(1);
    *b = _defaultColorGroupColor->getValue(2);
}

void
Settings::getFilterGroupColor(float *r,
                              float *g,
                              float *b) const
{
    *r = _defaultFilterGroupColor->getValue(0);
    *g = _defaultFilterGroupColor->getValue(1);
    *b = _defaultFilterGroupColor->getValue(2);
}

void
Settings::getTransformGroupColor(float *r,
                                 float *g,
                                 float *b) const
{
    *r = _defaultTransformGroupColor->getValue(0);
    *g = _defaultTransformGroupColor->getValue(1);
    *b = _defaultTransformGroupColor->getValue(2);
}

void
Settings::getTimeGroupColor(float *r,
                            float *g,
                            float *b) const
{
    *r = _defaultTimeGroupColor->getValue(0);
    *g = _defaultTimeGroupColor->getValue(1);
    *b = _defaultTimeGroupColor->getValue(2);
}

void
Settings::getDrawGroupColor(float *r,
                            float *g,
                            float *b) const
{
    *r = _defaultDrawGroupColor->getValue(0);
    *g = _defaultDrawGroupColor->getValue(1);
    *b = _defaultDrawGroupColor->getValue(2);
}

void
Settings::getKeyerGroupColor(float *r,
                             float *g,
                             float *b) const
{
    *r = _defaultKeyerGroupColor->getValue(0);
    *g = _defaultKeyerGroupColor->getValue(1);
    *b = _defaultKeyerGroupColor->getValue(2);
}

void
Settings::getChannelGroupColor(float *r,
                               float *g,
                               float *b) const
{
    *r = _defaultChannelGroupColor->getValue(0);
    *g = _defaultChannelGroupColor->getValue(1);
    *b = _defaultChannelGroupColor->getValue(2);
}

void
Settings::getMergeGroupColor(float *r,
                             float *g,
                             float *b) const
{
    *r = _defaultMergeGroupColor->getValue(0);
    *g = _defaultMergeGroupColor->getValue(1);
    *b = _defaultMergeGroupColor->getValue(2);
}

void
Settings::getViewsGroupColor(float *r,
                             float *g,
                             float *b) const
{
    *r = _defaultViewsGroupColor->getValue(0);
    *g = _defaultViewsGroupColor->getValue(1);
    *b = _defaultViewsGroupColor->getValue(2);
}

void
Settings::getDeepGroupColor(float *r,
                            float *g,
                            float *b) const
{
    *r = _defaultDeepGroupColor->getValue(0);
    *g = _defaultDeepGroupColor->getValue(1);
    *b = _defaultDeepGroupColor->getValue(2);
}

int
Settings::getDisconnectedArrowLength() const
{
    return _disconnectedArrowLength->getValue();
}

std::string
Settings::getHostName() const
{
    int entry_i =  _hostName->getValue();
    std::vector<ChoiceOption> entries = _hostName->getEntries_mt_safe();

    if ( (entry_i >= 0) && ( entry_i < (int)entries.size() ) && (entries[entry_i].id == NATRON_CUSTOM_HOST_NAME_ENTRY) ) {
        return _customHostName->getValue();
    } else {
        if ( (entry_i >= 0) && ( entry_i < (int)_knownHostNames.size() ) ) {
            return _knownHostNames[entry_i].id;
        }

        return std::string();
    }
}

bool
Settings::getRenderOnEditingFinishedOnly() const
{
    return _renderOnEditingFinished->getValue();
}

void
Settings::setRenderOnEditingFinishedOnly(bool render)
{
    _renderOnEditingFinished->setValue(render);
}

bool
Settings::getIconsBlackAndWhite() const
{
    return _useBWIcons->getValue();
}

std::string
Settings::getDefaultLayoutFile() const
{
    return _defaultLayoutFile->getValue();
}

bool
Settings::getLoadProjectWorkspce() const
{
    return _loadProjectsWorkspace->getValue();
}

#ifdef Q_OS_WIN
bool
Settings::getEnableConsoleWindow() const
{
    return _enableConsoleWindow->getValue();
}
#endif

bool
Settings::useCursorPositionIncrements() const
{
    return _useCursorPositionIncrements->getValue();
}

bool
Settings::isAutoProjectFormatEnabled() const
{
    return _firstReadSetProjectFormat->getValue();
}

bool
Settings::isAutoFixRelativeFilePathEnabled() const
{
    return _fixPathsOnProjectPathChanged->getValue();
}

int
Settings::getNumberOfParallelRenders() const
{
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL

    return _numberOfParallelRenders->getValue();
#else

    return 1;
#endif
}

void
Settings::setNumberOfParallelRenders(int nb)
{
#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    _numberOfParallelRenders->setValue(nb);
#endif
}

bool
Settings::areRGBPixelComponentsSupported() const
{
    return _activateRGBSupport->getValue();
}

bool
Settings::isTransformConcatenationEnabled() const
{
    return _activateTransformConcatenationSupport->getValue();
}

bool
Settings::useGlobalThreadPool() const
{
    return _useThreadPool->getValue();
}

void
Settings::setUseGlobalThreadPool(bool use)
{
    _useThreadPool->setValue(use);
}

bool
Settings::useInputAForMergeAutoConnect() const
{
    return _useInputAForMergeAutoConnect->getValue();
}

void
Settings::doOCIOStartupCheckIfNeeded()
{
    bool docheck = _ocioStartupCheck->getValue();
    AppInstancePtr mainInstance = appPTR->getTopLevelInstance();

    if (!mainInstance) {
        qDebug() << "WARNING: doOCIOStartupCheckIfNeeded() called without a AppInstance";

        return;
    }

    if (docheck && mainInstance) {
        int entry_i = _ocioConfigKnob->getValue();
        std::vector<ChoiceOption> entries = _ocioConfigKnob->getEntries_mt_safe();
        std::string warnText;
        if ( (entry_i < 0) || ( entry_i >= (int)entries.size() ) ) {
            warnText = tr("The current OCIO config selected in the preferences is invalid, would you like to set it to the default config (%1)?").arg( QString::fromUtf8(NATRON_DEFAULT_OCIO_CONFIG_NAME) ).toStdString();
        } else if (entries[entry_i].id != NATRON_DEFAULT_OCIO_CONFIG_NAME) {
            warnText = tr("The current OCIO config selected in the preferences is not the default one (%1), would you like to set it to the default config?").arg( QString::fromUtf8(NATRON_DEFAULT_OCIO_CONFIG_NAME) ).toStdString();
        } else {
            return;
        }

        bool stopAsking = false;
        StandardButtonEnum reply = mainInstance->questionDialog("OCIO config", warnText, false,
                                                                StandardButtons(eStandardButtonYes | eStandardButtonNo),
                                                                eStandardButtonYes,
                                                                &stopAsking);
        if (stopAsking != !docheck) {
            _ocioStartupCheck->setValue(!stopAsking);
            saveSetting( _ocioStartupCheck.get() );
        }

        if (reply == eStandardButtonYes) {
            int defaultIndex = -1;
            for (unsigned i = 0; i < entries.size(); ++i) {
                if (entries[i].id.find(NATRON_DEFAULT_OCIO_CONFIG_NAME) != std::string::npos) {
                    defaultIndex = i;
                    break;
                }
            }
            if (defaultIndex != -1) {
                _ocioConfigKnob->setValue(defaultIndex);
                saveSetting( _ocioConfigKnob.get() );
            } else {
                Dialogs::warningDialog( "OCIO config", tr("The %2 OCIO config could not be found.\n"
                                                          "This is probably because you're not using the OpenColorIO-Configs folder that should "
                                                          "be bundled with your %1 installation.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).arg( QString::fromUtf8(NATRON_DEFAULT_OCIO_CONFIG_NAME) ).toStdString() );
            }
        }
    }
} // Settings::doOCIOStartupCheckIfNeeded

bool
Settings::didSettingsExistOnStartup() const
{
    return _settingsExisted;
}

bool
Settings::notifyOnFileChange() const
{
    return _notifyOnFileChange->getValue();
}

bool
Settings::isAutoTurboEnabled() const
{
    return _autoTurbo->getValue();
}

void
Settings::setAutoTurboModeEnabled(bool e)
{
    _autoTurbo->setValue(e);
}

void
Settings::setOptionalInputsAutoHidden(bool hidden)
{
    _hideOptionalInputsAutomatically->setValue(hidden);
}

bool
Settings::areOptionalInputsAutoHidden() const
{
    return _hideOptionalInputsAutomatically->getValue();
}

void
Settings::getPythonGroupsSearchPaths(std::list<std::string>* templates) const
{
    _templatesPluginPaths->getPaths(templates);
}

void
Settings::appendPythonGroupsPath(const std::string& path)
{
    _templatesPluginPaths->appendPath(path);
    QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );
    settings.setValue( QString::fromUtf8( _templatesPluginPaths->getName().c_str() ), QVariant( QString::fromUtf8( _templatesPluginPaths->getValue(0).c_str() ) ) );
}

std::string
Settings::getDefaultOnProjectLoadedCB()
{
    return _defaultOnProjectLoaded->getValue();
}

std::string
Settings::getDefaultOnProjectSaveCB()
{
    return _defaultOnProjectSave->getValue();
}

std::string
Settings::getDefaultOnProjectCloseCB()
{
    return _defaultOnProjectClose->getValue();
}

std::string
Settings::getDefaultOnNodeCreatedCB()
{
    return _defaultOnNodeCreated->getValue();
}

std::string
Settings::getDefaultOnNodeDeleteCB()
{
    return _defaultOnNodeDelete->getValue();
}

std::string
Settings::getOnProjectCreatedCB()
{
    return _onProjectCreated->getValue();
}

bool
Settings::isLoadFromPyPlugsEnabled() const
{
    return _loadPyPlugsFromPythonScript->getValue();
}

bool
Settings::isAutoDeclaredVariablePrintActivated() const
{
    return _echoVariableDeclarationToPython->getValue();
}

void
Settings::setAutoDeclaredVariablePrintEnabled(bool enabled)
{
    _echoVariableDeclarationToPython->setValue(enabled);
    saveSetting( _echoVariableDeclarationToPython.get() );
}

bool
Settings::isPluginIconActivatedOnNodeGraph() const
{
    return _usePluginIconsInNodeGraph->getValue();
}

bool
Settings::isNodeGraphAntiAliasingEnabled() const
{
    return _useAntiAliasing->getValue();
}

void
Settings::getSunkenColor(double* r,
                         double* g,
                         double* b) const
{
    *r = _sunkenColor->getValue(0);
    *g = _sunkenColor->getValue(1);
    *b = _sunkenColor->getValue(2);
}

void
Settings::getBaseColor(double* r,
                       double* g,
                       double* b) const
{
    *r = _baseColor->getValue(0);
    *g = _baseColor->getValue(1);
    *b = _baseColor->getValue(2);
}

void
Settings::getRaisedColor(double* r,
                         double* g,
                         double* b) const
{
    *r = _raisedColor->getValue(0);
    *g = _raisedColor->getValue(1);
    *b = _raisedColor->getValue(2);
}
