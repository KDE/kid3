/**
 * \file serverimporter.cpp
 * Generic baseclass to import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2013  Urs Fleisch
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

#include "serverimporter.h"
#include <QStandardItemModel>
#include "serverimporterconfig.h"
#include "importclient.h"
#include "trackdata.h"

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
ServerImporter::ServerImporter(QNetworkAccessManager* netMgr,
                               TrackDataModel* trackDataModel)
  : ImportClient(netMgr),
    m_albumListModel(new QStandardItemModel(this)),
    m_trackDataModel(trackDataModel), m_standardTagsEnabled(true),
    m_additionalTagsEnabled(false), m_coverArtEnabled(false)
{
  setObjectName(QLatin1String("ServerImporter"));
}

/**
 * Destructor.
 */
ServerImporter::~ServerImporter()
{
}

/** NULL-terminated array of server strings, 0 if not used */
const char** ServerImporter::serverList() const { return 0; }

/** default server, 0 to disable */
const char* ServerImporter::defaultServer() const { return 0; }

/** default CGI path, 0 to disable */
const char* ServerImporter::defaultCgiPath() const { return 0; }

/** anchor to online help, 0 to disable */
const char* ServerImporter::helpAnchor() const { return 0; }

/** configuration, 0 if not used */
ServerImporterConfig* ServerImporter::config() const { return 0; }

/** additional tags option, false if not used */
bool ServerImporter::additionalTags() const { return false; }

/**
 * Clear model data.
 */
void ServerImporter::clear()
{
  m_albumListModel->clear();
}

/**
 * Replace HTML entities in a string.
 *
 * @param str string with HTML entities (e.g. &quot;)
 *
 * @return string with replaced HTML entities.
 */
QString ServerImporter::replaceHtmlEntities(QString str)
{
  str.replace(QLatin1String("&quot;"), QLatin1String("\""));
  str.replace(QLatin1String("&nbsp;"), QLatin1String(" "));
  str.replace(QLatin1String("&lt;"), QLatin1String("<"));
  str.replace(QLatin1String("&gt;"), QLatin1String(">"));
  str.replace(QLatin1String("&amp;"), QLatin1String("&"));
  str.replace(QLatin1String("&times;"), QString(QChar(0xd7)));
  str.replace(QLatin1String("&ndash;"), QLatin1String("-"));

  QRegExp numEntityRe(QLatin1String("&#(\\d+);"));
  int pos = 0;
  while ((pos = numEntityRe.indexIn(str, pos)) != -1) {
    str.replace(pos, numEntityRe.matchedLength(),
                QChar(numEntityRe.cap(1).toInt()));
    ++pos;
  }
  return str;
}

/**
 * Replace HTML entities and remove HTML tags.
 *
 * @param str string containing HTML
 *
 * @return clean up string
 */
QString ServerImporter::removeHtml(QString str)
{
  QRegExp htmlTagRe(QLatin1String("<[^>]+>"));
  return replaceHtmlEntities(str.remove(htmlTagRe)).trimmed();
}


/**
 * Constructor.
 * @param text    title
 * @param cat     category
 * @param idStr   ID
 */
AlbumListItem::AlbumListItem(const QString& text,
        const QString& cat, const QString& idStr) :
  QStandardItem(text)
{
  setData(cat, Qt::UserRole + 1);
  setData(idStr, Qt::UserRole + 2);
}

/**
 * Destructor.
 */
AlbumListItem::~AlbumListItem()
{
}

/**
 * Get type of item.
 * Used to distinguish items of this custom type from base class items.
 * @return AlbumListItem::Type.
 */
int AlbumListItem::type() const
{
  return Type;
}

/**
 * Get category.
 * @return category.
 */
QString AlbumListItem::getCategory() const
{
  return data(Qt::UserRole + 1).toString();
}

/**
 * Get ID.
 * @return ID.
 */
QString AlbumListItem::getId() const
{
  return data(Qt::UserRole + 2).toString();
}

#ifndef QT_NO_DEBUG
/**
 * Dump an album list.
 * @param albumModel album list model
 */
void AlbumListItem::dumpAlbumList(const QStandardItemModel* albumModel)
{
  for (int row = 0; row < albumModel->rowCount(); ++row) {
    AlbumListItem* item = static_cast<AlbumListItem*>(albumModel->item(row, 0));
    if (item && item->type() == AlbumListItem::Type) {
      qDebug("%s (%s, %s)", qPrintable(item->text()),
             qPrintable(item->getCategory()), qPrintable(item->getId()));
    }
  }
}
#endif
