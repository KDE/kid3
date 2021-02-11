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
            R"([^"]+">[\s\n]*<span[^>]*>([^<]+)</span>)"
            R"((?:[\s\n]*(?:</a>|</h2>|<div[^>]*>|<span[^>]*>))*by </span>)"
            R"([\s\n]*<(?:a|span)[^>]*>([^<]+)</)"));
  QRegularExpression nextElementRe(QLatin1String(">([^<]+)<"));

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
    title (empty lines removed):
<span id="productTitle" class="a-size-large">The Avenger</span>
<span class="author notFaded" data-width="">
<a class="a-link-normal" href="/Amon-Amarth/e/B000APIBHO/ref=dp_byline_cont_music_1">Amon Amarth</a>
</span>

    details (empty lines removed):
<a name="productDetails" id="productDetails"></a>
<hr noshade="noshade" size="1" class="bucketDivider" />
<table cellpadding="0" cellspacing="0" border="0">
  <tr>
    <td class="bucket">
<h2>Product Details</h2>
  <div class="content">
<ul>
<li><b>Audio CD</b>  (November 2, 1999)</li>
<li><b>Original Release Date:</b> November 2, 1999</li>
<li><b>Number of Discs:</b> 1</li>
<li><b>Label:</b> Metal Blade</li>

    tracks:
<tr class='rowEven'><td class="playCol"><a href="/gp/dmusic/media/sample.m3u/ref=dm_mu_dp_trk1_smpl/175-1810673-7649752?ie=UTF8&catalogItemType=track&ASIN=B0016OAHCK&DownloadLocation=CD" onclick='return cd_trackPreviewPressed("B0016OAHCK");'><img src="http://g-ecx.images-amazon.com/images/G/01/digital/music/dp/play-control-2._V223646478_.gif" width="19" alt="listen" id="cd_trackPreviewB0016OAHCK" title="listen" height="19" border="0" /></a></td><td class="titleCol">&nbsp; 1. <a href="http://www.amazon.com/gp/product/B0016OAHCK/ref=dm_mu_dp_trk1/175-1810673-7649752">Bleed For Ancient Gods</a></td><td class="runtimeCol"> 4:31</td><td class="priceCol">$0.99</td><td class="buyCol">

    alternatively (empty lines removed):
<tr class="listRowEven">
<td>
1. Before the Devil Knows You're Dead
</td>
   */
  QString str = QString::fromUtf8(albumStr);
  FrameCollection framesHdr;
  const bool standardTags = getStandardTags();
  // search for 'dmusicProductTitle', next element after '>' until ' [' or '<' => album
  int end = 0;
  int start = str.indexOf(
      QLatin1String("data-feature-name=\"dmusicProductTitle\""));
  if (start >= 0 && standardTags) {
    start = str.indexOf(QLatin1Char('>'), start + 39);
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
        start = str.indexOf(QLatin1String("id=\"ArtistLinkSection"), end);
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
          QLatin1String(R"(>Date First Available\s*:.*?)"
                        R"(<span>[^<]*(\d{4})[^<]*</span>)"),
          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression labelRe(
          QLatin1String(R"(>Manufacturer\s*:.*?<span>([^<]+)</span>)"),
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
          QLatin1String("data-feature-name=\"digitalMusicProductImage\">\\s*"
                        "<img[^>]*src=\"([^\"]+)\""),
          QRegularExpression::DotMatchesEverythingOption);
    auto match = imgSrcRe.match(str);
    if (match.hasMatch()) {
      trackDataVector.setCoverArtUrl(QUrl(match.captured(1)));
    }
  }

  start = str.indexOf(QLatin1String("id=\"dmusic_tracklist"));
  if (start >= 0) {
    QRegularExpression trackNumberRe(
          QLatin1String(R"(id="trackNumber.*?>(\d+)</div>)"),
          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression titleRe(
          QLatin1String(R"(id="dmusic_tracklist_track_title.*?>([^<]+)</a>)"),
          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression durationRe(
          QLatin1String(R"(id="dmusic_tracklist_duration.*?>\s*(\d+):(\d+)\s*</span>)"),
          QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression artistRe(
          QLatin1String(R"(>\s*by\s*</span>[^>]+>([^<]+)</)"));
    FrameCollection frames(framesHdr);
    auto it = trackDataVector.begin();
    bool atTrackDataListEnd = (it == trackDataVector.end());
    while (start >= 0) {
      start = str.indexOf(QLatin1String("<tr id=\"dmusic_tracklist_player_row"),
                          start);
      if (start >= 0) {
        end = str.indexOf(QLatin1String("</tr>"), start);
        if (end > start) {
          QString trackRow = str.mid(start, end - start);
          start = end + 5;
          QString title, artist;
          int trackNr = 0;
          int duration = 0;
          auto match = trackNumberRe.match(trackRow);
          if (match.hasMatch()) {
            trackNr = match.captured(1).toInt();
          }
          match = titleRe.match(trackRow);
          if (match.hasMatch()) {
            title = match.captured(1).trimmed();
          }
          match = durationRe.match(trackRow);
          if (match.hasMatch()) {
            duration = match.captured(1).toInt() * 60 +
              match.captured(2).toInt();
          }
          match = artistRe.match(trackRow);
          if (match.hasMatch()) {
            artist = match.captured(1).trimmed();
          }
          if (!title.isEmpty()) {
            if (standardTags) {
              frames.setTitle(removeExplicit(replaceHtmlEntities(title)));
              frames.setTrack(trackNr);
              if (!artist.isEmpty()) {
                frames.setArtist(replaceHtmlEntities(artist));
                if (additionalTags) {
                  frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
                }
              }
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
  /*
   * Query looks like this:
   * http://www.amazon.com/gp/search/ref=sr_adv_m_pop/?search-alias=popular&field-artist=amon+amarth&field-title=the+avenger
   */
  sendRequest(cfg->server(),
              QLatin1String("/s?i=digital-music&k=") +
              encodeUrlQuery(artist + QLatin1Char(' ') + album),
              QLatin1String("https"));
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
              QLatin1String("https"));
}
