/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#if QT_VERSION >= 0x040000
#include <Q3PopupMenu>
#include <Q3Process>
#include <Q3Header>
#else
#include <qpopupmenu.h>
#include <qprocess.h>
#include <qheader.h>
#endif

#include "taggedfile.h"
#include "filelist.h"
#include "filelistitem.h"
#include "kid3.h"
#include "qtcompatmac.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.h"
#endif
#ifdef HAVE_FLAC
#include "flacfile.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

/**
 * Constructor.
 */
FileList::FileList(QWidget* parent, const char* name, Qt::WFlags f) :
	Q3ListView(parent, name, f), m_currentItemInDir(0), m_process(0)
{
	setSelectionMode(Extended);
	setSorting(-1);
	addColumn("");
	header()->hide();
#if QT_VERSION >= 0x040000
	connect(this, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
			this, SLOT(contextMenu(Q3ListViewItem*, const QPoint&)));
#else
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
			this, SLOT(contextMenu(QListViewItem*, const QPoint&)));
#endif
}

/**
 * Destructor.
 */
FileList::~FileList()
{
}

/**
 * Returns the recommended size for the widget.
 * @return recommended size.
 */
QSize FileList::sizeHint() const
{
	return QSize(fontMetrics().maxWidth() * 25, Q3ListView::sizeHint().height());
}

/**
 * Get the next item in the filelist which contains a file.
 *
 * @param it list view iterator
 *
 * @return next item with file.
 */
static FileListItem* getNextItemWithFile(Q3ListViewItemIterator& it)
{
	Q3ListViewItem* lvItem;
	while ((lvItem = it.current()) != 0) {
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
	m_iterator = Q3ListViewItemIterator(this);
	return getNextItemWithFile(m_iterator);
}

/**
 * Get the next item in the filelist.
 *
 * @return next file.
 */
FileListItem* FileList::next()
{
	if (m_iterator.current()) {
		++m_iterator;
		return getNextItemWithFile(m_iterator);
	}
	return 0;
}

/**
 * Get the next item in the current directory which contains a file.
 *
 * @param lvItem list view item
 *
 * @return next item with file.
 */
static FileListItem* getNextItemWithFileInDir(Q3ListViewItem* lvItem)
{
	while (lvItem) {
		FileListItem* flItem = dynamic_cast<FileListItem*>(lvItem);
		if (flItem && flItem->getFile()) {
			return flItem;
		}
		lvItem = lvItem->nextSibling();
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
	// try to get the currently selected directory
	Q3ListViewItemIterator it(this, Q3ListViewItemIterator::Selected);
	FileListItem* item = dynamic_cast<FileListItem*>(it.current());
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
		it = Q3ListViewItemIterator(this);
		m_currentItemInDir = getNextItemWithFile(it);
	}

	return m_currentItemInDir;
}

/**
 * Get the next item in the current directory.
 *
 * @return next file.
 */
FileListItem* FileList::nextInDir()
{
	if (m_currentItemInDir) {
		m_currentItemInDir =
			getNextItemWithFileInDir(m_currentItemInDir->nextSibling());
	}
	return m_currentItemInDir;
}

/**
 * Get the number of files selected in the filelist.
 *
 * @return number of files selected.
 */
int FileList::numFilesSelected()
{
	int numSelected = 0;
	Q3ListViewItemIterator it(this, Q3ListViewItemIterator::Selected);
	FileListItem* item;
	while ((item = getNextItemWithFile(it)) != 0) {
		++numSelected;
		++it;
	}
	return numSelected;
}

/**
 * Select the next file.
 */
void FileList::selectNextFile()
{
	Q3ListViewItem* item = currentItem();
	if (item && (item = item->itemBelow()) != 0) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);		
	}
}

/**
 * Select the previous file.
 */
void FileList::selectPreviousFile()
{
	Q3ListViewItem* item = currentItem();
	if (item && (item = item->itemAbove()) != 0) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);		
	}
}

/**
 * Fill the filelist with the files found in the directory tree.
 *
 * @param dirInfo  information  about directory
 * @param item     parent directory item or 0 if top-level
 * @param listView parent list view if top-level, else 0
 */
