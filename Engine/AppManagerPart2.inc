
Plugin*
AppManager::getPluginBinary(const QString & pluginId,
                            int majorVersion,
                            int minorVersion,
                            bool convertToLowerCase) const
{
    Q_UNUSED(minorVersion);
    
    PluginsMap::const_iterator foundID = _imp->_plugins.find( pluginId.toStdString() );
    if ( convertToLowerCase && foundID == _imp->_plugins.end() ) {
        foundID = _imp->_plugins.end();
        for (PluginsMap::const_iterator it = _imp->_plugins.begin(); it != _imp->_plugins.end(); ++it) {
            QString pID = QString::fromUtf8( it->first.c_str() );
            if ( !pluginId.startsWith( QString::fromUtf8(NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.") ) ) {
                QString lowerCase = pID.toLower();
                if (lowerCase == pluginId) {
                    foundID = it;
                    break;
                }
            }
        }
    }

    if ( foundID != _imp->_plugins.end() ) {
        assert( !foundID->second.empty() );

        // see also OFX::Host::PluginCache::getPluginById()
        // see also AppManager::getPluginBinaryFromOldID()

        if (majorVersion == -1) {
            // -1 means we want to load the highest version existing
            return *foundID->second.rbegin();
        }

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
    QString exc = QString::fromUtf8("Couldn't find a plugin attached to the ID %1, with a major version of %2")
                  .arg(pluginId)
                  .arg(majorVersion);

    throw std::invalid_argument( exc.toStdString() );

    return 0;
}

EffectInstancePtr
AppManager::createOFXEffect(NodePtr node,
                            const CreateNodeArgs& args
#ifndef NATRON_ENABLE_IO_META_NODES
                            ,
                            bool allowFileDialogs,
                            bool *hasUsedFileDialog
#endif
                            ) const
{
    return _imp->ofxHost->createOfxEffect(node, args
#ifndef NATRON_ENABLE_IO_META_NODES
                                          , allowFileDialogs,
                                          hasUsedFileDialog
#endif
                                          );
}

void
AppManager::removeFromNodeCache(const ImagePtr & image)
{
    _imp->_nodeCache->removeEntry(image);
}

void
AppManager::removeFromViewerCache(const FrameEntryPtr & texture)
{
    _imp->_viewerCache->removeEntry(texture);
}

void
AppManager::removeFromNodeCache(U64 hash)
{
    _imp->_nodeCache->removeEntry(hash);
}

void
AppManager::removeFromViewerCache(U64 hash)
{
    _imp->_viewerCache->removeEntry(hash);
}

void
AppManager::getMemoryStatsForCacheEntryHolder(const CacheEntryHolder* holder,
                                              std::size_t* ramOccupied,
                                              std::size_t* diskOccupied) const
{
    assert(holder);

    *ramOccupied = 0;
    *diskOccupied = 0;

    std::size_t viewerCacheMem = 0;
    std::size_t viewerCacheDisk = 0;
    std::size_t diskCacheMem = 0;
    std::size_t diskCacheDisk = 0;
    std::size_t nodeCacheMem = 0;
    std::size_t nodeCacheDisk = 0;
    const Node* isNode = dynamic_cast<const Node*>(holder);
    if (isNode) {
        ViewerInstance* isViewer = isNode->isEffectViewer();
        if (isViewer) {
            _imp->_viewerCache->getMemoryStatsForCacheEntryHolder(holder, &viewerCacheMem, &viewerCacheDisk);
        }
    }
    _imp->_diskCache->getMemoryStatsForCacheEntryHolder(holder, &diskCacheMem, &diskCacheDisk);
    _imp->_nodeCache->getMemoryStatsForCacheEntryHolder(holder, &nodeCacheMem, &nodeCacheDisk);

    *ramOccupied = diskCacheMem + viewerCacheMem + nodeCacheMem;
    *diskOccupied = diskCacheDisk + viewerCacheDisk + nodeCacheDisk;
}

void
AppManager::removeAllImagesFromCacheWithMatchingIDAndDifferentKey(const CacheEntryHolder* holder,
                                                                  U64 treeVersion)
{
    _imp->_nodeCache->removeAllEntriesWithDifferentNodeHashForHolderPublic(holder, treeVersion);
}

void
AppManager::removeAllImagesFromDiskCacheWithMatchingIDAndDifferentKey(const CacheEntryHolder* holder,
                                                                      U64 treeVersion)
{
    _imp->_diskCache->removeAllEntriesWithDifferentNodeHashForHolderPublic(holder, treeVersion);
}

void
AppManager::removeAllTexturesFromCacheWithMatchingIDAndDifferentKey(const CacheEntryHolder* holder,
                                                                    U64 treeVersion)
{
    _imp->_viewerCache->removeAllEntriesWithDifferentNodeHashForHolderPublic(holder, treeVersion);
}

void
AppManager::removeAllCacheEntriesForHolder(const CacheEntryHolder* holder,
                                           bool blocking)
{
    _imp->_nodeCache->removeAllEntriesForHolderPublic(holder, blocking);
    _imp->_diskCache->removeAllEntriesForHolderPublic(holder, blocking);
    _imp->_viewerCache->removeAllEntriesForHolderPublic(holder, blocking);
}

const QString &
AppManager::getApplicationBinaryPath() const
{
    return _imp->_binaryPath;
}

void
AppManager::setNumberOfThreads(int threadsNb)
{
    if (_imp->_settings) {
        _imp->_settings->setNumberOfThreads(threadsNb);
    }
}

bool
AppManager::getImage(const ImageKey & key,
                     std::list<ImagePtr>* returnValue) const
{
    return _imp->_nodeCache->get(key, returnValue);
}

bool
AppManager::getImageOrCreate(const ImageKey & key,
                             const ImageParamsPtr& params,
                             ImagePtr* returnValue) const
{
    return _imp->_nodeCache->getOrCreate(key, params, 0, returnValue);
}

bool
AppManager::getImage_diskCache(const ImageKey & key,
                               std::list<ImagePtr>* returnValue) const
{
    return _imp->_diskCache->get(key, returnValue);
}

bool
AppManager::getImageOrCreate_diskCache(const ImageKey & key,
                                       const ImageParamsPtr& params,
                                       ImagePtr* returnValue) const
{
    return _imp->_diskCache->getOrCreate(key, params, 0, returnValue);
}

bool
AppManager::getTexture(const FrameKey & key,
                       std::list<FrameEntryPtr>* returnValue) const
{
    std::list<FrameEntryPtr> retList;
    bool ret =  _imp->_viewerCache->get(key, &retList);

    *returnValue = retList;

    return ret;
}

bool
AppManager::getTextureOrCreate(const FrameKey & key,
                               const FrameParamsPtr& params,
                               FrameEntryLocker* locker,
                               FrameEntryPtr* returnValue) const
{
    return _imp->_viewerCache->getOrCreate(key, params, locker, returnValue);
}

bool
AppManager::isAggressiveCachingEnabled() const
{
    return _imp->_settings->isAggressiveCachingEnabled();
}

U64
AppManager::getCachesTotalMemorySize() const
{
    return  _imp->_nodeCache->getMemoryCacheSize();
}

U64
AppManager::getCachesTotalDiskSize() const
{
    return  _imp->_diskCache->getDiskCacheSize() + _imp->_viewerCache->getDiskCacheSize();
}

CacheSignalEmitterPtr
AppManager::getOrActivateViewerCacheSignalEmitter() const
{
    return _imp->_viewerCache->activateSignalEmitter();
}

SettingsPtr AppManager::getCurrentSettings() const
{
    return _imp->_settings;
}

void
AppManager::setLoadingStatus(const QString & str)
{
    if ( isLoaded() ) {
        return;
    }
    std::cout << str.toStdString() << std::endl;
}

AppInstancePtr
AppManager::makeNewInstance(int appID) const
{
    return std::make_shared<AppInstance>(appID);
}

void
AppManager::registerEngineMetaTypes() const
{
    qRegisterMetaType<Variant>("Variant");
    qRegisterMetaType<Format>("Format");
    qRegisterMetaType<SequenceTime>("SequenceTime");
    qRegisterMetaType<StandardButtons>("StandardButtons");
    qRegisterMetaType<RectI>("RectI");
    qRegisterMetaType<RectD>("RectD");
    qRegisterMetaType<RenderStatsPtr>("RenderStatsPtr");
    qRegisterMetaType<RenderStatsMap>("RenderStatsMap");
    qRegisterMetaType<ViewIdx>("ViewIdx");
    qRegisterMetaType<ViewSpec>("ViewSpec");
    qRegisterMetaType<NodePtr>("NodePtr");
    qRegisterMetaType<std::list<double> >("std::list<double>");
}

void
AppManager::refreshDiskCacheLocation()
{
    QMutexLocker k(&_imp->diskCachesLocationMutex);
    QString path = QString::fromUtf8(qgetenv(NATRON_DISK_CACHE_PATH_ENV_VAR));
    if ( !path.isEmpty() ) {
        QDir d(path);
        // create the full path if the directory does not exist
        if ( d.exists() || d.mkpath(path) ) {
            _imp->diskCachesLocation = path;
            return;
        }
    }
    _imp->diskCachesLocation = StandardPaths::writableLocation(StandardPaths::eStandardLocationCache);
}

const QString&
AppManager::getDiskCacheLocation() const
{
    QMutexLocker k(&_imp->diskCachesLocationMutex);

    return _imp->diskCachesLocation;
}

bool
AppManager::isNCacheFilesOpenedCapped() const
{
    QMutexLocker l(&_imp->currentCacheFilesCountMutex);

    return _imp->currentCacheFilesCount >= _imp->maxCacheFiles;
}

size_t
AppManager::getNCacheFilesOpened() const
{
    QMutexLocker l(&_imp->currentCacheFilesCountMutex);

    return _imp->currentCacheFilesCount;
}

void
AppManager::increaseNCacheFilesOpened()
{
    QMutexLocker l(&_imp->currentCacheFilesCountMutex);

    ++_imp->currentCacheFilesCount;
#ifdef DEBUG
    if (_imp->currentCacheFilesCount > _imp->maxCacheFiles) {
        qDebug() << "Cache has more files opened than the limit allowed:" << _imp->currentCacheFilesCount << '/' << _imp->maxCacheFiles;
    }
#endif
#ifdef NATRON_DEBUG_CACHE
    qDebug() << "N Cache Files Opened:" << _imp->currentCacheFilesCount;
#endif
}

void
AppManager::decreaseNCacheFilesOpened()
{
    QMutexLocker l(&_imp->currentCacheFilesCountMutex);

    --_imp->currentCacheFilesCount;
#ifdef NATRON_DEBUG_CACHE
    qDebug() << "NFiles Opened:" << _imp->currentCacheFilesCount;
#endif
}

void
AppManager::onMaxPanelsOpenedChanged(int maxPanels)
{
    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->onMaxPanelsOpenedChanged(maxPanels);
    }
}

void
AppManager::onQueueRendersChanged(bool queuingEnabled)
{
    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->onRenderQueuingChanged(queuingEnabled);
    }
}

int
AppManager::exec()
{
#ifdef DEBUG
    boost_adaptbx::floating_point::exception_trapping trap(0);
#endif
    return qApp->exec();
}

void
AppManager::onNodeMemoryRegistered(qint64 mem)
{
    ///runs only in the main thread
    assert( QThread::currentThread() == qApp->thread() );

    if ( ( (qint64)_imp->_nodesGlobalMemoryUse + mem ) < 0 ) {
        qDebug() << "Memory underflow...a node is trying to release more memory than it registered.";
        _imp->_nodesGlobalMemoryUse = 0;

        return;
    }

    _imp->_nodesGlobalMemoryUse += mem;
}

qint64
AppManager::getTotalNodesMemoryRegistered() const
{
    assert( QThread::currentThread() == qApp->thread() );

    return _imp->_nodesGlobalMemoryUse;
}

void
AppManager::getErrorLog_mt_safe(std::list<LogEntry>* entries) const
{
    QMutexLocker l(&_imp->errorLogMutex);
    *entries = _imp->errorLog;
}

void
AppManager::writeToErrorLog_mt_safe(const QString& context,
                                    const QDateTime& date,
                                    const QString & str,
                                    bool isHtml,
                                    const LogEntry::LogEntryColor& color)
{
    QMutexLocker l(&_imp->errorLogMutex);
    LogEntry e;
    e.context = context;
    e.date = date;
    e.message = str;
    e.isHtml = isHtml;
    e.color = color;
    _imp->errorLog.push_back(e);
}

void
AppManager::showErrorLog()
{
    std::list<LogEntry> log;
    getErrorLog_mt_safe(&log);
    for (std::list<LogEntry>::iterator it = log.begin(); it != log.end(); ++it) {
        // only print time - QTime.toString() uses the system locale, that's not what we want
        std::cout << QString::fromUtf8("[%2] %1: %3")
                     .arg(it->context)
                     .arg( QLocale().toString( it->date.time(), QString::fromUtf8("HH:mm:ss.zzz")) )
                     .arg(it->message).toStdString() << std::endl;
    }
}

void
AppManager::clearErrorLog_mt_safe()
{
    QMutexLocker l(&_imp->errorLogMutex);

    _imp->errorLog.clear();
}

void
AppManager::exitApp(bool /*warnUserForSave*/)
{
    const AppInstanceVec & instances = getAppInstances();

    for (AppInstanceVec::const_iterator it = instances.begin(); it != instances.end(); ++it) {
        (*it)->quitNow();
    }
}

#ifdef Q_OS_UNIX
QString
AppManager::qt_tildeExpansion(const QString &path,
                              bool *expanded)
{
    if (expanded != 0) {
        *expanded = false;
    }
    if ( !path.startsWith( QLatin1Char('~') ) ) {
        return path;
    }
    QString ret = path;
    QStringList tokens = ret.split( QDir::separator() );
    if ( tokens.first() == QLatin1String("~") ) {
        ret.replace( 0, 1, QDir::homePath() );
    } /*else {
         QString userName = tokens.first();
         userName.remove(0, 1);

         const QString homePath = QString::fro#if defined(Q_OS_VXWORKS)
         const QString homePath = QDir::homePath();
         #elif defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
         passwd pw;
         passwd *tmpPw;
         char buf[200];
         const int bufSize = sizeof(buf);
         int err = 0;
         #if defined(Q_OS_SOLARIS) && (_POSIX_C_SOURCE - 0 < 199506L)
         tmpPw = getpwnam_r(userName.toLocal8Bit().constData(), &pw, buf, bufSize);
         #else
         err = getpwnam_r(userName.toLocal8Bit().constData(), &pw, buf, bufSize, &tmpPw);
         #endif
         if (err || !tmpPw)
         return ret;mLocal8Bit(pw.pw_dir);
         #else
         passwd *pw = getpwnam(userName.toLocal8Bit().constData());
         if (!pw)
         return ret;
         const QString homePath = QString::fromLocal8Bit(pw->pw_dir);
         #endif
         ret.replace(0, tokens.first().length(), homePath);
         }*/
    if (expanded != 0) {
        *expanded = true;
    }

    return ret;
}

#endif

bool
AppManager::isNodeCacheAlmostFull() const
{
    std::size_t nodeCacheSize = _imp->_nodeCache->getMemoryCacheSize();
    std::size_t nodeMaxCacheSize = _imp->_nodeCache->getMaximumMemorySize();

    if (nodeMaxCacheSize == 0) {
        return true;
    }

    if ( (double)nodeCacheSize / nodeMaxCacheSize >= NATRON_CACHE_LIMIT_PERCENT ) {
        return true;
    } else {
        return false;
    }
}

void
AppManager::checkCacheFreeMemoryIsGoodEnough()
{
    ///Before allocating the memory check that there's enough space to fit in memory
    size_t systemRAMToKeepFree = getSystemTotalRAM() * appPTR->getCurrentSettings()->getUnreachableRamPercent();
    size_t totalFreeRAM = getAmountFreePhysicalRAM();

    while (totalFreeRAM <= systemRAMToKeepFree) {
#ifdef NATRON_DEBUG_CACHE
        qDebug() << "Total system free RAM is below the threshold:" << printAsRAM(totalFreeRAM)
        << ", clearing least recently used NodeCache image...";
#endif
        if ( !_imp->_nodeCache->evictLRUInMemoryEntry() ) {
            break;
        }


        totalFreeRAM = getAmountFreePhysicalRAM();
    }
}

U64
AppManager::freeDiskBytesAt(const QString& path)
{
    if ( path.isEmpty() ) {
        return 0;
    }
#if defined(__NATRON_UNIX__)
    struct statvfs st;
    const QByteArray utf8 = path.toUtf8();
    if (::statvfs(utf8.constData(), &st) != 0) {
        return 0;
    }
    return (U64)st.f_bavail * (U64)st.f_frsize;
#elif defined(__NATRON_WIN32__)
    ULARGE_INTEGER available;
    available.QuadPart = 0;
    const std::wstring wpath = StrUtils::utf8_to_utf16( path.toStdString() );
    if ( !GetDiskFreeSpaceExW(wpath.c_str(), &available, 0, 0) ) {
        return 0;
    }
    return (U64)available.QuadPart;
#else
    Q_UNUSED(path);
    return 0;
#endif
}

bool
AppManager::hasEnoughFreeDiskForCache(std::size_t extraBytes) const
{
    SettingsPtr settings = getCurrentSettings();
    const U64 reserve = settings ? settings->getMinimumFreeDiskBytes() : (512ULL * 1024ULL * 1024ULL);
    const U64 freeBytes = freeDiskBytesAt( getDiskCacheLocation() );
    if (freeBytes == 0) {
        // Unknown: let the write proceed and rely on ENOSPC handling in MemoryFile.
        return true;
    }
    const U64 need = (U64)extraBytes + reserve;
    if (freeBytes <= need) {
        std::cerr << NATRON_APPLICATION_NAME << ": refusing disk-cache growth of " << extraBytes
                  << " bytes; only " << freeBytes << " bytes free (reserve " << reserve << ")." << std::endl;
        return false;
    }
    return true;
}

void
AppManager::onOCIOConfigPathChanged(const std::string& path)
{
    _imp->currentOCIOConfigPath = path;

    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->onOCIOConfigPathChanged(path);
    }
}

