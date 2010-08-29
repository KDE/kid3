/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2008  Urs Fleisch
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

#include "config.h"
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qurl.h>
#include <qregexp.h>
#if QT_VERSION >= 0x040000
#include <QMenu>
#include <QHeaderView>
#if QT_VERSION >= 0x040300
#include <QDirIterator>
#endif
#else
#include <qpopupmenu.h>
#include <qheader.h>
#endif
#ifdef CONFIG_USE_KDE
#include <kdeversion.h>
#include <kmessagebox.h>
#endif

#include "taggedfile.h"
#include "filelist.h"
#include "filelistitem.h"
#include "kid3.h"
#include "externalprocess.h"
#include "qtcompatmac.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.hpp"
#endif
#ifdef HAVE_FLAC
#include "flacfile.hpp"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_1 I18N_NOOP("Do you really want to delete these %1 items?")
/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_2 I18N_NOOP("Error while deleting these %1 items:")

/**
 * Replaces context command format codes in a string.
 */
class CommandFormatReplacer : public FrameFormatReplacer {
public:
	/**
	 * Constructor.
	 *
	 * @param frames frame collection
	 * @param str    string with format codes
	 * @param files  file list
	 * @param isDir  true if directory
	 */
	explicit CommandFormatReplacer(
		const FrameCollection& frames, const QString& str,
		const QStringList& files, bool isDir);

	/**
	 * Destructor.
	 */
	virtual ~CommandFormatReplacer();

	/**
	 * Get help text for supported format codes.
	 *
	 * @param onlyRows if true only the tr elements are returned,
	 *                 not the surrounding table
	 *
	 * @return help text.
	 */
	static QString getToolTip(bool onlyRows = false);

protected:
	/**
	 * Replace a format code (one character %c or multiple characters %{chars}).
	 * Supported format fields:
	 * Those supported by FrameFormatReplacer::getReplacement()
	 * %f %{file} filename
	 * %d %{directory} directory name
	 * %b %{browser} the web browser set in the configuration
	 *
	 * @param code format code
	 *
	 * @return replacement string,
	 *         QString::null if code not found.
	 */
	virtual QString getReplacement(const QString& code) const;

private:
	const QStringList& m_files;
	const bool m_isDir;
};


/**
 * Constructor.
 *
 * @param frames frame collection
 * @param str    string with format codes
 * @param files  file list
 * @param isDir  true if directory
 */
CommandFormatReplacer::CommandFormatReplacer(
	const FrameCollection& frames, const QString& str,
	const QStringList& files, bool isDir) :
	FrameFormatReplacer(frames, str), m_files(files), m_isDir(isDir) {}

/**
 * Destructor.
 */
CommandFormatReplacer::~CommandFormatReplacer() {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement()
 * %f %{file} filename
 * %d %{directory} directory name
 * %b %{browser} the web browser set in the configuration
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString CommandFormatReplacer::getReplacement(const QString& code) const
{
	QString result = FrameFormatReplacer::getReplacement(code);
	if (result.isNull()) {
		QString name;

		if (code.length() == 1) {
			static const struct {
				char shortCode;
				const char* longCode;
			} shortToLong[] = {
				{ 'f', "file" },
				{ 'd', "directory" },
				{ 'b', "browser" }
			};
#if QT_VERSION >= 0x040000
			const char c = code[0].toLatin1();
#else
			const char c = code[0].latin1();
#endif
			for (unsigned i = 0; i < sizeof(shortToLong) / sizeof(shortToLong[0]); ++i) {
				if (shortToLong[i].shortCode == c) {
					name = shortToLong[i].longCode;
					break;
				}
			}
		} else if (code.length() > 1) {
			name = code;
		}

		if (!name.isNull()) {
			if (name == "file") {
				result = m_files.front();
			} else if (name == "directory") {
				result = m_files.front();
				if (!m_isDir) {
					int sepPos = result.QCM_lastIndexOf('/');
					if (sepPos < 0) {
						sepPos = result.QCM_lastIndexOf(QDir::separator());
					}
					if (sepPos >= 0) {
						result.truncate(sepPos);
					}
				}
			} else if (name == "browser") {
				result = Kid3App::s_miscCfg.m_browser;
			} else if (name == "url") {
				if (!m_files.empty()) {
					QUrl url;
					url.QCM_setScheme("file");
					url.QCM_setPath(m_files.front());
					result = url.toString(
#if QT_VERSION < 0x040000
						true
#endif
						);
				}
			}
		}
	}

	return result;
}

/**
 * Get help text for supported format codes.
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString CommandFormatReplacer::getToolTip(bool onlyRows)
{
	QString str;
	if (!onlyRows) str += "<table>\n";
	str += FrameFormatReplacer::getToolTip(true);

	str += "<tr><td>%f</td><td>%{file}</td><td>";
	str += QCM_translate("Filename");
	str += "</td></tr>\n";

	str += "<tr><td>%F</td><td>%{files}</td><td>";
	str += QCM_translate(I18N_NOOP("Filenames"));
	str += "</td></tr>\n";

	str += "<tr><td>%uf</td><td>%{url}</td><td>";
	str += QCM_translate("URL");
	str += "</td></tr>\n";

	str += "<tr><td>%uF</td><td>%{urls}</td><td>";
	str += QCM_translate(I18N_NOOP("URLs"));
	str += "</td></tr>\n";

	str += "<tr><td>%d</td><td>%{directory}</td><td>";
	str += QCM_translate(I18N_NOOP("Directory name"));
	str += "</td></tr>\n";

	str += "<tr><td>%b</td><td>%{browser}</td><td>";
	str += QCM_translate("Browser");
	str += "</td></tr>\n";

	str += "<tr><td>%ua...</td><td>%u{artist}...</td><td>";
	str += QCM_translate(I18N_NOOP("Encode as URL"));
	str += "</td></tr>\n";

	if (!onlyRows) str += "</table>\n";
	return str;
}


/**
 * Constructor.
 * @param parent parent widget
 * @param app    application widget
 */
FileList::FileList(QWidget* parent, Kid3App* app) :
#if QT_VERSION >= 0x040000
	QTreeWidget(parent), m_iterator(0),
#else
	QListView(parent),
#endif
	m_currentItemInDir(0), m_process(0), m_app(app)
{
#if QT_VERSION >= 0x040000
	setSelectionMode(ExtendedSelection);
	setSortingEnabled(false);
	setColumnCount(1);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(customContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
					this, SLOT(expandItem(QTreeWidgetItem*)));
	connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
					this, SLOT(collapseItem(QTreeWidgetItem*)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
					this, SLOT(expandOrCollapseEmptyItem(QTreeWidgetItem*)));
#else
	setSelectionMode(Extended);
	setSorting(-1);
	addColumn("");
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
			this, SLOT(contextMenu(QListViewItem*, const QPoint&)));
#endif
	header()->hide();
}

