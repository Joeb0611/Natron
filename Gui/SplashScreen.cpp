/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron+ <https://github.com/Joeb0611/Natron>,
 * a fork of Natron <https://natrongithub.github.io/>.
 * (C) 2026 Natron+ contributors
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Modified 2026-09-16: Natron+ splash (no upstream lettermark).
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

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "SplashScreen.h"

#include <ctime>
#include <stdexcept>

CLANG_DIAG_OFF(deprecated)
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QFont>
#include <QPen>
#include <QBrush>

#include <QScreen>

CLANG_DIAG_ON(deprecated)

#ifdef DEBUG
#include "Global/FloatingPointExceptions.h"
#endif

NATRON_NAMESPACE_ENTER

SplashScreen::SplashScreen(const QString & /*filePath*/)
    : QWidget(0, Qt::FramelessWindowHint)
    , _pixmap()
    , _text()
    , _versionString()
{
    QString customBuildString = QString::fromUtf8(NATRON_CUSTOM_BUILD_USER_NAME);

    if ( customBuildString.isEmpty() ) {
        QString buildNo;
        if ( QString::fromUtf8(NATRON_DEVELOPMENT_STATUS) == QString::fromUtf8(NATRON_DEVELOPMENT_RELEASE_CANDIDATE) ) {
            buildNo = QString::number(NATRON_BUILD_NUMBER);
        }
        _versionString = tr("v%1 - %2 %3")
                         .arg( QString::fromUtf8(NATRON_VERSION_STRING) )
                         .arg( QString::fromUtf8(NATRON_DEVELOPMENT_STATUS) )
                         .arg(buildNo);
#     if defined(NATRON_CONFIG_SNAPSHOT) || defined(DEBUG)
        _versionString += QString::fromUtf8(" - ") + tr("built on %1").arg( QString::fromUtf8(__DATE__) );
#     endif
    } else {
        _versionString = tr("%1 for %2")
                         .arg( QString::fromUtf8(NATRON_APPLICATION_NAME) )
                         .arg(customBuildString);
#     if defined(NATRON_CONFIG_SNAPSHOT) || defined(DEBUG)
        _versionString += QString::fromUtf8(" - ") + tr("built on %1").arg( QString::fromUtf8(__DATE__) );
#     endif
    }

    setAttribute( Qt::WA_TransparentForMouseEvents );
    setAttribute(Qt::WA_TranslucentBackground, true);

    // Draw a Natron+ splash instead of shipping the upstream Natron lettermark.
    _scale = 1.;
    const int w = int(720 * _scale);
    const int h = int(400 * _scale);
    _pixmap = QPixmap(w, h);
    _pixmap.fill(Qt::transparent);
    {
        QPainter gp(&_pixmap);
        gp.setRenderHint(QPainter::Antialiasing, true);
        gp.setBrush( QColor(20, 22, 28) );
        gp.setPen( QPen(QColor(200, 176, 122), 3) );
        gp.drawRoundedRect( QRectF(1, 1, w - 2, h - 2), 18, 18 );
        const QRectF plusV( w / 2. - 14, 56, 28, 112 );
        const QRectF plusH( w / 2. - 56, 98, 112, 28 );
        gp.setBrush( QColor(232, 217, 168) );
        gp.setPen(Qt::NoPen);
        gp.drawRoundedRect(plusV, 6, 6);
        gp.drawRoundedRect(plusH, 6, 6);
        gp.setPen( QColor(232, 217, 168) );
        QFont title = gp.font();
        title.setPointSize(28);
        title.setBold(true);
        gp.setFont(title);
        gp.drawText( QRect(0, 180, w, 48), Qt::AlignHCenter, QString::fromUtf8(NATRON_APPLICATION_NAME) );
        QFont sub = gp.font();
        sub.setPointSize(11);
        sub.setBold(false);
        gp.setFont(sub);
        gp.setPen( QColor(180, 184, 192) );
        gp.drawText( QRect(40, 228, w - 80, 40), Qt::AlignHCenter,
                     tr("Independent fork of Natron — not an official Natron Project build") );
    }
    resize( _pixmap.width(), _pixmap.height() );
    {
#ifdef DEBUG
        boost_adaptbx::floating_point::exception_trapping trap(0);
#endif
        show();
    }

    QScreen* desktop = QGuiApplication::primaryScreen();
    QRect screen = desktop->availableGeometry();
    move(screen.width() / 2 - width() / 2, screen.height() / 2 - height() / 2);
}