const std::string&
AppManager::getOCIOConfigPath() const
{
    return _imp->currentOCIOConfigPath;
}

void
AppManager::setNThreadsToRender(int nThreads)
{
    QMutexLocker l(&_imp->nThreadsMutex);

    _imp->nThreadsToRender = nThreads;
}

void
AppManager::getNThreadsSettings(int* nThreadsToRender,
                                int* nThreadsPerEffect) const
{
    QMutexLocker l(&_imp->nThreadsMutex);

    *nThreadsToRender = _imp->nThreadsToRender;
    *nThreadsPerEffect = _imp->nThreadsPerEffect;
}

void
AppManager::setNThreadsPerEffect(int nThreadsPerEffect)
{
    QMutexLocker l(&_imp->nThreadsMutex);

    _imp->nThreadsPerEffect = nThreadsPerEffect;
}

void
AppManager::setUseThreadPool(bool useThreadPool)
{
    QMutexLocker l(&_imp->nThreadsMutex);

    _imp->useThreadPool = useThreadPool;
}

bool
AppManager::getUseThreadPool() const
{
    QMutexLocker l(&_imp->nThreadsMutex);

    return _imp->useThreadPool;
}

void
AppManager::fetchAndAddNRunningThreads(int nThreads)
{
    _imp->runningThreadsCount.fetchAndAddRelaxed(nThreads);
}