void FileList::readSubDirectory(DirInfo* dirInfo, FileListItem* item,
																FileList* listView)
{
	if (!dirInfo) return;
	QString dirname = dirInfo->getDirname();
	int numFiles = 0;
	FileListItem* last = 0;
	QDir dir(dirname);
	QStringList dirContents = dir.entryList(QDir::Dirs) +
		dir.entryList(Kid3App::s_miscCfg.nameFilter, QDir::Files);
	for (QStringList::Iterator it = dirContents.begin();
			 it != dirContents.end(); ++it) {
		QString filename = dirname + QDir::separator() + *it;
		if (!QFileInfo(filename).isDir()) {
			TaggedFile* taggedFile = 0;
#ifdef HAVE_VORBIS
			if ((*it).right(4).lower() == ".ogg")
				taggedFile = new OggFile(dirInfo, *it);
			else
#endif
#ifdef HAVE_FLAC
				if ((*it).right(5).lower() == ".flac")
					taggedFile = new FlacFile(dirInfo, *it);
				else
#endif
#ifdef HAVE_ID3LIB
					if ((*it).right(4).lower() == ".mp3"
#ifdef HAVE_TAGLIB
							&& Kid3App::s_miscCfg.m_id3v2Version != MiscConfig::ID3v2_4_0
#endif
						)
						taggedFile = new Mp3File(dirInfo, *it);
					else
#endif
#ifdef HAVE_TAGLIB
						taggedFile = new TagLibFile(dirInfo, *it);
#else
			;
#endif
			if (taggedFile) {
				if (item) {
					last = new FileListItem(item, last, taggedFile);
				} else if (listView) {
					last = new FileListItem(listView, last, taggedFile);
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
				}
			}
		}
	}
	dirInfo->setNumFiles(numFiles);
}

/**
 * Fill the filelist with the files found in a directory.
 *
 * @param name path of directory
 * @return false if name is not directory path, else true.
 */
bool FileList::readDir(const QString& name)
{
	QFileInfo file(name);
	if(file.isDir()) {
		clear();
		m_dirInfo.setDirname(file.absFilePath());
		readSubDirectory(&m_dirInfo, 0, this);
		return true;
	}
	return false;
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
	triggerUpdate();
	return modified;
}

/**
 * Display a context menu with operations for selected files.
 *
 * @param item list box item
 * @param pos  position where context menu is drawn on screen
 */
void FileList::contextMenu(Q3ListViewItem* item, const QPoint& pos)
{
	if (item && !Kid3App::s_miscCfg.m_contextMenuCommands.empty()) {
		Q3PopupMenu menu(this);
		int id = 0;
		for (QStringList::const_iterator
					 it = Kid3App::s_miscCfg.m_contextMenuCommands.begin();
				 it != Kid3App::s_miscCfg.m_contextMenuCommands.end();
				 ++it) {
			QString cmd = *it;
			if (cmd[0] == '!') {
				cmd = cmd.mid(1);
			}
			menu.insertItem(cmd, this, SLOT(executeContextCommand(int)), 0, id);
			++id;			
		}
		menu.setMouseTracking(true);
		menu.exec(pos);
	}
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
		QString cmd = Kid3App::s_miscCfg.m_contextMenuCommands[id];
		bool confirm = false;
		if (cmd[0] == '!') {
			cmd = cmd.mid(1);
			confirm = true;
		}
		args.push_back(cmd);
		FileListItem* item = first();
		while (item != 0) {
			if (item->isInSelection()) {
				args.push_back(item->getFile()->getAbsFilename());
			}
			item = next();
		}

		if (args.size() > 1) {
			if (confirm &&
#if QT_VERSION >= 0x030100
					QMessageBox::question
#else
					QMessageBox::warning
#endif
          (
						this, i18n("Execute Command"),
						i18n("Execute ") + args.join(" ") + "?",
						QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {
				return;
			}
			if (!m_process) {
				m_process = new Q3Process(this);
			}
			if (m_process) {
				m_process->setArguments(args);
				m_process->launch(QString(""));
			}
		}
	}
}
