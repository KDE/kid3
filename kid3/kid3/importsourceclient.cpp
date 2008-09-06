/**
 * \file importsourceclient.cpp
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

#include "importsourceclient.h"
#include <qregexp.h>
#include <qstatusbar.h>
#include <qurl.h>

#include "importsourceconfig.h"
#include "kid3.h"

/**
 * Constructor.
 */
ImportSourceClient::ImportSourceClient() :
 m_statusBar(0), m_requestType(RT_None)
{
#if QT_VERSION >= 0x040000
	m_sock = new QTcpSocket();
	connect(m_sock, SIGNAL(disconnected()),
			this, SLOT(slotConnectionClosed()));
	connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(slotError(QAbstractSocket::SocketError)));
#else
	m_sock = new QSocket();
	connect(m_sock, SIGNAL(connectionClosed()),
			this, SLOT(slotConnectionClosed()));
	connect(m_sock, SIGNAL(error(int)),
			this, SLOT(slotError(int)));
#endif
	connect(m_sock, SIGNAL(hostFound()),
			this, SLOT(slotHostFound()));
	connect(m_sock, SIGNAL(connected()),
			this, SLOT(slotConnected()));
	connect(m_sock, SIGNAL(readyRead()),
			this, SLOT(slotReadyRead()));
}

/**
 * Destructor.
 */
ImportSourceClient::~ImportSourceClient()
{
	m_sock->close();
	m_sock->disconnect();
	delete m_sock;
}

/**
 * Initialize object.
 * Has to be called before use.
 *
 * @param sb status bar to display progress information.
 */
void ImportSourceClient::init(QStatusBar* sb)
{
	m_statusBar = sb;
	m_statusBar->QCM_showMessage(i18n("Ready."));
}

/**
 * Get string with proxy or destination and port.
 * If a proxy is set, the proxy is returned, else the real destination.
 *
 * @param dst real destination
 *
 * @return "destinationname:port".
 */
QString ImportSourceClient::getProxyOrDest(const QString& dst)
{
	QString dest;
	if (Kid3App::s_miscCfg.m_useProxy) {
		dest = Kid3App::s_miscCfg.m_proxy;
	}
	if (dest.isEmpty()) {
		dest = dst;
	}
	return dest;
}

/**
 * Extract name and port from string.
 *
 * @param namePort input string with "name:port"
 * @param name     output string with "name"
 * @param port     output integer with port
 */
void ImportSourceClient::splitNamePort(const QString& namePort,
																 QString& name, int& port)
{
	int colPos = namePort.QCM_lastIndexOf(':');
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

/**
 * Find keyword on server.
 *
 * @param cfg    import source configuration
 * @param artist artist to search
 * @param album  album to search
 */
void ImportSourceClient::find(const ImportSourceConfig* cfg,
															const QString& artist, const QString& album)
{
	QString dest;
	int destPort;
	constructFindQuery(cfg, artist, album, dest, destPort);
	m_sock->connectToHost(dest, destPort);
	m_requestType = RT_Find;

	m_statusBar->QCM_showMessage(i18n("Connecting..."));
}

/**
 * Display status if host is found.
 */
void ImportSourceClient::slotHostFound()
{
	m_statusBar->QCM_showMessage(i18n("Host found..."));
}

/**
 * Display status if connection is established.
 */
void ImportSourceClient::slotConnected()
{
	m_sock->QCM_writeBlock(m_request.QCM_latin1(), m_request.length());
	m_statusBar->QCM_showMessage(i18n("Request sent..."));
}

/**
 * Read received data when the server has closed the connection.
 * The data is sent to other objects via signals.
 */
void ImportSourceClient::slotConnectionClosed()
{
	unsigned long len = m_sock->bytesAvailable();
	QByteArray rcvStr;
	rcvStr.resize(len + 1);
	m_sock->QCM_readBlock(rcvStr.data(), len);
	switch (m_requestType) {
		case RT_Album:
			emit albumFinished(rcvStr);
			break;
		case RT_Find:
			emit findFinished(rcvStr);
			break;
		default:
			qWarning("Unknown import request type");
	}
	m_sock->close();
	m_statusBar->QCM_showMessage(i18n("Ready."));
}

/**
 * Display information about read progress.
 */
void ImportSourceClient::slotReadyRead()
{
	m_statusBar->QCM_showMessage(KCM_i18n1("Data received: %1", m_sock->bytesAvailable()));
}

/**
 * Display information about socket error.
 */
#if QT_VERSION >= 0x040000
void ImportSourceClient::slotError(QAbstractSocket::SocketError err)
{
	if (err == QAbstractSocket::RemoteHostClosedError)
		return;
	QString msg(i18n("Socket error: "));
	switch (err) {
		case QAbstractSocket::ConnectionRefusedError:
			msg += i18n("Connection refused");
			break;
		case QAbstractSocket::HostNotFoundError:
			msg += i18n("Host not found");
			break;
		case QAbstractSocket::SocketAccessError:
			msg += i18n("Read failed");
			break;
		default:
			msg += m_sock->errorString();
	}
	m_statusBar->QCM_showMessage(msg);
}
#else
void ImportSourceClient::slotError(int err)
{
	QString msg(i18n("Socket error: "));
	switch (err) {
		case QSocket::ErrConnectionRefused:
			msg += i18n("Connection refused");
			break;
		case QSocket::ErrHostNotFound:
			msg += i18n("Host not found");
			break;
		case QSocket::ErrSocketRead:
			msg += i18n("Read failed");
			break;
		default:
			msg += QString::number(err);
	}
	m_statusBar->QCM_showMessage(msg);
}
#endif

/**
 * Request track list from server.
 *
 * @param cfg import source configuration
 * @param cat category
 * @param id  ID
 */
void ImportSourceClient::getTrackList(const ImportSourceConfig* cfg, QString cat, QString id)
{
	QString dest;
	int destPort;
	constructTrackListQuery(cfg, cat, id, dest, destPort);
	m_sock->connectToHost(dest, destPort);
	m_requestType = RT_Album;
	m_statusBar->QCM_showMessage(i18n("Connecting..."));
}
