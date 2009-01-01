/**
 * \file downloaddialog.h
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

#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <qprogressdialog.h>
#include <qstring.h>

class HttpClient;

/**
 * Dialog displayed during a download.
 */
class DownloadDialog : public QProgressDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
   *
	 * @param parent  parent widget
	 * @param caption dialog title
   */
	DownloadDialog(QWidget* parent, const QString& caption);

	/**
	 * Destructor.
	 */
	virtual ~DownloadDialog();

  /**
   * Send a download request.
   *
   * @param hostName server
   * @param path     path on server
   */
	void startDownload(const QString& hostName, const QString& path);

public slots:
	/**
	 * Cancel a download.
	 */
	void cancelDownload();

private slots:
	/**
	 * Handle response when request is finished.
	 * downloadFinished() is emitted.
	 *
	 * @param data received data
	 */
	void requestFinished(const QByteArray& data);

	/**
	 * Display progress status.
	 *
	 * @param msg           status message
	 * @param receivedBytes bytes received
	 * @param totalBytes    total bytes
	 */
	void updateProgressStatus(const QString& msg, int receivedBytes, int totalBytes);

signals:
	/**
	 * Emitted when download finished.
	 * Parameter: bytes containing download, content type, URL
	 */
	void downloadFinished(const QByteArray&, const QString&, const QString&);

private:
	HttpClient* m_client;
	QString m_url;
};

#endif // DOWNLOADDIALOG_H
