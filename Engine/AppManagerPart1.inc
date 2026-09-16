
AppInstancePtr
AppManager::newAppInstanceInternal(const CLArgs& cl,
                                   bool alwaysBackground,
                                   bool makeEmptyInstance)
{
    AppInstancePtr instance;

    if (!alwaysBackground) {
        instance = makeNewInstance(_imp->_availableID);
    } else {
        instance = std::make_shared<AppInstance>(_imp->_availableID);
    }

    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        _imp->_appInstances.push_back(instance);
    }

    setAsTopLevelInstance( instance->getAppID() );

    ++_imp->_availableID;

    try {
        instance->load(cl, makeEmptyInstance);
    } catch (const std::exception & e) {
        Dialogs::errorDialog( NATRON_APPLICATION_NAME, e.what(), false );
        removeInstance(_imp->_availableID);
        instance.reset();
        --_imp->_availableID;

        return instance;
    } catch (...) {
        Dialogs::errorDialog( NATRON_APPLICATION_NAME, tr("Cannot load project").toStdString(), false );
        removeInstance(_imp->_availableID);
        instance.reset();
        --_imp->_availableID;

        return instance;
    }

    ///flag that we finished loading the Appmanager even if it was already true
    _imp->_loaded = true;

    return instance;
}

AppInstancePtr
AppManager::newBackgroundInstance(const CLArgs& cl,
                                  bool makeEmptyInstance)
{
    return newAppInstanceInternal(cl, true, makeEmptyInstance);
}

AppInstancePtr
AppManager::newAppInstance(const CLArgs& cl,
                           bool makeEmptyInstance)
{
    return newAppInstanceInternal(cl, false, makeEmptyInstance);
}

AppInstancePtr
AppManager::getAppInstance(int appID) const
{
    QMutexLocker k(&_imp->_appInstancesMutex);

    for (AppInstanceVec::const_iterator it = _imp->_appInstances.begin(); it != _imp->_appInstances.end(); ++it) {
        if ( (*it)->getAppID() == appID ) {
            return *it;
        }
    }

    return AppInstancePtr();
}

int
AppManager::getNumInstances() const
{
    QMutexLocker k(&_imp->_appInstancesMutex);

    return (int)_imp->_appInstances.size();
}

const AppInstanceVec &
AppManager::getAppInstances() const
{
    assert( QThread::currentThread() == qApp->thread() );

    return _imp->_appInstances;
}

void
AppManager::removeInstance(int appID)
{
    int newApp = -1;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        for (AppInstanceVec::iterator it = _imp->_appInstances.begin(); it != _imp->_appInstances.end(); ++it) {
            if ( (*it)->getAppID() == appID ) {
                _imp->_appInstances.erase(it);
                break;
            }
        }

        if ( !_imp->_appInstances.empty() ) {
            newApp = _imp->_appInstances.front()->getAppID();
        }
    }

    if (newApp != -1) {
        setAsTopLevelInstance(newApp);
    }
}

AppManager::AppTypeEnum
AppManager::getAppType() const
{
    return _imp->_appType;
}

void
AppManager::clearPlaybackCache()
{
    if (!_imp->_viewerCache) {
        return;
    }
    _imp->_viewerCache->clearInMemoryPortion();
    clearLastRenderedTextures();
}

void
AppManager::clearViewerCache()
{
    if (!_imp->_viewerCache) {
        return;
    }

    _imp->_viewerCache->clear();
}

void
AppManager::clearDiskCache()
{
    if (!_imp->_viewerCache) {
        return;
    }

    clearLastRenderedTextures();
    _imp->_viewerCache->clear();
    _imp->_diskCache->clear();
}

void
AppManager::clearNodeCache()
{
    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->clearAllLastRenderedImages();
    }
    _imp->_nodeCache->clear();
}

void
AppManager::clearPluginsLoadedCache()
{
    _imp->ofxHost->clearPluginsLoadedCache();
}

void
AppManager::clearAllCaches()
{
    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->abortAllViewers();
    }

    clearDiskCache();
    clearNodeCache();


    ///for each app instance clear all its nodes cache
    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->clearOpenFXPluginsCaches();
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->renderAllViewers(true);
    }

    Project::clearAutoSavesDir();
}

void
AppManager::wipeAndCreateDiskCacheStructure()
{
    //Should be called on the main-thread because potentially can interact with rendering
    assert( QThread::currentThread() == qApp->thread() );

    abortAnyProcessing();

    clearAllCaches();

    assert(_imp->_diskCache);
    _imp->cleanUpCacheDiskStructure( _imp->_diskCache->getCachePath(), false );
    assert(_imp->_viewerCache);
    _imp->cleanUpCacheDiskStructure( _imp->_viewerCache->getCachePath() , true);
}

