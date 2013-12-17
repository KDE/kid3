/**
 * \file kid3application.h
 * Kid3 application logic, independent of GUI.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jul 2011
 *
 * Copyright (C) 2011-2012  Urs Fleisch
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
class QTimer;
class FileProxyModel;
class FileProxyModelIterator;
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
class ServerTrackImporter;
class ITaggedFileFactory;
class TextExporter;
class DirRenamer;
class BatchImportProfile;
class BatchImporter;
class IAbortable;
class ICorePlatformTools;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
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
   * @param platformTools platform tools
   * @param parent parent object
   */
  explicit Kid3Application(ICorePlatformTools* platformTools,
                           QObject* parent = 0);

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
  * Get file proxy model iterator.
  * @return file proxy model iterator.
   */
  FileProxyModelIterator* getFileProxyModelIterator() {
    return m_fileProxyModelIterator;
  }

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
  ISettings* getSettings() const;

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
   * Get available server track importers.
   * @return list of server track importers.
   */
  QList<ServerTrackImporter*> getServerTrackImporters() {
    return m_trackImporters;
  }

  /**
   * Get directory renamer.
   * @return directory renamer.
   */
  DirRenamer* getDirRenamer() { return m_dirRenamer; }

  /**
   * Get batch importer.
   * @return batch importer.
   */
  BatchImporter* getBatchImporter() { return m_batchImporter; }

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
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
   * When finished directoryOpened() is emitted, also if false is returned.
   *
   * @param paths file or directory paths, if multiple paths are given, the
   * common directory is opened and the files are selected
   * @param fileCheck if true, only open directory if paths exist
   *
   * @return true if ok.
   */
  bool openDirectory(const QStringList& paths, bool fileCheck = false);

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
   * Update tags of selected files to contain contents of frame models.
   *
   * @param selItems list of selected file indexes
   */
  void frameModelsToTags(const QList<QPersistentModelIndex>& selItems);

  /**
   * Update frame models to contain contents of selected files.
   * The properties starting with "selection" will be set by this method.
   *
   * @param selItems list of selected file indexes
   */
  void tagsToFrameModels(const QList<QPersistentModelIndex>& selItems);

  /**
   * Check if a single file is selected.
   * @return if a single file is selected, this tagged file, else 0.
   */
  const TaggedFile* selectionSingleFile() const {
    return m_selectionSingleFile;
  }

  /**
   * Number of selected files which support tag 1.
   * @return number of selected files which support tag 1.
   */
  int selectionTagV1SupportedCount() const {
    return m_selectionTagV1SupportedCount;
  }

  /**
   * Number of selected files.
   * @return number of selected files which support tag 2.
   */
  int selectionFileCount() const {
    return m_selectionFileCount;
  }

  /**
   * Check if any of the selected files has a tag 1.
   * @return true if any of the selected files has a tag 1.
   */
  bool selectionHasTagV1() const { return m_selectionHasTagV1; }

  /**
   * Check if any of the selected files has a tag 2.
   * @return true if any of the selected files has a tag 2.
   */
  bool selectionHasTagV2() const { return m_selectionHasTagV2; }

  /**
   * Import.
   *
   * @param tagMask tag mask
   * @param path    path of file, "clipboard" for import from clipboard
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
   * @param path   path of file, "clipboard" for export to clipboard
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
   * renameActionsScheduled() is emitted when the rename actions have been
   * scheduled. Then performRenameActions() has to be called to effectively
   * rename the directory.
   *
   * @param tagMask tag mask
   * @param format  directory name format
   * @param create  true to create, false to rename
   *
   * @return true if ok.
   */
  bool renameDirectory(TrackData::TagVersion tagMask,
                       const QString& format, bool create);

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
   * Get value of frame.
   * To get binary data like a picture, the name of a file to write can be
   * added after the @a name, e.g. "Picture:/path/to/file".
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   * @param name    name of frame (e.g. "artist")
   */
  QString getFrame(TrackData::TagVersion tagMask, const QString& name);

  /**
   * Set value of frame.
   * For tag 2 (@a tagMask 2), if no frame with @a name exists, a new frame
   * is added, if @a value is empty, the frame is deleted.
   * To add binary data like a picture, a file can be added after the
   * @a name, e.g. "Picture:/path/to/file".
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   * @param name    name of frame (e.g. "artist")
   * @param value   value of frame
   */
  bool setFrame(TrackData::TagVersion tagMask, const QString& name,
                const QString& value);

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
   * Open directory or add pictures on drop.
   *
   * @param paths paths of directories or files in directory
   */
  void openDrop(const QStringList& paths);

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
   * Update modification state from files.
   */
  void updateModified();

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
   * Get format used to generate filename from tags.
   * @return format
   */
  QString getFilenameToTagsFormat() const {
    return m_filenameToTagsFormat;
  }

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
   * Notify the tagged file factories about the changed configuration.
   */
  static void notifyConfigurationChange();

  /**
   * Load plugins.
   * @return list of plugin instances.
   */
  static QObjectList loadPlugins();

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

  /**
   * Apply text encoding.
   * Set the text encoding selected in the settings Tags/ID3v2/Text encoding
   * for all selected files which have an ID3v2 tag.
   */
  void applyTextEncoding();

  /**
   * Convert ID3v2.3 to ID3v2.4 tags.
   */
  void convertToId3v24();

  /**
   * Convert ID3v2.4 to ID3v2.3 tags.
   */
  void convertToId3v23();

  /**
   * Copy tags 1 into copy buffer.
   */
  void copyTagsV1();

  /**
   * Copy tags 2 into copy buffer.
   */
  void copyTagsV2();

  /**
   * Copy tags into copy buffer.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void copyTags(TrackData::TagVersion tagMask);

  /**
   * Paste from copy buffer to ID3v1 tags.
   */
  void pasteTagsV1();

  /**
   * Paste from copy buffer to ID3v2 tags.
   */
  void pasteTagsV2();

  /**
   * Paste from copy buffer to tags.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void pasteTags(TrackData::TagVersion tagMask);

  /**
   * Copy ID3v1 tags to ID3v2 tags of selected files.
   */
  void copyV1ToV2();

  /**
   * Copy ID3v2 tags to ID3v1 tags of selected files.
   */
  void copyV2ToV1();

  /**
   * Set tag from other tag.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void copyToOtherTag(TrackData::TagVersion tagMask);

  /**
   * Remove ID3v1 tags in selected files.
   */
  void removeTagsV1();

  /**
   * Remove ID3v2 tags in selected files.
   */
  void removeTagsV2();

  /**
   * Remove tags in selected files.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void removeTags(TrackData::TagVersion tagMask);

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
   * Set tags according to filename.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void getTagsFromFilename(TrackData::TagVersion tagMask);

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
   * Fetch entries of directory and toggle expanded state if GUI available.
   * @param index index of directory item
   */
  void expandDirectory(const QModelIndex& index);

  /**
   * Process change of selection.
   * The GUI is signaled to update the current selection and the controls.
   */
  void fileSelected();

  /**
   * Schedule actions to rename a directory.
   * When finished renameActionsScheduled() is emitted.
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

  /**
   * Perform a batch import for the selected directories.
   * @param profile batch import profile
   * @param tagVersion import destination tag versions
   */
  void batchImport(const BatchImportProfile& profile,
                   TrackData::TagVersion tagVersion);

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /**
   * Play audio file.
   */
  void playAudio();
