/**
 * \file coretaggedfileiconprovider.cpp
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

#include "coretaggedfileiconprovider.h"
#include "taggedfile.h"
#include "tagconfig.h"

/**
 * Constructor.
 */
CoreTaggedFileIconProvider::CoreTaggedFileIconProvider()
{
}

/**
 * Set icon to be used for modified files.
 * @param icon modified icon
 */
void CoreTaggedFileIconProvider::setModifiedIcon(const QVariant& icon) {
  Q_UNUSED(icon)
}

/**
 * Set the requested size for icons.
 *
 * The size set with this method will be used to create icons.
 *
 * @param size icon size, the default is 16x16.
 */
void CoreTaggedFileIconProvider::setRequestedSize(const QSize& size)
{
  Q_UNUSED(size)
}

/**
 * Get an icon for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return icon for tagged file
 */
QVariant CoreTaggedFileIconProvider::iconForTaggedFile(const TaggedFile* taggedFile)
{
  Q_UNUSED(taggedFile)
  return QVariant();
}

/**
 * Get an icon ID for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return icon ID for tagged file
 */
QByteArray CoreTaggedFileIconProvider::iconIdForTaggedFile(
    const TaggedFile* taggedFile) const
{
  if (taggedFile) {
    if (taggedFile->isChanged()) {
      return "modified";
    } else {
      if (!taggedFile->isTagInformationRead())
        return "null";

      QByteArray id;
      if (taggedFile->hasTag(Frame::Tag_1))
        id += "v1";
      if (taggedFile->hasTag(Frame::Tag_2))
        id += "v2";
      if (taggedFile->hasTag(Frame::Tag_3))
        id += "v3";
      if (id.isEmpty())
        id = "notag";
      return id;
    }
  }
  return "";
}

/**
 * Get pixmap for an icon ID.
 * @param id icon ID as returned by iconIdForTaggedFile(), or data for image
 * set with setImageData()
 * @return pixmap for @a id.
 */
QVariant CoreTaggedFileIconProvider::pixmapForIconId(const QByteArray& id)
{
  Q_UNUSED(id)
  return QVariant();
}

/**
 * Get background color for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return background color for tagged file, invalid color if background
 * should not be set
 */
QVariant CoreTaggedFileIconProvider::backgroundForTaggedFile(
    const TaggedFile* taggedFile) {
  Q_UNUSED(taggedFile)
  return QVariant();
}

/**
 * Get brush with color for a context.
 * @param context color context
 * @return brush.
 */
QVariant CoreTaggedFileIconProvider::colorForContext(ColorContext context) const
{
  switch (context) {
  case ColorContext::None:
    break;
  case ColorContext::Marked:
    return QLatin1String("*");
  case ColorContext::Error:
    return QLatin1String("E");
  }
  return QVariant();
}

/**
 * Get context for a brush.
 * @param color brush
 * @return color context.
 */
ColorContext CoreTaggedFileIconProvider::contextForColor(const QVariant& color) const
{
  QString code = color.toString();
  if (code == QLatin1String("E")) {
    return ColorContext::Error;
  } else if (code == QLatin1String("*")) {
    return ColorContext::Marked;
  }
  return ColorContext::None;
}
