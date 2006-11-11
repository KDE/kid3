/**
 * \file musicbrainzclient.h
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 */

#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include "config.h"

#include <qobject.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <qbytearray.h>
#else
#include <qcstring.h>
#endif

#ifdef HAVE_TUNEPIMP
#if HAVE_TUNEPIMP >= 5
#include <qbuffer.h>
#include <tunepimp-0.5/tp_c.h>
class Q3Socket;
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
							const QString& serverName, Q_UINT16 serverPort = 80,
							const QString& proxyName = "", Q_UINT16 proxyPort = 80);

	/**
	 * Destructor.
	 */
	virtual ~LookupQuery();

#if HAVE_TUNEPIMP >= 5
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
	void socketError();

	/**
	 * Read received data when the server has closed the connection.
	 */
	void socketConnectionClosed();

#if HAVE_TUNEPIMP >= 5
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
	Q_UINT16 m_serverPort;
	/** Proxy */
	QString m_proxyName;
	/** Port of proxy */
	Q_UINT16 m_proxyPort;
	/**
	 * -1 if not yet started,
	 * 0..m_numFiles-1 if a file is currently processed,
	 * >=m_numFiles if all files processed.
	 */ 
	int m_currentFile;
	FileQuery* m_fileQueries;
	Q3Socket* m_sock;
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
