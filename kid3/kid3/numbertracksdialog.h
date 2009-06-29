/**
 * \file numbertracksdialog.h
 * Number tracks dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 May 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#ifndef NUMBERTRACKSDIALOG_H
#define NUMBERTRACKSDIALOG_H

#include <qdialog.h>

class QSpinBox;
class QComboBox;

/**
 * Number tracks dialog.
 */
class NumberTracksDialog : public QDialog {
Q_OBJECT
public:
	/** Destinations */
	enum Destination { DestV1, DestV2, DestV1V2 };

	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	NumberTracksDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	~NumberTracksDialog();

	/**
	 * Get start number.
	 */
	int getStartNumber() const;

	/**
	 * Get destination.
	 *
	 * @return DestV1, DestV2 or DestV1V2 if ID3v1, ID2v2 or both are destination
	 */
	Destination getDestination() const;

private slots:
	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

private:
	/** spinbox with starting track number */
	QSpinBox* m_trackSpinBox;
	/** combobox with destination */
	QComboBox* m_destComboBox;
};

#endif