/**
 * Destructor.
 */
FileList::~FileList()
{
	delete m_process;
#if QT_VERSION >= 0x040000
	delete m_iterator;
#endif
}

/**
 * Returns the recommended size for the widget.
 * @return recommended size.
 */
QSize FileList::sizeHint() const
{
	return QSize(fontMetrics().maxWidth() * 25,
#if QT_VERSION >= 0x040000
							 QTreeWidget::sizeHint().height()
#else
							 QListView::sizeHint().height()
#endif
		);
}

/**
 * Get the next item in the filelist which contains a file.
 *
 * @param it list view iterator
 *
 * @return next item with file.
 */
static FileListItem* getNextItemWithFile(
#if QT_VERSION >= 0x040000
	QTreeWidgetItemIterator& it
#else
	QListViewItemIterator& it
#endif
	)
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* lvItem;
#else
	QListViewItem* lvItem;
#endif
	while ((lvItem = *it) != 0) {
		FileListItem* flItem = dynamic_cast<FileListItem*>(lvItem);
		if (flItem && flItem->getFile()) {
			return flItem;
		}
		++it;
	}
	return 0;
}

/**
 * Get the first item in the filelist.
 *
 * @return first file.
 */
FileListItem* FileList::first()
{
#if QT_VERSION >= 0x040000
	delete m_iterator;
	if (topLevelItemCount() > 0) {
		m_iterator = new QTreeWidgetItemIterator(this);
		return getNextItemWithFile(*m_iterator);
	} else {
		m_iterator = 0;
		return 0;
	}
#else
	m_iterator = QListViewItemIterator(this);
	return getNextItemWithFile(m_iterator);
#endif
}

/**
 * Get the next item in the filelist.
 *
 * @return next file.
 */
FileListItem* FileList::next()
{
#if QT_VERSION >= 0x040000
	if (m_iterator && **m_iterator) {
		++*m_iterator;
		return getNextItemWithFile(*m_iterator);
	}
#else
	if (*m_iterator) {
		++m_iterator;
		return getNextItemWithFile(m_iterator);
	}
#endif
	return 0;
}

/**
 * Get the current item in the filelist.
 *
 * @return current file.
 */
FileListItem* FileList::current()
{
	return dynamic_cast<FileListItem*>(currentItem());
}

/**
 * Get the next item in the current directory which contains a file.
 *
 * @param lvItem list view item
 *
 * @return next item with file.
 */
static FileListItem* getNextItemWithFileInDir(
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* lvItem
#else
	QListViewItem* lvItem
#endif
	)
{
	while (lvItem) {
		FileListItem* flItem = dynamic_cast<FileListItem*>(lvItem);
		if (flItem && flItem->getFile()) {
			return flItem;
		}
#if QT_VERSION >= 0x040000
		QTreeWidgetItem* parent = lvItem->parent();
		QTreeWidgetItemIterator it(lvItem);
		lvItem = *(++it);
		if (lvItem && lvItem->parent() != parent) {
			lvItem = 0;
		}
#else
		lvItem = lvItem->nextSibling();
#endif
	}
	return 0;
}