AppInstancePtr
AppManager::getTopLevelInstance () const
{
    QMutexLocker k(&_imp->_appInstancesMutex);

    for (AppInstanceVec::const_iterator it = _imp->_appInstances.begin(); it != _imp->_appInstances.end(); ++it) {
        if ( (*it)->getAppID() == _imp->_topLevelInstanceID ) {
            return *it;
        }
    }

    return AppInstancePtr();
}

bool
AppManager::isLoaded() const
{
    return _imp->_loaded;
}

void
AppManager::abortAnyProcessing()
{
    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->getProject()->quitAnyProcessingForAllNodes_non_blocking();
    }
}

bool
AppManager::writeToOutputPipe(const QString & longMessage,
                              const QString & shortMessage,
                              bool printIfNoChannel)
{
    if (!_imp->_backgroundIPC) {
        if (printIfNoChannel) {
            QMutexLocker k(&_imp->errorLogMutex);
            ///Don't use qdebug here which is disabled if QT_NO_DEBUG_OUTPUT is defined.
            std::cout << longMessage.toStdString() << std::endl;
        }

        return false;
    }
    _imp->_backgroundIPC->writeToOutputChannel(shortMessage);

    return true;
}

void
AppManager::setApplicationsCachesMaximumMemoryPercent(double p)
{
    size_t maxCacheRAM = p * getSystemTotalRAM_conditionnally();

    _imp->_nodeCache->setMaximumCacheSize(maxCacheRAM);
    _imp->_nodeCache->setMaximumInMemorySize(1);
}

void
AppManager::setApplicationsCachesMaximumViewerDiskSpace(unsigned long long size)
{
    _imp->_viewerCache->setMaximumCacheSize(size);
}

void
AppManager::setApplicationsCachesMaximumDiskSpace(unsigned long long size)
{
    _imp->_diskCache->setMaximumCacheSize(size);
}

void
AppManager::loadAllPlugins()
{
    assert( _imp->_plugins.empty() );
    assert( _imp->_formats.empty() );

    // Load plug-ins bundled into Natron
    loadBuiltinNodePlugins(&_imp->readerPlugins, &_imp->writerPlugins);

    // Load OpenFX plug-ins
    _imp->ofxHost->loadOFXPlugins( &_imp->readerPlugins, &_imp->writerPlugins);

    // Load PyPlugs and init.py & initGui.py scripts
    // Should be done after settings are declared
    loadPythonGroups();

    _imp->_settings->restorePluginSettings();


    onAllPluginsLoaded();
}

void
AppManager::onAllPluginsLoaded()
{
    //We try to make nicer plug-in labels, only do this if the user use Natron with some sort of interaction (either command line
    //or GUI, otherwise don't bother doing this)

    AppManager::AppTypeEnum appType = appPTR->getAppType();

    if ( (appType != AppManager::eAppTypeBackground) &&
         ( appType != AppManager::eAppTypeGui) &&
         ( appType != AppManager::eAppTypeInterpreter) ) {
        return;
    }

    //Make sure there is no duplicates with the same label
    const PluginsMap& plugins = getPluginsList();
    for (PluginsMap::const_iterator it = plugins.begin(); it != plugins.end(); ++it) {
        assert( !it->second.empty() );
        if (it->second.empty()) {
            continue;
        }
        PluginVersionsOrdered::reverse_iterator first = it->second.rbegin();
        bool isUserCreatable = false;
        for (PluginVersionsOrdered::reverse_iterator itver = it->second.rbegin(); itver != it->second.rend(); ++itver) {
            if ( (*itver)->getIsUserCreatable() ) {
                isUserCreatable = true;
            } else {
                (*itver)->setLabelWithoutSuffix( (*itver)->getPluginLabel() );
            }
        }
        if (!isUserCreatable) {
            continue;
        }

        QString labelWithoutSuffix = Plugin::makeLabelWithoutSuffix( (*first)->getPluginLabel() );

        //Find a duplicate
        for (PluginsMap::const_iterator it2 = plugins.begin(); it2 != plugins.end(); ++it2) {
            if (it->first == it2->first) {
                continue;
            }


            PluginVersionsOrdered::reverse_iterator other = it2->second.rbegin();
            bool isOtherUserCreatable = false;
            for (PluginVersionsOrdered::reverse_iterator it2ver = it2->second.rbegin(); it2ver != it2->second.rend(); ++it2ver) {
                if ( (*it2ver)->getIsUserCreatable() ) {
                    isOtherUserCreatable = true;
                    break;
                }
            }

            if (!isOtherUserCreatable) {
                continue;
            }

            QString otherLabelWithoutSuffix = Plugin::makeLabelWithoutSuffix( (*other)->getPluginLabel() );
            if (otherLabelWithoutSuffix == labelWithoutSuffix) {
                QString otherGrouping = (*other)->getGrouping().join( QChar::fromLatin1('/') );
                const QStringList& thisGroupingSplit = (*first)->getGrouping();
                QString thisGrouping = thisGroupingSplit.join( QChar::fromLatin1('/') );
                if (otherGrouping == thisGrouping) {
                    labelWithoutSuffix = (*first)->getPluginLabel();
                }
                break;
            }
        }


        for (PluginVersionsOrdered::reverse_iterator itver = it->second.rbegin(); itver != it->second.rend(); ++itver) {
            if ( (*itver)->getIsUserCreatable() ) {
                (*itver)->setLabelWithoutSuffix(labelWithoutSuffix);
                onPluginLoaded(*itver);
            }
        }
    }
} // AppManager::onAllPluginsLoaded

