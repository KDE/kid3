/**
 * \file freedbclient.cpp
 * freedb.org client.
 * Originalliy implemented using QHttp, but this does not work on Qt < 3.0
 * and it hanged on connection errors or going via a proxy. Thus it is
 * now done using QSocket, which is not much more complicated.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
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
#include "freedbconfig.h"
#include "freedbclient.h"

#if defined WIN32 && QT_VERSION < 300
// see hostnameToAddress()
#include <winsock.h>
#endif

/**
 * Constructor.
 *
 * @param sb status bar to display progress information.
 */
FreedbClient::FreedbClient(QStatusBar *sb) :
 statusBar(sb), m_requestType(RT_None)
{
	sock = new QSocket();
	connect(sock, SIGNAL(hostFound()),
			this, SLOT(slotHostFound()));
	connect(sock, SIGNAL(connected()),
			this, SLOT(slotConnected()));
	connect(sock, SIGNAL(connectionClosed()),
			this, SLOT(slotConnectionClosed()));
	connect(sock, SIGNAL(readyRead()),
			this, SLOT(slotReadyRead()));
	connect(sock, SIGNAL(error(int)),
			this, SLOT(slotError(int)));
	statusBar->message(i18n("Ready."));
}

/**
 * Destructor.
 */
FreedbClient::~FreedbClient()
{
	sock->close();
	sock->disconnect();
	delete sock;
}

/**
 * Get string with proxy or destination and port.
 * If a proxy is set, the proxy is returned, else the real destination.
 *
 * @param cfg  freedb configuration
 * @param dst real destination
 *
 * @return "destinationname:port".
 */
static QString getProxyOrDest(const FreedbConfig *cfg, const QString dst)
{
	QString dest;
	if (cfg->useProxy) {
		dest = cfg->proxy;
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
void FreedbClient::splitNamePort(const QString &namePort,
																 QString &name, int &port)
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

#if defined WIN32 && QT_VERSION < 300
/**
 * Lookup hostname.
 * connectToHost() does not seem to look up the host names correctly on
 * Qt 2.3 non commerical Windows, so some Windows specific code is used here.
 *
 * @param adr input: hostname, output: IP address string
 *
 * @return 0 if ok,
 *         WINSOCK error code if error.
 */
static int hostnameToAddress(QString &adr)
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (err == 0) {
		struct hostent FAR *he = gethostbyname(adr.latin1());
		if (he) {
			adr = QHostAddress(htonl(*(u_long *)he->h_addr_list[0])).toString();
		} else {
			err = WSAGetLastError();
		}
		(void)WSACleanup();
	}
	return err;
}
#endif

/**
 * Find keyword in freedb.
 *
 * @param cfg  freedb configuration
 * @param what string with words to search
 */
void FreedbClient::find(const FreedbConfig *cfg, QString what)
{
	QString destNamePort(getProxyOrDest(cfg, "www.freedb.org:80"));
	QString dest;
	int destPort;
	splitNamePort(destNamePort, dest, destPort);
	what.replace(QRegExp(" +"), "+"); // replace spaces by '+'
	request = "GET http://www.freedb.org/freedb_search.php?words=" + what +
		"&allfields=NO&fields=artist&fields=title&allcats=NO" +
		"&cats=blues&cats=classical&cats=country&cats=folk&cats=jazz" +
		"&cats=misc&cats=newage&cats=reggae&cats=rock&cats=soundtrack" +
		"&grouping=none HTTP/1.1\r\nHost: " +
		cfg->server + "\r\nUser-Agent: Kid3 " VERSION "\r\nConnection: close\r\n\r\n";
#if defined WIN32 && QT_VERSION < 300
	int err = hostnameToAddress(dest);
	if (err) {
		statusBar->message(QString("WinSock error %1").arg(err));
		return;
	}
#endif
	sock->connectToHost(dest, destPort);
	m_requestType = RT_FindFreedbSearch;

	statusBar->message(i18n("Connecting..."));
}

/**
 * Find keyword in freedb with "cddb album" command of freedb2.org.
 *
 * @param cfg  freedb configuration
 * @param what string with words to search
 */
void FreedbClient::findCddbAlbum(const FreedbConfig *cfg, QString what)
{
	QString destNamePort(getProxyOrDest(cfg, cfg->server));
	QString dest;
	int destPort;
	splitNamePort(destNamePort, dest, destPort);
	what.replace(QRegExp(" +"), " "); // collapse spaces
	QUrl::encode(what);
	what.replace("%20", "+"); // replace spaces by '+'
	request = "GET http://" + cfg->server + cfg->cgiPath +
		"?cmd=cddb+album+" + what + "&hello=noname+localhost+" +
		"Kid3+" VERSION "&proto=1 HTTP/1.1\r\nHost: " + cfg->server +
		"\r\nConnection: close\r\n\r\n";
#if defined WIN32 && QT_VERSION < 300
	int err = hostnameToAddress(dest);
	if (err) {
		statusBar->message(QString("WinSock error %1").arg(err));
		return;
	}
#endif
	sock->connectToHost(dest, destPort);
	m_requestType = RT_FindCddbAlbum;

	statusBar->message(i18n("Connecting..."));
}

/**
 * Display status if host is found.
 */
void FreedbClient::slotHostFound()
{
	statusBar->message(i18n("Host found..."));
}

/**
 * Display status if connection is established.
 */
void FreedbClient::slotConnected()
{
	sock->writeBlock(request.latin1(), request.length());
	statusBar->message(i18n("Request sent..."));
}

/**
 * Read received data when the server has closed the connection.
 * The data is sent to other objects via signals.
 */
void FreedbClient::slotConnectionClosed()
{
	Q_ULONG len = sock->bytesAvailable();
	QCString s;
	s.resize(len + 1);
	sock->readBlock(s.data(), len);
	rcvStr = QString::fromUtf8(s.data());
	switch (m_requestType) {
		case RT_Album:
			emit albumFinished(rcvStr);
			break;
		case RT_FindFreedbSearch:
			emit findFinished(rcvStr);
			break;
		case RT_FindCddbAlbum:
			emit findCddbAlbumFinished(rcvStr);
			break;
		default:
			qWarning("Unknown freedb request type");
	}
	sock->close();
	statusBar->message(i18n("Ready."));
}

/**
 * Display information about read progress.
 */
void FreedbClient::slotReadyRead()
{
	statusBar->message(i18n("Data received: %1").arg(sock->bytesAvailable()));
}

/**
 * Display information about socket error.
 */
void FreedbClient::slotError(int err)
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
	statusBar->message(msg);
}

/**
 * Request track list from freedb server.
 *
 * @param cfg freedb configuration
 * @param cat category
 * @param id  ID
 */
void FreedbClient::getTrackList(const FreedbConfig *cfg, QString cat, QString id)
{
	QString destNamePort(getProxyOrDest(cfg, cfg->server));
	QString dest;
	int destPort;
	splitNamePort(destNamePort, dest, destPort);
	request = "GET http://" + cfg->server + cfg->cgiPath +
		"?cmd=cddb+read+" + cat + "+" +
		id + "&hello=noname+localhost+" +
		"Kid3+" VERSION "&proto=1 HTTP/1.1\r\nHost: " + cfg->server +
		"\r\nConnection: close\r\n\r\n";

#if defined WIN32 && QT_VERSION < 300
	int err = hostnameToAddress(dest);
	if (err) {
		statusBar->message(QString("WinSock error %1").arg(err));
		return;
	}
#endif
	sock->connectToHost(dest, destPort);
	m_requestType = RT_Album;
	statusBar->message(i18n("Connecting..."));
}
