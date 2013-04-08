/**
 * \file importconfig.cpp
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "importconfig.h"
#include <QString>
#include "config.h"

namespace {

/**
 * Convert tag version to import destination value in configuration.
 * @param tagVersion tag version
 * @return value used in configuration, kept for backwards compatibility.
 */
inline int tagVersionToImportDestCfg(TrackData::TagVersion tagVersion) {
  return static_cast<int>(tagVersion) - 1;
}

/**
 * Convert import destination value in configuration to tag version.
 * @param importDest value used in configuration, kept for backwards
 *                   compatibility.
 * @return tag version.
 */
inline TrackData::TagVersion importDestCfgToTagVersion(int importDest) {
  return TrackData::tagVersionCast(importDest + 1);
}

}

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
ImportConfig::ImportConfig(const QString& grp) :
  GeneralConfig(grp), m_importServer(0),
  m_importDest(TrackData::TagV1), m_importFormatIdx(0),
  m_enableTimeDifferenceCheck(true), m_maxTimeDifference(3),
  m_importVisibleColumns(0x2000000000ULL),
  m_importTagsIdx(0),
  m_exportSrcV1(TrackData::TagV1), m_exportFormatIdx(0),
  m_pictureSourceIdx(0)
{
  /**
   * Preset import format regular expressions.
   * The following codes are used before the () expressions.
   * %s %{title} title (song)
   * %l %{album} album
   * %a %{artist} artist
   * %c %{comment} comment
   * %y %{year} year
   * %t %{track} track, at least two digits
   * %T %{tracknumber} track number
   * %g %{genre} genre
   * %d %{duration} duration mm:ss
   * %D %{seconds} duration in seconds
   * %f %{file} file name
   * %p %{filepath} absolute file path
   * %u %{url} URL
   * %n %{tracks} number of tracks
   */
  m_importFormatNames.append(QLatin1String("CSV unquoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String("%{track}(\\d+)\\t%{title}([^\\r\\n\\t]*)\\t%{artist}([^\\r\\n\\t]*)\\t%{album}([^\\r\\n\\t]*)\\t%{year}(\\d+)\\t%{genre}([^\\r\\n\\t]*)\\t%{comment}([^\\r\\n\\t]*)\\t(?:\\d+:)?%{duration}(\\d+:\\d+)"));

  m_importFormatNames.append(QLatin1String("CSV quoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String("\"?%{track}(\\d+)\"?\\t\"?%{title}([^\\r\\n\\t\"]*)\"?\\t\"?%{artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{album}([^\\r\\n\\t\"]*)\"?\\t\"?%{year}(\\d+)\"?\\t\"?%{genre}([^\\r\\n\\t\"]*)\"?\\t\"?%{comment}([^\\r\\n\\t\"]*)\"?\\t\"?(?:\\d+:)?%{duration}(\\d+:\\d+)"));

  m_importFormatNames.append(QLatin1String("CSV more unquoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    "%{track}(\\d+)\\t%{title}([^\\r\\n\\t]*)\\t%{artist}([^\\r\\n\\t]*)\\t"
    "%{album}([^\\r\\n\\t]*)\\t%{year}(\\d+)\\t%{genre}([^\\r\\n\\t]*)\\"
    "t%{comment}([^\\r\\n\\t]*)\\t(?:\\d+:)?%{duration}(\\d+:\\d+)(?:\\.\\d+)?\\t"
    "%{album artist}([^\\r\\n\\t]*)\\t%{arranger}([^\\r\\n\\t]*)\\t"
    "%{author}([^\\r\\n\\t]*)\\t%{bpm}([^\\r\\n\\t]*)\\t"
    "%{composer}([^\\r\\n\\t]*)\\t%{conductor}([^\\r\\n\\t]*)\\t"
    "%{copyright}([^\\r\\n\\t]*)\\t%{disc number}([^\\r\\n\\t]*)\\t"
    "%{encoded-by}([^\\r\\n\\t]*)\\t%{grouping}([^\\r\\n\\t]*)\\t%{isrc}([^\\r\\n\\t]*)\\t"
    "%{language}([^\\r\\n\\t]*)\\t%{lyricist}([^\\r\\n\\t]*)\\t%{lyrics}([^\\r\\n\\t]*)\\t"
    "%{media}([^\\r\\n\\t]*)\\t%{original album}([^\\r\\n\\t]*)\\t"
    "%{original artist}([^\\r\\n\\t]*)\\t%{original date}([^\\r\\n\\t]*)\\t"
    "%{part}([^\\r\\n\\t]*)\\t%{performer}([^\\r\\n\\t]*)\\t"
    "%{publisher}([^\\r\\n\\t]*)\\t%{remixer}([^\\r\\n\\t]*)\\t"
    "%{subtitle}([^\\r\\n\\t]*)\\t%{website}([^\\r\\n\\t]*)"));

  m_importFormatNames.append(QLatin1String("CSV more quoted"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(
    "\"?%{track}(\\d+)\"?\\t\"?%{title}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{album}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{year}(\\d+)\"?\\t\"?%{genre}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{comment}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?(?:\\d+:)?%{duration}(\\d+:\\d+)(?:\\.\\d+)?\"?\\t"
    "\"?%{album artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{arranger}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{author}([^\\r\\n\\t\"]*)\"?\\t\"?%{bpm}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{composer}([^\\r\\n\\t\"]*)\"?\\t\"?%{conductor}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{copyright}([^\\r\\n\\t\"]*)\"?\\t\"?%{disc number}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{encoded-by}([^\\r\\n\\t\"]*)\"?\\t\"?%{grouping}([^\\r\\n\\t\"]*)\"?\\t\"?%{isrc}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{language}([^\\r\\n\\t\"]*)\"?\\t\"?%{lyricist}([^\\r\\n\\t\"]*)\"?\\t\"?%{lyrics}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{media}([^\\r\\n\\t\"]*)\"?\\t\"?%{original album}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{original artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{original date}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{part}([^\\r\\n\\t\"]*)\"?\\t\"?%{performer}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{publisher}([^\\r\\n\\t\"]*)\"?\\t\"?%{remixer}([^\\r\\n\\t\"]*)\"?\\t"
    "\"?%{subtitle}([^\\r\\n\\t\"]*)\"?\\t\"?%{website}([^\\r\\n\\t\"]*)"));

  m_importFormatNames.append(QLatin1String("freedb HTML text"));
  m_importFormatHeaders.append(QLatin1String("%{artist}(\\S[^\\r\\n/]*\\S)\\s*/\\s*%{album}(\\S[^\\r\\n]*\\S)[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%{year}(\\d+)?.*genre:\\s*%{genre}(\\S[^\\r\\n]*\\S)?[\\r\\n]"));
  m_importFormatTracks.append(QLatin1String("[\\r\\n]%{track}(\\d+)[\\.\\s]+%{duration}(\\d+:\\d+)\\s+%{title}(\\S[^\\r\\n]*\\S)"));

  m_importFormatNames.append(QLatin1String("freedb HTML source"));
  m_importFormatHeaders.append(QLatin1String("<[^>]+>%{artist}([^<\\s][^\\r\\n/]*\\S)\\s*/\\s*%{album}(\\S[^\\r\\n]*[^\\s>])<[^>]+>[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%{year}(\\d+)?.*genre:\\s*%{genre}(\\S[^\\r\\n>]*\\S)?<[^>]+>[\\r\\n]"));
  m_importFormatTracks.append(QLatin1String("<td[^>]*>\\s*%{track}(\\d+).</td><td[^>]*>\\s*%{duration}(\\d+:\\d+)</td><td[^>]*>(?:<[^>]+>)?%{title}([^<\\r\\n]+)"));

  m_importFormatNames.append(QLatin1String("Title"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String("\\s*%{title}(\\S[^\\r\\n]*\\S)\\s*"));

  m_importFormatNames.append(QLatin1String("Track Title"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String("\\s*%{track}(\\d+)[\\.\\s]+%{title}(\\S[^\\r\\n]*\\S)\\s*"));

  m_importFormatNames.append(QLatin1String("Track Title Time"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String("\\s*%{track}(\\d+)[\\.\\s]+%{title}(\\S[^\\r\\n]*\\S)\\s+%{duration}(\\d+:\\d+)\\s*"));

  m_importFormatNames.append(QLatin1String("Custom Format"));
  m_importFormatHeaders.append(QLatin1String(""));
  m_importFormatTracks.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("CSV unquoted"));
  m_exportFormatHeaders.append(QLatin1String(""));
  m_exportFormatTracks.append(QLatin1String("%{track}\\t%{title}\\t%{artist}\\t%{album}\\t%{year}\\t%{genre}\\t%{comment}\\t%{duration}.00"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("CSV quoted"));
  m_exportFormatHeaders.append(QLatin1String(""));
  m_exportFormatTracks.append(QLatin1String("\"%{track}\"\\t\"%{title}\"\\t\"%{artist}\"\\t\"%{album}\"\\t\"%{year}\"\\t\"%{genre}\"\\t\"%{comment}\"\\t\"%{duration}.00\""));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_importTagsNames.append(QLatin1String("Artist to Album Artist"));
  m_importTagsSources.append(QLatin1String("%{artist}"));
  m_importTagsExtractions.append(QLatin1String("%{albumartist}(.+)"));

  m_importTagsNames.append(QLatin1String("Album Artist to Artist"));
  m_importTagsSources.append(QLatin1String("%{albumartist}"));
  m_importTagsExtractions.append(QLatin1String("%{artist}(.+)"));

  m_importTagsNames.append(QLatin1String("Artist to Composer"));
  m_importTagsSources.append(QLatin1String("%{artist}"));
  m_importTagsExtractions.append(QLatin1String("%{composer}(.+)"));

  m_importTagsNames.append(QLatin1String("Artist to Conductor"));
  m_importTagsSources.append(QLatin1String("%{artist}"));
  m_importTagsExtractions.append(QLatin1String("%{conductor}(.+)"));

  m_importTagsNames.append(QLatin1String("Track Number from Title"));
  m_importTagsSources.append(QLatin1String("%{title}"));
  m_importTagsExtractions.append(QLatin1String("\\s*%{track}(\\d+)[\\.\\s]+%{title}(\\S.*\\S)\\s*"));

  m_importTagsNames.append(QLatin1String("Track Number to Title"));
  m_importTagsSources.append(QLatin1String("%{track} %{title}"));
  m_importTagsExtractions.append(QLatin1String("%{title}(.+)"));

  m_importTagsNames.append(QLatin1String("Subtitle from Title"));
  m_importTagsSources.append(QLatin1String("%{title}"));
  m_importTagsExtractions.append(QLatin1String("%{subtitle}(.+) - "));

  m_importTagsNames.append(QLatin1String("Title Annotation to Comment"));
  m_importTagsSources.append(QLatin1String("%{title}"));
  m_importTagsExtractions.append(QLatin1String("%{title}(.+) +\\(%{comment}((?:Bonus|Remix)[^)]*)\\)"));

  m_importTagsNames.append(QLatin1String("Custom Format"));
  m_importTagsSources.append(QLatin1String(""));
  m_importTagsExtractions.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("CSV more unquoted"));
  m_exportFormatHeaders.append(QLatin1String(
    "Track\\tTitle\\tArtist\\tAlbum\\tDate\\tGenre\\tComment\\tDuration\\t"
    "Album Artist\\tArranger\\tAuthor\\tBPM\\tComposer\\t"
    "Conductor\\tCopyright\\tDisc Number\\tEncoded-by\\tGrouping\\tISRC\\t"
    "Language\\tLyricist\\tLyrics\\tMedia\\tOriginal Album\\t"
    "Original Artist\\tOriginal Date\\tPart\\tPerformer\\t"
    "Publisher\\tRemixer\\tSubtitle\\tWebsite"));
  m_exportFormatTracks.append(QLatin1String(
    "%{track}\\t%{title}\\t%{artist}\\t%{album}\\t%{year}\\t%{genre}\\t%{comment}\\t"
    "%{duration}.00\\t"
    "%{album artist}\\t%{arranger}\\t%{author}\\t%{bpm}\\t%{composer}\\t"
    "%{conductor}\\t%{copyright}\\t%{disc number}\\t%{encoded-by}\\t%{grouping}\\t%{isrc}\\t"
    "%{language}\\t%{lyricist}\\t%{lyrics}\\t%{media}\\t%{original album}\\t"
    "%{original artist}\\t%{original date}\\t%{part}\\t%{performer}\\t"
    "%{publisher}\\t%{remixer}\\t%{subtitle}\\t%{website}"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("CSV more quoted"));
  m_exportFormatHeaders.append(QLatin1String(
    "\"Track\"\\t\"Title\"\\t\"Artist\"\\t\"Album\"\\t\"Date\"\\t"
    "\"Genre\"\\t\"Comment\"\\t\"Duration\"\\t"
    "\"Album Artist\"\\t\"Arranger\"\\t\"Author\"\\t\"BPM\"\\t"
    "\"Composer\"\\t\"Conductor\"\\t\"Copyright\"\\t\"Disc Number\"\\t"
    "\"Encoded-by\"\\t\"Grouping\"\\t\"ISRC\"\\t\"Language\"\\t\"Lyricist\"\\t\"Lyrics\"\\t"
    "\"Media\"\\t\"Original Album\"\\t\"Original Artist\"\\t"
    "\"Original Date\"\\t\"Part\"\\t\"Performer\"\\t\"Publisher\"\\t"
    "\"Remixer\"\\t\"Subtitle\"\\t\"Website\""));
  m_exportFormatTracks.append(QLatin1String(
    "\"%{track}\"\\t\"%{title}\"\\t\"%{artist}\"\\t\"%{album}\"\\t\"%{year}\"\\t"
    "\"%{genre}\"\\t\"%{comment}\"\\t\"%{duration}.00\"\\t"
    "\"%{album artist}\"\\t\"%{arranger}\"\\t\"%{author}\"\\t\"%{bpm}\"\\t"
    "\"%{composer}\"\\t\"%{conductor}\"\\t\"%{copyright}\"\\t\"%{disc number}\"\\t"
    "\"%{encoded-by}\"\\t\"%{grouping}\"\\t\"%{isrc}\"\\t\"%{language}\"\\t\"%{lyricist}\"\\t\"%{lyrics}\"\\t"
    "\"%{media}\"\\t\"%{original album}\"\\t\"%{original artist}\"\\t"
    "\"%{original date}\"\\t\"%{part}\"\\t\"%{performer}\"\\t\"%{publisher}\"\\t"
    "\"%{remixer}\"\\t\"%{subtitle}\"\\t\"%{website}\""));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("Extended M3U"));
  m_exportFormatHeaders.append(QLatin1String("#EXTM3U"));
  m_exportFormatTracks.append(QLatin1String("#EXTINF:%{seconds},%{artist} - %{title}\\n%{filepath}"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("Extended PLS"));
  m_exportFormatHeaders.append(QLatin1String("[playlist]"));
  m_exportFormatTracks.append(QLatin1String("File%{tracknumber}=%{filepath}\\nTitle%{tracknumber}=%{artist} - %{title}\\nLength%{tracknumber}=%{seconds}"));
  m_exportFormatTrailers.append(QLatin1String("NumberOfEntries=%{tracks}\\nVersion=2"));

  m_exportFormatNames.append(QLatin1String("HTML"));
  m_exportFormatHeaders.append(QLatin1String("<html>\\n <head>\\n  <title>%{artist} - %{album}</title>\\n </head>\\n <body>\\n  <h1>%{artist} - %{album}</h1>\\n  <dl>"));
  m_exportFormatTracks.append(QLatin1String("   <dt><a href=\"%{url}\">%{track}. %{title}</a></dt>"));
  m_exportFormatTrailers.append(QLatin1String("  </dl>\\n </body>\\n</html>"));

  m_exportFormatNames.append(QLatin1String("Kover XML"));
  m_exportFormatHeaders.append(QLatin1String("<kover>\\n <title>\\n  <text><![CDATA[%{artist} ]]></text>\\n  <text><![CDATA[%{album}]]></text>\\n </title>\\n <content>"));
  m_exportFormatTracks.append(QLatin1String("  <text><![CDATA[%{track}. %{title}]]></text>"));
  m_exportFormatTrailers.append(QLatin1String(" </content>\\n</kover>"));

  m_exportFormatNames.append(QLatin1String("Technical Details"));
  m_exportFormatHeaders.append(QLatin1String("File\\tBitrate\\tVBR\\tDuration\\tSamplerate\\tChannels\\tMode\\tCodec"));
  m_exportFormatTracks.append(QLatin1String("%{file}\\t%{bitrate}\\t%{vbr}\\t%{duration}\\t%{samplerate}\\t%{channels}\\t%{mode}\\t%{codec}"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("Custom Format"));
  m_exportFormatHeaders.append(QLatin1String(""));
  m_exportFormatTracks.append(QLatin1String(""));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_pictureSourceNames.append(QLatin1String("Google Images"));
  m_pictureSourceUrls.append(QLatin1String("http://images.google.com/images?q=%u{artist}%20%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Yahoo Images"));
  m_pictureSourceUrls.append(QLatin1String("http://images.search.yahoo.com/search/images?ei=UTF-8&p=%u{artist}%20%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon"));
  m_pictureSourceUrls.append(QLatin1String("http://www.amazon.com/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon.co.uk"));
  m_pictureSourceUrls.append(QLatin1String("http://www.amazon.co.uk/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon.de"));
  m_pictureSourceUrls.append(QLatin1String("http://www.amazon.de/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Amazon.fr"));
  m_pictureSourceUrls.append(QLatin1String("http://www.amazon.fr/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("MusicBrainz"));
  m_pictureSourceUrls.append(QLatin1String("http://musicbrainz.org/search/textsearch.html?query=%u{artist}+%u{album}&type=release"));
  m_pictureSourceNames.append(QLatin1String("Discogs"));
  m_pictureSourceUrls.append(QLatin1String("http://www.discogs.com/search?q=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("CD Universe"));
  m_pictureSourceUrls.append(QLatin1String("http://www.cduniverse.com/sresult.asp?HT_Search_Info=%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Coveralia"));
  m_pictureSourceUrls.append(QLatin1String("http://www.coveralia.com/mostrar.php?bus=%u{artist}%20%u{album}&bust=2"));
  m_pictureSourceNames.append(QLatin1String("FreeCovers"));
  m_pictureSourceUrls.append(QLatin1String("http://www.freecovers.net/search.php?search=%u{artist}+%u{album}&cat=4"));
  m_pictureSourceNames.append(QLatin1String("CoverHunt"));
  m_pictureSourceUrls.append(QLatin1String("http://www.coverhunt.com/search/%u{artist}+%u{album}"));
  m_pictureSourceNames.append(QLatin1String("SlothRadio"));
  m_pictureSourceUrls.append(QLatin1String("http://www.slothradio.com/covers/?artist=%u{artist}&album=%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Albumart"));
  m_pictureSourceUrls.append(QLatin1String("http://www.albumart.org/index.php?srchkey=%u{artist}+%u{album}&searchindex=Music"));
  m_pictureSourceNames.append(QLatin1String("Yalp!"));
  m_pictureSourceUrls.append(QLatin1String("http://search.yalp.alice.it/search/search.html?txtToSearch=%u{artist}%20%u{album}"));
  m_pictureSourceNames.append(QLatin1String("HMV"));
  m_pictureSourceUrls.append(QLatin1String("http://hmv.com/hmvweb/advancedSearch.do?searchType=2&artist=%u{artist}&title=%u{album}"));
  m_pictureSourceNames.append(QLatin1String("CD Baby"));
  m_pictureSourceUrls.append(QLatin1String("http://cdbaby.com/found?artist=%u{artist}&album=%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Jamendo"));
  m_pictureSourceUrls.append(QLatin1String("http://www.jamendo.com/en/search/all/%u{artist}%20%u{album}"));
  m_pictureSourceNames.append(QLatin1String("Custom Source"));
  m_pictureSourceUrls.append(QLatin1String(""));

  m_matchPictureUrlMap[QLatin1String("http://images.google.com/.*imgurl=([^&]+)&.*")] =
    QLatin1String("\\1");
  m_matchPictureUrlMap[QLatin1String("http://rds.yahoo.com/.*%26imgurl=((?:[^%]|%(?!26))+).*")] =
    QLatin1String("http%253A%252F%252F\\1");
  m_matchPictureUrlMap[QLatin1String("http://rds.yahoo.com/.*&imgurl=([^&]+)&.*")] =
    QLatin1String("http%3A%2F%2F\\1");
  m_matchPictureUrlMap[QLatin1String("http://(?:www.)?amazon.(?:com|co.uk|de|fr).*/(?:dp|ASIN|images|product|-)/([A-Z0-9]+).*")] =
    QLatin1String("http://images.amazon.com/images/P/\\1.01._SCLZZZZZZZ_.jpg");
  m_matchPictureUrlMap[QLatin1String("http://musicbrainz.org/misc/redirects/.*&asin=([A-Z0-9]+).*")] =
    QLatin1String("http://images.amazon.com/images/P/\\1.01._SCLZZZZZZZ_.jpg");
  m_matchPictureUrlMap[QLatin1String("http://www.freecovers.net/view/(\\d+)/([0-9a-f]+)/.*")] =
    QLatin1String("http://www.freecovers.net/preview/\\1/\\2/big.jpg");
  m_matchPictureUrlMap[QLatin1String("http://cdbaby.com/cd/(\\w)(\\w)(\\w+)")] =
    QLatin1String("http://cdbaby.name/\\1/\\2/\\1\\2\\3.jpg");
  m_matchPictureUrlMap[QLatin1String("http://www.jamendo.com/en/album/(\\d+)")] =
    QLatin1String("http://imgjam.com/albums/\\1/covers/1.0.jpg");
}

/**
 * Destructor.
 */
ImportConfig::~ImportConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void ImportConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("ImportServer"), QVariant(m_importServer));
  config->setValue(QLatin1String("ImportDestination"),
                   QVariant(tagVersionToImportDestCfg(m_importDest)));
  config->setValue(QLatin1String("ImportFormatNames"), QVariant(m_importFormatNames));
  config->setValue(QLatin1String("ImportFormatHeaders"), QVariant(m_importFormatHeaders));
  config->setValue(QLatin1String("ImportFormatTracks"), QVariant(m_importFormatTracks));
  config->setValue(QLatin1String("ImportFormatIdx"), QVariant(m_importFormatIdx));
  config->setValue(QLatin1String("EnableTimeDifferenceCheck"), QVariant(m_enableTimeDifferenceCheck));
  config->setValue(QLatin1String("MaxTimeDifference"), QVariant(m_maxTimeDifference));
  config->setValue(QLatin1String("ImportVisibleColumns"), QVariant(m_importVisibleColumns));
  config->setValue(QLatin1String("ImportWindowGeometry"), QVariant(m_importWindowGeometry));

  config->setValue(QLatin1String("ImportTagsNames"), QVariant(m_importTagsNames));
  config->setValue(QLatin1String("ImportTagsSources"), QVariant(m_importTagsSources));
  config->setValue(QLatin1String("ImportTagsExtractions"), QVariant(m_importTagsExtractions));
  config->setValue(QLatin1String("ImportTagsIdx"), QVariant(m_importTagsIdx));

  config->setValue(QLatin1String("ExportSourceV1"), QVariant(m_exportSrcV1 == TrackData::TagV1));
  config->setValue(QLatin1String("ExportFormatNames"), QVariant(m_exportFormatNames));
  config->setValue(QLatin1String("ExportFormatHeaders"), QVariant(m_exportFormatHeaders));
  config->setValue(QLatin1String("ExportFormatTracks"), QVariant(m_exportFormatTracks));
  config->setValue(QLatin1String("ExportFormatTrailers"), QVariant(m_exportFormatTrailers));
  config->setValue(QLatin1String("ExportFormatIdx"), QVariant(m_exportFormatIdx));
  config->setValue(QLatin1String("ExportWindowGeometry"), QVariant(m_exportWindowGeometry));

  config->setValue(QLatin1String("PictureSourceNames"), QVariant(m_pictureSourceNames));
  config->setValue(QLatin1String("PictureSourceUrls"), QVariant(m_pictureSourceUrls));
  config->setValue(QLatin1String("PictureSourceIdx"), QVariant(m_pictureSourceIdx));
  config->setValue(QLatin1String("MatchPictureUrlMapKeys"), QVariant(m_matchPictureUrlMap.keys()));
  config->setValue(QLatin1String("MatchPictureUrlMapValues"), QVariant(m_matchPictureUrlMap.values()));
  config->setValue(QLatin1String("BrowseCoverArtWindowGeometry"), QVariant(m_browseCoverArtWindowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void ImportConfig::readFromConfig(ISettings* config)
{
  QStringList names, headers, tracks;
  QStringList tagsNames, tagsSources, tagsExtractions;
  QStringList expNames, expHeaders, expTracks, expTrailers, picNames, picUrls;

  config->beginGroup(m_group);
  m_importServer = config->value(QLatin1String("ImportServer"), m_importServer).toInt();
  m_importDest = importDestCfgToTagVersion(
    config->value(QLatin1String("ImportDestination"),
                  tagVersionToImportDestCfg(m_importDest)).toInt());
  names = config->value(QLatin1String("ImportFormatNames"), m_importFormatNames).toStringList();
  headers = config->value(QLatin1String("ImportFormatHeaders"), m_importFormatHeaders).toStringList();
  tracks = config->value(QLatin1String("ImportFormatTracks"), m_importFormatTracks).toStringList();
  m_importFormatIdx = config->value(QLatin1String("ImportFormatIdx"), m_importFormatIdx).toInt();
  m_enableTimeDifferenceCheck = config->value(QLatin1String("EnableTimeDifferenceCheck"), m_enableTimeDifferenceCheck).toBool();
  m_maxTimeDifference = config->value(QLatin1String("MaxTimeDifference"), m_maxTimeDifference).toInt();
  m_importVisibleColumns = config->value(QLatin1String("ImportVisibleColumns"), m_importVisibleColumns).toULongLong();
  m_importWindowGeometry = config->value(QLatin1String("ImportWindowGeometry"), m_importWindowGeometry).toByteArray();

  tagsNames = config->value(QLatin1String("ImportTagsNames"), m_importTagsNames).toStringList();
  tagsSources = config->value(QLatin1String("ImportTagsSources"), m_importTagsSources).toStringList();
  tagsExtractions = config->value(QLatin1String("ImportTagsExtractions"), m_importTagsExtractions).toStringList();
  m_importTagsIdx = config->value(QLatin1String("ImportTagsIdx"), m_importTagsIdx).toInt();

  m_exportSrcV1 = config->value(QLatin1String("ExportSourceV1"),
      m_exportSrcV1 == TrackData::TagV1).toBool()
      ? TrackData::TagV1 : TrackData::TagV2;
  expNames = config->value(QLatin1String("ExportFormatNames"), m_exportFormatNames).toStringList();
  expHeaders = config->value(QLatin1String("ExportFormatHeaders"), m_exportFormatHeaders).toStringList();
  expTracks = config->value(QLatin1String("ExportFormatTracks"), m_exportFormatTracks).toStringList();
  expTrailers = config->value(QLatin1String("ExportFormatTrailers"), m_exportFormatTrailers).toStringList();
  m_exportFormatIdx = config->value(QLatin1String("ExportFormatIdx"), m_exportFormatIdx).toInt();
  m_exportWindowGeometry = config->value(QLatin1String("ExportWindowGeometry"), m_exportWindowGeometry).toByteArray();

  picNames = config->value(QLatin1String("PictureSourceNames"), m_pictureSourceNames).toStringList();
  picUrls = config->value(QLatin1String("PictureSourceUrls"), m_pictureSourceUrls).toStringList();
  m_pictureSourceIdx = config->value(
    QLatin1String("PictureSourceIdx"), m_pictureSourceIdx).toInt();
  QStringList keys = config->value(QLatin1String("MatchPictureUrlMapKeys"), QStringList()).toStringList();
  QStringList values = config->value(QLatin1String("MatchPictureUrlMapValues"), QStringList()).toStringList();
  if (!keys.empty() && !values.empty()) {
    QStringList::Iterator itk, itv;
    m_matchPictureUrlMap.clear();
    for (itk = keys.begin(), itv = values.begin();
       itk != keys.end() && itv != values.end();
       ++itk, ++itv) {
      m_matchPictureUrlMap[*itk] = *itv;
    }
  }
  m_browseCoverArtWindowGeometry = config->value(
        QLatin1String("BrowseCoverArtWindowGeometry"),
        m_browseCoverArtWindowGeometry).toByteArray();
  config->endGroup();

  // KConfig seems to strip empty entries from the end of the string lists,
  // so we have to append them again.
  unsigned numNames = names.size();
  while (static_cast<unsigned>(headers.size()) < numNames) headers.append(QLatin1String(""));
  while (static_cast<unsigned>(tracks.size()) < numNames) tracks.append(QLatin1String(""));
  unsigned numExpNames = expNames.size();
  while (static_cast<unsigned>(expHeaders.size()) < numExpNames) expHeaders.append(QLatin1String(""));
  while (static_cast<unsigned>(expTracks.size()) < numExpNames) expTracks.append(QLatin1String(""));
  while (static_cast<unsigned>(expTrailers.size()) < numExpNames) expTrailers.append(QLatin1String(""));
  unsigned numPicNames = picNames.size();
  while (static_cast<unsigned>(picUrls.size()) < numPicNames) picUrls.append(QLatin1String(""));

  /* Use defaults if no configuration found */
  QStringList::const_iterator namesIt, headersIt, tracksIt;
  for (namesIt = names.begin(), headersIt = headers.begin(),
         tracksIt = tracks.begin();
       namesIt != names.end() && headersIt != headers.end() &&
         tracksIt != tracks.end();
       ++namesIt, ++headersIt, ++tracksIt) {
    int idx = m_importFormatNames.indexOf(*namesIt);
    if (idx >= 0) {
      m_importFormatHeaders[idx] = *headersIt;
      m_importFormatTracks[idx] = *tracksIt;
    } else if (!(*namesIt).isEmpty()) {
      m_importFormatNames.append(*namesIt);
      m_importFormatHeaders.append(*headersIt);
      m_importFormatTracks.append(*tracksIt);
    }
  }

  QStringList::const_iterator tagsNamesIt, sourcesIt, extractionsIt;
  for (tagsNamesIt = tagsNames.begin(), sourcesIt = tagsSources.begin(),
         extractionsIt = tagsExtractions.begin();
       tagsNamesIt != tagsNames.end() && sourcesIt != tagsSources.end() &&
         extractionsIt != tagsExtractions.end();
       ++tagsNamesIt, ++sourcesIt, ++extractionsIt) {
    int idx = m_importTagsNames.indexOf(*tagsNamesIt);
    if (idx >= 0) {
      m_importTagsSources[idx] = *sourcesIt;
      m_importTagsExtractions[idx] = *extractionsIt;
    } else if (!(*tagsNamesIt).isEmpty()) {
      m_importTagsNames.append(*tagsNamesIt);
      m_importTagsSources.append(*sourcesIt);
      m_importTagsExtractions.append(*extractionsIt);
    }
  }

  QStringList::const_iterator expNamesIt, expHeadersIt, expTracksIt,
    expTrailersIt;
  for (expNamesIt = expNames.begin(), expHeadersIt = expHeaders.begin(),
         expTracksIt = expTracks.begin(), expTrailersIt = expTrailers.begin();
       expNamesIt != expNames.end() && expHeadersIt != expHeaders.end() &&
         expTracksIt != expTracks.end() && expTrailersIt != expTrailers.end();
       ++expNamesIt, ++expHeadersIt, ++expTracksIt, ++expTrailersIt) {
    int idx = m_exportFormatNames.indexOf(*expNamesIt);
    if (idx >= 0) {
      m_exportFormatHeaders[idx] = *expHeadersIt;
      m_exportFormatTracks[idx] = *expTracksIt;
      m_exportFormatTrailers[idx] = *expTrailersIt;
    } else if (!(*expNamesIt).isEmpty()) {
      m_exportFormatNames.append(*expNamesIt);
      m_exportFormatHeaders.append(*expHeadersIt);
      m_exportFormatTracks.append(*expTracksIt);
      m_exportFormatTrailers.append(*expTrailersIt);
    }
  }

  QStringList::const_iterator picNamesIt, picUrlsIt;
  for (picNamesIt = picNames.begin(), picUrlsIt = picUrls.begin();
       picNamesIt != picNames.end() && picUrlsIt != picUrls.end();
       ++picNamesIt, ++picUrlsIt) {
    int idx = m_pictureSourceNames.indexOf(*picNamesIt);
    if (idx >= 0) {
      m_pictureSourceUrls[idx] = *picUrlsIt;
    } else if (!(*picNamesIt).isEmpty()) {
      m_pictureSourceNames.append(*picNamesIt);
      m_pictureSourceUrls.append(*picUrlsIt);
    }
  }

  if (m_importFormatIdx >= static_cast<int>(m_importFormatNames.size()))
    m_importFormatIdx = 0;
  if (m_importTagsIdx >= static_cast<int>(m_importTagsNames.size()))
    m_importTagsIdx = 0;
  if (m_exportFormatIdx >=  static_cast<int>(m_exportFormatNames.size()))
    m_exportFormatIdx = 0;
  if (m_pictureSourceIdx >=  static_cast<int>(m_pictureSourceNames.size()))
    m_pictureSourceIdx = 0;
}
