/**
 * \file amazonimporter.cpp
 * Amazon database importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Dec 2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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
#include <QRegExp>
#include <QDomDocument>
#include "trackdatamodel.h"
#include "amazonconfig.h"

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
 * Destructor.
 */
AmazonImporter::~AmazonImporter()
{
}

/**
 * Name of import source.
 * @return name.
 */
const char* AmazonImporter::name() const { return QT_TRANSLATE_NOOP("@default", "Amazon"); }

/** NULL-terminated array of server strings, 0 if not used */
const char** AmazonImporter::serverList() const
{
  static const char* servers[] = {
    // Parsing only works with English text
    "www.amazon.com:80",
    "www.amazon.co.uk:80",
    0                  // end of StrList
  };
  return servers;
}

/** default server, 0 to disable */
const char* AmazonImporter::defaultServer() const { return "www.amazon.com:80"; }

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
  QRegExp catIdTitleRe(QLatin1String(
    "<a class=\"[^\"]*s-access-detail-page[^\"]*\"[^>]+title=\"([^\"]+)\"[^>]+"
    "href=\"[^\"]+/(dp|ASIN|images|product|-)/([A-Z0-9]+)[^\"]+\">"));
  QRegExp nextElementRe(QLatin1String(">([^<]+)<"));

  str.remove(QLatin1Char('\r'));
  m_albumListModel->clear();
  int end = 0;
  for (;;) {
    int start = catIdTitleRe.indexIn(str, end);
    if (start == -1)
      break;
    end = start + catIdTitleRe.matchedLength();
    start = str.indexOf(QLatin1String(">by <"), end);
    if (start == -1)
      break;
    end = start + 5;
    start = nextElementRe.indexIn(str, end);
    if (start == -1)
      break;
    end = start + nextElementRe.matchedLength();
    m_albumListModel->appendRow(new AlbumListItem(
      nextElementRe.cap(1) + QLatin1String(" - ") +
      catIdTitleRe.cap(1),
      catIdTitleRe.cap(2),
      catIdTitleRe.cap(3)));
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
  // search for 'id="productTitle"', text after '>' until ' [' or '<' => album
  int end = 0;
  int start = str.indexOf(QLatin1String("id=\"productTitle\""));
  if (start >= 0 && standardTags) {
    start = str.indexOf(QLatin1Char('>'), start);
    if (start >= 0) {
      end = str.indexOf(QLatin1Char('<'), start);
      if (end > start) {
        int bracketPos = str.indexOf(QLatin1String(" ["), start);
        if (bracketPos >= 0 && bracketPos < end) {
          end = bracketPos;
        }
        framesHdr.setAlbum(
              replaceHtmlEntities(str.mid(start + 1, end - start - 1)));
        // next 'class="author'
        start = str.indexOf(QLatin1String("class=\"author"), end);
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
                      replaceHtmlEntities(str.mid(start + 1, end - start - 1)));
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
  start = str.indexOf(QLatin1String(">Product Details<"));
  if (start >= 0) {
    int detailStart = str.indexOf(QLatin1String(">Original Release Date:<"), start);
    if (detailStart < 0) {
      detailStart  = str.indexOf(QLatin1String(">Audio CD<"), start);
    }
    if (detailStart >= 0 && standardTags) {
      int detailEnd = str.indexOf(QLatin1Char('\n'), detailStart + 10);
      if (detailEnd > detailStart + 10) {
        QRegExp yearRe(QLatin1String("(\\d{4})"));
        if (yearRe.indexIn(
              str.mid(detailStart + 10, detailEnd - detailStart - 11)) >= 0) {
          framesHdr.setYear(yearRe.cap(1).toInt());
        }
      }
    }
    if (additionalTags) {
      detailStart = str.indexOf(QLatin1String(">Label:<"), start);
      if (detailStart > 0) {
        int detailEnd = str.indexOf(QLatin1Char('\n'), detailStart + 8);
        if (detailEnd > detailStart + 8) {
          QRegExp labelRe(QLatin1String(">\\s*([^<]+)<"));
          if (labelRe.indexIn(
                str.mid(detailStart + 8, detailEnd - detailStart - 9)) >= 0) {
            framesHdr.setValue(Frame::FT_Publisher, removeHtml(labelRe.cap(1)));
          }
        }
      }
      detailStart = str.indexOf(QLatin1String(">Performer:<"), start);
      if (detailStart > 0) {
        int detailEnd = str.indexOf(QLatin1String("</li>"), detailStart + 12);
        if (detailEnd > detailStart + 12) {
          framesHdr.setValue(
            Frame::FT_Performer,
            removeHtml(str.mid(detailStart + 11, detailEnd - detailStart - 11)));
        }
      }
      detailStart = str.indexOf(QLatin1String(">Orchestra:<"), start);
      if (detailStart > 0) {
        int detailEnd = str.indexOf(QLatin1String("</li>"), detailStart + 12);
        if (detailEnd > detailStart + 12) {
          albumArtist =
            removeHtml(str.mid(detailStart + 11, detailEnd - detailStart - 11));
        }
      }
      detailStart = str.indexOf(QLatin1String(">Conductor:<"), start);
      if (detailStart > 0) {
        int detailEnd = str.indexOf(QLatin1String("</li>"), detailStart + 12);
        if (detailEnd > detailStart + 12) {
          framesHdr.setValue(
            Frame::FT_Conductor,
            removeHtml(str.mid(detailStart + 11, detailEnd - detailStart - 11)));
        }
      }
      detailStart = str.indexOf(QLatin1String(">Composer:<"), start);
      if (detailStart > 0) {
        int detailEnd = str.indexOf(QLatin1String("</li>"), detailStart + 11);
        if (detailEnd > detailStart + 11) {
          framesHdr.setValue(
            Frame::FT_Composer,
            removeHtml(str.mid(detailStart + 10, detailEnd - detailStart - 10)));
        }
      }
    }
  }

  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  trackDataVector.setCoverArtUrl(QUrl());
  if (getCoverArt()) {
    // <input type="hidden" id="ASIN" name="ASIN" value="B0025AY48W" />
    start = str.indexOf(QLatin1String("id=\"ASIN\""));
    if (start > 0) {
      start = str.indexOf(QLatin1String("value=\""), start);
      if (start > 0) {
        end = str.indexOf(QLatin1Char('"'), start + 7);
        if (end > start) {
          trackDataVector.setCoverArtUrl(
            QUrl(QLatin1String("http://www.amazon.com/dp/") +
            str.mid(start + 7, end - start - 7)));
        }
      }
    }
  }

  bool hasTitleCol = false, hasListRow = false;
  bool hasArtist = str.indexOf(QLatin1String("<td>Song Title</td><td>Artist</td>")) != -1;
  // search 'class="titleCol"', next '<a href=', text after '>' until '<'
  // => title
  // if not found: alternatively look for 'class="listRow'
  start = str.indexOf(QLatin1String("class=\"titleCol\""));
  if (start >= 0) {
    hasTitleCol = true;
  } else if ((start = str.indexOf(QLatin1String("class=\"listRow"))) >= 0) {
    hasListRow = true;
  } else {
    start = str.indexOf(QLatin1String("id=\"a-popover-trackTitlePopover"));
  }
  if (start >= 0) {
    QRegExp durationRe(QLatin1String("(\\d+):(\\d+)"));
    QRegExp nrTitleRe(QLatin1String("\\s*\\d+\\.\\s+(.*\\S)"));
    FrameCollection frames(framesHdr);
    ImportTrackDataVector::iterator it = trackDataVector.begin();
    bool atTrackDataListEnd = (it == trackDataVector.end());
    int trackNr = 1;
    while (start >= 0) {
      QString title;
      QString artist;
      int duration = 0;
      if (hasTitleCol) {
        end = str.indexOf(QLatin1Char('\n'), start);
        if (end > start) {
          QString line = str.mid(start, end - start);
          int titleStart = line.indexOf(QLatin1String("<a href="));
          if (titleStart >= 0) {
            titleStart = line.indexOf(QLatin1Char('>'), titleStart);
            if (titleStart >= 0) {
              int titleEnd = line.indexOf(QLatin1Char('<'), titleStart);
              if (titleEnd > titleStart) {
                title = line.mid(titleStart + 1, titleEnd - titleStart - 1);
                // if there was an Artist title,
                // search for artist in a second titleCol
                if (hasArtist) {
                  int artistStart =
                    line.indexOf(QLatin1String("class=\"titleCol\""), titleEnd);
                  if (artistStart >= 0) {
                    artistStart = line.indexOf(QLatin1String("<a href="), artistStart);
                    if (artistStart >= 0) {
                      artistStart = line.indexOf(QLatin1Char('>'), artistStart);
                      if (artistStart >= 0) {
                        int artistEnd = line.indexOf(QLatin1Char('<'), artistStart);
                        if (artistEnd > artistStart) {
                          artist = line.mid(
                            artistStart + 1, artistEnd - artistStart - 1);
                          if (albumArtist.isEmpty()) {
                            albumArtist = frames.getArtist();
                          }
                        }
                      }
                    }
                  }
                }
                // search for next 'class="', if it is 'class="runtimeCol"',
                // text after '>' until '<' => duration
                int runtimeStart =
                  line.indexOf(QLatin1String("class=\"runtimeCol\""), titleEnd);
                if (runtimeStart >= 0) {
                  runtimeStart = line.indexOf(QLatin1Char('>'), runtimeStart + 18);
                  if (runtimeStart >= 0) {
                    int runtimeEnd = line.indexOf(QLatin1Char('<'), runtimeStart);
                    if (runtimeEnd > runtimeStart) {
                      if (durationRe.indexIn(
                            line.mid(runtimeStart + 1,
                                     runtimeEnd - runtimeStart - 1)) >= 0) {
                        duration = durationRe.cap(1).toInt() * 60 +
                          durationRe.cap(2).toInt();
                      }
                    }
                  }
                }
                start = str.indexOf(QLatin1String("class=\"titleCol\""), end);
              } else {
                start = -1;
              }
            }
          }
        }
      } else if (hasListRow) {
        // 'class="listRow' found
        start = str.indexOf(QLatin1String("<td>"), start);
        if (start >= 0) {
          end = str.indexOf(QLatin1String("</td>"), start);
          if (end > start &&
              nrTitleRe.indexIn(str.mid(start + 4, end - start - 4)) >= 0) {
            title = nrTitleRe.cap(1);
            start = str.indexOf(QLatin1String("class=\"listRow"), end);
          } else {
            start = -1;
          }
        }
      } else {
        // a-popover-trackTitlePopover id found
        start = str.indexOf(QLatin1String("<a"), start);
        if (start >= 0) {
          start = str.indexOf(QLatin1Char('>'), start);
          if (start >= 0) {
            end = str.indexOf(QLatin1Char('<'), start);
            if (end >= start) {
              title = str.mid(start + 1, end - start - 1);
              int runtimeStart = str.indexOf(
                    QLatin1String("<td id=\"dmusic_tracklist_duration"), end);
              if (runtimeStart >= 0) {
                int runtimeEnd = str.indexOf(QLatin1String("</td>"),
                                             runtimeStart);
                if (runtimeEnd > runtimeStart &&
                    durationRe.indexIn(
                      str.mid(runtimeStart + 1, runtimeEnd - runtimeStart - 1).
                      remove(QLatin1Char('\n')).remove(QLatin1Char('\r')))
                    >= 0) {
                  duration = durationRe.cap(1).toInt() * 60 +
                      durationRe.cap(2).toInt();
                }
              }
              start = str.indexOf(
                    QLatin1String("id=\"a-popover-trackTitlePopover"), end);
            } else {
              start = -1;
            }
          }
        }
      }
      if (!title.isEmpty()) {
        if (standardTags) {
          frames.setTitle(replaceHtmlEntities(title));
          if (!artist.isEmpty()) {
            frames.setArtist(replaceHtmlEntities(artist));
          }
          frames.setTrack(trackNr);
        }
        if (!albumArtist.isEmpty() && additionalTags) {
          frames.setValue(Frame::FT_AlbumArtist, albumArtist);
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
        frames = framesHdr;
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
    for (ImportTrackDataVector::iterator it = trackDataVector.begin();
         it != trackDataVector.end();
         ++it) {
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
              QLatin1String("/gp/search/ref=sr_adv_m_pop/"
                      "?search-alias=popular&field-artist=") +
              encodeUrlQuery(artist) + QLatin1String("&field-title=") + encodeUrlQuery(album));
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
  sendRequest(cfg->server(), QLatin1Char('/') + cat + QLatin1Char('/') + id);
}
