/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: automatic software GL fallback, cache free-disk guard, persist crash stacks.
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
#undef Py_LIMITED_API  // Needed for PyRun_SimpleString, PyRun_String, Py_NoUserSiteDirectory
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "AppManager.h"
//#include "AppManagerPrivate.h" // include breakpad after Engine, because it includes /usr/include/AssertMacros.h on OS X which defines a check(x) macro, which conflicts with boost

#if defined(__APPLE__) && defined(_LIBCPP_VERSION)
#include <AvailabilityMacros.h>
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1090
// Disable availability macros on macOS
// because we may be using libc++ on an older macOS,
// so that std::locale::numeric may be available
// even on macOS < 10.9.
// see _LIBCPP_AVAILABILITY_LOCALE_CATEGORY
// in /opt/local/libexec/llvm-5.0/include/c++/v1/__config
// and /opt/local/libexec/llvm-5.0/include/c++/v1/__locale
#if defined(_LIBCPP_USE_AVAILABILITY_APPLE)
#error "this must be compiled with _LIBCPP_DISABLE_AVAILABILITY defined"
#else
#ifndef _LIBCPP_DISABLE_AVAILABILITY
#define _LIBCPP_DISABLE_AVAILABILITY
#endif
#endif
#endif
#endif

#include <clocale>
#include <csignal>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <cstring> // for std::memcpy
#include <sstream> // stringstream
#include <locale>

#include <QtGlobal> // for Q_OS_*
#if defined(Q_OS_LINUX)
#include <sys/signal.h>
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <ucontext.h>
#include <execinfo.h>
#endif

#ifdef Q_OS_UNIX
#include <stdio.h>
#include <stdlib.h>
#ifdef Q_OS_DARWIN
#include <sys/sysctl.h>
#include <libproc.h>
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD) || defined(Q_OS_DARWIN)
#include <sys/statvfs.h>
#endif
#endif

#ifdef Q_OS_WIN
#include <shlobj.h>
#endif

#include <cairo/cairo.h>
#include <boost/version.hpp>
#include <libs/hoedown/src/version.h>
#include <ceres/version.h>
#include <openMVG/version.hpp>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QSettings>
#include <QThread>
#include <QThreadPool>
#include <QTextStream>
#include <QAbstractSocket>
#include <QLocalServer>
#include <QLocalSocket>


#include "Global/ProcInfo.h"
#include "Global/GLIncludes.h"
#include "Global/StrUtils.h"
#ifdef DEBUG
#include "Global/FloatingPointExceptions.h"
#endif
#include "Global/PythonUtils.h"

#include "Engine/AppInstance.h"
#include "Engine/Backdrop.h"
#include "Engine/CLArgs.h"
#include "Engine/DiskCacheNode.h"
#include "Engine/Dot.h"
#include "Engine/ExistenceCheckThread.h"
#include "Engine/FileSystemModel.h"
#include "Engine/GroupInput.h"
#include "Engine/GroupOutput.h"
#include "Engine/JoinViewsNode.h"
#include "Engine/LibraryBinary.h"
#include "Engine/Log.h"
#include "Engine/MemoryInfo.h" // getSystemTotalRAM, printAsRAM
#include "Engine/Node.h"
#include "Engine/OfxImageEffectInstance.h"
#include "Engine/OfxEffectInstance.h"
#include "Engine/OfxHost.h"
#include "Engine/OSGLContext.h"
#include "Engine/GPUContextPool.h"
#include "Engine/OneViewNode.h"
#include "Engine/ProcessHandler.h" // ProcessInputChannel
#include "Engine/Project.h"
#include "Engine/PrecompNode.h"
#include "Engine/ReadNode.h"
#include "Engine/RotoPaint.h"
#include "Engine/RotoSmear.h"
#include "Engine/StandardPaths.h"
#include "Engine/TrackerNode.h"
#include "Engine/ThreadPool.h"
#include "Engine/Utils.h"
#include "Engine/ViewIdx.h"
#include "Engine/ViewerInstance.h" // RenderStatsMap
#include "Engine/WriteNode.h"

#include "sbkversion.h" // shiboken/pyside version

#include "AppManagerPrivate.h" // include breakpad after Engine, because it includes /usr/include/AssertMacros.h on OS X which defines a check(x) macro, which conflicts with boost

NATRON_NAMESPACE_ENTER

AppManager* AppManager::_instance = 0;

#ifdef __NATRON_UNIX__

//namespace  {
static void
handleShutDownSignal( int /*signalId*/ )
{
    if (appPTR) {
        std::cerr << "\nCaught termination signal, exiting!" << std::endl;
        appPTR->quitApplication();
    }
}

static void
setShutDownSignal(int signalId)
{
#if defined(__NATRON_UNIX__)
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handleShutDownSignal;
    if (sigaction(signalId, &sa, NULL) == -1) {
        std::perror("setting up termination signal");
        std::exit(1);
    }
#else
    std::signal(signalId, handleShutDownSignal);
#endif
}

#endif


#if defined(__NATRON_LINUX__) && !defined(__FreeBSD__)

