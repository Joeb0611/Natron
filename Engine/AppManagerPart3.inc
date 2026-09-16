
StandardButtonEnum
Dialogs::questionDialog(const std::string & title,
                        const std::string & message,
                        bool useHtml,
                        StandardButtons buttons,
                        StandardButtonEnum defaultButton,
                        bool* stopAsking)
{
    appPTR->hideSplashScreen();
    AppInstancePtr topLvlInstance = appPTR->getTopLevelInstance();
    if ( topLvlInstance && !appPTR->isBackground() ) {
        return topLvlInstance->questionDialog(title, message, useHtml, buttons, defaultButton, stopAsking);
    } else {
        std::cout << "QUESTION ASKED: " << title << ":" << message << std::endl;
        std::cout << NATRON_APPLICATION_NAME " answered yes." << std::endl;

        return eStandardButtonYes;
    }
}

#if 0 // dead code
std::size_t
NATRON_PYTHON_NAMESPACE::findNewLineStartAfterImports(std::string& script)
{
    ///Find position of the last import
    size_t foundImport = script.find("import ");

    if (foundImport != std::string::npos) {
        for (;; ) {
            size_t found = script.find("import ", foundImport + 1);
            if (found == std::string::npos) {
                break;
            } else {
                foundImport = found;
            }
        }
    }

    if (foundImport == std::string::npos) {
        return 0;
    }

    ///find the next end line aftr the import
    size_t endLine = script.find('\n', foundImport + 1);


    if (endLine == std::string::npos) {
        //no end-line, add one
        script.append("\n");

        return script.size();
    } else {
        return endLine + 1;
    }
}

#endif

PyObject*
NATRON_PYTHON_NAMESPACE::getMainModule()
{
    return appPTR->getMainModule();
}

#if 0 // dead code
std::size_t
NATRON_PYTHON_NAMESPACE::ensureScriptHasModuleImport(const std::string& moduleName,
                                                     std::string& script)
{
    /// import module
    script = "from " + moduleName + " import * \n" + script;

    return NATRON_PYTHON_NAMESPACE::findNewLineStartAfterImports(script);
}

#endif

bool
NATRON_PYTHON_NAMESPACE::interpretPythonScript(const std::string& script,
                                               std::string* error,
                                               std::string* output)
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return true;
#endif
    PythonGILLocker pgl;
    PyObject* mainModule = NATRON_PYTHON_NAMESPACE::getMainModule();
    int status = -1;
#if 0 //USE_PYRUN_SIMPLESTRING
    status = PyRun_SimpleString(script.c_str());
#else
    PyObject* dict = PyModule_GetDict(mainModule);

    PyErr_Clear();

    ///This is faster than PyRun_SimpleString since is doesn't call PyImport_AddModule("__main__")
    PyObject* v = PyRun_String(script.c_str(), Py_file_input, dict, 0);
    if (v) {
        Py_DECREF(v);
        status = 0;
    }
