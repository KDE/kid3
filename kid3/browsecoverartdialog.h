/**
 * \file browsecoverartdialog.h
 * Browse cover art dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11-Jan-2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#ifndef BROWSECOVERARTDIALOG_H
#define BROWSECOVERARTDIALOG_H

#include <qdialog.h>
#include <qstringlist.h>
#include "frame.h"

class QTextEdit;
class QLineEdit;
class QComboBox;
class ExternalProcess;
class ConfigTable;

/**
 * Browse cover art dialog.
 */
class BrowseCoverArtDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	BrowseCoverArtDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	virtual ~BrowseCoverArtDialog();

	/**
	 * Read the local settings from the configuration.
	 */
	void readConfig();

	/**
	 * Set frames for which picture has to be found.
	 *
	 * @param frames track data
	 */
	void setFrames(const FrameCollection& frames);

	/**
	 * Get the URL of an image file.
	 * The input URL is transformed using the match picture URL table to
	 * get the URL of an image file.
	 *
	 * @param url URL from image drag
	 *
	 * @return URL of image file, empty if no image URL found.
	 */
	static QString getImageUrl(const QString& url);

public slots:
	/**
	 * Hide modal dialog, start browse command.
	 */
	virtual void accept();

	/**
	 * Set the source lineedits to the source selected in the combo box.
	 *
	 * @param index current index of the combo box
	 */
	void setSourceLineEdit(int index);

private slots:
	/**
	 * Show browse command as preview.
	 */
	void showPreview();

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

private:
	/**
	 * Set the source combo box and line edits from the configuration.
	 */
	void setSourceFromConfig();

	/** Text editor with command preview */
	QTextEdit* m_edit;
	/** Combobox with artist */
	QLineEdit* m_artistLineEdit;
	/** Combobox with album */
	QLineEdit* m_albumLineEdit;
	/** Combobox with sources */
	QComboBox* m_sourceComboBox;
	/** LineEdit for URL */
	QLineEdit* m_urlLineEdit;
	/** Table to extract picture URL */
	ConfigTable* m_matchUrlTable;
	/** URLs */
	QStringList m_urls;
	/** Formatted URL */
	QString m_url;

	/** Track data */
	FrameCollection m_frames;
	/** Process for browser command */
	ExternalProcess* m_process;
};

#endif