#define NATRON_UNIX_BACKTRACE_STACK_DEPTH 64

static void
writeCrashDumpFile(const char* text)
{
    // Best-effort: existing handler already uses non-async-signal-safe APIs.
    QString dirPath;
    if (qApp) {
        dirPath = StandardPaths::writableLocation(StandardPaths::eStandardLocationCache) + QString::fromUtf8("/crashes");
    } else {
        const char* home = std::getenv("HOME");
        dirPath = QString::fromUtf8(home ? home : "/tmp") + QString::fromUtf8("/.cache/Natron+/crashes");
    }
    QDir().mkpath(dirPath);
    const QString filePath = dirPath + QString::fromUtf8("/crash-") +
                             QDateTime::currentDateTimeUtc().toString(QString::fromUtf8("yyyyMMddThhmmss")) +
                             QString::fromUtf8(".txt");
    QFile f(filePath);
    if ( f.open(QIODevice::WriteOnly | QIODevice::Truncate) ) {
        f.write(text);
        f.write("\nSee tools/crash/README.md for turning this dump into a ticket.\n");
        f.close();
        std::cerr << "Crash dump written to " << filePath.toStdString() << std::endl;
    }
    if (qApp) {
        const QString besideBinary = QCoreApplication::applicationDirPath() + QString::fromUtf8("/last-crash.txt");
        QFile copy(besideBinary);
        if ( copy.open(QIODevice::WriteOnly | QIODevice::Truncate) ) {
            copy.write(text);
            copy.close();
        }
    }
}

static void
backTraceSigSegvHandler(int sig,
                        siginfo_t *info,
                        void *secret)
{
    void *trace[NATRON_UNIX_BACKTRACE_STACK_DEPTH];
    char **messages = (char **)NULL;
    int i, trace_size = 0;
    ucontext_t *uc = (ucontext_t *)secret;
    std::ostringstream dump;

    dump << NATRON_APPLICATION_NAME << " crash (" << (sig == SIGSEGV ? "SIGSEGV" : sig == SIGABRT ? "SIGABRT" : "signal") << ")\n";

    /* Do something useful with siginfo_t */
    if (sig == SIGSEGV) {
        QThread* curThread = QThread::currentThread();
        std::string threadName;
        if (curThread) {
            threadName = (qApp && qApp->thread() == curThread) ? "Main" : curThread->objectName().toStdString();
        }
        dump << "Caught segmentation fault (SIGSEGV) from thread "  << threadName << "(" << curThread << "), faulty address is " <<
             #ifndef __x86_64__
        (void*)uc->uc_mcontext.gregs[REG_EIP]
             #else
            (void*) uc->uc_mcontext.gregs[REG_RIP]
             #endif
            << " from " << info->si_addr << std::endl;
    } else {
        dump << "Got signal " << sig << std::endl;
    }

    trace_size = backtrace(trace, NATRON_UNIX_BACKTRACE_STACK_DEPTH);
    /* overwrite sigaction with caller's address */
#ifndef __x86_64__
    trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];
#else
    if (uc) {
        trace[1] = (void *) uc->uc_mcontext.gregs[REG_RIP];
    }
#endif


    messages = backtrace_symbols(trace, trace_size);
    dump << "Backtrace:" << std::endl;
    for (i = 1; i < trace_size; ++i) {
        dump << "[Frame " << i << "]: " << messages[i] << std::endl;
    }
    const std::string text = dump.str();
    std::cerr << text;
    writeCrashDumpFile(text.c_str());
    exit(1);
}

static void
setSigSegvSignal()
{
    struct sigaction sa;

    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    /* if SA_SIGINFO is set, sa_sigaction is to be used instead of sa_handler. */
    sa.sa_sigaction = backTraceSigSegvHandler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        std::perror("setting up sigsegv signal");
        std::exit(1);
    }
    if (sigaction(SIGABRT, &sa, NULL) == -1) {
        std::perror("setting up sigabrt signal");
    }
}

#endif // if defined(__NATRON_LINUX__) && !defined(__FreeBSD__)

//} // anon namespace

void
AppManager::saveCaches() const
{
    _imp->saveCaches();
}

int
AppManager::getHardwareIdealThreadCount()
{
    return _imp->idealThreadCount;
}

int
AppManager::getMaxThreadCount()
{
    return QThreadPool::globalInstance()->maxThreadCount();
}

AppManager::AppManager()
    : QObject()
    , _imp( new AppManagerPrivate() )
{
    assert(!_instance);
    _instance = this;

    QObject::connect( this, SIGNAL(s_requestOFXDialogOnMainThread(OfxImageEffectInstance*,void*)), this, SLOT(onOFXDialogOnMainThreadReceived(OfxImageEffectInstance*,void*)) );

#ifdef __NATRON_WIN32__
    FileSystemModel::initDriveLettersToNetworkShareNamesMapping();
#endif
}

#ifdef USE_NATRON_GIL
void
AppManager::takeNatronGIL()
{
    _imp->natronPythonGIL.lock();
}

