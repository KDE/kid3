/**
 * \file miscconfig.cpp
 * Miscellaneous Configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Jan 2004
 */

#include <qstring.h>
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kdeversion.h>
#include <kconfig.h>
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
	m_renDirSrcV1(true),
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
	config->setGroup(m_group);
	config->writeEntry("NameFilter2", m_nameFilter);
	config->writeEntry("FormatItem", m_formatItem);
	config->writeEntry("FormatText2", m_formatText);
	config->writeEntry("DirFormatItem", m_dirFormatItem);
	config->writeEntry("DirFormatText", m_dirFormatText);
	config->writeEntry("RenameDirectorySourceV1", m_renDirSrcV1);
	config->writeEntry("MarkTruncations", m_markTruncations);
	config->writeEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	config->writeEntry("PreserveTime", m_preserveTime);
	config->writeEntry("CommentName", m_commentName);
	config->writeEntry("SplitterSizes", m_splitterSizes);
	config->writeEntry("VSplitterSizes", m_vSplitterSizes);
	config->writeEntry("CustomGenres", m_customGenres);
	config->writeEntry("HideV1", m_hideV1);
	config->writeEntry("HideV2", m_hideV2);
	config->writeEntry("ID3v2Version", m_id3v2Version);
	config->writeEntry("UseProxy", m_useProxy);
	config->writeEntry("Proxy", m_proxy);
	config->writeEntry("Browser", m_browser);
	config->writeEntry("OnlyCustomGenres", m_onlyCustomGenres);

	config->setGroup("MenuCommands");
	int cmdNr = 1;
	for (Q3ValueList<MiscConfig::MenuCommand>::const_iterator it = m_contextMenuCommands.begin();
			 it != m_contextMenuCommands.end();
			 ++it) {
		config->writeEntry(QString("Command%1").arg(cmdNr++), (*it).toStringList());
	}
