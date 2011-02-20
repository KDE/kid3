/**
 * \file dirinfo.h
 * Information about directory
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Dec 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#ifndef DIRINFO_H
#define DIRINFO_H

/** Directory containing tagged files. */
class DirInfo {
public:
	/**
	 * Constructor.
	 *
	 * @param dirname  directory name
	 * @param numFiles number of tagged files
	 */
	explicit DirInfo(const QString& dirname = QString::null, int numFiles = 0) :
	m_dirname(dirname), m_numFiles(numFiles) {}

	/**
	 * Destructor.
	 */
	~DirInfo() {}

	/**
	 * Set directory name.
	 * @param dirname directory name
	 */
	void setDirname(const QString& dirname) { m_dirname = dirname; }

	/**
	 * Get directory name.
	 * @return directory name.
	 */
	QString getDirname() const { return m_dirname; }

	/**
	 * Set number of tagged files.
	 * @param numFiles number of files
	 */
	void setNumFiles(int numFiles) { m_numFiles = numFiles; }

	/**
	 * Get number of tagged files.
	 * @return number of files.
	 */
	int getNumFiles() const { return m_numFiles; }

private:
	/** Directory name */
	QString m_dirname;
	/** Number of tagged files in directory */
	int m_numFiles;
};

/**
 * Directory with contained files.
 */
class DirContents : public DirInfo {
public:
	/** List of directory contents */
#if QT_VERSION >= 0x040000
	typedef QList<DirContents*> DirContentsList;
#else
	typedef QValueList<DirContents*> DirContentsList;
#endif

	/**
	 * Constructor.
	 *
	 * @param dirname  directory name
	 * @param numFiles number of tagged files in directory
	 */
	explicit DirContents(const QString& dirname = QString::null, int numFiles = 0) :
		DirInfo(dirname, numFiles)
	{
	}

	/**
	 * Destructor.
	 */
	~DirContents()
	{
#if QT_VERSION >= 0x040000
		qDeleteAll(m_dirs);
#else
		for (DirContentsList::iterator it = m_dirs.begin();
				 it != m_dirs.end();
				 ++it) {
			delete *it;
		}
#endif
	}

 /**
  * Get directories.
  * @return directories.
  */
	const DirContentsList& getDirs() const { return m_dirs; }

 /**
  * Reference to directories.
  * @return directories.
  */
	DirContentsList& dirs() { return m_dirs; }

 /**
  * Get file names.
  * @return file names.
  */
	const QStringList& getFiles() const { return m_files; }

 /**
  * Reference to file names.
  * @return file names.
  */
	QStringList& files() { return m_files; }

private:
	DirContentsList m_dirs;
	QStringList m_files;
};

#endif // DIRINFO_H
