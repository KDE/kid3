/**
 * \file importsourceclient.cpp
 * Client to connect to server with import data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qregexp.h>
#include <qsocket.h>
#include <qstatusbar.h>
#include <qurl.h>
#include "importsourceclient.h"
#include "importsourceconfig.h"
#include "kid3.h"

/**
 * Constructor.
 */
ImportSourceClient::ImportSourceClient() :
 m_statusBar(0), m_requestType(RT_None)
{
	m_sock = new QSocket();
	connect(m_sock, SIGNAL(hostFound()),
			this, SLOT(slotHostFound()));
	connect(m_sock, SIGNAL(connected()),
			this, SLOT(slotConnected()));
	connect(m_sock, SIGNAL(connectionClosed()),
			this, SLOT(slotConnectionClosed()));
	connect(m_sock, SIGNAL(readyRead()),
			this, SLOT(slotReadyRead()));
	connect(m_sock, SIGNAL(error(int)),
			this, SLOT(slotError(int)));
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
	m_statusBar->message(i18n("Ready."));
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
	int colPos = namePort.findRev(':');
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

	m_statusBar->message(i18n("Connecting..."));
}

/**
 * Display status if host is found.
 */
void ImportSourceClient::slotHostFound()
{
	m_statusBar->message(i18n("Host found..."));
}

/**
 * Display status if connection is established.
 */
void ImportSourceClient::slotConnected()
{
	m_sock->writeBlock(m_request.latin1(), m_request.length());
	m_statusBar->message(i18n("Request sent..."));
}

/**
 * Read received data when the server has closed the connection.
 * The data is sent to other objects via signals.
 */
void ImportSourceClient::slotConnectionClosed()
{
	Q_ULONG len = m_sock->bytesAvailable();
	QCString rcvStr;
	rcvStr.resize(len + 1);
	m_sock->readBlock(rcvStr.data(), len);
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
	m_statusBar->message(i18n("Ready."));
}

/**
 * Display information about read progress.
 */
void ImportSourceClient::slotReadyRead()
{
	m_statusBar->message(i18n("Data received: %1").arg(m_sock->bytesAvailable()));
}

/**
 * Display information about socket error.
 */
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
	m_statusBar->message(msg);
}

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
	m_statusBar->message(i18n("Connecting..."));
}
