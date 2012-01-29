/**
 * \file miscconfig.cpp
 * Miscellaneous Configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Jan 2004
 *
 * Copyright (C) 2004-2009  Urs Fleisch
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

#include "miscconfig.h"
#include <QString>
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfiggroup.h>
#endif

#include "generalconfig.h"
#include "taggedfile.h"

/** Default value for comment name */
const char* const defaultCommentName = "COMMENT";

/** Default value for web browser */
#ifdef __APPLE__
const char* const defaultBrowser = "open";
#else
const char* const defaultBrowser = "xdg-open";
#endif

/** Default to filename format list */
static const char* defaultToFilenameFormats[] = {
  "%{track} %{title}",
  "%{track}. %{title}",
  "%{track} - %{artist} - %{title}",
  "%{track}. %{artist} - %{title}",
  "%{artist} - %{track} - %{title}",
  "%{artist} - %{album} - %{track} - %{title}",
  "%{artist} - [%{year}] %{album} - %{track} - %{title}",
  "%{artist} - %{title}",
  "%{artist}-%{title}",
  "(%{artist}) %{title}",
  "%{artist}-%{title}-%{album}",
  0
};

/** Default from filename format list */
static const char* defaultFromFilenameFormats[] = {
  "%{artist} - %{album}/%{track} %{title}",
  "%{artist} - %{album}/%{track}. %{title}",
  "%{artist} - [%{year}] %{album}/%{track} %{title}",
  "%{artist} - [%{year}] %{album}/%{track}. %{title}",
  "%{artist}/%{album}/%{track} %{title}",
  "%{artist}/%{album}/%{track}. %{title}",
  "%{artist}/[%{year}] %{album}/%{track} %{title}",
  "%{artist}/[%{year}] %{album}/%{track}. %{title}",
  "%{album}/%{track} - %{artist} - %{title}",
  "%{album}/%{track}. %{artist} - %{title}",
  "%{album}/%{artist} - %{track} - %{title}",
  "[%{year}] %{album}/%{track} - %{artist} - %{title}",
  "%{artist} - %{album} - %{track} - %{title}",
  "%{artist} - [%{year}] %{album} - %{track} - %{title}",
  "%{album}/%{artist} - %{track} - %{title}",
  "[%{year}] %{album}/%{artist} - %{track} - %{title}",
  "%{album}/%{artist} - %{title}",
  "%{album}/%{artist}-%{title}",
  "%{album}/(%{artist}) %{title}",
  "%{artist}-%{title}-%{album}",
  0
};

/** Default directory format list */
static const char* dirFmt[] = {
  "%{artist} - %{album}",
  "%{artist} - [%{year}] %{album}",
  "%{artist}/%{album}",
  "%{artist}/[%{year}] %{album}",
  "%{album}",
  "[%{year}] %{album}",
  0                  // end of StrList
};

/** Default directory format list */
const char** MiscConfig::s_defaultDirFmtList = &dirFmt[0];

namespace {

/**
 * Convert tag version to rename directory value in configuration.
 * @param tagVersion tag version
 * @return value used in configuration, kept for backwards compatibility.
 */
inline int tagVersionToRenDirCfg(TrackData::TagVersion tagVersion) {
  int renDirSrc = static_cast<int>(tagVersion);
  if (renDirSrc == 3)
    renDirSrc = 0;
  return renDirSrc;
}

/**
 * Convert rename directory value in configuration to tag version.
 * @param renDirSrc value used in configuration, kept for backwards
 *                  compatibility.
 * @return tag version.
 */
inline TrackData::TagVersion renDirCfgToTagVersion(int renDirSrc) {
  if (renDirSrc == 0)
    renDirSrc = 3;
  return TrackData::tagVersionCast(renDirSrc);
}

/**
 * Convert tag version to number tracks destination value in configuration.
 * @param tagVersion tag version
 * @return value used in configuration, kept for backwards compatibility.
 */
inline int tagVersionToNumberTracksDestCfg(TrackData::TagVersion tagVersion) {
  return static_cast<int>(tagVersion) - 1;
}

/**
 * Convert number tracks destination value in configuration to tag version.
 * @param importDest value used in configuration, kept for backwards
 *                   compatibility.
 * @return tag version.
 */
inline TrackData::TagVersion numberTracksDestCfgToTagVersion(int importDest) {
  return TrackData::tagVersionCast(importDest + 1);
}

}

