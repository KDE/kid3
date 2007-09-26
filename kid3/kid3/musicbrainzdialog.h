/**
 * \file musicbrainzdialog.h
 * MusicBrainz import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#ifndef MUSICBRAINZDIALOG_H
#define MUSICBRAINZDIALOG_H

#include "config.h"

#include "importtrackdata.h"
#include <qdialog.h>
#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVector>
class QTableWidget;
#else
#include <qvaluevector.h>
class QTable;
#endif

class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QTimer;
class QStatusBar;
class MusicBrainzConfig;
class MusicBrainzClient;

/**
 * musicBrainz.org import dialog.
 */
class MusicBrainzDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent          parent widget
	 * @param trackDataVector track data to be filled with imported values,
	 *                        is passed with filenames set
	 */
	MusicBrainzDialog(QWidget* parent,
										ImportTrackDataVector& trackDataVector);
	/**
	 * Destructor.
	 */
	~MusicBrainzDialog();

#ifdef HAVE_TUNEPIMP
	/**
	 * Initialize the table.
	 * Has to be called before reusing the dialog with new track data.
	 */
	void initTable();

	/**
	 * Get string with server and port.
	 *
	 * @return "servername:port".
	 */
	QString getServer() const;

	/**
	 * Set string with server and port.
	 *
	 * @param srv "servername:port"
	 */
	void setServer(const QString& srv);
#endif // HAVE_TUNEPIMP

signals:
	/**
	 * Emitted when the m_trackDataVector was updated with new imported data.
	 */
	void trackDataUpdated();

public slots:
	/**
	 * Shows the dialog as a modal dialog.
	 */
	int exec();

protected slots:
	/**
	 * Hides the dialog and sets the result to QDialog::Accepted.
	 */
	virtual void accept();

	/**
	 * Hides the dialog and sets the result to QDialog::Rejected.
	 */
	virtual void reject();

private slots:
	/**
	 * Set the configuration in the client.
	 */
	void setClientConfig();

	/**
	 * Called when the periodic timer times out.
	 * Used to poll the MusicBrainz client.
	 */
	void timerDone();

	/**
	 * Apply imported data.
	 */
	void apply();

	/**
	 * Set the status of a file.
	 *
	 * @param index  index of file
	 * @param status status string
	 */
	void setFileStatus(int index, QString status);

	/**
	 * Update the track data combo box of a file.
	 *
	 * @param index  index of file
	 */
	void updateFileTrackData(int index);

	/**
	 * Set meta data for a file.
	 *
	 * @param index     index of file
	 * @param trackData meta data
	 */
	void setMetaData(int index, ImportTrackData& trackData);

	/**
	 * Set result list for a file.
	 *
	 * @param index           index of file
	 * @param trackDataVector result list
	 */
	void setResults(int index, ImportTrackDataVector& trackDataVector);

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

	/**
	 * Show the name of the current track in the status bar.
	 *
	 * @param row table row
	 */
	void showFilenameInStatusBar(int row);

#ifdef HAVE_TUNEPIMP
private:
	/**
	 * Clear all results.
	 */
	void clearResults();

 /**
	* Create and start the MusicBrainz client.
	*/
	void startClient();

	/**
	 * Stop and destroy the MusicBrainz client.
	 */
	void stopClient();

	QComboBox* m_serverComboBox;
#if QT_VERSION >= 0x040000
	QTableWidget* m_albumTable;
#else
	QTable* m_albumTable;
#endif
	QStatusBar* m_statusBar;
	QTimer* m_timer;
	MusicBrainzClient* m_client;
	ImportTrackDataVector& m_trackDataVector;
#if QT_VERSION >= 0x040000
	QVector<ImportTrackDataVector> m_trackResults;
#else
	QValueVector<ImportTrackDataVector> m_trackResults;
#endif
#endif // HAVE_TUNEPIMP
};

#endif // MUSICBRAINZDIALOG_H
