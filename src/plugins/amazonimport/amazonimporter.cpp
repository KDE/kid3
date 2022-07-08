/**
 * \file amazonimporter.cpp
 * Amazon database importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Dec 2009
 *
 * Copyright (C) 2009-2021  Urs Fleisch
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

#include "amazonimporter.h"
#include <QRegularExpression>
#include "trackdatamodel.h"
#include "amazonconfig.h"

namespace {

/**
 * Remove " [Explicit]" suffix from end of string.
 * @param str string to modify
 * @return modified string.
 */
QString removeExplicit(QString str)
{
  if (str.endsWith(QLatin1String(" [Explicit]"))) {
    str.truncate(str.length() - 11);
  }
  return str;
}

}


/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values
 */
AmazonImporter::AmazonImporter(
  QNetworkAccessManager* netMgr,
  TrackDataModel* trackDataModel)
  : ServerImporter(netMgr, trackDataModel)
{
  setObjectName(QLatin1String("AmazonImporter"));
  m_headers["User-Agent"] =
      "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.2) "
      "Gecko/20090729 Firefox/3.5.2 GTB5";
}

/**
 * Name of import source.
 * @return name.
 */
const char* AmazonImporter::name() const {
  return QT_TRANSLATE_NOOP("@default", "Amazon");
}

/** NULL-terminated array of server strings, 0 if not used */
const char** AmazonImporter::serverList() const
{
  static const char* servers[] = {
    // Parsing only works with English text
    "www.amazon.com",
    "www.amazon.co.uk",
    nullptr            // end of StrList
  };
  return servers;
}

/** default server, 0 to disable */
const char* AmazonImporter::defaultServer() const { return "www.amazon.com"; }

/** anchor to online help, 0 to disable */
const char* AmazonImporter::helpAnchor() const { return "import-amazon"; }

/** configuration, 0 if not used */
ServerImporterConfig* AmazonImporter::config() const { return &AmazonConfig::instance(); }

/** additional tags option, false if not used */
bool AmazonImporter::additionalTags() const { return true; }

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void AmazonImporter::parseFindResults(const QByteArray& searchStr)
{
  /* products have the following format:
<a class="a-link-normal s-access-detail-page  a-text-normal" title="The Avenger" href="http://www.amazon.com/Avenger-AMON-AMARTH/dp/B001VROVHO/ref=sr_1_1?s=music&amp;ie=UTF8&amp;qid=1426338609&amp;sr=1-1">
(..)>by </span>(..)<a class="a-link-normal a-text-normal" href="/Amon-Amarth/e/B000APIBHO/ref=sr_ntt_srch_lnk_1?qid=1426338609&sr=1-1">Amon Amarth</a>
   */
  QString str = QString::fromUtf8(searchStr);
  QRegularExpression catIdTitleRe(
        QLatin1String(R"(href="[^"]+/(dp|ASIN|images|product|-)/([A-Z0-9]+))"
            R"([^"]+">.*<span[^>]*>([^<]+)</span>)"
            R"((?:[\s\n]*(?:</a>|</h2>|<div[^>]*>|<span[^>]*>))*by </span>)"
            R"([\s\n]*<(?:a|span)[^>]*>([^<]+)</)"));

  str.remove(QLatin1Char('\r'));
  m_albumListModel->clear();
  auto it = catIdTitleRe.globalMatch(str);
  while (it.hasNext()) {
    auto match = it.next();
    QString category = match.captured(1);
    QString id = match.captured(2);
    QString artistTitle = replaceHtmlEntities(
          match.captured(4).trimmed() + QLatin1String(" - ") +
          removeExplicit(match.captured(3).trimmed()));
    m_albumListModel->appendItem(artistTitle, category, id);
  }
}

/**
 * Parse result of album request and populate m_trackDataModel with results.
 *
 * @param albumStr album data received
 */
