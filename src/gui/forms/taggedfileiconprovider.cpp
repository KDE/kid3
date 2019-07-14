/**
 * \file taggedfileiconprovider.cpp
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

#include "taggedfileiconprovider.h"
#include <QGuiApplication>
#include <QPalette>
#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QPainter>
#include "taggedfile.h"
#include "tagconfig.h"

/**
 * Constructor.
 */
TaggedFileIconProvider::TaggedFileIconProvider()
  : m_requestedSize(16, 16)
{
}

/**
 * Set icon to be used for modified files.
 * @param icon modified icon
 */
void TaggedFileIconProvider::setModifiedIcon(const QVariant& icon) {
  m_modifiedIcon = icon;
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
    m_iconMap.insert(it.key(), QIcon(it.value().value<QPixmap>()));
  }

  if (!m_modifiedIcon.isNull()) {
    m_iconMap.insert("modified", m_modifiedIcon);
    m_pixmapMap.insert("modified",
                       m_modifiedIcon.value<QIcon>().pixmap(m_requestedSize));
  }
}

/**
 * Get an icon for a tagged file.
 *
 * @param taggedFile tagged file
 *
 * @return icon for tagged file
 */
QVariant TaggedFileIconProvider::iconForTaggedFile(const TaggedFile* taggedFile)
{
  if (taggedFile) {
    if (m_iconMap.isEmpty()) {
      createIcons();
    }
    return m_iconMap.value(iconIdForTaggedFile(taggedFile));
  }
  return QVariant();
}

/**
 * Get pixmap for an icon ID.
 * @param id icon ID as returned by iconIdForTaggedFile(), or data for image
 * set with setImageData()
 * @return pixmap for @a id.
 */
QVariant TaggedFileIconProvider::pixmapForIconId(const QByteArray& id)
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
QVariant TaggedFileIconProvider::backgroundForTaggedFile(
    const TaggedFile* taggedFile) {
  if (taggedFile &&
      ((TagConfig::instance().markTruncations() &&
        taggedFile->getTruncationFlags(Frame::Tag_Id3v1) != 0) ||
       taggedFile->isMarked()))
    return QColor(Qt::red);
  return QVariant();
}

/**
 * Get brush with color for a context.
 * @param context color context
 * @return brush.
 */
QVariant TaggedFileIconProvider::colorForContext(ColorContext context) const
{
  const bool isGuiApp =
      qobject_cast<QGuiApplication*>(QCoreApplication::instance()) != nullptr;
  switch (context) {
  case ColorContext::None:
    break;
  case ColorContext::Marked:
    return isGuiApp ? QGuiApplication::palette().mid()
                    : QBrush(Qt::gray);
  case ColorContext::Error:
    return QBrush(Qt::red);
  }
  return QBrush(Qt::NoBrush);
}

/**
 * Get context for a brush.
 * @param color brush
 * @return color context.
 */
ColorContext TaggedFileIconProvider::contextForColor(const QVariant& color) const
{
  if (color.type() == QVariant::Brush) {
    QBrush brush = color.value<QBrush>();
    if (brush == Qt::red) {
      return ColorContext::Error;
    } else if (brush != Qt::NoBrush) {
      return ColorContext::Marked;
    }
  }
  return ColorContext::None;
}
