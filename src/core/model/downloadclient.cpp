/**
 * \file downloadclient.cpp
 * Client to download via http.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Jun 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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

#include "downloadclient.h"
#include "importconfig.h"

/**
 * Constructor.
 *
 * @param netMgr network access manager
 */
DownloadClient::DownloadClient(QNetworkAccessManager* netMgr) :
  HttpClient(netMgr), m_canceled(false)
{
  connect(this, SIGNAL(bytesReceived(QByteArray)),
          this, SLOT(requestFinished(QByteArray)));
}

/**
 * Destructor.
 */
DownloadClient::~DownloadClient()
{
}

/**
 * Send a download request.
 *
 * @param hostName server
 * @param path     path on server
 */
void DownloadClient::startDownload(const QString& hostName, const QString& path)
{
  m_canceled = false;
  m_url = QLatin1String("http://");
  m_url += hostName;
  m_url += path;
  emit downloadStarted(m_url);
  emit progress(tr("Ready."), 0, 0);
  sendRequest(hostName, path);
}

/**
 * Send a download request.
 *
 * @param url URL of resource to download
 */
void DownloadClient::startDownload(const QString& url)
{
  int hostPos = url.indexOf(QLatin1String("://"));
  if (hostPos > 0) {
    int pathPos = url.indexOf(QLatin1Char('/'), hostPos + 3);
    if (pathPos > hostPos) {
      startDownload(url.mid(hostPos + 3, pathPos - hostPos - 3),
                    url.mid(pathPos));
    }
  }
}

/**
 * Cancel a download.
 */
void DownloadClient::cancelDownload()
{
  m_canceled = true;
  abort();
  emit aborted();
}

/**
 * Handle response when request is finished.
 * downloadFinished() is emitted.
 *
 * @param data received data
 */
void DownloadClient::requestFinished(const QByteArray& data)
{
  if (!m_canceled) {
    emit downloadFinished(data, getContentType(), m_url);
  }
}

/**
 * Get the URL of an image file.
 * The input URL is transformed using the match picture URL table to
 * get the URL of an image file.
 *
 * @param url URL from image drag
 *
 * @return URL of image file, empty if no image URL found.
 */
QString DownloadClient::getImageUrl(const QString& url)
{
  QString imgurl;
  if (url.startsWith(QLatin1String("http://"))) {
    if (url.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive) ||
        url.endsWith(QLatin1String(".jpeg"), Qt::CaseInsensitive) ||
        url.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)) {
      imgurl = url;
    }
    else {
      for (QMap<QString, QString>::ConstIterator it =
             ImportConfig::instance().m_matchPictureUrlMap.begin();
           it != ImportConfig::instance().m_matchPictureUrlMap.end();
           ++it) {
        QRegExp re(it.key());
        if (re.exactMatch(url)) {
          imgurl = url;
          imgurl.replace(re, *it);
          if (imgurl.indexOf(QLatin1String("%25")) != -1) {
            // double URL encoded: first decode
            imgurl = QUrl::fromPercentEncoding(imgurl.toUtf8());
          }
          if (imgurl.indexOf(QLatin1String("%2F")) != -1) {
            // URL encoded: decode
            imgurl = QUrl::fromPercentEncoding(imgurl.toUtf8());
          }
          break;
        }
      }
    }
  }
  return imgurl;
}
