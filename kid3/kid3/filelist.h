/**
 * \file filelist.h
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#ifndef FILELIST_H
#define FILELIST_H

#include <qstring.h>
#include <qsize.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QTreeWidget>
typedef QTreeWidget FileListBaseClass;
#else
#include <qlistview.h>
typedef QListView FileListBaseClass;
class QAction;
#endif
#include "dirinfo.h"

class FileListItem;
class TaggedFile;
class ExternalProcess;

/**
 * List of files to operate on.
 */
class FileList : public FileListBaseClass
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 */
	FileList(QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~FileList();

	/**
	 * Returns the recommended size for the widget.
	 * @return recommended size.
	 */
	virtual QSize sizeHint() const;

	/**
	 * Get the first item in the filelist.
	 *
	 * @return first file.
	 */
	FileListItem* first();

	/**
	 * Get the next item in the filelist.
	 *
	 * @return next file.
	 */
	FileListItem* next();

	/**
	 * Get the first item in the the current directory.
	 *
	 * @return first file.
	 */
	FileListItem* firstInDir();

	/**
	 * Get the next item in the current directory.
	 *
	 * @return next file.
	 */
	FileListItem* nextInDir();

	/**
	 * Get the number of files selected in the filelist.
	 *
	 * @return number of files selected.
	 */
	int numFilesSelected();

	/**
	 * Select the next file.
	 */
	void selectNextFile();

	/**
	 * Select the previous file.
	 */
	void selectPreviousFile();

	/**
	 * Fill the filelist with the files found in a directory.
	 *
	 * @param name path of directory
	 * @return false if name is not directory path, else true.
	 */
	bool readDir(const QString& name);

	/**
	 * Refresh text of all files in listview and check if any file is modified.
	 *
	 * @return true if a file is modified.
	 */
	bool updateModificationState();

	/**
	 * Create a TaggedFile subclass depending on the file extension.
	 *
	 * @param di directory information
	 * @param fn filename
	 *
	 * @return tagged file, 0 if no type found.
	 */
	static TaggedFile* createTaggedFile(const DirInfo* di, const QString& fn);

	/**
	 * Fill the filelist with the files found in the directory tree.
	 *
	 * @param dirInfo  information  about directory
	 * @param item     parent directory item or 0 if top-level
	 * @param listView parent list view if top-level, else 0
	 */
	static void readSubDirectory(DirInfo* dirInfo, FileListItem* item,
															 FileList* listView);

signals:
	/**
	 * Emitted when some of the selected files have been renamed.
	 */
	void selectedFilesRenamed();

private slots:
	/**
	 * Display a context menu with operations for selected files.
	 *
	 * @param item list box item
	 * @param pos  position where context menu is drawn on screen
	 */
#if QT_VERSION >= 0x040000
	void contextMenu(QTreeWidgetItem* item, const QPoint& pos);
#else
	void contextMenu(QListViewItem* item, const QPoint& pos);
#endif

	/**
	 * Execute a context menu command.
	 *
	 * @param id command ID
	 */
	void executeContextCommand(int id);

	/**
	 * Execute a context menu action.
	 *
	 * @param action action of selected menu
	 */
	void executeAction(QAction* action);

	/**
	 * Rename the selected file(s).
	 */
	void renameFile();

	/**
	 * Delete the selected file(s).
	 */
	void deleteFile();

	/**
	 * Expand an item.
	 *
	 * @param item item
	 */
	void expandItem(QTreeWidgetItem* item);

	/**
	 * Collapse an item.
	 *
	 * @param item item
	 */
	void collapseItem(QTreeWidgetItem* item);

	/**
	 * Display a custom context menu with operations for selected files.
	 *
	 * @param pos  position where context menu is drawn on screen
	 */
	void customContextMenu(const QPoint& pos);

	/**
	 * Expand or collapse an item which has no children.
	 *
	 * @param item item
	 */
	void expandOrCollapseEmptyItem(QTreeWidgetItem* item);

private:
	FileList(const FileList&);
	FileList& operator=(const FileList&);

	/**
	 * Format a string list from the selected files.
	 * Supported format fields:
	 * Those supported by StandardTags::formatString(),
	 * when prefixed with u, encoded as URL
	 * %f filename
	 * %F list of files
	 * %uf URL of single file
	 * %uF list of URLs
	 * %d directory name
	 * %b the web browser set in the configuration
	 *
	 * @todo %f and %F are full paths, which is inconsistent with the
	 * export format strings but compatible with .desktop files.
	 * %d is duration in export format.
	 * The export codes should be changed.
	 *
	 * @param format format specification
	 *
	 * @return formatted string list.
	 */
	QStringList formatStringList(const QStringList& format);

	/** information about directory */
	DirInfo m_dirInfo;
	/** iterator pointing to current file */
#if QT_VERSION >= 0x040000
	QTreeWidgetItemIterator* m_iterator;
#else
	QListViewItemIterator m_iterator;
#endif
	/** current item in current directory */
	FileListItem* m_currentItemInDir;
	/** Process for context menu commands */
	ExternalProcess* m_process;
};

#endif // FILELIST_H
