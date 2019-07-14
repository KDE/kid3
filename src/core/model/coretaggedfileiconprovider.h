/**
 * \file coretaggedfileiconprovider.h
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

#include "kid3api.h"

class QVariant;
class QSize;
class TaggedFile;

/**
 * Contextual colors.
 */
enum class ColorContext {
  None,
  Marked,
  Error
};

/**
 * Provides icons for tagged files.
 */
class KID3_CORE_EXPORT CoreTaggedFileIconProvider {
public:
  /**
   * Constructor.
   */
  CoreTaggedFileIconProvider();

  /**
   * Destructor.
   */
  virtual ~CoreTaggedFileIconProvider() = default;

  /**
   * Set icon to be used for modified files.
   * @param icon modified icon
   */
  virtual void setModifiedIcon(const QVariant& icon);

  /**
   * Set the requested size for icons.
   *
   * The size set with this method will be used to create icons.
   *
   * @param size icon size, the default is 16x16.
   */
  virtual void setRequestedSize(const QSize& size);

  /**
   * Get an icon for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return icon for tagged file
   */
  virtual QVariant iconForTaggedFile(const TaggedFile* taggedFile);

  /**
   * Get an icon ID for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return icon ID for tagged file
   */
  virtual QByteArray iconIdForTaggedFile(const TaggedFile* taggedFile) const;

  /**
   * Get pixmap for an icon ID.
   * @param id icon ID as returned by iconIdForTaggedFile(), or data for image
   * set with setImageData()
   * @return pixmap for @a id.
   */
  virtual QVariant pixmapForIconId(const QByteArray& id);

  /**
   * Get background color for a tagged file.
   *
   * @param taggedFile tagged file
   *
   * @return background color for tagged file
   */
  virtual QVariant backgroundForTaggedFile(const TaggedFile* taggedFile);

  /**
   * Get brush with color for a context.
   * @param context color context
   * @return brush.
   */
  virtual QVariant colorForContext(ColorContext context) const;

  /**
   * Get context for a brush.
   * @param color brush
   * @return color context.
   */
  virtual ColorContext contextForColor(const QVariant& color) const;
};
