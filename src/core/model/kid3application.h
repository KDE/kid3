/**
 * \file kid3application.h
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
#include "trackdata.h"
#include "filefilter.h"
#include "generalconfig.h"
#include "config.h"

class QFileSystemModel;
class QItemSelectionModel;
class QModelIndex;
class QNetworkAccessManager;
class FileProxyModel;
class DirProxyModel;
class TrackDataModel;
class FrameTableModel;
class ConfigStore;
class PlaylistConfig;
class DownloadClient;
class TaggedFile;
class FrameList;
class IFrameEditor;
class ServerImporter;
class MusicBrainzClient;
class TextExporter;
class DirRenamer;
#ifdef HAVE_PHONON
class AudioPlayer;
#endif

/**
 * Kid3 application logic, independent of GUI.
 */
class KID3_CORE_EXPORT Kid3Application : public QObject {
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
   * Get frame list.
   * @return frame list.
   */
  FrameList* getFrameList() { return m_framelist; }

  /**
   * Get settings.
   * @return settings.
   */
  Kid3Settings* getSettings() const;

  /**
   * Get configuration.
   * @return configuration.
   */
  ConfigStore* getConfigStore() const { return m_configStore; }

  /**
   * Get download client.
   * @return download client.
   */
  DownloadClient* getDownloadClient() { return m_downloadClient; }

  /**
   * Get text exporter.
   * @return text exporter.
   */
  TextExporter* getTextExporter() { return m_textExporter; }

  /**
   * Get available server importers.
   * @return list of server importers.
   */
  QList<ServerImporter*> getServerImporters() { return m_importers; }

  /**
   * Get MusicBrainz client.
   * @return MusicBrainz client if supported, else 0
   */
  MusicBrainzClient* getMusicBrainzClient() { return m_musicBrainzClient; }

  /**
   * Get directory renamer.
   * @return directory renamer.
   */
  DirRenamer* getDirRenamer() { return m_dirRenamer; }

#ifdef HAVE_PHONON
  /**
   * Get audio player.
   * @return audio player.
   */
  AudioPlayer* getAudioPlayer();
#endif

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
   * Import.
   *
   * @param tagMask tag mask
   * @param path    path of file
   * @param fmtIdx  index of format
   *
   * @return true if ok.
   */
  bool importTags(TrackData::TagVersion tagMask, const QString& path,
                  int fmtIdx);

  /**
   * Export.
   *
   * @param tagVersion tag version
   * @param path   path of file
   * @param fmtIdx index of format
   *
   * @return true if ok.
   */
  bool exportTags(TrackData::TagVersion tagVersion,
                  const QString& path, int fmtIdx);

  /**
   * Write playlist according to playlist configuration.
   *
   * @param cfg playlist configuration to use
   *
   * @return true if ok.
   */
  bool writePlaylist(const PlaylistConfig& cfg);

  /**
   * Write playlist using current playlist configuration.
   *
   * @return true if ok.
   */
  bool writePlaylist();

  /**
   * Perform rename actions and change application directory afterwards if it
   * was renamed.
   *
   * @return error messages, null string if no error occurred.
   */
  QString performRenameActions();

  /**
   * Set the directory name from the tags.
   * The directory must not have modified files.
   *
   * @param tagMask tag mask
   * @param format  directory name format
   * @param create  true to create, false to rename
   * @param errStr  if not 0, a string describing the error is returned here
   *
   * @return true if ok.
   */
  bool renameDirectory(TrackData::TagVersion tagMask,
                       const QString& format,
                       bool create, QString* errStr);

  /**
   * Number tracks in selected files of directory.
   *
   * @param nr start number
   * @param total total number of tracks, used if >0
   * @param tagVersion determines on which tags the numbers are set
   */
  void numberTracks(int nr, int total, TrackData::TagVersion tagVersion);

  /**
   * Set track data with tagged files of directory.
   *
   * @param tagVersion tag version
   * @param trackDataList is filled with track data
   */
  void filesToTrackData(TrackData::TagVersion tagVersion,
                        ImportTrackDataVector& trackDataList);

  /**
   * Set track data model with tagged files of directory.
   *
   * @param tagVersion tag version
   */
  void filesToTrackDataModel(TrackData::TagVersion tagVersion);

  /**
   * Set tagged files of directory from track data model.
   *
   * @param tagVersion tags to set
   */
  void trackDataModelToFiles(TrackData::TagVersion tagVersion);

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
   * Open directory on drop.
   *
   * @param txt URL of directory or file in directory
   */
  void openDrop(QString txt);

  /**
   * Add picture on drop.
   *
   * @param image dropped image.
   */
  void dropImage(const QImage& image);

