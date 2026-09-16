/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * Copyright (C) 2026 Natron+ contributors
 * Copyright (C) 2018-2023 The Natron developers
 * Copyright (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: crash-dialog product string for Natron+.
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

#include "CrashDialog.h"

#include <algorithm>
#include <iostream>
#include <cassert>

#include <QDir>
#include <QThread>
#include <QTextStream>
#include <QFile>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QFileInfo>
#include <QPainter>
#include <QApplication>
#include <QLocalSocket>
#include <QFileDialog>
#include <QTextDocument>
#include <QMessageBox>
#include <QStyle>
#include <QSettings>

#include <QtOpenGL/QGLWidget>
#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/gl.h>
#  include <GL/glext.h>
#endif

#define NATRON_FONT "Droid Sans"
#define NATRON_FONT_SIZE_DEFAULT 11