#else
	config->beginGroup("/" + m_group);
	config->writeEntry("/NameFilter2", m_nameFilter);
	config->writeEntry("/FormatItem", m_formatItem);
	config->writeEntry("/FormatText2", m_formatText);
	config->writeEntry("/DirFormatItem", m_dirFormatItem);
	config->writeEntry("/DirFormatText", m_dirFormatText);
	config->writeEntry("/RenameDirectorySourceV1", m_renDirSrcV1);
	config->writeEntry("/MarkTruncations", m_markTruncations);
	config->writeEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	config->writeEntry("/PreserveTime", m_preserveTime);
	config->writeEntry("/CommentName", m_commentName);

	Q3ValueList<int>::const_iterator it;
	int i;
	for (it = m_splitterSizes.begin(), i = 0;
		 it != m_splitterSizes.end();
		 ++it, ++i) {
		config->writeEntry("/SplitterSize" + QString::number(i), *it);
	}
	for (it = m_vSplitterSizes.begin(), i = 0;
		 it != m_vSplitterSizes.end();
		 ++it, ++i) {
		config->writeEntry("/VSplitterSize" + QString::number(i), *it);
	}
	config->writeEntry("/CustomGenres", m_customGenres);
	config->writeEntry("/HideV1", m_hideV1);
	config->writeEntry("/HideV2", m_hideV2);
	config->writeEntry("/ID3v2Version", m_id3v2Version);
	config->writeEntry("/UseProxy", m_useProxy);
	config->writeEntry("/Proxy", m_proxy);
	config->writeEntry("/Browser", m_browser);
	config->writeEntry("/OnlyCustomGenres", m_onlyCustomGenres);
	config->writeEntry("/WindowWidth", m_windowWidth);
	config->writeEntry("/WindowHeight", m_windowHeight);
	config->endGroup();

	config->beginGroup("/MenuCommands");
	int cmdNr = 1;
	for (Q3ValueList<MiscConfig::MenuCommand>::const_iterator it = m_contextMenuCommands.begin();
			 it != m_contextMenuCommands.end();
			 ++it) {
		config->writeEntry(QString("/Command%1").arg(cmdNr++), (*it).toStringList());
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
	config->setGroup(m_group);
	m_nameFilter =
	    config->readEntry("NameFilter2", s_defaultNameFilter);
	m_formatItem =
	    config->readNumEntry("FormatItem", 0);
	m_dirFormatItem =
	    config->readNumEntry("DirFormatItem", 0);
	m_renDirSrcV1 = config->readBoolEntry("RenameDirectorySourceV1", m_renDirSrcV1);
	m_markTruncations = config->readBoolEntry("MarkTruncations", m_markTruncations);
	m_enableTotalNumberOfTracks = config->readBoolEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_preserveTime = config->readBoolEntry("PreserveTime", m_preserveTime);
	m_commentName = config->readEntry("CommentName", s_defaultCommentName);
	m_formatText =
	    config->readEntry("FormatText2", s_defaultFnFmtList[0]);
	m_dirFormatText =
	    config->readEntry("DirFormatText", s_defaultDirFmtList[0]);
	m_splitterSizes = config->readIntListEntry("SplitterSizes");
	m_vSplitterSizes = config->readIntListEntry("VSplitterSizes");
	m_customGenres = config->readListEntry("CustomGenres");
	m_hideV1 = config->readBoolEntry("HideV1", m_hideV1);
	m_hideV2 = config->readBoolEntry("HideV2", m_hideV2);
	m_id3v2Version = config->readNumEntry("ID3v2Version", ID3v2_3_0);
	m_useProxy = config->readBoolEntry("UseProxy", m_useProxy);
	m_proxy = config->readEntry("Proxy", m_proxy);
	m_browser = config->readEntry("Browser", s_defaultBrowser);
	m_onlyCustomGenres = config->readBoolEntry("OnlyCustomGenres", m_onlyCustomGenres);

	m_contextMenuCommands.clear();
	config->setGroup("MenuCommands");
	int cmdNr = 1;
	for (;;) {
		QStringList strList = config->readListEntry(QString("Command%1").arg(cmdNr));
		if (strList.empty()) {
			break;
		}
		m_contextMenuCommands.push_back(MiscConfig::MenuCommand(strList));
		++cmdNr;
	}
#else
	config->beginGroup("/" + m_group);
	m_nameFilter =
	    config->readEntry("/NameFilter2", s_defaultNameFilter);
	m_formatItem =
	    config->readNumEntry("/FormatItem", 0);
	m_dirFormatItem =
	    config->readNumEntry("/DirFormatItem", 0);
	m_renDirSrcV1 = config->readBoolEntry("/RenameDirectorySourceV1", m_renDirSrcV1);
	m_markTruncations = config->readBoolEntry("/MarkTruncations", m_markTruncations);
	m_enableTotalNumberOfTracks = config->readBoolEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_preserveTime = config->readBoolEntry("/PreserveTime", m_preserveTime);
	m_commentName = config->readEntry("/CommentName", s_defaultCommentName);

	m_formatText =
	    config->readEntry("/FormatText2", s_defaultFnFmtList[0]);
	m_dirFormatText =
	    config->readEntry("/DirFormatText", s_defaultDirFmtList[0]);
	m_splitterSizes.clear();
	for (int i = 0; i < 5; ++i) {
		int val = config->readNumEntry("/SplitterSize" + QString::number(i), -1);
		if (val != -1) {
			m_splitterSizes.push_back(val);
		} else {
			break;
		}
	}
	m_vSplitterSizes.clear();
	for (int j = 0; j < 5; ++j) {
		int val = config->readNumEntry("/VSplitterSize" + QString::number(j), -1);
		if (val != -1) {
			m_vSplitterSizes.push_back(val);
		} else {
			break;
		}
	}
	m_customGenres = config->readListEntry("/CustomGenres");
	m_hideV1 = config->readBoolEntry("/HideV1", m_hideV1);
	m_hideV2 = config->readBoolEntry("/HideV2", m_hideV2);
	m_id3v2Version = config->readNumEntry("/ID3v2Version", ID3v2_3_0);
	m_useProxy = config->readBoolEntry("/UseProxy", m_useProxy);
	m_proxy = config->readEntry("/Proxy", m_proxy);
	m_browser = config->readEntry("/Browser", s_defaultBrowser);
	m_onlyCustomGenres = config->readBoolEntry("/OnlyCustomGenres", m_onlyCustomGenres);
	m_windowWidth = config->readNumEntry("/WindowWidth", -1);
	m_windowHeight = config->readNumEntry("/WindowHeight", -1);
	config->endGroup();

	m_contextMenuCommands.clear();
	config->beginGroup("/MenuCommands");
	int cmdNr = 1;
	bool ok;
	for (;;) {
		QStringList strList = config->readListEntry(QString("/Command%1").arg(cmdNr), &ok);
		if (!ok) {
			break;
		}
		m_contextMenuCommands.push_back(MiscConfig::MenuCommand(strList));
		++cmdNr;
	}
	config->endGroup();
#endif
	if (cmdNr == 1) {
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("xmms", "xmms %F"));
		m_contextMenuCommands.push_back(
			MiscConfig::MenuCommand("AlbumArt", "albumart-qt %d"));
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