#endif

    if (error) {
        error->clear();
    }
    PyObject* ex = PyErr_Occurred();
    if (ex) {
        assert(status < 0);
        if (!error) {
            PyErr_Clear();
        } else {
            PyObject *pyExcType;
            PyObject *pyExcValue;
            PyObject *pyExcTraceback;
            PyErr_Fetch(&pyExcType, &pyExcValue, &pyExcTraceback); // also clears the error indicator
            //PyErr_NormalizeException(&pyExcType, &pyExcValue, &pyExcTraceback);

            PyObject* pyStr = PyObject_Str(pyExcValue);
            if (pyStr) {
                std::string str = NATRON_PYTHON_NAMESPACE::PyStringToStdString(pyStr);
                if ( error && !str.empty() ) {
                    *error += std::string("Python exception: ") + str + '\n';
                }
                Py_DECREF(pyStr);

                // See if we can get a full traceback
                PyObject* module_name = PyUnicode_FromString("traceback");
                PyObject* pyth_module = PyImport_Import(module_name);
                Py_DECREF(module_name);

                if (pyth_module != NULL) {
                    PyObject* pyth_func;
                    if (!pyExcTraceback) {
                        pyth_func = PyObject_GetAttrString(pyth_module, "format_exception_only");
                    } else {
                        pyth_func = PyObject_GetAttrString(pyth_module, "format_exception");
                    }
                    Py_DECREF(pyth_module);
                    if (pyth_func && PyCallable_Check(pyth_func)) {
                        PyObject *pyth_val = PyObject_CallFunctionObjArgs(pyth_func, pyExcType, pyExcValue, pyExcTraceback, NULL);
                        if (pyth_val) {
                            PyObject *emptyString = PyUnicode_FromString("");
                            PyObject *strList = PyObject_CallMethod(emptyString, (char*)"join", (char*)"(O)", pyth_val);
                            Py_DECREF(emptyString);
                            Py_DECREF(pyth_val);
                            pyStr = PyObject_Str(strList);
                            Py_DECREF(strList);
                            if (pyStr) {
                                str = NATRON_PYTHON_NAMESPACE::PyStringToStdString(pyStr);
                                if ( error && !str.empty() ) {
                                    *error += str + '\n';
                                }
                                Py_DECREF(pyStr);
                            }
                        }
                    }
                }
            }
        }
    }
    if ( !appPTR->isBackground() ) {
        ///Gui session, do stdout, stderr redirection
        PyObject *errCatcher = 0;
        PyObject *outCatcher = 0;

        if ( error && PyObject_HasAttrString(mainModule, "catchErr") ) {
            errCatcher = PyObject_GetAttrString(mainModule, "catchErr"); //get our catchOutErr created above, new ref
        }

        if ( output && PyObject_HasAttrString(mainModule, "catchOut") ) {
            outCatcher = PyObject_GetAttrString(mainModule, "catchOut"); //get our catchOutErr created above, new ref
        }

        PyErr_Print(); //make python print any errors

        PyObject *errorObj = 0;
        if (errCatcher && error) {
            errorObj = PyObject_GetAttrString(errCatcher, "value"); //get the  stderr from our catchErr object, new ref
            assert(errorObj);
            *error += PyStringToStdString(errorObj);
            PyObject* unicode = PyUnicode_FromString("");
            PyObject_SetAttrString(errCatcher, "value", unicode);
            Py_DECREF(errorObj);
            Py_DECREF(errCatcher);
        }
        PyObject *outObj = 0;
        if (outCatcher && output) {
            outObj = PyObject_GetAttrString(outCatcher, "value"); //get the stdout from our catchOut object, new ref
            assert(outObj);
            *output = PyStringToStdString(outObj);
            PyObject* unicode = PyUnicode_FromString("");
            PyObject_SetAttrString(outCatcher, "value", unicode);
            Py_DECREF(outObj);
            Py_DECREF(outCatcher);
        }

        if ( error && !error->empty() ) {
            *error = "While executing script:\n" + script + "Python error:\n" + *error;

            return false;
        }

        return status == 0;
    } else {
        if (ex) {
            PyErr_Print();

            return false;
        } else {
            return status == 0;
        }
    }
} // NATRON_PYTHON_NAMESPACE::interpretPythonScript

#if 0 // dead code
void
NATRON_PYTHON_NAMESPACE::compilePyScript(const std::string& script,
                                         PyObject** code)
{
    ///Must be locked
    assert( PyThreadState_Get() );

    *code = (PyObject*)Py_CompileString(script.c_str(), "<string>", Py_file_input);
    if (PyErr_Occurred() || !*code) {
#ifdef DEBUG
        PyErr_Print();
#endif
        throw std::runtime_error("failed to compile the script");
    }
}

#endif

