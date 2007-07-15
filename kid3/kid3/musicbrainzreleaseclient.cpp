/**
 * \file musicbrainzreleaseclient.cpp
 * MusicBrainz release database client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 */

#include "musicbrainzreleaseclient.h"
#include "importsourceconfig.h"
#include <qregexp.h>
#include <qurl.h>

/**
 * Constructor.
 */
MusicBrainzReleaseClient::MusicBrainzReleaseClient()
{
}

/**
 * Destructor.
 */
MusicBrainzReleaseClient::~MusicBrainzReleaseClient()
{
}

/**
 * Construct a query command in m_request to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 * @param dest     the server to connect to is returned hera
 * @param destPort the port of the server is returned here
 */
void MusicBrainzReleaseClient::constructFindQuery(
	const ImportSourceConfig* cfg,
	const QString& artist, const QString& album,
	QString& dest, int& destPort)
{
	/*
	 * Query looks like this:
	 * http://musicbrainz.org/ws/1/release/?type=xml&artist=wizard&title=odin
	 */
	QString destNamePort(getProxyOrDest(cfg->m_server));
	splitNamePort(destNamePort, dest, destPort);
	QString serverName;
	int serverPort;
	splitNamePort(cfg->m_server, serverName, serverPort);
	QString encArtist(artist);
	encArtist.replace(QRegExp(" +"), " "); // collapse spaces
	QCM_QUrl_encode(encArtist);
	encArtist.replace("%20", "+"); // replace spaces by '+'
	QString encAlbum(album);
	encAlbum.replace(QRegExp(" +"), " "); // collapse spaces
	QCM_QUrl_encode(encAlbum);
	encAlbum.replace("%20", "+"); // replace spaces by '+'
	m_request = "GET ";
	if (dest != serverName) {
		m_request += "http://";
		m_request += serverName;
		if (serverPort != 80) {
			m_request += ':';
			m_request += QString::number(serverPort);
		}
	}
	m_request += "/ws/1/release/?type=xml&artist=";
	m_request += encArtist;
	m_request += "&title=";
	m_request += encAlbum;
	m_request += " HTTP/1.1\r\nUser-Agent: Kid3/" VERSION "\r\nHost: ";
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
 * @param dest     the server to connect to is returned hera
 * @param destPort the port of the server is returned here
 */
void MusicBrainzReleaseClient::constructTrackListQuery(
	const ImportSourceConfig* cfg, const QString& cat, const QString& id,
	QString& dest, int& destPort)
{
	/*
	 * Query looks like this:
	 * http://musicbrainz.org/ws/1/release/978c7ed1-a854-4ef2-bd4e-e7c1317be854/?type=xml&inc=artist+tracks
	 */
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
	m_request += "/ws/1/";
	m_request += cat;
	m_request += '/';
	m_request += id;
	m_request += "/?type=xml&inc=artist+tracks HTTP/1.1\r\nUser-Agent: Kid3/" VERSION "\r\nHost: ";
	m_request += serverName;
	m_request += "\r\nConnection: close\r\n\r\n";
}
