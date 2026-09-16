/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: abort unwind + short headless render stall/zero-frame
 * regression for NatronGitHub/Natron#248.
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

#include <atomic>

#include <QElapsedTimer>
#include <QThread>

#include "Engine/AbortableRenderInfo.h"
#include "Engine/AppInstance.h"
#include "Engine/AppManager.h"
#include "Engine/CreateNodeArgs.h"
#include "Engine/EffectInstance.h"
#include "Engine/Image.h"
#include "Engine/ImagePlaneDesc.h"
#include "Engine/Node.h"
#include "Engine/Project.h"
#include "Engine/RectD.h"
#include "Engine/RectI.h"
#include "Engine/ThreadPool.h"

NATRON_NAMESPACE_USING

namespace {

class TrimapWaiterThread
    : public QThread
    , public AbortableThread
{
public:
    EffectInstance* effect;
    ImagePtr img;
    RectI roi;
    AbortableRenderInfoPtr abortInfo;
    EffectInstancePtr treeRoot;
    std::atomic<bool> enteredWait;
    bool waitResult;

    TrimapWaiterThread()
        : QThread()
        , AbortableThread(this)
        , effect(0)
        , img()
        , roi()
        , abortInfo()
        , treeRoot()
        , enteredWait(false)
        , waitResult(true)
    {
        setThreadName("SchedulerAbort-trimap-waiter");
    }

    virtual void run() OVERRIDE
    {
        setAbortInfo(false /*sequential*/, abortInfo, treeRoot);
        effect->markImageAsBeingRenderedForTests(img, roi);
        enteredWait.store(true);
        waitResult = effect->waitForImageBeingRenderedElsewhereForTests(roi, img);
        effect->unmarkImageAsBeingRenderedForTests(img, !waitResult);
    }
};

} // namespace

TEST(SchedulerAbort, AbortableRenderInfoSetAbortedIsVisible)
{
    AbortableRenderInfoPtr info = AbortableRenderInfo::create(true, 1);
    ASSERT_NE(info.get(), nullptr);
    ASSERT_FALSE(info->isAborted());
    info->setAborted();
    ASSERT_TRUE(info->isAborted());
}

TEST(SchedulerAbort, AbortUnwindsTrimapWait)
{
    ASSERT_NE(appPTR, nullptr);
    AppInstancePtr app = appPTR->getTopLevelInstance();
    ASSERT_NE(app.get(), nullptr);

    CreateNodeArgs args(PLUGINID_NATRON_DOT, app->getProject());
    NodePtr node = app->createNode(args);
    ASSERT_NE(node.get(), nullptr);
    EffectInstancePtr effect = node->getEffectInstance();
    ASSERT_NE(effect.get(), nullptr);

    const RectD rod(0, 0, 8, 8);
    const RectI bounds(0, 0, 8, 8);
    ImagePtr img( new Image(ImagePlaneDesc::getRGBAComponents(),
                            rod,
                            bounds,
                            0,
                            1.,
                            eImageBitDepthFloat,
                            eImagePremultiplicationPremultiplied,
                            eImageFieldingOrderNone,
                            true) );
    ASSERT_TRUE( img->usesBitMap() );

    // Holder keeps refCount >= 1 so the waiter actually blocks.
    effect->markImageAsBeingRenderedForTests(img, bounds);

    TrimapWaiterThread waiter;
    waiter.effect = effect.get();
    waiter.img = img;
    waiter.roi = bounds;
    waiter.abortInfo = AbortableRenderInfo::create(true, 7);
    waiter.treeRoot = effect;

    waiter.start();

    QElapsedTimer started;
    started.start();
    while ( !waiter.enteredWait.load() && started.elapsed() < 2000 ) {
        QThread::msleep(5);
    }
    ASSERT_TRUE(waiter.enteredWait.load()) << "waiter never entered the trimap wait";

    waiter.abortInfo->setAborted();

    ASSERT_TRUE( waiter.wait(3000) ) << "abort did not unwind waitForImageBeingRenderedElsewhere (stall)";
    ASSERT_FALSE(waiter.waitResult) << "waiter should unwind with failure/abort, not succeed";

    effect->unmarkImageAsBeingRenderedForTests(img, true);
    node->destroyNode(true, false);
}

TEST(SchedulerAbort, ShortHeadlessRenderDoesNotStallOrWriteZeroFrames)
{
    ASSERT_NE(appPTR, nullptr);
    AppInstancePtr app = appPTR->getTopLevelInstance();
    ASSERT_NE(app.get(), nullptr);

    CreateNodeArgs args(PLUGINID_NATRON_DOT, app->getProject());
    NodePtr node = app->createNode(args);
    ASSERT_NE(node.get(), nullptr);
    EffectInstancePtr effect = node->getEffectInstance();
    ASSERT_NE(effect.get(), nullptr);

    const RectD rod(0, 0, 8, 8);
    const RectI bounds(0, 0, 8, 8);
    const int kWantFrames = 8;
    const qint64 kStallMs = 15000;
    int framesOk = 0;

    QElapsedTimer stallTimer;
    stallTimer.start();
    for (int i = 0; i < kWantFrames; ++i) {
        ASSERT_LT(stallTimer.elapsed(), kStallMs) << "short headless render stalled after " << framesOk << " frames";

        ImagePtr img( new Image(ImagePlaneDesc::getRGBAComponents(),
                                rod,
                                bounds,
                                0,
                                1.,
                                eImageBitDepthFloat,
                                eImagePremultiplicationPremultiplied,
                                eImageFieldingOrderNone,
                                true) );
        img->fillZero(bounds);
        effect->markImageAsBeingRenderedForTests(img, bounds);

        TrimapWaiterThread waiter;
        waiter.effect = effect.get();
        waiter.img = img;
        waiter.roi = bounds;
        waiter.abortInfo = AbortableRenderInfo::create(true, (U64)(i + 1));
        waiter.treeRoot = effect;
        waiter.start();

        QElapsedTimer started;
        started.start();
        while ( !waiter.enteredWait.load() && started.elapsed() < 2000 ) {
            QThread::msleep(1);
        }
        ASSERT_TRUE(waiter.enteredWait.load()) << "waiter never entered the trimap wait on frame " << (i + 1);

        // Produce the tile (non-zero pixels) and release the hold so the waiter unwinds.
        img->fillZero(bounds);
        {
            Image::WriteAccess acc(img.get());
            float* pix = reinterpret_cast<float*>( acc.pixelAt(0, 0) );
            ASSERT_NE(pix, nullptr);
            pix[0] = float(i + 1);
        }
        effect->unmarkImageAsBeingRenderedForTests(img, false);

        ASSERT_TRUE( waiter.wait(3000) ) << "frame " << (i + 1) << " stalled in waitForImageBeingRenderedElsewhere";
        ASSERT_TRUE(waiter.waitResult) << "frame " << (i + 1) << " wait failed (zero/aborted tile)";

        Image::ReadAccess racc(img.get());
        const float* out = reinterpret_cast<const float*>( racc.pixelAt(0, 0) );
        ASSERT_NE(out, nullptr);
        ASSERT_GT(out[0], 0.f) << "frame " << (i + 1) << " wrote zero";
        ++framesOk;
    }

    ASSERT_EQ(framesOk, kWantFrames) << "short headless render wrote zero frames";
    ASSERT_LT(stallTimer.elapsed(), kStallMs) << "short headless render stalled";

    node->destroyNode(true, false);
}