/**
 * Get the first item in the the current directory.
 *
 * @return first file.
 */
FileListItem* FileList::firstInDir()
{
#if QT_VERSION >= 0x040000
	// try to get the currently selected directory
	if (topLevelItemCount() <= 0) {
		m_currentItemInDir = 0;
		return 0;
	}
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
	FileListItem* item = dynamic_cast<FileListItem*>(*it);
	if (item &&
		 (item->getDirInfo() ||
			(item->getFile() &&
			 (item = dynamic_cast<FileListItem*>(item->parent())) != 0))) {
		m_currentItemInDir =
			getNextItemWithFileInDir(item->child(0));
	} else {
		// if not found, get the first child of the list view
		m_currentItemInDir =
			getNextItemWithFileInDir(topLevelItem(0));
	}

	// if still not found, get the first file in the list view
	if (!m_currentItemInDir) {
		it = QTreeWidgetItemIterator(this);
		m_currentItemInDir = getNextItemWithFile(it);
	}

	return m_currentItemInDir;
#else
	// try to get the currently selected directory
	QListViewItemIterator it(this, QListViewItemIterator::Selected);
	FileListItem* item = dynamic_cast<FileListItem*>(*it);
	if (item &&
		 (item->getDirInfo() ||
			(item->getFile() &&
			 (item = dynamic_cast<FileListItem*>(item->parent())) != 0))) {
		m_currentItemInDir =
			getNextItemWithFileInDir(item->firstChild());
	} else {
		// if not found, get the first child of the list view
		m_currentItemInDir =
			getNextItemWithFileInDir(firstChild());
	}

	// if still not found, get the first file in the list view
	if (!m_currentItemInDir) {
		it = QListViewItemIterator(this);
		m_currentItemInDir = getNextItemWithFile(it);
	}

	return m_currentItemInDir;
#endif
}

/**
 * Get the next item in the current directory.
 *
 * @return next file.
 */
FileListItem* FileList::nextInDir()
{
	if (m_currentItemInDir) {
#if QT_VERSION >= 0x040000
		QTreeWidgetItem* parent = m_currentItemInDir->parent();
		QTreeWidgetItemIterator it(m_currentItemInDir);
		QTreeWidgetItem* nextSibling = *(++it);
		if (nextSibling && nextSibling->parent() != parent) {
			nextSibling = 0;
		}
		m_currentItemInDir = getNextItemWithFileInDir(nextSibling);
#else
		m_currentItemInDir =
			getNextItemWithFileInDir(m_currentItemInDir->nextSibling());
#endif
	}
	return m_currentItemInDir;
}

/**
 * Get the next item in the filelist which contains a file or a directory.
 *
 * @param it list view iterator
 *
 * @return next item with file or directory.
 */
static FileListItem* getNextItemWithFileOrDir(
#if QT_VERSION >= 0x040000
	QTreeWidgetItemIterator& it
#else
	QListViewItemIterator& it
#endif
	)
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* lvItem;
#else
	QListViewItem* lvItem;
#endif
	while ((lvItem = *it) != 0) {
		FileListItem* flItem = dynamic_cast<FileListItem*>(lvItem);
		if (flItem) {
			return flItem;
		}
		++it;
	}
	return 0;
}

/**
 * Get the first file or directory item in the filelist.
 *
 * @return first file.
 */
FileListItem* FileList::firstFileOrDir()
{
#if QT_VERSION >= 0x040000
	delete m_iterator;
	if (topLevelItemCount() > 0) {
		m_iterator = new QTreeWidgetItemIterator(this);
		return getNextItemWithFileOrDir(*m_iterator);
	} else {
		m_iterator = 0;
		return 0;
	}
#else
	m_iterator = QListViewItemIterator(this);
	return getNextItemWithFileOrDir(m_iterator);
#endif
}

/**
 * Get the next file or directory item in the filelist.
 *
 * @return next file.
 */
FileListItem* FileList::nextFileOrDir()
{
#if QT_VERSION >= 0x040000
	if (m_iterator && **m_iterator) {
		++*m_iterator;
		return getNextItemWithFileOrDir(*m_iterator);
	}
#else
	if (*m_iterator) {
		++m_iterator;
		return getNextItemWithFileOrDir(m_iterator);
	}
#endif
	return 0;
}

/**
 * Get the number of files selected in the filelist.
 *
 * @return number of files selected.
 */
