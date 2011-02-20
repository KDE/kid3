/**
 * \file filelistitem.h
 * Item in filelist.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 02 Oct 2006
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

#ifndef FILELISTITEM_H
#define FILELISTITEM_H

#include <qpixmap.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QTreeWidget>
#else
#include <qlistview.h>
#endif

class QPainter;
class TaggedFile;
class DirInfo;
class FileList;

/** List box item containing tagged file */
class FileListItem : public
#if QT_VERSION >= 0x040000
QTreeWidgetItem
#else
QListViewItem
#endif
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent file list
	 * @param after  this item is inserted after item @a after
	 * @param file   tagged file (will be owned by this item)
	 */
	FileListItem(FileList* parent, FileListItem* after, TaggedFile* file);

	/**
	 * Constructor for non top-level items.
	 *
	 * @param parent parent file list item
	 * @param after  this item is inserted after item @a after
	 * @param file   tagged file (will be owned by this item)
	 */
	FileListItem(FileListItem* parent, FileListItem* after, TaggedFile* file);

	/**
	 * Destructor.
	 */
	virtual ~FileListItem();

#if QT_VERSION < 0x040000
	/**
	 * Paints the contents of one column of an item.
	 *
   * @param p      painter
   * @param cg     color group
   * @param column number of column
   * @param width  width
   * @param align  alignment
   */
	virtual void paintCell(QPainter* p, const QColorGroup& cg,
												 int column, int width, int align);

	/**
	 * Called before showing the item.
	 */
	virtual void setup();
#endif

	/**
	 * Opens or closes an item.
	 *
	 * @param o true to open
	 */
	virtual void setOpen(bool o);

	/**
	 * Get tagged file.
	 * @return tagged file.
	 */
	TaggedFile* getFile() { return m_file; }

	/**
	 * Set tagged file.
	 * The item takes ownership of this file and the old file is deleted.
	 *
	 * @param file tagged file.
	 */
	void setFile(TaggedFile* file);
 
	/**
	 * Set directory information.
	 * An item can represent a file (file is set) or
	 * a directory (directory information is set).
	 * The item takes ownership of this directory information
	 * and the old information is deleted.
	 *
	 * @param dirInfo directory information
	 */
	void setDirInfo(DirInfo* dirInfo);

	/**
	 * Get directory information.
	 * Is only available if the item represents a directory.
	 *
	 * @return directory information.
	 */
	const DirInfo* getDirInfo() const { return m_dirInfo; }

	/**
	 * Set directory name.
	 * Sets a new directory name if the item represents a directory.
	 *
	 * @param dirName new directory name
	 */
	void setDirName(const QString& dirName);

	/**
	 * Update the icons according to the modificaton state and the tags present.
	 */
	void updateIcons();

	/**
	 * Update the text according to the file name.
	 */
	void updateText();

	/**
	 * Mark file as selected.
	 *
	 * @param val true to set file selected.
	 */
	void setInSelection(bool val) {
#if QT_VERSION >= 0x040000
		setSelected(val);
#else
		m_selected = val;
#endif
	}

	/**
	 * Check if file is marked selected.
	 *
	 * @return true if file is marked selected.
	 */
	bool isInSelection() {
#if QT_VERSION >= 0x040000
		return isSelected();
#else
		return m_selected;
#endif
	}

#if QT_VERSION >= 0x040000
	/**
	 * Check if directory is open.
	 * @return true if open.
	 */
	bool isOpen() const { return m_isOpen; }
#endif

private:
	FileListItem(const FileListItem&);
	FileListItem& operator=(const FileListItem&);

	/**
	 * Initialize file list item.
	 * Common initialization for all constructors.
	 */
	void init();

#if QT_VERSION >= 0x040000
	/** true if directory open. */
	bool m_isOpen;
#endif

	/** the tagged file represented by this item */
	TaggedFile* m_file;

	/** information about directory if item represents directory */
	DirInfo* m_dirInfo;

#if QT_VERSION < 0x040000
	/** true if file is marked selected */
	bool m_selected;
#endif

	/** pointer to pixmap for modified file */
	static QPixmap* modifiedPixmap;
	/** pointer to empty pixmap */
	static QPixmap* nullPixmap;
	/** pointer to V1V2 pixmap */
	static QPixmap* v1v2Pixmap;
	/** pointer to V1 pixmap */
	static QPixmap* v1Pixmap;
	/** pointer to V2 pixmap */
	static QPixmap* v2Pixmap;
	/** pointer to "no tag" pixmap */
	static QPixmap* notagPixmap;
	/** pointer to folder closed pixmap */
	static QPixmap* folderClosedPixmap;
	/** pointer to folder open pixmap */
	static QPixmap* folderOpenPixmap;
};

#endif // FILELISTITEM_H
