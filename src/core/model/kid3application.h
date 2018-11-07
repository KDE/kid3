/**
 * \file kid3application.h
 * Kid3 application logic, independent of GUI.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jul 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#pragma once

#include <QObject>
#include <QPersistentModelIndex>
#include <QScopedPointer>
#include "frame.h"
#include "trackdata.h"
#include "tagsearcher.h"
#include "generalconfig.h"
#include "config.h"

class QItemSelectionModel;
class QItemSelection;
class QModelIndex;
class QNetworkAccessManager;
class QDir;
class QUrl;
class FileSystemModel;
class FileProxyModel;
class FileProxyModelIterator;
class DirProxyModel;
class TrackDataModel;
class GenreModel;
class FrameTableModel;
class TaggedFileSelection;
class ConfigStore;
class PlaylistConfig;
class PlaylistModel;
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
class Kid3ApplicationTagContext;
class IAbortable;
class ICorePlatformTools;
class IUserCommandProcessor;
class AudioPlayer;
class PixmapProvider;
class FrameEditorObject;
class FileFilter;

/**
 * Kid3 application logic, independent of GUI.
 */
class KID3_CORE_EXPORT Kid3Application : public QObject {
  Q_OBJECT
  /** File proxy model. */
  Q_PROPERTY(FileProxyModel* fileProxyModel READ getFileProxyModel CONSTANT)
  /** Directory proxy model. */
  Q_PROPERTY(DirProxyModel* dirProxyModel READ getDirProxyModel CONSTANT)
  /** File selection model. */
  Q_PROPERTY(QItemSelectionModel* fileSelectionModel READ getFileSelectionModel CONSTANT)
  /** Directory selection model. */
  Q_PROPERTY(QItemSelectionModel* dirSelectionModel READ getDirSelectionModel CONSTANT)
  /** Information about selected tagged files. */
  Q_PROPERTY(TaggedFileSelection* selectionInfo READ selectionInfo CONSTANT)
  /** Root index of opened directory in file proxy model. */
  Q_PROPERTY(QModelIndex fileRootIndex READ getRootIndex NOTIFY fileRootIndexChanged)
  /** Root index of opened directory in directory proxy model. */
  Q_PROPERTY(QModelIndex dirRootIndex READ getDirRootIndex NOTIFY dirRootIndexChanged)
  /** Directory name. */
  Q_PROPERTY(QString dirName READ getDirName NOTIFY dirNameChanged)
  /** Modification state. */
  Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
  /** Filtered state. */
  Q_PROPERTY(bool filtered READ isFiltered WRITE setFiltered NOTIFY filteredChanged)
  /** Number of files which have passed the filter. */
  Q_PROPERTY(int filterPassedCount READ filterPassedCount NOTIFY fileFiltered)
  /** Total number of files checked by filter. */
  Q_PROPERTY(int filterTotalCount READ filterTotalCount NOTIFY fileFiltered)
  /** Frame editor. */
  Q_PROPERTY(FrameEditorObject* frameEditor READ frameEditor WRITE setFrameEditor
             NOTIFY frameEditorChanged)
  /** ID to get cover art image. */
  Q_PROPERTY(QString coverArtImageId READ coverArtImageId
             NOTIFY coverArtImageIdChanged)
  /** Directory renamer. */
  Q_PROPERTY(DirRenamer* dirRenamer READ getDirRenamer CONSTANT)
  /** Batch importer. */
  Q_PROPERTY(BatchImporter* batchImporter READ getBatchImporter CONSTANT)
  /** Download client */
  Q_PROPERTY(DownloadClient* downloadClient READ getDownloadClient CONSTANT)
  Q_FLAGS(NumberTrackOption NumberTrackOptions)
public:
  /** Destination for downloadImage(). */
  enum DownloadImageDestination {
    ImageForSelectedFiles,         /**< only for current file */
    ImageForAllFilesInDirectory, /**< for all files in directory */
    ImageForImportTrackData      /**< for enabled files in m_trackDataModel */
  };

  /** Options for numberTracks(). */
  enum NumberTrackOption {
    NumberTracksEnabled = 1,                     /**< Enable track numbering */
    NumberTracksResetCounterForEachDirectory = 2 /**< Reset counter */
  };
  Q_DECLARE_FLAGS(NumberTrackOptions, NumberTrackOption)

