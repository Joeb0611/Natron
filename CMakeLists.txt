# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
# a fork of Natron <https://natrongithub.github.io/>.
# (C) 2026 Natron+ contributors
# (C) 2018-2023 The Natron developers
# (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
#
# Modified 2026-09-16: Natron+ homepage and description.
#
# Natron is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Natron is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
# ***** END LICENSE BLOCK *****

cmake_minimum_required(VERSION 3.16.7)

project(Natron
    VERSION "2.6.0"
    DESCRIPTION "Natron+ — independent fork of Natron, focused on production stability"
    HOMEPAGE_URL "https://github.com/Joeb0611/Natron"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)

# Native arch by default so Apple Silicon (arm64) is first-class and not
# accidentally built as x86_64. Override with -DCMAKE_OSX_ARCHITECTURES=.
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "macOS target architecture")
endif()

include(GNUInstallDirs)

option(NATRON_QT6 "use Qt6" OFF)
option(NATRON_SYSTEM_LIBS "use system versions of dependencies instead of bundled ones" OFF)
option(NATRON_BUILD_TESTS "build the Natron test suite" ON)
option(NATRON_ASAN "build with AddressSanitizer (Linux/macOS)" OFF)
option(NATRON_TSAN "build with ThreadSanitizer (Linux/macOS)" OFF)

set(IS_DEBUG_BUILD OFF)
if(CMAKE_BUILD_TYPE MATCHES "^(debug|Debug|DEBUG)$")
    set(IS_DEBUG_BUILD ON)
    add_definitions(-DDEBUG)

    if(WIN32)
        # Debug builds need minimal optimizations to avoid excessive link times and link errors related to Eigen.
        add_compile_options(-O)
    endif()
else()
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
endif()

if(NATRON_ASAN)
    if(MSVC)
        message(FATAL_ERROR "NATRON_ASAN is not supported with MSVC")
    endif()
    if(NATRON_TSAN)
        message(FATAL_ERROR "NATRON_ASAN and NATRON_TSAN cannot be combined")
    endif()
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
    message(STATUS "AddressSanitizer enabled")
endif()

if(NATRON_TSAN)
    if(MSVC)
        message(FATAL_ERROR "NATRON_TSAN is not supported with MSVC")
    endif()
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
    message(STATUS "ThreadSanitizer enabled")
endif()

find_package(Boost COMPONENTS headers serialization CONFIG)
if(WIN32)
    # Prevent looking in the registry so that the MinGW python can be found.
    set(Python3_FIND_REGISTRY NEVER)
endif()
find_package(Python3 COMPONENTS Interpreter Development)
if(Python3_EXECUTABLE)
    # Shiboken2 still calls FindPythonInterp. Pin it to the same CPython
    # Natron linked so a newer Homebrew python3 (e.g. 3.14) is not picked.
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
endif()

# png/ico are generated (MCP/git cannot safely round-trip those binaries).
# The committed natronplusIcon.xpm is the Qt resource; this fills raster
# install/Windows-resource files when they are missing.
set(_NATRONPLUS_ICON_SCRIPT "${CMAKE_SOURCE_DIR}/tools/generate-natronplus-icons.py")
set(_NATRONPLUS_ICON_PNG "${CMAKE_SOURCE_DIR}/Gui/Resources/Images/natronplusIcon256.png")
set(_NATRONPLUS_ICON_ICO "${CMAKE_SOURCE_DIR}/Gui/Resources/Images/natronplusIcon256.ico")
if(Python3_EXECUTABLE AND EXISTS "${_NATRONPLUS_ICON_SCRIPT}")
    if(NOT EXISTS "${_NATRONPLUS_ICON_PNG}" OR NOT EXISTS "${_NATRONPLUS_ICON_ICO}")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${_NATRONPLUS_ICON_SCRIPT}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _np_icon_rv
        )
        if(NOT _np_icon_rv EQUAL 0)
            message(FATAL_ERROR "Failed to generate Natron+ icons (${_np_icon_rv})")
        endif()
    endif()
endif()

if(IS_DEBUG_BUILD AND WIN32)
    # Explicitly setting SHIBOKEN_PYTHON_LIBRARIES variable to avoid PYTHON_DEBUG_LIBRARY-NOTFOUND
    # link errors on Windows debug builds.
    set(SHIBOKEN_PYTHON_LIBRARIES ${Python3_LIBRARIES})
endif()

set(QT_COMPONENTS Core Gui Network Widgets Concurrent)
if(NATRON_QT6)
    list(APPEND QT_COMPONENTS OpenGLWidgets)
    find_package(Qt6 6.3 CONFIG REQUIRED COMPONENTS ${QT_COMPONENTS})
    find_package(Shiboken6 6.3 CONFIG REQUIRED COMPONENTS libshiboken6)
    find_package(Shiboken6Tools 6.3 CONFIG REQUIRED COMPONENTS shiboken6)
    find_package(PySide6 6.3 CONFIG REQUIRED COMPONENTS pyside6)
    set(QT_VERSION_MAJOR 6)
    set(PYSIDE_VERSION_MAJOR 6)

    get_target_property(PYSIDE_INCLUDE_DIRS PySide6::pyside6 INTERFACE_INCLUDE_DIRECTORIES)
    set(QTCORE_INCLUDE_DIRS ${Qt6Core_INCLUDE_DIRS})
    set(QTGUI_INCLUDE_DIRS ${Qt6Gui_INCLUDE_DIRS})
    set(QTWIDGETS_INCLUDE_DIRS ${Qt6Widgets_INCLUDE_DIRS})

    set(SHIBOKEN_EXE Shiboken6::shiboken6)
    set(SHIBOKEN_LIB Shiboken6::libshiboken)
    set(PYSIDE_LIB PySide6::pyside6)
