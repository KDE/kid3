/**
 * \file downloadclient.h
 * Client to download via http.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Jun 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#pragma once

#include <QString>
#include "httpclient.h"

/**
 * Client to download via HTTP.
 * @see DownloadDialog
 */
class KID3_CORE_EXPORT DownloadClient : public HttpClient {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param netMgr network access manager
   */
  explicit DownloadClient(QNetworkAccessManager* netMgr);

  /**
   * Destructor.
   */
  virtual ~DownloadClient() override = default;

  /**
   * Send a download request.
   *
   * @param url URL of resource to download
   */
  void startDownload(const QUrl& url);

  /**
   * Get the URL of an image file.
   * The input URL is transformed using the match picture URL table to
   * get the URL of an image file.
   *
   * @param url URL from image drag
   *
   * @return URL of image file, empty if no image URL found.
   */
  static QUrl getImageUrl(const QUrl& url);

public slots:
  /**
   * Cancel a download.
   */
  void cancelDownload();

signals:
  /**
   * Emitted when download is started
   * @param url URL of download
   */
  void downloadStarted(const QString& url);

  /**
   * Emitted when download finished.
   * @param data bytes containing download
   * @param contentType content type
   * @param url URL
   */
  void downloadFinished(const QByteArray& data, const QString& contentType,
                        const QString& url);

  /**
   * Emitted when a download is aborted.
   */
  void aborted();

private slots:
  /**
   * Handle response when request is finished.
   * downloadFinished() is emitted.
   *
   * @param data received data
   */
  void requestFinished(const QByteArray& data);

private:
  QUrl m_url;
  bool m_canceled;
};