int FileList::numFilesSelected()
{
	int numSelected = 0;
#if QT_VERSION >= 0x040000
	if (topLevelItemCount() <= 0) {
		return 0;
	}
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
#else
	QListViewItemIterator it(this, QListViewItemIterator::Selected);
#endif
	FileListItem* item;
	while ((item = getNextItemWithFile(it)) != 0) {
		++numSelected;
		++it;
	}
	return numSelected;
}

/**
 * Get the number of files or directories selected in the filelist.
 *
 * @return number of files or directories selected.
 */
int FileList::numFilesOrDirsSelected()
{
	int numSelected = 0;
#if QT_VERSION >= 0x040000
	if (topLevelItemCount() <= 0) {
		return 0;
	}
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
#else
	QListViewItemIterator it(this, QListViewItemIterator::Selected);
#endif
	FileListItem* item;
	while ((item = getNextItemWithFileOrDir(it)) != 0) {
		++numSelected;
		++it;
	}
	return numSelected;
}

/**
 * Select the first file.
 *
 * @return true if a file exists.
 */
bool FileList::selectFirstFile()
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* item = *QTreeWidgetItemIterator(this);
	if (item) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
		return true;
	}
#else
	QListViewItem* item = firstChild();
	if (item) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);
		return true;
	}
#endif
	return false;
}

/**
 * Select the next file.
 *
 * @return true if a next file exists.
 */
bool FileList::selectNextFile()
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* item = currentItem();
	if (item && (item = *(++QTreeWidgetItemIterator(item))) != 0) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
		return true;
	}
#else
	QListViewItem* item = currentItem();
	if (item && (item = item->itemBelow()) != 0) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);
		return true;
	}
#endif
	return false;
}

/**
 * Select the previous file.
 *
 * @return true if a previous file exists.
 */
bool FileList::selectPreviousFile()
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* item = currentItem();
	if (item && (item = *(--QTreeWidgetItemIterator(item))) != 0) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
		return true;
	}
#else
	QListViewItem* item = currentItem();
	if (item && (item = item->itemAbove()) != 0) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);
		return true;
	}
#endif
	return false;
}

/**
 * Fill the filelist with the files found in the directory tree.
 *
 * @param dirInfo  information  about directory
 * @param item     parent directory item or 0 if top-level
 * @param listView parent list view if top-level, else 0
 * @param fileName name of file to select (optional, else empty)
 */
void FileList::readSubDirectory(DirInfo* dirInfo, FileListItem* item,
																FileList* listView, const QString& fileName)
{
	if (!dirInfo) return;
	QString dirname = dirInfo->getDirname();
	int numFiles = 0;
	FileListItem* last = 0;
	QDir dir(dirname);
#if QT_VERSION >= 0x040000
	const QDir::Filters dirFilters =
		QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files;
	QStringList nameFilters(Kid3App::s_miscCfg.m_nameFilter.split(' '));
	QStringList dirContents = dir.entryList(
		nameFilters, dirFilters, QDir::DirsFirst | QDir::IgnoreCase);
#else
	QStringList dirContents = dir.entryList(QDir::Dirs) +
		dir.entryList(Kid3App::s_miscCfg.m_nameFilter, QDir::Files);
#endif
	for (QStringList::Iterator it = dirContents.begin();
			 it != dirContents.end(); ++it) {
		QString filename = dirname + QDir::separator() + *it;
		if (!QFileInfo(filename).isDir()) {
			TaggedFile* taggedFile = TaggedFile::createFile(dirInfo, *it);
			if (taggedFile) {
				if (item) {
					last = new FileListItem(item, last, taggedFile);
				} else if (listView) {
					last = new FileListItem(listView, last, taggedFile);
				}
				if (!fileName.isEmpty() && fileName == *it && last && listView) {
					listView->clearSelection();
					listView->setCurrentItem(last);
#if QT_VERSION >= 0x040000
					listView->setItemSelected(last, true);
#else
					listView->setSelected(last, true);
					listView->ensureItemVisible(last);
#endif
				}
				++numFiles;
			}
		} else {
			if (*it != "." && *it != "..") {
				if (item) {
					last = new FileListItem(item, last, 0);
				} else if (listView) {
					last = new FileListItem(listView, last, 0);
				}
				if (last) {
					last->setDirInfo(new DirInfo(filename));
#if QT_VERSION >= 0x040300
					last->setChildIndicatorPolicy(
						QDirIterator(filename, nameFilters, dirFilters).hasNext() ?
						QTreeWidgetItem::ShowIndicator :
						QTreeWidgetItem::DontShowIndicatorWhenChildless);
#endif
				}
			}
		}
	}
	dirInfo->setNumFiles(numFiles);
}

/**
 * Fill the filelist with the files found in a directory.
 *
 * @param name     path of directory
 * @param fileName name of file to select (optional, else empty)
 *
 * @return false if name is not directory path, else true.
 */
bool FileList::readDir(const QString& name, const QString& fileName)
{
	QFileInfo file(name);
	if(file.isDir()) {
		clear();
		m_dirInfo.setDirname(file.QCM_absoluteFilePath());
		readSubDirectory(&m_dirInfo, 0, this, fileName);
		return true;
	}
	return false;
}

