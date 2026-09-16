/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: minimum free-disk cache setting.
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Settings.h"

#include <cassert>
#include <limits>
#include <stdexcept>

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QThreadPool>
#include <QThread>
#include <QTextStream>

#ifdef WINDOWS
#include <tchar.h>
#endif

#include "Global/StrUtils.h"

#include "Engine/AppManager.h"
#include "Engine/AppInstance.h"
#include "Engine/KnobFactory.h"
#include "Engine/KnobFile.h"
#include "Engine/KnobTypes.h"
#include "Engine/LibraryBinary.h"
#include "Engine/MemoryInfo.h" // getSystemTotalRAM, isApplication32Bits, printAsRAM
#include "Engine/Node.h"
#include "Engine/OSGLContext.h"
#include "Engine/OutputSchedulerThread.h"
#include "Engine/Plugin.h"
#include "Engine/Project.h"
#include "Engine/StandardPaths.h"
#include "Engine/Utils.h"
#include "Engine/ViewIdx.h"
#include "Engine/ViewerInstance.h"

#include "Gui/GuiDefines.h"

#include <SequenceParsing.h> // for removePath

#ifdef WINDOWS
#include <ofxhPluginCache.h>
#endif

#define NATRON_DEFAULT_OCIO_CONFIG_NAME "blender"


#define NATRON_CUSTOM_OCIO_CONFIG_NAME "Custom config"

#define NATRON_DEFAULT_APPEARANCE_VERSION 1

#define NATRON_CUSTOM_HOST_NAME_ENTRY "Custom..."

NATRON_NAMESPACE_ENTER

Settings::Settings()
    : KnobHolder( AppInstancePtr() ) // < Settings are process wide and do not belong to a single AppInstance
    , _restoringSettings(false)
    , _ocioRestored(false)
    , _settingsExisted(false)
    , _defaultAppearanceOutdated(false)
{
}

static QStringList
getDefaultOcioConfigPaths()
{
    QString binaryPath = appPTR->getApplicationBinaryPath();
    StrUtils::ensureLastPathSeparator(binaryPath);

#ifdef __NATRON_LINUX__
    QStringList ret;
    ret.push_back( QString::fromUtf8("/usr/share/OpenColorIO-Configs") );
    ret.push_back( QString( binaryPath + QString::fromUtf8("../share/OpenColorIO-Configs") ) );
    ret.push_back( QString( binaryPath + QString::fromUtf8("../Resources/OpenColorIO-Configs") ) );

    return ret;
#elif defined(__NATRON_WIN32__)

    return QStringList( QString( binaryPath + QString::fromUtf8("../Resources/OpenColorIO-Configs") ) );
#elif defined(__NATRON_OSX__)

    return QStringList( QString( binaryPath + QString::fromUtf8("../Resources/OpenColorIO-Configs") ) );
#endif
}

void
Settings::initializeKnobs()
{
    initializeKnobsGeneral();
    initializeKnobsThreading();
    initializeKnobsRendering();
    initializeKnobsGPU();
    initializeKnobsProjectSetup();
    initializeKnobsDocumentation();
    initializeKnobsUserInterface();
    initializeKnobsColorManagement();
    initializeKnobsCaching();
    initializeKnobsViewers();
    initializeKnobsNodeGraph();
    initializeKnobsPlugins();
    initializeKnobsPython();
    initializeKnobsAppearance();
    initializeKnobsGuiColors();
    initializeKnobsCurveEditorColors();
    initializeKnobsDopeSheetColors();
    initializeKnobsNodeGraphColors();
    initializeKnobsScriptEditorColors();

    setDefaultValues();
}