void
SplashScreen::updateText(const QString & text)
{
    _text = text;
    {
#ifdef DEBUG
        boost_adaptbx::floating_point::exception_trapping trap(0);
#endif
        update();
        QCoreApplication::processEvents();
    }
}

void
SplashScreen::paintEvent(QPaintEvent*)
{
    QStyleOption opt;

    opt.init(this);
    QPainter p(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.drawPixmap(0, 0, _pixmap);
    p.setPen( QColor(232, 217, 168) );
    p.drawText(QPointF(40 * _scale, 280 * _scale), _text);
    p.setPen( QColor(180, 184, 192) );
    p.drawText(QPointF(40 * _scale, _pixmap.height() - 24 * _scale), _versionString);
}

LoadProjectSplashScreen::LoadProjectSplashScreen(const QString & filePath)
    : QWidget(0, Qt::SplashScreen)
    , _pixmap()
    , _text()
    , _projectName(filePath)
{
    setAttribute( Qt::WA_TransparentForMouseEvents );
    setAttribute(Qt::WA_TranslucentBackground, true);

    _scale = 1.;
    const int w = int(720 * _scale);
    const int h = int(220 * _scale);
    _pixmap = QPixmap(w, h);
    _pixmap.fill(Qt::transparent);
    {
        QPainter gp(&_pixmap);
        gp.setRenderHint(QPainter::Antialiasing, true);
        gp.setBrush( QColor(20, 22, 28) );
        gp.setPen( QPen(QColor(200, 176, 122), 3) );
        gp.drawRoundedRect( QRectF(1, 1, w - 2, h - 2), 16, 16 );
        gp.setBrush( QColor(232, 217, 168) );
        gp.setPen(Qt::NoPen);
        gp.drawRoundedRect( QRectF(48, 54, 18, 80), 4, 4 );
        gp.drawRoundedRect( QRectF(17, 85, 80, 18), 4, 4 );
        gp.setPen( QColor(232, 217, 168) );
        QFont title = gp.font();
        title.setPointSize(18);
        title.setBold(true);
        gp.setFont(title);
        gp.drawText( QPoint(120, 88), QString::fromUtf8(NATRON_APPLICATION_NAME) );
    }

    resize( _pixmap.width(), _pixmap.height() );
    show();

    QScreen* desktop = QGuiApplication::primaryScreen();
    QRect screen = desktop->availableGeometry();
    move(screen.width() / 2 - width() / 2, screen.height() / 2 - height() / 2);
}

void
LoadProjectSplashScreen::updateText(const QString & text)
{
    _text = text;
    update();
    {
#ifdef DEBUG
        boost_adaptbx::floating_point::exception_trapping trap(0);
#endif
        QCoreApplication::processEvents();
    }
}

void
LoadProjectSplashScreen::paintEvent(QPaintEvent* /*e*/)
{
    QStyleOption opt;

    opt.init(this);
    QPainter p(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.drawPixmap(0, 0, _pixmap);
    p.setPen(Qt::white);
    p.drawText(QPointF(120 * _scale, _pixmap.height() - 28 * _scale), _text);

    QString loadString( tr("Loading ") );
    QFontMetrics fm = p.fontMetrics();
    QPointF loadStrPos(120 * _scale, _pixmap.height() / 2. + 18 * _scale);
    p.drawText(QPointF(loadStrPos.x() + (fm.horizontalAdvance(loadString) + 5) * _scale, loadStrPos.y()), _projectName);
    p.setPen( QColor(200, 176, 122) );
    p.drawText(loadStrPos, loadString);
}

NATRON_NAMESPACE_EXIT

NATRON_NAMESPACE_USING
#include "moc_SplashScreen.cpp"