/**
 * Fill the filelist with the files from a DirContents tree.
 *
 * @param dirContents recursive information about directory and files
 * @param item        parent directory item or 0 if top-level
 * @param listView    parent list view if top-level, else 0
 */
static void setSubDirectoryFromDirContents(
	const DirContents& dirContents, FileListItem* item, FileList* listView)
{
	const DirInfo* dirInfo =
		listView ? listView->getDirInfo() : item->getDirInfo();
	QString dirname = dirContents.getDirname();
	FileListItem* last = 0;
	for (DirContents::DirContentsList::const_iterator it =
				 dirContents.getDirs().begin();
			 it != dirContents.getDirs().end(); ++it) {
		if (item) {
			last = new FileListItem(item, last, 0);
		} else if (listView) {
			last = new FileListItem(listView, last, 0);
		}
		if (last) {
			const DirContents& subDirContents = *(*it);
			last->setDirInfo(
				new DirInfo(subDirContents.getDirname(), subDirContents.getNumFiles()));
			setSubDirectoryFromDirContents(subDirContents, last, 0);
#if QT_VERSION >= 0x040000
			last->setExpanded(true);
#else
			last->setOpen(true);
#endif
		}
	}
	for (QStringList::const_iterator it = dirContents.getFiles().begin();
			 it != dirContents.getFiles().end(); ++it) {
		QString filename = dirname + QDir::separator() + *it;
		TaggedFile* taggedFile = TaggedFile::createFile(dirInfo, *it);
		if (taggedFile) {
			if (item) {
				last = new FileListItem(item, last, taggedFile);
			} else if (listView) {
				last = new FileListItem(listView, last, taggedFile);
			}
		}
	}
}

/**
 * Fill the filelist with the files from a DirContents tree.
 *
 * @param dirContents recursive information about directory and files
 */
void FileList::setFromDirContents(const DirContents& dirContents)
{
	clear();
	m_dirInfo.setDirname(dirContents.getDirname());
	m_dirInfo.setNumFiles(dirContents.getNumFiles());
	setSubDirectoryFromDirContents(dirContents, 0, this);
}

/**
 * Refresh text of all files in listview and check if any file is modified.
 *
 * @return true if a file is modified.
 */
bool FileList::updateModificationState()
{
	FileListItem* item = first();
	bool modified = false;
	while (item != 0) {
		if (item->getFile()->isChanged()) {
			modified = true;
		}
		item->updateIcons();
		item = next();
	}
#if QT_VERSION >= 0x040000
	update();
#else
	triggerUpdate();
#endif
	return modified;
}

/**
 * Display a context menu with operations for selected files.
 *
 * @param item list box item
 * @param pos  position where context menu is drawn on screen
 */
void FileList::contextMenu(
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* item,
#else
	QListViewItem* item,
#endif
	const QPoint& pos)
{
	if (item && !Kid3App::s_miscCfg.m_contextMenuCommands.empty()) {
#if QT_VERSION >= 0x040000
		QMenu menu(this);
		menu.addAction(i18n("&Expand all"), this, SLOT(expandAll()));
		menu.addAction(i18n("&Collapse all"), this, SLOT(collapseAll()));
		menu.addAction(i18n("&Rename"), this, SLOT(renameFile()));
		menu.addAction(i18n("&Delete"), this, SLOT(deleteFile()));
#ifdef HAVE_PHONON
		menu.addAction(i18n("&Play"), m_app, SLOT(slotPlayAudio()));
#endif
		int id = 0;
		for (MiscConfig::MenuCommandList::const_iterator
					 it = Kid3App::s_miscCfg.m_contextMenuCommands.begin();
				 it != Kid3App::s_miscCfg.m_contextMenuCommands.end();
				 ++it) {
			menu.addAction((*it).getName());
			++id;
		}
		connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
#else
		QPopupMenu menu(this);
		menu.insertItem(i18n("&Expand all"), this, SLOT(expandAll()));
		menu.insertItem(i18n("&Collapse all"), this, SLOT(collapseAll()));
		menu.insertItem(i18n("&Rename"), this, SLOT(renameFile()));
		menu.insertItem(i18n("&Delete"), this, SLOT(deleteFile()));
#ifdef HAVE_PHONON
		menu.insertItem(i18n("&Play"), m_app, SLOT(slotPlayAudio()));
#endif
		int id = 0;
		for (MiscConfig::MenuCommandList::const_iterator
					 it = Kid3App::s_miscCfg.m_contextMenuCommands.begin();
				 it != Kid3App::s_miscCfg.m_contextMenuCommands.end();
				 ++it) {
			menu.insertItem((*it).getName(), this, SLOT(executeContextCommand(int)), 0, id);
			++id;
		}
#endif
		menu.setMouseTracking(true);
		menu.exec(pos);
	}
}

