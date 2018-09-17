/**
 * \file discogsimporter.cpp
 * Discogs importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 *
 * Copyright (C) 2006-2014  Urs Fleisch
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

namespace {

const char discogsServer[] = "www.discogs.com";

/**
 * Remove trailing stars and numbers like (2) from a string.
 *
 * @param str string
 *
 * @return fixed up string.
 */
QString fixUpArtist(QString str)
{
  str.replace(QRegExp(QLatin1String(",(\\S)")), QLatin1String(", \\1"));
  str.replace(QLatin1String("* / "), QLatin1String(" / "));
  str.replace(QLatin1String("*,"), QLatin1String(","));
  str.remove(QRegExp(QLatin1String("\\*$")));
  str.remove(QRegExp(QLatin1String("[*\\s]*\\(\\d+\\)\\(tracks:[^)]+\\)")));
  str.replace(QRegExp(
    QLatin1String("[*\\s]*\\((?:\\d+|tracks:[^)]+)\\)(\\s*/\\s*,|\\s*&amp;|"
                  "\\s*And|\\s*and)")),
    QLatin1String("\\1"));
  str.remove(QRegExp(QLatin1String("[*\\s]*\\((?:\\d+|tracks:[^)]+)\\)$")));
  return ServerImporter::removeHtml(str);
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
 * Set tags from a string with credits lines.
 * The string must have lines like "Composed By - Iommi", separated by \\n.
 *
 * @param str    credits string
 * @param frames tags will be added to these frames
 *
 * @return true if credits found.
 */
bool parseCredits(const QString& str, FrameCollection& frames)
{
  bool result = false;
  QStringList lines = str.split(QLatin1Char('\n'));
  for (QStringList::const_iterator it = lines.begin();
       it != lines.end();
       ++it) {
    int nameStart = (*it).indexOf(QLatin1String(" - "));
    if (nameStart != -1) {
      const QStringList names = (*it).mid(nameStart + 3).split(QLatin1String(", "));
      QString name;
      for (const QString& namesPart : names) {
        if (!name.isEmpty()) {
          name += QLatin1String(", ");
        }
        name += fixUpArtist(namesPart);
      }
      QStringList credits = (*it).left(nameStart).split(QLatin1String(", "));
      for (QStringList::const_iterator cit = credits.begin();
           cit != credits.end();
           ++cit) {
        static const struct {
          const char* credit;
          Frame::Type type;
        } creditToType[] = {
          { "Composed By", Frame::FT_Composer },
          { "Conductor", Frame::FT_Conductor },
          { "Orchestra", Frame::FT_AlbumArtist },
          { "Lyrics By", Frame::FT_Lyricist },
          { "Written-By", Frame::FT_Author },
          { "Written By", Frame::FT_Author },
          { "Remix", Frame::FT_Remixer },
          { "Music By", Frame::FT_Composer },
          { "Songwriter", Frame::FT_Composer }
        };
        bool found = false;
        for (unsigned i = 0;
             i < sizeof(creditToType) / sizeof(creditToType[0]);
             ++i) {
          if (*cit == QString::fromLatin1(creditToType[i].credit)) {
            frames.setValue(creditToType[i].type, name);
            found = true;
            break;
          }
        }
        if (found) {
          result = true;
        } else {
          static const struct {
            const char* credit;
            const char* arrangement;
          } creditToArrangement[] = {
            { "Arranged By", "Arranger" },
            { "Mixed By", "Mixer" },
            { "DJ Mix", "DJMixer" },
            { "Dj Mix", "DJMixer" },
            { "Engineer", "Engineer" },
            { "Mastered By", "Engineer" },
            { "Producer", "Producer" },
            { "Co-producer", "Producer" },
            { "Executive Producer", "Producer" }
          };
          for (unsigned i = 0;
               i < sizeof(creditToArrangement) / sizeof(creditToArrangement[0]);
               ++i) {
            if ((*cit).startsWith(
                  QString::fromLatin1(creditToArrangement[i].credit))) {
              addInvolvedPeople(frames, Frame::FT_Arranger,
                QString::fromLatin1(creditToArrangement[i].arrangement), name);
              found = true;
              break;
            }
          }
        }
        if (found) {
          result = true;
        } else {
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
            if ((*cit).contains(QString::fromLatin1(instruments[i]))) {
              addInvolvedPeople(frames, Frame::FT_Performer, *cit, name);
              found = true;
              break;
            }
          }
        }
        if (found) {
          result = true;
        }
      }
    }
  }
  return result;
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
  m_discogsHeaders["User-Agent"] =
      "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_3_2 like Mac OS X; en-us) "
      "AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8H7 "
      "Safari/6533.18.5";
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
const char* DiscogsImporter::name() const {
  return QT_TRANSLATE_NOOP("@default", "Discogs");
}

/** anchor to online help, 0 to disable */
const char* DiscogsImporter::helpAnchor() const { return "import-discogs"; }

/** configuration, 0 if not used */
ServerImporterConfig* DiscogsImporter::config() const {
  return &DiscogsConfig::instance();
}

/** additional tags option, false if not used */
bool DiscogsImporter::additionalTags() const { return true; }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void DiscogsImporter::parseFindResults(const QByteArray& searchStr)
{
  // releases have the format:
  // <a href="/artist/256076-Amon-Amarth">Amon Amarth</a>         </span> -
  // <a class="search_result_title " href="/Amon-Amarth-The-Avenger/release/398878" data-followable="true">The Avenger</a>
  QString str = QString::fromUtf8(searchStr);
  QRegExp idTitleRe(QLatin1String(
      "<a href=\"/artist/[^>]+>([^<]+)</a>[^-]*-"
      "\\s*<a class=\"search_result_title[ \"]+href=\"/([^/]*/?release)/"
      "([0-9]+)\"[^>]*>([^<]+)</a>"));
  m_albumListModel->clear();
  int pos = 0;
  while ((pos = idTitleRe.indexIn(str, pos)) != -1) {
    int len = idTitleRe.matchedLength();
    QString artist = fixUpArtist(idTitleRe.cap(1).trimmed());
    QString title = removeHtml(idTitleRe.cap(4).trimmed());
    if (!title.isEmpty()) {
      m_albumListModel->appendRow(new AlbumListItem(
        artist + QLatin1String(" - ") + title,
        idTitleRe.cap(2),
        idTitleRe.cap(3)));
    }
    pos += len;
  }
}

/**
 * Parse result of album request and populate m_trackDataModel with results.
 *
 * @param albumStr album data received
 */
void DiscogsImporter::parseAlbumResults(const QByteArray& albumStr)
{
  QRegExp nlSpaceRe(QLatin1String("[\r\n]+\\s*"));
  QRegExp atDiscogsRe(QLatin1String("\\s*\\([^)]+\\) at Discogs\n?$"));
  QString str = QString::fromUtf8(albumStr);

  FrameCollection framesHdr;
  int start, end;
  const bool standardTags = getStandardTags();
  if (standardTags) {
    /*
     * artist and album can be found in the title:
<title>Amon Amarth - The Avenger (CD, Album, Dig) at Discogs</title>
     */
    start = str.indexOf(QLatin1String("<title>"));
    if (start >= 0) {
      start += 7; // skip <title>
      end = str.indexOf(QLatin1String("</title>"), start);
      if (end > start) {
        QString titleStr = str.mid(start, end - start);
        titleStr.replace(atDiscogsRe, QLatin1String(""));
        // reduce new lines and space after them
        titleStr.replace(nlSpaceRe, QLatin1String(" "));
        start = 0;
        end = titleStr.indexOf(QLatin1String(" - "), start);
        if (end > start) {
          framesHdr.setArtist(fixUpArtist(titleStr.mid(start, end - start)));
          start = end + 3; // skip " - "
        }
        framesHdr.setAlbum(removeHtml(titleStr.mid(start)));
      }
    }
    /*
     * the year can be found in "Released:"
<div class="head">Released:</div><div class="content">02 Nov 1999</div>
     */
    start = str.indexOf(QLatin1String("Released:<"));
    if (start >= 0) {
      start += 9; // skip "Released:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString yearStr = str.mid(start, end - start);
        // strip new lines and space after them
        yearStr.replace(nlSpaceRe, QLatin1String(""));
        yearStr = removeHtml(yearStr); // strip HTML tags and entities
        // this should skip day and month numbers
        QRegExp yearRe(QLatin1String("(\\d{4})"));
        if (yearRe.indexIn(yearStr) >= 0) {
          framesHdr.setYear(yearRe.cap(1).toInt());
        }
      }
    }
    /*
     * the genre can be found in "Genre:" or "Style:" (lines with only whitespace
     *  in between):
<div class="head">Genre:</div><div class="content">
      Rock
</div>
<div class="head">Style:</div><div class="content">
    Viking Metal,
    Death Metal
</div>
     */
    // All genres found are checked for an ID3v1 number, starting with those
    // in the Style field.
    QStringList genreList;
    static const char* const fields[] = { "Style:", "Genre:" };
    for (unsigned i = 0; i < sizeof(fields) / sizeof(fields[0]); ++i) {
      start = str.indexOf(QString::fromLatin1(fields[i]) + QLatin1Char('<'));
      if (start >= 0) {
        start += qstrlen(fields[i]); // skip field
        end = str.indexOf(QLatin1String("</div>"), start + 1);
        if (end > start) {
          QString genreStr = str.mid(start, end - start);
          // strip new lines and space after them
          genreStr.replace(nlSpaceRe, QLatin1String(""));
          genreStr = removeHtml(genreStr); // strip HTML tags and entities
          if (genreStr.indexOf(QLatin1Char(',')) >= 0) {
            genreList += genreStr.split(QRegExp(QLatin1String(",\\s*")));
          } else {
            if (!genreStr.isEmpty()) {
              genreList += genreStr;
            }
          }
        }
      }
    }
    int genreNum = 255;
    for (QStringList::const_iterator it = genreList.begin();
         it != genreList.end();
         ++it) {
      genreNum = Genres::getNumber(*it);
      if (genreNum != 255) {
        break;
      }
    }
    if (genreNum != 255) {
      framesHdr.setGenre(QString::fromLatin1(Genres::getName(genreNum)));
    } else if (!genreList.empty()) {
      framesHdr.setGenre(genreList.front());
    }
  }

  const bool additionalTags = getAdditionalTags();
  if (additionalTags) {
    /*
     * publisher can be found in "Label:"
     */
    start = str.indexOf(QLatin1String("Label:<"));
    if (start >= 0) {
      start += 6; // skip "Label:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString labelStr = str.mid(start, end - start);
        // strip new lines and space after them
        labelStr.replace(nlSpaceRe, QLatin1String(""));
        labelStr = fixUpArtist(labelStr);
        QRegExp catNoRe(QLatin1String(" \\s*(?:&lrm;)?- +(\\S[^,]*[^, ])"));
        int catNoPos = catNoRe.indexIn(labelStr);
        if (catNoPos != -1) {
          QString catNo = catNoRe.cap(1);
          labelStr.truncate(catNoPos);
          if (!catNo.isEmpty()) {
            framesHdr.setValue(Frame::FT_CatalogNumber, catNo);
          }
        }
        if (labelStr != QLatin1String("Not On Label")) {
          framesHdr.setValue(Frame::FT_Publisher, fixUpArtist(labelStr));
        }
      }
    }

    /*
     * media can be found in "Format:"
     */
    start = str.indexOf(QLatin1String("Format:<"));
    if (start >= 0) {
      start += 7; // skip "Format:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString mediaStr = str.mid(start, end - start);
        // strip new lines and space after them
        mediaStr.replace(nlSpaceRe, QLatin1String(""));
        mediaStr = removeHtml(mediaStr); // strip HTML tags and entities
        framesHdr.setValue(Frame::FT_Media, mediaStr);
      }
    }

    /*
     * Release country can be found in "Country:"
     */
    start = str.indexOf(QLatin1String("Country:<"));
    if (start >= 0) {
      start += 8; // skip "Country:"
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString countryStr = str.mid(start, end - start);
        // strip new lines and space after them
        countryStr.replace(nlSpaceRe, QLatin1String(""));
        countryStr = removeHtml(countryStr); // strip HTML tags and entities
        framesHdr.setValue(Frame::FT_ReleaseCountry, countryStr);
      }
    }

    /*
     * credits can be found in "Credits"
     */
    start = str.indexOf(QLatin1String(">Credits</h"));
    if (start >= 0) {
      start += 13; // skip "Credits" plus end of element (e.g. "3>")
      end = str.indexOf(QLatin1String("</div>"), start + 1);
      if (end > start) {
        QString creditsStr = str.mid(start, end - start);
        // strip new lines and space after them
        creditsStr.replace(nlSpaceRe, QLatin1String(""));
        creditsStr.replace(QLatin1String("<br />"), QLatin1String("\n"));
        creditsStr.replace(QLatin1String("</li>"), QLatin1String("\n"));
        creditsStr.replace(QLatin1String("&ndash;"), QLatin1String(" - "));
        creditsStr = removeHtml(creditsStr); // strip HTML tags and entities
        parseCredits(creditsStr, framesHdr);
      }
    }
  }

  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  trackDataVector.setCoverArtUrl(QUrl());
  if (getCoverArt()) {
    /*
     * cover art can be found in image source
     */
    start = str.indexOf(QLatin1String("<meta property=\"og:image\" content=\""));
    if (start >= 0) {
      start += 35;
      end = str.indexOf(QLatin1String("\""), start);
      if (end > start) {
        trackDataVector.setCoverArtUrl(QUrl(str.mid(start, end - start)));
      }
    }
  }

  /*
   * album tracks have the format (lines with only whitespace in between):
<div id="tracklist" class="section tracklist" data-toggle="tracklist">
                    <td class="tracklist_track_pos">1</td>
<span class="tracklist_track_title" itemprop="name">Bleed For Ancient Gods</span>
        <td width="25" class="tracklist_track_duration">
            <meta itemprop="duration" content="PT0H04M31S">
            <span>4:31</span>
        </td>

<h1>Tracklist</h1>
<div class="section_content">
<table>
  <tr class="first">
    <td class="track_pos">1</td>
      <td>&nbsp;</td>
    <td class="track_title">Bleed For Ancient Gods</td>
    <td class="track_duration">4:31</td>
    <td class="track_itunes"></td>
  </tr>
  <tr>
    <td class="track_pos">2</td>
(..)
</table>
   *
   * Variations: strange track numbers, no durations, links instead of tracks,
   * only "track" instead of "track_title", align attribute in "track_duration"
   */
  start = str.indexOf(QLatin1String("class=\"section tracklist\""));
  if (start >= 0) {
    end = str.indexOf(QLatin1String("</table>"), start);
    if (end > start) {
      str = str.mid(start, end - start);
      // strip whitespace
      str.replace(nlSpaceRe, QLatin1String(""));

      FrameCollection frames(framesHdr);
      QRegExp posRe(QLatin1String(
        "<td [^>]*class=\"tracklist_track_pos\">(\\d+)</td>"));
      QRegExp artistsRe(QLatin1String(
        "class=\"tracklist_content_multi_artist_dash\">&ndash;</span>"
        "<a href=\"/artist/[^>]+>([^<]+)</a>"));
      QRegExp moreArtistsRe(QLatin1String(
        "^([^<>]+)<a href=\"/artist/[^>]+>([^<]+)</a>"));
      QRegExp titleRe(QLatin1String(
        "class=\"tracklist_track_title\"[^>]*>([^<]+)<"));
      QRegExp durationRe(QLatin1String(
        "<td [^>]*class=\"tracklist_track_duration\"[^>]*>(?:<meta[^>]*>)?"
        "(?:<span>)?(\\d+):(\\d+)</"));
      QRegExp indexRe(QLatin1String("<td class=\"track_index\">([^<]+)$"));
      QRegExp rowEndRe(QLatin1String("</td>[\\s\\r\\n]*</tr>"));
      ImportTrackDataVector::iterator it = trackDataVector.begin();
      bool atTrackDataListEnd = (it == trackDataVector.end());
      int trackNr = 1;
      start = 0;
      while ((end = rowEndRe.indexIn(str, start)) > start) {
        QString trackDataStr = str.mid(start, end - start);
        QString title;
        int duration = 0;
        int pos = trackNr;
        if (titleRe.indexIn(trackDataStr) >= 0) {
          title = removeHtml(titleRe.cap(1));
        }
        if (durationRe.indexIn(trackDataStr) >= 0) {
          duration = durationRe.cap(1).toInt() * 60 +
            durationRe.cap(2).toInt();
        }
        if (posRe.indexIn(trackDataStr) >= 0) {
          pos = posRe.cap(1).toInt();
        }
        if (additionalTags) {
          if (artistsRe.indexIn(trackDataStr) >= 0) {
            // use the artist in the header as the album artist
            // and the artist in the track as the artist
            QString artist(fixUpArtist(artistsRe.cap(1)));
            // Look if there are more artists
            int artistEndPos = artistsRe.pos() + artistsRe.matchedLength();
            while (moreArtistsRe.indexIn(
                     trackDataStr, artistEndPos, QRegExp::CaretAtOffset) >=
                   artistEndPos) {
              artist += moreArtistsRe.cap(1);
              artist += fixUpArtist(moreArtistsRe.cap(2));
              int endPos = moreArtistsRe.pos() + moreArtistsRe.matchedLength();
              if (endPos <= artistEndPos) // must be true for regexp
                break;
              artistEndPos = endPos;
            }
            if (standardTags) {
              frames.setArtist(artist);
            }
            frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
          }
        }
        start = end + 10; // skip </td></tr>
        if (indexRe.indexIn(trackDataStr) >= 0) {
          if (additionalTags) {
            QString subtitle(removeHtml(indexRe.cap(1)));
            framesHdr.setValue(Frame::FT_Part, subtitle);
            frames.setValue(Frame::FT_Part, subtitle);
          }
          continue;
        }
        if (additionalTags) {
          int blockquoteStart =
              trackDataStr.indexOf(QLatin1String("<blockquote>"));
          if (blockquoteStart >= 0) {
            blockquoteStart += 12;
            int blockquoteEnd =
                trackDataStr.indexOf(QLatin1String("</blockquote>"),
                                     blockquoteStart);
            if (blockquoteEnd == -1) {
              // If the element is not correctly closed, search for </span>
              blockquoteEnd = trackDataStr.indexOf(QLatin1String("</span>"),
                                                   blockquoteStart);
            }
            if (blockquoteEnd > blockquoteStart) {
              QString blockquoteStr(trackDataStr.mid(blockquoteStart,
                blockquoteEnd - blockquoteStart));
              // additional track info like "Music By, Lyrics By - "
              blockquoteStr.replace(QLatin1String("<br />"),
                                    QLatin1String("\n"));
              blockquoteStr.replace(QLatin1String("</li>"),
                                    QLatin1String("\n"));
              blockquoteStr.replace(QLatin1String("</span>"),
                                    QLatin1String("\n"));
              blockquoteStr.replace(QLatin1String(" &ndash; "),
                                    QLatin1String(" - "));
              blockquoteStr.replace(QLatin1String("&ndash;"),
                                    QLatin1String(" - "));
              blockquoteStr = removeHtml(blockquoteStr);
              parseCredits(blockquoteStr, frames);
            }
          }
        }

        if (!title.isEmpty() || duration != 0) {
          if (standardTags) {
            frames.setTrack(pos);
            frames.setTitle(title);
          }
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
    }
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
   * http://www.discogs.com/search/?q=amon+amarth+avenger&type=release&layout=sm
   */
  sendRequest(QString::fromLatin1(discogsServer),
              QString(QLatin1String("/search/?q=")) +
              encodeUrlQuery(artist + QLatin1Char(' ') + album) +
              QLatin1String("&type=release&layout=sm"), QLatin1String("https"),
              m_discogsHeaders);
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
   * http://www.discogs.com/release/761529
   */
  sendRequest(QString::fromLatin1(discogsServer), QLatin1Char('/') +
              QString::fromLatin1(QUrl::toPercentEncoding(cat)) +
              QLatin1Char('/') + id, QLatin1String("https"), m_discogsHeaders);
}