int
AppManager::getNRunningThreads() const
{
    return (int)_imp->runningThreadsCount;
}

void
AppManager::setThreadAsActionCaller(OfxImageEffectInstance* instance,
                                    bool actionCaller)
{
    _imp->ofxHost->setThreadAsActionCaller(instance, actionCaller);
}

void
AppManager::requestOFXDIalogOnMainThread(OfxImageEffectInstance* instance,
                                         void* instanceData)
{
    if ( QThread::currentThread() == qApp->thread() ) {
        onOFXDialogOnMainThreadReceived(instance, instanceData);
    } else {
        Q_EMIT s_requestOFXDialogOnMainThread(instance, instanceData);
    }
}

void
AppManager::onOFXDialogOnMainThreadReceived(OfxImageEffectInstance* instance,
                                            void* instanceData)
{
    assert( QThread::currentThread() == qApp->thread() );
    if (instance == NULL) {
        // instance may be NULL if using OfxDialogSuiteV1
        OfxHost::OfxHostDataTLSPtr tls = _imp->ofxHost->getTLSData();
        instance = tls->lastEffectCallingMainEntry;
    } else {
#ifdef DEBUG
        OfxHost::OfxHostDataTLSPtr tls = _imp->ofxHost->getTLSData();
        assert(instance == tls->lastEffectCallingMainEntry);
#endif
    }
#ifdef OFX_SUPPORTS_DIALOG
    if (instance) {
        instance->dialog(instanceData);
    }
#else
    Q_UNUSED(instanceData);
#endif
}

