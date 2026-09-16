# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
# a fork of Natron <https://natrongithub.github.io/>.
# (C) 2026 Natron+ contributors
# (C) 2018-2023 The Natron developers
# (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
#
# Modified 2026-09-16: include Engine *.inc in headers for shiboken/AUTOMOC deps.
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

include(FindPkgConfig)

pkg_check_modules(Cairo REQUIRED IMPORTED_TARGET cairo fontconfig)

file(GLOB NatronEngine_HEADERS *.h)
file(GLOB NatronEngine_INCS *.inc)
list(APPEND NatronEngine_HEADERS ${NatronEngine_INCS})
file(GLOB NatronEngine_SOURCES *.cpp)

set(PYENGINE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Qt${QT_VERSION_MAJOR})
set(PYENGINE_INCLUDE_DIRS ${PYSIDE_INCLUDE_DIRS} ${PYSIDE_INCLUDE_DIRS}/QtCore)
list(APPEND PYENGINE_INCLUDE_DIRS ${QTCORE_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
list(TRANSFORM PYENGINE_INCLUDE_DIRS PREPEND "-I" OUTPUT_VARIABLE PYENGINE_INCS)
set(PYENGINE_HEADER PySide${PYSIDE_VERSION_MAJOR}_Engine_Python.h)

set(shiboken_args
    "-std=c++17" "--enable-parent-ctor-heuristic" "--use-isnull-as-nb_nonzero"
    "--avoid-protected-hack" "--enable-pyside-extensions"
    "-I." "-I.." "-I../Global" "-I../libs/OpenFX/include" ${PYENGINE_INCS}
    "-T${PYSIDE_TYPESYSTEMS}" "--output-directory=${PYENGINE_OUT}"
    "${PYENGINE_HEADER}" "typesystem_engine.xml")

execute_process(COMMAND ${Python3_EXECUTABLE} ../tools/utils/sourceList.py typesystem_engine.xml ${PYENGINE_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PyEngine_SOURCES)
set_property(SOURCE ${PyEngine_SOURCES} PROPERTY SKIP_AUTOGEN ON)
add_custom_command(OUTPUT ${PyEngine_SOURCES}
    COMMAND ${SHIBOKEN_EXE} ${shiboken_args}
    COMMAND ${Python3_EXECUTABLE} ../tools/utils/sourceCleanup.py typesystem_engine.xml ${PYENGINE_OUT}
    DEPENDS ${SHIBOKEN_EXE} ${PYENGINE_HEADER} ${NatronEngine_HEADERS} ../tools/utils/sourceCleanup.py typesystem_engine.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX AND NOT APPLE)
    set(XDG_LIBS ${X11_LIBRARIES})
    if(Wayland_FOUND)
        set(XDG_LIBS Wayland::Client Wayland::Egl ${XDG_LIBS})
        set(XDG_DEFS __NATRON_WAYLAND__)
    endif()
endif()


if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/SequenceParsing/tinydir/tinydir.h")
    message(FATAL_ERROR
        "Nested submodule libs/SequenceParsing/tinydir is missing (tinydir.h). "
        "Run: git submodule update --init --recursive")
endif()

list(APPEND NatronEngine_SOURCES
    ${NatronEngine_SOURCES}
    ${PyEngine_SOURCES}
    ../Global/glad_source.c
    ../Global/FStreamsSupport.cpp
    ../Global/ProcInfo.cpp
    ../Global/PythonUtils.cpp
    ../Global/StrUtils.cpp
    ../libs/SequenceParsing/SequenceParsing.cpp
)

# AUTOMOC of ViewerInstancePrivate.h also emits every QObject pulled in via
# ViewerInstance.h. Keep the nested ViewerInstancePrivate metaobject only.
set_property(SOURCE ViewerInstancePrivate.h PROPERTY SKIP_AUTOMOC ON)
set(_np_vip_full "${CMAKE_CURRENT_BINARY_DIR}/moc_ViewerInstancePrivate_full.cpp")
set(_np_vip_nested "${CMAKE_CURRENT_BINARY_DIR}/moc_ViewerInstancePrivate_nested.cpp")
add_custom_command(OUTPUT "${_np_vip_nested}"
    COMMAND Qt5::moc
        "${CMAKE_CURRENT_SOURCE_DIR}/ViewerInstancePrivate.h"
        -I "${CMAKE_CURRENT_SOURCE_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}/.."
        -I "${CMAKE_CURRENT_SOURCE_DIR}/../Global"
        -o "${_np_vip_full}"
    COMMAND /usr/bin/python3 "${CMAKE_SOURCE_DIR}/tools/extract-nested-moc.py"
        "${_np_vip_full}" "${_np_vip_nested}"
        Natron__ViewerInstance__ViewerInstancePrivate
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/ViewerInstancePrivate.h"
        "${CMAKE_SOURCE_DIR}/tools/extract-nested-moc.py"
    VERBATIM)
list(APPEND NatronEngine_SOURCES "${_np_vip_nested}")
set_property(SOURCE "${_np_vip_nested}" PROPERTY SKIP_AUTOGEN ON)

add_library(NatronEngine STATIC ${NatronEngine_HEADERS} ${NatronEngine_SOURCES})
target_link_libraries(NatronEngine
    PUBLIC
        HostSupport
        Boost::headers
        Boost::serialization
        PkgConfig::Cairo
    PRIVATE
        ${SHIBOKEN_LIB}
        ${PYSIDE_LIB}
        Qt::Core
        Qt::Concurrent
        Qt::Network
        Python3::Module
        ${XDG_LIBS}
        ceres
        hoedown
        mv
        openMVG
        tess
)

if(WIN32)
    #System library is required on windows to map network share names from drive letters
    target_link_libraries(NatronEngine PRIVATE mpr)
endif()

target_include_directories(NatronEngine
    PUBLIC
        ../Global/glad$<IF:$<CONFIG:Debug>,Deb,Rel>/include
    PRIVATE
        ${PYSIDE_INCLUDE_DIRS}/QtCore
        .
        ..
        ../Global
        ../libs/SequenceParsing
        ${X11_INCLUDE_DIR}
)
target_compile_definitions(NatronEngine
    PRIVATE
        NATRON_CUSTOM_BUILD_USER_TOKEN="${BUILD_USER_NAME}"
        NATRON_BUILD_NUMBER=0
        QT_NO_KEYWORDS
        ${XDG_DEFS}
)
