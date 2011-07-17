/**
 * \file kid3application.hpp
 * Kid3 application logic, independent of GUI.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef KID3APPLICATION_H
#define KID3APPLICATION_H

#include <QObject>
#include <QPersistentModelIndex>
#include "frame.h"
#include "generalconfig.h"

class QFileSystemModel;
class QItemSelectionModel;
class QModelIndex;
class FileProxyModel;
class DirProxyModel;
class TrackDataModel;
class FrameTableModel;
class ConfigStore;
class PlaylistConfig;
class DownloadClient;
class TaggedFile;

/**
 * Kid3 application logic, independent of GUI.
 */
class Kid3Application : public QObject {
	Q_OBJECT
public:
	/** Destination for downloadImage(). */
	enum DownloadImageDestination {
		ImageForSelectedFiles,         /**< only for current file */
		ImageForAllFilesInDirectory, /**< for all files in directory */
		ImageForImportTrackData      /**< for enabled files in m_trackDataModel */
	};

	/**
	 * Constructor.
	 * @param parent parent object
	 */
	explicit Kid3Application(QObject* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~Kid3Application();

 /**
	* Get file system model.
	* @return file system model.
	*/
	QFileSystemModel* getFileSystemModel() { return m_fileSystemModel; }

 /**
	* Get file proxy model.
	* @return file proxy model.
	*/
	FileProxyModel* getFileProxyModel() { return m_fileProxyModel; }

 /**
	* Get directory proxy model.
	* @return directory proxy model.
	*/
	DirProxyModel* getDirProxyModel() { return m_dirProxyModel; }

 /**
	* Get track data model.
	* @return track data model.
	*/
	TrackDataModel* getTrackDataModel() { return m_trackDataModel; }

	/**
	 * Get selection model of files.
	 */
	QItemSelectionModel* getFileSelectionModel() { return m_fileSelectionModel; }

	/**
	 * Get tag 1 frame table model.
	 * @return frame table.
	 */
	FrameTableModel* frameModelV1() { return m_framesV1Model; }

	/**
	 * Get tag 2 frame table model.
	 * @return frame table.
	 */
	FrameTableModel* frameModelV2() { return m_framesV2Model; }

	/**
	 * Get selection model of tag 1 frame table model.
	 */
	QItemSelectionModel* getFramesV1SelectionModel() {
		return m_framesV1SelectionModel;
	}

	/**
	 * Get selection model of tag 2 frame table model.
	 */
	QItemSelectionModel* getFramesV2SelectionModel() {
		return m_framesV2SelectionModel;
	}

	/**
	 * Get settings.
	 * @return settings.
	 */
	Kid3Settings* getSettings() const;

	/**
	 * Get download client.
	 * @return download client.
	 */
	DownloadClient* getDownloadClient() { return m_downloadClient; }

	/**
	 * Get current index in file proxy model or root index if current index is
	 * invalid.
	 * @return current index, root index if not valid.
	 */
	QModelIndex currentOrRootIndex() const;

	/**
	 * Save settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Read settings from the configuration.
	 */
	void readConfig();

	/**
	 * Open directory.
	 *
	 * @param dir       directory or file path
	 * @param fileCheck if true and dir in not directory, only open directory
	 *                  if dir is a valid file path
	 *
	 * @return true if ok, directoryOpened() is emitted.
	 */
	bool openDirectory(QString dir, bool fileCheck = false);

	/**
	 * Get root index of opened directory in file proxy model.
	 * @return index of directory root.
	 */
	QPersistentModelIndex getRootIndex() const {
		return m_fileProxyModelRootIndex;
	}

	/**
	 * Get directory path of opened directory.
	 * @return directory path.
	 */
	QString getDirPath() const;

	/**
	 * Save all changed files.
	 * saveStarted() and saveProgress() are emitted while saving files.
	 *
	 * @return list of files with error, empty if ok.
	 */
	QStringList saveDirectory();

	/**
	 * Write playlist according to playlist configuration.
	 *
	 * @param cfg playlist configuration to use
	 *
	 * @return true if ok.
	 */
	bool writePlaylist(const PlaylistConfig& cfg);

	/**
	 * Set track data model with tagged files of directory.
	 */
	void filesToTrackDataModel();

	/**
	 * Set tagged files of directory from track data model.
	 *
	 * @param destV1 true to set tag 1
	 * @param destV2 true to set tag 2
	 */
	void trackDataModelToFiles(bool destV1, bool destV2);

	/**
	 * Download an image file.
	 *
	 * @param url  URL of image
	 * @param dest specifies affected files
	 */
	void downloadImage(const QString& url, DownloadImageDestination dest);

	/**
	 * Format a filename if format while editing is switched on.
	 *
	 * @param taggedFile file to modify
	 */
	void formatFileNameIfEnabled(TaggedFile* taggedFile) const;

	/**
	 * Format frames if format while editing is switched on.
	 *
	 * @param frames frames
	 */
	void formatFramesIfEnabled(FrameCollection& frames) const;

	/**
	 * Convert ID3v2.3 to ID3v2.4 tags.
	 */
	void convertToId3v24();

	/**
	 * Convert ID3v2.4 to ID3v2.3 tags.
	 */
	void convertToId3v23();

	/**
	 * Apply filename format.
	 */
	void applyFilenameFormat();

	/**
	 * Apply ID3 format.
	 */
	void applyId3Format();

	/**
	 * Copy tags 1 into copy buffer.
	 */
	void copyTagsV1();

	/**
	 * Copy tags 2 into copy buffer.
	 */
	void copyTagsV2();

	/**
	 * Paste from copy buffer to ID3v1 tags.
	 */
	void pasteTagsV1();

	/**
	 * Paste from copy buffer to ID3v2 tags.
	 */
	void pasteTagsV2();

	/**
	 * Copy ID3v1 tags to ID3v2 tags of selected files.
	 */
	void copyV1ToV2();

	/**
	 * Copy ID3v2 tags to ID3v1 tags of selected files.
	 */
	void copyV2ToV1();

	/**
	 * Remove ID3v1 tags in selected files.
	 */
	void removeTagsV1();

	/**
	 * Remove ID3v2 tags in selected files.
	 */
	void removeTagsV2();

	/**
	 * Get number of tracks in current directory.
	 *
	 * @return number of tracks, 0 if not found.
	 */
	int getTotalNumberOfTracksInDir();

	/**
	 * Create a filter string for the file dialog.
	 * The filter string contains entries for all supported types.
	 *
	 * @param defaultNameFilter if not 0, return default name filter here
	 *
	 * @return filter string.
	 */
	QString createFilterString(QString* defaultNameFilter = 0) const;

	/**
	 * Get image destination set by downloadImage().
	 * @return image destination.
	 */
	DownloadImageDestination getDownloadImageDestination() const {
		return m_downloadImageDest;
	}

	/**
	 * Set modification state.
	 *
	 * @param val true if a file is modified
	 */
	void setModified(bool val) { m_modified = val; }

	/**
	 * Check modification state.
	 *
	 * @return true if a file is modified.
	 */
	bool isModified() { return m_modified; }

	/**
	 * Set filter state.
	 *
	 * @param val true if list is filtered
	 */
	void setFiltered(bool val) { m_filtered = val; }

	/**
	 * Check filter state.
	 *
	 * @return true if list is filtered.
	 */
	bool isFiltered() { return m_filtered; }

	/**
	 * Get directory name.
	 * @return directory.
	 */
	static QString getDirName() { return s_dirName; }

	/**
	 * Set directory name.
	 * @param dirName directory.
	 */
	static void setDirName(const QString& dirName) { s_dirName = dirName; }

	/**
	 * Set the ID3v1 and ID3v2 text encodings from the configuration.
	 */
	static void setTextEncodings();

	/**
	 * Get the URL of an image file.
	 * The input URL is transformed using the match picture URL table to
	 * get the URL of an image file.
	 *
	 * @param url URL from image drag
	 *
	 * @return URL of image file, empty if no image URL found.
	 */
	static QString getImageUrl(const QString& url);

signals:
	/**
	 * Emitted when a new directory is opened.
	 * @param directoryIndex root path file system model index
	 * @param filePathIndex file path index in the file system model
	 */
	void directoryOpened(const QModelIndex& directoryIndex,
											 const QModelIndex& fileIndex);

	/**
	 * Emitted when saving files is started.
	 * @param totalFiles total number of files to be saved
	 */
	void saveStarted(int totalFiles);

	/**
	 * Emitted when a file has bee saved.
	 * @param numFiles number of files saved
	 */
	void saveProgress(int numFiles);

public slots:

private:
	/**
	* Init file types.
	*/
	void initFileTypes();

	/** model of filesystem */
	QFileSystemModel* m_fileSystemModel;
	FileProxyModel* m_fileProxyModel;
	DirProxyModel* m_dirProxyModel;
	QItemSelectionModel* m_fileSelectionModel;
	/** Track data model */
	TrackDataModel* m_trackDataModel;
	FrameTableModel* m_framesV1Model;
	FrameTableModel* m_framesV2Model;
	QItemSelectionModel* m_framesV1SelectionModel;
	QItemSelectionModel* m_framesV2SelectionModel;
	/** Configuration */
	ConfigStore* m_configStore;
	/** Download client */
	DownloadClient* m_downloadClient;
	/** Affected files to add frame when downloading image */
	DownloadImageDestination m_downloadImageDest;
	/** Copy buffer */
	FrameCollection m_copyTags;
	/** true if any file was modified */
	bool m_modified;
	/** true if list is filtered */
	bool m_filtered;
	/** Root index in file proxy model */
	QPersistentModelIndex m_fileProxyModelRootIndex;
	/** Current directory */
	static QString s_dirName;
};

#endif // KID3APPLICATION_H
