/**
 * \file discogsimporter.cpp
 * Discogs importer.
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

#include "discogsimporter.h"
#include <QUrl>
#include "serverimporterconfig.h"
#include "trackdatamodel.h"
#include "discogsconfig.h"
#include "config.h"
#include "genres.h"
#include "jsonparser.h"

/**
 * Stores information about extra artists.
 * The information can be used to add frames to the appropriate tracks.
 */
class ExtraArtist {
public:
  /**
   * Constructor.
   * @param varMap variant map containing extra artist information
   */
  explicit ExtraArtist(const QVariantMap& varMap);

  /**
   * Add extra artist information to frames.
   * @param frames   frame collection
   * @param trackPos optional position, the extra artist information will
   *                 only be added if this track position is listed in the
   *                 track restrictions or is empty
   */
  void addToFrames(FrameCollection& frames,
                   const QString& trackPos = QString()) const;

  /**
   * Check if extra artist information is only valid for a subset of the tracks.
   * @return true if extra artist has track restriction.
   */
  bool hasTrackRestriction() const { return !m_tracks.isEmpty(); }

private:
  QString m_name;
  QString m_role;
  QStringList m_tracks;
};


namespace {

const char discogsServer[] = "api.discogs.com:80";

/**
 * Replace unicode escape sequences (e.g. "\u2022") by unicode characters.
 * @param str string containing unicode escape sequences
 * @return string with replaced unicode escape sequences.
 */
QString replaceEscapedUnicodeCharacters(QString str)
{
  QRegExp unicodeRe(QLatin1String("\\\\u([0-9a-fA-F]{4})"));
  int offset = 0;
  while (offset >= 0) {
    offset = unicodeRe.indexIn(str, offset);
    if (offset >= 0) {
      str.replace(offset, unicodeRe.matchedLength(),
                  QChar(unicodeRe.cap(1).toUInt(0, 16)));
      ++offset;
    }
  }
  return str;
}

/**
 * Remove trailing stars and numbers like (2) from a string.
 *
 * @param str string
 *
 * @return fixed up string.
 */
QString fixUpArtist(QString str)
{
  str.remove(QRegExp(QLatin1String("[*\\s]*\\(\\d+\\)")));
  str.replace(QRegExp(QLatin1String("\\*($| - |, | / )")), QLatin1String("\\1"));

  return str;
}

/**
 * Create a string with artists contained in an artist list.
 * @param artists list containing artist maps
 * @return string with artists joined appropriately.
 */
QString getArtistString(const QVariantList& artists)
{
  QString artist;
  if (!artists.isEmpty()) {
    QString join;
    foreach (const QVariant& var, artists) {
      QVariantMap varMap = var.toMap();
      if (!artist.isEmpty()) {
        artist += join;
      }
      artist += fixUpArtist(varMap.value(QLatin1String("name")).toString());
      join = varMap.value(QLatin1String("join")).toString();
      if (join.isEmpty() || join == QLatin1String(",")) {
        join = QLatin1String(", ");
      } else {
        join = QLatin1Char(' ') + join + QLatin1Char(' ');
      }
    }
  }
  return artist;
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
  value += involvement;
  value += Frame::stringListSeparator();
  value += involvee;
  frames.setValue(type, value);
}

/**
 * Add name to frame with credits.
 * @param frames frame collection
 * @param type   type of frame
 * @param name   name of person to credit
 */
void addCredit(FrameCollection& frames, Frame::Type type, const QString& name)
{
  QString value = frames.getValue(type);
  if (!value.isEmpty()) value += QLatin1String(", ");
  value += name;
  frames.setValue(type, value);
}

}

/**
 * Constructor.
 * @param varMap variant map containing extra artist information
 */
ExtraArtist::ExtraArtist(const QVariantMap& varMap) :
  m_name(fixUpArtist(varMap.value(QLatin1String("name")).toString())),
  m_role(varMap.value(QLatin1String("role")).toString())
{
  static const QRegExp tracksSepRe(QLatin1String(",\\s*"));
  QString tracks = varMap.value(QLatin1String("tracks")).toString();
  if (!tracks.isEmpty()) {
    m_tracks = tracks.split(tracksSepRe);
  }
}

/**
 * Add extra artist information to frames.
 * @param frames   frame collection
 * @param trackPos optional position, the extra artist information will
 *                 only be added if this track position is listed in the
 *                 track restrictions or is empty
 */