static std::string
makeNameScriptFriendlyInternal(const std::string& str,
                               bool allowDots)
{
    if (str == "from") {
        return "pFrom";
    } else if (str == "lambda") {
        return "pLambda";
    }
    ///Remove any non alpha-numeric characters from the baseName
    std::locale loc;
    std::string cpy;
    for (std::size_t i = 0; i < str.size(); ++i) {
        ///Ignore starting digits
        if ( cpy.empty() && std::isdigit(str[i], loc) ) {
            cpy.push_back('p');
            cpy.push_back(str[i]);
            continue;
        }

        ///Spaces becomes underscores
        if ( std::isspace(str[i], loc) ) {
            cpy.push_back('_');
        }
        ///Non alpha-numeric characters are not allowed in python
        else if ( (str[i] == '_') || std::isalnum(str[i], loc) || ( allowDots && (str[i] == '.') ) ) {
            cpy.push_back(str[i]);
        }
    }

    return cpy;
}

std::string
NATRON_PYTHON_NAMESPACE::makeNameScriptFriendlyWithDots(const std::string& str)
{
    return makeNameScriptFriendlyInternal(str, true);
}

std::string
NATRON_PYTHON_NAMESPACE::makeNameScriptFriendly(const std::string& str)
{
    return makeNameScriptFriendlyInternal(str, false);
}

bool
NATRON_PYTHON_NAMESPACE::isKeyword(const std::string& str)
{
    const char* const kw[] = { "False", "None", "True", "and", "as", "assert",
        "async", "await", "break", "class", "continue", "def", "del", "elif",
        "else", "except", "finally", "for", "from", "global", "if", "import",
        "in", "is", "lambda", "nonlocal", "not", "or", "pass", "raise", "return",
        "try", "while", "with", "yield", NULL };
    for (const char* const *k = kw; *k != NULL; ++k) {
        if (str == *k) {
            return true;
        }
    }
    return false;
}

#if !defined(NDEBUG) && !defined(DEBUG_PYTHON_GIL)
//#pragma message WARN("define DEBUG_PYTHON_GIL in AppManager.h to debug Python GIL issues")
#endif

#ifdef DEBUG_PYTHON_GIL
QMap<QString,int> PythonGILLocker::pythonCount;
#ifdef USE_NATRON_GIL
QMap<QString,int> PythonGILLocker::natronCount;
#endif
#endif

// Follow https://web.archive.org/web/20150918224620/http://wiki.blender.org/index.php/Dev:2.4/Source/Python/API/Threads
PythonGILLocker::PythonGILLocker()
    : state(PyGILState_UNLOCKED)
{
#ifdef DEBUG_PYTHON_GIL
    if (!Py_IsInitialized()) {
        throw std::runtime_error("Trying to execute python code, but Py_IsInitialized() returns false");
    }
#endif
    assert(Py_IsInitialized());

    // Also take the Python GIL, since not doing so seems to crash Natron during recursive Natron->Python->Natron->Python calls, see https://github.com/NatronGitHub/Natron/issues/379
    // Follow https://web.archive.org/web/20150918224620/http://wiki.blender.org/index.php/Dev:2.4/Source/Python/API/Threads
    // Take the GIL for this thread
#ifdef DEBUG_PYTHON_GIL
    QThread* curThread = QThread::currentThread();
    QString threadname = (qApp && qApp->thread() == curThread) ? QString::fromUtf8("Main") : curThread->objectName();
    qDebug() << QString::fromUtf8("Thread '%1' is asking the Python GIL").arg(threadname);
#endif
    state = PyGILState_Ensure();
#ifdef DEBUG_PYTHON_GIL
    ++pythonCount[threadname];
    qDebug() << QString::fromUtf8("Thread '%1' got the Python GIL (%2)").arg(threadname).arg(pythonCount[threadname]);
#endif

#ifdef USE_NATRON_GIL
    // Take the Natron GIL https://github.com/NatronGitHub/Natron/commit/46d9d616dfebfbb931a79776734e2fa17202f7cb
    // We do this after we got the Python GIL, to avoid deadlocks:
    // If we do it the other way, this thread may be waiting for the Python GIL while keeping the Natron GIL
    // locked, thus preventing other threads from getting the Python GIL.
    if (appPTR) {
#ifdef DEBUG_PYTHON_GIL
        qDebug() << QString::fromUtf8("Thread '%1' is asking the Natron GIL").arg(threadname);
#endif
        appPTR->takeNatronGIL();
#ifdef DEBUG_PYTHON_GIL
        ++natronCount[threadname];
        qDebug() << QString::fromUtf8("Thread '%1' got the Natron GIL (%2)").arg(threadname).arg(natronCount[threadname]);
#endif
    }
#endif

    assert(PyThreadState_Get());
#if PY_VERSION_HEX >= 0x030400F0
    assert(PyGILState_Check()); // Not available prior to Python 3.4
#endif
}