std::list<std::string>
AppManager::getPluginIDs() const
{
    std::list<std::string> ret;

    for (PluginsMap::const_iterator it = _imp->_plugins.begin(); it != _imp->_plugins.end(); ++it) {
        assert( !it->second.empty() );
        ret.push_back(it->first);
    }

    return ret;
}

std::list<std::string>
AppManager::getPluginIDs(const std::string& filter)
{
    QString qFilter = QString::fromUtf8( filter.c_str() );
    std::list<std::string> ret;

    for (PluginsMap::const_iterator it = _imp->_plugins.begin(); it != _imp->_plugins.end(); ++it) {
        assert( !it->second.empty() );

        QString pluginID = QString::fromUtf8( it->first.c_str() );
        if ( pluginID.contains(qFilter, Qt::CaseInsensitive) ) {
            ret.push_back(it->first);
        }
    }

    return ret;
}


void
AppManager::initPython()
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return;
#endif
    //Disable user sites as they could conflict with Natron bundled packages.
    //If this is set, Python won’t add the user site-packages directory to sys.path.
    //See https://www.python.org/dev/peps/pep-0370/
    qputenv("PYTHONNOUSERSITE", "1");
    ++Py_NoUserSiteDirectory;

    //
    // set up paths, clear those that don't exist or are not valid
    //
    QString binPath = QCoreApplication::applicationDirPath();
    binPath = QDir::toNativeSeparators(binPath);
    NATRON_PYTHON_NAMESPACE::setupPythonEnv(binPath.toStdString());

    ///Must be called prior to Py_Initialize (calls PyImport_AppendInittab())
    initBuiltinPythonModules();
    
    _imp->mainModule = NATRON_PYTHON_NAMESPACE::initializePython3(_imp->commandLineArgsWide);

    std::string err;
    std::string modulename = NATRON_ENGINE_PYTHON_MODULE_NAME;
    bool ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript("import sys\nfrom math import *\nimport " + modulename, &err, 0);
    if (!ok) {
        throw std::runtime_error( tr("Error while loading python module %1: %2").arg( QString::fromUtf8( modulename.c_str() ) ).arg( QString::fromUtf8( err.c_str() ) ).toStdString() );
    }

    ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript(modulename + ".natron = " + modulename + ".PyCoreApplication()\n", &err, 0);
    assert(ok);
    if (!ok) {
        throw std::runtime_error( tr("Error while loading python module %1: %2").arg( QString::fromUtf8( modulename.c_str() ) ).arg( QString::fromUtf8( err.c_str() ) ).toStdString() );
    }

    if ( !isBackground() ) {
        modulename = NATRON_GUI_PYTHON_MODULE_NAME;
        ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript("import sys\nimport " + modulename, &err, 0);
        assert(ok);
        if (!ok) {
            throw std::runtime_error( tr("Error while loading python module %1: %2").arg( QString::fromUtf8( modulename.c_str() ) ).arg( QString::fromUtf8( err.c_str() ) ).toStdString() );
        }

        ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript(modulename + ".natron = " +
                                                            modulename + ".PyGuiApplication()\n", &err, 0);
        assert(ok);
        if (!ok) {
            throw std::runtime_error( tr("Error while loading python module %1: %2").arg( QString::fromUtf8( modulename.c_str() ) ).arg( QString::fromUtf8( err.c_str() ) ).toStdString() );
        }

        //redirect stdout/stderr
        std::string script(
            "class StreamCatcher:\n"
            "   def __init__(self):\n"
            "       self.value = ''\n"
            "   def write(self,txt):\n"
            "       self.value += txt\n"
            "   def clear(self):\n"
            "       self.value = ''\n"
            "catchOut = StreamCatcher()\n"
            "catchErr = StreamCatcher()\n"
            "sys.stdout = catchOut\n"
            "sys.stderr = catchErr\n");
        ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript(script, &err, 0);
        assert(ok);
        if (!ok) {
            throw std::runtime_error( tr("Error while loading StreamCatcher: %1").arg( QString::fromUtf8( err.c_str() ) ).toStdString() );
        }
    }
    // Set QT_API for QtPy
    // https://github.com/spyder-ide/qtpy
    qputenv("QT_API", "pyside2");
} // AppManager::initPython