  /**
   * Handle URL on drop.
   *
   * @param txt dropped URL.
   */
  void dropUrl(const QString& txt);

  /**
   * Get number of tracks in current directory.
   *
   * @return number of tracks, 0 if not found.
   */
  int getTotalNumberOfTracksInDir();

  /**
   * Get name of selected file.
   *
   * @return absolute file name, ends with "/" if it is a directory.
   */
  QString getFileNameOfSelectedFile();

  /**
   * Create a filter string for the file dialog.
   * The filter string contains entries for all supported types.
   *
   * @return filter string.
   */
  QString createFilterString() const;

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
   * Get format used to generate tags from filename.
   * @return format
   */
  QString getTagsToFilenameFormat() const {
    return m_tagsToFilenameFormat;
  }

  /**
   * Get the selected file.
   *
   * @return the selected file,
   *         0 if not exactly one file is selected
   */
  TaggedFile* getSelectedFile();

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

public slots:
  /**
   * Revert file modifications.
   * Acts on selected files or all files if no file is selected.
   */
  void revertFileModifications();

  /**
   * Set name of selected file.
   * Exactly one file has to be selected.
   *
   * @param name file name.
   */
  void setFileNameOfSelectedFile(const QString& name);

  /**
   * Apply filename format.
   */
  void applyFilenameFormat();

  /**
   * Apply ID3 format.
   */
  void applyId3Format();

#ifdef HAVE_TAGLIB
  /**
   * Convert ID3v2.3 to ID3v2.4 tags.
   */
  void convertToId3v24();
#endif

#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
  /**
   * Convert ID3v2.4 to ID3v2.3 tags.
   */
  void convertToId3v23();
#endif

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
   * Set ID3v1 tags according to filename.
   * If a single file is selected the tags in the GUI controls
   * are set, else the tags in the multiple selected files.
   */
  void getTagsFromFilenameV1();

  /**
   * Set ID3v2 tags according to filename.
   * If a single file is selected the tags in the GUI controls
   * are set, else the tags in the multiple selected files.
   */
  void getTagsFromFilenameV2();

  /**
   * Set format used to generate filename from tags.
   * @param format format
   */
  void setFilenameToTagsFormat(const QString& format);

  /**
   * Set format used to generate filename from tags without emitting
   * filenameToTagsFormatChanged() signal.
   * This has to be used when connected from the GUI to avoid that the GUI
   * is updated because of its own changes.
   * @param format format
   */
  void setFilenameToTagsFormatWithoutSignaling(const QString& format);

  /**
   * Set format used to generate tags from filename.
   * @param format format
   */
  void setTagsToFilenameFormat(const QString& format);

  /**
   * Set format used to generate tags from filename without emitting
   * tagsToFilenameFormatChanged() signal.
   * This has to be used when connected from the GUI to avoid that the GUI
   * is updated because of its own changes.
   * @param format format
   */
  void setTagsToFilenameFormatWithoutSignaling(const QString& format);

  /**
   * Set filename according to tags.
   * If a single file is selected the tags in the GUI controls
   * are used, else the tags in the multiple selected files.
   *
   * @param tagVersion tag version
   */
  void getFilenameFromTags(TrackData::TagVersion tagVersion);

  /**
   * Edit selected frame.
   *
   * @param frameEditor editor for frame fields
   */
  void editFrame(IFrameEditor* frameEditor);

  /**
   * Delete selected frame.
   *
   * @param frameName name of frame to delete, empty to delete selected frame
   */
  void deleteFrame(const QString& frameName = QString());

  /**
   * Select a frame type and add such a frame to frame list.
   *
   * @param frame frame to add, if 0 the user has to select and edit the frame
   * @param frameEditor editor for frame fields, if not null and a frame
   * is set, the user can edit the frame before it is added
   */
  void addFrame(const Frame* frame, IFrameEditor* frameEditor = 0);

  /**
   * Edit a picture frame if one exists or add a new one.
   *
   * @param frameEditor editor for frame fields
   */
  void editOrAddPicture(IFrameEditor* frameEditor);

  /**
   * Add a downloaded image.
   *
   * @param data     HTTP response of download
   * @param mimeType MIME type of data
   * @param url      URL of downloaded data
   */
  void imageDownloaded(const QByteArray& data,
                       const QString& mimeType, const QString& url);

  /**
   * Set the first file as the current file.
   *
   * @param select true to select the file
   *
   * @return true if a file exists.
   */
  bool firstFile(bool select = true);

  /**
   * Set the next file as the current file.
   *
   * @param select true to select the file
   *
   * @return true if a next file exists.
   */
  bool nextFile(bool select = true);

  /**
   * Set the previous file as the current file.
   *
   * @param select true to select the file
   *
   * @return true if a previous file exists.
   */
  bool previousFile(bool select = true);

