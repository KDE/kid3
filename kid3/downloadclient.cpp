/**
 * \file downloadclient.cpp
 * Client to download via http.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Jun 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent object
 */
DownloadClient::DownloadClient(QObject* parent) : HttpClient(parent),
	m_canceled(false)
{
	connect(this, SIGNAL(bytesReceived(const QByteArray&)),
					this, SLOT(requestFinished(const QByteArray&)));
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
	m_url = "http://";
	m_url += hostName;
	m_url += path;
	emit downloadStarted(m_url);
	emit progress(i18n("Ready."),
								HttpClient::CS_RequestConnection,
								HttpClient::CS_EstimatedBytes);
	sendRequest(hostName, path);
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