void
AppManager::tearDownPython()
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return;
#endif
    if ( !Py_IsInitialized() ) {
        return;
    }

    ///See https://web.archive.org/web/20150918224620/http://wiki.blender.org/index.php/Dev:2.4/Source/Python/API/Threads
#if !defined(NDEBUG)
    QThread* curThread = QThread::currentThread();
    QString threadname = (qApp && qApp->thread() == curThread) ? QString::fromUtf8("Main") : curThread->objectName();
    qDebug() << QString::fromUtf8("Thread '%1' is asking the Python GIL").arg(threadname);
#endif
    PyGILState_Ensure();
#if !defined(NDEBUG)
    qDebug() << QString::fromUtf8("Thread '%1' got the Python GIL").arg(threadname);
#endif

    Py_DECREF(_imp->mainModule);
#if !defined(NDEBUG)
    qDebug() << "Finalizing Python";
#endif
    Py_Finalize();
}

PyObject*
AppManager::getMainModule()
{
    return _imp->mainModule;
}

///The symbol has been generated by Shiboken in  Engine/NatronEngine/natronengine_module_wrapper.cpp
extern "C"
{
PyObject* PyInit_NatronEngine();
}

void
AppManager::initBuiltinPythonModules()
{
    int ret = PyImport_AppendInittab(NATRON_ENGINE_PYTHON_MODULE_NAME, &PyInit_NatronEngine);
    if (ret == -1) {
        throw std::runtime_error("Failed to initialize built-in Python module.");
    }
}

