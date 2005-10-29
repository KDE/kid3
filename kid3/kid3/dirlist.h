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
#include <qlistbox.h>

/**
 * List of directories to operate on.
 */
class DirList : public QListBox {
public:
	/**
	 * Constructor.
	 */
	DirList(QWidget* parent = 0, const char* name = 0, WFlags f = 0);

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

private:
	/** path of directory */
	QString m_dirname;
};

#endif // DIRLIST_H