  /**
   * Constructor.
   * @param platformTools platform tools
   * @param parent parent object
   */
  explicit Kid3Application(ICorePlatformTools* platformTools,
                           QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~Kid3Application() override;

#ifdef HAVE_QTDBUS
  /**
   * Activate the D-Bus interface.
   * This method shall be called only once at initialization.
   */
  void activateDbusInterface();
#endif

 /**
  * Get file system model.
  * @return file system model.
  */
  FileSystemModel* getFileSystemModel() { return m_fileSystemModel; }

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
   * Get selection model of directories.
   */
  QItemSelectionModel* getDirSelectionModel() { return m_dirSelectionModel; }

  /**
   * Store index of directory from where "directory up" (..) is activated.
   * This directory will then be selected in the new (parent) directory.
   *
   * @param index model index of target directory entered when going up
   */
  void setDirUpIndex(const QPersistentModelIndex& index) {
    m_dirUpIndex = index;
  }

  /**
   * Get genre model.
   * @param tagNr tag number
   * @return genre model.
   */
  GenreModel*  genreModel(Frame::TagNumber tagNr) const { return m_genreModel[tagNr]; }

  /**
   * Get frame table model.
   * @param tagNr tag number
   * @return frame table.
   */
  FrameTableModel* frameModel(Frame::TagNumber tagNr) { return m_framesModel[tagNr]; }

  /**
   * Get selection model of frame table model.
   * @param tagNr tag number
   * @return selection model.
   */
  QItemSelectionModel* getFramesSelectionModel(Frame::TagNumber tagNr) {
    return m_framesSelectionModel[tagNr];
  }

  /**
   * Get frame list.
   * @param tagNr tag number
   * @return frame list.
   */
  FrameList* getFrameList(Frame::TagNumber tagNr) { return m_framelist[tagNr]; }

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
   * Get available user command processors.
   * @return list of user command processors.
   */
  QList<IUserCommandProcessor*> getUserCommandProcessors() {
    return m_userCommandProcessors;
  }

  /**
   * Get tag searcher.
   * @return tag searcher.
   */
  TagSearcher* getTagSearcher() const { return m_tagSearcher; }

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

  /**
   * Get audio player.
   * This method will create an audio player if it does not already exist.
   * The returned audio player can be deleted after this call, so objects which
   * hold a pointer must be deleted before deleteAudioPlayer() is called!
   * @return audio player.
   */
  Q_INVOKABLE AudioPlayer* getAudioPlayer();

  /**
   * Delete audio player.
   */
  void deleteAudioPlayer();

  /**
   * Get context for tag.
   * @param tagNr tag number
   * @return tag context.
   */
  Q_INVOKABLE Kid3ApplicationTagContext* tag(Frame::TagNumber tagNr) const {
    return m_tagContext[tagNr];
  }

  /**
   * Get current index in file proxy model or root index if current index is
   * invalid.
   * @return current index, root index if not valid.
   */
  QModelIndex currentOrRootIndex() const;

  /**
   * Apply configuration changes.
   */
  Q_INVOKABLE void applyChangedConfiguration();

  /**
   * Save settings to the configuration.
   */
  Q_INVOKABLE void saveConfig();

  /**
   * Read settings from the configuration.
   */
  Q_INVOKABLE void readConfig();

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
  Q_INVOKABLE bool openDirectory(const QStringList& paths, bool fileCheck = false);

  /**
   * Get root index of opened directory in file proxy model.
   * @return index of directory root.
   */
  QPersistentModelIndex getRootIndex() const {
    return m_fileProxyModelRootIndex;
  }

  /**
   * Get root index of opened directory in directory proxy model.
   * @return index of directory root.
   */
  QPersistentModelIndex getDirRootIndex() const {
    return m_dirProxyModelRootIndex;
  }

  /**
   * Get directory path of opened directory.
   * @return directory path.
   */
  QString getDirPath() const;

  /**
   * Handle drop of URLs.
   *
   * @param urlList picture, tagged file and folder URLs to handle (if local)
   * @param isInternal true if this is an internal drop
   */
  void dropUrls(const QList<QUrl>& urlList, bool isInternal);

  /**
   * Save all changed files.
   * longRunningOperationProgress() is emitted while saving files.
   *
   * @return list of files with error, empty if ok.
   */
  Q_INVOKABLE QStringList saveDirectory();

  /**
   * Update tags of selected files to contain contents of frame models.
   */
  Q_INVOKABLE void frameModelsToTags();

  /**
   * Update frame models to contain contents of selected files.
   * The properties starting with "selection" will be set by this method.
   */
  Q_INVOKABLE void tagsToFrameModels();

  /**
   * Update frame models to contain contents of item selection.
   * The properties starting with "selection" will be set by this method.
   * @param selected item selection
   */
  void selectedTagsToFrameModels(const QItemSelection& selected);

  /**
   * Access to information about selected tagged files.
   * @return selection information.
   */
  TaggedFileSelection* selectionInfo() const { return m_selection; }

  /**
   * Import.
   *
   * @param tagMask tag mask
   * @param path    path of file, "clipboard" for import from clipboard
   * @param fmtIdx  index of format
   *
   * @return true if ok.
   */
  Q_INVOKABLE bool importTags(Frame::TagVersion tagMask, const QString& path,
                              int fmtIdx);

  /**
   * Import from tags.
   *
   * @param tagMask tag mask
   * @param source format to get source text from tags
   * @param extraction regular expression with frame names and captures to
   * extract from source text
   *
   * @return true if ok.
   */
  Q_INVOKABLE void importFromTags(Frame::TagVersion tagMask,
                                  const QString& source,
                                  const QString& extraction);

  /**
   * Export.
   *
   * @param tagVersion tag version
   * @param path   path of file, "clipboard" for export to clipboard
   * @param fmtIdx index of format
   *
   * @return true if ok.
   */
  Q_INVOKABLE bool exportTags(Frame::TagVersion tagVersion,
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
   * Write empty playlist.
   * @param cfg playlist configuration to use
   * @param fileName file name for playlist
   * @return true if ok.
   */
  bool writeEmptyPlaylist(const PlaylistConfig& cfg, const QString& fileName);

  /**
   * Write playlist using current playlist configuration.
   *
   * @return true if ok.
   */
  Q_INVOKABLE bool writePlaylist();

  /**
   * Get items of a playlist.
   * @param path path to playlist file
   * @return list of absolute paths to playlist items.
   */
  Q_INVOKABLE QStringList getPlaylistItems(const QString& path);

  /**
   * Set items of a playlist.
   * @param path path to playlist file
   * @param items list of absolute paths to playlist items
   * @return true if ok, false if not all @a items were found and added or
   *         saving failed.
   */
  Q_INVOKABLE bool setPlaylistItems(const QString& path, const QStringList& items);

  /**
   * Get playlist model for a play list file.
   * @param path path to playlist file
   * @return playlist model.
   */
  PlaylistModel* playlistModel(const QString& path);

  /**
   * Check if any playlist model has unsaved modifications.
   * @return true if there is a modified playlist model.
   */
  bool hasModifiedPlaylistModel() const;

  /**
   * Save all modified playlist models.
   */
  void saveModifiedPlaylistModels();

  /**
   * Perform rename actions and change application directory afterwards if it
   * was renamed.
   *
   * @return error messages, null string if no error occurred.
   */
  Q_INVOKABLE QString performRenameActions();

  /**
   * Reset the file system model and then try to perform the rename actions.
   * On Windows, renaming directories fails when they have a subdirectory which
   * is open in the file system model. This method can be used to retry in such
   * a situation.
   */
  void tryRenameActionsAfterReset();

  /**
   * Reset the file system model and then try to rename a file.
   * On Windows, renaming directories fails when they have a subdirectory which
   * is open in the file system model. This method can be used to retry in such
   * a situation.
   *
   * @param oldName old file name
   * @param newName new file name
   */
  void tryRenameAfterReset(const QString& oldName, const QString& newName);

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
  Q_INVOKABLE bool renameDirectory(Frame::TagVersion tagMask,
                       const QString& format, bool create);

  /**
   * Number tracks in selected files of directory.
   *
   * @param nr start number
   * @param total total number of tracks, used if >0
   * @param tagVersion determines on which tags the numbers are set
   * @param options options for numbering operation
   */
  Q_INVOKABLE void numberTracks(int nr, int total, Frame::TagVersion tagVersion,
                                Kid3Application::NumberTrackOptions options = nullptr);

  /**
   * Set track data with tagged files of directory.
   *
   * @param tagVersion tag version
   * @param trackDataList is filled with track data
   */
  void filesToTrackData(Frame::TagVersion tagVersion,
                        ImportTrackDataVector& trackDataList);

  /**
   * Set track data model with tagged files of directory.
   *
   * @param tagVersion tag version
   */
  void filesToTrackDataModel(Frame::TagVersion tagVersion);

  /**
   * Set tagged files of directory from track data model.
   *
   * @param tagVersion tags to set
   */
  void trackDataModelToFiles(Frame::TagVersion tagVersion);

  /**
   * Download an image file.
   *
   * @param url  URL of image
   * @param dest specifies affected files
   */
  void downloadImage(const QUrl& url, DownloadImageDestination dest);

  /**
   * Download an image file.
   *
   * @param url URL of image
   * @param allFilesInDir true to add the image to all files in the directory
   */
  Q_INVOKABLE void downloadImage(const QString& url, bool allFilesInDir);

  /**
   * Get value of frame.
   * To get binary data like a picture, the name of a file to write can be
   * added after the @a name, e.g. "Picture:/path/to/file".
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   * @param name    name of frame (e.g. "artist")
   *
   * @return value of frame.
   */
  Q_INVOKABLE QString getFrame(Frame::TagVersion tagMask,
                               const QString& name) const;

  /**
   * Get names and values of all frames.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   *
   * @return map containing frame values.
   */
  Q_INVOKABLE QVariantMap getAllFrames(Frame::TagVersion tagMask) const;

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
   *
   * @return true if ok.
   */
  Q_INVOKABLE bool setFrame(Frame::TagVersion tagMask, const QString& name,
                            const QString& value);

  /**
   * Get data from picture frame.
   * @return picture data, empty if not found.
   */
  Q_INVOKABLE QByteArray getPictureData() const;

  /**
   * Set data in picture frame.
   * @param data picture data
   */
  Q_INVOKABLE void setPictureData(const QByteArray& data);

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
   * Add picture on drop.
   *
   * @param image dropped image.
   */
  void dropImage(const QImage& image);

  /**
   * Handle URL on drop.
   *
   * @param url dropped URL.
   */
  void dropUrl(const QUrl& url);

  /**
   * Get number of tracks in current directory.
   *
   * @return number of tracks, 0 if not found.
   */
  Q_INVOKABLE int getTotalNumberOfTracksInDir();

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
  Q_INVOKABLE QString createFilterString() const;


  /**
   * Remove the file filter if necessary to open the files.
   * @param filePaths paths to files or directories
   */
  void resetFileFilterIfNotMatching(const QStringList& filePaths);

  /**
   * Get image destination set by downloadImage().
   * @return image destination.
   */
  DownloadImageDestination getDownloadImageDestination() const {
    return m_downloadImageDest;
  }

  /**
   * Check modification state.
   *
   * @return true if a file is modified.
   */
  bool isModified() const;

  /**
   * Set filter state.
   *
   * @param val true if list is filtered
   */
  void setFiltered(bool val);

  /**
   * Check filter state.
   *
   * @return true if list is filtered.
   */
  bool isFiltered() const { return m_filtered; }

  /**
   * Get number of files which have passed the filter.
   *
   * This number is only valid if isFiltered() is true.
   *
   * @return number of files which have passed the filter.
   */
  int filterPassedCount() const { return m_filterPassed; }

  /**
   * Get total number of files which have been checked by the filter.
   *
   * This number is only valid if isFiltered() is true.
   *
   * @return total number of files checked by filter.
   */
  int filterTotalCount() const { return m_filterTotal; }

  /**
   * Get the selected file.
   *
   * @return the selected file,
   *         0 if not exactly one file is selected
   */
  TaggedFile* getSelectedFile();

  /**
   * Get the stored current selection.
   * @return stored selection.
   */
  const QList<QPersistentModelIndex>& getCurrentSelection() const {
    return m_currentSelection;
  }

  /**
   * Update the stored current selection with the list of all selected items.
   */
  void updateCurrentSelection();

  /**
   * Get directory name.
   * @return directory.
   */
  QString getDirName() const { return m_dirName; }

  /**
   * Get frame editor set with setFrameEditor().
   * @return frame editor, null if no frame editor is set
   */
  FrameEditorObject* frameEditor() const { return m_frameEditor; }

  /**
   * Set a frame editor object to act as the frame editor.
   * @param frameEditor frame editor object, null to disable
   */
  void setFrameEditor(FrameEditorObject* frameEditor);

  /**
   * Remove frame editor.
   * Has to be called in the destructor of the frame editor to avoid a dangling
   * pointer to a deleted object.
   * @param frameEditor frame editor
   */
  void removeFrameEditor(IFrameEditor* frameEditor);

  /**
   * Get ID to get cover art image.
   * @return ID for cover art image.
   */
  QString coverArtImageId() const { return m_coverArtImageId; }

  /**
   * Set the image provider.
   * @param imageProvider image provider
   */
  void setImageProvider(PixmapProvider* imageProvider);

  /**
   * Get the numbers of the selected rows in a list suitable for scripting.
   * @return list with row numbers.
   */
  Q_INVOKABLE QVariantList getFileSelectionRows();

  /**
   * Set the file selection from a list of model indexes.
   * @param indexes list of model indexes suitable for scripting
   */
  Q_INVOKABLE void setFileSelectionIndexes(const QVariantList& indexes);

  /**
   * Get paths to all selected files.
   * @param onlyTaggedFiles only consider tagged files
   * @return list of absolute file paths.
   */
  Q_INVOKABLE QStringList getSelectedFilePaths(bool onlyTaggedFiles = true)
  const;

  /**
   * Set picture data for image provider.
   * @param picture picture data
   */
  Q_INVOKABLE void setCoverArtImageData(const QByteArray& picture);

  /**
   * Open a file select dialog to get a file name.
   * For script support, is only supported when a GUI is available.
   * @param caption dialog caption
   * @param dir working directory
   * @param filter file type filter
   * @param saveFile true to open a save file dialog
   * @return selected file, empty if canceled.
   */
  Q_INVOKABLE QString selectFileName(
      const QString& caption = QString(), const QString& dir = QString(),
      const QString& filter = QString(), bool saveFile = false);

  /**
   * Open a file select dialog to get a directory name.
   * For script support, is only supported when a GUI is available.
   * @param caption dialog caption
   * @param dir working directory
   * @return selected directory, empty if canceled.
   */
  Q_INVOKABLE QString selectDirName(
      const QString& caption = QString(), const QString& dir = QString());

  /**
   * Notify the tagged file factories about the changed configuration.
   */
  static void notifyConfigurationChange();

  /**
   * Find directory containing plugins.
   * @param pluginsDir the plugin directory is returned here
   * @return true if found.
   */
  static bool findPluginsDirectory(QDir& pluginsDir);

  /**
   * Set fallback path for directory containing plugins.
   * @param path path to be searched for plugins if they are not found at the
   * standard location relative to the application directory
   */
  static void setPluginsPathFallback(const QString& path);

  /**
   * Load plugins.
   * @return list of plugin instances.
   */
  static QObjectList loadPlugins();

public slots:
  /**
   * Open directory or add pictures on drop.
   *
   * @param paths paths of directories or files in directory
   */
  void openDrop(const QStringList& paths);

  /**
   * Handle drop of URLs.
   *
   * @param urlList picture, tagged file and folder URLs to handle (if local)
   */
  void openDropUrls(const QList<QUrl>& urlList);

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
   * Apply tag format.
   */
  void applyTagFormat();

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
   * Copy tags into copy buffer.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void copyTags(Frame::TagVersion tagMask);

  /**
   * Paste from copy buffer to tags.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void pasteTags(Frame::TagVersion tagMask);

  /**
   * Set tag from other tag.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void copyToOtherTag(Frame::TagVersion tagMask);

  /**
   * Copy from a tag to another tag.
   * @param srcTagNr source tag number
   * @param dstTagNr destination tag number
   */
  void copyTag(Frame::TagNumber srcTagNr, Frame::TagNumber dstTagNr);

  /**
   * Remove tags in selected files.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void removeTags(Frame::TagVersion tagMask);

  /**
   * Set tags according to filename.
   *
   * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
   */
  void getTagsFromFilename(Frame::TagVersion tagMask);

  /**
   * Set filename according to tags.
   * If a single file is selected the tags in the GUI controls
   * are used, else the tags in the multiple selected files.
   *
   * @param tagVersion tag version
   */
  void getFilenameFromTags(Frame::TagVersion tagVersion);

  /**
   * Edit selected frame.
   * @param tagNr tag number
   */
  void editFrame(Frame::TagNumber tagNr);

  /**
   * Delete selected frame.
   * @param tagNr tag number
   * @param frameName name of frame to delete, empty to delete selected frame
   * @param index 0 for first frame with @a frameName, 1 for second, etc.
   */
  void deleteFrame(Frame::TagNumber tagNr,
                   const QString& frameName = QString(), int index = 0);

  /**
   * Select a frame type and add such a frame to the frame list.
   * @param tagNr tag number
   */
  void selectAndAddFrame(Frame::TagNumber tagNr);

  /**
   * Edit a picture frame if one exists or add a new one.
   */
  void editOrAddPicture();

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
   * @param onlyTaggedFiles only consider tagged files
   *
   * @return true if a file exists.
   */
  bool firstFile(bool select = true, bool onlyTaggedFiles = false);

  /**
   * Set the next file as the current file.
   *
   * @param select true to select the file
   * @param onlyTaggedFiles only consider tagged files
   *
   * @return true if a next file exists.
   */
  bool nextFile(bool select = true, bool onlyTaggedFiles = false);

  /**
   * Set the previous file as the current file.
   *
   * @param select true to select the file
   * @param onlyTaggedFiles only consider tagged files
   *
   * @return true if a previous file exists.
   */
  bool previousFile(bool select = true, bool onlyTaggedFiles = false);

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
   * Select all files in the current directory.
   */
  void selectAllInDirectory();

  /**
   * Set a specific file as the current file.
   *
   * @param filePath path to file
   * @param select true to select the file
   *
   * @return true if file exists.
   */
  bool selectFile(const QString& filePath, bool select = true);

  /**
   * Fetch entries of directory if not already fetched.
   * This works like FileList::expand(), but without expanding tree view
   * items and independent of the GUI. The processing is done in the background
   * by FileSystemModel, so the fetched items are not immediately available
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
   * Expand the whole file list if GUI available.
   */
  void requestExpandFileList();

  /**
   * Called when operation for requestExpandFileList() is finished.
   */
  void notifyExpandFileListFinished();

  /**
   * Process change of selection.
   * The GUI is signaled to update the current selection and the controls.
   * @param selected selected items
   * @param deselected deselected items
   */
  void fileSelected(const QItemSelection& selected,
                    const QItemSelection& deselected);

  /**
   * Search in tags for a given text.
   * @param params search parameters
   */
  void findText(const TagSearcher::Parameters& params);

  /**
   * Replace found text.
   * @param params search parameters
   */
  void replaceText(const TagSearcher::Parameters& params);

  /**
   * Replace all occurrences.
   * @param params search parameters
   */
  void replaceAll(const TagSearcher::Parameters& params);

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
   * Abort expression file filter.
   */
  void abortFilter();

  /**
   * Perform a batch import for the selected directories.
   * @param profile batch import profile
   * @param tagVersion import destination tag versions
   */
  void batchImport(const BatchImportProfile& profile,
                   Frame::TagVersion tagVersion);

  /**
   * Perform a batch import for the selected directories.
   * @param profileName batch import profile name
   * @param tagVersion import destination tag versions
   * @return true if profile with @a profileName found.
   */
  bool batchImport(const QString& profileName, Frame::TagVersion tagVersion);

  /**
   * Play audio file.
   */
  void playAudio();

  /**
   * Show play tool bar.
   */
  void showAudioPlayer();

#ifdef HAVE_QTDBUS
  /**
   * Activate the MPRIS D-Bus Interface if not already active.
   */
  void activateMprisInterface();

  /**
   * Deactivate the MPRIS D-Bus Interface if it is active.
   */
  void deactivateMprisInterface();
#endif

  /**
   * Close the file handle of a tagged file.
   * @param filePath path to file
   */
  void closeFileHandle(const QString& filePath);

signals:
  /**
   * Emitted when the file proxy model root index changes.
   * @param index new root index
   */
  void fileRootIndexChanged(const QModelIndex& index);

  /**
   * Emitted when the directory proxy model root index changes.
   * @param index new root index
   */
  void dirRootIndexChanged(const QModelIndex& index);

  /**
   * Emitted when the directory has been opened, the file and directory proxy
   * model root indexes changed and the selection updated.
   */
  void directoryOpened();

  /**
   * Emitted when a confirmed opening of a directory or file is requested.
   * @param paths directory or file paths
   */
  void confirmedOpenDirectoryRequested(const QStringList& paths);

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
   * Emitted after the file selection is changed.
   * The GUI should update its controls from the tags of the new file selection
   * when receiving this signal.
   * @param selected selected items
   * @param deselected deselected items
   */
  void selectedFilesChanged(const QItemSelection& selected,
                            const QItemSelection& deselected);

  /**
   * Emitted after a frame of a tagged file has been modified.
   * The GUI should update the corresponding controls when receiving this
   * signal.
   *
   * @param taggedFile tagged file with modified frame
   * @param tagNr tag number
   */
  void frameModified(TaggedFile* taggedFile, Frame::TagNumber tagNr);

  /**
   * Emitted when modification state is changed.
   * @param modified true if any file is modified
   * @see isModified()
   */
  void modifiedChanged(bool modified);

  /**
   * Emitted when filtered state is changed.
   * @param filtered true if file list is filtered
   * @see isFiltered(), setFiltered()
   */
  void filteredChanged(bool filtered);

  /**
   * Emitted when the directory name is changed.
   * @param name current directory name
   */
  void dirNameChanged(const QString& name);

  /**
   * Emitted when a file is filtered.
   * @param type filter event type, enum FileFilter::FilterEventType
   * @param fileName name of filtered file
   * @param passed number of files which passed the filter
   * @param total total number of files checked
   */
  void fileFiltered(int type, const QString& fileName, int passed, int total);

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

  /**
   * Emitted to request expanding of all directories in the file list.
   */
  void expandFileListRequested();

  /**
   * Emitted when operation requested by requestExpandFileList()
   * (signal expandFileListRequested()) is finished.
   */
  void expandFileListFinished();

  /**
   * Emitted when the file selection is changed.
   * @see getFileSelectionRows()
   */
  void fileSelectionChanged();

  /**
   * Emitted when a new cover art image is available
   * @param id ID of image.
   */
  void coverArtImageIdChanged(const QString& id);

  /**
   * Emitted when the frame editor is changed.
   */
  void frameEditorChanged();

  /**
   * Emitted to report progress about a long running operation.
   *
   * This signal is used to report different states of a long running operation.
   * - Operation is started: done is -1,
   * - Operation is finished: done == total and total is not 0,
   * - Operation in progress: done < total or both done and total are 0
   *
   * @param name name of operation
   * @param done amount of work done
   * @param total total amount of work
   * @param abort if not 0, can be set to true to abort the operation
   */
  void longRunningOperationProgress(const QString& name, int done, int total,
                                    bool* abort);

private slots:
  /**
   * Apply file filter after the file system model has been reset.
   */
  void applyFilterAfterReset();

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
   * Perform rename actions after the file system model has been reset.
   */
  void performRenameActionsAfterReset();

  /**
   * Rename after the file system model has been reset.
   */
  void renameAfterReset();

  /**
   * Update selection and emit signals when directory is opened.
   */
  void onDirectoryOpened();

  /**
   * Called when the gatherer thread has finished to load the directory.
   */
  void onDirectoryLoaded();

  /**
   * Called when a frame is edited.
   * @param frame edited frame, 0 if canceled
   */
  void onFrameEdited(const Frame* frame);

  /**
   * Called when a frame is added.
   * @param frame added frame, 0 if canceled
   * @param tagNr tag number used if slot is not invoked by framelist signal
   */
  void onFrameAdded(const Frame* frame, Frame::TagNumber tagNr = Frame::Tag_2);

  /**
   * Called by framelist when a frame is added.
   * Same as onFrameAdded() with default argument, provided for functor-based
   * connections.
   * @param frame added frame, 0 if canceled
   */
  void onTag2FrameAdded(const Frame* frame);

  /**
   * If an image provider is used, update its picture and change the
   * coverArtImageId property if the picture of the selection changed.
   * This can be used to change a QML image.
   */
  void updateCoverArtImageId();

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
   * Update frame models to contain contents of selected files.
   * @param indexes tagged file indexes
   * @param startSelection true if a new selection is started, false to add to
   * the existing selection
   * @return true if ok, false if selection operation is already running.
   */
  bool addTaggedFilesToSelection(
      const QList<QPersistentModelIndex>& indexes, bool startSelection);

  /**
   * Select a frame type and add such a frame to frame list.
   * @param tagNr tag number
   * @param frame frame to add, if 0 the user has to select and edit the frame
   * @param edit if true and a frame is set, the user can edit the frame before
   * it is added
   */
  void addFrame(Frame::TagNumber tagNr, const Frame* frame, bool edit = false);

  /**
   * Open directory after resetting the file system model.
   * This will create a new file system model and reset the file and directory
   * proxy models.
   * When finished directoryOpened() is emitted, also if false is returned.
   *
   * @param paths file or directory paths, if multiple paths are given, the
   * common directory is opened and the files are selected, if empty, the
   * currently open directory is reopened
   *
   * @return true if ok.
   */
  bool openDirectoryAfterReset(const QStringList& paths = QStringList());

  /**
   * Open directory or add pictures on drop.
   *
   * @param paths paths of directories or files in directory
   * @param isInternal true if this is an internal drop
   */
  void dropLocalFiles(const QStringList& paths, bool isInternal);

  /**
   * Second stage for applyFilter().
   */
  void proceedApplyingFilter();

  /**
   * Set the coverArtImageId property to a new value.
   * This can be used to trigger an update of QML images.
   */
  void setNextCoverArtImageId();

  void setAllFilesFileFilter();

  ICorePlatformTools* m_platformTools;
  /** Configuration */
  QScopedPointer<ConfigStore> m_configStore;
  /** model of filesystem */
  FileSystemModel* m_fileSystemModel;
  FileProxyModel* m_fileProxyModel;
  FileProxyModelIterator* m_fileProxyModelIterator;
  DirProxyModel* m_dirProxyModel;
  QItemSelectionModel* m_fileSelectionModel;
  QItemSelectionModel* m_dirSelectionModel;
  /** Track data model */
  TrackDataModel* m_trackDataModel;
  GenreModel* m_genreModel[Frame::Tag_NumValues];
  FrameTableModel* m_framesModel[Frame::Tag_NumValues];
  QItemSelectionModel* m_framesSelectionModel[Frame::Tag_NumValues];
  QMap<QString, PlaylistModel*> m_playlistModels;
  /** Frame list */
  FrameList* m_framelist[Frame::Tag_NumValues];
  /** Tag context */
  Kid3ApplicationTagContext* m_tagContext[Frame::Tag_NumValues];
  /** Network access manager */
  QNetworkAccessManager* m_netMgr;
  /** Download client */
  DownloadClient* m_downloadClient;
  /** Text exporter */
  TextExporter* m_textExporter;
  /** Tag searcher */
  TagSearcher* m_tagSearcher;
  /** Directory renamer */
  DirRenamer* m_dirRenamer;
  /** Batch importer */
  BatchImporter* m_batchImporter;
  /** Audio player */
  AudioPlayer* m_player;
#ifdef HAVE_QTDBUS
  QString m_mprisServiceName;
#endif
  FileFilter* m_expressionFileFilter;
  /** Information about selected tagged files */
  TaggedFileSelection* m_selection;
  /** Affected files to add frame when downloading image */
  DownloadImageDestination m_downloadImageDest;
  /** Copy buffer */
  FrameCollection m_copyTags;
  /** Root index in file proxy model */
  QPersistentModelIndex m_fileProxyModelRootIndex;
  /** Root index in directory proxy model */
  QPersistentModelIndex m_dirProxyModelRootIndex;
  /** Indexes of opened file in file proxy model */
  QList<QPersistentModelIndex> m_fileProxyModelFileIndexes;
  /** Importers for different servers */
  QList<ServerImporter*> m_importers;
  /** Importer for MusicBrainz fingerprints */
  QList<ServerTrackImporter*> m_trackImporters;
  /** Processors for user commands */
  QList<IUserCommandProcessor*> m_userCommandProcessors;
  /** Current directory */
  QString m_dirName;
  /** Stored current selection with the list of all selected items */
  QList<QPersistentModelIndex> m_currentSelection;
  /** directory from where "directory up" (..) was activated. */
  QPersistentModelIndex m_dirUpIndex;

  /* Context for filterNextFile() */
  FileFilter* m_fileFilter;
  QString m_lastProcessedDirName;
  int m_filterPassed;
  int m_filterTotal;
  /* Context for batchImportNextFile() */
  QScopedPointer<BatchImportProfile> m_namedBatchImportProfile;
  const BatchImportProfile* m_batchImportProfile;
  Frame::TagVersion m_batchImportTagVersion;
  QList<ImportTrackDataVector> m_batchImportAlbums;
  ImportTrackDataVector m_batchImportTrackDataList;

  /* Context for renameAfterReset() */
  QString m_renameAfterResetOldName;
  QString m_renameAfterResetNewName;

  /* Context for editFrame() */
  TaggedFile* m_editFrameTaggedFile;
  QString m_editFrameName;

  /* Context for addFrame() */
  TaggedFile* m_addFrameTaggedFile;

  /* Support for frame editor object */
  FrameEditorObject* m_frameEditor;
  IFrameEditor* m_storedFrameEditor;
  /* Support for image provider */
  PixmapProvider* m_imageProvider;
  QString m_coverArtImageId;

#ifdef HAVE_QTDBUS
  /** true if D-Bus is enabled */
  bool m_dbusEnabled;
#endif
  /** true if list is filtered */
  bool m_filtered;
  /** true if a selection operation is running */
  bool m_selectionOperationRunning;

  /** Fallback for path to search for plugins */
  static QString s_pluginsPathFallback;
};

/**
 * Facade to have a uniform interface for different tags.
 */
class KID3_CORE_EXPORT Kid3ApplicationTagContext : public QObject {
  Q_OBJECT
  /** Genre model. */
  Q_PROPERTY(GenreModel* genreModel READ genreModel CONSTANT)
  /** Frame table model. */
  Q_PROPERTY(FrameTableModel* frameModel READ frameModel CONSTANT)
  /** Frame selection model. */
  Q_PROPERTY(QItemSelectionModel* frameSelectionModel READ frameSelectionModel CONSTANT)
  /** Frame list. */
  Q_PROPERTY(FrameList* frameList READ frameList CONSTANT)
public:
  /**
   * Constructor.
   * @param app application
   * @param tagNr tag number
   */
  Kid3ApplicationTagContext(Kid3Application* app, Frame::TagNumber tagNr)
    : QObject(app), m_app(app), m_tagNr(tagNr),
      m_tagVersion(Frame::tagVersionFromNumber(tagNr)) {
  }

public slots:
  /**
   * Copy tags into copy buffer.
   */
  void copyTags() { m_app->copyTags(m_tagVersion); }

  /**
   * Paste from copy buffer to tags.
   */
  void pasteTags() { m_app->pasteTags(m_tagVersion); }

  /**
   * Copy tags to other tags of selected files.
   */
  void copyToOtherTag() { m_app->copyToOtherTag(m_tagVersion); }

  /**
   * Remove tags in selected files.
   */
  void removeTags() { m_app->removeTags(m_tagVersion); }

  /**
   * Set tags from filename.
   */
  void getTagsFromFilename() { m_app->getTagsFromFilename(m_tagVersion); }

  /**
   * Set filename from tags.
   */
  void getFilenameFromTags() { m_app->getFilenameFromTags(m_tagVersion); }

  /**
   * Edit selected frame of tag.
   */
  void editFrame() { m_app->editFrame(m_tagNr); }

  /**
   * Delete selected frame from tag.
   */
  void deleteFrame() { m_app->deleteFrame(m_tagNr); }

  /**
   * Select a frame type and add such a frame to the frame list.
   */
  void addFrame() { m_app->selectAndAddFrame(m_tagNr); }

private:
  /**
   * Get genre model.
   * @return genre model.
   */
  GenreModel* genreModel() const { return m_app->genreModel(m_tagNr); }

  /**
   * Get frame table model.
   * @return frame table.
   */
  FrameTableModel* frameModel() { return m_app->frameModel(m_tagNr); }

  /**
   * Get selection model of frame table model.
   */
  QItemSelectionModel* frameSelectionModel() {
    return m_app->getFramesSelectionModel(m_tagNr);
  }

  /**
   * Get frame list.
   */
  FrameList* frameList() { return m_app->getFrameList(m_tagNr); }

  Kid3Application* const m_app;
  const Frame::TagNumber m_tagNr;
  const Frame::TagVersion m_tagVersion;
};
