/**
 * \file musicbrainzclient.h
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include "config.h"

#include <qobject.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QByteArray>
#include <QTcpSocket>
#else
#include <qcstring.h>
#include <qsocket.h>
#endif

#ifdef HAVE_TUNEPIMP
#if HAVE_TUNEPIMP >= 5
#include <qbuffer.h>
#include <tunepimp-0.5/tp_c.h>
#else
#include <tunepimp/tp_c.h>
#endif
#endif // HAVE_TUNEPIMP

class ImportTrackData;
class ImportTrackDataVector;

/**
 * A HTTP query to a musicbrainz server for HAVE_TUNEPIMP >= 5.
 */
class LookupQuery : public QObject {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param numFiles   number of files to be queried
	 * @param serverName server name
	 * @param serverPort server port
	 * @param proxyName  proxy name, empty if no proxy
	 * @param proxyPort  proxy port
	 */
	LookupQuery(int numFiles,
							const QString& serverName, unsigned short serverPort = 80,
							const QString& proxyName = "", unsigned short proxyPort = 80);

	/**
	 * Destructor.
	 */
	virtual ~LookupQuery();

#if defined HAVE_TUNEPIMP && HAVE_TUNEPIMP >= 5
	/**
	 * Query a PUID from the server.
	 *
	 * @param puid     PUID
	 * @param index    index of file
	 */
	void query(const char* puid, int index);
#endif

signals:
	/**
	 * Emitted when the query response is received
	 */
	void queryResponseReceived(int, const QByteArray&);

private slots:
	/**
	 * Send query when the socket is connected.
	 */
	void socketConnected();

	/**
	 * Error on socket connection.
	 */
#if QT_VERSION >= 0x040000
	void socketError(QAbstractSocket::SocketError err);
#else
	void socketError(int);
#endif
	/**
	 * Read received data when the server has closed the connection.
	 */
	void socketConnectionClosed();

#if defined HAVE_TUNEPIMP && HAVE_TUNEPIMP >= 5
private:
	/**
	 * Connect to server to query information about the current file.
	 */
	void socketQuery();

	/**
	 * Query the next file.
	 */
	void queryNext();

	struct FileQuery {
		bool requested;
		QString puid;
	};

	/** Number of files to be queried. */
	int m_numFiles;
	/** MusicBrainz server */
	QString m_serverName;
	/** Port of MusicBrainz server */
	unsigned short m_serverPort;
	/** Proxy */
	QString m_proxyName;
	/** Port of proxy */
	unsigned short m_proxyPort;
	/**
	 * -1 if not yet started,
	 * 0..m_numFiles-1 if a file is currently processed,
	 * >=m_numFiles if all files processed.
	 */ 
	int m_currentFile;
	FileQuery* m_fileQueries;
#if QT_VERSION >= 0x040000
	QTcpSocket* m_sock;
#else
	QSocket* m_sock;
#endif
	QString m_request;
#endif
};


/**
 * MusicBrainz client.
 */
class MusicBrainzClient : public QObject
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param trackDataList track data to be filled with imported values,
	 *                      is passed with filenames set
	 */
	explicit MusicBrainzClient(ImportTrackDataVector& trackDataList);
	/**
	 * Destructor.
	 */
	virtual ~MusicBrainzClient();

#ifdef HAVE_TUNEPIMP
	/**
	 * Poll the status of the MusicBrainz query.
	 */
	void pollStatus();

	/**
	 * Set configuration.
	 *
	 * @param server   server
	 * @param proxy    proxy
	 * @param useProxy true if proxy has to be used
	 */
	void setConfig(const QString& server, const QString& proxy,
								 bool useProxy);

	/**
	 * Add the files in the file list.
	 */
	void addFiles();

	/**
	 * Remove all files.
	 */
	void removeFiles();
#endif // HAVE_TUNEPIMP

signals:
	/**
	 * Emitted when status of a file changed.
	 * Parameter: index of file, status text
	 */
	void statusChanged(int, QString);

	/**
	 * Emitted when meta data for a recognized file are received.
	 * Parameter index of file, track data
	 */
	void metaDataReceived(int, ImportTrackData&);

	/**
	 * Emitted when results for an ambiguous file are received.
	 * Parameter index of file, track data list
	 */
	void resultsReceived(int, ImportTrackDataVector&);

private slots:
	/**
	 * Process server response with lookup data.
	 *
	 * @param index    index of file
	 * @param response response from server
	 */
	void parseLookupResponse(int index, const QByteArray& response);

#ifdef HAVE_TUNEPIMP
private:
	/**
	 * Get i for m_id[i] == id.
	 *
	 * @return index, -1 if not found.
	 */
	int getIndexOfId(int id) const;

	/**
	 * Get the file name for an ID.
	 *
	 * @param id ID of file
	 *
	 * @return absolute file name, QString::null if not found.
	 */
	QString getFilename(int id) const;

	/**
	 * Get meta data for recognized file.
	 *
	 * @param id        ID of file
	 * @param trackData the meta data is returned here
	 */
	void getMetaData(int id, ImportTrackData& trackData);

	/**
	 * Get results for an ambiguous file.
	 *
	 * @param id            ID of file
	 * @param trackDataList the results are returned here
	 *
	 * @return true if some results were received,
	 *         false if no results available.
	 */
	bool getResults(int id, ImportTrackDataVector& trackDataList);

	ImportTrackDataVector& m_trackDataVector;
	tunepimp_t m_tp;
	int* m_ids;
	int m_numFiles;
#if HAVE_TUNEPIMP >= 5
	LookupQuery* m_lookupQuery;
#endif
#endif // HAVE_TUNEPIMP
};

#endif // MUSICBRAINZCLIENT_H