PythonGILLocker::~PythonGILLocker()
{
    // Release the Natron GIL https://github.com/NatronGitHub/Natron/commit/46d9d616dfebfbb931a79776734e2fa17202f7cb
#ifdef DEBUG_PYTHON_GIL
    QThread* curThread = QThread::currentThread();
    QString threadname = (qApp && qApp->thread() == curThread) ? QString::fromUtf8("Main") : curThread->objectName();
#endif
#ifdef USE_NATRON_GIL
    if (appPTR) {
#ifdef DEBUG_PYTHON_GIL
        qDebug() << QString::fromUtf8("Thread '%1' is releasing the Natron GIL (%2)").arg(threadname).arg(natronCount[threadname]);
        --natronCount[threadname];
#endif
        appPTR->releaseNatronGIL();
    }
#endif

    // We took the Python GIL too, so release it here.
    // Follow https://web.archive.org/web/20150918224620/http://wiki.blender.org/index.php/Dev:2.4/Source/Python/API/Threads
#if PY_VERSION_HEX >= 0x030400F0
    assert(PyGILState_Check());  // Not available prior to Python 3.4
#endif
    // Release the GIL, no thread will own it afterwards.
#ifdef DEBUG_PYTHON_GIL
    qDebug() << QString::fromUtf8("Thread '%1' is releasing the Python GIL (%2)").arg(threadname).arg(pythonCount[threadname]);
    --pythonCount[threadname];
#endif
    PyGILState_Release(state);
}

PythonGILUnlocker::PythonGILUnlocker()
{
#ifdef DEBUG_PYTHON_GIL
    if (!Py_IsInitialized()) {
        throw std::runtime_error("Trying to execute python code, but Py_IsInitialized() returns false");
    }
#endif
    assert(Py_IsInitialized());

    // Also take the Python GIL, since not doing so seems to crash Natron during recursive Natron->Python->Natron->Python calls, see https://github.com/NatronGitHub/Natron/issues/379
    // Follow https://web.archive.org/web/20150918224620/http://wiki.blender.org/index.php/Dev:2.4/Source/Python/API/Threads
    // Take the GIL for this thread
#ifdef DEBUG_PYTHON_GIL
    QThread* curThread = QThread::currentThread();
    QString threadname = (qApp && qApp->thread() == curThread) ? QString::fromUtf8("Main") : curThread->objectName();
    qDebug() << QString::fromUtf8("Thread '%1' is releasing the Python GIL").arg(threadname);
#endif
    assert(PyThreadState_Get());
#if PY_VERSION_HEX >= 0x030400F0
    assert(PyGILState_Check()); // Not available prior to Python 3.4
#endif
    PyGILState_Release(PyGILState_UNLOCKED);
#ifdef DEBUG_PYTHON_GIL
    qDebug() << QString::fromUtf8("Thread '%1' released the Python GIL").arg(threadname);
#endif

#ifdef USE_NATRON_GIL
    // Take the Natron GIL https://github.com/NatronGitHub/Natron/commit/46d9d616dfebfbb931a79776734e2fa17202f7cb
    // We do this after we got the Python GIL, to avoid deadlocks:
    // If we do it the other way, this thread may be waiting for the Python GIL while keeping the Natron GIL
    // locked, thus preventing other threads from getting the Python GIL.
    if (appPTR) {
#ifdef DEBUG_PYTHON_GIL
        qDebug() << QString::fromUtf8("Thread '%1' is releasing the Natron GIL").arg(threadname);
#endif
        appPTR->releaseNatronGIL();
#ifdef DEBUG_PYTHON_GIL
        qDebug() << QString::fromUtf8("Thread '%1' released the Natron GIL").arg(threadname);
#endif
    }
#endif
}