template <typename PLUGIN>
void
AppManager::registerBuiltInPlugin(const QString& iconPath,
                                  bool isDeprecated,
                                  bool internalUseOnly)
{
    EffectInstancePtr node( PLUGIN::BuildEffect( NodePtr() ) );
    std::map<std::string, void (*)()> functions;

    functions.insert( std::make_pair("BuildEffect", ( void (*)() ) & PLUGIN::BuildEffect) );
    LibraryBinary *binary = new LibraryBinary(functions);
    assert(binary);

    std::list<std::string> grouping;
    node->getPluginGrouping(&grouping);
    QStringList qgrouping;

    for (std::list<std::string>::iterator it = grouping.begin(); it != grouping.end(); ++it) {
        qgrouping.push_back( QString::fromUtf8( it->c_str() ) );
    }

    // Empty since statically bundled
    QString resourcesPath = QString();
    Plugin* p = registerPlugin(resourcesPath, qgrouping, QString::fromUtf8( node->getPluginID().c_str() ), QString::fromUtf8( node->getPluginLabel().c_str() ),
                               iconPath, QStringList(), node->isReader(), node->isWriter(), binary, node->renderThreadSafety() == eRenderSafetyUnsafe, node->getMajorVersion(), node->getMinorVersion(), isDeprecated);
    std::list<PluginActionShortcut> shortcuts;
    node->getPluginShortcuts(&shortcuts);
    p->setShorcuts(shortcuts);

    PluginOpenGLRenderSupport glSupport = node->supportsOpenGLRender();
    p->setOpenGLRenderSupport(glSupport);

    if (internalUseOnly) {
        p->setForInternalUseOnly(true);
    }
}

void
AppManager::loadBuiltinNodePlugins(IOPluginsMap* /*readersMap*/,
                                   IOPluginsMap* /*writersMap*/)
{
    registerBuiltInPlugin<Backdrop>(QString::fromUtf8(NATRON_IMAGES_PATH "backdrop_icon.png"), false, false);
    registerBuiltInPlugin<GroupOutput>(QString::fromUtf8(NATRON_IMAGES_PATH "output_icon.png"), false, false);
    registerBuiltInPlugin<GroupInput>(QString::fromUtf8(NATRON_IMAGES_PATH "input_icon.png"), false, false);
    registerBuiltInPlugin<NodeGroup>(QString::fromUtf8(NATRON_IMAGES_PATH "group_icon.png"), false, false);
    registerBuiltInPlugin<Dot>(QString::fromUtf8(NATRON_IMAGES_PATH "dot_icon.png"), false, false);
    registerBuiltInPlugin<DiskCacheNode>(QString::fromUtf8(NATRON_IMAGES_PATH "diskcache_icon.png"), false, false);
    registerBuiltInPlugin<RotoPaint>(QString::fromUtf8(NATRON_IMAGES_PATH "GroupingIcons/Set2/paint_grouping_2.png"), false, false);
    registerBuiltInPlugin<RotoNode>(QString::fromUtf8(NATRON_IMAGES_PATH "rotoNodeIcon.png"), false, false);
    registerBuiltInPlugin<RotoSmear>(QString::fromUtf8(""), false, true);
    registerBuiltInPlugin<PrecompNode>(QString::fromUtf8(NATRON_IMAGES_PATH "precompNodeIcon.png"), false, false);
    registerBuiltInPlugin<TrackerNode>(QString::fromUtf8(NATRON_IMAGES_PATH "trackerNodeIcon.png"), false, false);
    registerBuiltInPlugin<JoinViewsNode>(QString::fromUtf8(NATRON_IMAGES_PATH "joinViewsNode.png"), false, false);
    registerBuiltInPlugin<OneViewNode>(QString::fromUtf8(NATRON_IMAGES_PATH "oneViewNode.png"), false, false);
#ifdef NATRON_ENABLE_IO_META_NODES
    registerBuiltInPlugin<ReadNode>(QString::fromUtf8(NATRON_IMAGES_PATH "readImage.png"), false, false);
    registerBuiltInPlugin<WriteNode>(QString::fromUtf8(NATRON_IMAGES_PATH "writeImage.png"), false, false);
#endif

    if ( !isBackground() ) {
        registerBuiltInPlugin<ViewerInstance>(QString::fromUtf8(NATRON_IMAGES_PATH "viewer_icon.png"), false, false);
    }
}