/**
 * Format a string list from the selected files.
 * Supported format fields:
 * Those supported by FrameFormatReplacer::getReplacement(),
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
QStringList FileList::formatStringList(const QStringList& format)
{
	QStringList files;
	FileListItem* firstSelectedItem = 0;
	FileListItem* item = first();
	while (item != 0) {
		if (item->isInSelection()) {
			if (!firstSelectedItem) {
				firstSelectedItem = item;
			}
			files.push_back(item->getFile()->getAbsFilename());
		}
		item = next();
	}

	const DirInfo* dirInfo = 0;
	if (files.empty() &&
			(item = dynamic_cast<FileListItem*>(currentItem())) != 0 &&
			(dirInfo = item->getDirInfo()) != 0) {
		files.push_back(dirInfo->getDirname());
		firstSelectedItem = firstInDir();
	}

	FrameCollection frames;
	QStringList fmt;
	for (QStringList::const_iterator it = format.begin();
			 it != format.end();
			 ++it) {
		if ((*it).QCM_indexOf('%') == -1) {
			fmt.push_back(*it);
		} else {
			if (*it == "%F" || *it == "%{files}") {
				// list of files
				fmt += files;
			} else if (*it == "%uF" || *it == "%{urls}") {
				// list of URLs or URL
				QUrl url;
				url.QCM_setScheme("file");
				for (QStringList::const_iterator fit = files.begin();
						 fit != files.end();
						 ++fit) {
					url.QCM_setPath(*fit);
					fmt.push_back(url.toString());
				}
			} else {
				if (firstSelectedItem) {
					// use merged tags 1 and 2 to format string
					FrameCollection frames1;
					firstSelectedItem->getFile()->getAllFramesV1(frames1);
					firstSelectedItem->getFile()->getAllFramesV2(frames);
					frames.merge(frames1);
				}
				QString str(*it);
				str.replace("%uf", "%{url}");
				CommandFormatReplacer cfr(frames, str, files, dirInfo != 0);
				cfr.replacePercentCodes(FrameFormatReplacer::FSF_SupportUrlEncode);
				fmt.push_back(cfr.getString());
			}
		}
	}
	return fmt;
}

/**
 * Get help text for format codes supported by formatStringList().
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString FileList::getFormatToolTip(bool onlyRows)
{
	return CommandFormatReplacer::getToolTip(onlyRows);
}

/**
 * Execute a context menu command.
 *
 * @param id command ID
 */