PythonGILUnlocker::~PythonGILUnlocker()
{
    // Take the Natron GIL https://github.com/NatronGitHub/Natron/commit/46d9d616dfebfbb931a79776734e2fa17202f7cb
#ifdef DEBUG_PYTHON_GIL
    QThread* curThread = QThread::currentThread();
    QString threadname = (qApp && qApp->thread() == curThread) ? QString::fromUtf8("Main") : curThread->objectName();
#endif
#ifdef USE_NATRON_GIL
    if (appPTR) {
#ifdef DEBUG_PYTHON_GIL
        qDebug() << QString::fromUtf8("Thread '%1' is asking the Natron GIL").arg(threadname);
#endif
        appPTR->takeNatronGIL();
    }
#endif

    // We released the Python GIL too, so take it here.
    // Follow https://web.archive.org/web/20150918224620/http://wiki.blender.org/index.php/Dev:2.4/Source/Python/API/Threads
#if PY_VERSION_HEX >= 0x030400F0
    assert(!PyGILState_Check());  // Not available prior to Python 3.4
#endif
    // Release the GIL, no thread will own it afterwards.
#ifdef DEBUG_PYTHON_GIL
    qDebug() << QString::fromUtf8("Thread '%1' is asking the Python GIL").arg(threadname);
#endif
    /* PyGILState_STATE state = */
    PyGILState_Ensure();
#ifdef DEBUG_PYTHON_GIL
    qDebug() << QString::fromUtf8("Thread '%1' got the Python GIL").arg(threadname);
#endif

    assert(PyThreadState_Get());
#if PY_VERSION_HEX >= 0x030400F0
    assert(PyGILState_Check()); // Not available prior to Python 3.4
#endif
}

