/**
 * \file fileproxymodel.h
 * Proxy for filesystem model which filters files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22-Mar-2011
 *
 * Copyright (C) 2011-2014  Urs Fleisch
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

#ifndef FILEPROXYMODEL_H
#define FILEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QHash>
#include <QSet>
#include <QFileInfo>
#include <QStringList>
#include "taggedfile.h"
#include "kid3api.h"

class QFileSystemModel;
class QTimer;
class TaggedFileIconProvider;
class ITaggedFileFactory;

/**
 * Proxy for filesystem model which filters files.
 */
class KID3_CORE_EXPORT FileProxyModel : public QSortFilterProxyModel {
  Q_OBJECT
public:
  /** Custom role, extending QFileSystemModel::Roles. */
  enum Roles {
    TaggedFileRole = Qt::UserRole + 4,
    IconIdRole = Qt::UserRole + 5,
    TruncatedRole =  Qt::UserRole + 6,
    IsDirRole = Qt::UserRole + 7
  };

  /**
   * Constructor.
   *
   * @param parent parent object
   */
  explicit FileProxyModel(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~FileProxyModel();

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  virtual QVariant data(const QModelIndex& index,
                        int role=Qt::DisplayRole) const;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role=Qt::EditRole);

  /**
   * Set source model.
   * @param sourceModel source model, must be QFileSystemModel
   */
  virtual void setSourceModel(QAbstractItemModel* sourceModel);

  /**
   * Check if more data is available.
   * @param parent parent index of items to fetch
   * @return true if more data available.
   */
  virtual bool canFetchMore(const QModelIndex& parent) const;

  /**
   * Fetches any available data.
   * @param parent parent index of items to fetch
   */
  virtual void fetchMore(const QModelIndex& parent);

  /**
   * Sort model.
   *
   * This method will directly call QFileSystemModel::sort() on the
   * sourceModel() to take advantage of that specialized behavior. This
   * will change the order in the souce model.
   *
   * @param column column to sort
   * @param order ascending or descending order
   */
  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

#if QT_VERSION >= 0x050000
  /**
   * Map role identifiers to role property names in scripting languages.
   * @return hash mapping role identifiers to names.
   */
  virtual QHash<int, QByteArray> roleNames() const;
#endif

  /**
   * Check if the model is currently loading a directory.
   * @return true if loading is in progress.
   */
  bool isLoading() const { return m_isLoading; }

  /**
   * Sets the name filters to apply against the existing files.
   * @param filters list of strings containing wildcards like "*.mp3"
   */
  void setNameFilters(const QStringList& filters);

  /**
   * Filter out a model index.
   * @param index model index which has to be filtered out
   */
  void filterOutIndex(const QPersistentModelIndex& index);

  /**
   * Stop filtering out indexes.
   */
  void disableFilteringOutIndexes();

  /**
   * Check if index filter is active.
   * @return true if indexes are filtered out
   */
  bool isFilteringOutIndexes() const;

  /**
   * Make filter changes active after adding indexes to be filtered out.
   */
  void applyFilteringOutIndexes();

  /**
   * Set filters for included and excluded folders.
   * @param includeFolders wildcard expressions for folders to be included
   * @param excludeFolders wildcard expressions for folders to be excluded
   */
  void setFolderFilters(const QStringList& includeFolders,
                        const QStringList& excludeFolders);

  /**
   * Get file information of model index.
   * @return file information
   */
  QFileInfo fileInfo(const QModelIndex& index) const;

  /**
   * Get file path of model index.
   * @return path to file or directory
   */
  QString filePath(const QModelIndex& index) const;

  /**
   * Get file name of model index.
   * @return name of file or directory
   */
  QString fileName(const QModelIndex& index) const;

  /**
   * Check if model index represents directory.
   * @return true if directory
   */
  bool isDir(const QModelIndex& index) const;

