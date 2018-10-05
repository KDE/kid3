/**
 * \file musicbrainzimporter.cpp
 * MusicBrainz release database importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
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

#include "musicbrainzimporter.h"
#include <QDomDocument>
#include <QUrl>
#include "serverimporterconfig.h"
#include "trackdatamodel.h"
#include "musicbrainzconfig.h"

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
MusicBrainzImporter::MusicBrainzImporter(
  QNetworkAccessManager* netMgr, TrackDataModel *trackDataModel) :
  ServerImporter(netMgr, trackDataModel)
{
  setObjectName(QLatin1String("MusicBrainzImporter"));
  m_headers["User-Agent"] = "curl/7.52.1";
}

/**
 * Name of import source.
 * @return name.
 */
const char* MusicBrainzImporter::name() const {
  return QT_TRANSLATE_NOOP("@default", "MusicBrainz Release");
}

/** NULL-terminated array of server strings, 0 if not used */
const char** MusicBrainzImporter::serverList() const
{
  return nullptr;
}

/** default server, 0 to disable */
const char* MusicBrainzImporter::defaultServer() const {
  return nullptr;
}

/** anchor to online help, 0 to disable */
const char* MusicBrainzImporter::helpAnchor() const {
  return "import-musicbrainzrelease";
}

/** configuration, 0 if not used */
ServerImporterConfig* MusicBrainzImporter::config() const {
  return &MusicBrainzConfig::instance();
}

/** additional tags option, false if not used */
bool MusicBrainzImporter::additionalTags() const { return true; }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void MusicBrainzImporter::parseFindResults(const QByteArray& searchStr)
{
  /* simplified XML result:
<metadata>
  <release-list offset="0" count="3">
    <release ext:score="100" id="978c7ed1-a854-4ef2-bd4e-e7c1317be854">
      <title>Odin</title>
      <artist-credit>
        <name-credit>
          <artist id="d1075cad-33e3-496b-91b0-d4670aabf4f8">
            <name>Wizard</name>
            <sort-name>Wizard</sort-name>
          </artist>
        </name-credit>
      </artist-credit>
    </release>
  */
  int start = searchStr.indexOf("<?xml");
  int end = searchStr.indexOf("</metadata>");
  QByteArray xmlStr = searchStr;
  if (start >= 0 && end > start) {
    xmlStr = xmlStr.mid(start, end + 11 - start);
  }
  QDomDocument doc;
  if (doc.setContent(xmlStr, false)) {
    m_albumListModel->clear();
    QDomElement releaseList =
      doc.namedItem(QLatin1String("metadata")).toElement().namedItem(QLatin1String("release-list")).toElement();
    for (QDomNode releaseNode = releaseList.namedItem(QLatin1String("release"));
         !releaseNode.isNull();
         releaseNode = releaseNode.nextSibling()) {
      QDomElement release = releaseNode.toElement();
      QString id = release.attribute(QLatin1String("id"));
      QString title = release.namedItem(QLatin1String("title")).toElement().text();
      QDomElement artist = release.namedItem(QLatin1String("artist-credit")).toElement().
          namedItem(QLatin1String("name-credit")).toElement().namedItem(QLatin1String("artist")).toElement();
      QString name = artist.namedItem(QLatin1String("name")).toElement().text();
      m_albumListModel->appendRow(new AlbumListItem(
        name + QLatin1String(" - ") + title,
        QLatin1String("release"),
        id));
    }
  }
}