/**
 * Constructor.
 *
 * @param group configuration group
 */
MiscConfig::MiscConfig(const QString& group) :
  GeneralConfig(group),
  m_markTruncations(true),
  m_enableTotalNumberOfTracks(false),
  m_genreNotNumeric(false),
  m_preserveTime(false),
  m_markChanges(true),
  m_commentName(defaultCommentName),
  m_pictureNameItem(VP_METADATA_BLOCK_PICTURE),
  m_nameFilter(""),
  m_formatText(defaultToFilenameFormats[0]),
  m_formatItem(0),
  m_formatFromFilenameText(defaultFromFilenameFormats[0]),
  m_formatFromFilenameItem(0),
  m_dirFormatText(s_defaultDirFmtList[0]),
  m_dirFormatItem(0),
  m_renDirSrc(TrackData::TagV2V1),
  m_numberTracksDst(TrackData::TagV1),
  m_numberTracksStart(1),
#ifndef CONFIG_USE_KDE
  m_hideToolBar(false),
  m_hideStatusBar(false),
#endif
  m_autoHideTags(true),
  m_hideFile(false),
  m_hideV1(false),
  m_hideV2(false),
  m_hidePicture(false),
  m_id3v2Version(ID3v2_3_0),
  m_textEncodingV1("ISO-8859-1"),
  m_textEncoding(TE_ISO8859_1),
  m_trackNumberDigits(1),
  m_playOnDoubleClick(false),
  m_useProxy(false),
  m_useProxyAuthentication(false),
  m_onlyCustomGenres(false)
#ifndef CONFIG_USE_KDE
  , m_useFont(false), m_fontSize(-1),
  m_dontUseNativeDialogs(
#if defined Q_OS_WIN32 || defined Q_OS_MAC
    false
#else
    true
#endif
  )
#endif
{
}

/**
 * Destructor.
 */