static bool
getGroupInfosInternal(const std::string& modulePath,
                      const std::string& pythonModule,
                      std::string* pluginID,
                      std::string* pluginLabel,
                      std::string* iconFilePath,
                      std::string* grouping,
                      std::string* description,
                      bool* isToolset,
                      unsigned int* version)
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return false;
#endif
    PythonGILLocker pgl;
    static const QString script = QString::fromUtf8("import sys\n"
                                                    "import %1\n"
                                                    "ret = True\n"
                                                    "if not hasattr(%1,\"createInstance\") or not hasattr(%1.createInstance,\"__call__\"):\n"
                                                    "    ret = False\n"
                                                    "if not hasattr(%1,\"getLabel\") or not hasattr(%1.getLabel,\"__call__\"):\n"
                                                    "    ret = False\n"
                                                    "templateLabel=\"\"\n"
                                                    "if ret == True:\n"
                                                    "    templateLabel = %1.getLabel()\n"
                                                    "pluginID = templateLabel\n"
                                                    "version = 1\n"
                                                    "isToolset = False\n"
                                                    "if hasattr(%1,\"getVersion\") and hasattr(%1.getVersion,\"__call__\"):\n"
                                                    "    version = %1.getVersion()\n"
                                                    "if hasattr(%1,\"getIsToolset\") and hasattr(%1.getIsToolset,\"__call__\"):\n"
                                                    "    isToolset = %1.getIsToolset()\n"
                                                    "description=\"\"\n"
                                                    "if hasattr(%1,\"getPluginDescription\") and hasattr(%1.getPluginDescription,\"__call__\"):\n"
                                                    "    description = %1.getPluginDescription()\n"
                                                    "elif hasattr(%1,\"getDescription\") and hasattr(%1.getDescription,\"__call__\"):\n" // Check old function name for compat
                                                    "    description = %1.getDescription()\n"
                                                    "if hasattr(%1,\"getPluginID\") and hasattr(%1.getPluginID,\"__call__\"):\n"
                                                    "    pluginID = %1.getPluginID()\n"
                                                    "if ret == True and hasattr(%1,\"getIconPath\") and hasattr(%1.getIconPath,\"__call__\"):\n"
                                                    "    global templateIcon\n"
                                                    "    templateIcon = %1.getIconPath()\n"
                                                    "if ret == True and hasattr(%1,\"getGrouping\") and hasattr(%1.getGrouping,\"__call__\"):\n"
                                                    "    global templateGrouping\n"
                                                    "    templateGrouping =  %1.getGrouping()\n");
    std::string toRun = script.arg( QString::fromUtf8( pythonModule.c_str() ) ).toStdString();
    std::string err;
    if ( !NATRON_PYTHON_NAMESPACE::interpretPythonScript(toRun, &err, 0) ) {
        QString logStr = QCoreApplication::translate("AppManager", "Was not recognized as a PyPlug: %1").arg( QString::fromUtf8( err.c_str() ) );
        appPTR->writeToErrorLog_mt_safe(QString::fromUtf8(pythonModule.c_str()), QDateTime::currentDateTime(), logStr);

        return false;
    }

    PyObject* mainModule = NATRON_PYTHON_NAMESPACE::getMainModule();
    PyObject* retObj = PyObject_GetAttrString(mainModule, "ret"); //new ref
    assert(retObj);
    if (PyObject_IsTrue(retObj) == 0) {
        Py_XDECREF(retObj);

        return false;
    }
    Py_XDECREF(retObj);

    std::string deleteScript("del ret\n"
                             "del templateLabel\n");
    PyObject* labelObj = 0;
    labelObj = PyObject_GetAttrString(mainModule, "templateLabel"); //new ref

    PyObject* idObj = 0;
    idObj = PyObject_GetAttrString(mainModule, "pluginID"); //new ref

    PyObject* iconObj = 0;
    if ( PyObject_HasAttrString(mainModule, "templateIcon") ) {
        iconObj = PyObject_GetAttrString(mainModule, "templateIcon"); //new ref
    }
    PyObject* iconGrouping = 0;
    if ( PyObject_HasAttrString(mainModule, "templateGrouping") ) {
        iconGrouping = PyObject_GetAttrString(mainModule, "templateGrouping"); //new ref
    }

    PyObject* versionObj = 0;
    if ( PyObject_HasAttrString(mainModule, "version") ) {
        versionObj = PyObject_GetAttrString(mainModule, "version"); //new ref
    }

    PyObject* isToolsetObj = 0;
    if ( PyObject_HasAttrString(mainModule, "isToolset") ) {
        isToolsetObj = PyObject_GetAttrString(mainModule, "isToolset"); //new ref
    }

    PyObject* pluginDescriptionObj = 0;
    if ( PyObject_HasAttrString(mainModule, "description") ) {
        pluginDescriptionObj = PyObject_GetAttrString(mainModule, "description"); //new ref
    }

    assert(labelObj);


    *pluginLabel = NATRON_PYTHON_NAMESPACE::PyStringToStdString(labelObj);
    Py_XDECREF(labelObj);

    if (idObj) {
        *pluginID = NATRON_PYTHON_NAMESPACE::PyStringToStdString(idObj);
        deleteScript.append("del pluginID\n");
        Py_XDECREF(idObj);
    }

    if (iconObj) {
        *iconFilePath = NATRON_PYTHON_NAMESPACE::PyStringToStdString(iconObj);
        QFileInfo iconInfo( QString::fromUtf8( modulePath.c_str() ) + QString::fromUtf8( iconFilePath->c_str() ) );
        *iconFilePath =  iconInfo.canonicalFilePath().toStdString();

        deleteScript.append("del templateIcon\n");
        Py_XDECREF(iconObj);
    }
    if (iconGrouping) {
        *grouping = NATRON_PYTHON_NAMESPACE::PyStringToStdString(iconGrouping);
        deleteScript.append("del templateGrouping\n");
        Py_XDECREF(iconGrouping);
    }

    if (versionObj) {
        *version = (unsigned int)PyLong_AsLong(versionObj);
        deleteScript.append("del version\n");
        Py_XDECREF(versionObj);
    }

    if ( isToolsetObj && PyBool_Check(isToolsetObj) ) {
        *isToolset = (isToolsetObj == Py_True) ? true : false;
        deleteScript.append("del isToolset\n");
        Py_XDECREF(isToolsetObj);
    }


    if (pluginDescriptionObj) {
        *description = NATRON_PYTHON_NAMESPACE::PyStringToStdString(pluginDescriptionObj);
        deleteScript.append("del description\n");
        Py_XDECREF(pluginDescriptionObj);
    }

    if ( grouping->empty() ) {
        *grouping = PLUGIN_GROUP_OTHER;
    }


    bool ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript(deleteScript, &err, NULL);
    assert(ok);
    if (!ok) {
        throw std::runtime_error("getGroupInfos(): interpretPythonScript(" + deleteScript + " failed!");
    }

    return true;
} // getGroupInfosInternal

