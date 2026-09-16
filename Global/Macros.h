/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: product strings, reverse-DNS, and URLs for Natron+.
 * .ntp XML tags remain <Natron> for project compatibility.
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
//
//  Created by Frédéric Devernay on 03/09/13.
//
//

#ifndef NATRON_GLOBAL_MACROS_H
#define NATRON_GLOBAL_MACROS_H

#ifdef __APPLE__
#define __NATRON_OSX__
#define __NATRON_UNIX__
#elif  defined(_WIN32)
#define __NATRON_WIN32__
#ifdef __MINGW32__
#define __NATRON_MINGW__
#endif
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__) || defined(__FreeBSD__)
#define __NATRON_UNIX__
#define __NATRON_LINUX__
#endif

#ifdef __cplusplus
#if __cplusplus < 201703L
#error "Natron 2.6+ requires C++17"
#endif
// Establish the name space.
namespace Natron { }
namespace Python { }
#define NATRON_NAMESPACE_USING using namespace Natron;
#define NATRON_PYTHON_NAMESPACE_USING using namespace Natron::Python;
#endif

#if defined(SBK_RUN)

// run shiboken without the Natron namespace, and add NATRON_NAMESPACE_USING to each cpp afterwards
#define NATRON_NAMESPACE
#define NATRON_NAMESPACE_ENTER
#define NATRON_NAMESPACE_EXIT
#define NATRON_PYTHON_NAMESPACE
#define NATRON_PYTHON_NAMESPACE_ENTER
#define NATRON_PYTHON_NAMESPACE_EXIT

#else // !SBK_RUN

#define NATRON_NAMESPACE Natron
// Macros to use in each file to enter and exit the right name spaces.
#define NATRON_NAMESPACE_ENTER namespace NATRON_NAMESPACE {
#define NATRON_NAMESPACE_EXIT }

#define NATRON_PYTHON_NAMESPACE Python
#define NATRON_PYTHON_NAMESPACE_ENTER namespace NATRON_PYTHON_NAMESPACE {
#define NATRON_PYTHON_NAMESPACE_EXIT }

#endif

#define NATRON_NAMESPACE_ANONYMOUS_ENTER namespace {
#define NATRON_NAMESPACE_ANONYMOUS_EXIT }

#define NATRON_APPLICATION_DESCRIPTION "Independent fork of Natron: open-source, cross-platform, nodal video compositing software."
#define NATRON_COPYRIGHT "(C) 2026 Natron+ contributors. (C) 2018-2023 The Natron developers. (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat."
#define NATRON_ORGANIZATION_NAME "Natron+"
#define NATRON_ORGANIZATION_DOMAIN_TOPLEVEL "io"
#define NATRON_ORGANIZATION_DOMAIN_SUB "github.joeb0611"
#define NATRON_ORGANIZATION_DOMAIN "io.github.joeb0611"
#define NATRON_APPLICATION_NAME "Natron+"
#define NATRON_WEBSITE_URL "https://github.com/Joeb0611/Natron"
#define NATRON_FORUM_URL "https://github.com/Joeb0611/Natron/discussions"
#define NATRON_ISSUE_TRACKER_URL "https://github.com/Joeb0611/Natron/issues"

// The MIME types for Natron documents are:
// *.ntp: application/vnd.natron.project
// *.nps: application/vnd.natron.nodepresets
// *.nl: application/vnd.natron.layout
// these MIME types are also used in:
// - NatronInfo.plist (for OSX)
// - tools/linux/include/qs/natron.qs
#define NATRON_PROJECT_FILE_EXT "ntp"
#define NATRON_PROJECT_FILE_MIME_TYPE "application/vnd.natron.project"
#define NATRON_PROJECT_UNTITLED "Untitled." NATRON_PROJECT_FILE_EXT
#define NATRON_CACHE_FILE_EXT "ntc"
#define NATRON_LAYOUT_FILE_EXT "nl"
#define NATRON_LAYOUT_FILE_MIME_TYPE "application/vnd.natron.layout"
#define NATRON_PRESETS_FILE_EXT "nps"
#define NATRON_PRESETS_FILE_MIME_TYPE "application/vnd.natron.nodepresets"
#define NATRON_PROJECT_ENV_VAR_NAME "Project"
#define NATRON_OCIO_ENV_VAR_NAME "OCIO"

