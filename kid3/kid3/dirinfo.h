/**
 * \file dirinfo.h
 * Information about directory
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Dec 2006
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

#endif // DIRINFO_H
