/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: abort/trimap test hooks for scheduler stall (#248).
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

#ifndef NATRON_ENGINE_EFFECTINSTANCE_H
#define NATRON_ENGINE_EFFECTINSTANCE_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/Macros.h"

#include <list>
#include <bitset>

#include "Global/GlobalDefines.h"
#include "Global/KeySymbols.h"

#include "Engine/ImagePlaneDesc.h"
#include "Engine/ImageLocker.h"
#include "Engine/Knob.h" // for KnobHolder
#include "Engine/RectD.h"
#include "Engine/RectI.h"
#include "Engine/RenderScale.h"
#include "Engine/RenderStats.h"
#include "Engine/EngineFwd.h"
#include "Engine/ParallelRenderArgs.h"
#include "Engine/PluginActionShortcut.h"
#include "Engine/ViewIdx.h"

// Various useful plugin IDs, @see EffectInstance::getPluginID()
#define PLUGINID_OFX_MERGE        "net.sf.openfx.MergePlugin"
#define PLUGINID_OFX_TRACKERPM    "net.sf.openfx.TrackerPM"
#define PLUGINID_OFX_DOTEXAMPLE   "net.sf.openfx.dotexample"
#define PLUGINID_OFX_RADIAL       "net.sf.openfx.Radial"
#define PLUGINID_OFX_SENOISE      "net.sf.openfx.SeNoise"
#define PLUGINID_OFX_READOIIO     "fr.inria.openfx.ReadOIIO"
#define PLUGINID_OFX_WRITEOIIO    "fr.inria.openfx.WriteOIIO"
#define PLUGINID_OFX_ROTO         "net.sf.openfx.RotoPlugin"
#define CLIP_OFX_ROTO             "Roto" // The Roto input clip from the Roto plugin
#define PLUGINID_OFX_TRANSFORM    "net.sf.openfx.TransformPlugin"
#define PLUGINID_OFX_GRADE        "net.sf.openfx.GradePlugin"
#define PLUGINID_OFX_COLORCORRECT "net.sf.openfx.ColorCorrectPlugin"
#define PLUGINID_OFX_COLORLOOKUP  "net.sf.openfx.ColorLookupPlugin"
#define PLUGINID_OFX_BLURCIMG     "net.sf.cimg.CImgBlur"
#define PLUGINID_OFX_CORNERPIN    "net.sf.openfx.CornerPinPlugin"
#define PLUGINID_OFX_CONSTANT     "net.sf.openfx.ConstantPlugin"
#define PLUGINID_OFX_TIMEOFFSET   "net.sf.openfx.timeOffset"
#define PLUGINID_OFX_FRAMEHOLD    "net.sf.openfx.FrameHold"
#define PLUGINID_OFX_RETIME       "net.sf.openfx.Retime"
#define PLUGINID_OFX_FRAMERANGE   "net.sf.openfx.FrameRange"
#define PLUGINID_OFX_RUNSCRIPT    "fr.inria.openfx.RunScript"
#define PLUGINID_OFX_READFFMPEG   "fr.inria.openfx.ReadFFmpeg"
#define PLUGINID_OFX_SHUFFLE      "net.sf.openfx.ShufflePlugin"
#define PLUGINID_OFX_TIMEDISSOLVE "net.sf.openfx.TimeDissolvePlugin"
#define PLUGINID_OFX_WRITEFFMPEG  "fr.inria.openfx.WriteFFmpeg"
#define PLUGINID_OFX_READPFM      "fr.inria.openfx.ReadPFM"
#define PLUGINID_OFX_WRITEPFM     "fr.inria.openfx.WritePFM"
#define PLUGINID_OFX_READMISC     "fr.inria.openfx.ReadMisc"
#define PLUGINID_OFX_READPSD      "net.fxarena.openfx.ReadPSD"
#define PLUGINID_OFX_READKRITA    "fr.inria.openfx.ReadKrita"
#define PLUGINID_OFX_READSVG      "net.fxarena.openfx.ReadSVG"
#define PLUGINID_OFX_READORA      "fr.inria.openfx.OpenRaster"
#define PLUGINID_OFX_READCDR      "fr.inria.openfx.ReadCDR"
#define PLUGINID_OFX_READPNG      "fr.inria.openfx.ReadPNG"
#define PLUGINID_OFX_WRITEPNG     "fr.inria.openfx.WritePNG"
#define PLUGINID_OFX_READPDF      "fr.inria.openfx.ReadPDF"
#define PLUGINID_OFX_READBRAW     "net.sf.openfx.BlackmagicRAW"

#define PLUGINID_NATRON_VIEWER    (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Viewer")
#define PLUGINID_NATRON_DISKCACHE (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.DiskCache")
#define PLUGINID_NATRON_DOT       (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Dot")
#define PLUGINID_NATRON_READQT    (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.ReadQt")
#define PLUGINID_NATRON_WRITEQT   (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.WriteQt")
#define PLUGINID_NATRON_GROUP     (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Group")
#define PLUGINID_NATRON_INPUT     (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Input")
#define PLUGINID_NATRON_OUTPUT    (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Output")
#define PLUGINID_NATRON_BACKDROP  (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.BackDrop") // DO NOT change the capitalization, even if it's wrong
#define PLUGINID_NATRON_ROTOPAINT (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.RotoPaint")
#define PLUGINID_NATRON_ROTO (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Roto")
#define PLUGINID_NATRON_ROTOSMEAR (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.RotoSmear")
#define PLUGINID_NATRON_PRECOMP     (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Precomp")
#define PLUGINID_NATRON_TRACKER   (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Tracker")
#define PLUGINID_NATRON_JOINVIEWS     (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.JoinViews")
#define PLUGINID_NATRON_READ    (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Read")
#define PLUGINID_NATRON_WRITE    (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.Write")
#define PLUGINID_NATRON_ONEVIEW    (NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "." NATRON_ORGANIZATION_DOMAIN_SUB ".built-in.OneView")

#define kReaderParamNameOriginalFrameRange "originalFrameRange"

NATRON_NAMESPACE_ENTER

/**
 * @brief This is the base class for visual effects.
 * A live instance is always living throughout the lifetime of a Node and other copies are
 * created on demand when a render is needed.
 **/
class EffectInstance
    : public NamedKnobHolder
      , public LockManagerI<Image>
      , public std::enable_shared_from_this<EffectInstance>
{
GCC_DIAG_SUGGEST_OVERRIDE_OFF
    Q_OBJECT
GCC_DIAG_SUGGEST_OVERRIDE_ON

public:
    typedef std::map<int, std::list<ImagePtr> > InputImagesMap;
    typedef std::map<int, std::list<ImagePlaneDesc> > ComponentsNeededMap;
    typedef std::shared_ptr<ComponentsNeededMap> ComponentsNeededMapPtr;

#include "Engine/EffectInstanceBody.inc"
};

class ClipPreferencesRunning_RAII
{
    EffectInstance* _effect;

public:

    ClipPreferencesRunning_RAII(EffectInstance* effect)
        : _effect(effect)
    {
        _effect->setClipPreferencesRunning(true);
    }

    ~ClipPreferencesRunning_RAII()
    {
        _effect->setClipPreferencesRunning(false);
    }
};


/**
 * @typedef Any plug-in should have a static function called BuildEffect with the following signature.
 * It is used to build a new instance of an effect. Basically it should just call the constructor.
 **/
typedef EffectInstance* (*EffectBuilder)(NodePtr);

NATRON_NAMESPACE_EXIT

#endif // NATRON_ENGINE_EFFECTINSTANCE_H
