/**
 * \file taggedfileiconprovider.h
 * Provides icons for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jul 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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

#include <QMap>
#include <QVariant>
#include <QSize>
#include "coretaggedfileiconprovider.h"

/**
 * Provides icons for tagged files.
 */
class KID3_GUI_EXPORT TaggedFileIconProvider :
    public CoreTaggedFileIconProvider {
public:
  /**
   * Constructor.
   */
  TaggedFileIconProvider();

  /**
   * Destructor.
   */
  virtual ~TaggedFileIconProvider() override = default;

  /**
   * Set icon to be used for modified files.
   * @param icon modified icon
   */
  virtual void setModifiedIcon(const QVariant& icon) override;

  /**
   * Set the requested size for icons.
   *
   * The size set with this method will be used to create icons.
   *
   * @param size icon size, the default is 16x16.
   */
  virtual void setRequestedSize(const QSize& size) override;

  /**
   * Get an icon for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return icon for tagged file
   */
  virtual QVariant iconForTaggedFile(const TaggedFile* taggedFile) override;

  /**
   * Get pixmap for an icon ID.
   * @param id icon ID as returned by iconIdForTaggedFile(), or data for image
   * set with setImageData()
   * @return pixmap for @a id.
   */
  virtual QVariant pixmapForIconId(const QByteArray& id) override;

  /**
   * Get background color for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return background color for tagged file
   */
  virtual QVariant backgroundForTaggedFile(const TaggedFile* taggedFile) override;

private:
  void createIcons();

  QMap<QByteArray, QVariant> m_iconMap;
  QMap<QByteArray, QVariant> m_pixmapMap;
  QSize m_requestedSize;
  QVariant m_modifiedIcon;
};