  /**
   * Select or deselect the current file.
   *
   * @param select true to select the file, false to deselect it
   *
   * @return true if a current file exists.
   */
  bool selectCurrentFile(bool select = true);

  /**
   * Select all files.
   */
  void selectAllFiles();

  /**
   * Deselect all files.
   */
  void deselectAllFiles();

  /**
   * Fetch entries of directory if not already fetched.
   * This works like FileList::expand(), but without expanding tree view
   * items and independent of the GUI. The processing is done in the background
   * by QFileSystemModel, so the fetched items are not immediately available
   * after calling this method.
   *
   * @param index index of directory item
   */
  void fetchDirectory(const QModelIndex& index);

  /**
   * Fetch entries for all directories if not already fetched.
   * This works like FileList::expandAll(), but without expanding tree view
   * items and independent of the GUI. The processing is done in the background
   * by QFileSystemModel, so the fetched items are not immediately available
   * after calling this method.
   */
  void fetchAllDirectories();

  /**
   * Process change of selection.
   * The GUI is signaled to update the current selection and the controls.
   */
  void fileSelected();

  /**
   * Schedule actions to rename a directory.
   */
  void scheduleRenameActions();

  /**
   * Apply a file filter.
   *
   * @param fileFilter filter to apply.
   */
  void applyFilter(FileFilter& fileFilter);

  /**
   * Apply a file filter.
   *
   * @param expression filter expression
   */
  void applyFilter(const QString& expression);

#ifdef HAVE_PHONON
  /**
   * Play audio file.
   */
  void playAudio();
#endif

signals:
  /**
   * Emitted when a new directory is opened.
   * @param directoryIndex root path file system model index
   * @param fileIndex file path index in the file system model
   */
  void directoryOpened(const QModelIndex& directoryIndex,
                       const QModelIndex& fileIndex);

  /**
   * Emitted when a confirmed opening of a directory or file is requested.
   * @param dir directory or file path
   */
  void confirmedOpenDirectoryRequested(const QString& dir);

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

  /**
   * Emitted before an operation on the selected files is performed.
   * The GUI should update the files of the current selection when
   * receiving this signal.
   */
  void fileSelectionUpdateRequested();

  /**
   * Emitted after an operation on the selected files has been performed.
   * The GUI should update its controls from the tags in the files when
   * receiving this signal.
   */
  void selectedFilesUpdated();

  /**
   * Emitted after a frame of a tagged file has been modified.
   * The GUI should update the corresponding controls when receiving this
   * signal.
   *
   * @param taggedFile tagged file with modified frame
   */
  void frameModified(TaggedFile* taggedFile);

  /**
   * Emitted after a file has been modified.
   * The GUI should update its modification state when receiving this signal.
   */
  void fileModified();

  /**
   * Emitted when setFilenameToTagsFormat() changed.
   * @param format new format
   */
  void filenameToTagsFormatChanged(const QString& format);

  /**
   * Emitted when setTagsToFilenameFormat() changed.
   * @param format new format
   */
  void tagsToFilenameFormatChanged(const QString& format);

  /**
   * Emitted when a file is filtered.
   * @param type filter event type
   * @param fileName name of filtered file
   */
  void fileFiltered(FileFilter::FilterEventType type,
                    const QString& fileName);

  /**
   * Emitted before an audio file is played.
   * The GUI can display a player when receiving this signal.
   */
  void aboutToPlayAudio();

private:
  /**
  * Init file types.
  */
  void initFileTypes();

  /**
   * Let the user select and edit a frame type and then edit the frame.
   * Add the frame if the edits are accepted.
   *
   * @param frameEditor frame editor
   *
   * @return true if edits accepted.
   */
  bool selectAddAndEditFrame(IFrameEditor* frameEditor);

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
  /** Frame list */
  FrameList* m_framelist;
  /** Configuration */
  ConfigStore* m_configStore;
  /** Network access manager */
  QNetworkAccessManager* m_netMgr;
  /** Download client */
  DownloadClient* m_downloadClient;
  /** Text exporter */
  TextExporter* m_textExporter;
  /** Directory renamer */
  DirRenamer* m_dirRenamer;
#ifdef HAVE_PHONON
  /** Audio player */
  AudioPlayer* m_player;
#endif
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
  /** Format to generate tags from filename */
  QString m_filenameToTagsFormat;
  /** Format to generate filename from tags */
  QString m_tagsToFilenameFormat;
  /** Importers for different servers */
  QList<ServerImporter*> m_importers;
  /** Importer for MusicBrainz fingerprints */
  MusicBrainzClient* m_musicBrainzClient;
  /** Current directory */
  static QString s_dirName;
};

#endif // KID3APPLICATION_H