bool
AppManager::findAndRunScriptFile(const QString& path,
                     const QStringList& files,
                     const QString& script)
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return false;
#endif
    for (QStringList::const_iterator it = files.begin(); it != files.end(); ++it) {
        if (*it == script) {
            QString absolutePath = path + *it;
            QFile file(absolutePath);
            if ( file.open(QIODevice::ReadOnly) ) {
                QTextStream ts(&file);
                QString content = ts.readAll();
                PythonGILLocker pgl;

                PyRun_SimpleString( content.toStdString().c_str() );


                PyObject* mainModule = NATRON_PYTHON_NAMESPACE::getMainModule();
                std::string error, output;

                ///Gui session, do stdout, stderr redirection
                PyObject *errCatcher = 0;
                PyObject *outCatcher = 0;

                if ( PyObject_HasAttrString(mainModule, "catchErr") ) {
                    errCatcher = PyObject_GetAttrString(mainModule, "catchErr"); //get our catchOutErr created above, new ref
                }

                if ( PyObject_HasAttrString(mainModule, "catchOut") ) {
                    outCatcher = PyObject_GetAttrString(mainModule, "catchOut"); //get our catchOutErr created above, new ref
                }

                PyErr_Print(); //make python print any errors

                PyObject *errorObj = 0;
                if (errCatcher) {
                    errorObj = PyObject_GetAttrString(errCatcher, "value"); //get the  stderr from our catchErr object, new ref
                    assert(errorObj);

                    error = NATRON_PYTHON_NAMESPACE::PyStringToStdString(errorObj);
                    PyObject* unicode = PyUnicode_FromString("");
                    PyObject_SetAttrString(errCatcher, "value", unicode);
                    Py_DECREF(errorObj);
                    Py_DECREF(errCatcher);
                }
                PyObject *outObj = 0;
                if (outCatcher) {
                    outObj = PyObject_GetAttrString(outCatcher, "value"); //get the stdout from our catchOut object, new ref
                    assert(outObj);
                    output = NATRON_PYTHON_NAMESPACE::PyStringToStdString(outObj);
                    PyObject* unicode = PyUnicode_FromString("");
                    PyObject_SetAttrString(outCatcher, "value", unicode);
                    Py_DECREF(outObj);
                    Py_DECREF(outCatcher);
                }


                if ( !error.empty() ) {
                    QString message(tr("Failed to load %1: %2").arg(absolutePath).arg(QString::fromUtf8( error.c_str() )) );
                    appPTR->writeToErrorLog_mt_safe(tr("Python Script"), QDateTime::currentDateTime(), message, false);
                    std::cerr << message.toStdString() << std::endl;

                    return false;
                }
                if ( !output.empty() ) {
                    QString message;
                    message.append(absolutePath);
                    message.append( QString::fromUtf8(": ") );
                    message.append( QString::fromUtf8( output.c_str() ) );
                    if ( appPTR->getTopLevelInstance() ) {
                        appPTR->getTopLevelInstance()->appendToScriptEditor( message.toStdString() );
                    }
                    std::cout << message.toStdString() << std::endl;
                }

                return true;
            }
            break;
        }
    }

    return false;
} // findAndRunScriptFile

QStringList
AppManager::getAllNonOFXPluginsPaths() const
{
    QStringList templatesSearchPath;

    //add ~/.Natron
    QString dataLocation = QDir::homePath();
    QString mainPath = dataLocation + QString::fromUtf8("/." ) + QString::fromUtf8(NATRON_APPLICATION_NAME);
    QDir mainPathDir(mainPath);

    if ( !mainPathDir.exists() ) {
        QDir dataDir(dataLocation);
        if ( dataDir.exists() ) {
            dataDir.mkdir( QChar::fromLatin1('.') + QString( QString::fromUtf8(NATRON_APPLICATION_NAME) ) );
        }
    }

    QString envvar( QString::fromUtf8( qgetenv(NATRON_PLUGIN_PATH_ENV_VAR) ) );
# ifdef __NATRON_WIN32__
    const QChar pathSep = QChar::fromLatin1(';');
# else
    const QChar pathSep = QChar::fromLatin1(':');
# endif
    QStringList splitDirs = envvar.split(pathSep);
    std::list<std::string> userSearchPaths;
    _imp->_settings->getPythonGroupsSearchPaths(&userSearchPaths);


    //This is the bundled location for PyPlugs
    QDir cwd( QCoreApplication::applicationDirPath() );
    cwd.cdUp();
    QString natronBundledPluginsPath = QString( cwd.absolutePath() +  QString::fromUtf8("/Plugins/PyPlugs") );
    bool preferBundleOverSystemWide = _imp->_settings->preferBundledPlugins();
    bool useBundledPlugins = _imp->_settings->loadBundledPlugins();
    if (preferBundleOverSystemWide && useBundledPlugins) {
        ///look-in the bundled plug-ins
        templatesSearchPath.push_back(natronBundledPluginsPath);
    }

    ///look-in the main system wide plugin path
    templatesSearchPath.push_back(mainPath);

    ///look-in the global system wide plugin path
    templatesSearchPath.push_back( getPyPlugsGlobalPath() );

    ///look-in the locations indicated by NATRON_PLUGIN_PATH
    Q_FOREACH(const QString &splitDir, splitDirs) {
        if ( !splitDir.isEmpty() ) {
            templatesSearchPath.push_back(splitDir);
        }
    }

    ///look-in extra search path set in the preferences
    for (std::list<std::string>::iterator it = userSearchPaths.begin(); it != userSearchPaths.end(); ++it) {
        if ( !it->empty() ) {
            templatesSearchPath.push_back( QString::fromUtf8( it->c_str() ) );
        }
    }

    if (!preferBundleOverSystemWide && useBundledPlugins) {
        ///look-in the bundled plug-ins
        templatesSearchPath.push_back(natronBundledPluginsPath);
    }
    qDebug() << "templatesSearchPath: " << templatesSearchPath;
    
    return templatesSearchPath;
} // AppManager::getAllNonOFXPluginsPaths

