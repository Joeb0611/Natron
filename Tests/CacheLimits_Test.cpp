/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: Slice 2 cache hard-limit and free-disk guard tests.
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

#include <Python.h>

#include "Global/Macros.h"

#include <gtest/gtest.h>

#include <iostream>

#include <QDir>
#include <QFile>

#include "Engine/AppManager.h"
#include "Engine/MemoryFile.h"
#include "Engine/Settings.h"

NATRON_NAMESPACE_USING

TEST(CacheLimits, FreeDiskBytesReadable)
{
    ASSERT_NE(appPTR, nullptr);
    const U64 freeBytes = AppManager::freeDiskBytesAt(QDir::tempPath());
    EXPECT_GT(freeBytes, 0u);
}

TEST(CacheLimits, MemoryFileRefusesWhenReserveExceedsFreeSpace)
{
    ASSERT_NE(appPTR, nullptr);
    SettingsPtr settings = appPTR->getCurrentSettings();
    ASSERT_NE(settings.get(), nullptr);

    const int previous = (int)( settings->getMinimumFreeDiskBytes() / (1024ULL * 1024ULL * 1024ULL) );
    settings->setMinimumFreeDiskGiBForTests(100000); // 100 TiB reserve — larger than any real volume

    const QString path = QDir::temp().filePath(QString::fromUtf8("natronplus-cache-guard.bin"));
    QFile::remove(path);
    bool threw = false;
    try {
        MemoryFile file(path.toStdString(), 64 * 1024, MemoryFile::eFileOpenModeEnumIfExistsTruncateElseCreate);
        (void)file;
    } catch (const DiskFullError&) {
        threw = true;
    } catch (const std::exception& e) {
        threw = true;
        std::cerr << e.what() << std::endl;
    }
    settings->setMinimumFreeDiskGiBForTests(previous > 0 ? previous : 1);
    QFile::remove(path);
    EXPECT_TRUE(threw);
}

TEST(CacheLimits, HasEnoughFreeDiskHonorsReserve)
{
    ASSERT_NE(appPTR, nullptr);
    SettingsPtr settings = appPTR->getCurrentSettings();
    ASSERT_NE(settings.get(), nullptr);
    const int previous = (int)( settings->getMinimumFreeDiskBytes() / (1024ULL * 1024ULL * 1024ULL) );
    settings->setMinimumFreeDiskGiBForTests(0);
    EXPECT_TRUE( appPTR->hasEnoughFreeDiskForCache(1) );
    settings->setMinimumFreeDiskGiBForTests(100000);
    EXPECT_FALSE( appPTR->hasEnoughFreeDiskForCache(1) );
    settings->setMinimumFreeDiskGiBForTests(previous > 0 ? previous : 1);
}