namespace {

/**
 * Uppercase the first characters of each word in a string.
 *
 * @param str string with words to uppercase
 *
 * @return string with first letters in uppercase.
 */
QString upperCaseFirstLetters(const QString& str)
{
  QString result(str);
  int len = result.length();
  int pos = 0;
  while (pos < len) {
    result[pos] = result.at(pos).toUpper();
    pos = result.indexOf(QLatin1Char(' '), pos);
    if (pos++ == -1) {
      break;
    }
  }
  return result;
}

/**
 * Add involved people to a frame.
 * The format used is (should be converted according to tag specifications):
 * involvee 1 (involvement 1)\n
 * involvee 2 (involvement 2)\n
 * ...
 * involvee n (involvement n)
 *
 * @param frames      frame collection
 * @param type        type of frame
 * @param involvement involvement (e.g. instrument)
 * @param involvee    name of involvee (e.g. musician)
 */
void addInvolvedPeople(
  FrameCollection& frames, Frame::Type type,
  const QString& involvement, const QString& involvee)
{
  QString value = frames.getValue(type);
  if (!value.isEmpty()) value += Frame::stringListSeparator();
  value += upperCaseFirstLetters(involvement);
  value += Frame::stringListSeparator();
  value += involvee;
  frames.setValue(type, value);
}

/**
 * Set tags from an XML node with a relation list.
 *
 * @param relationList relation-list with target-type Artist
 * @param frames       tags will be added to these frames
 *
 * @return true if credits found.
 */
bool parseCredits(const QDomElement& relationList, FrameCollection& frames)
{
  bool result = false;
  QDomNode relation(relationList.firstChild());
  while (!relation.isNull()) {
    QString artist(relation.toElement().namedItem(QLatin1String("artist")).toElement().
                   namedItem(QLatin1String("name")).toElement().text());
    if (!artist.isEmpty()) {
      QString type(relation.toElement().attribute(QLatin1String("type")));
      if (type == QLatin1String("instrument")) {
        QDomNode attributeList(relation.toElement().namedItem(QLatin1String("attribute-list")));
        if (!attributeList.isNull()) {
          addInvolvedPeople(frames, Frame::FT_Performer,
            attributeList.firstChild().toElement().text(), artist);
        }
      } else if (type == QLatin1String("vocal")) {
        addInvolvedPeople(frames, Frame::FT_Performer, type, artist);
      } else {
        static const struct {
          const char* credit;
          Frame::Type type;
        } creditToType[] = {
          { "composer", Frame::FT_Composer },
          { "conductor", Frame::FT_Conductor },
          { "performing orchestra", Frame::FT_AlbumArtist },
          { "lyricist", Frame::FT_Lyricist },
          { "publisher", Frame::FT_Publisher },
          { "remixer", Frame::FT_Remixer }
        };
        bool found = false;
        for (const auto& c2t : creditToType) {
          if (type == QString::fromLatin1(c2t.credit)) {
            frames.setValue(c2t.type, artist);
            found = true;
            break;
          }
        }
        if (!found && type != QLatin1String("tribute")) {
          addInvolvedPeople(frames, Frame::FT_Arranger, type, artist);
        }
      }
    }
    result = true;
    relation = relation.nextSibling();
  }
  return result;
}

}

/**
 * Parse result of album request and populate m_trackDataModel with results.
 *
 * @param albumStr album data received
 */
