/**
 * \file httpclient.h
 * Client to connect to HTTP server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Dec 2008
 *
 * Copyright (C) 2008-2009  Urs Fleisch
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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <qobject.h>
#include <qstring.h>
#include "qtcompatmac.h"

#if QT_VERSION >= 0x040000
#include <QTcpSocket>
#include <QByteArray>
#else
#include <qsocket.h>
#include <qcstring.h>
#endif

/**
 * Client to connect to HTTP server.
 */
class HttpClient : public QObject
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	HttpClient();

	/**
	 * Destructor.
	 */
	virtual ~HttpClient();

	/**
	 * Send a HTTP GET request.
	 *
	 * @param server host name
	 * @param path   path of the URL
	 */
	void sendRequest(const QString& server, const QString& path);

	/**
	 * Get content length.
	 * @return size of body in bytes, 0 if unknown.
	 */
	unsigned long getContentLength() const { return m_rcvBodyLen; }

	/**
	 * Get content type.
	 * @return MIME type, empty if unknown.
	 */
	QString getContentType() const { return m_rcvBodyType; }

	/**
	 * Extract name and port from string.
	 *
	 * @param namePort input string with "name:port"
	 * @param name     output string with "name"
	 * @param port     output integer with port
	 */
	static void splitNamePort(const QString& namePort,
	                          QString& name, int& port);

signals:
	/**
	 * Emitted to report progress.
	 * Parameter: state text, bytes received, total bytes.
	 */
	void progress(const QString&, int, int);

	/**
	 * Emitted when response received.
	 * Parameter: bytes containing result of request
	 */
	void bytesReceived(const QByteArray&);

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
	 * A bytesReceived() signal is emitted.
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

private:
	/**
	 * Emit a progress signal.
	 *
	 * @param text state text
	 */
	void emitProgress(const QString& text);

	/**
	 * Read the available bytes.
	 */
	void readBytesAvailable();

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
	/** client socket */
#if QT_VERSION >= 0x040000
	QTcpSocket* m_sock;
#else
	QSocket* m_sock;
#endif
	/** current index in receive buffer */
	unsigned long m_rcvIdx;
	/** index of entity-body in receive buffer, 0 if not available */
	unsigned long m_rcvBodyIdx;
	/** content length of entitiy-body, 0 if not available */
	unsigned long m_rcvBodyLen;
	/** content type */
	QString m_rcvBodyType;
	/** receive buffer */
	QByteArray m_rcvBuf;
};

#endif
