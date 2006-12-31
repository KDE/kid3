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
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ListView>
class Q3Process;
#else
#include <qlistview.h>
class QProcess;
#endif
#include "dirinfo.h"

class FileListItem;

/**
 * List of files to operate on.
 */
class FileList : public Q3ListView
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
	 * Get the first item in the the current directory.
	 *
	 * @return first file.
	 */
	FileListItem* firstInDir();

	/**
	 * Get the next item in the current directory.
	 *
	 * @return next file.
	 */
	FileListItem* nextInDir();

	/**
	 * Get the number of files selected in the filelist.
	 *
	 * @return number of files selected.
	 */
	int numFilesSelected();

	/**
	 * Select the next file.
	 */
	void selectNextFile();

	/**
	 * Select the previous file.
	 */
	void selectPreviousFile();

	/**
	 * Fill the filelist with the files found in a directory.
	 *
	 * @param name path of directory
	 * @return false if name is not directory path, else true.
	 */
	bool readDir(const QString& name);

	/**
	 * Refresh text of all files in listview and check if any file is modified.
	 *
	 * @return true if a file is modified.
	 */
	bool updateModificationState();

	/**
	 * Fill the filelist with the files found in the directory tree.
	 *
	 * @param dirInfo  information  about directory
	 * @param item     parent directory item or 0 if top-level
	 * @param listView parent list view if top-level, else 0
	 */
	static void readSubDirectory(DirInfo* dirInfo, FileListItem* item,
															 FileList* listView);

private slots:
	/**
	 * Display a context menu with operations for selected files.
	 *
	 * @param item list box item
	 * @param pos  position where context menu is drawn on screen
	 */
#if QT_VERSION >= 0x040000
	void contextMenu(Q3ListViewItem* item, const QPoint& pos);
#else
	void contextMenu(QListViewItem* item, const QPoint& pos);
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

	/** information about directory */
	DirInfo m_dirInfo;
	/** iterator pointing to current file */
	Q3ListViewItemIterator m_iterator;
	/** current item in current directory */
	FileListItem* m_currentItemInDir;
	/** Process for context menu commands */
	Q3Process* m_process;
};

#endif // FILELIST_H
