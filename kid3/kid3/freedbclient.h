/**
 * \file freedbclient.h
 * freedb.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 */

#ifndef FREEDBCLIENT_H
#define FREEDBCLIENT_H

#include "config.h"
#include <qobject.h>
#include <qstring.h>

class QStatusBar;
class QSocket;
class FreedbConfig;

/**
 * freedb.org client.
 */
class FreedbClient : public QObject
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param sb status bar to display progress information.
	 */
	FreedbClient(QStatusBar *sb);
	/**
	 * Destructor.
	 */
	virtual ~FreedbClient();
	/**
	 * Find keyword in freedb.
	 *
	 * @param cfg  freedb configuration
	 * @param what string with words to search
	 */
	void find(const FreedbConfig *cfg, QString what);
	/**
	 * Find keyword in freedb with "cddb album" command of freedb2.org.
	 *
	 * @param cfg  freedb configuration
	 * @param what string with words to search
	 */
	void findCddbAlbum(const FreedbConfig *cfg, QString what);
	/**
	 * Request track list from freedb server.
	 *
	 * @param cfg freedb configuration
	 * @param cat category
	 * @param id  ID
	 */
	void getTrackList(const FreedbConfig *cfg, QString cat, QString id);

	/**
	 * Extract name and port from string.
	 *
	 * @param namePort input string with "name:port"
	 * @param name     output string with "name"
	 * @param port     output integer with port
	 */
	static void splitNamePort(const QString &namePort,
														QString &name, int &port);

private slots:
	/**
	 * Display status if host is found.
	 */
	void slotHostFound();
	/**
	 * Display status if connection is established.
	 */
	void slotConnected();
	/**
	 * Read received data when the server has closed the connection.
	 * The data is sent to other objects via signals.
	 */
	void slotConnectionClosed();
	/**
	 * Display information about read progress.
	 */
	void slotReadyRead();
	/**
	 * Display information about socket error.
	 */
	void slotError(int err);

signals:
	/**
	 * Emitted when find request finished.
	 * Parameter: text containing result of find request
	 */
	void findFinished(QString);
	/**
	 * Emitted when findCddbAlbum request finished.
	 * Parameter: text containing result of findCddbAlbum request
	 */
	void findCddbAlbumFinished(QString);
	/**
	 * Emitted when album track data request finished.
	 * Parameter: text containing result of album request
	 */
	void albumFinished(QString);
private:
	/** status bar to display progress */
	QStatusBar *statusBar;
	/** client socket */
	QSocket *sock;
	/** request to set */
	QString request;
	/** buffer for received data */
	QString rcvStr;
	/** type of current request */
	enum RequestType {
		RT_None,
		RT_FindFreedbSearch,
		RT_FindCddbAlbum,
		RT_Album
	} m_requestType;
};

#endif