void
AppManager::setProjectCreatedDuringRC2Or3(bool b)
{
    _imp->lastProjectLoadedCreatedDuringRC2Or3 = b;
}

//To by-pass a bug introduced in RC3 with the serialization of bezier curves
bool
AppManager::wasProjectCreatedDuringRC2Or3() const
{
    return _imp->lastProjectLoadedCreatedDuringRC2Or3;
}

void
AppManager::toggleAutoHideGraphInputs()
{
    AppInstanceVec copy;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        copy = _imp->_appInstances;
    }

    for (AppInstanceVec::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it)->toggleAutoHideGraphInputs();
    }
}

void
AppManager::launchPythonInterpreter()
{
    std::string err;
    std::string s = "app = app1\n";
    bool ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript(s, &err, 0);

    assert(ok);
    if (!ok) {
        throw std::runtime_error("AppInstance::launchPythonInterpreter(): interpretPythonScript(" + s + " failed!");
    }

    PyGILState_Ensure(); // Py_Main does PyGILState_Release() for us.

    assert(PyThreadState_Get());
#if PY_VERSION_HEX >= 0x030400F0
    assert(PyGILState_Check()); // Not available prior to Python 3.4
#endif

    Py_Main(1, &_imp->commandLineArgsWide[0]);
}

int
AppManager::isProjectAlreadyOpened(const std::string& projectFilePath) const
{
    QMutexLocker k(&_imp->_appInstancesMutex);

    for (AppInstanceVec::iterator it = _imp->_appInstances.begin(); it != _imp->_appInstances.end(); ++it) {
        ProjectPtr proj = (*it)->getProject();
        if (proj) {
            QString path = proj->getProjectPath();
            QString name = proj->getProjectFilename();
            std::string existingProject = path.toStdString() + name.toStdString();
            if (existingProject == projectFilePath) {
                return (*it)->getAppID();
            }
        }
    }

    return -1;
}