QString
AppManager::getPyPlugsGlobalPath() const
{
    QString path;

#ifdef __NATRON_UNIX__
#ifdef __NATRON_OSX__
    path = QString::fromUtf8("/Library/Application Support/%1/Plugins").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) );
#else
    path = QString::fromUtf8("/usr/share/%1/Plugins").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) );
#endif
#elif defined(__NATRON_WIN32__)
    wchar_t buffer[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, SHGFP_TYPE_CURRENT, buffer);
    std::wstring str;
    str.append(L"\\");
    str.append( QString::fromUtf8("%1\\Plugins").arg( QString::fromUtf8(NATRON_APPLICATION_NAME) ).toStdWString() );
    wcscat_s(buffer, MAX_PATH, str.c_str());
    path = QString::fromStdWString( std::wstring(buffer) );
#endif

    return path;
}

typedef void (*NatronPathFunctor)(const QDir&);
static void
operateOnPathRecursive(NatronPathFunctor functor,
                       const QDir& directory)
{
    if ( !directory.exists() ) {
        return;
    }

    functor(directory);

    QStringList subDirs = directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    Q_FOREACH(const QString &subDir, subDirs) {
        QDir d(directory.absolutePath() + QChar::fromLatin1('/') + subDir);

        operateOnPathRecursive(functor, d);
    }
}

static void
addToPythonPathFunctor(const QDir& directory)
{
    std::string addToPythonPath("sys.path.append(str('");

    addToPythonPath += directory.absolutePath().toStdString();
    addToPythonPath += "'))\n";

    std::string err;
    bool ok  = NATRON_PYTHON_NAMESPACE::interpretPythonScript(addToPythonPath, &err, 0);
    if (!ok) {
        std::string message = QCoreApplication::translate("AppManager", "Could not add %1 to python path:").arg( directory.absolutePath() ).toStdString() + ' ' + err;
        std::cerr << message << std::endl;
        AppInstancePtr topLevel = appPTR->getTopLevelInstance();
        if (topLevel) {
            topLevel->appendToScriptEditor( message.c_str() );
        }
    }
}

void
AppManager::findAllScriptsRecursive(const QDir& directory,
                        QStringList& allPlugins,
                        QStringList *foundInit,
                        QStringList *foundInitGui)
{
    if ( !directory.exists() ) {
        return;
    }

    QStringList filters;
    filters << QString::fromUtf8("*.py");
    QStringList files = directory.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
    bool ok = findAndRunScriptFile( directory.absolutePath() + QChar::fromLatin1('/'), files, QString::fromUtf8("init.py") );
    if (ok) {
        foundInit->append( directory.absolutePath() + QString::fromUtf8("/init.py") );
    }
    if ( !appPTR->isBackground() ) {
        ok = findAndRunScriptFile( directory.absolutePath() + QChar::fromLatin1('/'), files, QString::fromUtf8("initGui.py") );
        if (ok) {
            foundInitGui->append( directory.absolutePath() + QString::fromUtf8("/initGui.py") );
        }
    }

    for (QStringList::iterator it = files.begin(); it != files.end(); ++it) {
        if ( it->endsWith( QString::fromUtf8(".py") ) && ( *it != QString::fromUtf8("init.py") ) && ( *it != QString::fromUtf8("initGui.py") ) ) {
            allPlugins.push_back(directory.absolutePath() + QChar::fromLatin1('/') + *it);
        }
    }

    QStringList subDirs = directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    Q_FOREACH(const QString &subDir, subDirs) {
        QDir d(directory.absolutePath() + QChar::fromLatin1('/') + subDir);

        findAllScriptsRecursive(d, allPlugins, foundInit, foundInitGui);
    }
}

