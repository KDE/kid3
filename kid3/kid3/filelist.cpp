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
#else
#include <qpopupmenu.h>
#include <qprocess.h>
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

/** Single instance */
FileList* FileList::s_instance = 0;

/**
 * Constructor.
 */
FileList::FileList(QWidget* parent, const char* name, Qt::WFlags f) :
	Q3ListBox(parent, name, f), m_process(0)
{
#if QT_VERSION >= 0x040000
	connect(this, SIGNAL(contextMenuRequested(Q3ListBoxItem*, const QPoint&)),
			this, SLOT(contextMenu(Q3ListBoxItem*, const QPoint&)));
#else
	connect(this, SIGNAL(contextMenuRequested(QListBoxItem*, const QPoint&)),
			this, SLOT(contextMenu(QListBoxItem*, const QPoint&)));
#endif
	if (s_instance) {
		qWarning("The must be only one instance of FileList");
	}
	s_instance = this;
}

/**
 * Destructor.
 */
FileList::~FileList()
{
	if (this == s_instance) {
		s_instance = 0;
	}
}

/**
 * Returns the recommended size for the widget.
 * @return recommended size.
 */
QSize FileList::sizeHint() const
{
	return QSize(fontMetrics().maxWidth() * 25, Q3ListBox::sizeHint().height());
}

/**
 * Get the first item in the filelist.
 *
 * @return first file.
 */

FileListItem* FileList::first()
{
	current_item = dynamic_cast<FileListItem*>(firstItem());
	return current_item;
}

/**
 * Get the next item in the filelist.
 *
 * @return next file.
 */

FileListItem* FileList::next()
{
	current_item = dynamic_cast<FileListItem*>(current_item->next());
	return current_item;
}

/**
 * Fill the filelist with the files found in a directory.
 *
 * @param name path of directory
 * @return FALSE if name is not directory path, else TRUE.
 */

bool FileList::readDir(const QString& name)
{
	QFileInfo file(name);
	if(file.isDir()) {
		clear();
		dirname = name;
		QDir dir(file.filePath());
		QStringList dirContents = dir.entryList(
			Kid3App::s_miscCfg.nameFilter);
		for (QStringList::Iterator it = dirContents.begin();
			 it != dirContents.end(); ++it) {
			if (!QFileInfo(
			    dirname + QDir::separator() + *it).isDir()) {
				TaggedFile* taggedFile = 0;
#ifdef HAVE_VORBIS
				if ((*it).right(4).lower() == ".ogg")
					taggedFile = new OggFile(dirname, *it);
				else
#endif
#ifdef HAVE_FLAC
				if ((*it).right(5).lower() == ".flac")
					taggedFile = new FlacFile(dirname, *it);
				else
#endif
#ifdef HAVE_ID3LIB
				if ((*it).right(4).lower() == ".mp3"
#ifdef HAVE_TAGLIB
						&& Kid3App::s_miscCfg.m_id3v2Version != MiscConfig::ID3v2_4_0
#endif
					)
					taggedFile = new Mp3File(dirname, *it);
				else
#endif
#ifdef HAVE_TAGLIB
					taggedFile = new TagLibFile(dirname, *it);
#else
					;
#endif
				if (taggedFile) {
					insertItem(new FileListItem(taggedFile));
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Refresh text of all files in listbox and check if any file is modified.
 *
 * @return TRUE if a file is modified.
 */

bool FileList::updateModificationState(void)
{
	FileListItem* item = first();
	bool modified = false;
	while (item != 0) {
		if (item->getFile()->isChanged()) {
			modified = true;
		}
		item = next();
	}
	triggerUpdate(true);
	return modified;
}

/**
 * Get absolute path of directory.
 *
 * @return absolute path of directory.
 */

QString FileList::getAbsDirname(void) const
{
	QDir dir(dirname);
	return dir.absPath();
}

/**
 * Display a context menu with operations for selected files.
 *
 * @param item list box item
 * @param pos  position where context menu is drawn on screen
 */
void FileList::contextMenu(Q3ListBoxItem* item, const QPoint& pos)
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
