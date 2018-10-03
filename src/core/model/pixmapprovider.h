/**
 * \file pixmapprovider.h
 * Image provider to get pixmaps by ID.
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

#ifndef PIXMAPPROVIDER_H
#define PIXMAPPROVIDER_H

#include <QByteArray>
#include <QPixmap>
#include "kid3api.h"

class TaggedFileIconProvider;

/**
 * Image provider to get pixmaps by ID.
 *
 * The following source IDs are supported (starting with "image://kid3/"):
 * - "fileicon/" followed by "null", "notag", "v1", "v2", "v1v2", or "modified",
 * - "data" followed by a changing string to force loading of the image set with
 *   TaggedFileIconProvider::setImageData().
 */
class KID3_CORE_EXPORT PixmapProvider {
public:
  /**
   * Constructor.
   * @param iconProvider icon provider to use
   */
  explicit PixmapProvider(TaggedFileIconProvider* iconProvider);

  /**
   * Destructor.
   */
  ~PixmapProvider() = default;

  /**
   * Request a pixmap.
   * @param id ID of pixmap to get, "image://kid3/fileicon/..." or
   *  "image://kid3/data..."
   * @param size the original size of the image is returned here
   * @param requestedSize the size requested via the Image.sourceSize property
   * @return pixmap for ID.
   */
  QPixmap getPixmap(const QString& id, QSize* size, const QSize& requestedSize);

  /**
   * Get image data for the pixmap available via the "data" icon ID.
   * @return image data.
   */
  QByteArray getImageData() const { return m_data; }

  /**
   * Set image data for the pixmap available via the "data" icon ID.
   * @param data image data
   */
  void setImageData(const QByteArray& data) {
    m_data = data;
  }

private:
  TaggedFileIconProvider* m_fileIconProvider;
  QByteArray m_data;
  QPixmap m_dataPixmap;
  uint m_pixmapHash;
};

#endif // PIXMAPPROVIDER_H