//Define here the name of the Engine module that was chosen in the typesystem_engine.xml
#define NATRON_ENGINE_PYTHON_MODULE_NAME "NatronEngine"
#define NATRON_GUI_PYTHON_MODULE_NAME "NatronGui"

#define NATRON_PROJECT_ENV_VAR_MAX_RECURSION 100
#define NATRON_MAX_CACHE_FILES_OPENED 20000
// Keep the historical <Natron> XML tags so existing .ntp projects still load.
// NATRON_APPLICATION_NAME is the product string (Natron+) and must not be used here.
#define NATRON_CUSTOM_HTML_TAG_START "<Natron>"
#define NATRON_CUSTOM_HTML_TAG_END "</Natron>"


#define NATRON_FILE_DIALOG_PREVIEW_READER_NAME "Natron_File_Dialog_Preview_Provider_Reader"
#define NATRON_FILE_DIALOG_PREVIEW_VIEWER_NAME "Natron_File_Dialog_Preview_Provider_Viewer"

//////////////////////////////////////////Natron version/////////////////////////////////////////////

// The currently maintained Natron versions
// RB-3: 3.0.0
#define NATRON_VERSION_MAJOR_30 3
#define NATRON_VERSION_MINOR_30 0
#define NATRON_VERSION_REVISION_30 0

// RB-2.6: 2.6.0
#define NATRON_VERSION_MAJOR_26 2
#define NATRON_VERSION_MINOR_26 6
#define NATRON_VERSION_REVISION_26 0

// RB-2.5: 2.5.0
#define NATRON_VERSION_MAJOR_25 2
#define NATRON_VERSION_MINOR_25 5
#define NATRON_VERSION_REVISION_25 0

// RB-2.4: 2.4.4
#define NATRON_VERSION_MAJOR_24 2
#define NATRON_VERSION_MINOR_24 4
#define NATRON_VERSION_REVISION_24 4

// RB-2.3: 2.3.16
#define NATRON_VERSION_MAJOR_23 2
#define NATRON_VERSION_MINOR_23 3
#define NATRON_VERSION_REVISION_23 16

// RB-2.2: 2.2.10
#define NATRON_VERSION_MAJOR_22 2
#define NATRON_VERSION_MINOR_22 2
#define NATRON_VERSION_REVISION_22 10

// RB-2.1: 2.1.10
#define NATRON_VERSION_MAJOR_21 2
#define NATRON_VERSION_MINOR_21 1
#define NATRON_VERSION_REVISION_21 10

// The Natron version for this branch
#define NATRON_VERSION_MAJOR NATRON_VERSION_MAJOR_26
#define NATRON_VERSION_MINOR NATRON_VERSION_MINOR_26
#define NATRON_VERSION_REVISION NATRON_VERSION_REVISION_26


#define NATRON_LATEST_VERSION_URL "https://natrongithub.github.io/LATEST_VERSION.txt"
#define NATRON_LAST_VERSION_FILE_VERSION 1

// homemade builds should always show "Devel"
#define NATRON_DEVELOPMENT_DEVEL "Devel"
// the following are reserved for actual releases (binary and tarballs)
#define NATRON_DEVELOPMENT_ALPHA "Alpha"
#define NATRON_DEVELOPMENT_BETA "Beta"
#define NATRON_DEVELOPMENT_RELEASE_CANDIDATE "RC"
#define NATRON_DEVELOPMENT_RELEASE_STABLE "Release"
// The snapshot build scripts should add '-DNATRON_CONFIG_SNAPSHOT' to the compile
// options.
#define NATRON_DEVELOPMENT_SNAPSHOT "Snapshot"


