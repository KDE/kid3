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

#include "config.h"
#include "importtrackdata.h"
#include "importconfig.h"
#include <qdialog.h>

class ImportSelector;
class FreedbConfig;
class QCheckBox;
class QSpinBox;
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

	/**
	 * Set the format lineedits.
	 *
	 * @param index format index
	 */
	void setFormatLineEdit(int index);

	/**
	 * Import from a file.
	 *
	 * @param fn file name
	 *
	 * @return true if ok.
	 */
	bool importFromFile(const QString& fn);

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

private:
	AutoStartSubDialog m_autoStartSubDialog;
	/** import selector widget */
	ImportSelector* m_impsel;
	ImportTrackDataVector& m_trackDataVector;
};

#endif