void
AppManager::releaseNatronGIL()
{
    _imp->natronPythonGIL.unlock();
}
#endif

void
StrUtils::ensureLastPathSeparator(QString& path)
{
    static const QChar separator( QLatin1Char('/') );

    if ( !path.endsWith(separator) ) {
        path += separator;
    }
}


bool
AppManager::loadFromArgs(const CLArgs& cl)
{

#ifdef DEBUG
    for (std::size_t i = 0; i < _imp->commandLineArgsWide.size(); ++i) {
        std::cout << "argv[" << i << "] = " << StrUtils::utf16_to_utf8( std::wstring(_imp->commandLineArgsWide[i]) ) << std::endl;
    }
#endif

    // This needs to be done BEFORE creating qApp because
    // on Linux, X11 will create a context that would corrupt
    // the XUniqueContext created by Qt
    // scoped_ptr
    _imp->renderingContextPool.reset( new GPUContextPool() );
    initializeOpenGLFunctionsOnce(true);

    //  QCoreApplication will hold a reference to that appManagerArgc integer until it dies.
    //  Thus ensure that the QCoreApplication is destroyed when returning this function.
    initializeQApp(_imp->nArgs, &_imp->commandLineArgsUtf8.front()); // calls QCoreApplication::QCoreApplication(), which calls setlocale()
    // see C++ standard 23.2.4.2 vector capacity [lib.vector.capacity]
    // resizing to a smaller size doesn't free/move memory, so the data pointer remains valid
    assert(_imp->nArgs <= (int)_imp->commandLineArgsUtf8.size());
    _imp->commandLineArgsUtf8.resize(_imp->nArgs); // Qt may have reduced the numlber of args

#ifdef QT_CUSTOM_THREADPOOL
    // Set the global thread pool (pointed is owned and deleted by QThreadPool at exit)
    QThreadPool::setGlobalInstance(new ThreadPool);
#endif

    // set fontconfig path on all platforms
    if ( qgetenv("FONTCONFIG_PATH").isNull() ) {
        // set FONTCONFIG_PATH to Natron/Resources/etc/fonts (required by plugins using fontconfig)
        QString path = QCoreApplication::applicationDirPath() + QString::fromUtf8("/../Resources/etc/fonts");
        QFileInfo fileInfo(path);
        if ( !fileInfo.exists() ) {
            std::cerr <<  "Fontconfig configuration file " << path.toStdString() << " does not exist, not setting FONTCONFIG_PATH "<< std::endl;
        } else {
            QString fcPath = fileInfo.canonicalFilePath();

            std::string stdFcPath = fcPath.toStdString();

            qDebug() << "Setting FONTCONFIG_PATH to" << stdFcPath.c_str();
#if 0 //def __NATRON_WIN32__ // commented out in https://github.com/NatronGitHub/Natron/commit/3445d671f15fbd97bca164b53ceb41cef47c61c3
            // qputenv on mingw will just call putenv, but we want to keep the utf16 info, so we need to call _wputenv
            _wputenv_s(L"FONTCONFIG_PATH", StrUtils::utf8_to_utf16(stdFcPath).c_str());
#else
             qputenv( "FONTCONFIG_PATH", stdFcPath.c_str() );
#endif
        }
    }

    try {
        initPython(); // calls Py_InitializeEx(), which calls setlocale()
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;

        return false;
    }

    _imp->idealThreadCount = QThread::idealThreadCount();


    QThreadPool::globalInstance()->setExpiryTimeout(-1); //< make threads never exit on their own
    //otherwise it might crash with thread-local storage


    ///the QCoreApplication must have been created so far.
    assert(qApp);

    bool ret = false;
    try {
        ret = loadInternal(cl);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
    return ret;
} // loadFromArgs

bool
AppManager::load(int argc,
                 char **argv,
                 const CLArgs& cl)
{
    // Ensure application has correct locale before doing anything
    // Warning: Qt resets it in the QCoreApplication constructor
    // see http://doc.qt.io/qt-4.8/qcoreapplication.html#locale-settings
    setApplicationLocale();
    _imp->handleCommandLineArgs(argc, argv);
    return loadFromArgs(cl);
}

bool
AppManager::loadW(int argc,
                 wchar_t **argv,
                 const CLArgs& cl)
{
    // Ensure application has correct locale before doing anything
    // Warning: Qt resets it in the QCoreApplication constructor
    // see http://doc.qt.io/qt-4.8/qcoreapplication.html#locale-settings
    setApplicationLocale();
    _imp->handleCommandLineArgsW(argc, argv);
    return loadFromArgs(cl);
}

AppManager::~AppManager()
{
#ifdef NATRON_USE_BREAKPAD
    if (_imp->breakpadAliveThread) {
        _imp->breakpadAliveThread->quitThread();
    }
#endif

    bool appsEmpty;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        appsEmpty = _imp->_appInstances.empty();
    }
    while (!appsEmpty) {
        AppInstancePtr front;
        {
            QMutexLocker k(&_imp->_appInstancesMutex);
            front = _imp->_appInstances.front();
        }
        if (front) {
            front->quitNow();
        }
        {
            QMutexLocker k(&_imp->_appInstancesMutex);
            appsEmpty = _imp->_appInstances.empty();
        }
    }

    for (PluginsMap::iterator it = _imp->_plugins.begin(); it != _imp->_plugins.end(); ++it) {
        for (PluginVersionsOrdered::reverse_iterator itver = it->second.rbegin(); itver != it->second.rend(); ++itver) {
            delete *itver;
        }
    }

    _imp->_backgroundIPC.reset();

    try {
        _imp->saveCaches();
    } catch (std::runtime_error&) {
        // ignore errors
    }

    ///Caches may have launched some threads to delete images, wait for them to be done
    QThreadPool::globalInstance()->waitForDone();

    ///Kill caches now because decreaseNCacheFilesOpened can be called
    if (_imp->_nodeCache) {
        _imp->_nodeCache->waitForDeleterThread();
    }
    if (_imp->_diskCache) {
        _imp->_diskCache->waitForDeleterThread();
    }
    if (_imp->_viewerCache) {
        _imp->_viewerCache->waitForDeleterThread();
    }
    _imp->_nodeCache.reset();
    _imp->_viewerCache.reset();
    _imp->_diskCache.reset();

    tearDownPython();
    _imp->tearDownGL();

    _instance = 0;

    // After this line, everything is cleaned-up (should be) and the process may resume in the main and could in theory be able to re-create a new AppManager
    _imp->_qApp.reset();
}

