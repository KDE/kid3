/**
 * \file httpclient.h
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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QPointer>
#include <QMap>
#include <QDateTime>
#include <QTimer>
#include "kid3api.h"

class QByteArray;
class QNetworkAccessManager;

/**
 * Client to connect to HTTP server.
 */
class KID3_CORE_EXPORT HttpClient : public QObject {
Q_OBJECT

public:
  /** Name-value map for raw HTTP headers. */
  typedef QMap<QByteArray, QByteArray> RawHeaderMap;

  /**
   * Constructor.
   *
   * @param netMgr  network access manager
   */
  explicit HttpClient(QNetworkAccessManager* netMgr);

  /**
   * Destructor.
   */
  virtual ~HttpClient();

  /**
   * Send a HTTP GET request.
   *
   * @param url URL
   * @param headers optional raw headers to send
   */
  void sendRequest(const QUrl& url,
                   const RawHeaderMap& headers = RawHeaderMap());

  /**
   * Send a HTTP GET request.
   *
   * @param server host name
   * @param path   path of the URL
   * @param scheme scheme, default is "http"
   * @param headers optional raw headers to send
   */
  void sendRequest(const QString& server, const QString& path,
                   const QString& scheme = QLatin1String("http"),
                   const RawHeaderMap& headers = RawHeaderMap());

  /**
   * Abort request.
   */
  void abort();

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
   * Called when the request is finished.
   */
  void networkReplyFinished();

  /**
   * Called to report connection progress.
   *
   * @param received bytes received
   * @param total total bytes
   */
  void networkReplyProgress(qint64 received, qint64 total);

  /**
   * Called when an error occurred.
   *
   * @param code error code
   */
  void networkReplyError(QNetworkReply::NetworkError code);

  /**
   * Called to start delayed sendRequest().
   */
  void delayedSendRequest();

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

  /** network access manager */
  QNetworkAccessManager* m_netMgr;
  /** network reply if available, else 0 */
  QPointer<QNetworkReply> m_reply;
  /** content length of entitiy-body, 0 if not available */
  unsigned long m_rcvBodyLen;
  /** content type */
  QString m_rcvBodyType;
  /** Timer used to delay requests */
  QTimer* m_requestTimer;
  /** Context for delayedSendRequest() */
  struct {
    QUrl url;
    RawHeaderMap headers;
  } m_delayedSendRequestContext;

  friend struct MinimumRequestIntervalInitializer;

  /** Time when last request was sent to server */
  static QMap<QString, QDateTime> s_lastRequestTime;
  /** Minimum interval between two requests to server in ms */
  static QMap<QString, int> s_minimumRequestInterval;
};

#endif
