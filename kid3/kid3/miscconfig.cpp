/**
 * \file miscconfig.cpp
 * Miscellaneous Configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Jan 2004
 *
 * Copyright (C) 2004-2007  Urs Fleisch
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
#else
#include <qfile.h>
#endif

#include "generalconfig.h"
#include "filelist.h"
#include "taggedfile.h"
#include "rendirdialog.h"
#include "miscconfig.h"

/** Default name filter */
#ifdef HAVE_VORBIS
const char* const MiscConfig::s_defaultNameFilter =
	"*.mp3 *.MP3 *.Mp3 *.mP3 "
	"*.ogg *.ogG *.oGg *.oGG *.Ogg *.OgG *.OGg *.OGG";
#else
const char* const MiscConfig::s_defaultNameFilter = "*.mp3 *.MP3 *.Mp3 *.mP3";
#endif

/** Default value for comment name */
const char* const MiscConfig::s_defaultCommentName = "COMMENT";

/** Default value for web browser */
const char* const MiscConfig::s_defaultBrowser =
#ifdef CONFIG_USE_KDE
	"konqueror";
#else
	"firefox";
#endif

/** Default filename format list */
static const char* fnFmt[] = {
	"%a - %l/%t %s",
	"%a - %l/%t. %s",
	"%a - [%y] %l/%t %s",
	"%a - [%y] %l/%t. %s",
	"%a/%l/%t %s",
	"%a/%l/%t. %s",
	"%a/[%y] %l/%t %s",
	"%a/[%y] %l/%t. %s",
	"%l/%t - %a - %s",
	"%l/%t. %a - %s",
	"%l/%a - %t - %s",
	"[%y] %l/%t - %a - %s",
	"%a - %l - %t - %s",
	"%a - [%y] %l - %t - %s",
	"%l/%a - %t - %s",
	"[%y] %l/%a - %t - %s",
	"%l/%a - %s",
	"%l/%a-%s",
	"%l/(%a) %s",
	"%a-%s-%l",
	0
};

/** Default filename format list */
const char** MiscConfig::s_defaultFnFmtList = &fnFmt[0];

