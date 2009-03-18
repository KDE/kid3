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
#include <QHttp>
#include <QByteArray>
#else
#include <qsocket.h>
#include <qcstring.h>
#endif

class QHttpResponseHeader;

/**
 * Client to connect to HTTP server.
 */
class HttpClient : public QObject
{
Q_OBJECT

public:
	/** Connection progress steps. */
	enum ConnectionSteps {
		CS_RequestConnection = 0, /**< Send Request */
		CS_Connecting        = 1, /**< Connecting */
		CS_HostFound         = 2, /**< Host Found */
		CS_RequestSent       = 3, /**< Request Sent */
		CS_EstimatedBytes = 75000 /**< Estimated total number of bytes */
	};

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
	void slotError(int err);


	/**
	 * Called when the connection state changes.
	 *
	 * @param state HTTP connection state
	 */
	void slotStateChanged(int state);

	/**
	 * Called to report connection progress.
	 *
	 * @param done  bytes received
	 * @param total total bytes, 0 if unknown
	 */
	void slotDataReadProgress(int done, int total);

	/**
	 * Called when the request is finished.
	 *
	 * @param error true if error occurred
	 */
	void slotDone(bool error);

	/**
	 * Called when the response header is available.
	 *
	 * @param resp HTTP response header
	 */
	void slotResponseHeaderReceived(const QHttpResponseHeader& resp);

private:
	/**
	 * Emit a progress signal with step/total steps.
	 *
	 * @param text       state text
	 * @param step       current step
	 * @param totalSteps total number of steps
	 */
	void emitProgress(const QString& text, int step, int totalSteps);

	/**
	 * Emit a progress signal with bytes received/total bytes.
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

#if QT_VERSION >= 0x040000
	/** client socket */
	QHttp* m_http;
#else
	/** client socket */
	QSocket* m_sock;
	/** current index in receive buffer */
	unsigned long m_rcvIdx;
	/** index of entity-body in receive buffer, 0 if not available */
	unsigned long m_rcvBodyIdx;
	/** receive buffer */
	QByteArray m_rcvBuf;
	/** request to set */
	QString m_request;
#endif
	/** content length of entitiy-body, 0 if not available */
	unsigned long m_rcvBodyLen;
	/** content type */
	QString m_rcvBodyType;
};

#endif
