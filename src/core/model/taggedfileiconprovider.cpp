/**
 * \file taggedfileiconprovider.cpp
 * Provides icons for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29-Mar-2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include "taggedfileiconprovider.h"
#include <QPixmap>
#include "taggedfile.h"
#include "tagconfig.h"

/* The bitmaps are stored here instead of using KDE bitmaps to make
   it work for the Qt only versions. */
/** picture for modified pixmap */
static const char* const modified_xpm[] = {
  "16 16 33 1",
  ". c None",
  "B c None",
  "A c None",
  "C c None",
  "D c None",
  "E c None",
  "# c #000000",
  "b c #006562",
  "j c #414041",
  "x c #525552",
  "f c #529594",
  "e c #52959c",
  "w c #5a555a",
  "v c #626162",
  "u c #626562",
  "r c #737173",
  "p c #737573",
  "q c #7b757b",
  "o c #838183",
  "m c #838583",
  "z c #8b8d8b",
  "l c #949194",
  "k c #9c959c",
  "i c #a4a1a4",
  "h c #a4a5a4",
  "y c #b4b6b4",
  "g c #bdb6bd",
  "a c #c5c2c5",
  "s c #c5c6c5",
  "c c #cdc6cd",
  "t c #dedade",
  "n c #eeeaee",
  "d c #ffffff",
  ".......##.......",
  "......#ab#......",
  ".....#cbde#.....",
  "....#abdddf#....",
  "...#gbddddde#...",
  "..#hijddddddf#..",
  ".#kjkljdddddd##.",
  "#mjnjmojddddjma#",
  "#jnpnjqrjddjqs#.",
  "#drtttjuvjjua#..",
  ".#dasajjwxws#...",
  "..#dyjzljxa#...A",
  "...#jrrjws#...AB",
  "....#cjxa#...ACB",
  ".....#cs#...ADE.",
  "......##...ABB.."
};

/** picture for empty pixmap */
static const char* const null_xpm[] = {
  "16 16 2 1",
  "# c None",
  ". c None",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#.",
  ".#.#.#.#.#.#.#.#",
  "#.#.#.#.#.#.#.#."
};

/** picture with V1 and V2 */
static const char* const v1v2_xpm[] = {
  "16 16 3 1",
  "  c None",
  ". c #FFFFFF",
  "+ c #000000",
  "                ",
  "  ..  ..   .    ",
  "  .+  .+  .+    ",
  "  .+  .+ .++    ",
  "   .+.+ .+.+    ",
  "   .+.+   .+    ",
  "    .+    .+    ",
  "                ",
  "  ..  ..  ..    ",
  "  .+  .+ .++.   ",
  "  .+  .+.+ .+   ",
  "   .+.+   .+    ",
  "   .+.+  .+..   ",
  "    .+  .++++   ",
  "                ",
  "                "};

/** picture with V1 */
static const char* const v1_xpm[] = {
  "16 16 3 1",
  "  c None",
  ". c #FFFFFF",
  "+ c #000000",
  "                ",
  "  ..  ..   .    ",
  "  .+  .+  .+    ",
  "  .+  .+ .++    ",
  "   .+.+ .+.+    ",
  "   .+.+   .+    ",
  "    .+    .+    ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                "};

/** picture with V2 */
static const char* const v2_xpm[] = {
  "16 16 3 1",
  "  c None",
  ". c #FFFFFF",
  "+ c #000000",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "  ..  ..  ..    ",
  "  .+  .+ .++.   ",
  "  .+  .+.+ .+   ",
  "   .+.+   .+    ",
  "   .+.+  .+..   ",
  "    .+  .++++   ",
  "                ",
  "                "};

/** picture with NO TAG */
static const char* const notag_xpm[] = {
  "16 16 3 1",
  "  c None",
  ". c #FFFFFF",
  "+ c #000000",
  "                ",
  "  ..  ..  ..    ",
  "  .+. .+ .++.   ",
  "  .++..+.+ .+   ",
  "  .+.+.+.+ .+   ",
  "  .+ .++.+..+   ",
  "  .+  .+ .++    ",
  "                ",
  " ....  .   ..   ",
  " .+++ .+. .++   ",
  "  .+ .+.+.+ ..  ",
  "  .+ .+++.+.++  ",
  "  .+ .+.+.+..+  ",
  "  .+ .+.+ .++   ",
  "                ",
  "                "};


/**
 * Constructor.
 */
TaggedFileIconProvider::TaggedFileIconProvider() :
  m_nullPixmap((const char **)null_xpm),
  m_nullIcon(m_nullPixmap),
  m_modifiedPixmap((const char **)modified_xpm),
  m_modifiedIcon(m_modifiedPixmap),
  m_v1v2Pixmap((const char **)v1v2_xpm),
  m_v1v2Icon(m_v1v2Pixmap),
  m_v1Pixmap((const char **)v1_xpm),
  m_v1Icon(m_v1Pixmap),
  m_v2Pixmap((const char **)v2_xpm),
  m_v2Icon(m_v2Pixmap),
  m_notagPixmap((const char **)notag_xpm),
  m_notagIcon(m_notagPixmap)
{
}

/**
 * Get an icon for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return icon for tagged file
 */
QIcon TaggedFileIconProvider::iconForTaggedFile(const TaggedFile* taggedFile)
{
  if (taggedFile) {
    if (taggedFile->isChanged()) {
      return m_modifiedIcon;
    } else {
      if (!taggedFile->isTagInformationRead())
        return m_nullIcon;
      if (taggedFile->hasTagV1())
        return taggedFile->hasTagV2() ? m_v1v2Icon : m_v1Icon;
      else
        return taggedFile->hasTagV2() ? m_v2Icon : m_notagIcon;
    }
  }
  return QIcon();
}

/**
 * Get an icon ID for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return icon ID for tagged file
 */
QByteArray TaggedFileIconProvider::iconIdForTaggedFile(
    const TaggedFile* taggedFile) const
{
  if (taggedFile) {
    if (taggedFile->isChanged()) {
      return "modified";
    } else {
      if (!taggedFile->isTagInformationRead())
        return "null";
      if (taggedFile->hasTagV1())
        return taggedFile->hasTagV2() ? "v1v2" : "v1";
      else
        return taggedFile->hasTagV2() ? "v2" : "notag";
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
QPixmap TaggedFileIconProvider::pixmapForIconId(const QByteArray& id) const
{
  if (id == "null") {
    return m_nullPixmap;
  } else if (id == "v1v2") {
    return m_v1v2Pixmap;
  } else if (id == "v1") {
    return m_v1Pixmap;
  } else if (id == "v2") {
    return m_v2Pixmap;
  } else if (id == "modified") {
    return m_modifiedPixmap;
  } else if (id == "notag") {
    return m_notagPixmap;
  }
  return QPixmap();
}

/**
 * Get background color for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return background color for tagged file, invalid color if background
 * should not be set
 */
QColor TaggedFileIconProvider::backgroundForTaggedFile(
    const TaggedFile* taggedFile) {
  if (taggedFile &&
      TagConfig::instance().markTruncations() &&
      taggedFile->getTruncationFlags() != 0)
    return Qt::red;
  return QColor();
}
