/**
 * \file exportconfig.cpp
 * Configuration for export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "exportconfig.h"

int ExportConfig::s_index = -1;

/**
 * Constructor.
 */
ExportConfig::ExportConfig() :
  StoredConfig<ExportConfig>(QLatin1String("Export")),
  m_exportSrcV1(TrackData::TagV1), m_exportFormatIdx(0)
{
  m_exportFormatNames.append(QLatin1String("CSV unquoted"));
  m_exportFormatHeaders.append(QLatin1String(""));
  m_exportFormatTracks.append(QLatin1String("%{track}\\t%{title}\\t%{artist}\\t%{album}\\t%{year}\\t%{genre}\\t%{comment}\\t%{duration}.00"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("CSV quoted"));
  m_exportFormatHeaders.append(QLatin1String(""));
  m_exportFormatTracks.append(QLatin1String("\"%{track}\"\\t\"%{title}\"\\t\"%{artist}\"\\t\"%{album}\"\\t\"%{year}\"\\t\"%{genre}\"\\t\"%{comment}\"\\t\"%{duration}.00\""));
  m_exportFormatTrailers.append(QLatin1String(""));

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
  m_exportFormatHeaders.append(QLatin1String("<html>\\n <head>\\n  <title>%h{artist} - %h{album}</title>\\n </head>\\n <body>\\n  <h1>%h{artist} - %h{album}</h1>\\n  <dl>"));
  m_exportFormatTracks.append(QLatin1String("   <dt><a href=\"%{url}\">%h{track}. %h{title}</a></dt>"));
  m_exportFormatTrailers.append(QLatin1String("  </dl>\\n </body>\\n</html>"));

  m_exportFormatNames.append(QLatin1String("Kover XML"));
  m_exportFormatHeaders.append(QLatin1String("<kover>\\n <title>\\n  <text><![CDATA[%{artist} ]]></text>\\n  <text><![CDATA[%{album}]]></text>\\n </title>\\n <content>"));
  m_exportFormatTracks.append(QLatin1String("  <text><![CDATA[%{track}. %{title}]]></text>"));
  m_exportFormatTrailers.append(QLatin1String(" </content>\\n</kover>"));

  m_exportFormatNames.append(QLatin1String("Technical Details"));
  m_exportFormatHeaders.append(QLatin1String("File\\tBitrate\\tVBR\\tDuration\\tSamplerate\\tChannels\\tMode\\tCodec"));
  m_exportFormatTracks.append(QLatin1String("%{file}\\t%{bitrate}\\t%{vbr}\\t%{duration}\\t%{samplerate}\\t%{channels}\\t%{mode}\\t%{codec}"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("CDRDAO TOC"));
  m_exportFormatHeaders.append(QLatin1String("CD_DA\\nCD_TEXT {\\n  LANGUAGE_MAP {\\n    0 : EN\\n  }\\n  LANGUAGE 0 {\\n    TITLE \"%{album}\"\\n    PERFORMER \"%{artist}\"\\n  }\\n}\\n"));
  m_exportFormatTracks.append(QLatin1String("TRACK AUDIO\\nCD_TEXT {\\n  LANGUAGE 0 {\\n    TITLE \"%{title}\"\\n    PERFORMER \"%{artist}\"\\n  }\\n}\\nFILE \"%{file}\" 0\\n"));
  m_exportFormatTrailers.append(QLatin1String(""));

  m_exportFormatNames.append(QLatin1String("Custom Format"));
  m_exportFormatHeaders.append(QLatin1String(""));
  m_exportFormatTracks.append(QLatin1String(""));
  m_exportFormatTrailers.append(QLatin1String(""));
}

/**
 * Destructor.
 */
ExportConfig::~ExportConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void ExportConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("ExportSourceV1"), QVariant(m_exportSrcV1 == TrackData::TagV1));
  config->setValue(QLatin1String("ExportFormatNames"), QVariant(m_exportFormatNames));
  config->setValue(QLatin1String("ExportFormatHeaders"), QVariant(m_exportFormatHeaders));
  config->setValue(QLatin1String("ExportFormatTracks"), QVariant(m_exportFormatTracks));
  config->setValue(QLatin1String("ExportFormatTrailers"), QVariant(m_exportFormatTrailers));
  config->setValue(QLatin1String("ExportFormatIdx"), QVariant(m_exportFormatIdx));
  config->setValue(QLatin1String("ExportWindowGeometry"), QVariant(m_exportWindowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void ExportConfig::readFromConfig(ISettings* config)
{
  QStringList expNames, expHeaders, expTracks, expTrailers;

  config->beginGroup(m_group);
  m_exportSrcV1 = config->value(QLatin1String("ExportSourceV1"),
      m_exportSrcV1 == TrackData::TagV1).toBool()
      ? TrackData::TagV1 : TrackData::TagV2;
  expNames = config->value(QLatin1String("ExportFormatNames"), m_exportFormatNames).toStringList();
  expHeaders = config->value(QLatin1String("ExportFormatHeaders"), m_exportFormatHeaders).toStringList();
  expTracks = config->value(QLatin1String("ExportFormatTracks"), m_exportFormatTracks).toStringList();
  expTrailers = config->value(QLatin1String("ExportFormatTrailers"), m_exportFormatTrailers).toStringList();
  m_exportFormatIdx = config->value(QLatin1String("ExportFormatIdx"), m_exportFormatIdx).toInt();
  m_exportWindowGeometry = config->value(QLatin1String("ExportWindowGeometry"), m_exportWindowGeometry).toByteArray();
  config->endGroup();

  // KConfig seems to strip empty entries from the end of the string lists,
  // so we have to append them again.
  unsigned numExpNames = expNames.size();
  while (static_cast<unsigned>(expHeaders.size()) < numExpNames) expHeaders.append(QLatin1String(""));
  while (static_cast<unsigned>(expTracks.size()) < numExpNames) expTracks.append(QLatin1String(""));
  while (static_cast<unsigned>(expTrailers.size()) < numExpNames) expTrailers.append(QLatin1String(""));

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

  if (m_exportFormatIdx >=  static_cast<int>(m_exportFormatNames.size()))
    m_exportFormatIdx = 0;

  // Use HTML escaping for old HTML export format.
  int htmlIdx = m_exportFormatNames.indexOf(QLatin1String("HTML"));
  if (htmlIdx != -1) {
    if (m_exportFormatHeaders.at(htmlIdx) == QLatin1String("<html>\\n <head>\\n  <title>%{artist} - %{album}</title>\\n </head>\\n <body>\\n  <h1>%{artist} - %{album}</h1>\\n  <dl>")) {
      m_exportFormatHeaders[htmlIdx] = QLatin1String("<html>\\n <head>\\n  <title>%h{artist} - %h{album}</title>\\n </head>\\n <body>\\n  <h1>%h{artist} - %h{album}</h1>\\n  <dl>");
    }
    if (m_exportFormatTracks.at(htmlIdx) == QLatin1String("   <dt><a href=\"%{url}\">%{track}. %{title}</a></dt>")) {
      m_exportFormatTracks[htmlIdx] = QLatin1String("   <dt><a href=\"%{url}\">%h{track}. %h{title}</a></dt>");
    }
  }
}