void FileList::executeContextCommand(int id)
{
	if (id < static_cast<int>(Kid3App::s_miscCfg.m_contextMenuCommands.size())) {
		QStringList args;
		const MiscConfig::MenuCommand& menuCmd = Kid3App::s_miscCfg.m_contextMenuCommands[id];
		QString cmd = menuCmd.getCommand();

		int len = cmd.length();
		int begin;
		int end = 0;
		while (end < len) {
			begin = end;
			while (begin < len && cmd[begin] == ' ') ++begin;
			if (begin >= len) break;
			if (cmd[begin] == '"') {
				++begin;
				QString str;
				while (begin < len) {
					if (cmd[begin] == '\\' && begin + 1 < len &&
							(cmd[begin + 1] == '\\' ||
							 cmd[begin + 1] == '"')) {
						++begin;
					} else if (cmd[begin] == '"') {
						break;
					}
					str += cmd[begin];
					++begin;
				}
				args.push_back(str);
				end = begin;
			} else {
				end = cmd.QCM_indexOf(' ', begin + 1);
				if (end == -1) end = len;
				args.push_back(cmd.mid(begin, end - begin));
			}
			++end;
		}

		args = formatStringList(args);

		if (!m_process) {
			m_process = new ExternalProcess(this);
		}
		if (m_process) {
			m_process->launchCommand(menuCmd.getName(), args, menuCmd.mustBeConfirmed(), menuCmd.outputShown());
		}
	}
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
#if QT_VERSION >= 0x040000
void FileList::executeAction(QAction* action)
{
	if (action) {
		QString name = action->text().remove('&');
		int id = 0;
		for (MiscConfig::MenuCommandList::const_iterator
					 it = Kid3App::s_miscCfg.m_contextMenuCommands.begin();
				 it != Kid3App::s_miscCfg.m_contextMenuCommands.end();
				 ++it) {
			if (name == (*it).getName()) {
				executeContextCommand(id);
				break;
			}
			++id;
		}
	}
}
#else
void FileList::executeAction(QAction*) {}
#endif

/**
 * Expand or collapse all folders.
 *
 * @param expand true to expand, false to collapse
 */
void FileList::setAllExpanded(bool expand)
{
	FileListItem* item = firstFileOrDir();
	while (item != 0) {
#if QT_VERSION >= 0x040000
		item->setExpanded(expand);
#else
		item->setOpen(expand);
#endif
		item = nextFileOrDir();
	}
}

/**
 * Expand all folders.
 */
void FileList::expandAll()
{
	setAllExpanded(true);
}

/**
 * Collapse all folders.
 */
void FileList::collapseAll()
{
	setAllExpanded(false);
}

/**
 * Rename the selected file(s).
 */
void FileList::renameFile()
{
	bool fileSelected = false;
	bool fileRenamed = false;
	FileListItem* item = first();
	TaggedFile* taggedFile;
	while (item != 0) {
		if (item->isInSelection() &&
				(taggedFile = item->getFile()) != 0) {
			bool ok;
			QString newFileName = QInputDialog::QCM_getText(
				this,
				i18n("Rename File"),
				i18n("Enter new file name:"),
				QLineEdit::Normal, taggedFile->getFilename(), &ok);
			if (ok && !newFileName.isEmpty()) {
				if (taggedFile->isChanged()) {
					taggedFile->setFilename(newFileName);
					fileRenamed = true;
				} else {
					QString newPath = taggedFile->getDirname() + '/' + newFileName;
					const DirInfo* dirInfo = taggedFile->getDirInfo();
					QString absFilename = taggedFile->getAbsFilename();
					QString filename = taggedFile->getFilename();
					// This will close the file.
					// The file must be closed before renaming on Windows.
					item->setFile(0);
					if (QDir().rename(absFilename, newPath)) {
						TaggedFile* newTaggedFile =
							TaggedFile::createFile(dirInfo, newFileName);
						if (newTaggedFile) {
							item->setFile(newTaggedFile);
							fileRenamed = true;
						}
					} else {
						item->setFile(TaggedFile::createFile(dirInfo, filename));
						QMessageBox::warning(
							0, i18n("File Error"),
							i18n("Error while renaming:\n") +
							KCM_i18n2("Rename %1 to %2 failed\n", filename, newFileName),
							QMessageBox::Ok, QCM_NoButton);
					}
				}
			}
			fileSelected = true;
		}
		item = next();
	}
	if (fileRenamed) {
		emit selectedFilesRenamed();
	}

	const DirInfo* dirInfo;
	if (!fileSelected &&
			(item = dynamic_cast<FileListItem*>(currentItem())) != 0 &&
			(dirInfo = item->getDirInfo()) != 0) {
		QFileInfo fi(dirInfo->getDirname());
		bool ok;
		QString newDirName = QInputDialog::QCM_getText(
			this,
			i18n("Rename Directory"),
			i18n("Enter new directory name:"),
			QLineEdit::Normal, fi.fileName(), &ok);
		if (ok && !newDirName.isEmpty()) {
#if QT_VERSION >= 0x040000
			QString newPath = fi.dir().path() + '/' + newDirName;
#else
			QString newPath = fi.dirPath() + '/' + newDirName;
#endif
			if (QDir().rename(dirInfo->getDirname(), newPath)) {
				item->setDirName(newPath);
			} else {
				QMessageBox::warning(
					0, i18n("File Error"),
					i18n("Error while renaming:\n") +
					KCM_i18n2("Rename %1 to %2 failed\n", fi.fileName(), newDirName),
					QMessageBox::Ok, QCM_NoButton);
			}
		}
	}
}

/**
 * Delete the selected file(s).
 */
void FileList::deleteFile()
{
	QStringList files;
	FileListItem* item = first();
	while (item != 0) {
		if (item->isInSelection()) {
			files.push_back(item->getFile()->getAbsFilename());
		}
		item = next();
	}

	unsigned numFiles = files.size();
	if (numFiles > 0) {
#ifdef CONFIG_USE_KDE
		if (KMessageBox::warningContinueCancelList(
					this,
#if KDE_VERSION >= 0x035c00
					i18np("Do you really want to delete this item?",
								"Do you really want to delete these %1 items?", numFiles),
					files,
					i18n("Delete Files"),
					KStandardGuiItem::del(), KStandardGuiItem::cancel(), QString(),
#else
					i18n("Do you really want to delete this item?",
							 "Do you really want to delete these %n items?", numFiles),
					files,
					i18n("Delete Files"),
					KStdGuiItem::del(), QString::null,
#endif
					KMessageBox::Dangerous) == KMessageBox::Continue)
#else
		QString txt = numFiles > 1 ?
			KCM_i18n1("Do you really want to delete these %1 items?", numFiles) :
			i18n("Do you really want to delete this item?");
		txt += '\n';
		txt += files.join("\n");
		if (
#if QT_VERSION >= 0x030100
			QMessageBox::question
#else
			QMessageBox::warning
#endif
			(
				this, i18n("Delete Files"), txt,
				QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
#endif
		{
#if QT_VERSION >= 0x040000
			QList<QTreeWidgetItem*> itemsToDelete;
#endif
			files.clear();
			FileListItem* item = first();
			while (item != 0) {
				FileListItem* nextItem = next();
				if (item->isInSelection()) {
					TaggedFile* taggedFile = item->getFile();
					if (taggedFile) {
						const DirInfo* dirInfo = taggedFile->getDirInfo();
						QString absFilename = taggedFile->getAbsFilename();
						QString filename = taggedFile->getFilename();
						// This will close the file.
						// The file must be closed before deleting on Windows.
						item->setFile(0);
						if (QDir().remove(absFilename)) {
#if QT_VERSION >= 0x040000
							itemsToDelete.append(item);
#else
							delete item;
#endif
						} else {
							item->setFile(TaggedFile::createFile(dirInfo, filename));
							files.push_back(absFilename);
						}
					}
				}
				item = nextItem;
			}
#if QT_VERSION >= 0x040000
			qDeleteAll(itemsToDelete);
			itemsToDelete.clear();
#endif
			if (!files.empty()) {
#ifdef CONFIG_USE_KDE
				KMessageBox::errorList(
					0,
#if KDE_VERSION >= 0x035c00
					i18np("Error while deleting this item:",
								"Error while deleting these %1 items:", files.size()),
#else
					i18n("Error while deleting this item:",
							 "Error while deleting these %n items:", files.size()),
#endif
					files,
					i18n("File Error"));
#else
				QString txt = files.size() > 1 ?
					KCM_i18n1("Error while deleting these %1 items:", files.size()) :
					i18n("Error while deleting this item:");
				txt += '\n';
				txt += files.join("\n");
				QMessageBox::warning(
					0, i18n("File Error"), txt,
					QMessageBox::Ok, QCM_NoButton);
#endif
			}
		}
	}

	const DirInfo* dirInfo;
	if (numFiles == 0 &&
			(item = dynamic_cast<FileListItem*>(currentItem())) != 0 &&
			(dirInfo = item->getDirInfo()) != 0) {
		if (
#ifdef CONFIG_USE_KDE
			KMessageBox::warningContinueCancelList(
				this,
				i18n("Do you really want to delete this item?"),
				QStringList(dirInfo->getDirname()),
				i18n("Delete Files"),
#if KDE_VERSION >= 0x035c00
				KStandardGuiItem::del(), KStandardGuiItem::cancel(), QString(),
#else
				KStdGuiItem::del(), QString::null,
#endif
				KMessageBox::Dangerous) == KMessageBox::Continue
#else
#if QT_VERSION >= 0x030100
			QMessageBox::question
#else
			QMessageBox::warning
#endif
			(
				this, i18n("Delete Files"),
				i18n("Do you really want to delete this item?") + '\n' + dirInfo->getDirname(),
				QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok
#endif
			) {
			if (QDir().rmdir(dirInfo->getDirname())) {
				delete item;
			} else {
#ifdef CONFIG_USE_KDE
				KMessageBox::errorList(
					0, i18n("Directory must be empty.\n") +
					i18n("Error while deleting this item:"),
					QStringList(dirInfo->getDirname()),
					i18n("File Error"));
#else
				QMessageBox::warning(
					0, i18n("File Error"),
					i18n("Directory must be empty.\n") +
					i18n("Error while deleting this item:") + '\n' + dirInfo->getDirname(),
					QMessageBox::Ok, QCM_NoButton);
#endif
			}
		}
	}
}

#if QT_VERSION >= 0x040000
/**
 * Expand an item.
 *
 * @param item item
 */
void FileList::expandItem(QTreeWidgetItem* item)
{
	FileListItem* fli = dynamic_cast<FileListItem*>(item);
	if (fli) {
		fli->setOpen(true);
	}
}

/**
 * Collapse an item.
 *
 * @param item item
 */
void FileList::collapseItem(QTreeWidgetItem* item)
{
	FileListItem* fli = dynamic_cast<FileListItem*>(item);
	if (fli) {
		fli->setOpen(false);
	}
}

/**
 * Display a custom context menu with operations for selected files.
 *
 * @param pos  position where context menu is drawn on screen
 */
void FileList::customContextMenu(const QPoint& pos)
{
	contextMenu(currentItem(), mapToGlobal(pos));
}

/**
 * Expand or collapse an item which has no children.
 *
 * @param item item
 */
void FileList::expandOrCollapseEmptyItem(QTreeWidgetItem* item)
{
	FileListItem* fli = dynamic_cast<FileListItem*>(item);
	if (fli && fli->getDirInfo() && !item->childCount()) {
		fli->setOpen(!fli->isOpen());
	}
}
#else
void FileList::expandItem(QTreeWidgetItem*) {}
void FileList::collapseItem(QTreeWidgetItem*) {}
void FileList::customContextMenu(const QPoint&) {}
void FileList::expandOrCollapseEmptyItem(QTreeWidgetItem*) {}
#endif