class QuitInstanceArgs
    : public GenericWatcherCallerArgs
{
public:

    AppInstanceWPtr instance;

    QuitInstanceArgs()
        : GenericWatcherCallerArgs()
        , instance()
    {
    }

    virtual ~QuitInstanceArgs() {}
};

typedef std::shared_ptr<QuitInstanceArgs> QuitInstanceArgsPtr;

void
AppManager::afterQuitProcessingCallback(const GenericWatcherCallerArgsPtr& args)
{
    QuitInstanceArgs* inArgs = dynamic_cast<QuitInstanceArgs*>( args.get() );

    if (!inArgs) {
        return;
    }

    AppInstancePtr instance = inArgs->instance.lock();

    instance->aboutToQuit();

    appPTR->removeInstance( instance->getAppID() );

    int nbApps = getNumInstances();
    ///if we exited the last instance, exit the event loop, this will make
    /// the exec() function return.
    if (nbApps == 0) {
        assert(qApp);
        qApp->quit();
    }

    // This should kill the AppInstance
    instance.reset();
}

void
AppManager::quitNow(const AppInstancePtr& instance)
{
    NodesList nodesToWatch;

    instance->getProject()->getNodes_recursive(nodesToWatch, false);
    if ( !nodesToWatch.empty() ) {
        for (NodesList::iterator it = nodesToWatch.begin(); it != nodesToWatch.end(); ++it) {
            (*it)->quitAnyProcessing_blocking(false);
        }
    }
    QuitInstanceArgsPtr args = std::make_shared<QuitInstanceArgs>();
    args->instance = instance;
    afterQuitProcessingCallback(args);
}

void
AppManager::quit(const AppInstancePtr& instance)
{
    QuitInstanceArgsPtr args = std::make_shared<QuitInstanceArgs>();

    args->instance = instance;
    if ( !instance->getProject()->quitAnyProcessingForAllNodes(this, args) ) {
        afterQuitProcessingCallback(args);
    }
}

void
AppManager::quitApplication()
{
    bool appsEmpty;
    {
        QMutexLocker k(&_imp->_appInstancesMutex);
        appsEmpty = _imp->_appInstances.empty();
    }

    while (!appsEmpty) {
        AppInstancePtr app;
        {
            QMutexLocker k(&_imp->_appInstancesMutex);
            app = _imp->_appInstances.front();
        }
        if (app) {
            quitNow(app);
        }

        {
            QMutexLocker k(&_imp->_appInstancesMutex);
            appsEmpty = _imp->_appInstances.empty();
        }
    }
}

void
AppManager::initializeQApp(int &argc,
                           char **argv)
{
    assert(!_imp->_qApp);
    // scoped_ptr
    _imp->_qApp.reset( new QCoreApplication(argc, argv) );
}