else()
    find_package(Qt5 5.15 CONFIG REQUIRED COMPONENTS ${QT_COMPONENTS})
    find_package(Shiboken2 5.15 CONFIG REQUIRED COMPONENTS libshiboken2 shiboken2)

    find_package(PySide2 5.15 CONFIG REQUIRED COMPONENTS pyside2)
    set(QT_VERSION_MAJOR 5)
    set(PYSIDE_VERSION_MAJOR 2)

    get_target_property(PYSIDE_INCLUDE_DIRS PySide2::pyside2 INTERFACE_INCLUDE_DIRECTORIES)
    set(QTCORE_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
    set(QTGUI_INCLUDE_DIRS ${Qt5Gui_INCLUDE_DIRS})
    set(QTWIDGETS_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS})

    set(SHIBOKEN_EXE Shiboken2::shiboken2)
    set(SHIBOKEN_LIB Shiboken2::libshiboken)
    set(PYSIDE_LIB PySide2::pyside2)

    # Homebrew/macOS Qt5 is framework-based. Shiboken includes
    # <QtCore/qnamespace.h>, but Qt5 CMake only adds the framework Headers
    # directory (bare qnamespace.h). Prepend prefix/include and pass -F so
    # Apple Silicon and Intel Homebrew/MacPorts Qt5 both resolve.
    if(APPLE)
        get_filename_component(_NATRON_QT5_PREFIX "${Qt5Core_DIR}/../../.." ABSOLUTE)
        set(_NATRON_QT5_INCLUDE "${_NATRON_QT5_PREFIX}/include")
        if(EXISTS "${_NATRON_QT5_INCLUDE}/QtCore")
            list(INSERT QTCORE_INCLUDE_DIRS 0 "${_NATRON_QT5_INCLUDE}")
            list(INSERT QTGUI_INCLUDE_DIRS 0 "${_NATRON_QT5_INCLUDE}")
            list(INSERT QTWIDGETS_INCLUDE_DIRS 0 "${_NATRON_QT5_INCLUDE}")
            message(STATUS "macOS Qt5 shiboken include prefix: ${_NATRON_QT5_INCLUDE}")
        endif()
        if(EXISTS "${_NATRON_QT5_PREFIX}/lib/QtCore.framework")
            list(APPEND SHIBOKEN_PLATFORM_ARGS "-F${_NATRON_QT5_PREFIX}/lib")
            message(STATUS "macOS Qt5 shiboken framework path: ${_NATRON_QT5_PREFIX}/lib")
        endif()
    endif()
endif()

if(NOT DEFINED SHIBOKEN_PLATFORM_ARGS)
    set(SHIBOKEN_PLATFORM_ARGS "")
endif()

if(IS_DEBUG_BUILD AND WIN32)
    # Remove NDEBUG from Shiboken INTERFACE_COMPILE_DEFINITIONS so it is not inherited in debug builds.
    get_property(SHIBOKEN_DEFS TARGET ${SHIBOKEN_LIB} PROPERTY INTERFACE_COMPILE_DEFINITIONS)
    list(REMOVE_ITEM SHIBOKEN_DEFS NDEBUG)
    set_property(TARGET ${SHIBOKEN_LIB} PROPERTY INTERFACE_COMPILE_DEFINITIONS SHIBOKEN_DEFS)
endif()

#Since in Natron and OpenFX all strings are supposed UTF-8 and that the constructor
#for QString(const char*) assumes ASCII strings, we may run into troubles
add_compile_definitions(QT_NO_CAST_FROM_ASCII)

if (MSVC)
    # Allow __cplusplus to properly reflect the c++ standard version.
    add_compile_options(/Zc:__cplusplus)
    add_compile_definitions(__STDC_LIMIT_MACROS WIN32_LEAN_AND_MEAN _USE_MATH_DEFINES)
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes" )
    add_compile_options(-Wall -Wextra -Wmissing-declarations -Wno-multichar -Winit-self -Wno-long-long
                        -Wvla -Wdate-time -Wno-format-overflow -Wno-deprecated-copy)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wshift-overflow=2 -Wduplicated-cond -Wno-stringop-overflow)
    endif()
endif()

if(NATRON_SYSTEM_LIBS)
    find_package(glog)
    find_package(Ceres)
endif()

if(UNIX AND NOT APPLE)
    find_package(ECM NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
    find_package(X11 REQUIRED)
    find_package(Wayland COMPONENTS Client Egl)
elseif(APPLE)
    enable_language(OBJCXX)
elseif(WIN32)
    enable_language(RC)
    add_compile_definitions(WINDOWS WIN32 _UNICODE UNICODE NOMINMAX QHTTP_SERVER_STATIC)
    add_compile_definitions(COMPILED_FROM_DSP XML_STATIC) # for expat
    if (NOT MSVC)
        add_compile_options(-municode)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-mwindows -mthreads)
        endif()
        add_link_options(-mwindows -municode)
        set(CMAKE_RC_COMPILER_INIT windres)
        set(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

enable_testing()

add_subdirectory(libs)

add_subdirectory(HostSupport)

add_subdirectory(Engine)

add_subdirectory(PythonBin)

add_subdirectory(Renderer)

add_subdirectory(Gui)

if(NATRON_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

add_subdirectory(App)
