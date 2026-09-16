/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: Slice 2 Dot create/destroy loop (Read→Viewer needs OFX Read + Gui).
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

#include "Engine/AppInstance.h"
#include "Engine/AppManager.h"
#include "Engine/CreateNodeArgs.h"
#include "Engine/EffectInstance.h"
#include "Engine/Node.h"
#include "Engine/Project.h"

NATRON_NAMESPACE_USING

TEST(AppLifecycle, DotCreateDestroyLoop)
{
    ASSERT_NE(appPTR, nullptr);
    AppInstancePtr app = appPTR->getTopLevelInstance();
    ASSERT_NE(app.get(), nullptr);
    ASSERT_NE(app->getProject().get(), nullptr);

    // Tests run as a background AppManager (no Gui). Viewer is skipped in
    // background; OFX Read is not in this compile. Dot is the Engine
    // open/close stand-in for Read→Viewer.
    for (int i = 0; i < 8; ++i) {
        CreateNodeArgs args(PLUGINID_NATRON_DOT, app->getProject());
        NodePtr node = app->createNode(args);
        ASSERT_NE(node.get(), nullptr) << "Dot create failed on iteration " << i;
        node->destroyNode(true, false);
    }
}