void
AppManager::loadPythonGroups()
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return;
#endif
    PythonGILLocker pgl; // useless?
    QStringList templatesSearchPath = getAllNonOFXPluginsPaths();
    std::string err;
    QStringList allPlugins;

    ///For all search paths, first add the path to the python path, then run in order the init.py and initGui.py
    Q_FOREACH(const QString &templatesSearchDir, templatesSearchPath) {
        //Adding Qt resources to Python path is useless as Python does not know how to use it
        if ( templatesSearchDir.startsWith( QString::fromUtf8(":/Resources") ) ) {
            continue;
        }
        QDir d(templatesSearchDir);
        operateOnPathRecursive(&addToPythonPathFunctor, d);
    }

    ///Also import qtpy.QtCore and qtpy.QtGui (the later only in non background mode)
    {
        std::string s;
        s = "import qtpy\nfrom qtpy import QtCore";
        bool ok  = NATRON_PYTHON_NAMESPACE::interpretPythonScript(s, &err, 0);
        if (!ok) {
            QString message = tr("Failed to import qtpy.QtCore, make sure it is bundled with your Natron installation "
                                     "or reachable through the Python path. "
                                     "Note that Natron disables usage "
                                 "of site-packages).");
            std::cerr << message.toStdString() << std::endl;
            appPTR->writeToErrorLog_mt_safe(QLatin1String("qtpy.QtCore"), QDateTime::currentDateTime(), message);
        }
    }

    if ( !isBackground() ) {
        std::string s = "from qtpy import QtGui";
        bool ok  = NATRON_PYTHON_NAMESPACE::interpretPythonScript(s, &err, 0);
        if (!ok) {
            QString message = tr("Failed to import qtpy.QtGui");
            std::cerr << message.toStdString() << std::endl;
            appPTR->writeToErrorLog_mt_safe(QLatin1String("qtpy.QtGui"), QDateTime::currentDateTime(), message);
        }
    }


    QStringList foundInit;
    QStringList foundInitGui;
    Q_FOREACH(const QString &templatesSearchDir, templatesSearchPath) {
        QDir d(templatesSearchDir);

        findAllScriptsRecursive(d, allPlugins, &foundInit, &foundInitGui);
    }
    if ( foundInit.isEmpty() ) {
        QString message = tr("Info: init.py script not loaded (this is not an error)");
        appPTR->setLoadingStatus(message);
        if ( !appPTR->isBackground() ) {
            std::cout << message.toStdString() << std::endl;
        }
    } else {
        Q_FOREACH(const QString &found, foundInit) {
            QString message = tr("Info: init.py script found and loaded at %1").arg(found);

            appPTR->setLoadingStatus(message);
            if ( !appPTR->isBackground() ) {
                std::cout << message.toStdString() << std::endl;
            }
        }
    }

    if ( !appPTR->isBackground() ) {
        if ( foundInitGui.isEmpty() ) {
            QString message = tr("Info: initGui.py script not loaded (this is not an error)");
            appPTR->setLoadingStatus(message);
            if ( !appPTR->isBackground() ) {
                std::cout << message.toStdString() << std::endl;
            }
        } else {
            Q_FOREACH(const QString &found, foundInitGui) {
                QString message = tr("Info: initGui.py script found and loaded at %1").arg(found);

                appPTR->setLoadingStatus(message);
                if ( !appPTR->isBackground() ) {
                    std::cout << message.toStdString() << std::endl;
                }
            }
        }
    }

    // Now that init.py and InitGui.py have run, we need to set the search path again for the PyPlug
    // as the user might have called appendToNatronPath

    QStringList newTemplatesSearchPath = getAllNonOFXPluginsPaths();
    {
        QStringList diffSearch;
        Q_FOREACH(const QString &newTemplatesSearchDir, newTemplatesSearchPath) {
            if ( !templatesSearchPath.contains(newTemplatesSearchDir) ) {
                diffSearch.push_back(newTemplatesSearchDir);
            }
        }

        //Add only paths that did not exist so far
        Q_FOREACH(const QString &diffDir, diffSearch) {
            QDir d(diffDir);

            operateOnPathRecursive(&addToPythonPathFunctor, d);
        }
    }

    appPTR->setLoadingStatus( tr("Loading PyPlugs...") );

    Q_FOREACH(const QString &plugin, allPlugins) {
        QString moduleName = plugin;
        QString modulePath;
        int lastDot = moduleName.lastIndexOf( QChar::fromLatin1('.') );

        if (lastDot != -1) {
            moduleName = moduleName.left(lastDot);
        }
        int lastSlash = moduleName.lastIndexOf( QChar::fromLatin1('/') );
        if (lastSlash != -1) {
            modulePath = moduleName.mid(0, lastSlash + 1);
            moduleName = moduleName.remove(0, lastSlash + 1);
        }


        {
            // Open the file and check for a line that imports NatronGui, if so do not attempt to load the script.
            QFile file(plugin);
            if (!file.open(QIODevice::ReadOnly)) {
                continue;
            }
            QTextStream ts(&file);
            bool gotNatronGuiImport = false;
            bool isPyPlug = false;
            while (!ts.atEnd()) {
                QString line = ts.readLine();
                if (line.startsWith(QString::fromUtf8("import %1").arg(QLatin1String(NATRON_GUI_PYTHON_MODULE_NAME))) ||
                    line.startsWith(QString::fromUtf8("from %1 import").arg(QLatin1String(NATRON_GUI_PYTHON_MODULE_NAME)))) {
                    gotNatronGuiImport = true;
                }
                // We have to find a way to tell PyPlugs from other python files.
                // We could check if the file was created by Natron...
                if (line.startsWith(QString::fromUtf8(NATRON_PYPLUG_GENERATED))) {
                    isPyPlug = true;
                }
                // Or we could check if createInstance(app,group) is defined
                if ( line.startsWith( QString::fromUtf8("def createInstance(") ) ) {
                    isPyPlug = true;
                }
                // Or we could check if it implements getIsToolSet()
                if ( line.startsWith( QString::fromUtf8("def getIsToolSet(") ) ) {
                    isPyPlug = true;
                }
                // Or we could check for the magic line that is in the doc.
                // See https://natron.readthedocs.io/en/master/devel/groups.html#creating-a-group-by-hand
                // and https://natron.readthedocs.io/en/master/devel/groups.html#toolsets
                if ( line.startsWith( QString::fromUtf8(NATRON_PYPLUG_MAGIC) ) ) {
                    isPyPlug = true;
                }
            }
            if (appPTR->isBackground() && gotNatronGuiImport) {
                continue;
            }
            if (!isPyPlug) {
                continue;
            }
        }

        std::string pluginLabel, pluginID, pluginGrouping, iconFilePath, pluginDescription;
        unsigned int version;
        bool isToolset;
        bool gotInfos = NATRON_PYTHON_NAMESPACE::getGroupInfos(modulePath.toStdString(), moduleName.toStdString(), &pluginID, &pluginLabel, &iconFilePath, &pluginGrouping, &pluginDescription, &isToolset, &version);


        if (gotInfos) {
            qDebug() << "Loading" << moduleName;
            QStringList grouping = QString::fromUtf8( pluginGrouping.c_str() ).split( QChar::fromLatin1('/') );
            Plugin* p = registerPlugin(modulePath, grouping, QString::fromUtf8( pluginID.c_str() ), QString::fromUtf8( pluginLabel.c_str() ), QString::fromUtf8( iconFilePath.c_str() ), QStringList(), false, false, 0, false, version, 0, false);

            p->setPythonModule(modulePath + moduleName);
            p->setToolsetScript(isToolset);
        }
    }
} // AppManager::loadPythonGroups

