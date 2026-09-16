/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: blocking headless render cannot hang at CPU-zero (#248).
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

#include "BlockingBackgroundRender.h"

#include <cassert>
#include <stdexcept>

#include "Global/Macros.h"
// clang-format off
CLANG_DIAG_OFF(deprecated-register) //'register' storage class specifier is deprecated
#include <QDebug>
CLANG_DIAG_ON(deprecated-register)
// clang-format on

#include "Engine/AppManager.h"
#include "Engine/EffectInstance.h"
#include "Engine/OutputEffectInstance.h"
#include "Engine/OutputSchedulerThread.h"
#include "Engine/Settings.h"

#include <QElapsedTimer>

NATRON_NAMESPACE_ENTER


BlockingBackgroundRender::BlockingBackgroundRender(OutputEffectInstance* writer)
    : _running(false)
    , _writer(writer)
{
}

void
BlockingBackgroundRender::blockingRender(bool enableRenderStats,
                                         int first,
                                         int last,
                                         int frameStep)
{
    // avoid race condition: the code must work even if renderFullSequence() calls notifyFinished()
    // immediately.
    QMutexLocker locker(&_runningMutex);

    assert(_running == false);
    _running = true;
    _writer->renderFullSequence(true, enableRenderStats, this, first, last, frameStep);
    if (appPTR->getCurrentSettings()->getNumberOfThreads() == -1) {
        _running = false;
    } else {
        QElapsedTimer stallTimer;
        stallTimer.start();
        const qint64 kMaxWaitMs = 30000;
        while (_running) {
            if (!_runningCond.wait(locker.mutex(), 250)) {
                if (stallTimer.hasExpired(kMaxWaitMs)) {
                    if (_writer && _writer->getRenderEngine()) {
                        _writer->getRenderEngine()->abortRenderingNoRestart();
                    }
                    _running = false;
                    break;
                }
            }
        }
    }
}

void
BlockingBackgroundRender::notifyFinished()
{
    QMutexLocker locker(&_runningMutex);

    if (!_running) {
        return;
    }
    _running = false;
    _runningCond.wakeOne();
}

NATRON_NAMESPACE_EXIT
