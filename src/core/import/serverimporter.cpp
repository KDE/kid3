/**
 * \file serverimporter.cpp
 * Generic baseclass to import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2024  Urs Fleisch
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
#include <QRegularExpression>
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
    m_albumListModel(new AlbumListModel(this)),
    m_trackDataModel(trackDataModel), m_standardTagsEnabled(true),
    m_additionalTagsEnabled(false), m_coverArtEnabled(false)
{
  setObjectName(QLatin1String("ServerImporter"));
}

/** NULL-terminated array of server strings, 0 if not used */
const char** ServerImporter::serverList() const { return nullptr; }

/** default server, 0 to disable */
const char* ServerImporter::defaultServer() const { return nullptr; }

/** default CGI path, 0 to disable */
const char* ServerImporter::defaultCgiPath() const { return nullptr; }

/** anchor to online help, 0 to disable */
const char* ServerImporter::helpAnchor() const { return nullptr; }

/** configuration, 0 if not used */
ServerImporterConfig* ServerImporter::config() const { return nullptr; }

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

  QRegularExpression numEntityRe(QLatin1String("&#(x?\\d+);"));
  auto it = numEntityRe.globalMatch(str);
  int numCharsRemoved = 0;
  while (it.hasNext()) {
    auto match = it.next();
    QString codeStr = match.captured(1);
    int code = codeStr.startsWith(QLatin1Char('x'))
        ? codeStr.mid(1).toInt(nullptr, 16) : codeStr.toInt();
    int pos = match.capturedStart() - numCharsRemoved;
    int len = match.capturedLength();
    str.replace(pos, len, QChar(code));
    numCharsRemoved += len - 1;
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
  QRegularExpression htmlTagRe(QLatin1String("<[^>]+>"));
  return replaceHtmlEntities(str.remove(htmlTagRe)).trimmed();
}


AlbumListModel::AlbumListModel(QObject* parent)
  : StandardTableModel(parent)
{
}

AlbumListModel::~AlbumListModel() = default;

/**
 * Get an album item.
 * @param row model row
 * @param text the text is returned here
 * @param category the category is returned here
 * @param id the internal ID is returned here
 */
void AlbumListModel::getItem(int row, QString& text,
                             QString& category, QString& id) const
{
  if (row < rowCount()) {
    QModelIndex idx = index(row, 0);
    text = idx.data().toString();
    category = idx.data(Qt::UserRole).toString();
    id = idx.data(Qt::UserRole + 1).toString();
  }
}

/**
 * Append an album item.
 * @param text display test
 * @param category category, e.g. "release"
 * @param id internal ID
 */
void AlbumListModel::appendItem(const QString& text,
                                const QString& category, const QString& id)
{
  if (int row = rowCount(); insertRow(row)) {
    QModelIndex idx = index(row, 0);
    setData(idx, text);
    setData(idx, category, Qt::UserRole);
    setData(idx, id, Qt::UserRole + 1);
  }
}