MiscConfig::~MiscConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void MiscConfig::writeToConfig(Kid3Settings* config) const
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  cfg.writeEntry("NameFilter2", m_nameFilter);
  cfg.writeEntry("FormatItem", m_formatItem);
  cfg.writeEntry("FormatItems", m_formatItems);
  cfg.writeEntry("FormatText2", m_formatText);
  cfg.writeEntry("FormatFromFilenameItem", m_formatFromFilenameItem);
  cfg.writeEntry("FormatFromFilenameItems", m_formatFromFilenameItems);
  cfg.writeEntry("FormatFromFilenameText", m_formatFromFilenameText);
  cfg.writeEntry("DirFormatItem", m_dirFormatItem);
  cfg.writeEntry("DirFormatText", m_dirFormatText);
  cfg.writeEntry("RenameDirectorySource", tagVersionToRenDirCfg(m_renDirSrc));
  cfg.writeEntry("NumberTracksDestination",
                 tagVersionToNumberTracksDestCfg(m_numberTracksDst));
  cfg.writeEntry("NumberTracksStartNumber", m_numberTracksStart);
  cfg.writeEntry("MarkTruncations", m_markTruncations);
  cfg.writeEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
  cfg.writeEntry("GenreNotNumeric", m_genreNotNumeric);
  cfg.writeEntry("PreserveTime", m_preserveTime);
  cfg.writeEntry("MarkChanges", m_markChanges);
  cfg.writeEntry("CommentName", m_commentName);
  cfg.writeEntry("PictureNameItem", m_pictureNameItem);
  cfg.writeEntry("SplitterSizes", m_splitterSizes);
  cfg.writeEntry("VSplitterSizes", m_vSplitterSizes);
  cfg.writeEntry("CustomGenres", m_customGenres);
  cfg.writeEntry("AutoHideTags", m_autoHideTags);
  cfg.writeEntry("HideFile", m_hideFile);
  cfg.writeEntry("HideV1", m_hideV1);
  cfg.writeEntry("HideV2", m_hideV2);
  cfg.writeEntry("HidePicture", m_hidePicture);
  cfg.writeEntry("ID3v2Version", m_id3v2Version);
  cfg.writeEntry("TextEncodingV1", m_textEncodingV1);
  cfg.writeEntry("TextEncoding", m_textEncoding);
  cfg.writeEntry("TrackNumberDigits", m_trackNumberDigits);
  cfg.writeEntry("PlayOnDoubleClick", m_playOnDoubleClick);
  cfg.writeEntry("UseProxy", m_useProxy);
  cfg.writeEntry("Proxy", m_proxy);
  cfg.writeEntry("UseProxyAuthentication", m_useProxyAuthentication);
  cfg.writeEntry("ProxyUserName", m_proxyUserName);
  cfg.writeEntry("ProxyPassword", m_proxyPassword);
  cfg.writeEntry("Browser", m_browser);
  cfg.writeEntry("OnlyCustomGenres", m_onlyCustomGenres);

  KConfigGroup menuCmdCfg = config->group("MenuCommands");
  int cmdNr = 1;
  for (QList<MenuCommand>::const_iterator
         it = m_contextMenuCommands.begin();
       it != m_contextMenuCommands.end();
       ++it) {
    menuCmdCfg.writeEntry(QString("Command%1").arg(cmdNr++), (*it).toStringList());
  }
  // delete entries which are no longer used
  for (;;) {
    QStringList strList = menuCmdCfg.readEntry(QString("Command%1").arg(cmdNr), QStringList());
    if (strList.empty()) {
      break;
    }
    menuCmdCfg.deleteEntry(QString("Command%1").arg(cmdNr));
    ++cmdNr;
  }
