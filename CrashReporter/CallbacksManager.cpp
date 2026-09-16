/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <https://natrongithub.github.io/>,
 * Copyright (C) 2018-2023 The Natron developers
 * Copyright (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
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

#include "CallbacksManager.h"

#include "Global/Macros.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstring>
#include <exception>
#include <stdexcept>

#include <QtCore/QDebug>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHttpMultiPart>

#ifndef REPORTER_CLI_ONLY
#include <QApplication>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#endif

#include <QCoreApplication>
#include <QLocalSocket>
#include <QLocalServer>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QFile>
#include <QProcess>
#include <QStringList>
#include <QString>
#include <QVarLengthArray>
#include <QSettings>

#ifdef DEBUG
#include <QTextStream>
#endif


#if defined(Q_OS_DARWIN)
#include "client/mac/crash_generation/crash_generation_server.h"
#include <execinfo.h>
#include <cstdio>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <spawn.h>

#elif defined(Q_OS_LINUX)
#include "client/linux/crash_generation/crash_generation_server.h"
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <ucontext.h>
#include <execinfo.h>
#include <cstdio>
#include <errno.h>
#include <sys/wait.h>

#elif defined(Q_OS_WIN32)
#include <windows.h>
#include "client/windows/crash_generation/crash_generation_server.h"
#endif

#ifdef NATRON_CRASH_REPORTER_USE_FORK
#include <sys/types.h>
#include <unistd.h> // getpid

extern char** environ;
#endif

#ifndef REPORTER_CLI_ONLY
#include "CrashDialog.h"
#endif

#include "Global/ProcInfo.h"
#include "Global/GitVersion.h"
#include "Global/StrUtils.h"

#define UPLOAD_URL "http://breakpad.natron.fr/submit"
#define FALLBACK_FORM_URL "http://breakpad.natron.fr/form/"

CallbacksManager* CallbacksManager::_instance = 0;

#define EXIT_APP(code, exitIfDumpReceived) ( CallbacksManager::instance()->s_emitDoExitCallBackOnMainThread(code, exitIfDumpReceived) )

namespace {
#ifdef NATRON_CRASH_REPORTER_USE_FORK


// strndup doesn't exist on OS X prior to 10.7
static char *
strndup_replacement(const char *str,
                    size_t n)
{
    size_t len;
    char *copy;

    for (len = 0; len < n && str[len]; ++len) {
        continue;
    }

    if ( ( copy = (char *)malloc(len + 1) ) == NULL ) {
        return (NULL);
    }
    memcpy(copy, str, len);
    copy[len] = '\0';

    return (copy);
}

static char*
qstringToMallocCharArray(const QString& str)
{
    std::string stdStr = str.toStdString();

    return strndup_replacement(stdStr.c_str(), stdStr.size() + 1);
}

static void
handleChildDeadSignal( int /*signalId*/ )
{
#ifdef Q_OS_LINUX
    int status;
    pid_t pid = waitpid(WAIT_ANY, &status, WUNTRACED | WNOHANG);
    if ( ( pid <= 0) || ( errno == ECHILD) ) {
        return;
    }
#endif
    if (qApp) {
        EXIT_APP(0, false);
    } else {
        std::exit(1);
    }
}

static void
setChildDeadSignal()
{
    struct sigaction sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handleChildDeadSignal;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        std::perror("setting up termination signal");
        std::exit(1);
    }
}
#endif
} // compacted restore - FULL FILE REQUIRED