void
AppManager::onCrashReporterNoLongerResponding()
{
#ifdef NATRON_USE_BREAKPAD
    //Crash reporter seems to no longer exist, quit
    QString error = tr("%1 has detected that the crash reporter process is no longer responding. "
                       "This most likely indicates that it was killed or that the "
                       "communication between the 2 processes is failing.")
                    .arg( QString::fromUtf8(NATRON_APPLICATION_NAME) );
    std::cerr << error.toStdString() << std::endl;
    writeToErrorLog_mt_safe(tr("Crash-Reporter"), QDateTime::currentDateTime(), error );
#endif
}

void
AppManager::setOnProjectLoadedCallback(const std::string& pythonFunc)
{
    _imp->_settings->setOnProjectLoadedCB(pythonFunc);
}

void
AppManager::setOnProjectCreatedCallback(const std::string& pythonFunc)
{
    _imp->_settings->setOnProjectCreatedCB(pythonFunc);
}

OFX::Host::ImageEffect::Descriptor*
AppManager::getPluginContextAndDescribe(OFX::Host::ImageEffect::ImageEffectPlugin* plugin,
                                        ContextEnum* ctx)
{
    return _imp->ofxHost->getPluginContextAndDescribe(plugin, ctx);
}

std::list<std::string>
AppManager::getNatronPath()
{
    std::list<std::string> ret;
    QStringList p = appPTR->getAllNonOFXPluginsPaths();

    for (QStringList::iterator it = p.begin(); it != p.end(); ++it) {
        ret.push_back( it->toStdString() );
    }

    return ret;
}

void
AppManager::appendToNatronPath(const std::string& path)
{
    appPTR->getCurrentSettings()->appendPythonGroupsPath(path);
}

#ifdef __NATRON_WIN32__
void
AppManager::registerUNCPath(const QString& path,
                            const QChar& driveLetter)
{
    assert( QThread::currentThread() == qApp->thread() );
    _imp->uncPathMapping[driveLetter] = path;
}

QString
AppManager::mapUNCPathToPathWithDriveLetter(const QString& uncPath) const
{
    assert( QThread::currentThread() == qApp->thread() );
    if ( uncPath.isEmpty() ) {
        return uncPath;
    }
    for (std::map<QChar, QString>::const_iterator it = _imp->uncPathMapping.begin(); it != _imp->uncPathMapping.end(); ++it) {
        int index = uncPath.indexOf(it->second);
        if (index == 0) {
            //We found the UNC mapping at the start of the path, replace it with a drive letter
            QString ret = uncPath;
            ret.remove( 0, it->second.size() );
            QString drive;
            drive.append(it->first);
            drive.append( QLatin1Char(':') );
            if ( !ret.isEmpty() && !ret.startsWith( QLatin1Char('/') ) ) {
                drive.append( QLatin1Char('/') );
            }
            ret.prepend(drive);

            return ret;
        }
    }

    return uncPath;
}

#endif

const IOPluginsMap&
AppManager::getFileFormatsForReadingAndReader() const
{
    return _imp->readerPlugins;
}

const IOPluginsMap&
AppManager::getFileFormatsForWritingAndWriter() const
{
    return _imp->writerPlugins;
}

void
AppManager::getSupportedReaderFileFormats(std::vector<std::string>* formats) const
{
    const IOPluginsMap& readersForFormat = getFileFormatsForReadingAndReader();

    formats->resize( readersForFormat.size() );
    int i = 0;
    for (IOPluginsMap::const_iterator it = readersForFormat.begin(); it != readersForFormat.end(); ++it, ++i) {
        (*formats)[i] = it->first;
    }
}

void
AppManager::getSupportedWriterFileFormats(std::vector<std::string>* formats) const
{
    const IOPluginsMap& writersForFormat = getFileFormatsForWritingAndWriter();

    formats->resize( writersForFormat.size() );
    int i = 0;
    for (IOPluginsMap::const_iterator it = writersForFormat.begin(); it != writersForFormat.end(); ++it, ++i) {
        (*formats)[i] = it->first;
    }
}

void
AppManager::getReadersForFormat(const std::string& format,
                                IOPluginSetForFormat* decoders) const
{
    // This will perform a case insensitive find
    IOPluginsMap::const_iterator found = _imp->readerPlugins.find(format);

    if ( found == _imp->readerPlugins.end() ) {
        return;
    }
    *decoders = found->second;
}

void
AppManager::getWritersForFormat(const std::string& format,
                                IOPluginSetForFormat* encoders) const
{
    // This will perform a case insensitive find
    IOPluginsMap::const_iterator found = _imp->writerPlugins.find(format);

    if ( found == _imp->writerPlugins.end() ) {
        return;
    }
    *encoders = found->second;
}

