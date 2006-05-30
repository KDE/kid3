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
	GeneralConfig(grp), importDestV1(true), importFormatIdx(0),
	enableTimeDifferenceCheck(true), maxTimeDifference(3),
	m_exportSrcV1(true), m_exportFormatIdx(0),
	m_exportWindowWidth(-1), m_exportWindowHeight(-1)
{
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
	importFormatNames.append("CSV unquoted");
	importFormatHeaders.append("");
	importFormatTracks.append("%t(\\d+)\\t%s([^\\r\\n\\t]*)\\t%a([^\\r\\n\\t]*)\\t%l([^\\r\\n\\t]*)\\t%y(\\d+)\\t%g([^\\r\\n\\t]*)\\t%c([^\\r\\n\\t]*)\\t(?:\\d+:)?%d(\\d+:\\d+)");

	importFormatNames.append("CSV quoted");
	importFormatHeaders.append("");
	importFormatTracks.append("\"?%t(\\d+)\"?\\t\"?%s([^\\r\\n\\t\"]*)\"?\\t\"?%a([^\\r\\n\\t\"]*)\"?\\t\"?%l([^\\r\\n\\t\"]*)\"?\\t\"?%y(\\d+)\"?\\t\"?%g([^\\r\\n\\t\"]*)\"?\\t\"?%c([^\\r\\n\\t\"]*)\"?\\t\"?(?:\\d+:)?%d(\\d+:\\d+)");
//	importFormatTracks.append("\"%t(\\d+)\"\\t\"%s([^\\r\\n]*)\"\\t\"%a([^\\r\\n]*)\"\\t\"%l([^\\r\\n]*)\"\\t\"%y(\\d+)\"\\t\"%g([^\\r\\n]*)\"\\t\"%c([^\\r\\n]*)\"\\t\"%d(\\d+:\\d+)\\.00\"");

	importFormatNames.append("freedb HTML text");
	importFormatHeaders.append("%a(\\S[^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*\\S)[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+)?.*genre:\\s*%g(\\S[^\\r\\n]*\\S)?[\\r\\n]");
	importFormatTracks.append("[\\r\\n]%t(\\d+)[\\.\\s]+%d(\\d+:\\d+)\\s+%s(\\S[^\\r\\n]*\\S)");

	importFormatNames.append("freedb HTML source");
	importFormatHeaders.append("<[^>]+>%a([^<\\s][^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*[^\\s>])<[^>]+>[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+)?.*genre:\\s*%g(\\S[^\\r\\n>]*\\S)?<[^>]+>[\\r\\n]");
	importFormatTracks.append("<td[^>]*>\\s*%t(\\d+).</td><td[^>]*>\\s*%d(\\d+:\\d+)</td><td[^>]*>(?:<[^>]+>)?%s([^<\\r\\n]+)");

	importFormatNames.append("Title");
	importFormatHeaders.append("");
	importFormatTracks.append("\\s*%s(\\S[^\\r\\n]*\\S)\\s*");

	importFormatNames.append("Track Title");
	importFormatHeaders.append("");
	importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s*");

	importFormatNames.append("Track Title Time");
	importFormatHeaders.append("");
	importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s+%d(\\d+:\\d+)\\s*");

	importFormatNames.append("Custom Format");
	importFormatHeaders.append("");
	importFormatTracks.append("");

	m_exportFormatNames.append("CSV unquoted");
	m_exportFormatHeaders.append("");
	m_exportFormatTracks.append("%t\\t%s\\t%a\\t%l\\t%y\\t%g\\t%c\\t%d.00");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("CSV quoted");
	m_exportFormatHeaders.append("");
	m_exportFormatTracks.append("\"%t\"\\t\"%s\"\\t\"%a\"\\t\"%l\"\\t\"%y\"\\t\"%g\"\\t\"%c\"\\t\"%d.00\"");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("Extended M3U");
	m_exportFormatHeaders.append("#EXTM3U");
	m_exportFormatTracks.append("#EXTINF:%D,%a - %s\\n%p");
	m_exportFormatTrailers.append("");

	m_exportFormatNames.append("Extended PLS");
	m_exportFormatHeaders.append("[playlist]");
	m_exportFormatTracks.append("File%T=%p\\nTitle%T=%a - %s\\nLength%T=%D");
	m_exportFormatTrailers.append("NumberOfEntries=%n\\nVersion=2");

	m_exportFormatNames.append("HTML");
	m_exportFormatHeaders.append("<html>\\n <head>\\n  <title>%a - %l</title>\\n </head>\\n <body>\\n  <h1>%a - %l</h1>\\n  <dl>\\n");
	m_exportFormatTracks.append("   <dt><a href=\"%u\">%t. %s</a></dt>");
	m_exportFormatTrailers.append("  </dl>\\n </body>\\n</html>");

	m_exportFormatNames.append("Kover XML");
	m_exportFormatHeaders.append("<kover>\\n <title>\\n  <text><![CDATA[%a ]]></text>\\n  <text><![CDATA[%l]]></text>\\n </title>\\n <content>");
	m_exportFormatTracks.append("  <text><![CDATA[%t. %s]]></text>");
	m_exportFormatTrailers.append(" </content>\\n</kover>");

	m_exportFormatNames.append("Custom Format");
	m_exportFormatHeaders.append("");
	m_exportFormatTracks.append("");
	m_exportFormatTrailers.append("");
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

	config->writeEntry("ExportSourceV1", m_exportSrcV1);
	config->writeEntry("ExportFormatNames", m_exportFormatNames);
	config->writeEntry("ExportFormatHeaders", m_exportFormatHeaders);
	config->writeEntry("ExportFormatTracks", m_exportFormatTracks);
	config->writeEntry("ExportFormatTrailers", m_exportFormatTrailers);
	config->writeEntry("ExportFormatIdx", m_exportFormatIdx);
	config->writeEntry("ExportWindowWidth", m_exportWindowWidth);
	config->writeEntry("ExportWindowHeight", m_exportWindowHeight);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/ImportDestinationV1", importDestV1);
	config->writeEntry("/ImportFormatNames", importFormatNames);
	config->writeEntry("/ImportFormatHeaders", importFormatHeaders);
	config->writeEntry("/ImportFormatTracks", importFormatTracks);
	config->writeEntry("/ImportFormatIdx", importFormatIdx);
	config->writeEntry("/EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	config->writeEntry("/MaxTimeDifference", maxTimeDifference);

	config->writeEntry("/ExportSourceV1", m_exportSrcV1);
	config->writeEntry("/ExportFormatNames", m_exportFormatNames);
	config->writeEntry("/ExportFormatHeaders", m_exportFormatHeaders);
	config->writeEntry("/ExportFormatTracks", m_exportFormatTracks);
	config->writeEntry("/ExportFormatTrailers", m_exportFormatTrailers);
	config->writeEntry("/ExportFormatIdx", m_exportFormatIdx);
	config->writeEntry("/ExportWindowWidth", m_exportWindowWidth);
	config->writeEntry("/ExportWindowHeight", m_exportWindowHeight);

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
	QStringList expNames, expHeaders, expTracks, expTrailers;
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	importDestV1 = config->readBoolEntry("ImportDestinationV1", importDestV1);
	names = config->readListEntry("ImportFormatNames");
	headers = config->readListEntry("ImportFormatHeaders");
	tracks = config->readListEntry("ImportFormatTracks");
	importFormatIdx = config->readNumEntry("ImportFormatIdx", importFormatIdx);
	enableTimeDifferenceCheck = config->readBoolEntry("EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	maxTimeDifference = config->readNumEntry("MaxTimeDifference", maxTimeDifference);

	m_exportSrcV1 = config->readBoolEntry("ExportSourceV1", m_exportSrcV1);
	expNames = config->readListEntry("ExportFormatNames");
	expHeaders = config->readListEntry("ExportFormatHeaders");
	expTracks = config->readListEntry("ExportFormatTracks");
	expTrailers = config->readListEntry("ExportFormatTrailers");
	m_exportFormatIdx = config->readNumEntry("ExportFormatIdx", m_exportFormatIdx);
	m_exportWindowWidth = config->readNumEntry("ExportWindowWidth", -1);
	m_exportWindowHeight = config->readNumEntry("ExportWindowHeight", -1);
#else
	config->beginGroup("/" + group);
	importDestV1 = config->readBoolEntry("/ImportDestinationV1", importDestV1);
	names = config->readListEntry("/ImportFormatNames");
	headers = config->readListEntry("/ImportFormatHeaders");
	tracks = config->readListEntry("/ImportFormatTracks");
	importFormatIdx = config->readNumEntry("/ImportFormatIdx", importFormatIdx);
	enableTimeDifferenceCheck = config->readBoolEntry("/EnableTimeDifferenceCheck", enableTimeDifferenceCheck);
	maxTimeDifference = config->readNumEntry("/MaxTimeDifference", maxTimeDifference);

	m_exportSrcV1 = config->readBoolEntry("/ExportSourceV1", m_exportSrcV1);
	expNames = config->readListEntry("/ExportFormatNames");
	expHeaders = config->readListEntry("/ExportFormatHeaders");
	expTracks = config->readListEntry("/ExportFormatTracks");
	expTrailers = config->readListEntry("/ExportFormatTrailers");
	m_exportFormatIdx = config->readNumEntry("/ExportFormatIdx", m_exportFormatIdx);
	m_exportWindowWidth = config->readNumEntry("/ExportWindowWidth", -1);
	m_exportWindowHeight = config->readNumEntry("/ExportWindowHeight", -1);

	config->endGroup();
#endif
	/* Use defaults if no configuration found */
	QStringList::const_iterator namesIt, headersIt, tracksIt;
	for (namesIt = names.begin(), headersIt = headers.begin(),
				 tracksIt = tracks.begin();
			 namesIt != names.end() && headersIt != headers.end() &&
				 tracksIt != tracks.end();
			 ++namesIt, ++headersIt, ++tracksIt) {
		int idx = importFormatNames.findIndex(*namesIt);
		if (idx >= 0) {
			importFormatHeaders[idx] = *headersIt;
			importFormatTracks[idx] = *tracksIt;
		} else {
			importFormatNames.append(*namesIt);
			importFormatHeaders.append(*headersIt);
			importFormatTracks.append(*tracksIt);
		}
	}

	QStringList::const_iterator expNamesIt, expHeadersIt, expTracksIt,
		expTrailersIt;
	for (expNamesIt = expNames.begin(), expHeadersIt = expHeaders.begin(),
				 expTracksIt = expTracks.begin(), expTrailersIt = expTrailers.begin();
			 expNamesIt != expNames.end() && expHeadersIt != expHeaders.end() &&
				 expTracksIt != expTracks.end() && expTrailersIt != expTrailers.end();
			 ++expNamesIt, ++expHeadersIt, ++expTracksIt, ++expTrailersIt) {
		int idx = m_exportFormatNames.findIndex(*expNamesIt);
		if (idx >= 0) {
			m_exportFormatHeaders[idx] = *expHeadersIt;
			m_exportFormatTracks[idx] = *expTracksIt;
			m_exportFormatTrailers[idx] = *expTrailersIt;
		} else {
			m_exportFormatNames.append(*expNamesIt);
			m_exportFormatHeaders.append(*expHeadersIt);
			m_exportFormatTracks.append(*expTracksIt);
			m_exportFormatTrailers.append(*expTrailersIt);
		}
	}
}
