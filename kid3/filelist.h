/**
 * \file filelist.h
 * List of files to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef FILELIST_H
#define FILELIST_H

#include <qstring.h>
#include "mp3file.h"

class QListBox;

/**
 * List of files to operate on.
 */
class FileList
{
 public:
	/**
	 * Constructor.
	 */
	FileList() : namefilter(defaultNameFilter) {}
	/**
	 * Get the first item in the filelist.
	 *
	 * @return first file.
	 */
	Mp3File *first();
	/**
	 * Get the next item in the filelist.
	 *
	 * @return next file.
	 */
	Mp3File *next();
	/**
	 * Set the file name filter to be used for the filelist.
	 *
	 * @param filter name filter
	 */
	void setNameFilter(const QString& filter) { namefilter = filter; }
	/**
	 * Get the file name filter to be used for the filelist.
	 *
	 * @return name filter.
	 */
	const QString& getNameFilter(void) const { return namefilter; }
	/**
	 * Set the listbox to be used for the filelist.
	 *
	 * @param lb listbox
	 */
	void setListBox(QListBox *lb) { listbox = lb; }
	/**
	 * Fill the filelist with the files found in a directory.
	 *
	 * @param name path of directory
	 * @return FALSE if name is not directory path, else TRUE.
	 */
	bool readDir(const QString& name);
	/**
	 * Refresh text of all files in listbox and check if any file is modified.
	 *
	 * @return TRUE if a file is modified.
	 */
	bool updateModificationState(void);
	/**
	 * Get path of directory.
	 *
	 * @return absolute path of directory.
	 */
	QString getDirname(void) const { return dirname; }
	/**
	 * Get absolute path of directory.
	 *
	 * @return absolute path of directory.
	 */
	QString getAbsDirname(void) const;

	/** Default name filter */
	static const QString defaultNameFilter;

 private:
	/** name filter */
	QString namefilter;
	/** path of directory */
	QString dirname;
	/** listbox to be used to hold file names */
	QListBox *listbox;
	/** current file */
	Mp3File *current_item;
};

#endif // FILELIST_H