static
bool
getGroupInfosFromQtResourceFile(const std::string& resourceFileName,
                                const std::string& modulePath,
                                const std::string& pythonModule,
                                std::string* pluginID,
                                std::string* pluginLabel,
                                std::string* iconFilePath,
                                std::string* grouping,
                                std::string* description,
                                bool* isToolset,
                                unsigned int* version)
{
    ///Must be locked
    assert( PyThreadState_Get() );

    QString qModulePath = QString::fromUtf8( resourceFileName.c_str() );

    assert( qModulePath.startsWith( QString::fromUtf8(":/Resources") ) );

    QFile moduleContent(qModulePath);
    if ( !moduleContent.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        return false;
    }

    QByteArray utf8bytes = QString::fromUtf8( pythonModule.c_str() ).toUtf8();
    char *moduleName = utf8bytes.data();
    PyObject* moduleCode = Py_CompileString(moduleContent.readAll().constData(), moduleName, Py_file_input);
    if (!moduleCode) {
        return false;
    }
    PyObject* module = PyImport_ExecCodeModule(moduleName, moduleCode);
    if (!module) {
        return false;
    }

    //Now that the module is loaded, use the regular version
    return getGroupInfosInternal(modulePath, pythonModule, pluginID, pluginLabel, iconFilePath, grouping, description, isToolset, version);
    //PyDict_SetItemString(priv->globals, moduleName, module);
}

bool
NATRON_PYTHON_NAMESPACE::getGroupInfos(const std::string& modulePath,
                                       const std::string& pythonModule,
                                       std::string* pluginID,
                                       std::string* pluginLabel,
                                       std::string* iconFilePath,
                                       std::string* grouping,
                                       std::string* description,
                                       bool* isToolset,
                                       unsigned int* version)
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return false;
#endif

    {
        std::string tofind(":/Resources");
        if (modulePath.substr( 0, tofind.size() ) == tofind) {
            std::string resourceFileName = modulePath + pythonModule + ".py";

            return getGroupInfosFromQtResourceFile(resourceFileName, modulePath, pythonModule, pluginID, pluginLabel, iconFilePath, grouping, description, isToolset, version);
        }
    }

    return getGroupInfosInternal(modulePath, pythonModule, pluginID, pluginLabel, iconFilePath, grouping, description, isToolset, version);
}

