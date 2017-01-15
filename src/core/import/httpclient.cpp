/**
 * \file httpclient.cpp
 * Client to connect to HTTP server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Dec 2008
 *
 * Copyright (C) 2008-2017  Urs Fleisch
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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QByteArray>
#include "networkconfig.h"


/** Time when last request was sent to server */
QMap<QString, QDateTime> HttpClient::s_lastRequestTime;
/** Minimum interval between two requests to server in ms */
QMap<QString, int> HttpClient::s_minimumRequestInterval;

/**
 * Used to initialize the minimum interval for some servers
 * at static initialization time.
 *
 * Rate limit requests to servers, MusicBrainz and Discogs impose a limit of
 * one request per second
 * http://musicbrainz.org/doc/XML_Web_Service/Rate_Limiting#Source_IP_address
 * http://www.discogs.com/developers/accessing.html#rate-limiting
 */
static struct MinimumRequestIntervalInitializer {
  MinimumRequestIntervalInitializer() {
    HttpClient::s_minimumRequestInterval[QLatin1String("musicbrainz.org")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("api.discogs.com")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("www.amazon.com")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("images.amazon.com")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("www.gnudb.org")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("gnudb.gnudb.org")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("tracktype.org")] = 1000;
    HttpClient::s_minimumRequestInterval[QLatin1String("api.acoustid.org")] = 1000;
  }
} minimumRequestIntervalInitializer;

/**
 * Constructor.
 *
 * @param netMgr  network access manager
 */
HttpClient::HttpClient(QNetworkAccessManager* netMgr) :
  QObject(netMgr), m_netMgr(netMgr), m_rcvBodyLen(0),
  m_requestTimer(new QTimer(this))
{
  setObjectName(QLatin1String("HttpClient"));
  m_requestTimer->setSingleShot(true);
  connect(m_requestTimer, SIGNAL(timeout()), this, SLOT(delayedSendRequest()));
}

/**
 * Destructor.
 */
HttpClient::~HttpClient()
{
  if (m_reply) {
    m_reply->close();
    m_reply->disconnect();
    m_reply->deleteLater();
  }
}

/** Only defined for generation of translation files */
#define DATA_RECEIVED_FOR_PO QT_TRANSLATE_NOOP("@default", "Data received: %1")

/**
 * Called when the request is finished.
 */
void HttpClient::networkReplyFinished()
{
  if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender())) {
    QByteArray data(reply->readAll());
    m_rcvBodyType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    m_rcvBodyLen = reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
    QString msg(tr("Ready."));
    if (reply->error() != QNetworkReply::NoError) {
      msg = tr("Error");
      msg += QLatin1String(": ");
      msg += reply->errorString();
    } else {
      QVariant redirect =
          reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
      if (!redirect.isNull()) {
        QUrl redirectUrl = redirect.toUrl();
        if (redirectUrl.isRelative()) {
          redirectUrl = reply->url().resolved(redirectUrl);
        }
        if (redirectUrl.isValid()) {
          reply->deleteLater();

          QNetworkRequest request(redirectUrl);
          reply = m_netMgr->get(request);
          m_reply = reply;
          connect(reply, SIGNAL(finished()),
                  this, SLOT(networkReplyFinished()));
          connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                  this, SLOT(networkReplyProgress(qint64,qint64)));
          connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                  this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
          return;
        }
      }
    }
    emit bytesReceived(data);
    emitProgress(msg, data.size(), data.size());
    reply->deleteLater();
  }
}

/**
 * Called to report connection progress.
 *
 * @param received bytes received
 * @param total total bytes
 */
void HttpClient::networkReplyProgress(qint64 received, qint64 total)
{
  emitProgress(tr("Data received: %1").arg(received), received, total);
}

/**
 * Called when an error occurred.
 *
 * @param code error code
 */
void HttpClient::networkReplyError(QNetworkReply::NetworkError)
{
  if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender())) {
    emitProgress(reply->errorString(), -1, -1);
  }
}

/**
 * Send a HTTP GET request.
 *
 * @param url URL
 * @param headers optional raw headers to send
 */
void HttpClient::sendRequest(const QUrl& url, const RawHeaderMap& headers)
{
  QString host = url.host();
  qint64 msSinceLastRequest;
  int minimumRequestInterval;
  QDateTime now = QDateTime::currentDateTime();
  QDateTime lastRequestTime = s_lastRequestTime.value(host);
  if (lastRequestTime.isValid() &&
      (minimumRequestInterval = s_minimumRequestInterval.value(host)) > 0 &&
      (msSinceLastRequest = lastRequestTime.msecsTo(now)) <
      minimumRequestInterval) {
    // Delay request to comply with minimum interval
    m_delayedSendRequestContext.url = url;
    m_delayedSendRequestContext.headers = headers;
    m_requestTimer->start(minimumRequestInterval -
                          static_cast<int>(msSinceLastRequest));
    return;
  }

  m_rcvBodyLen = 0;
  m_rcvBodyType = QLatin1String("");
  QString proxy, username, password;
  int proxyPort = 0;
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
  const NetworkConfig& networkCfg = NetworkConfig::instance();
  if (networkCfg.useProxy()) {
    splitNamePort(networkCfg.proxy(), proxy, proxyPort);
    proxyType = QNetworkProxy::HttpProxy;
  }
  if (networkCfg.useProxyAuthentication()) {
    username = networkCfg.proxyUserName();
    password = networkCfg.proxyPassword();
  }
  m_netMgr->setProxy(QNetworkProxy(proxyType, proxy,
                                   static_cast<quint16>(proxyPort),
                                   username, password));

  QNetworkRequest request(url);
  for (RawHeaderMap::const_iterator it =
         headers.constBegin();
       it != headers.constEnd();
       ++it) {
    request.setRawHeader(it.key(), it.value());
  }
  QNetworkReply* reply = m_netMgr->get(request);
  m_reply = reply;
  connect(reply, SIGNAL(finished()),
          this, SLOT(networkReplyFinished()));
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
          this, SLOT(networkReplyProgress(qint64,qint64)));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
  s_lastRequestTime[host] = now;
  emitProgress(tr("Request sent..."), 0, 0);
}

/**
 * Send a HTTP GET request.
 *
 * @param server host name
 * @param path   path of the URL
 * @param scheme scheme, default is "http"
 * @param headers optional raw headers to send
 */
void HttpClient::sendRequest(const QString& server, const QString& path,
                             const QString& scheme, const RawHeaderMap& headers)
{
  QString host(server);
  if (host.endsWith(QLatin1String(":80"))) {
    host.chop(3);
  }
  QUrl url;
#if QT_VERSION >= 0x050000
  url.setUrl(scheme + QLatin1String("://") + host + path);
#else
  url.setEncodedUrl((scheme + QLatin1String("://") + host + path).toAscii());
#endif
  sendRequest(url, headers);
}

/**
 * Called to start delayed sendRequest().
 */
void HttpClient::delayedSendRequest()
{
  sendRequest(m_delayedSendRequestContext.url,
              m_delayedSendRequestContext.headers);
}

/**
 * Abort request.
 */
void HttpClient::abort()
{
  if (m_reply) {
    m_reply->abort();
  }
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
  int colPos = namePort.lastIndexOf(QLatin1Char(':'));
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
