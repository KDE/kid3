/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#if QT_VERSION >= 300
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qprocess.h>
#endif

#include "taggedfile.h"
#include "mp3file.h"
#include "filelist.h"
#include "miscconfig.h"
#ifdef HAVE_VORBIS
#include "oggfile.h"
#endif
#ifdef HAVE_FLAC
#include "flacfile.h"
#endif

/**
 * Constructor.
 */
FileList::FileList(QWidget* parent, const char* name, WFlags f) :
	QListBox(parent, name, f), m_miscCfg(0), m_process(0)
{
#if QT_VERSION >= 300
	connect(this, SIGNAL(contextMenuRequested(QListBoxItem*, const QPoint&)),
			this, SLOT(contextMenu(QListBoxItem*, const QPoint&)));
#endif
}

/**
 * Destructor.
 */
FileList::~FileList() {}

/**
 * Returns the recommended size for the widget.
 * @return recommended size.
 */
QSize FileList::sizeHint() const
{
	return QSize(fontMetrics().maxWidth() * 25, QListBox::sizeHint().height());
}

/**
 * Get the first item in the filelist.
 *
 * @return first file.
 */

TaggedFile *FileList::first()
{
	current_item = dynamic_cast<TaggedFile *>(firstItem());
	return current_item;
}

/**
 * Get the next item in the filelist.
 *
 * @return next file.
 */

TaggedFile *FileList::next()
{
	current_item = dynamic_cast<TaggedFile *>(current_item->next());
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
			m_miscCfg ? m_miscCfg->nameFilter : "*");
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
				if ((*it).right(4).lower() == ".mp3")
					taggedFile = new Mp3File(dirname, *it);
				if (taggedFile) {
					insertItem(taggedFile);
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
	TaggedFile *taggedFile = first();
	bool modified = FALSE;
	while (taggedFile != 0) {
		if (taggedFile->isChanged()) {
			modified = TRUE;
		}
		taggedFile = next();
	}
	triggerUpdate(TRUE);
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
void FileList::contextMenu(QListBoxItem* item, const QPoint& pos)
{
#if QT_VERSION >= 300
	if (item && m_miscCfg && !m_miscCfg->m_contextMenuCommands.empty()) {
		QPopupMenu menu(this);
		int id = 0;
		for (QStringList::const_iterator
					 it = m_miscCfg->m_contextMenuCommands.begin();
				 it != m_miscCfg->m_contextMenuCommands.end();
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
#endif
}

/**
 * Execute a context menu command.
 *
 * @param id command ID
 */
void FileList::executeContextCommand(int id)
{
#if QT_VERSION >= 300
	if (m_miscCfg &&
			id < static_cast<int>(m_miscCfg->m_contextMenuCommands.size())) {
		QStringList args;
		QString cmd = m_miscCfg->m_contextMenuCommands[id];
		bool confirm = false;
		if (cmd[0] == '!') {
			cmd = cmd.mid(1);
			confirm = true;
		}
		args.push_back(cmd);
		TaggedFile *taggedFile = first();
		while (taggedFile != 0) {
			if (taggedFile->isInSelection()) {
				args.push_back(taggedFile->getAbsFilename());
			}
			taggedFile = next();
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
				m_process = new QProcess(this);
			}
			if (m_process) {
				m_process->setArguments(args);
				m_process->launch("");
			}
		}
	}
#endif
}
