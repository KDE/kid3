/**
 * \file qmlimageprovider.cpp
 * Image provider to get images from QML code.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Jun 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "qmlimageprovider.h"

/**
 * Constructor.
 * @param iconProvider icon provider to use
 */
QmlImageProvider::QmlImageProvider(TaggedFileIconProvider* iconProvider) :
  QQuickImageProvider(QQuickImageProvider::Pixmap), PixmapProvider(iconProvider)
{
}

/**
 * Request a pixmap.
 * @param id ID of pixmap to get, "image://kid3/fileicon/..." or
 *  "image://kid3/data..."
 * @param size the original size of the image is returned here
 * @param requestedSize the size requested via the Image.sourceSize property
 * @return pixmap for ID.
 */
QPixmap QmlImageProvider::requestPixmap(const QString& id, QSize* size,
                                        const QSize& requestedSize)
{
  return getPixmap(id, size, requestedSize);
}
