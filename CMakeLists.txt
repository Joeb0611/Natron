# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
# a fork of Natron <https://natrongithub.github.io/>.
# (C) 2026 Natron+ contributors
# (C) 2018-2023 The Natron developers
# (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
#
# Modified 2026-09-16: Natron+ homepage and description; windres include path.
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

include(GNUInstallDirs)

option(NATRON_QT6 "use Qt6" OFF)
option(NATRON_SYSTEM_LIBS "use system versions of dependencies instead of bundled ones" OFF)
option(NATRON_BUILD_TESTS "build the Natron test suite" ON)
option(NATRON_ASAN "build with AddressSanitizer (Linux/macOS)" OFF)
option(NATRON_TSAN "build with ThreadSanitizer (Linux/macOS)" OFF)
set(NATRON_WINE_TOOLS_DIR "" CACHE PATH
    "Unix wrappers for mingw64 moc/rcc/shiboken2 when cross-compiling from Linux")

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

# Ubuntu MinGW gcc-13 cannot link MSYS2's static Boost (built with gcc 16:
# __cxa_call_terminate). Use the shared import library so those symbols stay
# inside libboost_serialization-mt.dll.
if(WIN32 AND CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32" AND NATRON_MINGW64)
    set(_np_boost_ser "${NATRON_MINGW64}/lib/cmake/boost_serialization-1.92.0-shared")
    if(EXISTS "${_np_boost_ser}/boost_serialization-config.cmake")
        set(Boost_USE_STATIC_LIBS OFF)
        set(boost_serialization_DIR "${_np_boost_ser}" CACHE PATH
            "MSYS2 shared Boost.Serialization" FORCE)
    endif()
    unset(_np_boost_ser)
endif()
find_package(Boost COMPONENTS headers serialization CONFIG)
if(WIN32)
    # Prevent looking in the registry so that the MinGW python can be found.
    set(Python3_FIND_REGISTRY NEVER)
endif()
find_package(Python3 COMPONENTS Interpreter Development)

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
endif()

# When cross-compiling from Linux, Qt/Shiboken IMPORTED tools are .exe files.
# Point them at Unix wrappers so AUTOMOC / shiboken custom commands can run.
# Config-specific IMPORTED_LOCATION_<CONFIG> (e.g. RELEASE) overrides the
# generic IMPORTED_LOCATION, so set every variant.
if(NATRON_WINE_TOOLS_DIR)
    set(_np_locs
        IMPORTED_LOCATION
        IMPORTED_LOCATION_RELEASE
        IMPORTED_LOCATION_RELWITHDEBINFO
        IMPORTED_LOCATION_MINSIZEREL
        IMPORTED_LOCATION_DEBUG
    )
    foreach(_np_tool IN ITEMS moc rcc qmake)
        if(TARGET Qt5::${_np_tool} AND EXISTS "${NATRON_WINE_TOOLS_DIR}/${_np_tool}")
            foreach(_np_loc IN LISTS _np_locs)
                set_property(TARGET Qt5::${_np_tool} PROPERTY ${_np_loc}
                    "${NATRON_WINE_TOOLS_DIR}/${_np_tool}")
            endforeach()
        endif()
    endforeach()
    # Prefer the in-tree moc wrapper: it strips metaobjects moc emitted from
    # included headers (duplicate NodeGroup / OutputEffectInstance on MinGW).
    if(TARGET Qt5::moc AND EXISTS "${CMAKE_SOURCE_DIR}/tools/mingw-moc.sh")
        foreach(_np_loc IN LISTS _np_locs)
            set_property(TARGET Qt5::moc PROPERTY ${_np_loc}
                "${CMAKE_SOURCE_DIR}/tools/mingw-moc.sh")
        endforeach()
    endif()
    if(TARGET Shiboken2::shiboken2 AND EXISTS "${NATRON_WINE_TOOLS_DIR}/shiboken2")
        foreach(_np_loc IN LISTS _np_locs)
            set_property(TARGET Shiboken2::shiboken2 PROPERTY ${_np_loc}
                "${NATRON_WINE_TOOLS_DIR}/shiboken2")
        endforeach()
    endif()
    if(TARGET Qt5::moc)
        get_target_property(QT_MOC_EXECUTABLE Qt5::moc LOCATION)
        set(Qt5Core_MOC_EXECUTABLE Qt5::moc)
        set(Qt5Core_RCC_EXECUTABLE Qt5::rcc)
    endif()
    unset(_np_tool)
    unset(_np_loc)
    unset(_np_locs)
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
        # windres does not inherit C++ include dirs; Natron.rc needs Global/Macros.h.
        set(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -I${CMAKE_SOURCE_DIR} -i <SOURCE> -o <OBJECT>")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

enable_testing()

add_subdirectory(libs)

add_subdirectory(HostSupport)

# Linux MinGW (Ubuntu gcc + MSYS2 Qt/Python sysroot): search the compiler CRT
# before ${MINGW64}/include so wchar.h/stdlib.h are not mixed.
if(WIN32 AND CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32")
    include_directories(BEFORE SYSTEM
        /usr/x86_64-w64-mingw32/include
        /usr/share/mingw-w64/include)
endif()

add_subdirectory(Engine)

add_subdirectory(PythonBin)

add_subdirectory(Renderer)

add_subdirectory(Gui)

if(NATRON_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

add_subdirectory(App)
