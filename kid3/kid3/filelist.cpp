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
#include <qinputdialog.h>
#include <qurl.h>
#include <qregexp.h>
#if QT_VERSION >= 0x040000
#include <Q3PopupMenu>
#include <Q3Process>
#include <Q3Header>
#else
#include <qpopupmenu.h>
#include <qprocess.h>
#include <qheader.h>
#endif
#ifdef CONFIG_USE_KDE
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
	delete m_process;
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
 * Create a TaggedFile subclass depending on the file extension.
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if no type found.
 */
TaggedFile* FileList::createTaggedFile(const DirInfo* di, const QString& fn)
{
	TaggedFile* taggedFile = 0;
#ifdef HAVE_VORBIS
	if (fn.right(4).lower() == ".ogg")
		taggedFile = new OggFile(di, fn);
	else
#endif
#ifdef HAVE_FLAC
		if (fn.right(5).lower() == ".flac")
			taggedFile = new FlacFile(di, fn);
		else
#endif
#ifdef HAVE_ID3LIB
			if (fn.right(4).lower() == ".mp3"
#ifdef HAVE_TAGLIB
			&& Kid3App::s_miscCfg.m_id3v2Version != MiscConfig::ID3v2_4_0
#endif
		)
		taggedFile = new Mp3File(di, fn);
			else
#endif
#ifdef HAVE_TAGLIB
		taggedFile = new TagLibFile(di, fn);
#else
	;
#endif
	return taggedFile;
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
		dir.entryList(Kid3App::s_miscCfg.m_nameFilter, QDir::Files);
	for (QStringList::Iterator it = dirContents.begin();
			 it != dirContents.end(); ++it) {
		QString filename = dirname + QDir::separator() + *it;
		if (!QFileInfo(filename).isDir()) {
			TaggedFile* taggedFile = createTaggedFile(dirInfo, *it);
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
		menu.insertItem(i18n("&Rename"), this, SLOT(renameFile()));
		menu.insertItem(i18n("&Delete"), this, SLOT(deleteFile()));
		int id = 0;
		for (Q3ValueList<MiscConfig::MenuCommand>::const_iterator
					 it = Kid3App::s_miscCfg.m_contextMenuCommands.begin();
				 it != Kid3App::s_miscCfg.m_contextMenuCommands.end();
				 ++it) {
			menu.insertItem((*it).getName(), this, SLOT(executeContextCommand(int)), 0, id);
			++id;
		}
		menu.setMouseTracking(true);
		menu.exec(pos);
	}
}

/**
 * Format a string list from the selected files.
 * Supported format fields:
 * Those supported by StandardTags::formatString()
 * %f filename
 * %F list of files
 * %u URL of single file
 * %U list of URLs
 * %d directory name
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

	QStringList fmt;
	for (QStringList::const_iterator it = format.begin();
			 it != format.end();
			 ++it) {
		if ((*it).find('%') == -1) {
			fmt.push_back(*it);
		} else {
			if (*it == "%F") {
				// list of files
				fmt += files;
			} else if (*it == "%U") {
				// list of URLs
				QUrl url;
				url.setProtocol("file");
				for (QStringList::const_iterator fit = files.begin();
						 fit != files.end();
						 ++fit) {
					url.setFileName(*fit);
					fmt.push_back(url.toString(
#if QT_VERSION < 0x040000
													true
#endif
													));
				}
			} else {
				const int numTagCodes = 3;
				const QChar tagCode[numTagCodes] = { 'f', 'u', 'd' };
				QString tagStr[numTagCodes];
				if (!files.empty()) {
					tagStr[0] = files.front();
					QUrl url;
					url.setFileName(tagStr[0]);
					url.setProtocol("file");
					tagStr[1] = url.toString(
#if QT_VERSION < 0x040000
						true
#endif
						);
					tagStr[2] = tagStr[0];
					if (!dirInfo) {
						int sepPos = tagStr[2].findRev('/');
						if (sepPos < 0) {
							sepPos = tagStr[2].findRev(QDir::separator());
						}
						if (sepPos >= 0) {
							tagStr[2].truncate(sepPos);
						}
					}
				}
				QString str = StandardTags::replacePercentCodes(*it, tagCode, tagStr, numTagCodes);
				if (firstSelectedItem) {
					// use merged tags 1 and 2 to format string
					StandardTags st1, st2;
					firstSelectedItem->getFile()->getStandardTagsV1(&st1);
					firstSelectedItem->getFile()->getStandardTagsV2(&st2);
					st2.merge(st1);
					str = st2.formatString(str);
				}
				fmt.push_back(str);
			}
		}
	}
	return fmt;
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

		QRegExp rx("(\".*\"|\\S+)");
		int pos = 0, lastPos = -1;
		while (pos >= 0 && pos > lastPos) {
			pos = rx.search(cmd, pos);
			if (pos >= 0) {
				args.push_back(rx.cap(1));
				lastPos = pos;
				pos += rx.matchedLength();
			}
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
			QString newFileName = QInputDialog::getText(
				i18n("Rename File"), i18n("Enter new file name:"), QLineEdit::Normal,
				taggedFile->getFilename(), &ok, this);
			if (ok && !newFileName.isEmpty()) {
				if (taggedFile->isChanged()) {
					taggedFile->setFilename(newFileName);
					fileRenamed = true;
				} else {
					QString newPath = taggedFile->getDirname() + '/' + newFileName;
					if (QDir().rename(taggedFile->getAbsFilename(), newPath)) {
						TaggedFile* newTaggedFile =
							createTaggedFile(taggedFile->getDirInfo(), newFileName);
						if (newTaggedFile) {
							item->setFile(newTaggedFile);
							fileRenamed = true;
						}
					} else {
						QMessageBox::warning(
							0, i18n("File Error"),
							i18n("Error while renaming:\n") +
							i18n("Rename %1 to %2 failed\n").
							arg(taggedFile->getFilename()).arg(newFileName),
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
		QString newDirName = QInputDialog::getText(
			i18n("Rename Directory"), i18n("Enter new directory name:"),
			QLineEdit::Normal,
			fi.fileName(), &ok, this);
		if (ok && !newDirName.isEmpty()) {
			QString newPath = fi.dirPath() + '/' + newDirName;
			if (QDir().rename(dirInfo->getDirname(), newPath)) {
				item->setDirName(newPath);
			} else {
				QMessageBox::warning(
					0, i18n("File Error"),
					i18n("Error while renaming:\n") +
					i18n("Rename %1 to %2 failed\n").arg(fi.fileName()).arg(newDirName),
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
					i18n("Do you really want to delete this item?",
							 "Do you really want to delete these %n items?", numFiles),
					files,
					i18n("Delete Files"), KStdGuiItem::del(), QString::null,
					KMessageBox::Dangerous) == KMessageBox::Continue)
#else
		QString txt = numFiles > 1 ?
			i18n("Do you really want to delete these %1 items?").arg(numFiles) :
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
			files.clear();
			FileListItem* item = first();
			while (item != 0) {
				FileListItem* nextItem = next();
				if (item->isInSelection()) {
					if (QDir().remove(item->getFile()->getAbsFilename())) {
						delete item;
					} else {
						files.push_back(item->getFile()->getAbsFilename());
					}
				}
				item = nextItem;
			}
			if (!files.empty()) {
#ifdef CONFIG_USE_KDE
				KMessageBox::errorList(
					0, i18n("Error while deleting this item:",
									"Error while deleting these %n items:", files.size()),
					files,
					i18n("File Error"));
#else
				QString txt = files.size() > 1 ?
					i18n("Error while deleting these %1 items:").arg(files.size()) :
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
				i18n("Delete Files"), KStdGuiItem::del(), QString::null,
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
