/**
 * \file tracktypeclient.cpp
 * TrackType.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Apr 2007
 */

#include "tracktypeclient.h"
#include "importsourceconfig.h"
#include <qregexp.h>
#include <qurl.h>

static const char trackTypeServer[] = "tracktype.org:80";

/**
 * Constructor.
 */
TrackTypeClient::TrackTypeClient()
{
}

/**
 * Destructor.
 */
TrackTypeClient::~TrackTypeClient()
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
void TrackTypeClient::constructFindQuery(
	const ImportSourceConfig* cfg,
	const QString& artist, const QString& album,
	QString& dest, int& destPort)
{
	// At the moment, only TrackType.org recognizes cddb album commands,
	// so we always use this server for find queries.
	QString server(trackTypeServer);
	QString what = artist + " / " + album;
	QString destNamePort(getProxyOrDest(server));
	splitNamePort(destNamePort, dest, destPort);
	QString serverName;
	int serverPort;
	splitNamePort(server, serverName, serverPort);
	what.replace(QRegExp(" +"), " "); // collapse spaces
	QUrl::encode(what);
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
	m_request += cfg->m_cgiPath;
	m_request += "?cmd=cddb+album+";
	m_request += what;
	m_request += "&hello=noname+localhost+";
	m_request += "Kid3+" VERSION "&proto=1 HTTP/1.1\r\nHost: ";
	m_request += serverName;
	m_request += "\r\nConnection: close\r\n\r\n";
}