void
Settings::initializeKnobsGeneral()
{
    _generalTab = AppManager::createKnob<KnobPage>( this, tr("General") );

    _natronSettingsExist = AppManager::createKnob<KnobBool>( this, tr("Existing settings") );
    _natronSettingsExist->setName("existingSettings");
    _natronSettingsExist->setSecretByDefault(true);
    _generalTab->addKnob(_natronSettingsExist);

    _saveSettings = AppManager::createKnob<KnobBool>( this, tr("Save settings on change") );
    _saveSettings->setName("saveSettings");
    _saveSettings->setDefaultValue(true);
    _saveSettings->setSecretByDefault(true);
    _generalTab->addKnob(_saveSettings);

    _checkForUpdates = AppManager::createKnob<KnobBool>( this, tr("Always check for updates on start-up") );
    _checkForUpdates->setName("checkForUpdates");
    _checkForUpdates->setHintToolTip( tr("When checked, %1 will check for new updates on start-up of the application.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _generalTab->addKnob(_checkForUpdates);

#ifdef NATRON_USE_BREAKPAD
    _enableCrashReports = AppManager::createKnob<KnobBool>( this, tr("Enable crash reporting") );
    _enableCrashReports->setName("enableCrashReports");
    _enableCrashReports->setHintToolTip( tr("When checked, if %1 crashes a window will pop-up asking you "
                                            "whether you want to upload the crash dump to the developers or not. "
                                            "This can help them track down the bug.\n"
                                            "If you need to turn the crash reporting system off, uncheck this.\n"
                                            "Note that when using the application in command-line mode, if crash reports are "
                                            "enabled, they will be automatically uploaded.\n"
                                            "Changing this requires a restart of the application to take effect.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _enableCrashReports->setAddNewLine(false);
    _generalTab->addKnob(_enableCrashReports);

    _testCrashReportButton = AppManager::createKnob<KnobButton>( this, tr("Test Crash Reporting") );
    _testCrashReportButton->setName("testCrashReporting");
    _testCrashReportButton->setHintToolTip( tr("This button is for developers only to test whether the crash reporting system "
                                               "works correctly. Do not use this.") );
    _generalTab->addKnob(_testCrashReportButton);
#endif

    _autoSaveDelay = AppManager::createKnob<KnobInt>( this, tr("Auto-save trigger delay") );
    _autoSaveDelay->setName("autoSaveDelay");
    _autoSaveDelay->disableSlider();
    _autoSaveDelay->setMinimum(0);
    _autoSaveDelay->setMaximum(60);
    _autoSaveDelay->setHintToolTip( tr("The number of seconds after an event that %1 should wait before "
                                       " auto-saving. Note that if a render is in progress, %1 will "
                                       " wait until it is done to actually auto-save.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _generalTab->addKnob(_autoSaveDelay);


    _autoSaveUnSavedProjects = AppManager::createKnob<KnobBool>( this, tr("Enable Auto-save for unsaved projects") );
    _autoSaveUnSavedProjects->setName("autoSaveUnSavedProjects");
    _autoSaveUnSavedProjects->setHintToolTip( tr("When activated %1 will auto-save projects that have never been "
                                                 "saved and will prompt you on startup if an auto-save of that unsaved project was found. "
                                                 "Disabling this will no longer save un-saved project.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _generalTab->addKnob(_autoSaveUnSavedProjects);

    _saveVersions = AppManager::createKnob<KnobInt>( this, tr("Save versions") );
    _saveVersions->setName("saveVersions");
    _saveVersions->disableSlider();
    _saveVersions->setMinimum(0);
    _saveVersions->setMaximum(32);
    _saveVersions->setHintToolTip( tr("Number of versions created (for backup) when saving newer versions of a file.\n"
                                      "This option keeps saved versions of your file in the same directory, adding "
                                      ".~1~, .~2~, etc., with the number increasing to the number of versions you specify.\n"
                                      "Older files will be named with a higher number. E.g. with the default setting of 2, "
                                      "you will have three versions of your file: *.ntp (last saved), *.ntp.~1~ (second "
                                      "last saved), *.~2~ (third last saved).") );
    _generalTab->addKnob(_saveVersions);

    _hostName = AppManager::createKnob<KnobChoice>( this, tr("Appear to plug-ins as") );
    _hostName->setName("pluginHostName");
    _hostName->setHintToolTip( tr("%1 will appear with the name of the selected application to the OpenFX plug-ins. "
                                  "Changing it to the name of another application can help loading plugins which "
                                  "restrict their usage to specific OpenFX host(s). "
                                  "If a Host is not listed here, use the \"Custom\" entry to enter a custom host name. Changing this requires "
                                  "a restart of the application and requires clearing "
                                  "the OpenFX plugins cache from the Cache menu.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _knownHostNames.clear();
    std::vector<ChoiceOption> visibleHostEntries;
    assert(visibleHostEntries.size() == (int)eKnownHostNameNatron);
    visibleHostEntries.push_back(ChoiceOption(NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB "." NATRON_APPLICATION_NAME, NATRON_APPLICATION_NAME, ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameNuke);
    visibleHostEntries.push_back(ChoiceOption("uk.co.thefoundry.nuke", "Nuke", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameFusion);
    visibleHostEntries.push_back(ChoiceOption("com.eyeonline.Fusion", "Fusion", "")); // or com.blackmagicdesign.Fusion
    assert(visibleHostEntries.size() == (int)eKnownHostNameCatalyst);
    visibleHostEntries.push_back(ChoiceOption("com.sony.Catalyst.Edit", "Sony Catalyst Edit", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameVegas);
    visibleHostEntries.push_back(ChoiceOption("com.sonycreativesoftware.vegas", "Sony Vegas", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameToxik);
    visibleHostEntries.push_back(ChoiceOption("Autodesk Toxik", "Toxik", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameScratch);
    visibleHostEntries.push_back(ChoiceOption("Assimilator", "Scratch", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameDustBuster);
    visibleHostEntries.push_back(ChoiceOption("Dustbuster", "DustBuster", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameResolve);
    visibleHostEntries.push_back(ChoiceOption("DaVinciResolve", "Da Vinci Resolve", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameResolveLite);
    visibleHostEntries.push_back(ChoiceOption("DaVinciResolveLite", "Da Vinci Resolve Lite", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameMistika);
    visibleHostEntries.push_back(ChoiceOption("Mistika", "SGO Mistika", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNamePablo);
    visibleHostEntries.push_back(ChoiceOption("com.quantel.genq", "Quantel Pablo Rio", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameMotionStudio);
    visibleHostEntries.push_back(ChoiceOption("com.idtvision.MotionStudio", "IDT Motion Studio", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameShake);
    visibleHostEntries.push_back(ChoiceOption("com.apple.shake", "Shake", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameBaselight);
    visibleHostEntries.push_back(ChoiceOption("Baselight", "Baselight", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameFrameCycler);
    visibleHostEntries.push_back(ChoiceOption("IRIDAS Framecycler", "FrameCycler", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameNucoda);
    visibleHostEntries.push_back(ChoiceOption("Nucoda", "Nucoda Film Master", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameAvidDS);
    visibleHostEntries.push_back(ChoiceOption("DS OFX HOST", "Avid DS", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameDX);
    visibleHostEntries.push_back(ChoiceOption("com.chinadigitalvideo.dx", "China Digital Video DX", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameTitlerPro);
    visibleHostEntries.push_back(ChoiceOption("com.newblue.titlerpro", "NewBlueFX Titler Pro", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameNewBlueOFXBridge);
    visibleHostEntries.push_back(ChoiceOption("com.newblue.ofxbridge", "NewBlueFX OFX Bridge", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameRamen);
    visibleHostEntries.push_back(ChoiceOption("Ramen", "Ramen", ""));
    assert(visibleHostEntries.size() == (int)eKnownHostNameTuttleOfx);
    visibleHostEntries.push_back(ChoiceOption("TuttleOfx", "TuttleOFX", ""));

    _knownHostNames = visibleHostEntries;

    visibleHostEntries.push_back(ChoiceOption(NATRON_CUSTOM_HOST_NAME_ENTRY, "Custom host name", ""));

    _hostName->populateChoices(visibleHostEntries);
    _hostName->setAddNewLine(false);
    _generalTab->addKnob(_hostName);

    _customHostName = AppManager::createKnob<KnobString>( this, tr("Custom Host name") );
    _customHostName->setName("customHostName");
    _customHostName->setHintToolTip( tr("This is the name of the OpenFX host (application) as it appears to the OpenFX plugins. "
                                        "Changing it to the name of another application can help loading some plugins which "
                                        "restrict their usage to specific OpenFX hosts. You should leave "
                                        "this to its default value, unless a specific plugin refuses to load or run. "
                                        "Changing this takes effect upon the next application launch, and requires clearing "
                                        "the OpenFX plugins cache from the Cache menu. "
                                        "The default host name is: \n%1").arg( QString::fromUtf8(NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB "." NATRON_APPLICATION_NAME) ) );
    _customHostName->setSecretByDefault(true);
    _generalTab->addKnob(_customHostName);
} // Settings::initializeKnobsGeneral

void
Settings::initializeKnobsThreading()
{
    _threadingPage = AppManager::createKnob<KnobPage>( this, tr("Threading") );

    _numberOfThreads = AppManager::createKnob<KnobInt>( this, tr("Number of render threads (0=\"guess\")") );
    _numberOfThreads->setName("noRenderThreads");

    QString numberOfThreadsToolTip = tr("Controls how many threads %1 should use to render. \n"
                                        "-1: Disable multithreading totally (useful for debugging) \n"
                                        "0: Guess the thread count from the number of cores and the available memory (min(num_cores,memory/3.5Gb)). The ideal threads count for this hardware is %2.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).arg( QThread::idealThreadCount() );
    _numberOfThreads->setHintToolTip( numberOfThreadsToolTip.toStdString() );
    _numberOfThreads->disableSlider();
    _numberOfThreads->setMinimum(-1);
    _numberOfThreads->setDisplayMinimum(-1);
    _threadingPage->addKnob(_numberOfThreads);

#ifndef NATRON_PLAYBACK_USES_THREAD_POOL
    _numberOfParallelRenders = AppManager::createKnob<KnobInt>( this, tr("Number of parallel renders (0=\"guess\")") );
    _numberOfParallelRenders->setHintToolTip( tr("Controls the number of parallel frame that will be rendered at the same time by the renderer. "
                                                 "A value of 0 indicate that %1 should automatically determine "
                                                 "the best number of parallel renders to launch given your CPU activity. "
                                                 "Setting a value different than 0 should be done only if you know what you're doing and can lead "
                                                 "in some situations to worse performances. Overall to get the best performances you should have your "
                                                 "CPU at 100% activity without idle times.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _numberOfParallelRenders->setName("nParallelRenders");
    _numberOfParallelRenders->setMinimum(0);
    _numberOfParallelRenders->disableSlider();
    _threadingPage->addKnob(_numberOfParallelRenders);
#endif

    _useThreadPool = AppManager::createKnob<KnobBool>( this, tr("Effects use the thread-pool") );
    _useThreadPool->setName("useThreadPool");
    _useThreadPool->setHintToolTip( tr("When checked, all effects will use a global thread-pool to do their processing instead of launching "
                                       "their own threads. "
                                       "This suppresses the overhead created by the operating system creating new threads on demand for "
                                       "each rendering of a special effect. As a result of this, the rendering might be faster on systems "
                                       "with a lot of cores (>= 8). \n"
                                       "WARNING: This is known not to work when using The Foundry's Furnace plug-ins (and potentially "
                                       "some other plug-ins that the dev team hasn't not tested against it). When using these plug-ins, "
                                       "make sure to uncheck this option first otherwise it will crash %1.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _threadingPage->addKnob(_useThreadPool);

    _nThreadsPerEffect = AppManager::createKnob<KnobInt>( this, tr("Max threads usable per effect (0=\"guess\")") );
    _nThreadsPerEffect->setName("nThreadsPerEffect");
    _nThreadsPerEffect->setHintToolTip( tr("Controls how many threads a specific effect can use at most to do its processing. "
                                           "A high value will allow 1 effect to spawn lots of thread and might not be efficient because "
                                           "the time spent to launch all the threads might exceed the time spent actually processing. "
                                           "By default (0) the renderer applies an heuristic to determine what's the best number of threads "
                                           "for an effect.") );

    _nThreadsPerEffect->setMinimum(0);
    _nThreadsPerEffect->disableSlider();
    _threadingPage->addKnob(_nThreadsPerEffect);

    _renderInSeparateProcess = AppManager::createKnob<KnobBool>( this, tr("Render in a separate process") );
    _renderInSeparateProcess->setName("renderNewProcess");
    _renderInSeparateProcess->setHintToolTip( tr("If true, %1 will render frames to disk in "
                                                 "a separate process so that if the main application crashes, the render goes on.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _threadingPage->addKnob(_renderInSeparateProcess);

    _queueRenders = AppManager::createKnob<KnobBool>( this, tr("Append new renders to queue") );
    _queueRenders->setHintToolTip( tr("When checked, renders will be queued in the Progress Panel and will start only when all "
                                      "other prior tasks are done.") );
    _queueRenders->setName("queueRenders");
    _threadingPage->addKnob(_queueRenders);
} // Settings::initializeKnobsThreading

void
Settings::initializeKnobsRendering()
{
    _renderingPage = AppManager::createKnob<KnobPage>( this, tr("Rendering") );

    _convertNaNValues = AppManager::createKnob<KnobBool>( this, tr("Convert NaN values") );
    _convertNaNValues->setName("convertNaNs");
    _convertNaNValues->setHintToolTip( tr("When activated, any pixel that is a Not-a-Number will be converted to 1 to avoid potential crashes from "
                                          "downstream nodes. These values can be produced by faulty plug-ins when they use wrong arithmetic such as "
                                          "division by zero. Disabling this option will keep the NaN(s) in the buffers: this may lead to an "
                                          "undefined behavior.") );
    _renderingPage->addKnob(_convertNaNValues);

    _pluginUseImageCopyForSource = AppManager::createKnob<KnobBool>( this, tr("Copy input image before rendering any plug-in") );
    _pluginUseImageCopyForSource->setName("copyInputImage");
    _pluginUseImageCopyForSource->setHintToolTip( tr("If checked, when before rendering any node, %1 will copy "
                                                     "the input image to a local temporary image. This is to work-around some plug-ins "
                                                     "that write to the source image, thus modifying the output of the node upstream in "
                                                     "the cache. This is a known bug of an old version of RevisionFX REMap for instance. "
                                                     "By default, this parameter should be leaved unchecked, as this will require an extra "
                                                     "image allocation and copy before rendering any plug-in.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _renderingPage->addKnob(_pluginUseImageCopyForSource);

    _activateRGBSupport = AppManager::createKnob<KnobBool>( this, tr("RGB components support") );
    _activateRGBSupport->setHintToolTip( tr("When checked %1 is able to process images with only RGB components "
                                            "(support for images with RGBA and Alpha components is always enabled). "
                                            "Un-checking this option may prevent plugins that do not well support RGB components from crashing %1. "
                                            "Changing this option requires a restart of the application.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _activateRGBSupport->setName("rgbSupport");
    _renderingPage->addKnob(_activateRGBSupport);


    _activateTransformConcatenationSupport = AppManager::createKnob<KnobBool>( this, tr("Transforms concatenation support") );
    _activateTransformConcatenationSupport->setHintToolTip( tr("When checked %1 is able to concatenate transform effects "
                                                               "when they are chained in the compositing tree. This yields better results and faster "
                                                               "render times because the image is only filtered once instead of as many times as there are "
                                                               "transformations.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _activateTransformConcatenationSupport->setName("transformCatSupport");
    _renderingPage->addKnob(_activateTransformConcatenationSupport);
}

void
Settings::populateOpenGLRenderers(const std::list<OpenGLRendererInfo>& renderers)
{
    if ( renderers.empty() ) {
        _availableOpenGLRenderers->setSecret(true);
        _nOpenGLContexts->setSecret(true);
        _enableOpenGL->setSecret(true);
        return;
    }

    _nOpenGLContexts->setSecret(false);
    _enableOpenGL->setSecret(false);

    std::vector<ChoiceOption> entries( renderers.size() );
    int i = 0;
    for (std::list<OpenGLRendererInfo>::const_iterator it = renderers.begin(); it != renderers.end(); ++it, ++i) {
        std::string option = it->vendorName + ' ' + it->rendererName + ' ' + it->glVersionString;
        entries[i] = ChoiceOption(option);
    }
    _availableOpenGLRenderers->populateChoices(entries);
    _availableOpenGLRenderers->setSecret(renderers.size() == 1);
}

bool
Settings::isOpenGLRenderingEnabled() const
{
    if (_enableOpenGL->getIsSecret()) {
        return false;
    }
    EnableOpenGLEnum enableOpenGL = (EnableOpenGLEnum)_enableOpenGL->getValue();
    return enableOpenGL == eEnableOpenGLEnabled || (enableOpenGL == eEnableOpenGLDisabledIfBackground && !appPTR->isBackground());
}

int
Settings::getMaxOpenGLContexts() const
{
    return _nOpenGLContexts->getValue();
}

GLRendererID
Settings::getActiveOpenGLRendererID() const
{
    if ( _availableOpenGLRenderers->getIsSecret() ) {
        // We were not able to detect multiple renderers, use default
        return GLRendererID();
    }
    int activeIndex = _availableOpenGLRenderers->getValue();
    const std::list<OpenGLRendererInfo>& renderers = appPTR->getOpenGLRenderers();
    if ( (activeIndex < 0) || ( activeIndex >= (int)renderers.size() ) ) {
        // Invalid index
        return GLRendererID();
    }
    int i = 0;
    for (std::list<OpenGLRendererInfo>::const_iterator it = renderers.begin(); it != renderers.end(); ++it, ++i) {
        if (i == activeIndex) {
            return it->rendererID;
        }
    }

    return GLRendererID();
}

void
Settings::initializeKnobsGPU()
{
    _gpuPage = AppManager::createKnob<KnobPage>( this, tr("GPU Rendering") );
    _openglRendererString = AppManager::createKnob<KnobString>( this, tr("Active OpenGL renderer") );
    _openglRendererString->setName("activeOpenGLRenderer");
    _openglRendererString->setHintToolTip( tr("The currently active OpenGL renderer.") );
    _openglRendererString->setAsLabel();
    _gpuPage->addKnob(_openglRendererString);

    _availableOpenGLRenderers = AppManager::createKnob<KnobChoice>( this, tr("OpenGL renderer") );
    _availableOpenGLRenderers->setName("chooseOpenGLRenderer");
    _availableOpenGLRenderers->setHintToolTip( tr("The renderer used to perform OpenGL rendering. Changing the OpenGL renderer requires a restart of the application.") );
    _gpuPage->addKnob(_availableOpenGLRenderers);

    _nOpenGLContexts = AppManager::createKnob<KnobInt>( this, tr("No. of OpenGL Contexts") );
    _nOpenGLContexts->setName("maxOpenGLContexts");
    _nOpenGLContexts->setMinimum(1);
    _nOpenGLContexts->setDisplayMinimum(1);
    _nOpenGLContexts->setDisplayMaximum(8);
    _nOpenGLContexts->setMaximum(8);
    _nOpenGLContexts->setHintToolTip( tr("The number of OpenGL contexts created to perform OpenGL rendering. Each OpenGL context can be attached to a CPU thread, allowing for more frames to be rendered simultaneously. Increasing this value may increase performances for graphs with mixed CPU/GPU nodes but can drastically reduce performances if too many OpenGL contexts are active at once.") );
    _gpuPage->addKnob(_nOpenGLContexts);


    _enableOpenGL = AppManager::createKnob<KnobChoice>( this, tr("OpenGL Rendering") );
    _enableOpenGL->setName("enableOpenGLRendering");
    {
        std::vector<ChoiceOption> entries;
        assert(entries.size() == (int)Settings::eEnableOpenGLEnabled);
        entries.push_back(ChoiceOption("enabled",
                                       tr("Enabled").toStdString(),
                                       tr("If a plug-in support GPU rendering, prefer rendering using the GPU if possible.").toStdString()));
        assert(entries.size() == (int)Settings::eEnableOpenGLDisabled);
        entries.push_back(ChoiceOption("disabled",
                                       tr("Disabled").toStdString(),
                                       tr("Disable GPU rendering for all plug-ins.").toStdString()));
        assert(entries.size() == (int)Settings::eEnableOpenGLDisabledIfBackground);
        entries.push_back(ChoiceOption("foreground",
                                       tr("Disabled If Background").toStdString(),
                                       tr("Disable GPU rendering when rendering with NatronRenderer but not in GUI mode.").toStdString()));
        _enableOpenGL->populateChoices(entries);
    }
    _enableOpenGL->setHintToolTip( tr("Select whether to activate OpenGL rendering or not. If disabled, even though a Project enable GPU rendering, it will not be activated.") );
    _gpuPage->addKnob(_enableOpenGL);
}

void
Settings::initializeKnobsProjectSetup()
{
    _projectsPage = AppManager::createKnob<KnobPage>( this, tr("Project Setup") );

    _firstReadSetProjectFormat = AppManager::createKnob<KnobBool>( this, tr("First image read set project format") );
    _firstReadSetProjectFormat->setName("autoProjectFormat");
    _firstReadSetProjectFormat->setHintToolTip( tr("If checked, the project size is set to this of the first image or video read within the project.") );
    _projectsPage->addKnob(_firstReadSetProjectFormat);


    _autoPreviewEnabledForNewProjects = AppManager::createKnob<KnobBool>( this, tr("Auto-preview enabled by default for new projects") );
    _autoPreviewEnabledForNewProjects->setName("enableAutoPreviewNewProjects");
    _autoPreviewEnabledForNewProjects->setHintToolTip( tr("If checked, then when creating a new project, the Auto-preview option"
                                                          " is enabled.") );
    _projectsPage->addKnob(_autoPreviewEnabledForNewProjects);


    _fixPathsOnProjectPathChanged = AppManager::createKnob<KnobBool>( this, tr("Auto fix relative file-paths") );
    _fixPathsOnProjectPathChanged->setHintToolTip( tr("If checked, when a project-path changes (either the name or the value pointed to), %1 checks all file-path parameters in the project and tries to fix them.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _fixPathsOnProjectPathChanged->setName("autoFixRelativePaths");

    _projectsPage->addKnob(_fixPathsOnProjectPathChanged);

    _enableMappingFromDriveLettersToUNCShareNames = AppManager::createKnob<KnobBool>( this, tr("Use drive letters instead of server names (Windows only)") );
    _enableMappingFromDriveLettersToUNCShareNames->setHintToolTip( tr("This is only relevant for Windows: If checked, %1 will not convert a path starting with a drive letter from the file dialog to a network share name. You may use this if for example you want to share a same project with several users across facilities with different servers but where users have all the same drive attached to a server.").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
    _enableMappingFromDriveLettersToUNCShareNames->setName("useDriveLetters");
#ifndef __NATRON_WIN32__
    _enableMappingFromDriveLettersToUNCShareNames->setAllDimensionsEnabled(false);
#endif
    _projectsPage->addKnob(_enableMappingFromDriveLettersToUNCShareNames);
}

void
Settings::initializeKnobsDocumentation()
{
    _documentationPage = AppManager::createKnob<KnobPage>( this, tr("Documentation") );

#ifdef NATRON_DOCUMENTATION_ONLINE
    _documentationSource = AppManager::createKnob<KnobChoice>( this, tr("Documentation Source") );
    _documentationSource->setName("documentationSource");
    _documentationSource->setHintToolTip( tr("Documentation source.") );
    _documentationSource->appendChoice(ChoiceOption("local",
                                                    tr("Local").toStdString(),
                                                    tr("Use the documentation distributed with the software.").toStdString()));
    _documentationSource->appendChoice(ChoiceOption("online",
                                                    tr("Online").toStdString(),
                                                    tr("Use the online version of the documentation (requires an internet connection).").toStdString()));
    _documentationSource->appendChoice(ChoiceOption("none",
                                                    tr("None").toStdString(),
                                                    tr("Disable documentation").toStdString()));
    _documentationPage->addKnob(_documentationSource);
#endif

    /// used to store temp port for local webserver
    _wwwServerPort = AppManager::createKnob<KnobInt>( this, tr("Documentation local port (0=auto)") );
    _wwwServerPort->setName("webserverPort");
    _wwwServerPort->setHintToolTip( tr("The port onto which the documentation server will listen to. A value of 0 indicate that the documentation should automatically find a port by itself.") );
    _documentationPage->addKnob(_wwwServerPort);
}
#include "SettingsPart1.inc"
#include "SettingsPart2.inc"
#include "SettingsPart3.inc"
#include "SettingsPart4.inc"
#include "SettingsPart5.inc"
