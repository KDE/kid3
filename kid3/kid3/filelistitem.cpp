/**
 * \file filelistitem.cpp
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

#include "filelistitem.h"
#include "filelist.h"
#include "taggedfile.h"
#include "dirinfo.h"
#include "kid3.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfileinfo.h>

/** Empty pixmap, will be allocated in constructor */
QPixmap* FileListItem::nullPixmap = 0;
/** Pixmap for modified file, will be allocated in constructor */
QPixmap* FileListItem::modifiedPixmap = 0;
/** Pixmap for V1V2, will be allocated in constructor */
QPixmap* FileListItem::v1v2Pixmap = 0;
/** Pixmap for V1, will be allocated in constructor */
QPixmap* FileListItem::v1Pixmap = 0;
/** Pixmap for V2, will be allocated in constructor */
QPixmap* FileListItem::v2Pixmap = 0;
/** Pixmap for "no tag", will be allocated in constructor */
QPixmap* FileListItem::notagPixmap = 0;
/** Pixmap for closed folder, will be allocated in constructor */
QPixmap* FileListItem::folderClosedPixmap = 0;
/** Pixmap for open folder, will be allocated in constructor */
QPixmap* FileListItem::folderOpenPixmap = 0;

/* The bitmaps are stored here instead of using KDE bitmaps to make
   it work for the Qt only versions. */
/** picture for modified pixmap */
static const char* const modified_xpm[] = {
	"16 16 33 1",
	". c None",
	"B c None",
	"A c None",
	"C c None",
	"D c None",
	"E c None",
	"# c #000000",
	"b c #006562",
	"j c #414041",
	"x c #525552",
	"f c #529594",
	"e c #52959c",
	"w c #5a555a",
	"v c #626162",
	"u c #626562",
	"r c #737173",
	"p c #737573",
	"q c #7b757b",
	"o c #838183",
	"m c #838583",
	"z c #8b8d8b",
	"l c #949194",
	"k c #9c959c",
	"i c #a4a1a4",
	"h c #a4a5a4",
	"y c #b4b6b4",
	"g c #bdb6bd",
	"a c #c5c2c5",
	"s c #c5c6c5",
	"c c #cdc6cd",
	"t c #dedade",
	"n c #eeeaee",
	"d c #ffffff",
	".......##.......",
	"......#ab#......",
	".....#cbde#.....",
	"....#abdddf#....",
	"...#gbddddde#...",
	"..#hijddddddf#..",
	".#kjkljdddddd##.",
	"#mjnjmojddddjma#",
	"#jnpnjqrjddjqs#.",
	"#drtttjuvjjua#..",
	".#dasajjwxws#...",
	"..#dyjzljxa#...A",
	"...#jrrjws#...AB",
	"....#cjxa#...ACB",
	".....#cs#...ADE.",
	"......##...ABB.."
};

/** picture for empty pixmap */
static const char* const null_xpm[] = {
	"16 16 2 1",
	"# c None",
	". c None",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#."
};

/** picture with V1 and V2 */
static const char* const v1v2_xpm[] = {
	"16 16 3 1",
	"  c None",
	". c #FFFFFF",
	"+ c #000000",
	"                ",
	"  ..  ..   .    ",
	"  .+  .+  .+    ",
	"  .+  .+ .++    ",
	"   .+.+ .+.+    ",
	"   .+.+   .+    ",
	"    .+    .+    ",
	"                ",
	"  ..  ..  ..    ",
	"  .+  .+ .++.   ",
	"  .+  .+.+ .+   ",
	"   .+.+   .+    ",
	"   .+.+  .+..   ",
	"    .+  .++++   ",
	"                ",
	"                "};

/** picture with V1 */
static const char* const v1_xpm[] = {
	"16 16 3 1",
	"  c None",
	". c #FFFFFF",
	"+ c #000000",
	"                ",
	"  ..  ..   .    ",
	"  .+  .+  .+    ",
	"  .+  .+ .++    ",
	"   .+.+ .+.+    ",
	"   .+.+   .+    ",
	"    .+    .+    ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                "};

/** picture with V2 */
static const char* const v2_xpm[] = {
	"16 16 3 1",
	"  c None",
	". c #FFFFFF",
	"+ c #000000",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"  ..  ..  ..    ",
	"  .+  .+ .++.   ",
	"  .+  .+.+ .+   ",
	"   .+.+   .+    ",
	"   .+.+  .+..   ",
	"    .+  .++++   ",
	"                ",
	"                "};