void
NATRON_PYTHON_NAMESPACE::getFunctionArguments(const std::string& pyFunc,
                                              std::string* error,
                                              std::vector<std::string>* args)
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return;
#endif
    std::stringstream ss;
    ss << "import inspect\n";
    ss << "args_spec = inspect.getfullargspec(" << pyFunc << ")\n";
    std::string script = ss.str();
    std::string output;
    bool ok = NATRON_PYTHON_NAMESPACE::interpretPythonScript(script, error, &output);
    if (!ok) {
        throw std::runtime_error("NATRON_PYTHON_NAMESPACE::getFunctionArguments(): interpretPythonScript(" + script + " failed!");
    }
    PyObject* mainModule = NATRON_PYTHON_NAMESPACE::getMainModule();
    PyObject* args_specObj = 0;
    PythonGILLocker pgl;

    if ( PyObject_HasAttrString(mainModule, "args_spec") ) {
        args_specObj = PyObject_GetAttrString(mainModule, "args_spec");
    }
    assert(args_specObj);
    PyObject* argListObj = 0;

    if (args_specObj) {
        argListObj = PyTuple_GetItem(args_specObj, 0);
        assert(argListObj);
        if (argListObj) {
            // size = PyObject_Size(argListObj)
            assert( PyList_Check(argListObj) );
            Py_ssize_t size = PyList_Size(argListObj);
            for (Py_ssize_t i = 0; i < size; ++i) {
                PyObject* itemObj = PyList_GetItem(argListObj, i);
                assert(itemObj);
                if (itemObj) {
                    std::string itemName = PyStringToStdString(itemObj);
                    assert( !itemName.empty() );
                    if ( !itemName.empty() ) {
                        args->push_back(itemName);
                    }
                }
            }
            if ( (PyTuple_GetItem(args_specObj, 1) != Py_None) || (PyTuple_GetItem(args_specObj, 2) != Py_None) ) {
                error->append("Function contains variadic arguments which is unsupported.");

                return;
            }
        }
    }
}

/**
 * @brief Given a fullyQualifiedName, e.g: app1.Group1.Blur1
 * this function returns the PyObject attribute of Blur1 if it is defined, or Group1 otherwise
 * If app1 or Group1 does not exist at this point, this is a failure.
 **/
PyObject*
NATRON_PYTHON_NAMESPACE::getAttrRecursive(const std::string& fullyQualifiedName,
                                          PyObject* parentObj,
                                          bool* isDefined)
{
#ifdef NATRON_RUN_WITHOUT_PYTHON

    return 0;
#endif
    std::size_t foundDot = fullyQualifiedName.find(".");
    std::string attrName = foundDot == std::string::npos ? fullyQualifiedName : fullyQualifiedName.substr(0, foundDot);
    PyObject* obj = 0;
    {
        PythonGILLocker pgl;
        if ( PyObject_HasAttrString( parentObj, attrName.c_str() ) ) {
            obj = PyObject_GetAttrString( parentObj, attrName.c_str() );
        }
    }
    ///We either found the parent object or we are on the last object in which case we return the parent
    if (!obj) {
        //assert(fullyQualifiedName.find(".") == std::string::npos);
        *isDefined = false;

        return parentObj;
    } else {
        std::string recurseName;
        if (foundDot != std::string::npos) {
            recurseName = fullyQualifiedName;
            recurseName.erase(0, foundDot + 1);
        }
        if ( !recurseName.empty() ) {
            return NATRON_PYTHON_NAMESPACE::getAttrRecursive(recurseName, obj, isDefined);
        } else {
            *isDefined = true;

            return obj;
        }
    }
}

NATRON_NAMESPACE_EXIT

NATRON_NAMESPACE_USING
#include "moc_AppManager.cpp"