#else
  config->beginGroup("/" + m_group);
  config->setValue("/NameFilter2", QVariant(m_nameFilter));
  config->setValue("/FormatItem", QVariant(m_formatItem));
  config->setValue("/FormatItems", QVariant(m_formatItems));
  config->setValue("/FormatText2", QVariant(m_formatText));
  config->setValue("/FormatFromFilenameItem", QVariant(m_formatFromFilenameItem));
  config->setValue("/FormatFromFilenameItems", QVariant(m_formatFromFilenameItems));
  config->setValue("/FormatFromFilenameText", QVariant(m_formatFromFilenameText));
  config->setValue("/DirFormatItem", QVariant(m_dirFormatItem));
  config->setValue("/DirFormatText", QVariant(m_dirFormatText));
  config->setValue("/RenameDirectorySource", QVariant(tagVersionToRenDirCfg(m_renDirSrc)));
  config->setValue("/NumberTracksDestination",
                   QVariant(tagVersionToNumberTracksDestCfg(m_numberTracksDst)));
  config->setValue("/NumberTracksStartNumber", QVariant(m_numberTracksStart));
  config->setValue("/MarkTruncations", QVariant(m_markTruncations));
  config->setValue("/EnableTotalNumberOfTracks", QVariant(m_enableTotalNumberOfTracks));
  config->setValue("/GenreNotNumeric", QVariant(m_genreNotNumeric));
  config->setValue("/PreserveTime", QVariant(m_preserveTime));
  config->setValue("/MarkChanges", QVariant(m_markChanges));
  config->setValue("/CommentName", QVariant(m_commentName));
  config->setValue("/PictureNameItem", QVariant(m_pictureNameItem));

  QList<int>::const_iterator it;
  int i;
  for (it = m_splitterSizes.begin(), i = 0;
     it != m_splitterSizes.end();
     ++it, ++i) {
    config->setValue("/SplitterSize" + QString::number(i), QVariant(*it));
  }
  for (it = m_vSplitterSizes.begin(), i = 0;
     it != m_vSplitterSizes.end();
     ++it, ++i) {
    config->setValue("/VSplitterSize" + QString::number(i), QVariant(*it));
  }
  config->setValue("/CustomGenres", QVariant(m_customGenres));
  config->setValue("/HideToolBar", QVariant(m_hideToolBar));
  config->setValue("/HideStatusBar", QVariant(m_hideStatusBar));
  config->setValue("/AutoHideTags", QVariant(m_autoHideTags));
  config->setValue("/HideFile", QVariant(m_hideFile));
  config->setValue("/HideV1", QVariant(m_hideV1));
  config->setValue("/HideV2", QVariant(m_hideV2));
  config->setValue("/HidePicture", QVariant(m_hidePicture));
  config->setValue("/ID3v2Version", QVariant(m_id3v2Version));
  config->setValue("/TextEncodingV1", QVariant(m_textEncodingV1));
  config->setValue("/TextEncoding", QVariant(m_textEncoding));
  config->setValue("/TrackNumberDigits", QVariant(m_trackNumberDigits));
  config->setValue("/PlayOnDoubleClick", QVariant(m_playOnDoubleClick));
  config->setValue("/UseProxy", QVariant(m_useProxy));
  config->setValue("/Proxy", QVariant(m_proxy));
  config->setValue("/UseProxyAuthentication", QVariant(m_useProxyAuthentication));
  config->setValue("/ProxyUserName", QVariant(m_proxyUserName));
  config->setValue("/ProxyPassword", QVariant(m_proxyPassword));
  config->setValue("/Browser", QVariant(m_browser));
  config->setValue("/OnlyCustomGenres", QVariant(m_onlyCustomGenres));
  config->setValue("/Geometry", m_geometry);
  config->setValue("/WindowState", m_windowState);
  config->setValue("/UseFont", QVariant(m_useFont));
  config->setValue("/FontFamily", QVariant(m_fontFamily));
  config->setValue("/FontSize", QVariant(m_fontSize));
  config->setValue("/Style", QVariant(m_style));
  config->setValue("/DontUseNativeDialogs", QVariant(m_dontUseNativeDialogs));
  config->endGroup();

  config->beginGroup("/MenuCommands");
  int cmdNr = 1;
  for (QList<MenuCommand>::const_iterator
         it = m_contextMenuCommands.begin();
       it != m_contextMenuCommands.end();
       ++it) {
    config->setValue(QString("/Command%1").arg(cmdNr++), QVariant((*it).toStringList()));
  }
  // delete entries which are no longer used
  for (;;) {
    QStringList strList = config->value(QString("/Command%1").arg(cmdNr)).toStringList();
    if (strList.empty()) {
      break;
    }
    config->remove(QString("/Command%1").arg(cmdNr));
    ++cmdNr;
  }
  config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void MiscConfig::readFromConfig(Kid3Settings* config)
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  m_nameFilter =
      cfg.readEntry("NameFilter2", "");
  m_formatItem =
      cfg.readEntry("FormatItem", 0);
  m_formatItems =
      cfg.readEntry("FormatItems", QStringList());
  m_formatFromFilenameItem =
      cfg.readEntry("FormatFromFilenameItem", 0);
  m_formatFromFilenameItems =
      cfg.readEntry("FormatFromFilenameItems", QStringList());
  m_dirFormatItem =
      cfg.readEntry("DirFormatItem", 0);
  m_renDirSrc = renDirCfgToTagVersion(cfg.readEntry("RenameDirectorySource", 0));
  m_numberTracksDst = numberTracksDestCfgToTagVersion(
        cfg.readEntry("NumberTracksDestination", 0));
  m_numberTracksStart = cfg.readEntry("NumberTracksStartNumber", 1);
  m_markTruncations = cfg.readEntry("MarkTruncations", m_markTruncations);
  m_enableTotalNumberOfTracks = cfg.readEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
  m_genreNotNumeric = cfg.readEntry("GenreNotNumeric", m_genreNotNumeric);
  m_preserveTime = cfg.readEntry("PreserveTime", m_preserveTime);
  m_markChanges = cfg.readEntry("MarkChanges", m_markChanges);
  m_commentName = cfg.readEntry("CommentName", defaultCommentName);
  m_pictureNameItem = cfg.readEntry("PictureNameItem",
      static_cast<int>(VP_METADATA_BLOCK_PICTURE));
  m_formatText =
      cfg.readEntry("FormatText2", defaultToFilenameFormats[0]);
  m_formatFromFilenameText =
      cfg.readEntry("FormatFromFilenameText", defaultFromFilenameFormats[0]);
  m_dirFormatText =
      cfg.readEntry("DirFormatText", s_defaultDirFmtList[0]);
  m_splitterSizes = cfg.readEntry("SplitterSizes", QList<int>());
  m_vSplitterSizes = cfg.readEntry("VSplitterSizes", QList<int>());
  m_customGenres = cfg.readEntry("CustomGenres", QStringList());
  m_autoHideTags = cfg.readEntry("AutoHideTags", m_autoHideTags);
  m_hideFile = cfg.readEntry("HideFile", m_hideFile);
  m_hideV1 = cfg.readEntry("HideV1", m_hideV1);
  m_hideV2 = cfg.readEntry("HideV2", m_hideV2);
  m_hidePicture = cfg.readEntry("HidePicture", m_hidePicture);
  m_id3v2Version = cfg.readEntry("ID3v2Version", static_cast<int>(ID3v2_3_0));
  m_textEncodingV1 = cfg.readEntry("TextEncodingV1", "ISO-8859-1");
  m_textEncoding = cfg.readEntry("TextEncoding", static_cast<int>(TE_ISO8859_1));
  m_trackNumberDigits = cfg.readEntry("TrackNumberDigits", 1);
  m_playOnDoubleClick = cfg.readEntry("PlayOnDoubleClick", m_playOnDoubleClick);
  m_useProxy = cfg.readEntry("UseProxy", m_useProxy);
  m_proxy = cfg.readEntry("Proxy", m_proxy);
  m_useProxyAuthentication = cfg.readEntry("UseProxyAuthentication", m_useProxyAuthentication);
  m_proxyUserName = cfg.readEntry("ProxyUserName", m_proxyUserName);
  m_proxyPassword = cfg.readEntry("ProxyPassword", m_proxyPassword);
  m_browser = cfg.readEntry("Browser", defaultBrowser);
  m_onlyCustomGenres = cfg.readEntry("OnlyCustomGenres", m_onlyCustomGenres);

  m_contextMenuCommands.clear();
  KConfigGroup menuCmdCfg = config->group("MenuCommands");
  int cmdNr = 1;
  for (;;) {
    QStringList strList = menuCmdCfg.readEntry(QString("Command%1").arg(cmdNr), QStringList());
    if (strList.empty()) {
      break;
    }
    m_contextMenuCommands.push_back(MiscConfig::MenuCommand(strList));
    ++cmdNr;
  }