  /**
   * Delete file of index.
   * @return true if ok
   */
  bool remove(const QModelIndex& index) const;

  /**
   * Delete directory of index.
   * @return true if ok
   */
  bool rmdir(const QModelIndex& index) const;

  /**
   * Get index for given path and column.
   * @param path path to file or directory
   * @param column model column
   * @return model index, invalid if not found.
   */
  QModelIndex index(const QString& path, int column = 0) const;

  using QSortFilterProxyModel::index;

  /**
   * Called from tagged file to notify modification state changes.
   * @param index model index
   * @param modified true if file is modified
   */
  void notifyModificationChanged(const QModelIndex& index, bool modified);

  /**
   * Called from tagged file to notify changes in extra model data, e.g. the
   * information on which the TaggedFileIconProvider depends.
   * @param index model index
   */
  void notifyModelDataChanged(const QModelIndex& index);

  /**
   * Check if any file has been modified.
   * @return true if at least one of the files in the model has been modified.
   */
  bool isModified() const { return m_numModifiedFiles > 0; }

  /**
   * Get icon provider.
   * @return icon provider.
   */
  TaggedFileIconProvider* getIconProvider() const { return m_iconProvider; }

  /**
   * Access to tagged file factories.
   * @return reference to tagged file factories.
   */
  static QList<ITaggedFileFactory*>& taggedFileFactories() {
    return s_taggedFileFactories;
  }

  /**
   * Create a tagged file with a given feature.
   *
   * @param feature tagged file feature
   * @param fileName filename
   * @param idx model index
   *
   * @return tagged file, 0 if feature not found or type not supported.
   */
  static TaggedFile* createTaggedFile(
      TaggedFile::Feature feature,
      const QString& fileName,
      const QPersistentModelIndex& idx);

  /**
   * Create a tagged file.
   *
   * @param fileName filename
   * @param idx model index
   *
   * @return tagged file, 0 if not found or type not supported.
   */
  static TaggedFile* createTaggedFile(
      const QString& fileName,
      const QPersistentModelIndex& idx);

  /**
   * Get tagged file data of model index.
   *
   * @param index model index
   * @param taggedFile a TaggedFile pointer is returned here
   *
   * @return true if index has a tagged file, *taggedFile is set to the pointer.
   */
  static bool getTaggedFileOfIndex(const QModelIndex& index,
                                   TaggedFile** taggedFile);

  /**
   * Get tagged file of model index.
   *
   * @param index model index
   *
   * @return tagged file, 0 is returned if the index does not contain a
   * TaggedFile or if has a TaggedFile which is null.
   */
  static TaggedFile* getTaggedFileOfIndex(const QModelIndex& index);

  /**
   * Get directory path if model index is of directory.
   *
   * @param index model index
   *
   * @return directory path, null if not directory
   */
  static QString getPathIfIndexOfDir(const QModelIndex& index);

  /**
   * Read tagged file with ID3v2.4.0.
   *
   * @param taggedFile tagged file
   *
   * @return tagged file (can be newly created tagged file).
   */
  static TaggedFile* readWithId3V24(TaggedFile* taggedFile);

  /**
   * Read tagged file with ID3v2.3.0.
   *
   * @param taggedFile tagged file
   *
   * @return tagged file (can be newly created tagged file).
   */
  static TaggedFile* readWithId3V23(TaggedFile* taggedFile);

  /**
   * Read file with ID3v2.4 if it has an ID3v2.4 or ID3v2.2 tag.
   * ID3v2.2 files are also read with ID3v2.4 because id3lib corrupts
   * images in ID3v2.2 tags.
   *
   * @param taggedFile tagged file
   *
   * @return tagged file (can be new TagLibFile).
   */
  static TaggedFile* readWithId3V24IfId3V24(TaggedFile* taggedFile);

  /**
   * Read tagged file with Ogg FLAC.
   *
   * @param taggedFile tagged file
   *
   * @return tagged file (can be newly created tagged file).
   */
  static TaggedFile* readWithOggFlac(TaggedFile* taggedFile);

