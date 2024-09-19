/**
 * \file freedbimporter.cpp
 * freedb.org importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 *
 * Copyright (C) 2004-2024  Urs Fleisch
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

#include "freedbimporter.h"
#include <QRegularExpression>
#include "serverimporterconfig.h"
#include "trackdatamodel.h"
#include "freedbconfig.h"
#include "config.h"
#include "genres.h"

namespace {

void setUserEmail(const ServerImporterConfig* cfg,
                  QMap<QByteArray, QByteArray>& headers, QString& nameHost)
{
  if (cfg) {
    if (QByteArray token = cfg->property("token").toByteArray();
        token.contains('@')) {
      headers["User-Email"] = token;
      nameHost = QString::fromLatin1(token);
      nameHost.replace(QLatin1Char('@'), QLatin1Char('+'));
    } else {
      headers.remove("User-Email");
      nameHost = QLatin1String("noname+localhost");
    }
  }
}

}

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
FreedbImporter::FreedbImporter(QNetworkAccessManager* netMgr,
                               TrackDataModel *trackDataModel)
  : ServerImporter(netMgr, trackDataModel)
{
  setObjectName(QLatin1String("FreedbImporter"));
}

/**
 * Name of import source.
 * @return name.
 */
const char* FreedbImporter::name() const { return QT_TRANSLATE_NOOP("@default", "gnudb.org"); }

/** NULL-terminated array of server strings, 0 if not used */
const char** FreedbImporter::serverList() const
{
  static const char* servers[] = {
    "gnudb.gnudb.org:80",
    nullptr      // end of StrList
  };
  return servers;
}

/** default server, 0 to disable */
const char* FreedbImporter::defaultServer() const { return "gnudb.gnudb.org:80"; }

/** default CGI path, 0 to disable */
const char* FreedbImporter::defaultCgiPath() const { return "/~cddb/cddb.cgi"; }

/** anchor to online help, 0 to disable */
const char* FreedbImporter::helpAnchor() const { return "import-freedb"; }

/** configuration, 0 if not used */
ServerImporterConfig* FreedbImporter::config() const { return &FreedbConfig::instance(); }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void FreedbImporter::parseFindResults(const QByteArray& searchStr)
{
/*
200 Found 1 matches, list follows (until terminating `.')\r\ndata 920b8189 Catharsis / Imago\r\n.\r\n
  or
202 No match found.\r\n
*/
  QString str = QString::fromUtf8(searchStr);
  QRegularExpression catIdTitleRe(QLatin1String(R"(^([a-z]+)\s+([0-9a-f]+)\s+(.*)$)"));
  QStringList lines = str.split(QRegularExpression(QLatin1String("[\\r\\n]+")));
  bool inEntries = false;
  m_albumListModel->clear();
  for (auto it = lines.constBegin(); it != lines.constEnd(); ++it) {
    if (inEntries) {
      if (*it == QLatin1String(".")) {
        break;
      }
      auto match = catIdTitleRe.match(*it);
      if (match.hasMatch()) {
        m_albumListModel->appendItem(
          match.captured(3),
          match.captured(1),
          match.captured(2));
      }
    } else if (it->startsWith(QLatin1String("200 Found"))) {
      inEntries = true;
    }
  }
}

namespace {

/**
 * Parse the track durations from freedb.org.
 *
 * @param text          text buffer containing data from freedb.org
 * @param trackDuration list for results
 */
void parseFreedbTrackDurations(
  const QString& text,
  QList<int>& trackDuration)
{
/* Example freedb format:
   # Track frame offsets:
   # 150
   # 2390
   # 23387
   # 44650
   # 61322
   # 94605
   # 121710
   # 144637
   # 176820
   # 187832
   # 218930
   #
   # Disc length: 3114 seconds
*/
  trackDuration.clear();
  QRegularExpression discLenRe(QLatin1String("Disc length:\\s*\\d+"));
if (auto match = discLenRe.match(text); match.hasMatch()) {
    int discLenPos = match.capturedStart();
    int len = match.capturedLength();
    discLenPos += 12;
#if QT_VERSION >= 0x060000
    int discLen = text.mid(discLenPos, len - 12).toInt();
#else
    int discLen = text.midRef(discLenPos, len - 12).toInt();
#endif
    if (int trackOffsetPos = text.indexOf(QLatin1String("Track frame offsets"), 0);
        trackOffsetPos != -1) {
      int lastOffset = -1;
      QRegularExpression re(QLatin1String("#\\s*\\d+"));
      auto it = re.globalMatch(text);
      while (it.hasNext()) {
        auto toMatch = it.next();
        trackOffsetPos = toMatch.capturedStart();
        if (trackOffsetPos >= discLenPos) {
          break;
        }
        len = toMatch.capturedLength();
        trackOffsetPos += 1;
#if QT_VERSION >= 0x060000
        int trackOffset = text.mid(trackOffsetPos, len - 1).toInt();
#else
        int trackOffset = text.midRef(trackOffsetPos, len - 1).toInt();
#endif
        if (lastOffset != -1) {
          int duration = (trackOffset - lastOffset) / 75;
          trackDuration.append(duration);
        }
        lastOffset = trackOffset;
      }
      if (lastOffset != -1) {
        int duration = (discLen * 75 - lastOffset) / 75;
        trackDuration.append(duration);
      }
    }
  }
}

/**
 * Parse the album specific data (artist, album, year, genre) from freedb.org.
 *
 * @param text text buffer containing data from freedb.org
 * @param frames tags to put result
 */
void parseFreedbAlbumData(const QString& text, FrameCollection& frames)
{
  QRegularExpression fdre(QLatin1String(R"(DTITLE=\s*(\S[^\r\n]*\S)\s*/\s*(\S[^\r\n]*\S)[\r\n])"));
  auto match = fdre.match(text);
  if (match.hasMatch()) {
    frames.setArtist(match.captured(1));
    frames.setAlbum(match.captured(2));
  }
  fdre.setPattern(QLatin1String(R"(EXTD=[^\r\n]*YEAR:\s*(\d+)\D)"));
  match = fdre.match(text);
  if (match.hasMatch()) {
    frames.setYear(match.captured(1).toInt());
  }
  fdre.setPattern(QLatin1String(R"(EXTD=[^\r\n]*ID3G:\s*(\d+)\D)"));
  match = fdre.match(text);
  if (match.hasMatch()) {
    frames.setGenre(QString::fromLatin1(Genres::getName(match.captured(1).toInt())));
  }
  fdre.setPattern(QLatin1String(R"(DYEAR=(\d+))"));
  match = fdre.match(text);
  if (match.hasMatch()) {
    frames.setYear(match.captured(1).toInt());
  }
  fdre.setPattern(QLatin1String(R"(DGENRE=([^\r\n]+))"));
  match = fdre.match(text);
  if (match.hasMatch()) {
    frames.setGenre(match.captured(1));
  }
}

}

