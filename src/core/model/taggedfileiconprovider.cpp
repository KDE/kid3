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
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include "taggedfile.h"
#include "tagconfig.h"

/**
 * Constructor.
 */
TaggedFileIconProvider::TaggedFileIconProvider() :
  m_requestedSize(16, 16)
{
}

/**
 * Set the requested size for icons.
 *
 * The size set with this method will be used to create icons.
 *
 * @param size icon size, the default is 16x16.
 */
void TaggedFileIconProvider::setRequestedSize(const QSize& size)
{
  if (size.isValid() && size.height() > m_requestedSize.height()) {
    m_requestedSize = size;
    m_iconMap.clear();
    m_pixmapMap.clear();
  }
}

/**
 * Create icons using requested size.
 */
void TaggedFileIconProvider::createIcons()
{
  static const struct {
    const char* id;
    const char* text1;
    const char* text2;
  } idTexts[] = {
    {"null", nullptr, nullptr},
    {"notag", "NO", "TAG" },
    {"v1v2", "V1", "V2"},
    {"v1", "V1", nullptr},
    {"v2", nullptr, "V2"},
    {"v3", nullptr, "V3"},
    {"v1v3", "V1", "V3"},
    {"v2v3", "V2", "V3"},
    {"v1v2v3", "V1", "23"}
};

  const int height = m_requestedSize.height();
  const int halfHeight = height / 2;
  QFont font(QLatin1String("helvetica"));
  font.setPixelSize(halfHeight);
  QFont smallFont(font);
  smallFont.setStretch(QFont::Condensed);
  for (const auto& it : idTexts) {
    const char* text1 = it.text1;
    const char* text2 = it.text2;

    QPixmap pixmap(m_requestedSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setFont(font);
    if (text1) {
      painter.setPen(Qt::white);
      painter.drawText(QPoint(2, halfHeight - 1), QLatin1String(text1));
      painter.setPen(Qt::black);
      painter.drawText(QPoint(3, halfHeight), QLatin1String(text1));
    }
    if (text2) {
      if (qstrlen(text2) > 2) {
        painter.setFont(smallFont);
      }
      painter.setPen(Qt::white);
      painter.drawText(QPoint(2, height - 2), QLatin1String(text2));
      painter.setPen(Qt::black);
      painter.drawText(QPoint(3, height - 1), QLatin1String(text2));
    }

    m_pixmapMap.insert(it.id, pixmap);
  }

  for (auto it = m_pixmapMap.constBegin(); it != m_pixmapMap.constEnd(); ++it) {
    m_iconMap.insert(it.key(), QIcon(it.value()));
  }

  QIcon modifiedIcon =
      QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon);
  m_iconMap.insert("modified", modifiedIcon);
  m_pixmapMap.insert("modified", modifiedIcon.pixmap(m_requestedSize));
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
    if (m_iconMap.isEmpty()) {
      createIcons();
    }
    return m_iconMap.value(iconIdForTaggedFile(taggedFile));
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
QPixmap TaggedFileIconProvider::pixmapForIconId(const QByteArray& id)
{
  if (m_pixmapMap.isEmpty()) {
    createIcons();
  }
  return m_pixmapMap.value(id);
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
      ((TagConfig::instance().markTruncations() &&
        taggedFile->getTruncationFlags(Frame::Tag_Id3v1) != 0) ||
       taggedFile->isMarked()))
    return Qt::red;
  return QColor();
}