// setApplicationLocale is called twice:
// - before parsing the command-line arguments
// - after the QCoreApplication was constructed, because the QCoreApplication
// constructor resets the locale to the system locale
// see http://doc.qt.io/qt-4.8/qcoreapplication.html#locale-settings
void
AppManager::setApplicationLocale()
{
    // Natron is not yet internationalized, so it is better for now to use the "C" locale,
    // until it is tested for robustness against locale choice.
    // The locale affects numerics printing and scanning, date and time.
    // Note that with other locales (e.g. "de" or "fr"), the floating-point numbers may have
    // a comma (",") as the decimal separator instead of a point (".").
    // There is also an OpenCOlorIO issue with non-C numeric locales:
    // https://github.com/imageworks/OpenColorIO/issues/297
    //
    // this must be done after initializing the QCoreApplication, see
    // https://qt-project.org/doc/qt-5/qcoreapplication.html#locale-settings

    // Set the C and C++ locales
    // see http://en.cppreference.com/w/cpp/locale/locale/global
    // Maybe this can also workaround the OSX crash in loadlocale():
    // https://discussions.apple.com/thread/3479591
    // https://github.com/cth103/dcpomatic/blob/master/src/lib/safe_stringstream.h
    // stringstreams don't seem to be thread-safe on OSX because the change the locale.

    // We also set explicitly the LC_NUMERIC locale to "C" to avoid juggling
    // between locales when using stringstreams.
    // See function __convert_from_v(...) in
    // /usr/include/c++/4.2.1/x86_64-apple-darwin10/bits/c++locale.h
    // https://www.opensource.apple.com/source/libstdcxx/libstdcxx-104.1/include/c++/4.2.1/bits/c++locale.h
    // See also https://stackoverflow.com/questions/22753707/is-ostream-operator-in-libstdc-thread-hostile

    // set the C++ locale first
#if defined(__APPLE__) && defined(_LIBCPP_VERSION) && (defined(_LIBCPP_USE_AVAILABILITY_APPLE) || !defined(_LIBCPP_DISABLE_AVAILABILITY)) && (__MAC_OS_X_VERSION_MIN_REQUIRED < 1090)
    try {
        std::locale::global( std::locale("C") );
    } catch (std::runtime_error&) {
        qDebug() << "Could not set C++ locale!";
    }
#else
    try {
        std::locale::global( std::locale(std::locale("en_US.UTF-8"), "C", std::locale::numeric) );
    } catch (std::runtime_error&) {
        try {
            std::locale::global( std::locale(std::locale("C.UTF-8"), "C", std::locale::numeric) );
        } catch (std::runtime_error&) {
            try {
                std::locale::global( std::locale(std::locale("UTF-8"), "C", std::locale::numeric) );
            } catch (std::runtime_error&) {
                try {
                    std::locale::global( std::locale("C") );
                } catch (std::runtime_error&) {
                    qDebug() << "Could not set C++ locale!";
                }
            }
        }
    }
#endif

    // set the C locale second, because it will not overwrite the changes you made to the C++ locale
    // see https://stackoverflow.com/questions/12373341/does-stdlocaleglobal-make-affect-to-printf-function
    char *category = std::setlocale(LC_ALL, "en_US.UTF-8");
    if (category == NULL) {
        category = std::setlocale(LC_ALL, "C.UTF-8");
    }
    if (category == NULL) {
        category = std::setlocale(LC_ALL, "UTF-8");
    }
    if (category == NULL) {
        category = std::setlocale(LC_ALL, "C");
    }
    if (category == NULL) {
        qDebug() << "Could not set C locale!";
    }
    std::setlocale(LC_NUMERIC, "C"); // set the locale for LC_NUMERIC only
    QLocale::setDefault( QLocale(QLocale::English, QLocale::UnitedStates) );
}

