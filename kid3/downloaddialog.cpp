/**
 * \file downloaddialog.cpp
 * Dialog displayed during a download.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 31 Dec 2008
 *
 * Copyright (C) 2008-2009  Urs Fleisch
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

#include "downloaddialog.h"
#include "httpclient.h"
#include <qstring.h>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
DownloadDialog::DownloadDialog(QWidget* parent, const QString& caption) :
	QProgressDialog(parent), m_client(0)
{
	QCM_setWindowTitle(caption);
	connect(this, SIGNAL(canceled()),
					this, SLOT(cancelDownload()));
}

/**
 * Destructor.
 */
DownloadDialog::~DownloadDialog()
{
	delete m_client;
	m_client = 0;
}

/**
 * Display message in status bar.
 *
 * @param msg           status message
 * @param receivedBytes bytes received
 * @param totalBytes    total bytes
 */
void DownloadDialog::updateProgressStatus(const QString& msg,
																					int receivedBytes, int totalBytes)
{
	setLabelText(m_url + '\n' + msg);
	if (receivedBytes >= 0 && totalBytes >= 0) {
#if QT_VERSION >= 0x040000
		setRange(0, totalBytes);
		setValue(receivedBytes);
#else
		setProgress(receivedBytes, totalBytes);
#endif
	}
}

/**
 * Send a download request.
 *
 * @param hostName server
 * @param path     path on server
 */
void DownloadDialog::startDownload(const QString& hostName, const QString& path)
{
	if (!m_client) {
		m_client = new HttpClient;
		connect(m_client, SIGNAL(bytesReceived(const QByteArray&)),
						this, SLOT(requestFinished(const QByteArray&)));
		connect(m_client, SIGNAL(progress(const QString&, int, int)),
						this, SLOT(updateProgressStatus(const QString&, int, int)));
	}
	if (m_client) {
		m_url = "http://";
		m_url += hostName;
		m_url += path;
		setLabelText(m_url);
		updateProgressStatus(i18n("Ready."),
		                     HttpClient::CS_RequestConnection,
		                     HttpClient::CS_EstimatedBytes);
		m_client->sendRequest(hostName, path);
	}
}

/**
 * Cancel a download.
 */
void DownloadDialog::cancelDownload()
{
	delete m_client;
	m_client = 0;
	reset();
}

/**
 * Handle response when request is finished.
 * downloadFinished() is emitted.
 *
 * @param data received data
 */
void DownloadDialog::requestFinished(const QByteArray& data)
{
	if (!wasCanceled()) {
		emit downloadFinished(data, m_client->getContentType(), m_url);
	}
}
