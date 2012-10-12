/**
 * \file httpclient.cpp
 * Client to connect to HTTP server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Dec 2008
 *
 * Copyright (C) 2008-2012  Urs Fleisch
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
#include "configstore.h"
#include "qtcompatmac.h"


/**
 * Constructor.
 *
 * @param netMgr  network access manager
 */
HttpClient::HttpClient(QNetworkAccessManager* netMgr) :
  QObject(netMgr), m_netMgr(netMgr), m_rcvBodyLen(0)
{
  setObjectName("HttpClient");
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
#define DATA_RECEIVED_FOR_PO I18N_NOOP("Data received: %1")

/**
 * Called when the request is finished.
 */
void HttpClient::networkReplyFinished()
{
  if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender())) {
    QByteArray data(reply->readAll());
    m_rcvBodyType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    m_rcvBodyLen = reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
    emit bytesReceived(data);
    QString msg(i18n("Ready."));
    if (reply->error() != QNetworkReply::NoError) {
      msg = i18n("Error");
      msg += ": ";
      msg += reply->errorString();
    }
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
  emitProgress(i18n("Data received: %1").arg(received), received, total);
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
 * @param server host name
 * @param path   path of the URL
 */
void HttpClient::sendRequest(const QString& server, const QString& path,
                             const QMap<QByteArray, QByteArray>& headers)
{
  m_rcvBodyLen = 0;
  m_rcvBodyType = "";
  QString proxy, username, password;
  int proxyPort = 0;
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
  if (ConfigStore::s_miscCfg.m_useProxy) {
    splitNamePort(ConfigStore::s_miscCfg.m_proxy, proxy, proxyPort);
    proxyType = QNetworkProxy::HttpProxy;
  }
  if (ConfigStore::s_miscCfg.m_useProxyAuthentication) {
    username = ConfigStore::s_miscCfg.m_proxyUserName;
    password = ConfigStore::s_miscCfg.m_proxyPassword;
  }
  m_netMgr->setProxy(QNetworkProxy(proxyType, proxy, proxyPort,
                                   username, password));

  QString host(server);
  if (host.endsWith(":80")) {
    host.chop(3);
  }
  QUrl url;
  url.setEncodedUrl(("http://" + host + path).toAscii());
  QNetworkRequest request(url);
  for (QMap<QByteArray, QByteArray>::const_iterator it =
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
  emitProgress(i18n("Request sent..."), 0, 0);
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