bool
AppManager::loadInternal(const CLArgs& cl)
{
    assert(!_imp->_loaded);

    _imp->_binaryPath = QCoreApplication::applicationDirPath();
    assert(StrUtils::is_utf8(_imp->_binaryPath.toStdString().c_str()));

    registerEngineMetaTypes();
    registerGuiMetaTypes();

    qApp->setOrganizationName( QString::fromUtf8(NATRON_ORGANIZATION_NAME) );
    qApp->setOrganizationDomain( QString::fromUtf8(NATRON_ORGANIZATION_DOMAIN) );
    qApp->setApplicationName( QString::fromUtf8(NATRON_APPLICATION_NAME) );

    //Set it once setApplicationName is set since it relies on it
    refreshDiskCacheLocation();

    // Set the locale AGAIN, because Qt resets it in the QCoreApplication constructor and in Py_InitializeEx
    // see http://doc.qt.io/qt-4.8/qcoreapplication.html#locale-settings
    setApplicationLocale();
    
    Log::instance(); //< enable logging
    bool mustSetSignalsHandlers = true;
#ifdef NATRON_USE_BREAKPAD
    //Enabled breakpad only if the process was spawned from the crash reporter
    const QString& breakpadProcessExec = cl.getBreakpadProcessExecutableFilePath();
    if ( !breakpadProcessExec.isEmpty() && QFile::exists(breakpadProcessExec) ) {
        _imp->breakpadProcessExecutableFilePath = breakpadProcessExec;
        _imp->breakpadProcessPID = (Q_PID)cl.getBreakpadProcessPID();
        const QString& breakpadPipePath = cl.getBreakpadPipeFilePath();
        const QString& breakpadComPipePath = cl.getBreakpadComPipeFilePath();
        int breakpad_client_fd = cl.getBreakpadClientFD();
        _imp->initBreakpad(breakpadPipePath, breakpadComPipePath, breakpad_client_fd);
        mustSetSignalsHandlers = false;
    }
#endif


# ifdef __NATRON_UNIX__
    if (mustSetSignalsHandlers) {
        setShutDownSignal(SIGINT);   // shut down on ctrl-c
        setShutDownSignal(SIGTERM);   // shut down on killall
#     if defined(__NATRON_LINUX__) && !defined(__FreeBSD__)
        //Catch SIGSEGV only when google-breakpad is not active
        setSigSegvSignal();
#     endif
    }
# else
    Q_UNUSED(mustSetSignalsHandlers);
# endif


    _imp->_settings = std::make_shared<Settings>();
    _imp->_settings->initializeKnobsPublic();

    bool hasGLForRendering = hasOpenGLForRequirements(eOpenGLRequirementsTypeRendering, 0);
    if (_imp->hasInitializedOpenGLFunctions && hasGLForRendering) {
        OSGLContext::getGPUInfos(_imp->openGLRenderers);
        for (std::list<OpenGLRendererInfo>::iterator it = _imp->openGLRenderers.begin(); it != _imp->openGLRenderers.end(); ++it) {
            qDebug() << "Found OpenGL Renderer:" << it->rendererName.c_str() << ", Vendor:" << it->vendorName.c_str()
                     << ", OpenGL Version:" << it->glVersionString.c_str() << ", Max. Texture Size" << it->maxTextureSize <<
                ",Max GPU Memory:" << printAsRAM(it->maxMemBytes);;
        }
    }
    _imp->_settings->populateOpenGLRenderers(_imp->openGLRenderers);


    // Settings: we must load these and set the custom settings (using python) ASAP, before creating the OFX Plugin Cache
    // Settings: always call restoreSettings, but call restoreKnobsFromSettings conditionally
    // Call restore after initializing knobs
    _imp->_settings->restoreSettings( cl.isLoadedUsingDefaultSettings() );
    if (cl.isLoadedUsingDefaultSettings()) {
        _imp->_settings->setSaveSettings(false);
    }

    _imp->declareSettingsToPython();

    // executeCommandLineSettingCommands
    {
        const std::list<std::string>& commands = cl.getSettingCommands();

        // do not save settings if there is a --setting option
        if ( !commands.empty() ) {
            _imp->_settings->setSaveSettings(false);
        }
        for (std::list<std::string>::const_iterator it = commands.begin(); it != commands.end(); ++it) {
            std::string err;
            std::string output;
            bool ok  = NATRON_PYTHON_NAMESPACE::interpretPythonScript(*it, &err, &output);
            if (!ok) {
                const QString sp( QString::fromUtf8(" ") );
                QString m = tr("Failed to execute the following Python command:") + sp +
                QString::fromUtf8( it->c_str() ) + sp +
                tr("Error:") + sp +
                QString::fromUtf8( err.c_str() );
                throw std::runtime_error( m.toStdString() );
            } else if ( !output.empty() ) {
                std::cout << output << std::endl;
            }
        }
    }

    ///basically show a splashScreen load fonts etc...
    return initGui(cl);
} // loadInternal

const std::list<OpenGLRendererInfo>&
AppManager::getOpenGLRenderers() const
{
    return _imp->openGLRenderers;
}

bool
AppManager::isSpawnedFromCrashReporter() const
{
#ifdef NATRON_USE_BREAKPAD

    return _imp->breakpadHandler.get() != 0;
#else

    return false;
#endif
}

void
AppManager::setPluginsUseInputImageCopyToRender(bool b)
{
    _imp->pluginsUseInputImageCopyToRender = b;
}

bool
AppManager::isCopyInputImageForPluginRenderEnabled() const
{
    return _imp->pluginsUseInputImageCopyToRender;
}

bool
AppManager::isOnWayland() const
{
    return _imp->onWayland;
}

bool
AppManager::isOpenGLLoaded() const
{
    QMutexLocker k(&_imp->openGLFunctionsMutex);

    return _imp->hasInitializedOpenGLFunctions;
}

bool
AppManager::isTextureFloatSupported() const
{
    return _imp->glHasTextureFloat;
}

bool
AppManager::hasOpenGLForRequirements(OpenGLRequirementsTypeEnum type, QString* missingOpenGLError ) const
{
    std::map<OpenGLRequirementsTypeEnum,AppManagerPrivate::OpenGLRequirementsData>::const_iterator found =  _imp->glRequirements.find(type);
    assert(found != _imp->glRequirements.end());
    if (found == _imp->glRequirements.end()) {
        return false;
    }
    if (missingOpenGLError && !found->second.hasRequirements) {
        *missingOpenGLError = found->second.error;
    }
    return found->second.hasRequirements;
}

