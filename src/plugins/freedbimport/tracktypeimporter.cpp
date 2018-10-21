/**
 * \file tracktypeimporter.cpp
 * TrackType.org importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Apr 2007
 *
 * Copyright (C) 2007-2018  Urs Fleisch
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

#include "tracktypeimporter.h"
#include "serverimporterconfig.h"
#include "freedbconfig.h"
#include "config.h"

namespace {

const char trackTypeServer[] = "tracktype.org:80";

}

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
TrackTypeImporter::TrackTypeImporter(QNetworkAccessManager* netMgr,
                                     TrackDataModel *trackDataModel)
  : FreedbImporter(netMgr, trackDataModel)
{
  setObjectName(QLatin1String("TrackTypeImporter"));
}

/**
 * Name of import source.
 * @return name.
 */
const char* TrackTypeImporter::name() const {
  return QT_TRANSLATE_NOOP("@default", "TrackType.org");
}

/** NULL-terminated array of server strings, 0 if not used */
const char** TrackTypeImporter::serverList() const
{
  static const char* servers[] = {
    "tracktype.org:80",
    nullptr            // end of StrList
  };
  return servers;
}

/** default server, 0 to disable */
const char* TrackTypeImporter::defaultServer() const { return "tracktype.org:80"; }

/** configuration, 0 if not used */
ServerImporterConfig* TrackTypeImporter::config() const { return &TrackTypeConfig::instance(); }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void TrackTypeImporter::parseFindResults(const QByteArray& searchStr)
{
/*
210 exact matches found
categ discid dtitle
(more matches...)
.
or
211 close matches found
rock 920b810c Catharsis / Imago
.
theoretically, but never seen
200 categ discid dtitle
*/
  QString str = QString::fromUtf8(searchStr);
  QRegExp catIdTitleRe(QLatin1String("([a-z]+)\\s+([0-9a-f]+)\\s+([^/]+ / .+)"));
  QStringList lines = str.split(QRegExp(QLatin1String("[\\r\\n]+")));
  bool inEntries = false;
  m_albumListModel->clear();
  for (auto it = lines.constBegin(); it != lines.constEnd(); ++it) {
    if (*it == QLatin1String(".")) {
      break;
    }
    if (inEntries) {
      if (catIdTitleRe.exactMatch(*it)) {
        m_albumListModel->appendRow(new AlbumListItem(
          catIdTitleRe.cap(3),
          catIdTitleRe.cap(1),
          catIdTitleRe.cap(2)));
      }
    } else {
      if ((*it).startsWith(QLatin1String("21")) && (*it).indexOf(QLatin1String(" match")) != -1) {
        inEntries = true;
      } else if ((*it).startsWith(QLatin1String("200 "))) {
        if (catIdTitleRe.exactMatch((*it).mid(4))) {
          m_albumListModel->appendRow(new AlbumListItem(
            catIdTitleRe.cap(3),
            catIdTitleRe.cap(1),
            catIdTitleRe.cap(2)));
        }
      }
    }
  }
}

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void TrackTypeImporter::sendFindQuery(
  const ServerImporterConfig* cfg,
  const QString& artist, const QString& album)
{
  // At the moment, only TrackType.org recognizes cddb album commands,
  // so we always use this server for find queries.
  sendRequest(QString::fromLatin1(trackTypeServer),
              cfg->cgiPath() + QLatin1String("?cmd=cddb+album+") +
              encodeUrlQuery(artist + QLatin1String(" / ") + album) +
              QLatin1String("&hello=noname+localhost+Kid3+" VERSION "&proto=6"));
}
