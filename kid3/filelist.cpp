/**
 * \file filelist.cpp
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qlistbox.h>

#include "mp3file.h"
#include "filelist.h"

/** Default name filter */
const QString FileList::defaultNameFilter("*.mp3 *.MP3");

/**
 * Get the first item in the filelist.
 *
 * @return first file.
 */

Mp3File *FileList::first()
{
	current_item = dynamic_cast<Mp3File *>(listbox->firstItem());
	return current_item;
}

/**
 * Get the next item in the filelist.
 *
 * @return next file.
 */

Mp3File *FileList::next()
{
	current_item = dynamic_cast<Mp3File *>(current_item->next());
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
		listbox->clear();
		dirname = name;
		QDir dir(file.filePath());
		QStringList dirContents=dir.entryList(namefilter);
		for (QStringList::Iterator it = dirContents.begin();
			 it != dirContents.end(); ++it) {
			if (!QFileInfo(
			    dirname + QDir::separator() + *it).isDir()) {
				Mp3File *mp3file = new Mp3File(dirname, *it);
				if (mp3file) {
					listbox->insertItem(mp3file);
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
	Mp3File *mp3file = first();
	bool modified = FALSE;
	while (mp3file != 0) {
		mp3file->refreshText();
		if (mp3file->isChanged()) {
			modified = TRUE;
		}
		mp3file = next();
	}
	listbox->triggerUpdate(TRUE);
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