/**
 * Parse result of album request and populate m_trackDataModel with results.
 *
 * @param albumStr album data received
 */
void FreedbImporter::parseAlbumResults(const QByteArray& albumStr)
{
  QString text = QString::fromUtf8(albumStr);
  FrameCollection framesHdr;
  QList<int> trackDuration;
  parseFreedbTrackDurations(text, trackDuration);
  parseFreedbAlbumData(text, framesHdr);

  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  trackDataVector.setCoverArtUrl(QUrl());
  FrameCollection frames(framesHdr);
  auto it = trackDataVector.begin();
  auto tdit = trackDuration.constBegin();
  bool atTrackDataListEnd = it == trackDataVector.end();
  int tracknr = 0;
  for (;;) {
    QRegularExpression fdre(QString(QLatin1String(R"(TTITLE%1=([^\r\n]+)[\r\n])")).arg(tracknr));
    QString title;
    auto fdIt = fdre.globalMatch(text);
    while (fdIt.hasNext()) {
      auto match = fdIt.next();
      title += match.captured(1);
    }
    if (!title.isNull()) {
      frames.setTrack(tracknr + 1);
      frames.setTitle(title);
    } else {
      break;
    }
    int duration = tdit != trackDuration.constEnd() ?
      *tdit++ : 0;
    if (atTrackDataListEnd) {
      ImportTrackData trackData;
      trackData.setFrameCollection(frames);
      trackData.setImportDuration(duration);
      trackDataVector.push_back(trackData);
    } else {
      while (!atTrackDataListEnd && !it->isEnabled()) {
        ++it;
        atTrackDataListEnd = it == trackDataVector.end();
      }
      if (!atTrackDataListEnd) {
        it->setFrameCollection(frames);
        it->setImportDuration(duration);
        ++it;
        atTrackDataListEnd = it == trackDataVector.end();
      }
    }
    frames = framesHdr;
    ++tracknr;
  }
  frames.clear();
  while (!atTrackDataListEnd) {
    if (it->isEnabled()) {
      if (it->getFileDuration() == 0) {
        it = trackDataVector.erase(it);
      } else {
        it->setFrameCollection(frames);
        it->setImportDuration(0);
        ++it;
      }
    } else {
      ++it;
    }
    atTrackDataListEnd = it == trackDataVector.end();
  }
  m_trackDataModel->setTrackData(trackDataVector);
}

/**
 * Send a query command in to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void FreedbImporter::sendFindQuery(
  const ServerImporterConfig* cfg,
  const QString& artist, const QString& album)
{
  // If an URL is entered in the first search field, its result will be directly
  // available in the album results list.
  QRegularExpression gnudbUrlRe(QLatin1String(R"(^https://gnudb.org/([a-z]+)/([0-9a-f]+)$)"));
  auto match = gnudbUrlRe.match(artist);
  if (match.hasMatch()) {
    m_albumListModel->clear();
    m_albumListModel->appendItem(artist, match.captured(1), match.captured(2));
    return;
  }

  QMap<QByteArray, QByteArray> headers;
  QString nameHost;
  setUserEmail(cfg, headers, nameHost);
  sendRequest(cfg->server(),
              cfg->cgiPath() + QLatin1String("?cmd=search&artist=") +
              encodeUrlQuery(artist) + QLatin1String("&album=") +
              encodeUrlQuery(album) +
              QLatin1String("&hello=") + nameHost +
              QLatin1String("+Kid3+" VERSION "&proto=6"),
              QLatin1String("http"), headers);
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void FreedbImporter::sendTrackListQuery(
  const ServerImporterConfig* cfg, const QString& cat, const QString& id)
{
  QMap<QByteArray, QByteArray> headers;
  QString nameHost;
  setUserEmail(cfg, headers, nameHost);
  sendRequest(cfg->server(),
              cfg->cgiPath() + QLatin1String("?cmd=cddb+read+") + cat + QLatin1Char('+') + id +
              QLatin1String("&hello=") + nameHost +
              QLatin1String("+Kid3+" VERSION "&proto=6"),
              QLatin1String("http"), headers);
}
