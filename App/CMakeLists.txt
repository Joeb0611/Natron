# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
# a fork of Natron <https://natrongithub.github.io/>.
# (C) 2026 Natron+ contributors
# (C) 2018-2023 The Natron developers
# (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
#
# Modified 2026-09-16: Natron+ bundle id, desktop id, and copyright.
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

set(Natron_SOURCES NatronApp_main.cpp)
if(WIN32)
    list(APPEND Natron_SOURCES ../Natron.rc)
endif()
add_executable(Natron ${Natron_SOURCES})
set_target_properties(Natron PROPERTIES ENABLE_EXPORTS TRUE)
if(APPLE)
    set_target_properties(Natron PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Natron+"
        MACOSX_BUNDLE_INFO_STRING "Natron+ Copyright (c) 2026 Natron+ contributors"
        MACOSX_BUNDLE_ICON_FILE Icon.icns
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.joeb0611.natronplus"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2026 Natron+ contributors. Copyright (c) 2013-2023 INRIA, Alexandre Gauthier-Foichat, and the Natron developers."
    )
endif()
target_link_libraries(Natron
    PRIVATE
        NatronGui
        Python3::Python
)
target_include_directories(Natron
    PRIVATE
        ..
        ../Gui
        ../Engine
        ../Global
)

install(TARGETS Natron DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES ../Gui/Resources/Applications/io.github.joeb0611.natronplus.desktop
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES ../Gui/Resources/Metainfo/io.github.joeb0611.natronplus.appdata.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
install(FILES ../Gui/Resources/Mime/x-natron.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/mime")
install(FILES ../Gui/Resources/Images/natronplusIcon.xpm
              ../Gui/Resources/Images/natronplusIcon256.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")
install(FILES ../Gui/Resources/Images/natronProjectIcon_linux.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")
