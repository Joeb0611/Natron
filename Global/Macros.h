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
namespace Natron { }
namespace Python { }
#define NATRON_NAMESPACE_USING using namespace Natron;
#define NATRON_PYTHON_NAMESPACE_USING using namespace Natron::Python;
#endif

#if defined(SBK_RUN)
#define NATRON_NAMESPACE
#define NATRON_NAMESPACE_ENTER
#define NATRON_NAMESPACE_EXIT
#define NATRON_PYTHON_NAMESPACE
#define NATRON_PYTHON_NAMESPACE_ENTER
#define NATRON_PYTHON_NAMESPACE_EXIT
#else
#define NATRON_NAMESPACE Natron
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
#define NATRON_ENGINE_PYTHON_MODULE_NAME "NatronEngine"
#define NATRON_GUI_PYTHON_MODULE_NAME "NatronGui"
#define NATRON_PROJECT_ENV_VAR_MAX_RECURSION 100
#define NATRON_MAX_CACHE_FILES_OPENED 20000
#define NATRON_CUSTOM_HTML_TAG_START "<Natron>"
#define NATRON_CUSTOM_HTML_TAG_END "</Natron>"
#define NATRON_FILE_DIALOG_PREVIEW_READER_NAME "Natron_File_Dialog_Preview_Provider_Reader"
#define NATRON_FILE_DIALOG_PREVIEW_VIEWER_NAME "Natron_File_Dialog_Preview_Provider_Viewer"
#define NATRON_VERSION_MAJOR_30 3
#define NATRON_VERSION_MINOR_30 0
#define NATRON_VERSION_REVISION_30 0
#define NATRON_VERSION_MAJOR_26 2
#define NATRON_VERSION_MINOR_26 6
#define NATRON_VERSION_REVISION_26 0
#define NATRON_VERSION_MAJOR_25 2
#define NATRON_VERSION_MINOR_25 5
#define NATRON_VERSION_REVISION_25 0
#define NATRON_VERSION_MAJOR_24 2
#define NATRON_VERSION_MINOR_24 4
#define NATRON_VERSION_REVISION_24 4
#define NATRON_VERSION_MAJOR_23 2
#define NATRON_VERSION_MINOR_23 3
#define NATRON_VERSION_REVISION_23 16
#define NATRON_VERSION_MAJOR_22 2
#define NATRON_VERSION_MINOR_22 2
#define NATRON_VERSION_REVISION_22 10
#define NATRON_VERSION_MAJOR_21 2
#define NATRON_VERSION_MINOR_21 1
#define NATRON_VERSION_REVISION_21 10
#define NATRON_VERSION_MAJOR NATRON_VERSION_MAJOR_26
#define NATRON_VERSION_MINOR NATRON_VERSION_MINOR_26
#define NATRON_VERSION_REVISION NATRON_VERSION_REVISION_26
#define NATRON_LATEST_VERSION_URL "https://natrongithub.github.io/LATEST_VERSION.txt"
#define NATRON_LAST_VERSION_FILE_VERSION 1
#define NATRON_DEVELOPMENT_DEVEL "Devel"
#define NATRON_DEVELOPMENT_ALPHA "Alpha"
#define NATRON_DEVELOPMENT_BETA "Beta"
#define NATRON_DEVELOPMENT_RELEASE_CANDIDATE "RC"
#define NATRON_DEVELOPMENT_RELEASE_STABLE "Release"
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
#define NATRON_DEVELOPMENT_STATUS NATRON_DEVELOPMENT_DEVEL
#endif
#define NATRON_DOCUMENTATION_ONLINE "http://natron.readthedocs.io"
#if defined(__NATRON_LINUX__) || defined(__NATRON_OSX__)
#define NATRON_CRASH_REPORTER_USE_FORK 1
#endif
#define NATRON_BREAKPAD_PROCESS_EXEC "breakpad_process_exec"
#define NATRON_BREAKPAD_PROCESS_PID "breakpad_process_pid"
#define NATRON_BREAKPAD_CLIENT_FD_ARG "breakpad_client_fd"
#define NATRON_BREAKPAD_PIPE_ARG "breakpad_pipe_path"
#define NATRON_BREAKPAD_COM_PIPE_ARG "breakpad_com_pipe_path"
#define NATRON_NATRON_TO_BREAKPAD_EXISTENCE_CHECK "-e"
#define NATRON_NATRON_TO_BREAKPAD_EXISTENCE_CHECK_ACK "-eack"
#define STRINGIZE_CPP_NAME_(token) # token
#define STRINGIZE_CPP_NAME(token) STRINGIZE_CPP_NAME_(token)
#ifdef NATRON_CUSTOM_BUILD_USER_TOKEN
#define NATRON_CUSTOM_BUILD_USER_NAME STRINGIZE_CPP_NAME(NATRON_CUSTOM_BUILD_USER_TOKEN)
#else
#define NATRON_CUSTOM_BUILD_USER_NAME ""
#endif
#define NATRON_VERSION_ENCODE(major, minor, revision) (((major) * 10000) + ((minor) * 100) + ((revision) * 1))
#define NATRON_VERSION_ENCODED NATRON_VERSION_ENCODE(NATRON_VERSION_MAJOR, NATRON_VERSION_MINOR, NATRON_VERSION_REVISION)
#if NATRON_VERSION_REVISION > 0
#define NATRON_VERSION_STRINGIZE__(major, minor, revision) # major "." # minor "." # revision
#define NATRON_VERSION_STRINGIZE_(major, minor, revision) NATRON_VERSION_STRINGIZE__(major, minor, revision)
#define NATRON_VERSION_STRING NATRON_VERSION_STRINGIZE_(NATRON_VERSION_MAJOR, NATRON_VERSION_MINOR, NATRON_VERSION_REVISION)
#else
#define NATRON_VERSION_STRINGIZE__(major, minor) # major "." # minor
#define NATRON_VERSION_STRINGIZE_(major, minor) NATRON_VERSION_STRINGIZE__(major, minor)
#define NATRON_VERSION_STRING NATRON_VERSION_STRINGIZE_(NATRON_VERSION_MAJOR, NATRON_VERSION_MINOR)
#endif
#define NATRON_PLUGIN_PATH_ENV_VAR "NATRON_PLUGIN_PATH"
#define NATRON_DISK_CACHE_PATH_ENV_VAR "NATRON_DISK_CACHE_PATH"
#define NATRON_IMAGES_PATH ":/Resources/Images/"
#define NATRON_APPLICATION_ICON_PATH NATRON_IMAGES_PATH "natronplusIcon.xpm"
#define NATRON_PYPLUG_MAGIC "# Natron PyPlug"
#define NATRON_PYPLUG_GENERATED "# This file was automatically generated by "
#define NATRON_ICON_SET_BLACK_AND_WHITE "2"
#define NATRON_ICON_SET_FADED_COLOURS "3"
#define NATRON_ICON_SET_NUMBER NATRON_ICON_SET_FADED_COLOURS
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
#define NATRON_ENABLE_TRIMAP 1
#define NATRON_ENABLE_IO_META_NODES 1
#define STRINGISE_IMPL(x) # x
#define STRINGISE(x) STRINGISE_IMPL(x)
#if _MSC_VER
#   define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#   define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#   define WARN(exp) ("WARNING: " exp)
#endif
#if defined(__clang__)
#define CLANG_PRAGMA(PRAGMA) _Pragma(PRAGMA)
#else
#define CLANG_PRAGMA(PRAGMA)
#endif
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define GCC_DIAG_STR(s) # s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
# define GCC_DIAG_DO_PRAGMA(x) _Pragma (# x)
# define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
# if defined(__clang__) || ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) GCC_DIAG_PRAGMA( ignored GCC_DIAG_JOINSTR(-W, x) )
#  define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
#  define GCC_DIAG_PEDANTIC_OFF GCC_DIAG_PRAGMA(push) GCC_DIAG_PRAGMA( ignored GCC_DIAG_PEDANTIC )
#  define GCC_DIAG_PEDANTIC_ON GCC_DIAG_PRAGMA(pop)
# else
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA( ignored GCC_DIAG_JOINSTR(-W, x) )
#  define GCC_DIAG_ON(x)  GCC_DIAG_PRAGMA( warning GCC_DIAG_JOINSTR(-W, x) )
#  define GCC_DIAG_PEDANTIC_OFF GCC_DIAG_PRAGMA( ignored GCC_DIAG_PEDANTIC )
#  define GCC_DIAG_PEDANTIC_ON  GCC_DIAG_PRAGMA( warning GCC_DIAG_PEDANTIC )
# endif
#else
# define GCC_DIAG_OFF(x)
# define GCC_DIAG_ON(x)
# define GCC_DIAG_PEDANTIC_OFF
# define GCC_DIAG_PEDANTIC_ON
#endif
#ifdef __clang__
#  define CLANG_DIAG_STR(s) # s
#  define CLANG_DIAG_JOINSTR(x, y) CLANG_DIAG_STR(x ## y)
#  define CLANG_DIAG_DO_PRAGMA(x) _Pragma (# x)
#  define CLANG_DIAG_PRAGMA(x) CLANG_DIAG_DO_PRAGMA(clang diagnostic x)
#    define CLANG_DIAG_OFF(x) CLANG_DIAG_PRAGMA(push) CLANG_DIAG_PRAGMA( ignored CLANG_DIAG_JOINSTR(-W, x) )
#   define CLANG_DIAG_ON(x) CLANG_DIAG_PRAGMA(pop)
#  define GCC_DIAG_PEDANTIC "-Wpedantic"
#  define GCC_ONLY_DIAG_OFF(x)
#  define GCC_ONLY_DIAG_ON(x)
#else
#  define CLANG_DIAG_OFF(x)
#  define CLANG_DIAG_ON(x)
#  define CLANG_DIAG_PRAGMA(x)
#  if (__GNUC__ >= 7)
#    define GCC_DIAG_PEDANTIC "-Wpedantic"
#  else
#    define GCC_DIAG_PEDANTIC "-pedantic"
#  endif
#  define GCC_ONLY_DIAG_OFF(x) GCC_DIAG_OFF(x)
#  define GCC_ONLY_DIAG_ON(x) GCC_DIAG_ON(x)
GCC_ONLY_DIAG_OFF(pragmas)
#endif
#ifndef __has_warning
#define __has_warning(x) 0
#endif
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 408
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF GCC_DIAG_OFF(unused-local-typedefs)
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON GCC_DIAG_ON(unused-local-typedefs)
#else
#if __has_warning("-Wunused-local-typedef")
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF CLANG_DIAG_OFF(unused-local-typedef)
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON CLANG_DIAG_ON(unused-local-typedef)
#elif __has_warning("-Wunused-local-typedefs")
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF CLANG_DIAG_OFF(unused-local-typedefs)
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON CLANG_DIAG_ON(unused-local-typedefs)
#else
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF
# define GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON
#endif
#endif
#if __has_warning("-Wunused-private-field")
# define GCC_DIAG_UNUSED_PRIVATE_FIELD_OFF CLANG_DIAG_OFF(unused-private-field)
# define GCC_DIAG_UNUSED_PRIVATE_FIELD_ON CLANG_DIAG_ON(unused-private-field)
#else
# define GCC_DIAG_UNUSED_PRIVATE_FIELD_OFF
# define GCC_DIAG_UNUSED_PRIVATE_FIELD_ON
#endif
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 510
# define GCC_DIAG_SUGGEST_OVERRIDE_OFF GCC_DIAG_OFF(suggest-override)
# define GCC_DIAG_SUGGEST_OVERRIDE_ON GCC_DIAG_ON(suggest-override)
#else
#if __has_warning("-Winconsistent-missing-override")
# define GCC_DIAG_SUGGEST_OVERRIDE_OFF CLANG_DIAG_OFF(inconsistent-missing-override)
# define GCC_DIAG_SUGGEST_OVERRIDE_ON CLANG_DIAG_ON(inconsistent-missing-override)
#else
# define GCC_DIAG_SUGGEST_OVERRIDE_OFF
# define GCC_DIAG_SUGGEST_OVERRIDE_ON
#endif
#endif
#ifdef COMPILER
#undef COMPILER
#endif
#define COMPILER(NATRON_FEATURE) (NATRON_COMPILER_ ## NATRON_FEATURE)
#define COMPILER_SUPPORTS(NATRON_COMPILER_FEATURE) (NATRON_COMPILER_SUPPORTS_ ## NATRON_COMPILER_FEATURE)
#define COMPILER_QUIRK(NATRON_COMPILER_QUIRK) (NATRON_COMPILER_QUIRK_ ## NATRON_COMPILER_QUIRK)
#ifdef __has_feature
#define COMPILER_HAS_CLANG_FEATURE(x) __has_feature(x)
#else
#define COMPILER_HAS_CLANG_FEATURE(x) 0
#endif
#ifdef __has_declspec_attribute
#define COMPILER_HAS_CLANG_DECLSPEC(x) __has_declspec_attribute(x)
#else
#define COMPILER_HAS_CLANG_DECLSPEC(x) 0
#endif
#if defined(__clang__)
#define NATRON_COMPILER_CLANG 1
#define NATRON_COMPILER_SUPPORTS_BLOCKS COMPILER_HAS_CLANG_FEATURE(blocks)
#define NATRON_COMPILER_SUPPORTS_C_STATIC_ASSERT COMPILER_HAS_CLANG_FEATURE(c_static_assert)
#define NATRON_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS COMPILER_HAS_CLANG_FEATURE(cxx_reference_qualified_functions)
#define NATRON_COMPILER_SUPPORTS_CXX_EXCEPTIONS COMPILER_HAS_CLANG_FEATURE(cxx_exceptions)
#define NATRON_COMPILER_SUPPORTS_BUILTIN_IS_TRIVIALLY_COPYABLE COMPILER_HAS_CLANG_FEATURE(is_trivially_copyable)
#ifdef __cplusplus
#if __cplusplus <= 201103L
#define NATRON_CPP_STD_VER 11
#elif __cplusplus <= 201402L
#define NATRON_CPP_STD_VER 14
#endif
#endif
#endif
#if defined(__GNUC__)
#define NATRON_COMPILER_GCC_OR_CLANG 1
#endif
#if COMPILER(GCC_OR_CLANG) && !COMPILER(CLANG)
#define NATRON_COMPILER_GCC 1
#define NATRON_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS 1
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define GCC_VERSION_AT_LEAST(major, minor, patch) (GCC_VERSION >= (major * 10000 + minor * 100 + patch))
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NATRON_COMPILER_SUPPORTS_C_STATIC_ASSERT 1
#endif
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#if defined(__MINGW32__)
#define NATRON_COMPILER_MINGW 1
#include <_mingw.h>
#endif
#if COMPILER(MINGW) && defined(__MINGW64_VERSION_MAJOR)
#define NATRON_COMPILER_MINGW64 1
#endif
#if defined(_MSC_VER)
#define NATRON_COMPILER_MSVC 1
#define NATRON_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS 1
#endif
#if defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#define NATRON_COMPILER_SUNCC 1
#endif
#if !COMPILER(CLANG) && !COMPILER(MSVC)
#define NATRON_COMPILER_QUIRK_CONSIDERS_UNREACHABLE_CODE 1
#endif
#if defined(__ARM_EABI__) || defined(__EABI__)
#define NATRON_COMPILER_SUPPORTS_EABI 1
#endif
#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304
#define NATRON_COMPILER_SUPPORTS_RELAXED_CONSTEXPR 1
#endif
#if !defined(RELAXED_CONSTEXPR)
#if COMPILER_SUPPORTS(RELAXED_CONSTEXPR)
#define RELAXED_CONSTEXPR constexpr
#else
#define RELAXED_CONSTEXPR
#endif
#endif
#define ASAN_ENABLED COMPILER_HAS_CLANG_FEATURE(address_sanitizer)
#if ASAN_ENABLED
#define SUPPRESS_ASAN __attribute__((no_sanitize_address))
#else
#define SUPPRESS_ASAN
#endif
#if !defined(ALWAYS_INLINE) && COMPILER(GCC_OR_CLANG) && defined(NDEBUG) && !COMPILER(MINGW)
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#endif
#if !defined(ALWAYS_INLINE) && COMPILER(MSVC) && defined(NDEBUG)
#define ALWAYS_INLINE __forceinline
#endif
#if !defined(ALWAYS_INLINE)
#define ALWAYS_INLINE inline
#endif
#ifdef __cplusplus
#define NATRON_EXTERN_C_BEGIN extern "C" {
#define NATRON_EXTERN_C_END }
#else
#define NATRON_EXTERN_C_BEGIN
#define NATRON_EXTERN_C_END
#endif
#if !defined(FALLTHROUGH) && defined(__cplusplus) && defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(clang::fallthrough)
#define FALLTHROUGH [[clang::fallthrough]]
#elif __has_cpp_attribute(gnu::fallthrough)
#define FALLTHROUGH [[gnu::fallthrough]]
#endif
#endif
#if !defined(FALLTHROUGH)
#define FALLTHROUGH
#endif
#if !defined(LIKELY) && COMPILER(GCC_OR_CLANG)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#endif
#if !defined(LIKELY)
#define LIKELY(x) (x)
#endif
#if !defined(NEVER_INLINE) && COMPILER(GCC_OR_CLANG)
#define NEVER_INLINE __attribute__((__noinline__))
#endif
#if !defined(NEVER_INLINE) && COMPILER(MSVC)
#define NEVER_INLINE __declspec(noinline)
#endif
#if !defined(NEVER_INLINE)
#define NEVER_INLINE
#endif
#if !defined(NO_RETURN) && COMPILER(GCC_OR_CLANG)
#define NO_RETURN __attribute((__noreturn__))
#endif
#if !defined(NO_RETURN) && COMPILER(MSVC)
#define NO_RETURN __declspec(noreturn)
#endif
#if !defined(NO_RETURN)
#define NO_RETURN
#endif
#if !defined(RETURNS_NONNULL) && COMPILER(GCC_OR_CLANG)
#define RETURNS_NONNULL __attribute__((returns_nonnull))
#endif
#if !defined(RETURNS_NONNULL)
#define RETURNS_NONNULL
#endif
#if !defined(NO_RETURN_WITH_VALUE) && !COMPILER(MSVC)
#define NO_RETURN_WITH_VALUE NO_RETURN
#endif
#if !defined(NO_RETURN_WITH_VALUE)
#define NO_RETURN_WITH_VALUE
#endif
#if !defined(OBJC_CLASS) && defined(__OBJC__)
#define OBJC_CLASS @class
#endif
#if !defined(OBJC_CLASS)
#define OBJC_CLASS class
#endif
#if !defined(PURE_FUNCTION) && COMPILER(GCC_OR_CLANG)
#define PURE_FUNCTION __attribute__((__pure__))
#endif
#if !defined(PURE_FUNCTION)
#define PURE_FUNCTION
#endif
#if !defined(UNUSED_FUNCTION) && COMPILER(GCC_OR_CLANG)
#define UNUSED_FUNCTION __attribute__((unused))
#endif
#if !defined(UNUSED_FUNCTION)
#define UNUSED_FUNCTION
#endif
#if !defined(REFERENCED_FROM_ASM) && COMPILER(GCC_OR_CLANG)
#define REFERENCED_FROM_ASM __attribute__((__used__))
#endif
#if !defined(REFERENCED_FROM_ASM)
#define REFERENCED_FROM_ASM
#endif
#if !defined(UNLIKELY) && COMPILER(GCC_OR_CLANG)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif
#if !defined(UNLIKELY)
#define UNLIKELY(x) (x)
#endif
#if !defined(UNUSED_LABEL) && COMPILER(MSVC)
#define UNUSED_LABEL(label) if (false) goto label
#endif
#if !defined(UNUSED_LABEL)
#define UNUSED_LABEL(label) UNUSED_PARAM(&& label)
#endif
#if !defined(UNUSED_PARAM) && COMPILER(MSVC)
#define UNUSED_PARAM(variable) (void)&variable
#endif
#if !defined(UNUSED_PARAM)
#define UNUSED_PARAM(variable) (void)variable
#endif
#if !defined(WARN_UNUSED_RETURN) && COMPILER(GCC_OR_CLANG)
#define WARN_UNUSED_RETURN __attribute__((__warn_unused_result__))
#endif
#if !defined(WARN_UNUSED_RETURN)
#define WARN_UNUSED_RETURN
#endif
#if !defined(__has_include) && COMPILER(MSVC)
#define __has_include(path) 0
#endif
#if defined(__clang__)
#ifdef __has_extension
#define COMPILER_HAS_CLANG_EXTENSION(x) __has_extension(x)
#else
#define COMPILER_HAS_CLANG_FEATURE(x) COMPILER_HAS_CLANG_FEATURE(x)
#endif
#define NATRON_COMPILER_SUPPORTS_CXX_VARIADIC_TEMPLATES __has_extension(cxx_variadic_templates)
#define NATRON_COMPILER_SUPPORTS_CXX_RVALUE_REFERENCES COMPILER_HAS_CLANG_EXTENSION(cxx_rvalue_references) && COMPILER_HAS_CLANG_EXTENSION(cxx_nonstatic_member_init)
#define NATRON_COMPILER_SUPPORTS_CXX_DELETED_FUNCTIONS COMPILER_HAS_CLANG_EXTENSION(cxx_deleted_functions)
#define NATRON_SUPPORTS_CXX_NULLPTR COMPILER_HAS_CLANG_FEATURE(cxx_nullptr)
#define NATRON_COMPILER_SUPPORTS_CXX_EXPLICIT_CONVERSIONS COMPILER_HAS_CLANG_FEATURE(cxx_explicit_conversions)
#define NATRON_COMPILER_SUPPORTS_CXX_OVERRIDE_CONTROL COMPILER_HAS_CLANG_EXTENSION(cxx_override_control)
#define NATRON_COMPILER_SUPPORTS_HAS_TRIVIAL_DESTRUCTOR COMPILER_HAS_CLANG_EXTENSION(has_trivial_destructor)
#endif
#if defined(_MSC_VER)
#if _MSC_VER < 1400
#define NATRON_COMPILER_MSVC7_OR_LOWER 1
#elif _MSC_VER < 1600
#define NATRON_COMPILER_MSVC9_OR_LOWER 1
#endif
#if !COMPILER(CLANG) && _MSC_VER >= 1600
#define NATRON_SUPPORTS_CXX_NULLPTR 1
#endif
#if !COMPILER(CLANG)
#define NATRON_COMPILER_SUPPORTS_CXX_OVERRIDE_CONTROL 1
#define NATRON_COMPILER_QUIRK_FINAL_IS_CALLED_SEALED 1
#endif
#endif
#if COMPILER(GCC) && !COMPILER(CLANG)
#if GCC_VERSION_AT_LEAST(4, 7, 0) && defined(__cplusplus) && __cplusplus >= 201103L
#define NATRON_COMPILER_SUPPORTS_CXX_RVALUE_REFERENCES 1
#define NATRON_COMPILER_SUPPORTS_CXX_DELETED_FUNCTIONS 1
#define NATRON_SUPPORTS_CXX_NULLPTR 1
#define NATRON_COMPILER_SUPPORTS_CXX_OVERRIDE_CONTROL 1
#define NATRON_COMPILER_QUIRK_GCC11_GLOBAL_ISINF_ISNAN 1
#elif GCC_VERSION_AT_LEAST(4, 6, 0) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#define NATRON_SUPPORTS_CXX_NULLPTR 1
#define NATRON_COMPILER_QUIRK_GCC11_GLOBAL_ISINF_ISNAN 1
#endif
#endif
#if COMPILER(GCC)
#ifdef __cplusplus
template<typename T>
inline T
ignore_result( T x __attribute__( (unused) ) )
{
    return x;
}
#endif
#else
#ifdef __cplusplus
template<typename T>
inline T
ignore_result(T x)
{
    return x;
}
#endif
#endif
#if COMPILER_SUPPORTS(CXX_OVERRIDE_CONTROL) && !COMPILER(MSVC)
#define OVERRIDE override
#if COMPILER_QUIRK(FINAL_IS_CALLED_SEALED)
#define FINAL sealed
#else
#define FINAL final
#endif
#else
#define OVERRIDE
#define FINAL
#endif
#if COMPILER_SUPPORTS(CXX_OVERRIDE_CONTROL)
CLANG_DIAG_OFF(c++11-extensions)
#endif

#endif // ifndef NATRON_GLOBAL_MACROS_H
