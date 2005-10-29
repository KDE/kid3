/**
 * \file importconfig.cpp
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include <qstring.h>
#include "importconfig.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
ImportConfig::ImportConfig(const QString &grp) :
	GeneralConfig(grp), importDestV1(true),
	enableTimeDifferenceCheck(true), maxTimeDifference(3)
{
	importFormatIdx = 0;
	importFormatNames.append("freedb HTML text");
	importFormatNames.append("freedb HTML source");
	importFormatNames.append("Title");
	importFormatNames.append("Track Title");
	importFormatNames.append("Track Title Time");
	importFormatNames.append("Custom Format");
	/**
	 * Preset import format regular expressions.
	 * The following codes are used before the () expressions.
	 * %s title (song)
	 * %l album
	 * %a artist
	 * %c comment
	 * %y year
	 * %t track
	 * %g genre
	 */
	importFormatHeaders.append("%a(\\S[^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*\\S)[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+)?.*genre:\\s*%g(\\S[^\\r\\n]*\\S)?[\\r\\n]");
	importFormatHeaders.append("<[^>]+>%a([^<\\s][^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*[^\\s>])<[^>]+>[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+)?.*genre:\\s*%g(\\S[^\\r\\n>]*\\S)?<[^>]+>[\\r\\n]");
	importFormatHeaders.append("");
	importFormatHeaders.append("");
	importFormatHeaders.append("");
	importFormatHeaders.append("");
	importFormatTracks.append("[\\r\\n]%t(\\d+)[\\.\\s]+%d(\\d+:\\d+)\\s+%s(\\S[^\\r\\n]*\\S)");
	importFormatTracks.append("<td[^>]*>\\s*%t(\\d+).</td><td[^>]*>\\s*%d(\\d+:\\d+)</td><td[^>]*>(?:<[^>]+>)?%s([^<\\r\\n]+)");
	importFormatTracks.append("\\s*%s(\\S[^\\r\\n]*\\S)\\s*");
	importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s*");
	importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s+%d(\\d+:\\d+)\\s*");
	importFormatTracks.append("");
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
	KConfig *config
#else
	Kid3Settings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	config->writeEntry("ImportDestinationV1", importDestV1);
	config->writeEntry("ImportFormatNames", importFormatNames);
	config->writeEntry("ImportFormatHeaders", importFormatHeaders);
	config->writeEntry("ImportFormatTracks", importFormatTracks);
	config->writeEntry("ImportFormatIdx", importFormatIdx);
	config->writeEntry("EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	config->writeEntry("MaxTimeDifference", maxTimeDifference);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/ImportDestinationV1", importDestV1);
	config->writeEntry("/ImportFormatNames", importFormatNames);
	config->writeEntry("/ImportFormatHeaders", importFormatHeaders);
	config->writeEntry("/ImportFormatTracks", importFormatTracks);
	config->writeEntry("/ImportFormatIdx", importFormatIdx);
	config->writeEntry("/EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	config->writeEntry("/MaxTimeDifference", maxTimeDifference);
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
	KConfig *config
#else
	Kid3Settings *config
#endif
	)
{
	QStringList names, headers, tracks;
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	importDestV1 = config->readBoolEntry("ImportDestinationV1", importDestV1);
	names = config->readListEntry("ImportFormatNames");
	headers = config->readListEntry("ImportFormatHeaders");
	tracks = config->readListEntry("ImportFormatTracks");
	importFormatIdx = config->readNumEntry("ImportFormatIdx", importFormatIdx);
	enableTimeDifferenceCheck = config->readBoolEntry("EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	maxTimeDifference = config->readNumEntry("MaxTimeDifference", maxTimeDifference);
#else
	config->beginGroup("/" + group);
	importDestV1 = config->readBoolEntry("/ImportDestinationV1", importDestV1);
	names = config->readListEntry("/ImportFormatNames");
	headers = config->readListEntry("/ImportFormatHeaders");
	tracks = config->readListEntry("/ImportFormatTracks");
	importFormatIdx = config->readNumEntry("/ImportFormatIdx", importFormatIdx);
	enableTimeDifferenceCheck = config->readBoolEntry("/EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	maxTimeDifference = config->readNumEntry("/MaxTimeDifference", maxTimeDifference);
	config->endGroup();
#endif
	/* Use defaults if no configuration found */
#if QT_VERSION >= 300
	if (!names.empty())   importFormatNames = names;
	if (!headers.empty()) importFormatHeaders = headers;
	if (!tracks.empty())  importFormatTracks = tracks;
#else
	if (!names.isEmpty())   importFormatNames = names;
	if (!headers.isEmpty()) importFormatHeaders = headers;
	if (!tracks.isEmpty())  importFormatTracks = tracks;
#endif
	/* Make sure that there are as many formats as names */
	int i, appendCnt = importFormatNames.count() - importFormatHeaders.count();
	for (i = 0; i < appendCnt; i++) {
		importFormatHeaders.append("");
	}
	appendCnt = importFormatNames.count() - importFormatTracks.count();
	for (i = 0; i < appendCnt; i++) {
		importFormatTracks.append("");
	}
}