/** picture with NO TAG */
static const char* const notag_xpm[] = {
	"16 16 3 1",
	"  c None",
	". c #FFFFFF",
	"+ c #000000",
	"                ",
	"  ..  ..  ..    ",
	"  .+. .+ .++.   ",
	"  .++..+.+ .+   ",
	"  .+.+.+.+ .+   ",
	"  .+ .++.+..+   ",
	"  .+  .+ .++    ",
	"                ",
	" ....  .   ..   ",
	" .+++ .+. .++   ",
	"  .+ .+.+.+ ..  ",
	"  .+ .+++.+.++  ",
	"  .+ .+.+.+..+  ",
	"  .+ .+.+ .++   ",
	"                ",
	"                "};

/** picture with closed folder */
static const char* folder_closed_xpm[]={
	"16 16 10 1",
	"  c #0E279A",
	". c #2852AF",
	"+ c #2B66D3",
	"@ c #2A83FE",
	"# c #627CB1",
	"$ c #5FA1FC",
	"% c #B0B2CA",
	"& c #89BEF9",
	"* c #CDE9FD",
	"= c #FCFFFC",
	"==++++++========",
	"=+======+=======",
	"+==*=*==#++++++=",
	"+**=*****=*=*=*+",
	".********+++++++",
	".*******+&====&+",
	".*&+++++&=&&&&&+",
	".&+$=*===$$$$$$+",
	".&+@$@$@$$$$$@$+",
	".&.@@@@@@@@@@@@ ",
	".& @@@@@@@@@@@@ ",
	".& ===========% ",
	".& ===========% ",
	".$ %%%%%%%%%%%% ",
	"=.             =",
	"================"};

/** picture with open folder */
static const char* folder_open_xpm[]={
	"16 16 10 1",
	"  c #0E279A",
	". c #2852AF",
	"+ c #2B66D3",
	"@ c #2A83FE",
	"# c #627CB1",
	"$ c #5FA1FC",
	"% c #B0B2CA",
	"& c #89BEF9",
	"* c #CDE9FD",
	"= c #FCFFFC",
	"==++++++========",
	"=+======+=======",
	"+==*=*==#++++++=",
	"+**=*****=*=*=*+",
	".********+++++++",
	".*******+&====*+",
	".*&+++++#=&&&&&+",
	".&+$=*==*$$$$$$+",
	".&+$$@$$@$$$$$@ ",
	".$+@@@@@@@@@@@@ ",
	"..+@@@@@@@@@@@+ ",
	".+%===========.=",
	". %==========% =",
	". %%%%%%%%%%%% =",
	"=              =",
	"================"};

/**
 * Constructor.
 *
 * @param parent parent file list
 * @param after  this item is inserted after item @a after
 * @param file   tagged file (will be owned by this item)
 */
FileListItem::FileListItem(FileList* parent, FileListItem* after,
													 TaggedFile* file) :
#if QT_VERSION >= 0x040000
	QTreeWidgetItem(parent, after),
#else
	QListViewItem(parent, after),
#endif
	m_file(file), m_dirInfo(0)
{
	init();
}

/**
 * Constructor for non top-level items.
 *
 * @param parent parent file list item
 * @param after  this item is inserted after item @a after
 * @param file   tagged file (will be owned by this item)
 */
FileListItem::FileListItem(FileListItem* parent, FileListItem* after,
													 TaggedFile* file) :
#if QT_VERSION >= 0x040000
	QTreeWidgetItem(parent, after), m_isOpen(false),
#else
	QListViewItem(parent, after),
#endif
	m_file(file), m_dirInfo(0)
{
	init();
}

/**
 * Initialize file list item.
 * Common initialization for all constructors.
 */
void FileListItem::init()
{
	setInSelection(false);
	if (m_file) {
		setText(0, m_file->getFilename());
	}

	// this two objects should be destructed when the program terminates.
	// static QPixmap objects are not possible:
	// "QPaintDevice: Must construct a QApplication before a QPaintDevice"
	if (!nullPixmap) {
		nullPixmap = new QPixmap((const char **)null_xpm);
	}
	if (!modifiedPixmap) {
		modifiedPixmap = new QPixmap((const char **)modified_xpm);
	}
	if (!v1v2Pixmap) {
		v1v2Pixmap = new QPixmap((const char **)v1v2_xpm);
	}
	if (!v1Pixmap) {
		v1Pixmap = new QPixmap((const char **)v1_xpm);
	}
	if (!v2Pixmap) {
		v2Pixmap = new QPixmap((const char **)v2_xpm);
	}
	if (!notagPixmap) {
		notagPixmap = new QPixmap((const char **)notag_xpm);
	}
	if (!folderClosedPixmap) {
		folderClosedPixmap = new QPixmap((const char **)folder_closed_xpm);
	}
	if (!folderOpenPixmap) {
		folderOpenPixmap = new QPixmap((const char **)folder_open_xpm);
	}

	updateIcons();
}

/**
 * Destructor.
 */
FileListItem::~FileListItem()
{
	delete m_file;
	delete m_dirInfo;
}

