/**
 * \file freedbimporter.cpp
 * freedb.org importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 *
 * Copyright (C) 2004-2013  Urs Fleisch
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
#include "serverimporterconfig.h"
#include "trackdatamodel.h"
#include "configstore.h"
#include "genres.h"

static const char gnudbServer[] = "www.gnudb.org:80";

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
FreedbImporter::FreedbImporter(QNetworkAccessManager* netMgr,
                               TrackDataModel *trackDataModel) :
  ServerImporter(netMgr, trackDataModel)
{
  setObjectName(QLatin1String("FreedbImporter"));
}

/**
 * Destructor.
 */
FreedbImporter::~FreedbImporter()
{
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
    "www.gnudb.org:80",
    "gnudb.gnudb.org:80",
    "freedb.org:80",
    "freedb.freedb.org:80",
    "at.freedb.org:80",
    "au.freedb.org:80",
    "ca.freedb.org:80",
    "es.freedb.org:80",
    "fi.freedb.org:80",
    "lu.freedb.org:80",
    "ru.freedb.org:80",
    "uk.freedb.org:80",
    "us.freedb.org:80",
    0                  // end of StrList
  };
  return servers;
}

/** default server, 0 to disable */
const char* FreedbImporter::defaultServer() const { return "www.gnudb.org:80"; }

/** default CGI path, 0 to disable */
const char* FreedbImporter::defaultCgiPath() const { return "/~cddb/cddb.cgi"; }

/** anchor to online help, 0 to disable */
const char* FreedbImporter::helpAnchor() const { return "import-freedb"; }

/** configuration, 0 if not used */
ServerImporterConfig* FreedbImporter::config() const { return &ConfigStore::s_freedbCfg; }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void FreedbImporter::parseFindResults(const QByteArray& searchStr)
{
/*
<h2>Search Results, 1 albums found:</h2>
<br><br>
<a href="http://www.gnudb.org/cd/ro920b810c"><b>Catharsis / Imago</b></a><br>
Tracks: 12, total time: 49:07, year: 2002, genre: Metal<br>
<a href="http://www.gnudb.org/gnudb/rock/920b810c" target=_blank>Discid: rock / 920b810c</a><br>
*/
  bool isUtf8 = false;
  int charSetPos = searchStr.indexOf("charset=");
  if (charSetPos != -1) {
    charSetPos += 8;
    QByteArray charset(searchStr.mid(charSetPos, 5));
    isUtf8 = charset == "utf-8" || charset == "UTF-8";
  }
  QString str = isUtf8 ? QString::fromUtf8(searchStr) :
                         QString::fromLatin1(searchStr);
  QRegExp titleRe(QLatin1String("<a href=\"[^\"]+/cd/[^\"]+\"><b>([^<]+)</b></a>"));
  QRegExp catIdRe(QLatin1String("Discid: ([a-z]+)[\\s/]+([0-9a-f]+)"));
  QStringList lines = str.split(QRegExp(QLatin1String("[\\r\\n]+")));
  QString title;
  bool inEntries = false;
  m_albumListModel->clear();
  for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
    if (inEntries) {
      if (titleRe.indexIn(*it) != -1) {
        title = titleRe.cap(1);
      }
      if (catIdRe.indexIn(*it) != -1) {
        m_albumListModel->appendRow(new AlbumListItem(
          title,
          catIdRe.cap(1),
          catIdRe.cap(2)));
      }
    } else if ((*it).indexOf(QLatin1String(" albums found:")) != -1) {
      inEntries = true;
    }
  }
}

/**
 * Parse the track durations from freedb.org.
 *
 * @param text          text buffer containing data from freedb.org
 * @param trackDuration list for results
 */
