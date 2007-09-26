/**
 * \file rendirdialog.h
 * Rename directory dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2004
 *
 * Copyright (C) 2004-2007  Urs Fleisch
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

#ifndef RENDIRDIALOG_H
#define RENDIRDIALOG_H

#include <qdialog.h>
#include <qstring.h>

class QComboBox;
class QLabel;
class TaggedFile;

/**
 * Rename directory dialog.
 */
class RenDirDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent     parent widget
	 * @param caption    dialog title
	 * @param taggedFile file to use for rename preview
	 */
	RenDirDialog(QWidget* parent, const QString& caption,
							 TaggedFile* taggedFile);

	/**
	 * Destructor.
	 */
	~RenDirDialog();

	/**
	 * Get new directory name.
	 *
	 * @return new directory name.
	 */
	void setNewDirname(const QString& dir);

	/**
	 * Get new directory name.
	 *
	 * @return new directory name.
	 */
	QString getNewDirname() const;

	/**
	 * Generate new directory name according to current settings.
	 *
	 * @param taggedFile file to get information from
	 * @param olddir pointer to QString to place old directory name into,
	 *               NULL if not used
	 *
	 * @return new directory name.
	 */
	QString generateNewDirname(TaggedFile* taggedFile, QString* olddir);

	/**
	 * Perform renaming or creation of directory according to current settings.
	 *
	 * @param taggedFile file to take directory from and to move
	 * @param again    is set true if the new directory (getNewDirname())
	 *                 should be read and processed again
	 * @param errorMsg if not NULL and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 *
	 * @return true if other files can be processed,
	 *         false if operation is finished.
	 */
	bool performAction(TaggedFile* taggedFile, bool& again, QString* errorMsg);

private slots:
	/**
	 * Set new directory name according to current settings.
	 */
	void slotUpdateNewDirname();

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
	 * Create a directory if it does not exist.
	 *
	 * @param dir      directory path
	 * @param errorMsg if not NULL and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 *
	 * @return true if directory exists or was created successfully.
	 */
	bool createDirectory(const QString& dir,
						 QString* errorMsg) const;

	/**
	 * Rename a directory.
	 *
	 * @param olddir   old directory name
	 * @param newdir   new directory name
	 * @param errorMsg if not NULL and an error occurred, a message is
	 *                 appended here, otherwise it is not touched
	 *
	 * @return true if rename successful.
	 */
	bool renameDirectory(
		const QString& olddir, const QString& newdir, QString* errorMsg) const;

	/**
	 * Rename a file.
	 *
	 * @param oldfn    old file name
	 * @param newfn    new file name
	 * @param errorMsg if not NULL and an error occurred, a message is
	 *                 appended here, otherwise it is not touched
	 *
	 * @return true if rename successful or newfn already exists.
	 */
	bool renameFile(const QString& oldfn, const QString& newfn,
									QString* errorMsg) const;

	enum Action { ActionRename = 0, ActionCreate = 1 };
	enum TagVersion { TagV2V1 = 0, TagV1 = 1, TagV2 = 2 };

	QComboBox* m_formatComboBox;
	QComboBox* m_actionComboBox;
	QComboBox* m_tagversionComboBox;
	QLabel* m_currentDirLabel;
	QLabel* m_newDirLabel;
	TaggedFile* m_taggedFile;
};

#endif
