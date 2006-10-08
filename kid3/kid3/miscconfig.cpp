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
const QString MiscConfig::defaultNameFilter(
	"*.mp3 *.MP3 *.Mp3 *.mP3 "
	"*.ogg *.ogG *.oGg *.oGG *.Ogg *.OgG *.OGg *.OGG");
#else
const QString MiscConfig::defaultNameFilter("*.mp3 *.MP3 *.Mp3 *.mP3");
#endif

/** Default value for comment name */
const QString MiscConfig::defaultCommentName("COMMENT");

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
const char** MiscConfig::defaultFnFmtList = &fnFmt[0];

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
const char** MiscConfig::defaultDirFmtList = &dirFmt[0];


/**
 * Constructor.
 *
 * @param group configuration group
 */
MiscConfig::MiscConfig(const QString &group) :
	GeneralConfig(group),
	m_enableTotalNumberOfTracks(false),
	m_preserveTime(false),
	m_commentName(defaultCommentName),
	nameFilter(defaultNameFilter),
	formatText(defaultFnFmtList[0]),
	formatItem(0),
	dirFormatText(defaultDirFmtList[0]),
	dirFormatItem(0),
	m_hideV1(false),
	m_hideV2(false),
	m_id3v2Version(ID3v2_3_0)
#ifndef CONFIG_USE_KDE
	, windowWidth(-1), windowHeight(-1)
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
	KConfig *config
#else
	Kid3Settings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	config->writeEntry("NameFilter2", nameFilter);
	config->writeEntry("FormatItem", formatItem);
	config->writeEntry("FormatText2", formatText);
	config->writeEntry("DirFormatItem", dirFormatItem);
	config->writeEntry("DirFormatText", dirFormatText);
	config->writeEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	config->writeEntry("PreserveTime", m_preserveTime);
	config->writeEntry("CommentName", m_commentName);
#if QT_VERSION >= 300
	config->writeEntry("ContextMenuCommands", m_contextMenuCommands);
#endif
	config->writeEntry("SplitterSizes", splitterSizes);
	config->writeEntry("VSplitterSizes", m_vSplitterSizes);
	config->writeEntry("CustomGenres", m_customGenres);
	config->writeEntry("HideV1", m_hideV1);
	config->writeEntry("HideV2", m_hideV2);
	config->writeEntry("ID3v2Version", m_id3v2Version);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/NameFilter2", nameFilter);
	config->writeEntry("/FormatItem", formatItem);
	config->writeEntry("/FormatText2", formatText);
	config->writeEntry("/DirFormatItem", dirFormatItem);
	config->writeEntry("/DirFormatText", dirFormatText);
	config->writeEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	config->writeEntry("/PreserveTime", m_preserveTime);
	config->writeEntry("/CommentName", m_commentName);

#if QT_VERSION >= 300
	config->writeEntry("/ContextMenuCommands", m_contextMenuCommands);
	QValueList<int>::const_iterator it;
#else
	QValueListConstIterator<int> it;
#endif
	int i;
	for (it = splitterSizes.begin(), i = 0;
		 it != splitterSizes.end();
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
	config->writeEntry("/WindowWidth", windowWidth);
	config->writeEntry("/WindowHeight", windowHeight);
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
	KConfig *config
#else
	Kid3Settings *config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	nameFilter =
	    config->readEntry("NameFilter2", defaultNameFilter);
	formatItem =
	    config->readNumEntry("FormatItem", 0);
	dirFormatItem =
	    config->readNumEntry("DirFormatItem", 0);
	m_enableTotalNumberOfTracks = config->readBoolEntry("EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_preserveTime = config->readBoolEntry("PreserveTime", m_preserveTime);
	m_commentName = config->readEntry("CommentName", defaultCommentName);
#if QT_VERSION >= 300
	m_contextMenuCommands = config->readListEntry("ContextMenuCommands"
#if KDE_VERSION >= 0x30200
																								, QStringList("xmms")
#endif
		);
	formatText =
	    config->readEntry("FormatText2", defaultFnFmtList[0]);
	dirFormatText =
	    config->readEntry("DirFormatText", defaultDirFmtList[0]);
#endif
	splitterSizes = config->readIntListEntry("SplitterSizes");
	m_vSplitterSizes = config->readIntListEntry("VSplitterSizes");
	m_customGenres = config->readListEntry("CustomGenres");
	m_hideV1 = config->readBoolEntry("HideV1", m_hideV1);
	m_hideV2 = config->readBoolEntry("HideV2", m_hideV2);
	m_id3v2Version = config->readNumEntry("ID3v2Version", ID3v2_3_0);
#else
	config->beginGroup("/" + group);
	nameFilter =
	    config->readEntry("/NameFilter2", defaultNameFilter);
	formatItem =
	    config->readNumEntry("/FormatItem", 0);
	dirFormatItem =
	    config->readNumEntry("/DirFormatItem", 0);
	m_enableTotalNumberOfTracks = config->readBoolEntry("/EnableTotalNumberOfTracks", m_enableTotalNumberOfTracks);
	m_preserveTime = config->readBoolEntry("/PreserveTime", m_preserveTime);
	m_commentName = config->readEntry("/CommentName", defaultCommentName);
#if QT_VERSION >= 300
	bool ok;
	m_contextMenuCommands = config->readListEntry("/ContextMenuCommands", &ok);
	if (!ok) {
		m_contextMenuCommands = "xmms";
	}
	formatText =
	    config->readEntry("/FormatText2", defaultFnFmtList[0]);
	dirFormatText =
	    config->readEntry("/DirFormatText", defaultDirFmtList[0]);
#endif
	splitterSizes.clear();
	for (int i = 0; i < 5; ++i) {
		int val = config->readNumEntry("/SplitterSize" + QString::number(i), -1);
		if (val != -1) {
#if QT_VERSION >= 300
			splitterSizes.push_back(val);
#else
			splitterSizes.append(val);
#endif
		} else {
			break;
		}
	}
	m_vSplitterSizes.clear();
	for (int j = 0; j < 5; ++j) {
		int val = config->readNumEntry("/VSplitterSize" + QString::number(j), -1);
		if (val != -1) {
#if QT_VERSION >= 300
			m_vSplitterSizes.push_back(val);
#else
			m_vSplitterSizes.append(val);
#endif
		} else {
			break;
		}
	}
	m_customGenres = config->readListEntry("/CustomGenres");
	m_hideV1 = config->readBoolEntry("/HideV1", m_hideV1);
	m_hideV2 = config->readBoolEntry("/HideV2", m_hideV2);
	m_id3v2Version = config->readNumEntry("/ID3v2Version", ID3v2_3_0);
	windowWidth = config->readNumEntry("/WindowWidth", -1);
	windowHeight = config->readNumEntry("/WindowHeight", -1);
	config->endGroup();
#endif
}