#if QT_VERSION >= 0x040000
/**
 * Opens or closes an item.
 *
 * @param o true to open
 */
void FileListItem::setOpen(bool o)
{
	if (m_dirInfo) {
		m_isOpen = o;
		setIcon(0, o ? *folderOpenPixmap : *folderClosedPixmap);

		if (o  && !childCount()) {
			treeWidget()->setUpdatesEnabled(false);
			FileList::readSubDirectory(m_dirInfo, this, 0);
			treeWidget()->setUpdatesEnabled(true);
		}
		updateIcons();
	}
}
#else
/**
 * Paints the contents of one column of an item.
 *
 * @param p      painter
 * @param cg     color group
 * @param column number of column
 * @param width  width
 * @param align  alignment
 */
void FileListItem::paintCell(QPainter* p, const QColorGroup& cg,
														 int column, int width, int align)
{
	if (Kid3App::s_miscCfg.m_markTruncations &&
			column == 0 && m_file && m_file->getTruncationFlags() != 0) {
		QColorGroup g(cg);
		g.setColor(QColorGroup::Base, Qt::red);
		QListViewItem::paintCell(p, g, column, width, align);
	} else {
		QListViewItem::paintCell(p, cg, column, width, align);
	}
}

/**
 * Opens or closes an item.
 *
 * @param o true to open
 */
void FileListItem::setOpen(bool o)
{
	if (m_dirInfo) {
		setPixmap(0, o ? *folderOpenPixmap : *folderClosedPixmap);

		if (o && !childCount()) {
			listView()->setUpdatesEnabled(false);
			FileList::readSubDirectory(m_dirInfo, this, 0);
			listView()->setUpdatesEnabled(true);
		}
		updateIcons();
	}
	QListViewItem::setOpen(o);
}

/**
 * Called before showing the item.
 */
void FileListItem::setup()
{
	if (!m_file) {
		setExpandable(true);
	}
	QListViewItem::setup();
}
#endif

/**
 * Set tagged file.
 * The item takes ownership of this file and the old file is deleted.
 *
 * @param file tagged file.
 */
void FileListItem::setFile(TaggedFile* file)
{
	if (m_file) {
		delete m_file;
	}
	m_file = file;
	if (m_file) {
		setText(0, m_file->getFilename());
	}
	updateIcons();
}

/**
 * Set directory information.
 * An item can represent a file (file is set) or
 * a directory (directory information is set).
 * The item takes ownership of this directory information
 * and the old information is deleted.
 *
 * @param dirInfo directory information
 */
void FileListItem::setDirInfo(DirInfo* dirInfo)
{
	if (m_dirInfo) {
		delete m_dirInfo;
	}
	m_dirInfo = dirInfo;
	if (m_dirInfo) {
		QFileInfo fi(m_dirInfo->getDirname());
		setText(0, fi.fileName());
		setOpen(false);
	}
}

/**
 * Update the icons according to the modificaton state and the tags present.
 */
void FileListItem::updateIcons()
{
	if (m_file) {
		if (m_file->isChanged()) {
			QCM_setIcon(0, *modifiedPixmap);
		} else {
			static const QPixmap* tagpm[] = {
				notagPixmap, v1Pixmap, v2Pixmap, v1v2Pixmap, nullPixmap
			};
			int tagpmIdx;
			if (!m_file->isTagInformationRead()) {
				tagpmIdx = 4;
			} else {
				tagpmIdx = 0;
				if (m_file->hasTagV1()) {
					tagpmIdx |= 1;
				}
				if (m_file->hasTagV2()) {
					tagpmIdx |= 2;
				}
			}
			QCM_setIcon(0, *tagpm[tagpmIdx]);
		}
#if QT_VERSION >= 0x040200
		if (Kid3App::s_miscCfg.m_markTruncations &&
				m_file->getTruncationFlags() != 0) {
			setBackground(0, Qt::red);
		} else {
			setBackground(0, Qt::NoBrush);
		}
#elif QT_VERSION >= 0x040000
		if (Kid3App::s_miscCfg.m_markTruncations &&
				m_file->getTruncationFlags() != 0) {
			setBackgroundColor(0, Qt::red);
		} else {
			setBackgroundColor(0, QColor());
		}
#endif
	}
}

/**
 * Update the text according to the file name.
 */
void FileListItem::updateText()
{
	if (m_file) {
		setText(0, m_file->getFilename());
	}
}

/**
 * Set directory name.
 * Sets a new directory name if the item represents a directory.
 *
 * @param dirName new directory name
 */
void FileListItem::setDirName(const QString& dirName)
{
	if (m_dirInfo) {
		m_dirInfo->setDirname(dirName);
		QFileInfo fi(dirName);
		setText(0, fi.fileName());
		setOpen(false);
	}
}