  /**
   * Try to read Ogg file with invalid tag detail info as an Ogg FLAC file.
   *
   * @param taggedFile tagged file
   *
   * @return tagged file (can be new TagLibFile).
   */
  static TaggedFile* readWithOggFlacIfInvalidOgg(TaggedFile* taggedFile);

  /**
   * Call readTags() on tagged file.
   * Reread file with other metadata plugin if it is not supported by current
   * plugin.
   *
   * @param taggedFile tagged file
   *
   * @return tagged file (can be new TaggedFile).
   */
  static TaggedFile* readTagsFromTaggedFile(TaggedFile* taggedFile);

  /**
   * Create name-file pattern pairs for all supported types.
   * The order is the same as in createFilterString().
   *
   * @return pairs containing name, pattern, e.g. ("MP3", "*.mp3"), ...,
   * ("All Files", "*").
   */
  static QList<QPair<QString, QString> > createNameFilters();

signals:
  /**
   * Emitted after directory loading when sorting is probably finished.
   * This signal is not accurate, it will be emitted 100 ms after
   * directoryLoaded() because it is not known when sorting is really finished.
   */
  void sortingFinished();

  /**
   * Emitted when the modification state of a file changes.
   * @param index model index
   * @param modified true if file is modified
   */
  void fileModificationChanged(const QModelIndex& index, bool modified);

  /**
   * Emitted when modification state changes.
   * @param modified true if any file has been modified
   * @see isModified()
   */
  void modifiedChanged(bool modified);

private slots:
  /**
   * Update the TaggedFile contents for rows inserted into the model.
   * @param parent parent model index
   * @param start starting row
   * @param end ending row
   */
  void updateInsertedRows(const QModelIndex& parent, int start, int end);

  /**
   * Called when the source model emits directoryLoaded().
   */
  void onDirectoryLoaded();

  /**
   * Emit sortingFinished().
   */
  void emitSortingFinished();

  /**
   * Called when loading the directory starts.
   */
  void onStartLoading();

protected:
  /**
   * Check if row should be included in model.
   *
   * @param srcRow source row
   * @param srcParent source parent
   *
   * @return true to include row.
   */
  virtual bool filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const;

private:
  /**
   * Retrieve tagged file for an index.
   * @param index model index
   * @return QVariant with tagged file, invalid QVariant if not found.
   */
  QVariant retrieveTaggedFileVariant(const QPersistentModelIndex& index) const;

  /**
   * Store tagged file from variant with index.
   * @param index model index
   * @param value QVariant containing tagged file
   * @return true if index and value valid
   */
  bool storeTaggedFileVariant(const QPersistentModelIndex& index,
                              QVariant value);

  /**
   * Clear store with tagged files.
   */
  void clearTaggedFileStore();

  /**
   * Initialize tagged file for model index.
   * @param index model index
   */
  void initTaggedFileData(const QModelIndex& index);

  /**
   * Check if a directory path passes the folder filters.
   * @param dirPath absolute path to directory
   * @return true if path passes filters.
   */
  bool passesFolderFilters(const QString& dirPath) const;

  QHash<QPersistentModelIndex, TaggedFile*> m_taggedFiles;
  QSet<QPersistentModelIndex> m_filteredOut;
  QList<QRegExp> m_includeFolderFilters;
  QList<QRegExp> m_excludeFolderFilters;
  TaggedFileIconProvider* m_iconProvider;
  QFileSystemModel* m_fsModel;
  QTimer* m_loadTimer;
  QTimer* m_sortTimer;
  QStringList m_extensions;
  unsigned int m_numModifiedFiles;
  bool m_isLoading;

  static QList<ITaggedFileFactory*> s_taggedFileFactories;
};

Q_DECLARE_METATYPE(TaggedFile*)

#endif // FILEPROXYMODEL_H
