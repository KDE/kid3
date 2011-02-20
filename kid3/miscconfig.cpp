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

#include <qstring.h>
#include "qtcompatmac.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kdeversion.h>
#include <kconfig.h>
#include <kconfigskeleton.h>
#else
#include <qfile.h>
#endif

#include "generalconfig.h"
#include "filelist.h"
#include "taggedfile.h"
#include "rendirdialog.h"
#include "miscconfig.h"

/** Default value for comment name */
const char* const MiscConfig::s_defaultCommentName = "COMMENT";

/** Default value for web browser */
#ifdef __APPLE__
const char* const MiscConfig::s_defaultBrowser = "open";
#else
const char* const MiscConfig::s_defaultBrowser = "xdg-open";
#endif

/** Default filename format list */
static const char* fnFmt[] = {
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

/** Default filename format list */
const char** MiscConfig::s_defaultFnFmtList = &fnFmt[0];

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
	m_commentName(s_defaultCommentName),
	m_pictureNameItem(VP_METADATA_BLOCK_PICTURE),
	m_nameFilter(""),
	m_formatText(s_defaultFnFmtList[0]),
	m_formatItem(0),
	m_formatFromFilenameText(s_defaultFnFmtList[0]),
	m_formatFromFilenameItem(0),
	m_dirFormatText(s_defaultDirFmtList[0]),
	m_dirFormatItem(0),
	m_renDirSrc(0),
	m_numberTracksDst(0),
	m_numberTracksStart(1),
#ifndef CONFIG_USE_KDE
#if QT_VERSION >= 0x040000
	m_hideToolBar(false),
#endif
	m_hideStatusBar(false),
#endif
	m_autoHideTags(true),
	m_hideFile(false),
	m_hideV1(false),
	m_hideV2(false),
	m_hidePicture(false),
	m_id3v2Version(ID3v2_3_0),
	m_textEncodingV1(""),
	m_textEncoding(TE_ISO8859_1),
	m_trackNumberDigits(1),
	m_useProxy(false),
#if QT_VERSION >= 0x040000
	m_useProxyAuthentication(false),
#endif
	m_onlyCustomGenres(false)
#ifndef CONFIG_USE_KDE
#if QT_VERSION >= 0x040200
	,
#else
	, m_windowX(-1), m_windowY(-1), m_windowWidth(-1), m_windowHeight(-1),
#endif
	m_useFont(false), m_fontSize(-1)
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
void MiscConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	cfg.writeEntry("NameFilter2", m_nameFilter);
	cfg.writeEntry("FormatItem", m_formatItem);
	cfg.writeEntry("FormatItems", m_formatItems);
	cfg.writeEntry("FormatText2", m_formatText);
	cfg.writeEntry("FormatFromFilenameItem", m_formatFromFilenameItem);
	cfg.writeEntry("FormatFromFilenameItems", m_formatFromFilenameItems);
	cfg.writeEntry("FormatFromFilenameText", m_formatFromFilenameText);
	cfg.writeEntry("DirFormatItem", m_dirFormatItem);
	cfg.writeEntry("DirFormatText", m_dirFormatText);
	cfg.writeEntry("RenameDirectorySource", m_renDirSrc);
	cfg.writeEntry("NumberTracksDestination", m_numberTracksDst);
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
	cfg.writeEntry("UseProxy", m_useProxy);
	cfg.writeEntry("Proxy", m_proxy);
#if QT_VERSION >= 0x040000
	cfg.writeEntry("UseProxyAuthentication", m_useProxyAuthentication);
	cfg.writeEntry("ProxyUserName", m_proxyUserName);
	cfg.writeEntry("ProxyPassword", m_proxyPassword);
#endif
	cfg.writeEntry("Browser", m_browser);
	cfg.writeEntry("OnlyCustomGenres", m_onlyCustomGenres);

	KCM_KConfigGroup(menuCmdCfg, config, "MenuCommands");
	int cmdNr = 1;
	for (MiscConfig::MenuCommandList::const_iterator
				 it = m_contextMenuCommands.begin();
			 it != m_contextMenuCommands.end();
			 ++it) {
		menuCmdCfg.writeEntry(QString("Command%1").arg(cmdNr++), (*it).toStringList());
	}
	// delete entries which are no longer used
	for (;;) {
		QStringList strList = menuCmdCfg.KCM_readListEntry(QString("Command%1").arg(cmdNr));
		if (strList.empty()) {
			break;
		}
		menuCmdCfg.deleteEntry(QString("Command%1").arg(cmdNr));
		++cmdNr;
	}
#else
	config->beginGroup("/" + m_group);
	config->QCM_writeEntry("/NameFilter2", m_nameFilter);
	config->QCM_writeEntry("/FormatItem", m_formatItem);
	config->QCM_writeEntry("/FormatItems", m_formatItems);
	config->QCM_writeEntry("/FormatText2", m_formatText);
	config->QCM_writeEntry("/FormatFromFilenameItem", m_formatFromFilenameItem);
	config->QCM_writeEntry("/FormatFromFilenameItems", m_formatFromFilenameItems);
	config->QCM_writeEntry("/FormatFromFilenameText", m_formatFromFilenameText);
	config->QCM_writeEntry("/DirFormatItem", m_dirFormatItem);
	config->QCM_writeEntry("/DirFormatText", m_dirFormatText);
	config->QCM_writeEntry("/RenameDirectorySource", m_renDirSrc);
	config->QCM_writeEntry("/NumberTracksDestination", m_numberTracksDst);
	config->QCM_writeEntry("/NumberTracksStartNumber", m_numberTracksStart);
	config->QCM_writeEntry("/MarkTruncations", m_markTruncations);
	config->QCM_writeEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	config->QCM_writeEntry("/GenreNotNumeric", m_genreNotNumeric);
	config->QCM_writeEntry("/PreserveTime", m_preserveTime);
	config->QCM_writeEntry("/MarkChanges", m_markChanges);
	config->QCM_writeEntry("/CommentName", m_commentName);
	config->QCM_writeEntry("/PictureNameItem", m_pictureNameItem);

#if QT_VERSION >= 0x040000
	QList<int>::const_iterator it;
#else
	QValueList<int>::const_iterator it;
#endif
	int i;
	for (it = m_splitterSizes.begin(), i = 0;
		 it != m_splitterSizes.end();
		 ++it, ++i) {
		config->QCM_writeEntry("/SplitterSize" + QString::number(i), *it);
	}
	for (it = m_vSplitterSizes.begin(), i = 0;
		 it != m_vSplitterSizes.end();
		 ++it, ++i) {
		config->QCM_writeEntry("/VSplitterSize" + QString::number(i), *it);
	}
	config->QCM_writeEntry("/CustomGenres", m_customGenres);
#if QT_VERSION >= 0x040000
	config->QCM_writeEntry("/HideToolBar", m_hideToolBar);
#endif
	config->QCM_writeEntry("/HideStatusBar", m_hideStatusBar);
	config->QCM_writeEntry("/AutoHideTags", m_autoHideTags);
	config->QCM_writeEntry("/HideFile", m_hideFile);
	config->QCM_writeEntry("/HideV1", m_hideV1);
	config->QCM_writeEntry("/HideV2", m_hideV2);
	config->QCM_writeEntry("/HidePicture", m_hidePicture);
	config->QCM_writeEntry("/ID3v2Version", m_id3v2Version);
	config->QCM_writeEntry("/TextEncodingV1", m_textEncodingV1);
	config->QCM_writeEntry("/TextEncoding", m_textEncoding);
	config->QCM_writeEntry("/TrackNumberDigits", m_trackNumberDigits);
	config->QCM_writeEntry("/UseProxy", m_useProxy);
	config->QCM_writeEntry("/Proxy", m_proxy);
#if QT_VERSION >= 0x040000
	config->QCM_writeEntry("/UseProxyAuthentication", m_useProxyAuthentication);
	config->QCM_writeEntry("/ProxyUserName", m_proxyUserName);
	config->QCM_writeEntry("/ProxyPassword", m_proxyPassword);
#endif
	config->QCM_writeEntry("/Browser", m_browser);
	config->QCM_writeEntry("/OnlyCustomGenres", m_onlyCustomGenres);
#if QT_VERSION >= 0x040200
	config->setValue("/Geometry", m_geometry);
	config->setValue("/WindowState", m_windowState);
#else
	config->QCM_writeEntry("/WindowX", m_windowX);
	config->QCM_writeEntry("/WindowY", m_windowY);
	config->QCM_writeEntry("/WindowWidth", m_windowWidth);
	config->QCM_writeEntry("/WindowHeight", m_windowHeight);
#endif
	config->QCM_writeEntry("/UseFont", m_useFont);
	config->QCM_writeEntry("/FontFamily", m_fontFamily);
	config->QCM_writeEntry("/FontSize", m_fontSize);
	config->QCM_writeEntry("/Style", m_style);
	config->endGroup();

	config->beginGroup("/MenuCommands");
	int cmdNr = 1;
	for (MiscConfig::MenuCommandList::const_iterator
				 it = m_contextMenuCommands.begin();
			 it != m_contextMenuCommands.end();
			 ++it) {
		config->QCM_writeEntry(QString("/Command%1").arg(cmdNr++), (*it).toStringList());
	}
	// delete entries which are no longer used
	for (;;) {
		QStringList strList = config->QCM_readListEntry(QString("/Command%1").arg(cmdNr));
		if (strList.empty()) {
			break;
		}
		config->QCM_removeEntry(QString("/Command%1").arg(cmdNr));
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
void MiscConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	m_nameFilter =
	    cfg.readEntry("NameFilter2", "");
	m_formatItem =
	    cfg.KCM_readNumEntry("FormatItem", 0);
	m_formatItems =
	    cfg.KCM_readListEntry("FormatItems");
	m_formatFromFilenameItem =
	    cfg.KCM_readNumEntry("FormatFromFilenameItem", 0);
	m_formatFromFilenameItems =
	    cfg.KCM_readListEntry("FormatFromFilenameItems");
	m_dirFormatItem =
	    cfg.KCM_readNumEntry("DirFormatItem", 0);
	m_renDirSrc = cfg.KCM_readNumEntry("RenameDirectorySource", 0);
	m_numberTracksDst = cfg.KCM_readNumEntry("NumberTracksDestination", 0);
	m_numberTracksStart = cfg.KCM_readNumEntry("NumberTracksStartNumber", 1);
	m_markTruncations = cfg.KCM_readBoolEntry("MarkTruncations", m_markTruncations);
	m_enableTotalNumberOfTracks = cfg.KCM_readBoolEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_genreNotNumeric = cfg.KCM_readBoolEntry("GenreNotNumeric", m_genreNotNumeric);
	m_preserveTime = cfg.KCM_readBoolEntry("PreserveTime", m_preserveTime);
	m_markChanges = cfg.KCM_readBoolEntry("MarkChanges", m_markChanges);
	m_commentName = cfg.readEntry("CommentName", s_defaultCommentName);
	m_pictureNameItem = cfg.KCM_readNumEntry("PictureNameItem",
	    static_cast<int>(VP_METADATA_BLOCK_PICTURE));
	m_formatText =
	    cfg.readEntry("FormatText2", s_defaultFnFmtList[0]);
	m_formatFromFilenameText =
	    cfg.readEntry("FormatFromFilenameText", s_defaultFnFmtList[0]);
	m_dirFormatText =
	    cfg.readEntry("DirFormatText", s_defaultDirFmtList[0]);
	m_splitterSizes = cfg.KCM_readIntListEntry("SplitterSizes");
	m_vSplitterSizes = cfg.KCM_readIntListEntry("VSplitterSizes");
	m_customGenres = cfg.KCM_readListEntry("CustomGenres");
	m_autoHideTags = cfg.KCM_readBoolEntry("AutoHideTags", m_autoHideTags);
	m_hideFile = cfg.KCM_readBoolEntry("HideFile", m_hideFile);
	m_hideV1 = cfg.KCM_readBoolEntry("HideV1", m_hideV1);
	m_hideV2 = cfg.KCM_readBoolEntry("HideV2", m_hideV2);
	m_hidePicture = cfg.KCM_readBoolEntry("HidePicture", m_hidePicture);
	m_id3v2Version = cfg.KCM_readNumEntry("ID3v2Version", static_cast<int>(ID3v2_3_0));
	m_textEncodingV1 = cfg.readEntry("TextEncodingV1", "");
	m_textEncoding = cfg.KCM_readNumEntry("TextEncoding", static_cast<int>(TE_ISO8859_1));
	m_trackNumberDigits = cfg.KCM_readNumEntry("TrackNumberDigits", 1);
	m_useProxy = cfg.KCM_readBoolEntry("UseProxy", m_useProxy);
	m_proxy = cfg.readEntry("Proxy", m_proxy);
#if QT_VERSION >= 0x040000
	m_useProxyAuthentication = cfg.KCM_readBoolEntry("UseProxyAuthentication", m_useProxyAuthentication);
	m_proxyUserName = cfg.readEntry("ProxyUserName", m_proxyUserName);
	m_proxyPassword = cfg.readEntry("ProxyPassword", m_proxyPassword);
#endif
	m_browser = cfg.readEntry("Browser", s_defaultBrowser);
	m_onlyCustomGenres = cfg.KCM_readBoolEntry("OnlyCustomGenres", m_onlyCustomGenres);

	m_contextMenuCommands.clear();
	KCM_KConfigGroup(menuCmdCfg, config, "MenuCommands");
	int cmdNr = 1;
	for (;;) {
		QStringList strList = menuCmdCfg.KCM_readListEntry(QString("Command%1").arg(cmdNr));
		if (strList.empty()) {
			break;
		}
		m_contextMenuCommands.push_back(MiscConfig::MenuCommand(strList));
		++cmdNr;
	}
#else
	config->beginGroup("/" + m_group);
	m_nameFilter =
	    config->QCM_readEntry("/NameFilter2", "");
	m_formatItem =
	    config->QCM_readNumEntry("/FormatItem", 0);
	m_formatItems =
	    config->QCM_readListEntry("/FormatItems");
	m_formatFromFilenameItem =
	    config->QCM_readNumEntry("/FormatFromFilenameItem", 0);
	m_formatFromFilenameItems =
	    config->QCM_readListEntry("/FormatFromFilenameItems");
	m_dirFormatItem =
	    config->QCM_readNumEntry("/DirFormatItem", 0);
	m_renDirSrc = config->QCM_readNumEntry("/RenameDirectorySource", 0);
	m_numberTracksDst = config->QCM_readNumEntry("/NumberTracksDestination", 0);
	m_numberTracksStart = config->QCM_readNumEntry("/NumberTracksStartNumber", 1);
	m_markTruncations = config->QCM_readBoolEntry("/MarkTruncations", m_markTruncations);
	m_enableTotalNumberOfTracks = config->QCM_readBoolEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_genreNotNumeric = config->QCM_readBoolEntry("/GenreNotNumeric", m_genreNotNumeric);
	m_preserveTime = config->QCM_readBoolEntry("/PreserveTime", m_preserveTime);
	m_markChanges = config->QCM_readBoolEntry("/MarkChanges", m_markChanges);
	m_commentName = config->QCM_readEntry("/CommentName", s_defaultCommentName);
	m_pictureNameItem = config->QCM_readNumEntry("/PictureNameItem", VP_METADATA_BLOCK_PICTURE);

	m_formatText =
	    config->QCM_readEntry("/FormatText2", s_defaultFnFmtList[0]);
	m_formatFromFilenameText =
	    config->QCM_readEntry("/FormatFromFilenameText", s_defaultFnFmtList[0]);
	m_dirFormatText =
	    config->QCM_readEntry("/DirFormatText", s_defaultDirFmtList[0]);
	m_splitterSizes.clear();
	for (int i = 0; i < 5; ++i) {
		int val = config->QCM_readNumEntry("/SplitterSize" + QString::number(i), -1);
		if (val != -1) {
			m_splitterSizes.push_back(val);
		} else {
			break;
		}
	}
	m_vSplitterSizes.clear();
	for (int j = 0; j < 5; ++j) {
		int val = config->QCM_readNumEntry("/VSplitterSize" + QString::number(j), -1);
		if (val != -1) {
			m_vSplitterSizes.push_back(val);
		} else {
			break;
		}
	}
	m_customGenres = config->QCM_readListEntry("/CustomGenres");
#if QT_VERSION >= 0x040000
	m_hideToolBar = config->QCM_readBoolEntry("/HideToolBar", m_hideToolBar);
#endif
	m_hideStatusBar = config->QCM_readBoolEntry("/HideStatusBar", m_hideStatusBar);
	m_autoHideTags = config->QCM_readBoolEntry("/AutoHideTags", m_autoHideTags);
	m_hideFile = config->QCM_readBoolEntry("/HideFile", m_hideFile);
	m_hideV1 = config->QCM_readBoolEntry("/HideV1", m_hideV1);
	m_hideV2 = config->QCM_readBoolEntry("/HideV2", m_hideV2);
	m_hidePicture = config->QCM_readBoolEntry("/HidePicture", m_hidePicture);
	m_id3v2Version = config->QCM_readNumEntry("/ID3v2Version", ID3v2_3_0);
	m_textEncodingV1 = config->QCM_readEntry("/TextEncodingV1", "");
	m_textEncoding = config->QCM_readNumEntry("/TextEncoding", TE_ISO8859_1);
	m_trackNumberDigits = config->QCM_readNumEntry("/TrackNumberDigits", 1);
	m_useProxy = config->QCM_readBoolEntry("/UseProxy", m_useProxy);
	m_proxy = config->QCM_readEntry("/Proxy", m_proxy);
#if QT_VERSION >= 0x040000
	m_useProxyAuthentication = config->QCM_readBoolEntry("/UseProxyAuthentication", m_useProxyAuthentication);
	m_proxyUserName = config->QCM_readEntry("/ProxyUserName", m_proxyUserName);
	m_proxyPassword = config->QCM_readEntry("/ProxyPassword", m_proxyPassword);
#endif
#if defined _WIN32 || defined WIN32
	m_browser = config->QCM_readEntry("/Browser", QString());
	if (m_browser.isEmpty()) {
		m_browser = ::getenv("ProgramFiles");
		m_browser += "\\Internet Explorer\\IEXPLORE.EXE";
	}
#else
	m_browser = config->QCM_readEntry("/Browser", s_defaultBrowser);
#endif
	m_onlyCustomGenres = config->QCM_readBoolEntry("/OnlyCustomGenres", m_onlyCustomGenres);
#if QT_VERSION >= 0x040200
	m_geometry = config->value("/Geometry").toByteArray();
	m_windowState = config->value("/WindowState").toByteArray();
#else
	m_windowX = config->QCM_readNumEntry("/WindowX", -1);
	m_windowY = config->QCM_readNumEntry("/WindowY", -1);
	m_windowWidth = config->QCM_readNumEntry("/WindowWidth", -1);
	m_windowHeight = config->QCM_readNumEntry("/WindowHeight", -1);
#endif
	m_useFont = config->QCM_readBoolEntry("/UseFont", m_useFont);
	m_fontFamily = config->QCM_readEntry("/FontFamily", m_fontFamily);
	m_fontSize = config->QCM_readNumEntry("/FontSize", -1);
	m_style = config->QCM_readEntry("/Style", m_style);
	config->endGroup();

	m_contextMenuCommands.clear();
	config->beginGroup("/MenuCommands");
	int cmdNr = 1;
	for (;;) {
		QStringList strList = config->QCM_readListEntry(QString("/Command%1").arg(cmdNr));
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
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand(
				"AlbumArt",
				QString('"') + prgDir +  "\\Album Cover Art Downloader\\albumart-qt.exe\" %{directory}"));
#elif !defined __APPLE__
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Amarok", "amarok %{files}"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("AlbumArt", "albumart-qt %{directory}"));
#endif
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Google Images", "%{browser} http://images.google.com/images?q=%u{artist}%20%u{album}"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Amazon", "%{browser} http://www.amazon.com/s?search-alias=aps&field-keywords=%u{artist}+%u{album}"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("LyricWiki", "%{browser} http://lyricwiki.org/%u{artist}:%u{title}"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("LeosLyrics", "%{browser} http://www.leoslyrics.com/search.php?search=%u{artist}%20%u{title}&sartist=1&ssongtitle=1"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Lyrc", "%{browser} http://lyrc.com.ar/en/tema1en.php?artist=%u{artist}&songname=%u{title}"));
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