#ifdef NATRON_CONFIG_SNAPSHOT
#define NATRON_DEVELOPMENT_STATUS NATRON_DEVELOPMENT_SNAPSHOT
#elif defined(NATRON_CONFIG_ALPHA)
#define NATRON_DEVELOPMENT_STATUS NATRON_DEVELOPMENT_ALPHA
#elif defined(NATRON_CONFIG_BETA)
#define NATRON_DEVELOPMENT_STATUS NATRON_CONFIG_BETA
#elif defined(NATRON_CONFIG_RC)
#define NATRON_DEVELOPMENT_STATUS NATRON_DEVELOPMENT_RELEASE_CANDIDATE
#elif defined(NATRON_CONFIG_STABLE)
#define NATRON_DEVELOPMENT_STATUS NATRON_DEVELOPMENT_RELEASE_STABLE
#else
//Fallback on "Devel" builds (most likely built from command line without passing to qmake the appropriate defines)
#define NATRON_DEVELOPMENT_STATUS NATRON_DEVELOPMENT_DEVEL
#endif

///For example RC 1, RC 2 etc... This is to be defined from within the qmake call, passing BUILD_NUMBER=X to the command line
//#define NATRON_BUILD_NUMBER 0


// Documentation
#if (NATRON_VERSION_MAJOR == NATRON_VERSION_MAJOR_22) && (NATRON_VERSION_MINOR == NATRON_VERSION_MINOR_22)
#define NATRON_DOCUMENTATION_ONLINE "http://natron.readthedocs.io/en/rb-2.2"
//#elif (NATRON_VERSION_MAJOR == NATRON_VERSION_MAJOR_30) && (NATRON_VERSION_MINOR == NATRON_VERSION_MINOR_30)
//#define NATRON_DOCUMENTATION_ONLINE "http://natron.readthedocs.io/en/rb-3.0"
#else
#define NATRON_DOCUMENTATION_ONLINE "http://natron.readthedocs.io"
#endif

#if defined(__NATRON_LINUX__) || defined(__NATRON_OSX__)
/*
   On Linux crash reporter MUST use fork() to spawn the Natron process because it needs to duplicate file descriptors for the pipe.
   On Windows, fork() doesn't exist so we use QProcess.
   OS X can use both because it doesn't require a file descriptor to be passed to Natron for the breakpad pipe.
 */
#define NATRON_CRASH_REPORTER_USE_FORK 1
#endif


#define NATRON_BREAKPAD_PROCESS_EXEC "breakpad_process_exec"
#define NATRON_BREAKPAD_PROCESS_PID "breakpad_process_pid"
#define NATRON_BREAKPAD_CLIENT_FD_ARG "breakpad_client_fd"
#define NATRON_BREAKPAD_PIPE_ARG "breakpad_pipe_path"
#define NATRON_BREAKPAD_COM_PIPE_ARG "breakpad_com_pipe_path"

#define NATRON_NATRON_TO_BREAKPAD_EXISTENCE_CHECK "-e"
#define NATRON_NATRON_TO_BREAKPAD_EXISTENCE_CHECK_ACK "-eack"

///If set the version of Natron will no longer be displayed in the splashscreen but the name of the user
///Set this from qmake

#define STRINGIZE_CPP_NAME_(token) # token
#define STRINGIZE_CPP_NAME(token) STRINGIZE_CPP_NAME_(token)

#ifdef NATRON_CUSTOM_BUILD_USER_TOKEN
#define NATRON_CUSTOM_BUILD_USER_NAME STRINGIZE_CPP_NAME(NATRON_CUSTOM_BUILD_USER_TOKEN)
#else
#define NATRON_CUSTOM_BUILD_USER_NAME ""
#endif

#define NATRON_VERSION_ENCODE(major, minor, revision) ( \
        ( (major) * 10000 ) \
        + ( (minor) * 100 )  \
        + ( (revision) * 1 ) )

#define NATRON_VERSION_ENCODED NATRON_VERSION_ENCODE( \
        NATRON_VERSION_MAJOR, \
        NATRON_VERSION_MINOR, \
        NATRON_VERSION_REVISION)

// Natron version string: if revision is 0, use only major.minor, else major.minor.revision
#if NATRON_VERSION_REVISION > 0
#define NATRON_VERSION_STRINGIZE__(major, minor, revision) \
    # major "." # minor "." # revision