#else
  config->beginGroup("/" + m_group);
  m_nameFilter =
      config->value("/NameFilter2", "").toString();
  m_formatItem =
      config->value("/FormatItem", 0).toInt();
  m_formatItems =
      config->value("/FormatItems").toStringList();
  m_formatFromFilenameItem =
      config->value("/FormatFromFilenameItem", 0).toInt();
  m_formatFromFilenameItems =
      config->value("/FormatFromFilenameItems").toStringList();
  m_dirFormatItem =
      config->value("/DirFormatItem", 0).toInt();
  m_renDirSrc = renDirCfgToTagVersion(config->value("/RenameDirectorySource", 0).toInt());
  m_numberTracksDst = numberTracksDestCfgToTagVersion(
        config->value("/NumberTracksDestination", 0).toInt());
  m_numberTracksStart = config->value("/NumberTracksStartNumber", 1).toInt();
  m_markTruncations = config->value("/MarkTruncations", m_markTruncations).toBool();
  m_enableTotalNumberOfTracks = config->value("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks).toBool();
  m_genreNotNumeric = config->value("/GenreNotNumeric", m_genreNotNumeric).toBool();
  m_preserveTime = config->value("/PreserveTime", m_preserveTime).toBool();
  m_markChanges = config->value("/MarkChanges", m_markChanges).toBool();
  m_commentName = config->value("/CommentName", defaultCommentName).toString();
  m_pictureNameItem = config->value("/PictureNameItem", VP_METADATA_BLOCK_PICTURE).toInt();

  m_formatText =
      config->value("/FormatText2", defaultToFilenameFormats[0]).toString();
  m_formatFromFilenameText =
      config->value("/FormatFromFilenameText", defaultFromFilenameFormats[0]).toString();
  m_dirFormatText =
      config->value("/DirFormatText", s_defaultDirFmtList[0]).toString();
  m_splitterSizes.clear();
  for (int i = 0; i < 5; ++i) {
    int val = config->value("/SplitterSize" + QString::number(i), -1).toInt();
    if (val != -1) {
      m_splitterSizes.push_back(val);
    } else {
      break;
    }
  }
  m_vSplitterSizes.clear();
  for (int j = 0; j < 5; ++j) {
    int val = config->value("/VSplitterSize" + QString::number(j), -1).toInt();
    if (val != -1) {
      m_vSplitterSizes.push_back(val);
    } else {
      break;
    }
  }
  m_customGenres = config->value("/CustomGenres").toStringList();
  m_hideToolBar = config->value("/HideToolBar", m_hideToolBar).toBool();
  m_hideStatusBar = config->value("/HideStatusBar", m_hideStatusBar).toBool();
  m_autoHideTags = config->value("/AutoHideTags", m_autoHideTags).toBool();
  m_hideFile = config->value("/HideFile", m_hideFile).toBool();
  m_hideV1 = config->value("/HideV1", m_hideV1).toBool();
  m_hideV2 = config->value("/HideV2", m_hideV2).toBool();
  m_hidePicture = config->value("/HidePicture", m_hidePicture).toBool();
  m_id3v2Version = config->value("/ID3v2Version", ID3v2_3_0).toInt();
  m_textEncodingV1 = config->value("/TextEncodingV1", "ISO-8859-1").toString();
  m_textEncoding = config->value("/TextEncoding", TE_ISO8859_1).toInt();
  m_trackNumberDigits = config->value("/TrackNumberDigits", 1).toInt();
  m_playOnDoubleClick = config->value("/PlayOnDoubleClick", m_playOnDoubleClick).toBool();
  m_useProxy = config->value("/UseProxy", m_useProxy).toBool();
  m_proxy = config->value("/Proxy", m_proxy).toString();
  m_useProxyAuthentication = config->value("/UseProxyAuthentication", m_useProxyAuthentication).toBool();
  m_proxyUserName = config->value("/ProxyUserName", m_proxyUserName).toString();
  m_proxyPassword = config->value("/ProxyPassword", m_proxyPassword).toString();
#if defined _WIN32 || defined WIN32
  m_browser = config->value("/Browser", QString()).toString();
  if (m_browser.isEmpty()) {
    m_browser = ::getenv("ProgramFiles");
    m_browser += "\\Internet Explorer\\IEXPLORE.EXE";
  }
#else
  m_browser = config->value("/Browser", defaultBrowser).toString();
#endif
  m_onlyCustomGenres = config->value("/OnlyCustomGenres", m_onlyCustomGenres).toBool();
  m_geometry = config->value("/Geometry").toByteArray();
  m_windowState = config->value("/WindowState").toByteArray();
  m_useFont = config->value("/UseFont", m_useFont).toBool();
  m_fontFamily = config->value("/FontFamily", m_fontFamily).toString();
  m_fontSize = config->value("/FontSize", -1).toInt();
  m_style = config->value("/Style", m_style).toString();
  m_dontUseNativeDialogs = config->value("/DontUseNativeDialogs",
                                         m_dontUseNativeDialogs).toBool();
  config->endGroup();

  m_contextMenuCommands.clear();
  config->beginGroup("/MenuCommands");
  int cmdNr = 1;
  for (;;) {
    QStringList strList = config->value(QString("/Command%1").arg(cmdNr)).toStringList();
    if (strList.empty()) {
      break;
    }
    m_contextMenuCommands.push_back(MiscConfig::MenuCommand(strList));
    ++cmdNr;
  }
  config->endGroup();
#endif
  if (cmdNr == 1) {
#if defined _WIN32 || defined WIN32
    QString prgDir = ::getenv("ProgramFiles");
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand(
        "Windows Media Player",
        QString('"') + prgDir + "\\Windows Media Player\\wmplayer.exe\" %{files}"));
#elif !defined __APPLE__
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("Amarok", "amarok %{files}"));
#endif
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("Google Images", "%{browser} http://images.google.com/images?q=%u{artist}%20%u{album}"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("Amazon", "%{browser} http://www.amazon.com/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("LyricWiki", "%{browser} http://lyricwiki.org/%u{artist}:%u{title}"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("Lyrics.com", "%{browser} http://www.lyrics.com/search.php?keyword=%u{artist}+%u{title}&what=all"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("AZLyrics", "%{browser} http://search.azlyrics.com/search.php?q=%u{artist}+%u{title}"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("Dark Lyrics", "%{browser} http://www.darklyrics.com/search?q=%u{album}"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("Metro Lyrics", "%{browser} http://www.metrolyrics.com/search.php?category=artisttitle&search=%u{artist}+%u{title}"));
    m_contextMenuCommands.push_back(
      MiscConfig::MenuCommand("SongLyrics", "%{browser} http://www.songlyrics.com/index.php?section=search&searchW=%u{artist}+%u{title}"));
  }
  if (m_formatItems.isEmpty()) {
    for (const char** sl = defaultToFilenameFormats; *sl != 0; ++sl) {
      m_formatItems += *sl;
    }
  }
  if (m_formatFromFilenameItems.isEmpty()) {
    for (const char** sl = defaultFromFilenameFormats; *sl != 0; ++sl) {
      m_formatFromFilenameItems += *sl;
    }
  }
}

