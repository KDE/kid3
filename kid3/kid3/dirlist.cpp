/**
 * \file dirlist.cpp
 * List of directories to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Jul 2005
 */

#include "dirlist.h"
#include <qfileinfo.h>
#include <qdir.h>
#include <qstringlist.h>

/**
 * Constructor.
 * @param parent parent widget
 */
DirList::DirList(QWidget* parent) :
#if QT_VERSION >= 0x040000
	QListWidget(parent)
#else
	QListBox(parent)
#endif
{}

/**
 * Destructor.
 */
DirList::~DirList() {}

/**
 * Fill the dirlist with the directories found in a directory.
 *
 * @param name path of directory
 * @return false if name is not directory path, else true.
 */
bool DirList::readDir(const QString& name) {
	QFileInfo file(name);
	if(file.isDir()) {
		clear();
		m_dirname = name;
		QDir dir(file.filePath());
		QCM_addItems(dir.entryList(QDir::Dirs | QDir::Drives));
		if (!m_entryToSelect.isEmpty()) {
#if QT_VERSION >= 0x040000
			QList<QListWidgetItem*> items =
				findItems(m_entryToSelect, Qt::MatchStartsWith);
			QListWidgetItem* lbi = items.empty() ? 0 : items.front();
#else
			QListBoxItem* lbi = findItem(m_entryToSelect);
#endif
			if (lbi) {
				setCurrentItem(lbi);
			}
		}
		return true;
	}
	return false;
}
