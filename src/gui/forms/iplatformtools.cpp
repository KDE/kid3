/**
 * \file iplatformtools.cpp
 * Interface for platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iplatformtools.h"
#include <QGuiApplication>
#include <QClipboard>
#include "config.h"
#include "audioplayer.h"
#ifdef HAVE_QTDBUS
#include "mprisinterface.h"
#endif

/**
 * Destructor.
 */
IPlatformTools::~IPlatformTools()
{
}

/**
 * Write text to clipboard.
 * @param text text to write
 * @return true if operation is supported.
 */
bool IPlatformTools::writeToClipboard(const QString& text) const
{
  QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
  return true;
}

/**
 * Read text from clipboard.
 * @return text, null if operation not supported.
 */
QString IPlatformTools::readFromClipboard() const
{
  QClipboard* cb = QGuiApplication::clipboard();
  QString text = cb->text(QClipboard::Clipboard);
  if (text.isNull())
    text = cb->text(QClipboard::Selection);
  return text;
}

/**
 * Create an audio player instance.
 * @param app application context
 * @param dbusEnabled true to enable MPRIS D-Bus interface
 * @return audio player, nullptr if not supported.
 */
QObject* IPlatformTools::createAudioPlayer(Kid3Application* app,
                                           bool dbusEnabled) const
{
  AudioPlayer* player = new AudioPlayer(app);
#ifdef HAVE_QTDBUS
  if (dbusEnabled) {
    new MprisInterface(player);
    new MprisPlayerInterface(player);
  }
#endif
  return player;
}
