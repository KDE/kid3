/**
 * \file picturelabel.cpp
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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

#include "picturelabel.h"
#include <QHash>
#include <QByteArray>
#include <QPixmap>
#include <QCoreApplication>

/**
 * Constructor.
 *
 * @param parent parent widget
 */
PictureLabel::PictureLabel(QWidget* parent) : QLabel(parent), m_pixmapHash(0)
{
  setObjectName(QLatin1String("PictureLabel"));
  setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  setWordWrap(true);
  clearPicture();
}

/**
 * Destructor.
 */
PictureLabel::~PictureLabel()
{
}

/**
 * Get preferred height for a given width.
 * @return height.
 */
int PictureLabel::heightForWidth(int w) const
{
  return w;
}

/**
 * Clear picture.
 */
void PictureLabel::clearPicture()
{
  const char* const msg = QT_TRANSLATE_NOOP("@default", "Drag album\nartwork\nhere");
  setText(QCoreApplication::translate("@default", msg));
}

/**
 * Set picture data.
 *
 * @param data picture data, empty if no picture is available
 */
void PictureLabel::setData(const QByteArray& data)
{
  if (!data.isEmpty()) {
    uint hash = qHash(data);
    if (pixmap() && hash == m_pixmapHash)
      return; // keep existing pixmap

    // creating new pixmap
    QPixmap pm;
    if (pm.loadFromData(data)) {
      QPixmap scaledPm = pm.scaled(width(), height(), Qt::KeepAspectRatio);
      if (!scaledPm.isNull()) {
        m_pixmapHash = hash;
        setContentsMargins(0, 0, 0, 0);
        setPixmap(scaledPm);
        return;
      }
    }
  }

  clearPicture();
}