void AmazonImporter::parseAlbumResults(const QByteArray& albumStr)
{
  /*
<span id="productTitle" class="a-size-large product-title-word-break">        The Avenger         </span>
<span class="author(..)<a class="a-link-normal" href="/Amon-Amarth/e/B000APIBHO/ref=dp_byline_cont_music_1">Amon Amarth</a>

<h2>Track Listings</h2>(..)<tr> <td>1</td> <td>Bleed for Ancient Gods</td> </tr>
  <tr> <td>2</td> <td>The Last with Pagan Blood</td>(..)

<h2>Product details</h2>(..)
<span class="a-text-bold">Manufacturer(..)</span> <span>Metal Blade</span>
<span class="a-text-bold">Date First Available(..)</span> <span>April 4, 2009</span>
   */
  QString str = QString::fromUtf8(albumStr);
  FrameCollection framesHdr;
  const bool standardTags = getStandardTags();
  // search for 'dmusicProductTitle', next element after '>' until ' [' or '<' => album
  int end = 0;
  int start = str.indexOf(
      QLatin1String("<span id=\"productTitle\""));
  if (start >= 0 && standardTags) {
    start = str.indexOf(QLatin1Char('>'), start + 23);
    if (start >= 0) {
      end = str.indexOf(QLatin1Char('<'), start);
      if (end > start) {
        int bracketPos = str.indexOf(QLatin1String(" ["), start);
        if (bracketPos >= 0 && bracketPos < end) {
          end = bracketPos;
        }
        framesHdr.setAlbum(
              replaceHtmlEntities(str.mid(start + 1, end - start - 1)
                                  .trimmed()));
        // next 'ArtistLinkSection'
        start = str.indexOf(QLatin1String("<span class=\"author"), end);
        if (start >= 0) {
          end = str.indexOf(QLatin1Char('>'), start);
          if (end > start) {

            // next '<a', text after '>' until '<' => artist
            start = str.indexOf(QLatin1String("<a"), end);
            if (start >= 0) {
              start = str.indexOf(QLatin1Char('>'), start);
              if (start >= 0) {
                end = str.indexOf(QLatin1Char('<'), start);
                if (end > start) {
                  framesHdr.setArtist(
                      replaceHtmlEntities(str.mid(start + 1, end - start - 1)
                                          .trimmed()));
                }
              }
            }

          }
        }
      }
    }
  }

  // search for >Product Details<, >Original Release Date:<, >Label:<
  const bool additionalTags = getAdditionalTags();
  QString albumArtist;
  start = str.indexOf(QLatin1String(">Product details<"));
  if (start >= 0) {
    QRegularExpression yearRe(
          QLatin1String(R"(>Date First Available.*?)"
                        R"(<span>[^<]*(\d{4})[^<]*</span>)"),
          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression labelRe(
          QLatin1String(R"(>Manufacturer.*?<span>([^<]+)</span>)"),
          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;
    if (additionalTags) {
      match = yearRe.match(str, start);
      if (match.hasMatch()) {
        framesHdr.setYear(match.captured(1).toInt());
      }
      match = labelRe.match(str, start);
      if (match.hasMatch()) {
        framesHdr.setValue(Frame::FT_Publisher, removeHtml(match.captured(1)));
      }
    }
  }

  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  trackDataVector.setCoverArtUrl(QUrl());
  if (getCoverArt()) {
    QRegularExpression imgSrcRe(
          QLatin1String("id=\"imgTagWrapperId\"[^>]*>\\s*"
                        "<img[^>]*src=\"([^\"]+)\""),
          QRegularExpression::DotMatchesEverythingOption);
    auto match = imgSrcRe.match(str);
    if (match.hasMatch()) {
      trackDataVector.setCoverArtUrl(QUrl(match.captured(1)));
    }
  }

  start = str.indexOf(QLatin1String("<h2>Track Listings</h2>"));
  if (start >= 0) {
    QRegularExpression trackNumberTitleRe(
          QLatin1String(R"(<td>(\d+)</td>\s*<td>([^<]+?)(?:\s*\[?(\d+):(\d+)\]?\s*)?</td>)"));
    FrameCollection frames(framesHdr);
    auto it = trackDataVector.begin();
    bool atTrackDataListEnd = (it == trackDataVector.end());
    while (start >= 0) {
      start = str.indexOf(QLatin1String("<tr"), start);
      if (start >= 0) {
        end = str.indexOf(QLatin1String("</tr>"), start);
        if (end > start) {
          QString trackRow = str.mid(start, end - start);
          start = end + 5;
          QString title;
          int trackNr = 0;
          int duration = 0;
          auto match = trackNumberTitleRe.match(trackRow);
          if (match.hasMatch()) {
            trackNr = match.captured(1).toInt();
            title = match.captured(2).remove(QLatin1String("[*]")).trimmed();
            duration = match.captured(3).toInt() * 60 +
              match.captured(4).toInt();
          }
          if (!title.isEmpty()) {
            if (standardTags) {
              frames.setTitle(removeExplicit(replaceHtmlEntities(title)));
              frames.setTrack(trackNr);
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
            frames = framesHdr;
          }
        }
      }
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
  } else if (!framesHdr.empty()) {
    // if there are no track data, fill frame header data
    for (auto it = trackDataVector.begin(); it != trackDataVector.end(); ++it) {
      if (it->isEnabled()) {
        (*it).setFrameCollection(framesHdr);
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
void AmazonImporter::sendFindQuery(
  const ServerImporterConfig* cfg,
  const QString& artist, const QString& album)
{
  // If an URL is entered in the first search field, its result will be directly
  // available in the album results list.
  if (artist.startsWith(QLatin1String("https://www.amazon.com/"))) {
    const int catBegin = 23;
    int catEnd = artist.indexOf(QLatin1Char('/'), catBegin);
    if (catEnd > catBegin) {
      m_albumListModel->clear();
      m_albumListModel->appendItem(
            artist,
            artist.mid(catBegin, catEnd - catBegin),
            artist.mid(catEnd + 1));
      return;
    }
  }
  /*
   * Query looks like this:
   * http://www.amazon.com/gp/search/ref=sr_adv_m_pop/?search-alias=popular&field-artist=amon+amarth&field-title=the+avenger
   */
  sendRequest(cfg->server(),
              QLatin1String("/s?i=music-intl-ship&k=") +
              encodeUrlQuery(artist + QLatin1Char(' ') + album),
              QLatin1String("https"), m_headers);
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void AmazonImporter::sendTrackListQuery(
  const ServerImporterConfig* cfg, const QString& cat, const QString& id)
{
  /*
   * Query looks like this:
   * http://www.amazon.com/dp/B001VROVHO
   */
  sendRequest(cfg->server(), QLatin1Char('/') + cat + QLatin1Char('/') + id,
              QLatin1String("https"), m_headers);
}
