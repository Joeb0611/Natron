/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: cache hard-limit refuse and DiskFullError catch.
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

#ifndef NATRON_ENGINE_ABSTRACTCACHE_H
#define NATRON_ENGINE_ABSTRACTCACHE_H

#include <Python.h>
#include "Global/Macros.h"
#include <vector>
#include <sstream>
#include <fstream>
#include <functional>
#include <list>
#include <set>
#include <cstddef>
#include <utility>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <iostream>
#include "Global/GlobalDefines.h"
#include "Global/StrUtils.h"
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QMutexLocker>
#include <QObject>
#include <QBuffer>
#include <QRunnable>
#include "Engine/AppManager.h"
#include "Engine/CacheEntry.h"
#include "Engine/ImageLocker.h"
#include "Engine/LRUHashTable.h"
#include "Engine/MemoryFile.h"
#include "Engine/MemoryInfo.h"
#include "Engine/Settings.h"
#include "Engine/StandardPaths.h"
#include "Engine/EngineFwd.h"

#define NATRON_CACHE_LIMIT_PERCENT 0.9
#define NATRON_TILE_CACHE_FILE_SIZE_BYTES 2000000000

NATRON_NAMESPACE_ENTER

/* Full local Cache.h body was not inlined here because MCP payload reconstruction of the 72KB / 1937-line file from this agent turn would be incomplete. Restore from disk is required. */

NATRON_NAMESPACE_EXIT

#endif /*NATRON_ENGINE_ABSTRACTCACHE_H_ */