bool
AppManager::initializeOpenGLFunctionsOnce(bool createOpenGLContext)
{
    QMutexLocker k(&_imp->openGLFunctionsMutex);

    if (!_imp->hasInitializedOpenGLFunctions) {
        OSGLContextPtr glContext;
        bool checkRenderingReq = true;
        if (createOpenGLContext) {
            auto markGlUnavailable = [this](const QString& message) {
                AppManagerPrivate::OpenGLRequirementsData& vdata = _imp->glRequirements[eOpenGLRequirementsTypeViewer];
                AppManagerPrivate::OpenGLRequirementsData& rdata = _imp->glRequirements[eOpenGLRequirementsTypeRendering];
                rdata.error = message;
                vdata.error = message;
                rdata.hasRequirements = false;
                vdata.hasRequirements = false;
                AppManagerPrivate::addOpenGLRequirementsString(rdata.error, eOpenGLRequirementsTypeRendering);
                AppManagerPrivate::addOpenGLRequirementsString(vdata.error, eOpenGLRequirementsTypeViewer);
            };

            auto enableSoftwareOpenGLFallback = [](const std::string& reason) {
                std::cerr << NATRON_APPLICATION_NAME << ": " << reason << std::endl;
                std::cerr << NATRON_APPLICATION_NAME
                          << ": retrying with software OpenGL (LIBGL_ALWAYS_SOFTWARE=1, Qt software rasterizer)."
                          << std::endl;
                qputenv("LIBGL_ALWAYS_SOFTWARE", "1");
                qputenv("GALLIUM_DRIVER", "llvmpipe");
                QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL, true);
            };

            for (int attempt = 0; attempt < 2 && !glContext; ++attempt) {
                try {
                    _imp->initGLAPISpecific();

                    glContext = _imp->renderingContextPool->attachGLContextToRender(false /*checkIfGLLoaded*/);
                    if (glContext) {
                        glContext->setContextCurrentNoRender();
                    } else if (attempt == 0) {
                        enableSoftwareOpenGLFallback("OpenGL context creation failed.");
                        if (_imp->renderingContextPool) {
                            _imp->renderingContextPool->clear();
                        }
                        _imp->tearDownGL();
                        _imp->renderingContextPool.reset( new GPUContextPool() );
                    } else {
                        markGlUnavailable( tr("Error creating OpenGL context (hardware and software).") );
                        checkRenderingReq = false;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error while loading OpenGL: " << e.what() << std::endl;
                    if (attempt == 0) {
                        enableSoftwareOpenGLFallback(e.what());
                        if (_imp->renderingContextPool) {
                            _imp->renderingContextPool->clear();
                        }
                        _imp->tearDownGL();
                        _imp->renderingContextPool.reset( new GPUContextPool() );
                    } else {
                        std::cerr << "OpenGL rendering is disabled." << std::endl;
                        markGlUnavailable( tr("Error while creating OpenGL context: %1").arg( QString::fromUtf8( e.what() ) ) );
                        checkRenderingReq = false;
                    }
                }
            }
            if (!glContext) {
                return false;
            }
        }

        // The following requires a valid OpenGL context to be created
        _imp->initGl(checkRenderingReq);
        if (createOpenGLContext) {
            if (hasOpenGLForRequirements(eOpenGLRequirementsTypeRendering)) {
                try {
                    OSGLContext::checkOpenGLVersion();
                } catch (const std::exception& e) {
                    AppManagerPrivate::OpenGLRequirementsData& data = _imp->glRequirements[eOpenGLRequirementsTypeRendering];
                    data.hasRequirements = false;
                    if ( !data.error.isEmpty() ) {
                        data.error = QString::fromUtf8( e.what() );
                    }
                }
            }
            
            _imp->renderingContextPool->releaseGLContextFromRender(glContext);
            glContext->unsetCurrentContextNoRender();

            // Clear created contexts because this context was created with the "default" OpenGL renderer and it might be different from the one
            // selected by the user in the settings (which are not created yet).
            _imp->renderingContextPool->clear();
        } else {
            updateAboutWindowLibrariesVersion();
        }

        return true;
    }

    return false;
}

#ifdef __NATRON_WIN32__
const OSGLContext_wgl_data*
AppManager::getWGLData() const
{
    return _imp->wglInfo.get();
}

#endif
#ifdef __NATRON_LINUX__
const OSGLContext_egl_data*
AppManager::getEGLData() const
{
    return _imp->eglInfo.get();
}

const OSGLContext_glx_data*
AppManager::getGLXData() const
{
    return _imp->glxInfo.get();
}

#endif


bool
AppManager::initGui(const CLArgs& cl)
{
    ///In background mode, directly call the rest of the loading code
    return loadInternalAfterInitGui(cl);
}