#define NATRON_VERSION_STRINGIZE_(major, minor, revision) \
    NATRON_VERSION_STRINGIZE__(major, minor, revision)

#define NATRON_VERSION_STRING NATRON_VERSION_STRINGIZE_( \
        NATRON_VERSION_MAJOR, \
        NATRON_VERSION_MINOR, \
        NATRON_VERSION_REVISION)
#else
#define NATRON_VERSION_STRINGIZE__(major, minor) \
# major "." # minor

#define NATRON_VERSION_STRINGIZE_(major, minor) \
NATRON_VERSION_STRINGIZE__(major, minor)

#define NATRON_VERSION_STRING NATRON_VERSION_STRINGIZE_( \
        NATRON_VERSION_MAJOR, \
        NATRON_VERSION_MINOR)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NATRON_PLUGIN_PATH_ENV_VAR "NATRON_PLUGIN_PATH"
#define NATRON_DISK_CACHE_PATH_ENV_VAR "NATRON_DISK_CACHE_PATH"
#define NATRON_IMAGES_PATH ":/Resources/Images/"
#define NATRON_APPLICATION_ICON_PATH NATRON_IMAGES_PATH "natronplusIcon.xpm"
#define NATRON_PYPLUG_MAGIC "# Natron PyPlug"
#define NATRON_PYPLUG_GENERATED "# This file was automatically generated by "

///Natron will load all icons that are associated to a group toolbutton with the following icon set number, i.e:
///if it is 2, then it will load color_grouping_2.png , filter_grouping_2.png , etc... this way you can compile
///with different icons set easily.
#define NATRON_ICON_SET_BLACK_AND_WHITE "2"
#define NATRON_ICON_SET_FADED_COLOURS "3"
#define NATRON_ICON_SET_NUMBER NATRON_ICON_SET_FADED_COLOURS

// Group ordering is set at every place in the code where GROUP_ORDER appears in the comments
#define PLUGIN_GROUP_IMAGE "Image"
#define PLUGIN_GROUP_IMAGE_READERS "Readers"
#define PLUGIN_GROUP_IMAGE_WRITERS "Writers"
#define PLUGIN_GROUP_PAINT "Draw"
#define PLUGIN_GROUP_TIME "Time"
#define PLUGIN_GROUP_CHANNEL "Channel"
#define PLUGIN_GROUP_COLOR "Color"
#define PLUGIN_GROUP_FILTER "Filter"
#define PLUGIN_GROUP_KEYER "Keyer"
#define PLUGIN_GROUP_MERGE "Merge"
#define PLUGIN_GROUP_TRANSFORM "Transform"
#define PLUGIN_GROUP_3D "3D"
#define PLUGIN_GROUP_DEEP "Deep"
#define PLUGIN_GROUP_MULTIVIEW "Views"
#define PLUGIN_GROUP_TOOLSETS "ToolSets"
#define PLUGIN_GROUP_OTHER "Other"
#define PLUGIN_GROUP_DEFAULT "Misc"
#define PLUGIN_GROUP_OFX "OFX"

//Use this to use trimap instead of bitmap to avoid several threads computing the same area of an image at the same time.
//When enabled the value of 2 is a code for a pixel being rendered but not yet available.
//In this context, the reader of the bitmap should then wait for the pixel to be available.
#define NATRON_ENABLE_TRIMAP 1

//Use this to have all readers inside the same Read meta-node and all the writers
//into the same Write meta-node
#define NATRON_ENABLE_IO_META_NODES 1

// compiler_warning.h
#define STRINGISE_IMPL(x) # x
#define STRINGISE(x) STRINGISE_IMPL(x)

// Use: #pragma message WARN(\"My message\")
#if _MSC_VER
#   define FILE_LINE_LINK __FILE__ \"(\" STRINGISE(__LINE__) \") : \"
#   define WARN(exp) (FILE_LINE_LINK \"WARNING: \" exp)
#else
#   define WARN(exp) (\"WARNING: \" exp)
#endif
