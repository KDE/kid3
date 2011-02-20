/**
 * \file importconfig.cpp
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2009  Urs Fleisch
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

#include <qstring.h>
#include "qtcompatmac.h"
#include "importconfig.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kconfigskeleton.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
ImportConfig::ImportConfig(const QString& grp) :
	GeneralConfig(grp), m_importServer(ImportConfig::ServerFreedb),
	m_importDest(ImportConfig::DestV1), m_importFormatIdx(0),
	m_enableTimeDifferenceCheck(true), m_maxTimeDifference(3),
	m_importWindowWidth(-1), m_importWindowHeight(-1),
	m_exportSrcV1(true), m_exportFormatIdx(0),
	m_exportWindowWidth(-1), m_exportWindowHeight(-1),
	m_pictureSourceIdx(0),
	m_browseCoverArtWindowWidth(-1), m_browseCoverArtWindowHeight(-1)
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
	m_importFormatNames.append("CSV unquoted");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("%{track}(\\d+)\\t%{title}([^\\r\\n\\t]*)\\t%{artist}([^\\r\\n\\t]*)\\t%{album}([^\\r\\n\\t]*)\\t%{year}(\\d+)\\t%{genre}([^\\r\\n\\t]*)\\t%{comment}([^\\r\\n\\t]*)\\t(?:\\d+:)?%{duration}(\\d+:\\d+)");

	m_importFormatNames.append("CSV quoted");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\"?%{track}(\\d+)\"?\\t\"?%{title}([^\\r\\n\\t\"]*)\"?\\t\"?%{artist}([^\\r\\n\\t\"]*)\"?\\t\"?%{album}([^\\r\\n\\t\"]*)\"?\\t\"?%{year}(\\d+)\"?\\t\"?%{genre}([^\\r\\n\\t\"]*)\"?\\t\"?%{comment}([^\\r\\n\\t\"]*)\"?\\t\"?(?:\\d+:)?%{duration}(\\d+:\\d+)");

	m_importFormatNames.append("CSV more unquoted");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append(
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
		"%{subtitle}([^\\r\\n\\t]*)\\t%{website}([^\\r\\n\\t]*)");

	m_importFormatNames.append("CSV more quoted");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append(
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
		"\"?%{subtitle}([^\\r\\n\\t\"]*)\"?\\t\"?%{website}([^\\r\\n\\t\"]*)");

	m_importFormatNames.append("freedb HTML text");
	m_importFormatHeaders.append("%{artist}(\\S[^\\r\\n/]*\\S)\\s*/\\s*%{album}(\\S[^\\r\\n]*\\S)[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%{year}(\\d+)?.*genre:\\s*%{genre}(\\S[^\\r\\n]*\\S)?[\\r\\n]");
	m_importFormatTracks.append("[\\r\\n]%{track}(\\d+)[\\.\\s]+%{duration}(\\d+:\\d+)\\s+%{title}(\\S[^\\r\\n]*\\S)");

	m_importFormatNames.append("freedb HTML source");
	m_importFormatHeaders.append("<[^>]+>%{artist}([^<\\s][^\\r\\n/]*\\S)\\s*/\\s*%{album}(\\S[^\\r\\n]*[^\\s>])<[^>]+>[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%{year}(\\d+)?.*genre:\\s*%{genre}(\\S[^\\r\\n>]*\\S)?<[^>]+>[\\r\\n]");
	m_importFormatTracks.append("<td[^>]*>\\s*%{track}(\\d+).</td><td[^>]*>\\s*%{duration}(\\d+:\\d+)</td><td[^>]*>(?:<[^>]+>)?%{title}([^<\\r\\n]+)");

	m_importFormatNames.append("Title");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\\s*%{title}(\\S[^\\r\\n]*\\S)\\s*");

	m_importFormatNames.append("Track Title");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\\s*%{track}(\\d+)[\\.\\s]+%{title}(\\S[^\\r\\n]*\\S)\\s*");

	m_importFormatNames.append("Track Title Time");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\\s*%{track}(\\d+)[\\.\\s]+%{title}(\\S[^\\r\\n]*\\S)\\s+%{duration}(\\d+:\\d+)\\s*");

	m_importFormatNames.append("Custom Format");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("");

	m_exportFormatNames.append("CSV unquoted");
	m_exportFormatHeaders.append("");
	m_exportFormatTracks.append("%{track}\\t%{title}\\t%{artist}\\t%{album}\\t%{year}\\t%{genre}\\t%{comment}\\t%{duration}.00");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("CSV quoted");
	m_exportFormatHeaders.append("");
	m_exportFormatTracks.append("\"%{track}\"\\t\"%{title}\"\\t\"%{artist}\"\\t\"%{album}\"\\t\"%{year}\"\\t\"%{genre}\"\\t\"%{comment}\"\\t\"%{duration}.00\"");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("CSV more unquoted");
	m_exportFormatHeaders.append(
		"Track\\tTitle\\tArtist\\tAlbum\\tDate\\tGenre\\tComment\\tDuration\\t"
		"Album Artist\\tArranger\\tAuthor\\tBPM\\tComposer\\t"
		"Conductor\\tCopyright\\tDisc Number\\tEncoded-by\\tGrouping\\tISRC\\t"
		"Language\\tLyricist\\tLyrics\\tMedia\\tOriginal Album\\t"
		"Original Artist\\tOriginal Date\\tPart\\tPerformer\\t"
		"Publisher\\tRemixer\\tSubtitle\\tWebsite");
	m_exportFormatTracks.append(
		"%{track}\\t%{title}\\t%{artist}\\t%{album}\\t%{year}\\t%{genre}\\t%{comment}\\t"
		"%{duration}.00\\t"
		"%{album artist}\\t%{arranger}\\t%{author}\\t%{bpm}\\t%{composer}\\t"
		"%{conductor}\\t%{copyright}\\t%{disc number}\\t%{encoded-by}\\t%{grouping}\\t%{isrc}\\t"
		"%{language}\\t%{lyricist}\\t%{lyrics}\\t%{media}\\t%{original album}\\t"
		"%{original artist}\\t%{original date}\\t%{part}\\t%{performer}\\t"
		"%{publisher}\\t%{remixer}\\t%{subtitle}\\t%{website}");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("CSV more quoted");
	m_exportFormatHeaders.append(
		"\"Track\"\\t\"Title\"\\t\"Artist\"\\t\"Album\"\\t\"Date\"\\t"
		"\"Genre\"\\t\"Comment\"\\t\"Duration\"\\t"
		"\"Album Artist\"\\t\"Arranger\"\\t\"Author\"\\t\"BPM\"\\t"
		"\"Composer\"\\t\"Conductor\"\\t\"Copyright\"\\t\"Disc Number\"\\t"
		"\"Encoded-by\"\\t\"Grouping\"\\t\"ISRC\"\\t\"Language\"\\t\"Lyricist\"\\t\"Lyrics\"\\t"
		"\"Media\"\\t\"Original Album\"\\t\"Original Artist\"\\t"
		"\"Original Date\"\\t\"Part\"\\t\"Performer\"\\t\"Publisher\"\\t"
		"\"Remixer\"\\t\"Subtitle\"\\t\"Website\"");
	m_exportFormatTracks.append(
		"\"%{track}\"\\t\"%{title}\"\\t\"%{artist}\"\\t\"%{album}\"\\t\"%{year}\"\\t"
		"\"%{genre}\"\\t\"%{comment}\"\\t\"%{duration}.00\"\\t"
		"\"%{album artist}\"\\t\"%{arranger}\"\\t\"%{author}\"\\t\"%{bpm}\"\\t"
		"\"%{composer}\"\\t\"%{conductor}\"\\t\"%{copyright}\"\\t\"%{disc number}\"\\t"
		"\"%{encoded-by}\"\\t\"%{grouping}\"\\t\"%{isrc}\"\\t\"%{language}\"\\t\"%{lyricist}\"\\t\"%{lyrics}\"\\t"
		"\"%{media}\"\\t\"%{original album}\"\\t\"%{original artist}\"\\t"
		"\"%{original date}\"\\t\"%{part}\"\\t\"%{performer}\"\\t\"%{publisher}\"\\t"
		"\"%{remixer}\"\\t\"%{subtitle}\"\\t\"%{website}\"");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("Extended M3U");
	m_exportFormatHeaders.append("#EXTM3U");
	m_exportFormatTracks.append("#EXTINF:%{seconds},%{artist} - %{title}\\n%{filepath}");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("Extended PLS");
	m_exportFormatHeaders.append("[playlist]");
	m_exportFormatTracks.append("File%{tracknumber}=%{filepath}\\nTitle%{tracknumber}=%{artist} - %{title}\\nLength%{tracknumber}=%{seconds}");
	m_exportFormatTrailers.append("NumberOfEntries=%{tracks}\\nVersion=2");

	m_exportFormatNames.append("HTML");
	m_exportFormatHeaders.append("<html>\\n <head>\\n  <title>%{artist} - %{album}</title>\\n </head>\\n <body>\\n  <h1>%{artist} - %{album}</h1>\\n  <dl>");
	m_exportFormatTracks.append("   <dt><a href=\"%{url}\">%{track}. %{title}</a></dt>");
	m_exportFormatTrailers.append("  </dl>\\n </body>\\n</html>");

	m_exportFormatNames.append("Kover XML");
	m_exportFormatHeaders.append("<kover>\\n <title>\\n  <text><![CDATA[%{artist} ]]></text>\\n  <text><![CDATA[%{album}]]></text>\\n </title>\\n <content>");
	m_exportFormatTracks.append("  <text><![CDATA[%{track}. %{title}]]></text>");
	m_exportFormatTrailers.append(" </content>\\n</kover>");

	m_exportFormatNames.append("Technical Details");
	m_exportFormatHeaders.append("File\\tBitrate\\tVBR\\tDuration\\tSamplerate\\tChannels\\tMode\\tCodec");
	m_exportFormatTracks.append("%{file}\\t%{bitrate}\\t%{vbr}\\t%{duration}\\t%{samplerate}\\t%{channels}\\t%{mode}\\t%{codec}");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("Custom Format");
	m_exportFormatHeaders.append("");
	m_exportFormatTracks.append("");
	m_exportFormatTrailers.append("");

	m_pictureSourceNames.append("Google Images");
	m_pictureSourceUrls.append("http://images.google.com/images?q=%u{artist}%20%u{album}");
	m_pictureSourceNames.append("Yahoo Images");
	m_pictureSourceUrls.append("http://images.search.yahoo.com/search/images?ei=UTF-8&p=%u{artist}%20%u{album}");
	m_pictureSourceNames.append("Amazon");
	m_pictureSourceUrls.append("http://www.amazon.com/s?search-alias=aps&field-keywords=%u{artist}+%u{album}");
	m_pictureSourceNames.append("Amazon.co.uk");
	m_pictureSourceUrls.append("http://www.amazon.co.uk/s?search-alias=aps&field-keywords=%u{artist}+%u{album}");
	m_pictureSourceNames.append("Amazon.de");
	m_pictureSourceUrls.append("http://www.amazon.de/s?search-alias=aps&field-keywords=%u{artist}+%u{album}");
	m_pictureSourceNames.append("Amazon.fr");
	m_pictureSourceUrls.append("http://www.amazon.fr/s?search-alias=aps&field-keywords=%u{artist}+%u{album}");
	m_pictureSourceNames.append("MusicBrainz");
	m_pictureSourceUrls.append("http://musicbrainz.org/search/textsearch.html?query=%u{artist}+%u{album}&type=release");
	m_pictureSourceNames.append("Discogs");
	m_pictureSourceUrls.append("http://www.discogs.com/search?q=%u{artist}+%u{album}");
	m_pictureSourceNames.append("CD Universe");
	m_pictureSourceUrls.append("http://www.cduniverse.com/sresult.asp?HT_Search_Info=%u{artist}+%u{album}");
 	m_pictureSourceNames.append("Coveralia");
	m_pictureSourceUrls.append("http://www.coveralia.com/mostrar.php?bus=%u{artist}%20%u{album}&bust=2");
	m_pictureSourceNames.append("FreeCovers");
	m_pictureSourceUrls.append("http://www.freecovers.net/search.php?search=%u{artist}+%u{album}&cat=4");
	m_pictureSourceNames.append("CoverHunt");
	m_pictureSourceUrls.append("http://www.coverhunt.com/search/%u{artist}+%u{album}");
	m_pictureSourceNames.append("SlothRadio");
	m_pictureSourceUrls.append("http://www.slothradio.com/covers/?artist=%u{artist}&album=%u{album}");
	m_pictureSourceNames.append("Albumart");
	m_pictureSourceUrls.append("http://www.albumart.org/index.php?srchkey=%u{artist}+%u{album}&searchindex=Music");
	m_pictureSourceNames.append("Yalp!");
	m_pictureSourceUrls.append("http://search.yalp.alice.it/search/search.html?txtToSearch=%u{artist}%20%u{album}");
	m_pictureSourceNames.append("HMV");
	m_pictureSourceUrls.append("http://hmv.com/hmvweb/advancedSearch.do?searchType=2&artist=%u{artist}&title=%u{album}");
	m_pictureSourceNames.append("CD Baby");
	m_pictureSourceUrls.append("http://cdbaby.com/found?artist=%u{artist}&album=%u{album}");
	m_pictureSourceNames.append("Jamendo");
	m_pictureSourceUrls.append("http://www.jamendo.com/en/search/all/%u{artist}%20%u{album}");
	m_pictureSourceNames.append("Custom Source");
	m_pictureSourceUrls.append("");

	m_matchPictureUrlMap["http://images.google.com/.*imgurl=([^&]+)&.*"] =
		"\\1";
	m_matchPictureUrlMap["http://rds.yahoo.com/.*%26imgurl=((?:[^%]|%(?!26))+).*"] =
		"http%253A%252F%252F\\1";
	m_matchPictureUrlMap["http://rds.yahoo.com/.*&imgurl=([^&]+)&.*"] =
		"http%3A%2F%2F\\1";
	m_matchPictureUrlMap["http://(?:www.)?amazon.(?:com|co.uk|de|fr).*/(?:dp|ASIN|images|product|-)/([A-Z0-9]+).*"] =
		"http://images.amazon.com/images/P/\\1.01._SCLZZZZZZZ_.jpg";
	m_matchPictureUrlMap["http://musicbrainz.org/misc/redirects/.*&asin=([A-Z0-9]+).*"] =
		"http://images.amazon.com/images/P/\\1.01._SCLZZZZZZZ_.jpg";
	m_matchPictureUrlMap["http://www.freecovers.net/view/(\\d+)/([0-9a-f]+)/.*"] =
		"http://www.freecovers.net/preview/\\1/\\2/big.jpg";
	m_matchPictureUrlMap["http://cdbaby.com/cd/(\\w)(\\w)(\\w+)"] =
		"http://cdbaby.name/\\1/\\2/\\1\\2\\3.jpg";
	m_matchPictureUrlMap["http://www.jamendo.com/en/album/(\\d+)"] =
		"http://imgjam.com/albums/\\1/covers/1.0.jpg";
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
void ImportConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	cfg.writeEntry("ImportServer", static_cast<int>(m_importServer));
	cfg.writeEntry("ImportDestination", static_cast<int>(m_importDest));
	cfg.writeEntry("ImportFormatNames", m_importFormatNames);
	cfg.writeEntry("ImportFormatHeaders", m_importFormatHeaders);
	cfg.writeEntry("ImportFormatTracks", m_importFormatTracks);
	cfg.writeEntry("ImportFormatIdx", m_importFormatIdx);
	cfg.writeEntry("EnableTimeDifferenceCheck", m_enableTimeDifferenceCheck);
	cfg.writeEntry("MaxTimeDifference", m_maxTimeDifference);
	cfg.writeEntry("ImportWindowWidth", m_importWindowWidth);
	cfg.writeEntry("ImportWindowHeight", m_importWindowHeight);

	cfg.writeEntry("ExportSourceV1", m_exportSrcV1);
	cfg.writeEntry("ExportFormatNames", m_exportFormatNames);
	cfg.writeEntry("ExportFormatHeaders", m_exportFormatHeaders);
	cfg.writeEntry("ExportFormatTracks", m_exportFormatTracks);
	cfg.writeEntry("ExportFormatTrailers", m_exportFormatTrailers);
	cfg.writeEntry("ExportFormatIdx", m_exportFormatIdx);
	cfg.writeEntry("ExportWindowWidth", m_exportWindowWidth);
	cfg.writeEntry("ExportWindowHeight", m_exportWindowHeight);

	cfg.writeEntry("PictureSourceNames", m_pictureSourceNames);
	cfg.writeEntry("PictureSourceUrls", m_pictureSourceUrls);
	cfg.writeEntry("PictureSourceIdx", m_pictureSourceIdx);
	cfg.writeEntry("MatchPictureUrlMapKeys", m_matchPictureUrlMap.keys());
	cfg.writeEntry("MatchPictureUrlMapValues", m_matchPictureUrlMap.values());
	cfg.writeEntry("BrowseCoverArtWindowWidth", m_browseCoverArtWindowWidth);
	cfg.writeEntry("BrowseCoverArtWindowHeight", m_browseCoverArtWindowHeight);
#else
	config->beginGroup("/" + m_group);
	config->QCM_writeEntry("/ImportServer", m_importServer);
	config->QCM_writeEntry("/ImportDestination", m_importDest);
	config->QCM_writeEntry("/ImportFormatNames", m_importFormatNames);
	config->QCM_writeEntry("/ImportFormatHeaders", m_importFormatHeaders);
	config->QCM_writeEntry("/ImportFormatTracks", m_importFormatTracks);
	config->QCM_writeEntry("/ImportFormatIdx", m_importFormatIdx);
	config->QCM_writeEntry("/EnableTimeDifferenceCheck", m_enableTimeDifferenceCheck);
	config->QCM_writeEntry("/MaxTimeDifference", m_maxTimeDifference);
	config->QCM_writeEntry("/ImportWindowWidth", m_importWindowWidth);
	config->QCM_writeEntry("/ImportWindowHeight", m_importWindowHeight);

	config->QCM_writeEntry("/ExportSourceV1", m_exportSrcV1);
	config->QCM_writeEntry("/ExportFormatNames", m_exportFormatNames);
	config->QCM_writeEntry("/ExportFormatHeaders", m_exportFormatHeaders);
	config->QCM_writeEntry("/ExportFormatTracks", m_exportFormatTracks);
	config->QCM_writeEntry("/ExportFormatTrailers", m_exportFormatTrailers);
	config->QCM_writeEntry("/ExportFormatIdx", m_exportFormatIdx);
	config->QCM_writeEntry("/ExportWindowWidth", m_exportWindowWidth);
	config->QCM_writeEntry("/ExportWindowHeight", m_exportWindowHeight);

	config->QCM_writeEntry("/PictureSourceNames", m_pictureSourceNames);
	config->QCM_writeEntry("/PictureSourceUrls", m_pictureSourceUrls);
	config->QCM_writeEntry("/PictureSourceIdx", m_pictureSourceIdx);
	config->QCM_writeEntry("/MatchPictureUrlMapKeys", m_matchPictureUrlMap.keys());
	config->QCM_writeEntry("/MatchPictureUrlMapValues", m_matchPictureUrlMap.values());
	config->QCM_writeEntry("/BrowseCoverArtWindowWidth", m_browseCoverArtWindowWidth);
	config->QCM_writeEntry("/BrowseCoverArtWindowHeight", m_browseCoverArtWindowHeight);

	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void ImportConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	)
{
	QStringList names, headers, tracks;
	QStringList expNames, expHeaders, expTracks, expTrailers, picNames, picUrls;
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	m_importServer = static_cast<ImportConfig::ImportServer>(
		cfg.KCM_readNumEntry("ImportServer", static_cast<int>(m_importServer)));
	m_importDest = static_cast<ImportConfig::ImportDestination>(
		cfg.KCM_readNumEntry("ImportDestination", static_cast<int>(m_importDest)));
	names = cfg.KCM_readListEntry("ImportFormatNames");
	headers = cfg.KCM_readListEntry("ImportFormatHeaders");
	tracks = cfg.KCM_readListEntry("ImportFormatTracks");
	m_importFormatIdx = cfg.KCM_readNumEntry("ImportFormatIdx", m_importFormatIdx);
	m_enableTimeDifferenceCheck = cfg.KCM_readBoolEntry("EnableTimeDifferenceCheck", m_enableTimeDifferenceCheck);
	m_maxTimeDifference = cfg.KCM_readNumEntry("MaxTimeDifference", m_maxTimeDifference);
	m_importWindowWidth = cfg.KCM_readNumEntry("ImportWindowWidth", -1);
	m_importWindowHeight = cfg.KCM_readNumEntry("ImportWindowHeight", -1);

	m_exportSrcV1 = cfg.KCM_readBoolEntry("ExportSourceV1", m_exportSrcV1);
	expNames = cfg.KCM_readListEntry("ExportFormatNames");
	expHeaders = cfg.KCM_readListEntry("ExportFormatHeaders");
	expTracks = cfg.KCM_readListEntry("ExportFormatTracks");
	expTrailers = cfg.KCM_readListEntry("ExportFormatTrailers");
	m_exportFormatIdx = cfg.KCM_readNumEntry("ExportFormatIdx", m_exportFormatIdx);
	m_exportWindowWidth = cfg.KCM_readNumEntry("ExportWindowWidth", -1);
	m_exportWindowHeight = cfg.KCM_readNumEntry("ExportWindowHeight", -1);

	picNames = cfg.KCM_readListEntry("PictureSourceNames");
	picUrls = cfg.KCM_readListEntry("PictureSourceUrls");
	m_pictureSourceIdx = cfg.KCM_readNumEntry("PictureSourceIdx", m_pictureSourceIdx);
	QStringList keys = cfg.KCM_readListEntry("MatchPictureUrlMapKeys");
	QStringList values = cfg.KCM_readListEntry("MatchPictureUrlMapValues");
	if (!keys.empty() && !values.empty()) {
		QStringList::Iterator itk, itv;
		m_matchPictureUrlMap.clear();
		for (itk = keys.begin(), itv = values.begin();
			 itk != keys.end() && itv != values.end();
			 ++itk, ++itv) {
			m_matchPictureUrlMap[*itk] = *itv;
		}
	}
	m_browseCoverArtWindowWidth = cfg.KCM_readNumEntry("BrowseCoverArtWindowWidth", -1);
	m_browseCoverArtWindowHeight = cfg.KCM_readNumEntry("BrowseCoverArtWindowHeight", -1);

	// KConfig seems to strip empty entries from the end of the string lists,
	// so we have to append them again.
	unsigned numNames = names.size();
	while (static_cast<unsigned>(headers.size()) < numNames) headers.append("");
	while (static_cast<unsigned>(tracks.size()) < numNames) tracks.append("");
	unsigned numExpNames = expNames.size();
	while (static_cast<unsigned>(expHeaders.size()) < numExpNames) expHeaders.append("");
	while (static_cast<unsigned>(expTracks.size()) < numExpNames) expTracks.append("");
	while (static_cast<unsigned>(expTrailers.size()) < numExpNames) expTrailers.append("");
	unsigned numPicNames = picNames.size();
	while (static_cast<unsigned>(picUrls.size()) < numPicNames) picUrls.append("");
#else
	config->beginGroup("/" + m_group);
	m_importServer = static_cast<ImportConfig::ImportServer>(
		config->QCM_readNumEntry("/ImportServer", m_importServer));
	m_importDest = static_cast<ImportConfig::ImportDestination>(
		config->QCM_readNumEntry("/ImportDestination", m_importDest));
	names = config->QCM_readListEntry("/ImportFormatNames");
	headers = config->QCM_readListEntry("/ImportFormatHeaders");
	tracks = config->QCM_readListEntry("/ImportFormatTracks");
	m_importFormatIdx = config->QCM_readNumEntry("/ImportFormatIdx", m_importFormatIdx);
	m_enableTimeDifferenceCheck = config->QCM_readBoolEntry("/EnableTimeDifferenceCheck", m_enableTimeDifferenceCheck);
	m_maxTimeDifference = config->QCM_readNumEntry("/MaxTimeDifference", m_maxTimeDifference);
	m_importWindowWidth = config->QCM_readNumEntry("/ImportWindowWidth", -1);
	m_importWindowHeight = config->QCM_readNumEntry("/ImportWindowHeight", -1);

	m_exportSrcV1 = config->QCM_readBoolEntry("/ExportSourceV1", m_exportSrcV1);
	expNames = config->QCM_readListEntry("/ExportFormatNames");
	expHeaders = config->QCM_readListEntry("/ExportFormatHeaders");
	expTracks = config->QCM_readListEntry("/ExportFormatTracks");
	expTrailers = config->QCM_readListEntry("/ExportFormatTrailers");
	m_exportFormatIdx = config->QCM_readNumEntry("/ExportFormatIdx", m_exportFormatIdx);
	m_exportWindowWidth = config->QCM_readNumEntry("/ExportWindowWidth", -1);
	m_exportWindowHeight = config->QCM_readNumEntry("/ExportWindowHeight", -1);

	picNames = config->QCM_readListEntry("/PictureSourceNames");
	picUrls = config->QCM_readListEntry("/PictureSourceUrls");
	m_pictureSourceIdx = config->QCM_readNumEntry(
		"/PictureSourceIdx", m_pictureSourceIdx);
	QStringList keys = config->QCM_readListEntry("/MatchPictureUrlMapKeys");
	QStringList values = config->QCM_readListEntry("/MatchPictureUrlMapValues");
	if (!keys.empty() && !values.empty()) {
		QStringList::Iterator itk, itv;
		m_matchPictureUrlMap.clear();
		for (itk = keys.begin(), itv = values.begin();
			 itk != keys.end() && itv != values.end();
			 ++itk, ++itv) {
			m_matchPictureUrlMap[*itk] = *itv;
		}
	}
	m_browseCoverArtWindowWidth = config->QCM_readNumEntry(
		"/BrowseCoverArtWindowWidth", -1);
	m_browseCoverArtWindowHeight = config->QCM_readNumEntry(
		"/BrowseCoverArtWindowHeight", -1);

	config->endGroup();
#endif
	/* Use defaults if no configuration found */
	QStringList::const_iterator namesIt, headersIt, tracksIt;
	for (namesIt = names.begin(), headersIt = headers.begin(),
				 tracksIt = tracks.begin();
			 namesIt != names.end() && headersIt != headers.end() &&
				 tracksIt != tracks.end();
			 ++namesIt, ++headersIt, ++tracksIt) {
#if QT_VERSION >= 0x040000
		int idx = m_importFormatNames.indexOf(*namesIt);
#else
		int idx = m_importFormatNames.findIndex(*namesIt);
#endif
		if (idx >= 0) {
			m_importFormatHeaders[idx] = *headersIt;
			m_importFormatTracks[idx] = *tracksIt;
		} else if (!(*namesIt).isEmpty()) {
			m_importFormatNames.append(*namesIt);
			m_importFormatHeaders.append(*headersIt);
			m_importFormatTracks.append(*tracksIt);
		}
	}

	QStringList::const_iterator expNamesIt, expHeadersIt, expTracksIt,
		expTrailersIt;
	for (expNamesIt = expNames.begin(), expHeadersIt = expHeaders.begin(),
				 expTracksIt = expTracks.begin(), expTrailersIt = expTrailers.begin();
			 expNamesIt != expNames.end() && expHeadersIt != expHeaders.end() &&
				 expTracksIt != expTracks.end() && expTrailersIt != expTrailers.end();
			 ++expNamesIt, ++expHeadersIt, ++expTracksIt, ++expTrailersIt) {
#if QT_VERSION >= 0x040000
		int idx = m_exportFormatNames.indexOf(*expNamesIt);
#else
		int idx = m_exportFormatNames.findIndex(*expNamesIt);
#endif
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
#if QT_VERSION >= 0x040000
		int idx = m_pictureSourceNames.indexOf(*picNamesIt);
#else
		int idx = m_pictureSourceNames.findIndex(*picNamesIt);
#endif
		if (idx >= 0) {
			m_pictureSourceUrls[idx] = *picUrlsIt;
		} else if (!(*picNamesIt).isEmpty()) {
			m_pictureSourceNames.append(*picNamesIt);
			m_pictureSourceUrls.append(*picUrlsIt);
		}
	}

	if (m_importFormatIdx >= static_cast<int>(m_importFormatNames.size()))
		m_importFormatIdx = 0;
	if (m_exportFormatIdx >=  static_cast<int>(m_exportFormatNames.size()))
		m_exportFormatIdx = 0;
	if (m_pictureSourceIdx >=  static_cast<int>(m_pictureSourceNames.size()))
		m_pictureSourceIdx = 0;
}