std::string
AppManager::getReaderPluginIDForFileType(const std::string & extension) const
{
    // This will perform a case insensitive find
    IOPluginsMap::const_iterator found = _imp->readerPlugins.find(extension);

    if ( found == _imp->readerPlugins.end() ) {
        return std::string();
    }
    // Return the "best" plug-in (i.e: higher score)

    return found->second.empty() ? std::string() : found->second.rbegin()->pluginID;
}

std::string
AppManager::getWriterPluginIDForFileType(const std::string & extension) const
{
    // This will perform a case insensitive find
    IOPluginsMap::const_iterator found = _imp->writerPlugins.find(extension);

    if ( found == _imp->writerPlugins.end() ) {
        return std::string();
    }
    // Return the "best" plug-in (i.e: higher score)

    return found->second.empty() ? std::string() : found->second.rbegin()->pluginID;
}


AppTLS*
AppManager::getAppTLS() const
{
    return &_imp->globalTLS;
}


QString
AppManager::getBoostVersion() const
{
    return QString::fromUtf8(BOOST_LIB_VERSION);
}

QString
AppManager::getQtVersion() const
{
    return QString::fromUtf8(QT_VERSION_STR) + QString::fromUtf8(" / ") + QString::fromUtf8( qVersion() );
}

QString
AppManager::getCairoVersion() const
{
    return QString::fromUtf8(CAIRO_VERSION_STRING) + QString::fromUtf8(" / ") + QString::fromUtf8( cairo_version_string() );
}


QString
AppManager::getHoedownVersion() const
{
    int major, minor, revision;
    hoedown_version(&major, &minor, &revision);
    return QString::fromUtf8(HOEDOWN_VERSION) + QString::fromUtf8(" / ") + QString::fromUtf8("%1.%2.%3").arg(major).arg(minor).arg(revision);
}


QString
AppManager::getCeresVersion() const
{
    return QString::fromUtf8(CERES_VERSION_STRING);
}


QString
AppManager::getOpenMVGVersion() const
{
    return QString::fromUtf8(OPENMVG_VERSION_STRING);
}


QString
AppManager::getPySideVersion() const
{
    return QString::fromUtf8(SHIBOKEN_VERSION);
}

const NATRON_NAMESPACE::OfxHost*
AppManager::getOFXHost() const
{
    return _imp->ofxHost.get();
}

GPUContextPool*
AppManager::getGPUContextPool() const
{
    return _imp->renderingContextPool.get();
}

void
AppManager::refreshOpenGLRenderingFlagOnAllInstances()
{
    for (std::size_t i = 0; i < _imp->_appInstances.size(); ++i) {
        _imp->_appInstances[i]->getProject()->refreshOpenGLRenderingFlagOnNodes();
    }
}

void
Dialogs::errorDialog(const std::string & title,
                     const std::string & message,
                     bool useHtml)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        topLvlInstance->errorDialog(title, message, useHtml);
    } else {
        std::cerr << "ERROR: " << title << ": " <<  message << std::endl;
    }
}

void
Dialogs::errorDialog(const std::string & title,
                     const std::string & message,
                     bool* stopAsking,
                     bool useHtml)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        topLvlInstance->errorDialog(title, message, stopAsking, useHtml);
    } else {
        std::cerr << "ERROR: " << title << ": " <<  message << std::endl;
    }
}

void
Dialogs::warningDialog(const std::string & title,
                       const std::string & message,
                       bool useHtml)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        topLvlInstance->warningDialog(title, message, useHtml);
    } else {
        std::cerr << "WARNING: " << title << ": " << message << std::endl;
    }
}

void
Dialogs::warningDialog(const std::string & title,
                       const std::string & message,
                       bool* stopAsking,
                       bool useHtml)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        topLvlInstance->warningDialog(title, message, stopAsking, useHtml);
    } else {
        std::cerr << "WARNING: " << title << ":" << message << std::endl;
    }
}

void
Dialogs::informationDialog(const std::string & title,
                           const std::string & message,
                           bool useHtml)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        topLvlInstance->informationDialog(title, message, useHtml);
    } else {
        std::cout << "INFO: " << title << ":" << message << std::endl;
    }
}

void
Dialogs::informationDialog(const std::string & title,
                           const std::string & message,
                           bool* stopAsking,
                           bool useHtml)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        topLvlInstance->informationDialog(title, message, stopAsking, useHtml);
    } else {
        std::cout << "INFO: " << title << ":" << message << std::endl;
    }
}

StandardButtonEnum
Dialogs::questionDialog(const std::string & title,
                        const std::string & message,
                        bool useHtml,
                        StandardButtons buttons,
                        StandardButtonEnum defaultButton)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        return topLvlInstance->questionDialog(title, message, useHtml, buttons, defaultButton);
    } else {
        std::cout << "QUESTION ASKED: " << title << ":" << message << std::endl;
        std::cout << NATRON_APPLICATION_NAME " answered yes." << std::endl;

        return eStandardButtonYes;
    }
}
