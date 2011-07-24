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
#include <QString>

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
DownloadDialog::DownloadDialog(QWidget* parent, const QString& caption) :
  QProgressDialog(parent)
{
  setWindowTitle(caption);
}

/**
 * Destructor.
 */
DownloadDialog::~DownloadDialog()
{
}

/**
 * Show dialog to report start of download.
 * @param url URL of download
 */
void DownloadDialog::showStartOfDownload(const QString& url)
{
  m_url = url;
  setLabelText(url);
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
    setRange(0, totalBytes);
    setValue(receivedBytes);
  }
}