Plugin*
AppManager::registerPlugin(const QString& resourcesPath,
                           const QStringList & groups,
                           const QString & pluginID,
                           const QString & pluginLabel,
                           const QString & pluginIconPath,
                           const QStringList & groupIconPath,
                           bool isReader,
                           bool isWriter,
                           LibraryBinary* binary,
                           bool mustCreateMutex,
                           int major,
                           int minor,
                           bool isDeprecated)
{
    QRecursiveMutex* pluginMutex = nullptr;

    if (mustCreateMutex) {
        pluginMutex = new QRecursiveMutex();
    }
    Plugin* plugin = new Plugin(binary, resourcesPath, pluginID, pluginLabel, pluginIconPath, groupIconPath, groups, pluginMutex, major, minor,
                                isReader, isWriter, isDeprecated);
    std::string stdID = pluginID.toStdString();

#ifdef NATRON_ENABLE_IO_META_NODES
    if ( ReadNode::isBundledReader( stdID, false ) ||
         WriteNode::isBundledWriter( stdID, false ) ) {
        plugin->setForInternalUseOnly(true);
    }
#endif

    PluginsMap::iterator found = _imp->_plugins.find(stdID);
    if ( found != _imp->_plugins.end() ) {
        found->second.insert(plugin);
    } else {
        PluginVersionsOrdered set;
        set.insert(plugin);
        _imp->_plugins.insert( std::make_pair(stdID, set) );
    }

    return plugin;
}

Format
AppManager::findExistingFormat(int w,
                               int h,
                               double par) const
{
    for (U32 i = 0; i < _imp->_formats.size(); ++i) {
        const Format& frmt = _imp->_formats[i];
        if ( (frmt.width() == w) && (frmt.height() == h) && (frmt.getPixelAspectRatio() == par) ) {
            return frmt;
        }
    }

    return Format();
}

