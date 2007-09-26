/**
 * \file importsourceclient.h
 * Client to connect to server with import data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#ifndef IMPORTSOURCECLIENT_H
#define IMPORTSOURCECLIENT_H

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include "qtcompatmac.h"

#if QT_VERSION >= 0x040000
#include <QTcpSocket>
#else
#include <qsocket.h>
#endif
class QStatusBar;
class ImportSourceConfig;

/**
 * Client to connect to server with import data.
 */
class ImportSourceClient : public QObject
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	ImportSourceClient();

	/**
	 * Destructor.
	 */
	virtual ~ImportSourceClient();

	/**
	 * Initialize object.
	 * Has to be called before use.
	 *
	 * @param sb status bar to display progress information.
	 */
	virtual void init(QStatusBar* sb);

	/**
	 * Construct a query command in m_request to search on the server.
	 * This method has to be reimplemented for the specific search command.
	 *
	 * @param cfg      import source configuration
	 * @param artist   artist to search
	 * @param album    album to search
	 * @param dest     the server to connect to is returned here
	 * @param destPort the port of the server is returned here
	 */
	virtual void constructFindQuery(
		const ImportSourceConfig* cfg,
		const QString& artist, const QString& album,
		QString& dest, int& destPort) = 0;

	/**
	 * Construct a query command in m_request to fetch the track list
	 * from the server.
	 * This method has to be reimplemented for the specific server.
	 *
	 * @param cfg      import source configuration
	 * @param cat      category
	 * @param id       ID
	 * @param dest     the server to connect to is returned here
	 * @param destPort the port of the server is returned here
	 */
	virtual void constructTrackListQuery(
		const ImportSourceConfig* cfg, const QString& cat, const QString& id,
		QString& dest, int& destPort) = 0;

	/**
	 * Find artist, album on server.
	 *
	 * @param cfg    import source configuration
	 * @param artist artist to search
	 * @param album  album to search
	 */
	void find(const ImportSourceConfig* cfg,
						const QString& artist, const QString& album);

	/**
	 * Request track list from server.
	 *
	 * @param cfg import source configuration
	 * @param cat category
	 * @param id  ID
	 */
	void getTrackList(const ImportSourceConfig* cfg, QString cat, QString id);

	/**
	 * Extract name and port from string.
	 *
	 * @param namePort input string with "name:port"
	 * @param name     output string with "name"
	 * @param port     output integer with port
	 */
	static void splitNamePort(const QString& namePort,
														QString& name, int& port);

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
#if QT_VERSION >= 0x040000
	void slotError(QAbstractSocket::SocketError err);
#else
	void slotError(int err);
#endif

signals:
	/**
	 * Emitted when find request finished.
	 * Parameter: text containing result of find request
	 */
	void findFinished(const QByteArray&);

	/**
	 * Emitted when album track data request finished.
	 * Parameter: text containing result of album request
	 */
	void albumFinished(const QByteArray&);

protected:
	/**
	 * Get string with proxy or destination and port.
	 * If a proxy is set, the proxy is returned, else the real destination.
	 *
	 * @param dst real destination
	 *
	 * @return "destinationname:port".
	 */
	static QString getProxyOrDest(const QString& dst);

	/** request to set */
	QString m_request;

private:
	/** status bar to display progress */
	QStatusBar* m_statusBar;
	/** client socket */
#if QT_VERSION >= 0x040000
	QTcpSocket* m_sock;
#else
	QSocket* m_sock;
#endif
	/** type of current request */
	enum RequestType {
		RT_None,
		RT_Find,
		RT_Album
	} m_requestType;
};

#endif
