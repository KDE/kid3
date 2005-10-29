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
#include <qlistbox.h>
#include <qsize.h>
#include "taggedfile.h"

class MiscConfig;
class QProcess;

/**
 * List of files to operate on.
 */
class FileList : public QListBox
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	FileList(QWidget* parent = 0, const char* name = 0, WFlags f = 0);
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
	TaggedFile *first();
	/**
	 * Get the next item in the filelist.
	 *
	 * @return next file.
	 */
	TaggedFile *next();
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
	 * Set configuration.
	 * @param cfg configuration.
	 */
	void setMiscConfig(MiscConfig* cfg) { m_miscCfg = cfg; }
	/**
	 * Get configuration.
	 * @return configuration.
	 */
	const MiscConfig* getMiscConfig() const { return m_miscCfg; }

private slots:
	/**
	 * Display a context menu with operations for selected files.
	 *
	 * @param item list box item
	 * @param pos  position where context menu is drawn on screen
	 */
	void contextMenu(QListBoxItem* item, const QPoint& pos);

	/**
	 * Execute a context menu command.
	 *
	 * @param id command ID
	 */
	void executeContextCommand(int id);

private:
	/** path of directory */
	QString dirname;
	/** current file */
	TaggedFile *current_item;
	/** Miscellaneous configuration */
	MiscConfig* m_miscCfg;
	/** Process for context menu commands */
	QProcess* m_process;
};

#endif // FILELIST_H
