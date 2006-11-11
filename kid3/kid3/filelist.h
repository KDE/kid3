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
#include <qsize.h>
#include "filelistitem.h"
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
class Q3Process;
#else
#include <qlistbox.h>
class QProcess;
#endif

/**
 * List of files to operate on.
 */
class FileList : public Q3ListBox
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	FileList(QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0);
	/**
	 * Destructor.
	 */
	virtual ~FileList();
	/**
	 * Returns the recommended size for the widget.
	 * @return recommended size.
	 */
	virtual QSize sizeHint() const;
	/**
	 * Get the first item in the filelist.
	 *
	 * @return first file.
	 */
	FileListItem* first();
	/**
	 * Get the next item in the filelist.
	 *
	 * @return next file.
	 */
	FileListItem* next();
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

	/**
	 * Get the number of files in the file list.
	 * @return number of files.
	 */
	static int getNumberOfFiles() {
		return s_instance ? s_instance->count() : 0;
	}

private slots:
	/**
	 * Display a context menu with operations for selected files.
	 *
	 * @param item list box item
	 * @param pos  position where context menu is drawn on screen
	 */
#if QT_VERSION >= 0x040000
	void contextMenu(Q3ListBoxItem* item, const QPoint& pos);
#else
	void contextMenu(QListBoxItem* item, const QPoint& pos);
#endif

	/**
	 * Execute a context menu command.
	 *
	 * @param id command ID
	 */
	void executeContextCommand(int id);

private:
	FileList(const FileList&);
	FileList& operator=(const FileList&);

	/** path of directory */
	QString dirname;
	/** current file */
	FileListItem* current_item;
	/** Process for context menu commands */
	Q3Process* m_process;

	/** Single instance */
	static FileList* s_instance;
};

#endif // FILELIST_H
