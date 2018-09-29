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

#include <QProgressDialog>
#include <QString>

/**
 * Dialog displayed during a download.
 *
 * The download dialog can be used together with the DownloadClient to get
 * progress feedback. These two objects have to be connected in the following
 * way (DownloadClient to DownloadDialog):
 * - progress() to updateProgressStatus(),
 * - downloadStarted() to showStartOfDownload(),
 * - cancelDownload() from canceled(),
 * - aborted() to reset().
 *
 * A download is started with DownloadClient::startDownload() and termination
 * signaled by DownloadClient::downloadFinished().
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
  virtual ~DownloadDialog() override;

public slots:
  /**
   * Show dialog to report start of download.
   * @param url URL of download
   */
  void showStartOfDownload(const QString& url);

  /**
   * Display progress status.
   *
   * @param msg           status message
   * @param receivedBytes bytes received
   * @param totalBytes    total bytes
   */
  void updateProgressStatus(const QString& msg, int receivedBytes, int totalBytes);

private:
  QString m_url;
};

#endif // DOWNLOADDIALOG_H
