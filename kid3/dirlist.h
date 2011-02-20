/**
 * \file dirlist.h
 * List of directories to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Jul 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#ifndef DIRLIST_H
#define DIRLIST_H

#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QListWidget>
#else
#include <qlistbox.h>
#endif

/**
 * List of directories to operate on.
 */
class DirList : public
#if QT_VERSION >= 0x040000
QListWidget
#else
QListBox
#endif
{
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 */
	DirList(QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~DirList();

	/**
	 * Fill the dirlist with the directories found in a directory.
	 *
	 * @param name path of directory
	 * @return false if name is not directory path, else true.
	 */
	bool readDir(const QString& name);

	/**
	 * Get path of directory.
	 *
	 * @return absolute path of directory.
	 */
	QString getDirname() const { return m_dirname; }

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