void ExtraArtist::addToFrames(FrameCollection& frames,
                              const QString& trackPos) const
{
  if (!trackPos.isEmpty() && !m_tracks.contains(trackPos))
    return;

  if (m_role.contains(QLatin1String("Composed By")) || m_role.contains(QLatin1String("Music By")) ||
      m_role.contains(QLatin1String("Songwriter"))) {
    addCredit(frames, Frame::FT_Composer, m_name);
  }
  if (m_role.contains(QLatin1String("Written-By")) || m_role.contains(QLatin1String("Written By"))) {
    addCredit(frames, Frame::FT_Author, m_name);
  }
  if (m_role.contains(QLatin1String("Lyrics By"))) {
    addCredit(frames, Frame::FT_Lyricist, m_name);
  }
  if (m_role.contains(QLatin1String("Conductor"))) {
    addCredit(frames, Frame::FT_Conductor, m_name);
  }
  if (m_role.contains(QLatin1String("Orchestra"))) {
    addCredit(frames, Frame::FT_AlbumArtist, m_name);
  }
  if (m_role.contains(QLatin1String("Remix"))) {
    addCredit(frames, Frame::FT_Remixer, m_name);
  }

  if (m_role.contains(QLatin1String("Arranged By"))) {
    addInvolvedPeople(frames, Frame::FT_Arranger,
                      QLatin1String("Arranger"), m_name);
  }
  if (m_role.contains(QLatin1String("Mixed By"))) {
    addInvolvedPeople(frames, Frame::FT_Arranger,
                      QLatin1String("Mixer"), m_name);
  }
  if (m_role.contains(QLatin1String("DJ Mix")) || m_role.contains(QLatin1String("Dj Mix"))) {
    addInvolvedPeople(frames, Frame::FT_Arranger,
                      QLatin1String("DJMixer"), m_name);
  }
  if (m_role.contains(QLatin1String("Engineer")) || m_role.contains(QLatin1String("Mastered By"))) {
    addInvolvedPeople(frames, Frame::FT_Arranger,
                      QLatin1String("Engineer"), m_name);
  }
  if (m_role.contains(QLatin1String("Producer")) || m_role.contains(QLatin1String("Co-producer")) ||
      m_role.contains(QLatin1String("Executive Producer"))) {
    addInvolvedPeople(frames, Frame::FT_Arranger,
                      QLatin1String("Producer"), m_name);
  }

  static const char* const instruments[] = {
    "Performer", "Vocals", "Voice", "Featuring", "Choir", "Chorus",
    "Baritone", "Tenor", "Rap", "Scratches", "Drums", "Percussion",
    "Keyboards", "Cello", "Piano", "Organ", "Synthesizer", "Keys",
    "Wurlitzer", "Rhodes", "Harmonica", "Xylophone", "Guitar", "Bass",
    "Strings", "Violin", "Viola", "Banjo", "Harp", "Mandolin",
    "Clarinet", "Horn", "Cornet", "Flute", "Oboe", "Saxophone",
    "Trumpet", "Tuba", "Trombone"
  };
  for (unsigned i = 0;
       i < sizeof(instruments) / sizeof(instruments[0]);
       ++i) {
    if (m_role.contains(QString::fromLatin1(instruments[i]))) {
      addInvolvedPeople(frames, Frame::FT_Performer, m_role, m_name);
      break;
    }
  }
}


/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
DiscogsImporter::DiscogsImporter(QNetworkAccessManager* netMgr,
                                 TrackDataModel* trackDataModel) :
  ServerImporter(netMgr, trackDataModel)
{
  setObjectName(QLatin1String("DiscogsImporter"));
  m_discogsHeaders["User-Agent"] = "Kid3/" VERSION
      " +http://kid3.sourceforge.net";
}

/**
 * Destructor.
 */
DiscogsImporter::~DiscogsImporter()
{
}

/**
 * Name of import source.
 * @return name.
 */
const char* DiscogsImporter::name() const { return QT_TRANSLATE_NOOP("@default", "Discogs"); }

/** anchor to online help, 0 to disable */
const char* DiscogsImporter::helpAnchor() const { return "import-discogs"; }

/** configuration, 0 if not used */
ServerImporterConfig* DiscogsImporter::config() const { return &DiscogsConfig::instance(); }

