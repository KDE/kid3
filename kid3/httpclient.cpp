/**
 * \file httpclient.cpp
 * Client to connect to HTTP server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Dec 2008
 *
 * Copyright (C) 2008-2011  Urs Fleisch
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

#include "httpclient.h"
#include <QHttp>
#include <QByteArray>
#include "configstore.h"
#include "qtcompatmac.h"


/**
 * Constructor.
 *
 * @param parent  parent object
 */
HttpClient::HttpClient(QObject* parent) : QObject(parent), m_rcvBodyLen(0)
{
	setObjectName("HttpClient");
	m_http = new QHttp();
	connect(m_http, SIGNAL(stateChanged(int)),
					this, SLOT(slotStateChanged(int)));
	connect(m_http, SIGNAL(dataReadProgress(int, int)),
					this, SLOT(slotDataReadProgress(int, int)));
	connect(m_http, SIGNAL(done(bool)),
					this, SLOT(slotDone(bool)));
	connect(m_http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)),
					this, SLOT(slotResponseHeaderReceived(const QHttpResponseHeader&)));
}

/**
 * Destructor.
 */
HttpClient::~HttpClient()
{
	m_http->close();
	m_http->disconnect();
	delete m_http;
}

/**
 * Called when the connection state changes.
 *
 * @param state HTTP connection state
 */
void HttpClient::slotStateChanged(int state)
{
	switch (state) {
		case QHttp::HostLookup:
			emitProgress(i18n("Ready."), CS_RequestConnection, CS_EstimatedBytes);
			break;
		case QHttp::Connecting:
			emitProgress(i18n("Connecting..."), CS_Connecting, CS_EstimatedBytes);
			break;
		case QHttp::Sending:
			emitProgress(i18n("Host found..."), CS_HostFound, CS_EstimatedBytes);
			break;
		case QHttp::Reading:
			emitProgress(i18n("Request sent..."), CS_RequestSent, CS_EstimatedBytes);
			break;
		case QHttp::Connected:
			emitProgress(i18n("Ready."), -1, -1);
			break;
		case QHttp::Unconnected:
		case QHttp::Closing:
		default:
			;
	}
}

/**
 * Called to report connection progress.
 *
 * @param done  bytes received
 * @param total total bytes, 0 if unknown
 */
void HttpClient::slotDataReadProgress(int done, int total)
{
	emitProgress(KCM_i18n1("Data received: %1", done), done, total);
}

/**
 * Called when the request is finished.
 *
 * @param error true if error occurred
 */
void HttpClient::slotDone(bool error)
{
	if (error) {
		QHttp::Error err = m_http->error();
		if (err != QHttp::UnexpectedClose) {
			QString msg(i18n("Socket error: "));
			switch (err) {
				case QHttp::ConnectionRefused:
					msg += i18n("Connection refused");
					break;
				case QHttp::HostNotFound:
					msg += i18n("Host not found");
					break;
				default:
					msg += m_http->errorString();
			}
			emitProgress(msg, -1, -1);
		}
	}
	emit bytesReceived(m_http->readAll());
	if (!error) {
		emitProgress(i18n("Ready."), CS_EstimatedBytes, CS_EstimatedBytes);
	}
}

/**
 * Called when the response header is available.
 *
 * @param resp HTTP response header
 */
void HttpClient::slotResponseHeaderReceived(const QHttpResponseHeader& resp)
{
	m_rcvBodyType = resp.contentType();
	m_rcvBodyLen = resp.contentLength();
}

/**
 * Send a HTTP GET request.
 *
 * @param server host name
 * @param path   path of the URL
 */
void HttpClient::sendRequest(const QString& server, const QString& path)
{
	m_rcvBodyLen = 0;
	m_rcvBodyType = "";
	QString dest;
	int destPort;
	splitNamePort(server, dest, destPort);
	m_http->setHost(dest, destPort);
	QString proxy, username, password;
	int proxyPort = 0;
	if (ConfigStore::s_miscCfg.m_useProxy) {
		splitNamePort(ConfigStore::s_miscCfg.m_proxy, proxy, proxyPort);
	}
	if (ConfigStore::s_miscCfg.m_useProxyAuthentication) {
		username = ConfigStore::s_miscCfg.m_proxyUserName;
		password = ConfigStore::s_miscCfg.m_proxyPassword;
	}
	m_http->setProxy(proxy, proxyPort, username, password);
	m_http->setHost(dest, destPort);
	m_http->get(path);
}

/**
 * Emit a progress signal with step/total steps.
 *
 * @param text       state text
 * @param step       current step
 * @param totalSteps total number of steps
 */
void HttpClient::emitProgress(const QString& text, int step, int totalSteps)
{
	emit progress(text, step, totalSteps);
}

/**
 * Extract name and port from string.
 *
 * @param namePort input string with "name:port"
 * @param name     output string with "name"
 * @param port     output integer with port
 */
void HttpClient::splitNamePort(const QString& namePort,
																 QString& name, int& port)
{
	int colPos = namePort.lastIndexOf(':');
	if (colPos >= 0) {
		bool ok;
		port = namePort.mid(colPos + 1).toInt(&ok);
		if (!ok) port = 80;
		name = namePort.left(colPos);
	} else {
		name = namePort;
		port = 80;
	}
}
