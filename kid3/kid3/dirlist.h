/**
 * \file dirlist.h
 * List of directories to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Jul 2005
 */

#ifndef DIRLIST_H
#define DIRLIST_H

#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
#else
#include <qlistbox.h>
#endif

/**
 * List of directories to operate on.
 */
class DirList : public Q3ListBox {
public:
	/**
	 * Constructor.
	 */
	DirList(QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0);

	/**
	 * Destructor.
	 */
	virtual ~DirList();

	/**
	 * Fill the dirlist with the directories found in a directory.
	 *
	 * @param name path of directory
	 * @return FALSE if name is not directory path, else TRUE.
	 */
	bool readDir(const QString& name);

	/**
	 * Get path of directory.
	 *
	 * @return absolute path of directory.
	 */
	QString getDirname(void) const { return m_dirname; }

	/**
	 * Set name of entry to select in next call to readDir().
	 *
	 * @param str name of entry to select
	 */
	void setEntryToSelect(const QString& str) { m_entryToSelect = str; }

private:
	/** path of directory */
	QString m_dirname;
	/** entry to select in readDir() */
	QString m_entryToSelect;
};

#endif // DIRLIST_H