static void parseFreedbTrackDurations(
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
  QRegExp discLenRe(QLatin1String("Disc length:\\s*\\d+"));
  int discLenPos = discLenRe.indexIn(text, 0);
  if (discLenPos != -1) {
    int len = discLenRe.matchedLength();
    discLenPos += 12;
    int discLen = text.mid(discLenPos, len - 12).toInt();
    int trackOffsetPos = text.indexOf(QLatin1String("Track frame offsets"), 0);
    if (trackOffsetPos != -1) {
      QRegExp re(QLatin1String("#\\s*\\d+"));
      int lastOffset = -1;
      while ((trackOffsetPos = re.indexIn(text, trackOffsetPos)) != -1 &&
             trackOffsetPos < discLenPos) {
        len = re.matchedLength();
        trackOffsetPos += 1;
        int trackOffset = text.mid(trackOffsetPos, len - 1).toInt();
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
static void parseFreedbAlbumData(const QString& text,
                                 FrameCollection& frames)
{
  QRegExp fdre(QLatin1String("DTITLE=\\s*(\\S[^\\r\\n]*\\S)\\s*/\\s*(\\S[^\\r\\n]*\\S)[\\r\\n]"));
  if (fdre.indexIn(text) != -1) {
    frames.setArtist(fdre.cap(1));
    frames.setAlbum(fdre.cap(2));
  }
  fdre.setPattern(QLatin1String("EXTD=[^\\r\\n]*YEAR:\\s*(\\d+)\\D"));
  if (fdre.indexIn(text) != -1) {
    frames.setYear(fdre.cap(1).toInt());
  }
  fdre.setPattern(QLatin1String("EXTD=[^\\r\\n]*ID3G:\\s*(\\d+)\\D"));
  if (fdre.indexIn(text) != -1) {
    frames.setGenre(QString::fromLatin1(Genres::getName(fdre.cap(1).toInt())));
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
  trackDataVector.setCoverArtUrl(QString());
  FrameCollection frames(framesHdr);
  ImportTrackDataVector::iterator it = trackDataVector.begin();
  QList<int>::const_iterator tdit = trackDuration.begin();
  bool atTrackDataListEnd = (it == trackDataVector.end());
  int pos = 0;
  int idx, oldpos = pos;
  int tracknr = 0;
  for (;;) {
    QRegExp fdre(QString(QLatin1String("TTITLE%1=([^\\r\\n]+)[\\r\\n]")).arg(tracknr));
    QString title;
    while ((idx = fdre.indexIn(text, pos)) != -1) {
      title += fdre.cap(1);
      pos = idx + fdre.matchedLength();
    }
    if (pos > oldpos) {
      frames.setTrack(tracknr + 1);
      frames.setTitle(title);
    } else {
      break;
    }
    int duration = (tdit != trackDuration.end()) ?
      *tdit++ : 0;
    if (atTrackDataListEnd) {
      ImportTrackData trackData;
      trackData.setFrameCollection(frames);
      trackData.setImportDuration(duration);
      trackDataVector.push_back(trackData);
    } else {
      while (!atTrackDataListEnd && !it->isEnabled()) {
        ++it;
        atTrackDataListEnd = (it == trackDataVector.end());
      }
      if (!atTrackDataListEnd) {
        (*it).setFrameCollection(frames);
        (*it).setImportDuration(duration);
        ++it;
        atTrackDataListEnd = (it == trackDataVector.end());
      }
    }
    frames = framesHdr;
    oldpos = pos;
    ++tracknr;
  }
  frames.clear();
  while (!atTrackDataListEnd) {
    if (it->isEnabled()) {
      if ((*it).getFileDuration() == 0) {
        it = trackDataVector.erase(it);
      } else {
        (*it).setFrameCollection(frames);
        (*it).setImportDuration(0);
        ++it;
      }
    } else {
      ++it;
    }
    atTrackDataListEnd = (it == trackDataVector.end());
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
  const ServerImporterConfig*,
  const QString& artist, const QString& album)
{
  // At the moment, only www.gnudb.org has a working search
  // so we always use this server for find queries.
  sendRequest(QString::fromLatin1(gnudbServer), QLatin1String("/search/") +
              encodeUrlQuery(artist + QLatin1Char(' ') + album));
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
  sendRequest(cfg->m_server,
              cfg->m_cgiPath + QLatin1String("?cmd=cddb+read+") + cat + QLatin1Char('+') + id +
              QLatin1String("&hello=noname+localhost+Kid3+" VERSION "&proto=6"));
}
