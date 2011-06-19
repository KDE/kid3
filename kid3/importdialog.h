/**
 * \file importdialog.h
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include "config.h"
#include "trackdata.h"
#include "importconfig.h"

class QString;
class QPushButton;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QTableView;
class TrackDataModel;
class FreedbImporter;
class TrackTypeImporter;
class MusicBrainzDialog;
class MusicBrainzReleaseImporter;
class DiscogsImporter;
class AmazonImporter;
class ServerImporter;
class ServerImportDialog;
class TextImportDialog;
class ImportTrackDataVector;
class FrameCollection;
class FreedbConfig;
#ifdef HAVE_TUNEPIMP
class MusicBrainzConfig;
#endif

/**
 * Import dialog.
 */
class ImportDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Sub-Dialog to be started automatically.
	 */
	enum AutoStartSubDialog {
		ASD_None,
		ASD_Freedb,
		ASD_TrackType,
		ASD_Discogs,
		ASD_Amazon,
		ASD_MusicBrainzRelease,
		ASD_MusicBrainz
	};

	/**
	 * Constructor.
	 *
	 * @param parent        parent widget
	 * @param caption       dialog title
	 * @param trackDataList track data to be filled with imported values,
	 *                      is passed with durations of files set
	 */
	ImportDialog(QWidget* parent, QString& caption,
							 ImportTrackDataVector& trackDataList);
	/**
	 * Destructor.
	 */
	~ImportDialog();

	/**
	 * Set dialog to be started automatically.
	 *
	 * @param asd dialog to be started
	 */
	void setAutoStartSubDialog(AutoStartSubDialog asd);

	/**
	 * Clear dialog data.
	 */
	void clear();

	/**
	 * Get import destination.
	 *
	 * @return DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both.
	 */
	ImportConfig::ImportDestination getDestination() const;

	/**
	 * Set import destination.
	 *
	 * @param dest DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both
	 */
	void setDestination(ImportConfig::ImportDestination dest);

public slots:
	/**
	 * Shows the dialog as a modal dialog.
	 */
	int exec();

private slots:
	/**
	 * Show help.
	 */
	void showHelp();

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Called when the maximum time difference value is changed.
	 */
	void maxDiffChanged();

	/**
	 * Move a table row.
	 *
	 * The first parameter @a section is not used.
	 * @param fromIndex index of position moved from
	 * @param toIndex   index of position moved to
	 */
	void moveTableRow(int, int fromIndex, int toIndex);

	/**
	 * Import from server and preview in table.
	 */
	void fromServer();

	/**
	 * Import from text.
	 */
	void fromText();

	/**
	 * Show fields to import in text as preview in table.
	 */
	void showPreview();

	/**
	 * Match import data with length.
	 */
	void matchWithLength();

	/**
	 * Match import data with track number.
	 */
	void matchWithTrack();

	/**
	 * Match import data with title.
	 */
	void matchWithTitle();

	/**
	 * Import from freedb.org and preview in table.
	 */
	void fromFreedb();

	/**
	 * Import from TrackType.org and preview in table.
	 */
	void fromTrackType();

	/**
	 * Import from MusicBrainz and preview in table.
	 */
	void fromMusicBrainz();

	/**
	 * Import from MusicBrainz release database and preview in table.
	 */
	void fromMusicBrainzRelease();

	/**
	 * Import from www.discogs.com and preview in table.
	 */
	void fromDiscogs();

	/**
	 * Import from www.amazon.com and preview in table.
	 */
	void fromAmazon();

	/**
	 * Hide subdialogs.
	 */
	void hideSubdialogs();

private:
	/**
	 * Get time difference check configuration.
	 *
	 * @param enable  true if check is enabled
	 * @param maxDiff maximum allowed time difference
	 */
	void getTimeDifferenceCheck(bool& enable, int& maxDiff) const;

	/**
	 * Display dialog with import source.
	 *
	 * @param source import source
	 */
	void displayImportSourceDialog(ServerImporter* source);

	AutoStartSubDialog m_autoStartSubDialog;
	/** Preview table */
	QTableView* m_trackDataTable;
	/** Track data model */
	TrackDataModel* m_trackDataModel;
	/** combobox with import servers */
	QComboBox* m_serverComboBox;
	/** combobox with import destinations */
	QComboBox* m_destComboBox;
	QCheckBox* m_mismatchCheckBox;
	QSpinBox* m_maxDiffSpinBox;
	/** freedb.org importer */
	FreedbImporter* m_freedbImporter;
	/** TrackType.org importer */
	TrackTypeImporter* m_trackTypeImporter;
	/** MusicBrainz import dialog */
	MusicBrainzDialog* m_musicBrainzDialog;
	/** MusicBrainz release importer */
	MusicBrainzReleaseImporter* m_musicBrainzReleaseImporter;
	/** Discogs importer */
	DiscogsImporter* m_discogsImporter;
	/** Amazon importer */
	AmazonImporter* m_amazonImporter;
	/** Server import dialog */
	ServerImportDialog* m_serverImportDialog;
	/** Text import dialog */
	TextImportDialog* m_textImportDialog;
	/** track data */
	ImportTrackDataVector& m_trackDataVector;
};

#endif