/**
 * Constructor.
 *
 * @param name display name
 * @param cmd  command string with argument codes
 * @param confirm true if confirmation required
 * @param showOutput true if output of command shall be shown
 */
MiscConfig::MenuCommand::MenuCommand(const QString& name, const QString& cmd,
                                     bool confirm, bool showOutput) :
  m_name(name), m_cmd(cmd), m_confirm(confirm), m_showOutput(showOutput)
{
}

/**
 * Constructor.
 *
 * @param strList string list with encoded command
 */
MiscConfig::MenuCommand::MenuCommand(const QStringList& strList)
{
  if (strList.size() == 3) {
    bool ok;
    uint flags = strList[2].toUInt(&ok);
    if (ok) {
      m_confirm = (flags & 1) != 0;
      m_showOutput = (flags & 2) != 0;
      m_name = strList[0];
      m_cmd = strList[1];
    } else {
      m_confirm = false;
      m_showOutput = false;
    }
  }
}

/**
 * Encode into string list.
 *
 * @return string list with encoded command.
 */
QStringList MiscConfig::MenuCommand::toStringList() const {
  QStringList strList;
  strList.push_back(m_name);
  strList.push_back(m_cmd);
  uint flags = (m_confirm ? 1 : 0) | (m_showOutput ? 2 : 0);
  strList.push_back(QString::number(flags));
  return strList;
}