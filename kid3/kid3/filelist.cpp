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
#include <QMenu>
#include <QHeaderView>
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
 * @param parent parent widget
 */
FileList::FileList(QWidget* parent) :
#if QT_VERSION >= 0x040000
	QTreeWidget(parent), m_iterator(0),
#else
	QListView(parent),
#endif
	m_currentItemInDir(0), m_process(0)
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
 * Select the next file.
 */
void FileList::selectNextFile()
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* item = currentItem();
	if (item && (item = *(++QTreeWidgetItemIterator(item))) != 0) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
	}
#else
	QListViewItem* item = currentItem();
	if (item && (item = item->itemBelow()) != 0) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);
	}
#endif
}

/**
 * Select the previous file.
 */
void FileList::selectPreviousFile()
{
#if QT_VERSION >= 0x040000
	QTreeWidgetItem* item = currentItem();
	if (item && (item = *(--QTreeWidgetItemIterator(item))) != 0) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
	}
#else
	QListViewItem* item = currentItem();
	if (item && (item = item->itemAbove()) != 0) {
		clearSelection();
		setCurrentItem(item);
		setSelected(item, true);
	}
#endif
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
	if (fn.right(4).QCM_toLower() == ".ogg")
		taggedFile = new OggFile(di, fn);
	else
#endif
#ifdef HAVE_FLAC
		if (fn.right(5).QCM_toLower() == ".flac")
			taggedFile = new FlacFile(di, fn);
		else
#endif
#ifdef HAVE_ID3LIB
			if (fn.right(4).QCM_toLower() == ".mp3"
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
#if QT_VERSION >= 0x040000
		dir.entryList(Kid3App::s_miscCfg.m_nameFilter.split(' '), QDir::Files);
#else
		dir.entryList(Kid3App::s_miscCfg.m_nameFilter, QDir::Files);
#endif
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
		m_dirInfo.setDirname(file.QCM_absoluteFilePath());
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
		menu.addAction(i18n("&Rename"), this, SLOT(renameFile()));
		menu.addAction(i18n("&Delete"), this, SLOT(deleteFile()));
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
		menu.insertItem(i18n("&Rename"), this, SLOT(renameFile()));
		menu.insertItem(i18n("&Delete"), this, SLOT(deleteFile()));
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
		if ((*it).QCM_indexOf('%') == -1) {
			fmt.push_back(*it);
		} else {
			if (*it == "%F") {
				// list of files
				fmt += files;
			} else if (*it == "%uF") {
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
				const int numTagCodes = 3;
				const QChar tagCode[numTagCodes] = { 'f', 'd', 'b' };
				QString tagStr[numTagCodes];
				if (!files.empty()) {
					tagStr[0] = files.front();
					tagStr[1] = tagStr[0];
					tagStr[2] = Kid3App::s_miscCfg.m_browser;
					if (!dirInfo) {
						int sepPos = tagStr[1].QCM_lastIndexOf('/');
						if (sepPos < 0) {
							sepPos = tagStr[1].QCM_lastIndexOf(QDir::separator());
						}
						if (sepPos >= 0) {
							tagStr[1].truncate(sepPos);
						}
					}
				}
				QString str = StandardTags::replacePercentCodes(*it, tagCode, tagStr, numTagCodes);

				int ufPos;
				if ((ufPos = str.QCM_indexOf("%uf")) != -1 && !files.empty()) {
					QUrl url;
					url.QCM_setScheme("file");
					url.QCM_setPath(files.front());
					str.replace(ufPos, 3, url.toString());
				}

				if (firstSelectedItem) {
					// use merged tags 1 and 2 to format string
					StandardTags st1, st2;
					firstSelectedItem->getFile()->getStandardTagsV1(&st1);
					firstSelectedItem->getFile()->getStandardTagsV2(&st2);
					st2.merge(st1);
					str = st2.formatString(str, StandardTags::FSF_SupportUrlEncode);
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
		QString name = action->text();
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
							createTaggedFile(dirInfo, newFileName);
						if (newTaggedFile) {
							item->setFile(newTaggedFile);
							fileRenamed = true;
						}
					} else {
						item->setFile(createTaggedFile(dirInfo, filename));
						QMessageBox::warning(
							0, i18n("File Error"),
							i18n("Error while renaming:\n") +
							i18n("Rename %1 to %2 failed\n").
							arg(filename).arg(newFileName),
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
							item->setFile(createTaggedFile(dirInfo, filename));
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
