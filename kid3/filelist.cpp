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
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QMessageBox>
#include <QInputDialog>
#include <QUrl>
#include <QRegExp>
#include <QMenu>
#include <QHeaderView>
#include <QDirIterator>
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
			const char c = code[0].toLatin1();
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
					int sepPos = result.lastIndexOf('/');
					if (sepPos < 0) {
						sepPos = result.lastIndexOf(QDir::separator());
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
					url.setScheme("file");
					url.setPath(m_files.front());
					result = url.toString();
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
	QTreeWidget(parent), m_iterator(0),
	m_currentItemInDir(0), m_process(0), m_app(app)
{
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
	header()->hide();
}

/**
 * Destructor.
 */
FileList::~FileList()
{
	delete m_process;
	delete m_iterator;
}

/**
 * Returns the recommended size for the widget.
 * @return recommended size.
 */
QSize FileList::sizeHint() const
{
	return QSize(fontMetrics().maxWidth() * 25,
							 QTreeWidget::sizeHint().height());
}

/**
 * Get the next item in the filelist which contains a file.
 *
 * @param it list view iterator
 *
 * @return next item with file.
 */
static FileListItem* getNextItemWithFile(QTreeWidgetItemIterator& it)
{
	QTreeWidgetItem* lvItem;
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
	delete m_iterator;
	if (topLevelItemCount() > 0) {
		m_iterator = new QTreeWidgetItemIterator(this);
		return getNextItemWithFile(*m_iterator);
	} else {
		m_iterator = 0;
		return 0;
	}
}

/**
 * Get the next item in the filelist.
 *
 * @return next file.
 */
FileListItem* FileList::next()
{
	if (m_iterator && **m_iterator) {
		++*m_iterator;
		return getNextItemWithFile(*m_iterator);
	}
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
static FileListItem* getNextItemWithFileInDir(QTreeWidgetItem* lvItem)
{
	while (lvItem) {
		FileListItem* flItem = dynamic_cast<FileListItem*>(lvItem);
		if (flItem && flItem->getFile()) {
			return flItem;
		}
		QTreeWidgetItem* parent = lvItem->parent();
		QTreeWidgetItemIterator it(lvItem);
		lvItem = *(++it);
		if (lvItem && lvItem->parent() != parent) {
			lvItem = 0;
		}
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
}

/**
 * Get the next item in the current directory.
 *
 * @return next file.
 */
FileListItem* FileList::nextInDir()
{
	if (m_currentItemInDir) {
		QTreeWidgetItem* parent = m_currentItemInDir->parent();
		QTreeWidgetItemIterator it(m_currentItemInDir);
		QTreeWidgetItem* nextSibling = *(++it);
		if (nextSibling && nextSibling->parent() != parent) {
			nextSibling = 0;
		}
		m_currentItemInDir = getNextItemWithFileInDir(nextSibling);
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
static FileListItem* getNextItemWithFileOrDir(QTreeWidgetItemIterator& it)
{
	QTreeWidgetItem* lvItem;
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
	delete m_iterator;
	if (topLevelItemCount() > 0) {
		m_iterator = new QTreeWidgetItemIterator(this);
		return getNextItemWithFileOrDir(*m_iterator);
	} else {
		m_iterator = 0;
		return 0;
	}
}

/**
 * Get the next file or directory item in the filelist.
 *
 * @return next file.
 */
FileListItem* FileList::nextFileOrDir()
{
	if (m_iterator && **m_iterator) {
		++*m_iterator;
		return getNextItemWithFileOrDir(*m_iterator);
	}
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
	if (topLevelItemCount() <= 0) {
		return 0;
	}
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
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
	if (topLevelItemCount() <= 0) {
		return 0;
	}
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
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
	QTreeWidgetItem* item = *QTreeWidgetItemIterator(this);
	if (item) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
		return true;
	}
	return false;
}

/**
 * Select the next file.
 *
 * @return true if a next file exists.
 */
bool FileList::selectNextFile()
{
	QTreeWidgetItem* item = currentItem();
	if (item && (item = *(++QTreeWidgetItemIterator(item))) != 0) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
		return true;
	}
	return false;
}

/**
 * Select the previous file.
 *
 * @return true if a previous file exists.
 */
bool FileList::selectPreviousFile()
{
	QTreeWidgetItem* item = currentItem();
	if (item && (item = *(--QTreeWidgetItemIterator(item))) != 0) {
		clearSelection();
		setCurrentItem(item);
		setItemSelected(item, true);
		return true;
	}
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
	const QDir::Filters dirFilters =
		QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files;
	QStringList nameFilters(Kid3App::s_miscCfg.m_nameFilter.split(' '));
	QStringList dirContents = dir.entryList(
		nameFilters, dirFilters, QDir::DirsFirst | QDir::IgnoreCase);
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
					listView->setItemSelected(last, true);
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
					last->setChildIndicatorPolicy(
						QDirIterator(filename, nameFilters, dirFilters).hasNext() ?
						QTreeWidgetItem::ShowIndicator :
						QTreeWidgetItem::DontShowIndicatorWhenChildless);
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
		m_dirInfo.setDirname(file.absoluteFilePath());
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
	for (QList<DirContents*>::const_iterator it =
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
			last->setExpanded(true);
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
	update();
	return modified;
}

/**
 * Display a context menu with operations for selected files.
 *
 * @param item list box item
 * @param pos  position where context menu is drawn on screen
 */
void FileList::contextMenu(QTreeWidgetItem* item, const QPoint& pos)
{
	if (item && !Kid3App::s_miscCfg.m_contextMenuCommands.empty()) {
		QMenu menu(this);
		menu.addAction(i18n("&Expand all"), this, SLOT(expandAll()));
		menu.addAction(i18n("&Collapse all"), this, SLOT(collapseAll()));
		menu.addAction(i18n("&Rename"), this, SLOT(renameFile()));
		menu.addAction(i18n("&Delete"), this, SLOT(deleteFile()));
#ifdef HAVE_PHONON
		menu.addAction(i18n("&Play"), m_app, SLOT(slotPlayAudio()));
#endif
		int id = 0;
		for (QList<MiscConfig::MenuCommand>::const_iterator
					 it = Kid3App::s_miscCfg.m_contextMenuCommands.begin();
				 it != Kid3App::s_miscCfg.m_contextMenuCommands.end();
				 ++it) {
			menu.addAction((*it).getName());
			++id;
		}
		connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
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
		if ((*it).indexOf('%') == -1) {
			fmt.push_back(*it);
		} else {
			if (*it == "%F" || *it == "%{files}") {
				// list of files
				fmt += files;
			} else if (*it == "%uF" || *it == "%{urls}") {
				// list of URLs or URL
				QUrl url;
				url.setScheme("file");
				for (QStringList::const_iterator fit = files.begin();
						 fit != files.end();
						 ++fit) {
					url.setPath(*fit);
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
				end = cmd.indexOf(' ', begin + 1);
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
void FileList::executeAction(QAction* action)
{
	if (action) {
		QString name = action->text().remove('&');
		int id = 0;
		for (QList<MiscConfig::MenuCommand>::const_iterator
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

/**
 * Expand or collapse all folders.
 *
 * @param expand true to expand, false to collapse
 */
void FileList::setAllExpanded(bool expand)
{
	FileListItem* item = firstFileOrDir();
	while (item != 0) {
		item->setExpanded(expand);
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
			QString newFileName = QInputDialog::getText(
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
							QMessageBox::Ok, Qt::NoButton);
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
			this,
			i18n("Rename Directory"),
			i18n("Enter new directory name:"),
			QLineEdit::Normal, fi.fileName(), &ok);
		if (ok && !newDirName.isEmpty()) {
			QString newPath = fi.dir().path() + '/' + newDirName;
			if (QDir().rename(dirInfo->getDirname(), newPath)) {
				item->setDirName(newPath);
			} else {
				QMessageBox::warning(
					0, i18n("File Error"),
					i18n("Error while renaming:\n") +
					KCM_i18n2("Rename %1 to %2 failed\n", fi.fileName(), newDirName),
					QMessageBox::Ok, Qt::NoButton);
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
					i18np("Do you really want to delete this item?",
								"Do you really want to delete these %1 items?", numFiles),
					files,
					i18n("Delete Files"),
					KStandardGuiItem::del(), KStandardGuiItem::cancel(), QString(),
					KMessageBox::Dangerous) == KMessageBox::Continue)
#else
		QString txt = numFiles > 1 ?
			KCM_i18n1("Do you really want to delete these %1 items?", numFiles) :
			i18n("Do you really want to delete this item?");
		txt += '\n';
		txt += files.join("\n");
		if (QMessageBox::question(
				this, i18n("Delete Files"), txt,
				QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
#endif
		{
			QList<QTreeWidgetItem*> itemsToDelete;
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
							itemsToDelete.append(item);
						} else {
							item->setFile(TaggedFile::createFile(dirInfo, filename));
							files.push_back(absFilename);
						}
					}
				}
				item = nextItem;
			}
			qDeleteAll(itemsToDelete);
			itemsToDelete.clear();
			if (!files.empty()) {
#ifdef CONFIG_USE_KDE
				KMessageBox::errorList(
					0,
					i18np("Error while deleting this item:",
								"Error while deleting these %1 items:", files.size()),
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
					QMessageBox::Ok, Qt::NoButton);
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
				KStandardGuiItem::del(), KStandardGuiItem::cancel(), QString(),
				KMessageBox::Dangerous) == KMessageBox::Continue
#else
			QMessageBox::question(
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
					QMessageBox::Ok, Qt::NoButton);
#endif
			}
		}
	}
}

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