#endif

signals:
  /**
   * Emitted when a new directory is opened.
   * @param directoryIndex root path file proxy model index
   * @param fileIndexes file path indexes in the file proxy model
   */
  void directoryOpened(const QPersistentModelIndex& directoryIndex,
                       const QList<QPersistentModelIndex>& fileIndexes);

  /**
   * Emitted when a confirmed opening of a directory or file is requested.
   * @param paths directory or file paths
   */
  void confirmedOpenDirectoryRequested(const QStringList& paths);

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

  /**
   * Emitted when all rename actions have been scheduled.
   * @see scheduleRenameActions()
   */
  void renameActionsScheduled();

  /**
   * Emitted to request toggling of the expanded state of a directory in the
   * file list.
   * @param index index of directory item
   */
  void toggleExpandedRequested(const QModelIndex& index);

private slots:
  /**
   * Apply single file to file filter.
   *
   * @param index index of file in file proxy model
   */
  void filterNextFile(const QPersistentModelIndex& index);

  /**
   * Apply single file to batch import.
   *
   * @param index index of file in file proxy model
   */
  void batchImportNextFile(const QPersistentModelIndex& index);

  /**
   * Schedule rename action for a file.
   *
   * @param index index of file in file proxy model
   */
  void scheduleNextRenameAction(const QPersistentModelIndex& index);

  /**
   * Emit directoryOpened().
   */
  void emitDirectoryOpened();

  /**
   * Called when the gatherer thread has finished to load the directory.
   */
  void onDirectoryLoaded();

private:
  /**
   * Load and initialize plugins depending on configuration.
   */
  void initPlugins();

  /**
   * Check type of a loaded plugin and register it.
   * @param plugin instance returned by plugin loader
   */
  void checkPlugin(QObject* plugin);

  /**
   * Let the user select and edit a frame type and then edit the frame.
   * Add the frame if the edits are accepted.
   *
   * @param frameEditor frame editor
   *
   * @return true if edits accepted.
   */
  bool selectAddAndEditFrame(IFrameEditor* frameEditor);

  ICorePlatformTools* m_platformTools;
  /** model of filesystem */
  QFileSystemModel* m_fileSystemModel;
  FileProxyModel* m_fileProxyModel;
  FileProxyModelIterator* m_fileProxyModelIterator;
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
  /** Batch importer */
  BatchImporter* m_batchImporter;
  /** Timeout timer for openDirectory() */
  QTimer* m_openDirectoryTimeoutTimer;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /** Audio player */
  AudioPlayer* m_player;
#endif
  FileFilter* m_expressionFileFilter;
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
  /** Indexes of opened file in file proxy model */
  QList<QPersistentModelIndex> m_fileProxyModelFileIndexes;
  /** Format to generate tags from filename */
  QString m_filenameToTagsFormat;
  /** Format to generate filename from tags */
  QString m_tagsToFilenameFormat;
  /** Importers for different servers */
  QList<ServerImporter*> m_importers;
  /** Importer for MusicBrainz fingerprints */
  QList<ServerTrackImporter*> m_trackImporters;

  /* Context for updateFrameModels() */
  /** If a single file is selected, this tagged file, else 0 */
  TaggedFile* m_selectionSingleFile;
  /** Number of selected files which support tag 1 */
  int m_selectionTagV1SupportedCount;
  /** Number of selected files */
  int m_selectionFileCount;
  /** true if any of the selected files has a tag 1 */
  bool m_selectionHasTagV1;
  /** true if any of the selected files has a tag 2 */
  bool m_selectionHasTagV2;

  /* Context for filterNextFile() */
  FileFilter* m_fileFilter;
  QString m_lastProcessedDirName;
  /* Context for batchImportNextFile() */
  const BatchImportProfile* m_batchImportProfile;
  TrackData::TagVersion m_batchImportTagVersion;
  QList<ImportTrackDataVector> m_batchImportAlbums;
  ImportTrackDataVector m_batchImportTrackDataList;

  /** Current directory */
  static QString s_dirName;
};

#endif // KID3APPLICATION_H