/** Default directory format list */
static const char* dirFmt[] = {
	"%a - %l",
	"%a - [%y] %l",
	"%a/%l",
	"%a/[%y] %l",
	"%l",
	"[%y] %l",
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
	m_preserveTime(false),
	m_commentName(s_defaultCommentName),
	m_nameFilter(s_defaultNameFilter),
	m_formatText(s_defaultFnFmtList[0]),
	m_formatItem(0),
	m_dirFormatText(s_defaultDirFmtList[0]),
	m_dirFormatItem(0),
	m_renDirSrc(0),
	m_hideV1(false),
	m_hideV2(false),
	m_id3v2Version(ID3v2_3_0),
	m_useProxy(false),
	m_onlyCustomGenres(false)
#ifndef CONFIG_USE_KDE
	, m_windowWidth(-1), m_windowHeight(-1)
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
 * @param group  configuration group
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
	cfg.writeEntry("FormatText2", m_formatText);
	cfg.writeEntry("DirFormatItem", m_dirFormatItem);
	cfg.writeEntry("DirFormatText", m_dirFormatText);
	cfg.writeEntry("RenameDirectorySource", m_renDirSrc);
	cfg.writeEntry("MarkTruncations", m_markTruncations);
	cfg.writeEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	cfg.writeEntry("PreserveTime", m_preserveTime);
	cfg.writeEntry("CommentName", m_commentName);
	cfg.writeEntry("SplitterSizes", m_splitterSizes);
	cfg.writeEntry("VSplitterSizes", m_vSplitterSizes);
	cfg.writeEntry("CustomGenres", m_customGenres);
	cfg.writeEntry("HideV1", m_hideV1);
	cfg.writeEntry("HideV2", m_hideV2);
	cfg.writeEntry("ID3v2Version", m_id3v2Version);
	cfg.writeEntry("UseProxy", m_useProxy);
	cfg.writeEntry("Proxy", m_proxy);
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
	config->QCM_writeEntry("/FormatText2", m_formatText);
	config->QCM_writeEntry("/DirFormatItem", m_dirFormatItem);
	config->QCM_writeEntry("/DirFormatText", m_dirFormatText);
	config->QCM_writeEntry("/RenameDirectorySource", m_renDirSrc);
	config->QCM_writeEntry("/MarkTruncations", m_markTruncations);
	config->QCM_writeEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	config->QCM_writeEntry("/PreserveTime", m_preserveTime);
	config->QCM_writeEntry("/CommentName", m_commentName);

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
	config->QCM_writeEntry("/HideV1", m_hideV1);
	config->QCM_writeEntry("/HideV2", m_hideV2);
	config->QCM_writeEntry("/ID3v2Version", m_id3v2Version);
	config->QCM_writeEntry("/UseProxy", m_useProxy);
	config->QCM_writeEntry("/Proxy", m_proxy);
	config->QCM_writeEntry("/Browser", m_browser);
	config->QCM_writeEntry("/OnlyCustomGenres", m_onlyCustomGenres);
	config->QCM_writeEntry("/WindowWidth", m_windowWidth);
	config->QCM_writeEntry("/WindowHeight", m_windowHeight);
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
 * @param group  configuration group
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
	    cfg.readEntry("NameFilter2", s_defaultNameFilter);
	m_formatItem =
	    cfg.KCM_readNumEntry("FormatItem", 0);
	m_dirFormatItem =
	    cfg.KCM_readNumEntry("DirFormatItem", 0);
	m_renDirSrc = cfg.KCM_readBoolEntry("RenameDirectorySource", m_renDirSrc);
	m_markTruncations = cfg.KCM_readBoolEntry("MarkTruncations", m_markTruncations);
	m_enableTotalNumberOfTracks = cfg.KCM_readBoolEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_preserveTime = cfg.KCM_readBoolEntry("PreserveTime", m_preserveTime);
	m_commentName = cfg.readEntry("CommentName", s_defaultCommentName);
	m_formatText =
	    cfg.readEntry("FormatText2", s_defaultFnFmtList[0]);
	m_dirFormatText =
	    cfg.readEntry("DirFormatText", s_defaultDirFmtList[0]);
	m_splitterSizes = cfg.KCM_readIntListEntry("SplitterSizes");
	m_vSplitterSizes = cfg.KCM_readIntListEntry("VSplitterSizes");
	m_customGenres = cfg.KCM_readListEntry("CustomGenres");
	m_hideV1 = cfg.KCM_readBoolEntry("HideV1", m_hideV1);
	m_hideV2 = cfg.KCM_readBoolEntry("HideV2", m_hideV2);
	m_id3v2Version = cfg.KCM_readNumEntry("ID3v2Version", static_cast<int>(ID3v2_3_0));
	m_useProxy = cfg.KCM_readBoolEntry("UseProxy", m_useProxy);
	m_proxy = cfg.readEntry("Proxy", m_proxy);
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
	    config->QCM_readEntry("/NameFilter2", s_defaultNameFilter);
	m_formatItem =
	    config->QCM_readNumEntry("/FormatItem", 0);
	m_dirFormatItem =
	    config->QCM_readNumEntry("/DirFormatItem", 0);
	m_renDirSrc = config->QCM_readBoolEntry("/RenameDirectorySource", m_renDirSrc);
	m_markTruncations = config->QCM_readBoolEntry("/MarkTruncations", m_markTruncations);
	m_enableTotalNumberOfTracks = config->QCM_readBoolEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_preserveTime = config->QCM_readBoolEntry("/PreserveTime", m_preserveTime);
	m_commentName = config->QCM_readEntry("/CommentName", s_defaultCommentName);

	m_formatText =
	    config->QCM_readEntry("/FormatText2", s_defaultFnFmtList[0]);
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
	m_hideV1 = config->QCM_readBoolEntry("/HideV1", m_hideV1);
	m_hideV2 = config->QCM_readBoolEntry("/HideV2", m_hideV2);
	m_id3v2Version = config->QCM_readNumEntry("/ID3v2Version", ID3v2_3_0);
	m_useProxy = config->QCM_readBoolEntry("/UseProxy", m_useProxy);
	m_proxy = config->QCM_readEntry("/Proxy", m_proxy);
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
	m_windowWidth = config->QCM_readNumEntry("/WindowWidth", -1);
	m_windowHeight = config->QCM_readNumEntry("/WindowHeight", -1);
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
				QString('"') + prgDir + "\\Windows Media Player\\wmplayer.exe\" %F"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand(
				"AlbumArt",
				QString('"') + prgDir +  "\\Album Cover Art Downloader\\albumart-qt.exe\" %d"));
#else
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("xmms", "xmms %F"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("AlbumArt", "albumart-qt %d"));
#endif
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Google Images", "%b http://images.google.com/images?q=%ua%20%ul"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Amazon", "%b http://www.amazon.com/s?field-artist=%ua&field-title=%ul"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("LyricWiki", "%b http://lyricwiki.org/%ua:%us"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("LeosLyrics", "%b http://www.leoslyrics.com/search.php?search=%ua%20%us&sartist=1&ssongtitle=1"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("Lyrc", "%b http://lyrc.com.ar/en/tema1en.php?artist=%ua&songname=%us"));
	}
}

/**
 * Constructor.
 *
 * @param name display name
 * @param cmd  command string with argument codes
 * @param config true if confirmation required
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