void
AppManager::setAsTopLevelInstance(int appID)
{
    QMutexLocker k(&_imp->_appInstancesMutex);

    if (_imp->_topLevelInstanceID == appID) {
        return;
    }
    _imp->_topLevelInstanceID = appID;
    for (AppInstanceVec::iterator it = _imp->_appInstances.begin();
         it != _imp->_appInstances.end();
         ++it) {
        if ( (*it)->getAppID() != _imp->_topLevelInstanceID ) {
            if ( !isBackground() ) {
                (*it)->disconnectViewersFromViewerCache();
            }
        } else {
            if ( !isBackground() ) {
                (*it)->connectViewersToViewerCache();
                setOFXHostHandle( (*it)->getOfxHostOSHandle() );
            }
        }
    }
}

void
AppManager::setOFXHostHandle(void* handle)
{
    _imp->ofxHost->setOfxHostOSHandle(handle);
}

void
AppManager::clearExceedingEntriesFromNodeCache()
{
    _imp->_nodeCache->clearExceedingEntries();
}

const PluginsMap&
AppManager::getPluginsList() const
{
    return _imp->_plugins;
}


const std::vector<Format> &
AppManager::getFormats() const
{
    return _imp->_formats;
}

const KnobFactory &
AppManager::getKnobFactory() const
{
    return *(_imp->_knobFactory);
}

Plugin*
AppManager::getPluginBinaryFromOldID(const QString & pluginId,
                                     int majorVersion,
                                     int minorVersion) const
{
    std::map<int, Plugin*> matches;

    if ( pluginId == QString::fromUtf8("Viewer") ) {
        return _imp->findPluginById(QString::fromUtf8(PLUGINID_NATRON_VIEWER), majorVersion, minorVersion);
    } else if ( pluginId == QString::fromUtf8("Dot") ) {
        return _imp->findPluginById(QString::fromUtf8(PLUGINID_NATRON_DOT), majorVersion, minorVersion );
    } else if ( pluginId == QString::fromUtf8("DiskCache") ) {
        return _imp->findPluginById(QString::fromUtf8(PLUGINID_NATRON_DISKCACHE), majorVersion, minorVersion);
    } else if ( pluginId == QString::fromUtf8("Backdrop") ) { // DO NOT change the capitalization, even if it's wrong
        return _imp->findPluginById(QString::fromUtf8(PLUGINID_NATRON_BACKDROP), majorVersion, minorVersion);
    } else if ( pluginId == QString::fromUtf8("RotoOFX  [Draw]") ) {
        return _imp->findPluginById(QString::fromUtf8(PLUGINID_NATRON_ROTO), majorVersion, minorVersion);
    }

    ///Try remapping these ids to old ids we had in Natron < 1.0 for backward-compat
    for (PluginsMap::const_iterator it = _imp->_plugins.begin(); it != _imp->_plugins.end(); ++it) {
        assert( !it->second.empty() );
        PluginVersionsOrdered::const_reverse_iterator itver = it->second.rbegin();
        QString friendlyLabel = (*itver)->getPluginLabel();
        const QStringList& s = (*itver)->getGrouping();
        QString grouping;
        if (s.size() > 0) {
            grouping = s[0];
        }
        friendlyLabel += QString::fromUtf8("  [") + grouping + QChar::fromLatin1(']');

        if (friendlyLabel == pluginId) {
            if (majorVersion == -1) {
                // -1 means we want to load the highest version existing
                return *( it->second.rbegin() );
            }

            PluginsMap::const_iterator foundID = it;
            
            // see also OFX::Host::PluginCache::getPluginById()
            // see also AppManager::getPluginBinary()

            // Let's be a bit smarter than HostSupport.
            // The best compatible plugin is, by order of preference
            // - the plugin with the same major version and the highest minor
            // - the plugin with the closest major above and the highest minor
            // - the plugin with the highest major and the highest minor
            //
            // For example, if versions (1,0) (1,2) (3,1) (3,4) (4,2) are available,
            // - asking for (1,0) returns (1,2)
            // - asking for (2,7) returns (3,4)
            // - asking for (3,1) returns (3,4)
            // - asking for (6,0) returns (4,2)

            // Try to find the exact major version, with the highest minor
            // (thus the reverse iterator)
            Plugin* nextPlugin = *(foundID->second.rbegin());
            int nextVersion = nextPlugin->getMajorVersion();
            for (PluginVersionsOrdered::const_reverse_iterator itver = foundID->second.rbegin(); itver != foundID->second.rend(); ++itver) {
                int thisMajorVersion = (*itver)->getMajorVersion();
                if (thisMajorVersion == majorVersion) {
                    return *itver;
                } else if (thisMajorVersion > majorVersion &&
                           thisMajorVersion < nextVersion) {
                    nextPlugin = *itver;
                    nextVersion = thisMajorVersion;
                } else if (thisMajorVersion < majorVersion) {
                    // no need to continue
                    break;
                }
            }
            assert(nextPlugin != NULL); // there should be at least one version

            // Could not find the exact version... get the major version above,
            // else the highest version.
            return nextPlugin;
        }
    }

    return 0;
}