/** additional tags option, false if not used */
bool DiscogsImporter::additionalTags() const { return true; }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void DiscogsImporter::parseFindResults(const QByteArray& searchStr)
{
  // search results have the format (JSON, simplified):
  // {"results": [{"style": ["Heavy Metal"], "title": "Wizard (23) - Odin",
  //               "type": "release", "id": 2487778}]}
  QString str = replaceEscapedUnicodeCharacters(QString::fromUtf8(searchStr));

  QVariantMap map = JsonParser::deserialize(str).toMap();
  m_albumListModel->clear();
  foreach (const QVariant& var, map.value(QLatin1String("results")).toList()) {
    QVariantMap result = var.toMap();
    QString title = fixUpArtist(result.value(QLatin1String("title")).toString());
    if (!title.isEmpty()) {
      m_albumListModel->appendRow(new AlbumListItem(
        title,
        QLatin1String("releases"),
        QString::number(result.value(QLatin1String("id")).toInt())));
    }
  }
}

/**
 * Parse result of album request and populate m_trackDataModel with results.
 *
 * @param albumStr album data received
 */
void DiscogsImporter::parseAlbumResults(const QByteArray& albumStr)
{
  // releases have the format (JSON, simplified):
  // { "styles": ["Heavy Metal"],
  //   "labels": [{"name": "LMP"}],
  //   "year": 2003,
  //   "artists": [{"name": "Wizard (23)"}],
  //   "images": [
  //   { "uri": "http://api.discogs.com/image/R-2487778-1293847958.jpeg",
  //     "type": "primary" },
  //   { "uri": "http://api.discogs.com/image/R-2487778-1293847967.jpeg",
  //     "type": "secondary" }],
  //   "id": 2487778,
  //   "genres": ["Rock"],
  //   "thumb": "http://api.discogs.com/image/R-150-2487778-1293847958.jpeg",
  //   "extraartists": [],
  //   "title": "Odin",
  //   "tracklist": [
  //     {"duration": "5:19", "position": "1", "title": "The Prophecy"},
  //     {"duration": "", "position": "Video", "title": "Betrayer"}
  //   ],
  //   "released": "2003",
  //   "formats": [{"name": "CD"}]
  // }
  QRegExp discTrackPosRe(QLatin1String("(\\d+)-(\\d+)"));
  QRegExp yearRe(QLatin1String("^\\d{4}-\\d{2}"));
  QString str = replaceEscapedUnicodeCharacters(QString::fromUtf8(albumStr));
  QVariantMap map = JsonParser::deserialize(str).toMap();

  QList<ExtraArtist> trackExtraArtists;
  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  FrameCollection framesHdr;
  const bool standardTags = getStandardTags();
  if (standardTags) {
    framesHdr.setAlbum(map.value(QLatin1String("title")).toString());
    framesHdr.setArtist(getArtistString(map.value(QLatin1String("artists")).toList()));

    // The year can be found in "released".
    QString released(map.value(QLatin1String("released")).toString());
    if (yearRe.indexIn(released) == 0) {
      released.truncate(4);
    }
    framesHdr.setYear(released.toInt());

    // The genre can be found in "genre" or "style".
    // All genres found are checked for an ID3v1 number, starting with those
    // in the style field.
    QVariantList genreList(map.value(QLatin1String("styles")).toList() +
                           map.value(QLatin1String("genres")).toList());
    int genreNum = 255;
    foreach (const QVariant& var, genreList) {
      genreNum = Genres::getNumber(var.toString());
      if (genreNum != 255) {
        break;
      }
    }
    if (genreNum != 255) {
      framesHdr.setGenre(QString::fromLatin1(Genres::getName(genreNum)));
    } else if (!genreList.isEmpty()) {
      framesHdr.setGenre(genreList.first().toString());
    }
  }

  trackDataVector.setCoverArtUrl(QString());
  const bool coverArt = getCoverArt();
  if (coverArt) {
    // Cover art can be found in "images"
    QVariantList images = map.value(QLatin1String("images")).toList();
    if (!images.isEmpty()) {
      trackDataVector.setCoverArtUrl(images.first().toMap().value(QLatin1String("uri")).
                                     toString());
    }
  }

  const bool additionalTags = getAdditionalTags();
  if (additionalTags) {
    // Publisher can be found in "label"
    QVariantList labels = map.value(QLatin1String("labels")).toList();
    if (!labels.isEmpty()) {
      QVariantMap firstLabelMap = labels.first().toMap();
      framesHdr.setValue(Frame::FT_Publisher,
          fixUpArtist(firstLabelMap.value(QLatin1String("name")).toString()));
      QString catNo = firstLabelMap.value(QLatin1String("catno")).toString();
      if (!catNo.isEmpty() && catNo.toLower() != QLatin1String("none")) {
        framesHdr.setValue(Frame::FT_CatalogNumber, catNo);
      }
    }
    // Media can be found in "formats"
    QVariantList formats = map.value(QLatin1String("formats")).toList();
    if (!formats.isEmpty()) {
      framesHdr.setValue(Frame::FT_Media,
                         formats.first().toMap().value(QLatin1String("name")).toString());
    }
    // Credits can be found in "extraartists"
    QVariantList extraartists = map.value(QLatin1String("extraartists")).toList();
    if (!extraartists.isEmpty()) {
      foreach (const QVariant& var, extraartists) {
        ExtraArtist extraArtist(var.toMap());
        if (extraArtist.hasTrackRestriction()) {
          trackExtraArtists.append(extraArtist);
        } else {
          extraArtist.addToFrames(framesHdr);
        }
      }
    }
    // Release country can be found in "country"
    QString country(map.value(QLatin1String("country")).toString());
    if (!country.isEmpty()) {
      framesHdr.setValue(Frame::FT_ReleaseCountry, country);
    }
  }

  FrameCollection frames(framesHdr);
  ImportTrackDataVector::iterator it = trackDataVector.begin();
  bool atTrackDataListEnd = (it == trackDataVector.end());
  int trackNr = 1;
  QVariantList trackList = map.value(QLatin1String("tracklist")).toList();

  // Check if all positions are empty.
  bool allPositionsEmpty = true;
  foreach (const QVariant& var, trackList) {
    if (!var.toMap().value(QLatin1String("position")).toString().isEmpty()) {
      allPositionsEmpty = false;
      break;
    }
  }

  foreach (const QVariant& var, trackList) {
    QVariantMap track = var.toMap();

    QString position(track.value(QLatin1String("position")).toString());
    bool ok;
    int pos = position.toInt(&ok);
    if (!ok) {
      if (discTrackPosRe.exactMatch(position)) {
        if (additionalTags) {
          frames.setValue(Frame::FT_Disc, discTrackPosRe.cap(1));
        }
        pos = discTrackPosRe.cap(2).toInt();
      } else {
        pos = trackNr;
      }
    }
    QString title(track.value(QLatin1String("title")).toString());

    QStringList durationHms = track.value(QLatin1String("duration")).toString().split(QLatin1Char(':'));
    int duration = 0;
    foreach (const QString& var, durationHms) {
      duration *= 60;
      duration += var.toInt();
    }
    if (!allPositionsEmpty && position.isEmpty()) {
      if (additionalTags) {
        framesHdr.setValue(Frame::FT_Part, title);
      }
    } else if (!title.isEmpty() || duration != 0) {
      if (standardTags) {
        frames.setTrack(pos);
        frames.setTitle(title);
      }
      QVariantList artists(track.value(QLatin1String("artists")).toList());
      if (!artists.isEmpty()) {
        if (standardTags) {
          frames.setArtist(getArtistString(artists));
        }
        if (additionalTags) {
          frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
        }
      }
      if (additionalTags) {
        QVariantList extraartists(track.value(QLatin1String("extraartists")).toList());
        if (!extraartists.isEmpty()) {
          foreach (const QVariant& var, extraartists) {
            ExtraArtist extraArtist(var.toMap());
            extraArtist.addToFrames(frames);
          }
        }
      }
      foreach (const ExtraArtist& extraArtist, trackExtraArtists) {
        extraArtist.addToFrames(frames, position);
      }

      if (atTrackDataListEnd) {
        ImportTrackData trackData;
        trackData.setFrameCollection(frames);
        trackData.setImportDuration(duration);
        trackDataVector.append(trackData);
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
    }
    frames = framesHdr;
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

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void DiscogsImporter::sendFindQuery(
  const ServerImporterConfig*,
  const QString& artist, const QString& album)
{
  /*
   * Query looks like this:
   * http://api.discogs.com//database/search?type=release&title&q=amon+amarth+avenger
   */
  sendRequest(QString::fromLatin1(discogsServer),
              QLatin1String("/database/search?type=release&title&q=") +
              encodeUrlQuery(artist + QLatin1Char(' ') + album), m_discogsHeaders);
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void DiscogsImporter::sendTrackListQuery(
  const ServerImporterConfig*, const QString& cat, const QString& id)
{
  /*
   * Query looks like this:
   * http://api.discogs.com/releases/761529
   */
  sendRequest(QString::fromLatin1(discogsServer), QLatin1Char('/') + QString::fromLatin1(QUrl::toPercentEncoding(cat)) + QLatin1Char('/')
              + id, m_discogsHeaders);
}