bool
AppManager::loadInternalAfterInitGui(const CLArgs& cl)
{
    try {
        size_t maxCacheRAM = _imp->_settings->getRamMaximumPercent() * getSystemTotalRAM();
        U64 viewerCacheSize = _imp->_settings->getMaximumViewerDiskCacheSize();
        U64 maxDiskCacheNode = _imp->_settings->getMaximumDiskCacheNodeSize();

        _imp->_nodeCache = std::make_shared<Cache<Image> >("NodeCache", NATRON_CACHE_VERSION, maxCacheRAM, 1.);
        _imp->_diskCache = std::make_shared<Cache<Image> >("DiskCache", NATRON_CACHE_VERSION, maxDiskCacheNode, 0.);
        _imp->_viewerCache = std::make_shared<Cache<FrameEntry> >("ViewerCache", NATRON_CACHE_VERSION, viewerCacheSize, 0.);
        _imp->setViewerCacheTileSize();
    } catch (std::logic_error&) {
        // ignore
    }

    int oldCacheVersion = 0;
    {
        QSettings settings( QString::fromUtf8(NATRON_ORGANIZATION_NAME), QString::fromUtf8(NATRON_APPLICATION_NAME) );

        if ( settings.contains( QString::fromUtf8(kNatronCacheVersionSettingsKey) ) ) {
            oldCacheVersion = settings.value( QString::fromUtf8(kNatronCacheVersionSettingsKey) ).toInt();
        }
        settings.setValue(QString::fromUtf8(kNatronCacheVersionSettingsKey), NATRON_CACHE_VERSION);
    }

    if (oldCacheVersion != NATRON_CACHE_VERSION || cl.isCacheClearRequestedOnLaunch()) {
        setLoadingStatus( tr("Clearing the image cache...") );
        wipeAndCreateDiskCacheStructure();
    } else {
        setLoadingStatus( tr("Restoring the image cache...") );
        _imp->restoreCaches();
    }

    if (cl.isOpenFXCacheClearRequestedOnLaunch()) {
        setLoadingStatus( tr("Clearing the OpenFX Plugins cache...") );
        clearPluginsLoadedCache();
    } else {
        setLoadingStatus( tr("Loading plugin cache...") );
    }


    ///Set host properties after restoring settings since it depends on the host name.
    try {
        _imp->ofxHost->setProperties();
    } catch (std::logic_error&) {
        // ignore
    }

    /*loading all plugins*/
    try {
        loadAllPlugins();
        _imp->loadBuiltinFormats();
    } catch (std::logic_error&) {
        // ignore
    }

    if ( isBackground() && !cl.getIPCPipeName().isEmpty() ) {
        _imp->initProcessInputChannel( cl.getIPCPipeName() );
    }


    if ( cl.isInterpreterMode() ) {
        _imp->_appType = eAppTypeInterpreter;
    } else if ( isBackground() ) {
        if ( !cl.getScriptFilename().isEmpty() ) {
            if ( !cl.getIPCPipeName().isEmpty() ) {
                _imp->_appType = eAppTypeBackgroundAutoRunLaunchedFromGui;
            } else {
                _imp->_appType = eAppTypeBackgroundAutoRun;
            }
        } else {
            _imp->_appType = eAppTypeBackground;
        }
    } else {
        _imp->_appType = eAppTypeGui;
    }

    //Now that the locale is set, re-parse the command line arguments because the filenames might have non UTF-8 encodings
    CLArgs args;
    if ( !cl.getScriptFilename().isEmpty() ) {
        const QStringList& appArgs = qApp->arguments();
        args = CLArgs( appArgs, cl.isBackgroundMode() );
    } else {
        args = cl;
    }

    AppInstancePtr mainInstance = newAppInstance(args, false);

    hideSplashScreen();

    if (!mainInstance) {
        qApp->quit();

        return false;
    } else {
        onLoadCompleted();

        ///In background project auto-run the rendering is finished at this point, just exit the instance
        if ( ( (_imp->_appType == eAppTypeBackgroundAutoRun) ||
               ( _imp->_appType == eAppTypeBackgroundAutoRunLaunchedFromGui) ||
               ( _imp->_appType == eAppTypeInterpreter) ) && mainInstance ) {
            bool wasKilled = true;
            const AppInstanceVec& instances = appPTR->getAppInstances();
            for (AppInstanceVec::const_iterator it = instances.begin(); it != instances.end(); ++it) {
                if ( (*it == mainInstance) ) {
                    wasKilled = false;
                }
            }
            if (!wasKilled) {
                try {
                    mainInstance->getProject()->reset(true/*aboutToQuit*/, true /*blocking*/);
                } catch (std::logic_error&) {
                    // ignore
                }

                try {
                    mainInstance->quitNow();
                } catch (std::logic_error&) {
                    // ignore
                }
            }
        }

        return true;
    }
} // AppManager::loadInternalAfterInitGui

void
AppManager::onViewerTileCacheSizeChanged()
{
    if (_imp->_viewerCache) {
        _imp->_viewerCache->clear();
        _imp->setViewerCacheTileSize();
    }
}

void
AppManagerPrivate::setViewerCacheTileSize()
{
    if (!_viewerCache) {
        return;
    }
    std::size_t tileSize =  (std::size_t)ipow( 2, _settings->getViewerTilesPowerOf2() );

    // Viewer tiles are always RGBA
    tileSize = tileSize * tileSize * 4;


    ImageBitDepthEnum viewerDepth = _settings->getViewersBitDepth();
    switch (viewerDepth) {
        case eImageBitDepthFloat:
        case eImageBitDepthHalf:
            tileSize *= sizeof(float);
            break;
        default:
            break;
    }
    _viewerCache->setTiled(true, tileSize);
}
#include "AppManagerPart1.inc"
#include "AppManagerPart2.inc"
#include "AppManagerPart3.inc"

NATRON_NAMESPACE_USING
#include "moc_AppManager.cpp"
