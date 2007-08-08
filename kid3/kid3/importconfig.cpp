/**
 * \file importconfig.cpp
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include <qstring.h>
#include "qtcompatmac.h"
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
ImportConfig::ImportConfig(const QString& grp) :
	GeneralConfig(grp), m_importServer(ImportConfig::ServerFreedb),
	m_importDest(ImportConfig::DestV1), m_importFormatIdx(0),
	m_enableTimeDifferenceCheck(true), m_maxTimeDifference(3),
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
	m_importFormatNames.append("CSV unquoted");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("%t(\\d+)\\t%s([^\\r\\n\\t]*)\\t%a([^\\r\\n\\t]*)\\t%l([^\\r\\n\\t]*)\\t%y(\\d+)\\t%g([^\\r\\n\\t]*)\\t%c([^\\r\\n\\t]*)\\t(?:\\d+:)?%d(\\d+:\\d+)");

	m_importFormatNames.append("CSV quoted");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\"?%t(\\d+)\"?\\t\"?%s([^\\r\\n\\t\"]*)\"?\\t\"?%a([^\\r\\n\\t\"]*)\"?\\t\"?%l([^\\r\\n\\t\"]*)\"?\\t\"?%y(\\d+)\"?\\t\"?%g([^\\r\\n\\t\"]*)\"?\\t\"?%c([^\\r\\n\\t\"]*)\"?\\t\"?(?:\\d+:)?%d(\\d+:\\d+)");
//	m_importFormatTracks.append("\"%t(\\d+)\"\\t\"%s([^\\r\\n]*)\"\\t\"%a([^\\r\\n]*)\"\\t\"%l([^\\r\\n]*)\"\\t\"%y(\\d+)\"\\t\"%g([^\\r\\n]*)\"\\t\"%c([^\\r\\n]*)\"\\t\"%d(\\d+:\\d+)\\.00\"");

	m_importFormatNames.append("freedb HTML text");
	m_importFormatHeaders.append("%a(\\S[^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*\\S)[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+)?.*genre:\\s*%g(\\S[^\\r\\n]*\\S)?[\\r\\n]");
	m_importFormatTracks.append("[\\r\\n]%t(\\d+)[\\.\\s]+%d(\\d+:\\d+)\\s+%s(\\S[^\\r\\n]*\\S)");

	m_importFormatNames.append("freedb HTML source");
	m_importFormatHeaders.append("<[^>]+>%a([^<\\s][^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*[^\\s>])<[^>]+>[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+)?.*genre:\\s*%g(\\S[^\\r\\n>]*\\S)?<[^>]+>[\\r\\n]");
	m_importFormatTracks.append("<td[^>]*>\\s*%t(\\d+).</td><td[^>]*>\\s*%d(\\d+:\\d+)</td><td[^>]*>(?:<[^>]+>)?%s([^<\\r\\n]+)");

	m_importFormatNames.append("Title");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\\s*%s(\\S[^\\r\\n]*\\S)\\s*");

	m_importFormatNames.append("Track Title");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s*");

	m_importFormatNames.append("Track Title Time");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s+%d(\\d+:\\d+)\\s*");

	m_importFormatNames.append("Custom Format");
	m_importFormatHeaders.append("");
	m_importFormatTracks.append("");

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
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(m_group);
	config->writeEntry("ImportServer", m_importServer);
	config->writeEntry("ImportDestination", m_importDest);
	config->writeEntry("ImportFormatNames", m_importFormatNames);
	config->writeEntry("ImportFormatHeaders", m_importFormatHeaders);
	config->writeEntry("ImportFormatTracks", m_importFormatTracks);
	config->writeEntry("ImportFormatIdx", m_importFormatIdx);
	config->writeEntry("EnableTimeDifferenceCheck", m_enableTimeDifferenceCheck);
	config->writeEntry("MaxTimeDifference", m_maxTimeDifference);

	config->writeEntry("ExportSourceV1", m_exportSrcV1);
	config->writeEntry("ExportFormatNames", m_exportFormatNames);
	config->writeEntry("ExportFormatHeaders", m_exportFormatHeaders);
	config->writeEntry("ExportFormatTracks", m_exportFormatTracks);
	config->writeEntry("ExportFormatTrailers", m_exportFormatTrailers);
	config->writeEntry("ExportFormatIdx", m_exportFormatIdx);
	config->writeEntry("ExportWindowWidth", m_exportWindowWidth);
	config->writeEntry("ExportWindowHeight", m_exportWindowHeight);
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

	config->QCM_writeEntry("/ExportSourceV1", m_exportSrcV1);
	config->QCM_writeEntry("/ExportFormatNames", m_exportFormatNames);
	config->QCM_writeEntry("/ExportFormatHeaders", m_exportFormatHeaders);
	config->QCM_writeEntry("/ExportFormatTracks", m_exportFormatTracks);
	config->QCM_writeEntry("/ExportFormatTrailers", m_exportFormatTrailers);
	config->QCM_writeEntry("/ExportFormatIdx", m_exportFormatIdx);
	config->QCM_writeEntry("/ExportWindowWidth", m_exportWindowWidth);
	config->QCM_writeEntry("/ExportWindowHeight", m_exportWindowHeight);

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
	QStringList expNames, expHeaders, expTracks, expTrailers;
#ifdef CONFIG_USE_KDE
	config->setGroup(m_group);
	m_importServer = static_cast<ImportConfig::ImportServer>(
		config->readNumEntry("ImportServer", m_importServer));
	m_importDest = static_cast<ImportConfig::ImportDestination>(
		config->readNumEntry("ImportDestination", m_importDest));
	names = config->readListEntry("ImportFormatNames");
	headers = config->readListEntry("ImportFormatHeaders");
	tracks = config->readListEntry("ImportFormatTracks");
	m_importFormatIdx = config->readNumEntry("ImportFormatIdx", m_importFormatIdx);
	m_enableTimeDifferenceCheck = config->readBoolEntry("EnableTimeDifferenceCheck", m_enableTimeDifferenceCheck);
	m_maxTimeDifference = config->readNumEntry("MaxTimeDifference", m_maxTimeDifference);

	m_exportSrcV1 = config->readBoolEntry("ExportSourceV1", m_exportSrcV1);
	expNames = config->readListEntry("ExportFormatNames");
	expHeaders = config->readListEntry("ExportFormatHeaders");
	expTracks = config->readListEntry("ExportFormatTracks");
	expTrailers = config->readListEntry("ExportFormatTrailers");
	m_exportFormatIdx = config->readNumEntry("ExportFormatIdx", m_exportFormatIdx);
	m_exportWindowWidth = config->readNumEntry("ExportWindowWidth", -1);
	m_exportWindowHeight = config->readNumEntry("ExportWindowHeight", -1);

	// KConfig seems to strip empty entries from the end of the string lists,
	// so we have to append them again.
	unsigned numNames = names.size();
	while (headers.size() < numNames) headers.append("");
	while (tracks.size() < numNames) tracks.append("");
	unsigned numExpNames = expNames.size();
	while (expHeaders.size() < numExpNames) expHeaders.append("");
	while (expTracks.size() < numExpNames) expTracks.append("");
	while (expTrailers.size() < numExpNames) expTrailers.append("");
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

	m_exportSrcV1 = config->QCM_readBoolEntry("/ExportSourceV1", m_exportSrcV1);
	expNames = config->QCM_readListEntry("/ExportFormatNames");
	expHeaders = config->QCM_readListEntry("/ExportFormatHeaders");
	expTracks = config->QCM_readListEntry("/ExportFormatTracks");
	expTrailers = config->QCM_readListEntry("/ExportFormatTrailers");
	m_exportFormatIdx = config->QCM_readNumEntry("/ExportFormatIdx", m_exportFormatIdx);
	m_exportWindowWidth = config->QCM_readNumEntry("/ExportWindowWidth", -1);
	m_exportWindowHeight = config->QCM_readNumEntry("/ExportWindowHeight", -1);

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
		} else {
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
		} else {
			m_exportFormatNames.append(*expNamesIt);
			m_exportFormatHeaders.append(*expHeadersIt);
			m_exportFormatTracks.append(*expTracksIt);
			m_exportFormatTrailers.append(*expTrailersIt);
		}
	}

	if (m_importFormatIdx >= static_cast<int>(m_importFormatNames.size()))
		m_importFormatIdx = 0;
	if (m_exportFormatIdx >=  static_cast<int>(m_exportFormatNames.size()))
		m_exportFormatIdx = 0;
}
