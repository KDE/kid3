/**
 * \file freedbclient.cpp
 * freedb.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 */

#include "freedbclient.h"
#include "importsourceconfig.h"
#include <qregexp.h>
#include <qurl.h>

static const char gnudbServer[] = "www.gnudb.org:80";

/**
 * Constructor.
 */
FreedbClient::FreedbClient()
{
}

/**
 * Destructor.
 */
FreedbClient::~FreedbClient()
{
}

/**
 * Construct a query command in m_request to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 * @param dest     the server to connect to is returned here
 * @param destPort the port of the server is returned here
 */
void FreedbClient::constructFindQuery(
	const ImportSourceConfig*,
	const QString& artist, const QString& album,
	QString& dest, int& destPort)
{
	// At the moment, only www.gnudb.org has a working search
	// so we always use this server for find queries.
	QString server(gnudbServer);
	QString what = artist + " " + album;
	QString destNamePort(getProxyOrDest(server));
	splitNamePort(destNamePort, dest, destPort);
	QString serverName;
	int serverPort;
	splitNamePort(server, serverName, serverPort);
	what.replace(QRegExp(" +"), " "); // collapse spaces
	QCM_QUrl_encode(what);
	what.replace("%20", "+"); // replace spaces by '+'
	m_request = "GET ";
	if (dest != serverName) {
		m_request += "http://";
		m_request += serverName;
		if (serverPort != 80) {
			m_request += ':';
			m_request += QString::number(serverPort);
		}
	}
	m_request += "/search/";
	m_request += what;
	m_request += " HTTP/1.0\r\nUser-Agent: Kid3/" VERSION "\r\nHost: ";
	m_request += serverName;
	m_request += "\r\nConnection: close\r\n\r\n";
}

/**
 * Construct a query command in m_request to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 * @param dest     the server to connect to is returned here
 * @param destPort the port of the server is returned here
 */
void FreedbClient::constructTrackListQuery(
	const ImportSourceConfig* cfg, const QString& cat, const QString& id,
	QString& dest, int& destPort)
{
	QString destNamePort(getProxyOrDest(cfg->m_server));
	splitNamePort(destNamePort, dest, destPort);
	QString serverName;
	int serverPort;
	splitNamePort(cfg->m_server, serverName, serverPort);
	m_request = "GET ";
	if (dest != serverName) {
		m_request += "http://";
		m_request += serverName;
		if (serverPort != 80) {
			m_request += ':';
			m_request += QString::number(serverPort);
		}
	}
	m_request += cfg->m_cgiPath;
	m_request += "?cmd=cddb+read+";
	m_request += cat;
	m_request += "+";
	m_request += id;
	m_request += "&hello=noname+localhost+";
	m_request += "Kid3+" VERSION "&proto=1 HTTP/1.1\r\nHost: ";
	m_request += serverName;
	m_request += "\r\nConnection: close\r\n\r\n";
}
