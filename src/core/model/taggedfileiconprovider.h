/**
 * \file taggedfileiconprovider.h
 * Provides icons for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29-Mar-2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#pragma once

#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QMap>
#include "kid3api.h"

class TaggedFile;

/**
 * Provides icons for tagged files.
 */
class KID3_CORE_EXPORT TaggedFileIconProvider {
public:
  /**
   * Constructor.
   */
  TaggedFileIconProvider();

  /**
   * Set the requested size for icons.
   *
   * The size set with this method will be used to create icons.
   *
   * @param size icon size, the default is 16x16.
   */
  void setRequestedSize(const QSize& size);

  /**
   * Get an icon for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return icon for tagged file
   */
  QIcon iconForTaggedFile(const TaggedFile* taggedFile);

  /**
   * Get an icon ID for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return icon ID for tagged file
   */
  QByteArray iconIdForTaggedFile(const TaggedFile* taggedFile) const;

  /**
   * Get pixmap for an icon ID.
   * @param id icon ID as returned by iconIdForTaggedFile(), or data for image
   * set with setImageData()
   * @return pixmap for @a id.
   */
  QPixmap pixmapForIconId(const QByteArray& id);

  /**
   * Get background color for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return background color for tagged file
   */
  QColor backgroundForTaggedFile(const TaggedFile* taggedFile);

private:
  void createIcons();

  QMap<QByteArray, QIcon> m_iconMap;
  QMap<QByteArray, QPixmap> m_pixmapMap;
  QSize m_requestedSize;
};
