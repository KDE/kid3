/**
 * \file discogsclient.cpp
 * Discogs client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 */

#include "discogsclient.h"
#include "importsourceconfig.h"
#include <qregexp.h>
#include <qurl.h>

static const char discogsServer[] = "www.discogs.com:80";

/**
 * Constructor.
 */
DiscogsClient::DiscogsClient()
{
}

/**
 * Destructor.
 */
DiscogsClient::~DiscogsClient()
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
void DiscogsClient::constructFindQuery(
	const ImportSourceConfig*,
	const QString& artist, const QString& album,
	QString& dest, int& destPort)
{
	/*
	 * Query looks like this:
	 * http://www.discogs.com/search?type=releases&q=amon+amarth+avenger&btn=Search
	 */
	QString server(discogsServer);
	QString what = artist + " " + album;
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
	m_request += "/search?type=releases&q=";
	m_request += what;
	m_request += "&btn=Search HTTP/1.0\r\nUser-Agent: Kid3/" VERSION "\r\nHost: ";
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
void DiscogsClient::constructTrackListQuery(
	const ImportSourceConfig*, const QString& cat, const QString& id,
	QString& dest, int& destPort)
{
	/*
	 * Query looks like this:
	 * http://www.discogs.com/release/761529
	 */
	QString server(discogsServer);
	QString destNamePort(getProxyOrDest(server));
	splitNamePort(destNamePort, dest, destPort);
	QString serverName;
	int serverPort;
	splitNamePort(server, serverName, serverPort);
	m_request = "GET ";
	if (dest != serverName) {
		m_request += "http://";
		m_request += serverName;
		if (serverPort != 80) {
			m_request += ':';
			m_request += QString::number(serverPort);
		}
	}
	m_request += '/';
	m_request += cat;
	m_request += '/';
	m_request += id;
	m_request += " HTTP/1.0\r\nUser-Agent: Kid3/" VERSION "\r\nHost: ";
	m_request += serverName;
	m_request += "\r\nConnection: close\r\n\r\n";
}