void MusicBrainzImporter::parseAlbumResults(const QByteArray& albumStr)
{
  /*
<metadata>
  <release id="978c7ed1-a854-4ef2-bd4e-e7c1317be854">
    <title>Odin</title>
    <artist-credit>
      <name-credit>
        <artist id="d1075cad-33e3-496b-91b0-d4670aabf4f8">
          <name>Wizard</name>
          <sort-name>Wizard</sort-name>
        </artist>
      </name-credit>
    </artist-credit>
    <date>2003-08-19</date>
    <asin>B00008OUEN</asin>
    <medium-list count="1">
      <medium>
        <position>1</position>
        <track-list count="11" offset="0">
          <track>
            <position>1</position>
            <recording id="dac7c002-432f-4dcb-ad57-5ebde8e258b0">
              <title>The Prophecy</title>
              <length>319173</length>
            </recording>
  */
  int start = albumStr.indexOf("<?xml");
  int end = albumStr.indexOf("</metadata>");
  QByteArray xmlStr = start >= 0 && end > start ?
    albumStr.mid(start, end + 11 - start) : albumStr;
  QDomDocument doc;
  if (doc.setContent(xmlStr, false)) {
    QDomElement release =
      doc.namedItem(QLatin1String("metadata")).toElement().namedItem(QLatin1String("release")).toElement();
    FrameCollection framesHdr;
    const bool standardTags = getStandardTags();
    if (standardTags) {
      framesHdr.setAlbum(release.namedItem(QLatin1String("title")).toElement().text());
      framesHdr.setArtist(release.namedItem(QLatin1String("artist-credit")).toElement().
                          namedItem(QLatin1String("name-credit")).toElement().
                          namedItem(QLatin1String("artist")).toElement().
                          namedItem(QLatin1String("name")).toElement().text());
      QString date(release.namedItem(QLatin1String("date")).toElement().text());
      if (!date.isEmpty()) {
        QRegExp dateRe(QLatin1String(R"((\d{4})(?:-\d{2})?(?:-\d{2})?)"));
        int year = 0;
        if (dateRe.exactMatch(date)) {
          year = dateRe.cap(1).toInt();
        } else {
          year = date.toInt();
        }
        if (year != 0) {
          framesHdr.setYear(year);
        }
      }
    }

    ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
    trackDataVector.setCoverArtUrl(QUrl());
    const bool coverArt = getCoverArt();
    if (coverArt) {
      QString asin(release.namedItem(QLatin1String("asin")).toElement().text());
      if (!asin.isEmpty()) {
        trackDataVector.setCoverArtUrl(
          QUrl(QLatin1String("http://www.amazon.com/dp/") + asin));
      }
    }

    const bool additionalTags = getAdditionalTags();
    if (additionalTags) {
      // label can be found in the label-info-list
      QDomElement labelInfoList(release.namedItem(QLatin1String("label-info-list")).toElement());
      if (!labelInfoList.isNull()) {
        QDomElement labelInfo((labelInfoList.namedItem(QLatin1String("label-info")).toElement()));
        if (!labelInfo.isNull()) {
          QString label(labelInfo.namedItem(QLatin1String("label")).namedItem(QLatin1String("name")).toElement().text());
          if (!label.isEmpty()) {
            framesHdr.setValue(Frame::FT_Publisher, label);
          }
          QString catNo(labelInfo.namedItem(QLatin1String("catalog-number")).toElement().text());
          if (!catNo.isEmpty()) {
            framesHdr.setValue(Frame::FT_CatalogNumber, catNo);
          }
        }
      }
      // Release country can be found in "country"
      QString country(release.namedItem(QLatin1String("country")).toElement().text());
      if (!country.isEmpty()) {
        framesHdr.setValue(Frame::FT_ReleaseCountry, country);
      }
    }

    if (additionalTags || coverArt) {
      QDomNode relationListNode(release.firstChild());
      while (!relationListNode.isNull()) {
        if (relationListNode.nodeName() == QLatin1String("relation-list")) {
          QDomElement relationList(relationListNode.toElement());
          if (!relationList.isNull()) {
            QString targetType(relationList.attribute(QLatin1String("target-type")));
            if (targetType == QLatin1String("artist")) {
              if (additionalTags) {
                parseCredits(relationList, framesHdr);
              }
            } else if (targetType == QLatin1String("url")) {
              if (coverArt) {
                QDomNode relationNode(relationList.firstChild());
                while (!relationNode.isNull()) {
                  if (relationNode.nodeName() == QLatin1String("relation")) {
                    QDomElement relation(relationNode.toElement());
                    if (!relation.isNull()) {
                      QString type(relation.attribute(QLatin1String("type")));
                      if (type == QLatin1String("cover art link") || type == QLatin1String("amazon asin")) {
                        QString coverArtUrl =
                            relation.namedItem(QLatin1String("target")).toElement().text();
                        // https://www.amazon.de/gp/product/ does not work,
                        // fix such links.
                        coverArtUrl.replace(
                            QRegExp(QLatin1String("https://www\\.amazon\\.[^/]+/gp/product/")),
                            QLatin1String("http://images.amazon.com/images/P/"));
                        if (!coverArtUrl.endsWith(QLatin1String(".jpg"))) {
                          coverArtUrl += QLatin1String(".jpg");
                        }
                        trackDataVector.setCoverArtUrl(
                          QUrl(coverArtUrl));
                      }
                    }
                  }
                  relationNode = relationNode.nextSibling();
                }
              }
            }
          }
        }
        relationListNode = relationListNode.nextSibling();
      }
    }

    auto it = trackDataVector.begin();
    bool atTrackDataListEnd = (it == trackDataVector.end());
    int discNr = 1, trackNr = 1;
    bool ok;
    FrameCollection frames(framesHdr);
    QDomElement mediumList = release.namedItem(QLatin1String("medium-list")).toElement();
    int mediumCount = mediumList.attribute(QLatin1String("count")).toInt();
    for (QDomNode mediumNode = mediumList.namedItem(QLatin1String("medium"));
         !mediumNode.isNull();
         mediumNode = mediumNode.nextSibling()) {
      int position = mediumNode.namedItem(QLatin1String("position")).toElement().text().toInt(&ok);
      if (ok) {
        discNr = position;
      }
      QDomElement trackList = mediumNode.namedItem(QLatin1String("track-list")).toElement();
      for (QDomNode trackNode = trackList.namedItem(QLatin1String("track"));
           !trackNode.isNull();
           trackNode = trackNode.nextSibling()) {
        if (mediumCount > 1 && additionalTags) {
          frames.setValue(Frame::FT_Disc, QString::number(discNr));
        }
        QDomElement track = trackNode.toElement();
        position = track.namedItem(QLatin1String("position")).toElement().text().toInt(&ok);
        if (ok) {
          trackNr = position;
        }
        if (standardTags) {
          frames.setTrack(trackNr);
        }
        int duration = track.namedItem(QLatin1String("length")).toElement().text().toInt();
        QDomElement recording = track.namedItem(QLatin1String("recording")).toElement();
        if (!recording.isNull()) {
          if (standardTags) {
            frames.setTitle(recording.namedItem(QLatin1String("title")).toElement().text());
          }
          int length = recording.namedItem(QLatin1String("length")).toElement().text().toInt(&ok);
          if (ok) {
            duration = length;
          }
          QDomNode artistNode = recording.namedItem(QLatin1String("artist-credit"));
          if (!artistNode.isNull()) {
            QString artist(artistNode.toElement().
                namedItem(QLatin1String("name-credit")).toElement().
                namedItem(QLatin1String("artist")).toElement().
                namedItem(QLatin1String("name")).toElement().text());
            if (!artist.isEmpty()) {
              // use the artist in the header as the album artist
              // and the artist in the track as the artist
              if (standardTags) {
                frames.setArtist(artist);
              }
              if (additionalTags) {
                frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
              }
            }
          }
          if (additionalTags) {
            QDomNode relationListNode(recording.firstChild());
            while (!relationListNode.isNull()) {
              if (relationListNode.nodeName() == QLatin1String("relation-list")) {
                QDomElement relationList(relationListNode.toElement());
                if (!relationList.isNull()) {
                  QString targetType(relationList.attribute(QLatin1String("target-type")));
                  if (targetType == QLatin1String("artist")) {
                    parseCredits(relationList, frames);
                  } else if (targetType == QLatin1String("work")) {
                    QDomNode workRelationListNode(relationList.
                          namedItem(QLatin1String("relation")).
                          namedItem(QLatin1String("work")).
                          namedItem(QLatin1String("relation-list")));
                    if (!workRelationListNode.isNull()) {
                      parseCredits(workRelationListNode.toElement(), frames);
                    }
                  }
                }
              }
              relationListNode = relationListNode.nextSibling();
            }
          }
        }
        duration /= 1000;
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
        ++trackNr;
        frames = framesHdr;
      }
      ++discNr;
    }
    // handle redundant tracks
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
}

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void MusicBrainzImporter::sendFindQuery(
  const ServerImporterConfig* cfg,
  const QString& artist, const QString& album)
{
  Q_UNUSED(cfg)
  /*
   * Query looks like this:
   * http://musicbrainz.org/ws/2/release?query=artist:wizard%20AND%20release:odin
   */
  QString path(QLatin1String("/ws/2/release?query="));
  if (!artist.isEmpty()) {
    QString artistQuery(artist.contains(QLatin1Char(' '))
                        ? QLatin1Char('"') + artist + QLatin1Char('"')
                        : artist);
    if (!album.isEmpty()) {
      artistQuery += QLatin1String(" AND ");
    }
    path += QLatin1String("artist:");
    path += QString::fromLatin1(QUrl::toPercentEncoding(artistQuery));
  }
  if (!album.isEmpty()) {
    QString albumQuery(album.contains(QLatin1Char(' '))
                        ? QLatin1Char('"') + album + QLatin1Char('"')
                        : album);
    path += QLatin1String("release:");
    path += QString::fromLatin1(QUrl::toPercentEncoding(albumQuery));
  }
  sendRequest(QLatin1String("musicbrainz.org"), path, QLatin1String("https"),
              m_headers);
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void MusicBrainzImporter::sendTrackListQuery(
  const ServerImporterConfig* cfg, const QString& cat, const QString& id)
{
  /*
   * Query looks like this:
   * http://musicbrainz.org/ws/2/release/978c7ed1-a854-4ef2-bd4e-e7c1317be854?inc=artists+recordings
   */
  QString path(QLatin1String("/ws/2/"));
  path += cat;
  path += QLatin1Char('/');
  path += id;
  path += QLatin1String("?inc=");
  if (cfg->additionalTags()) {
    path += QLatin1String("artist-credits+labels+recordings+media+isrcs+"
                "discids+artist-rels+label-rels+recording-rels+release-rels");
  } else {
    path += QLatin1String("artists+recordings");
  }
  if (cfg->coverArt()) {
    path += QLatin1String("+url-rels");
  }
  if (cfg->additionalTags()) {
    path += QLatin1String("+work-rels+recording-level-rels+work-level-rels");
  }
  sendRequest(QLatin1String("musicbrainz.org"), path, QLatin1String("https"),
              m_headers);
}
