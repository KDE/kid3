/**
 * \file fileproxymodel.cpp
 * Proxy for filesystem model which filters files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22-Mar-2011
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

#include "fileproxymodel.h"
#include <QTimer>
#include <QRegularExpression>
#include "taggedfilesystemmodel.h"
#include "itaggedfilefactory.h"
#include "config.h"

namespace {

QHash<int,QByteArray> getRoleHash()
{
  QHash<int, QByteArray> roles;
  roles[FileSystemModel::FileNameRole] = "fileName";
  roles[FileSystemModel::FilePathRole] = "filePath";
  roles[TaggedFileSystemModel::IconIdRole] = "iconId";
  roles[TaggedFileSystemModel::TruncatedRole] = "truncated";
  roles[TaggedFileSystemModel::IsDirRole] = "isDir";
  roles[Qt::CheckStateRole] = "checkState";
  return roles;
}

}

/**
 * Constructor.
 *
 * @param parent parent object
 */
FileProxyModel::FileProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent),
    m_fsModel(nullptr),
    m_loadTimer(new QTimer(this)), m_sortTimer(new QTimer(this)),
    m_numModifiedFiles(0), m_isLoading(false)
{
  setObjectName(QLatin1String("FileProxyModel"));
  m_loadTimer->setSingleShot(true);
  m_loadTimer->setInterval(1000);
  connect(m_loadTimer, &QTimer::timeout, this, &FileProxyModel::onDirectoryLoaded);
  m_sortTimer->setSingleShot(true);
  m_sortTimer->setInterval(100);
  connect(m_sortTimer, &QTimer::timeout, this, &FileProxyModel::emitSortingFinished);
}

/**
 * Destructor.
 */
FileProxyModel::~FileProxyModel()
{
}

/**
 * Map role identifiers to role property names in scripting languages.
 * @return hash mapping role identifiers to names.
 */
QHash<int,QByteArray> FileProxyModel::roleNames() const
{
  static QHash<int, QByteArray> roles = getRoleHash();
  return roles;
}

/**
 * Get file information of model index.
 * @return file information
 */
QFileInfo FileProxyModel::fileInfo(const QModelIndex& index) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->fileInfo(sourceIndex);
  }
  return QFileInfo();
}

/**
 * Get file path of model index.
 * @return path to file or directory
 */
QString FileProxyModel::filePath(const QModelIndex& index) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->filePath(sourceIndex);
  }
  return QString();
}

/**
 * Get file name of model index.
 * @return name of file or directory
 */
QString FileProxyModel::fileName(const QModelIndex& index) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->fileName(sourceIndex);
  }
  return QString();
}

/**
 * Check if model index represents directory.
 * @return true if directory
 */
bool FileProxyModel::isDir(const QModelIndex& index) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->isDir(sourceIndex);
  }
  return false;
}

/**
 * Delete file of index.
 * @return true if ok
 */
bool FileProxyModel::remove(const QModelIndex& index) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->remove(sourceIndex);
  }
  return false;
}

/**
 * Delete directory of index.
 * @return true if ok
 */
bool FileProxyModel::rmdir(const QModelIndex& index) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->rmdir(sourceIndex);
  }
  return false;
}

/**
 * Create a directory with @a name in the @a parent model index.
 * @return index of created directory.
 */
QModelIndex FileProxyModel::mkdir(const QModelIndex& parent, const QString& name) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(parent));
    return mapFromSource(m_fsModel->mkdir(sourceIndex, name));
  }
  return QModelIndex();
}

/**
 * Rename file or directory of @a index to @a newName.
 * @return true if ok
 */
bool FileProxyModel::rename(const QModelIndex& index, const QString& newName)
{
  if (m_fsModel) {
    QModelIndex sourceIndex(mapToSource(index));
    return m_fsModel->rename(sourceIndex, newName);
  }
  return false;
}

/**
 * Get index for given path and column.
 * @param path path to file or directory
 * @param column model column
 * @return model index, invalid if not found.
 */
QModelIndex FileProxyModel::index(const QString& path, int column) const
{
  if (m_fsModel) {
    QModelIndex sourceIndex = m_fsModel->index(path, column);
    if (sourceIndex.isValid()) {
      return mapFromSource(sourceIndex);
    }
  }
  return QModelIndex();
}

/**
 * Check if row should be included in model.
 *
 * @param srcRow source row
 * @param srcParent source parent
 *
 * @return true to include row.
 */
bool FileProxyModel::filterAcceptsRow(
    int srcRow, const QModelIndex& srcParent) const
{
  QAbstractItemModel* srcModel = sourceModel();
  if (srcModel) {
    QModelIndex srcIndex(srcModel->index(srcRow, 0, srcParent));
    if (!m_filteredOut.isEmpty()) {
      if (m_filteredOut.contains(srcIndex))
        return false;
    }
    QString item(srcIndex.data().toString());
    if (item == QLatin1String(".") || item == QLatin1String(".."))
      return false;
    if (!m_fsModel)
      return true;
    if (m_fsModel->isDir(srcIndex))
      return passesExcludeFolderFilters(m_fsModel->filePath(srcIndex));
    if (m_extensions.isEmpty())
      return true;
    for (auto it = m_extensions.constBegin(); it != m_extensions.constEnd(); ++it) {
      if (item.endsWith(*it, Qt::CaseInsensitive))
        return true;
    }
  }
  return false;
}

/**
 * Get item flags.
 * @param index index of item
 * @return default flags plus drag enabled depending on
 * setExclusiveDraggableIndex().
 */
Qt::ItemFlags FileProxyModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags itemFlags = QSortFilterProxyModel::flags(index);

  if (index.isValid()) {
    if (!m_exclusiveDraggableIndex.isValid() ||
        index == m_exclusiveDraggableIndex) {
      itemFlags |= Qt::ItemIsDragEnabled;
    } else {
      itemFlags &= ~Qt::ItemIsDragEnabled;
    }
  }
  if (index.column() < TaggedFileSystemModel::NUM_FILESYSTEM_COLUMNS) {
    // Prevent inplace editing (i.e. renaming) of files and directories.
    itemFlags &= ~Qt::ItemIsEditable;
  } else {
    itemFlags |= Qt::ItemIsEditable;
  }

  return itemFlags;
}

/**
 * Set source model.
 * @param sourceModel source model, must be TaggedFileSystemModel
 */
void FileProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  auto fsModel = qobject_cast<TaggedFileSystemModel*>(sourceModel);
  Q_ASSERT_X(fsModel != nullptr , "setSourceModel",
             "sourceModel is not TaggedFileSystemModel");
  if (fsModel != m_fsModel) {
    if (m_fsModel) {
      m_isLoading = false;
      disconnect(m_fsModel, &FileSystemModel::rootPathChanged,
                 this, &FileProxyModel::onStartLoading);
      disconnect(m_fsModel, &FileSystemModel::directoryLoaded,
                 this, &FileProxyModel::onDirectoryLoaded);
      disconnect(m_fsModel, &TaggedFileSystemModel::fileModificationChanged,
                 this, &FileProxyModel::onFileModificationChanged);
    }
    m_fsModel = fsModel;
    if (m_fsModel) {
      connect(m_fsModel, &FileSystemModel::rootPathChanged,
              this, &FileProxyModel::onStartLoading);
      connect(m_fsModel, &FileSystemModel::directoryLoaded,
              this, &FileProxyModel::onDirectoryLoaded);
      connect(m_fsModel, &TaggedFileSystemModel::fileModificationChanged,
              this, &FileProxyModel::onFileModificationChanged);
    }
  }
  QSortFilterProxyModel::setSourceModel(sourceModel);
}

/**
 * Called when directoryLoaded() is emitted.
 */
void FileProxyModel::onDirectoryLoaded()
{
  m_loadTimer->stop();
  m_sortTimer->start();
}

/**
 * Emit sortingFinished().
 */
void FileProxyModel::emitSortingFinished()
{
  m_isLoading = false;
  emit sortingFinished();
}

/**
 * Count items in model.
 * @param rootIndex index of root item
 * @param folderCount the folder count is returned here
 * @param fileCount the file count is returned here
 */
void FileProxyModel::countItems(const QModelIndex& rootIndex,
                                int& folderCount, int& fileCount)
{
  folderCount = 0;
  fileCount = 0;
  QModelIndexList todo;
  todo.append(rootIndex);
  while (!todo.isEmpty()) {
    QModelIndex parent = todo.takeFirst();
    for (int row = 0, numRows = rowCount(parent); row < numRows; ++row) {
      QModelIndex idx = index(row, 0, parent);
      if (!hasChildren(idx)) {
        ++fileCount;
      } else {
        ++folderCount;
        todo.append(idx);
      }
    }
  }
}

/**
 * Called when loading the directory starts.
 */
void FileProxyModel::onStartLoading()
{
  m_isLoading = true;
  // Last resort timeout for the case that directoryLoaded() would not be
  // fired and for empty directories with Qt < 4.7
  m_loadTimer->start();
}

/**
 * Check if more data is available.
 * @param parent parent index of items to fetch
 * @return true if more data available.
 */
bool FileProxyModel::canFetchMore(const QModelIndex& parent) const
{
  QString path = filePath(parent);
  if (!passesIncludeFolderFilters(path) || !passesExcludeFolderFilters(path))
    return false;

  return QSortFilterProxyModel::canFetchMore(parent);
}

/**
 * Fetches any available data.
 * @param parent parent index of items to fetch
 */
void FileProxyModel::fetchMore(const QModelIndex& parent)
{
  onStartLoading();
  QSortFilterProxyModel::fetchMore(parent);
}

/**
 * Sort model.
 *
 * This method will directly call FileSystemModel::sort() on the
 * sourceModel() to take advantage of that specialized behavior. This
 * will change the order in the source model.
 *
 * @param column column to sort
 * @param order ascending or descending order
 */
void FileProxyModel::sort(int column, Qt::SortOrder order)
{
  QAbstractItemModel* srcModel = nullptr;
  if (rowCount() > 0 && (srcModel = sourceModel()) != nullptr) {
    if (column < TaggedFileSystemModel::NUM_FILESYSTEM_COLUMNS) {
      if (sortColumn() >= TaggedFileSystemModel::NUM_FILESYSTEM_COLUMNS) {
        // restore the source model order
        QSortFilterProxyModel::sort(-1, order);
      }
      srcModel->sort(column, order);
    } else {
      QSortFilterProxyModel::sort(column, order);
    }
  }
}

/**
 * Sets the name filters to apply against the existing files.
 * @param filters list of strings containing wildcards like "*.mp3"
 */
void FileProxyModel::setNameFilters(const QStringList& filters)
{
  QRegularExpression wildcardRe(QLatin1String("\\.\\w+"));
  QSet<QString> exts;
  for (const QString& filter : filters) {
    auto it = wildcardRe.globalMatch(filter);
    while (it.hasNext()) {
      auto match = it.next();
      int pos = match.capturedStart();
      int len = match.capturedLength();
      exts.insert(filter.mid(pos, len).toLower());
    }
  }
  QStringList oldExtensions(m_extensions);
#if QT_VERSION >= 0x050e00
  m_extensions = QStringList(exts.constBegin(), exts.constEnd());
#else
  m_extensions = exts.toList();
#endif
  if (m_extensions != oldExtensions) {
    invalidateFilter();
  }
}

/**
 * Filter out a model index.
 * @param index source model index which has to be filtered out
 */
void FileProxyModel::filterOutIndex(const QPersistentModelIndex& index)
{
  m_filteredOut.insert(index);
}

/**
 * Reset internal data of the model.
 * Is called from endResetModel().
 */
void FileProxyModel::resetInternalData()
{
  QSortFilterProxyModel::resetInternalData();
  m_filteredOut.clear();
  m_loadTimer->stop();
  m_sortTimer->stop();
  m_numModifiedFiles = 0;
  m_isLoading = false;
}

/**
 * Stop filtering out indexes.
 */
void FileProxyModel::disableFilteringOutIndexes()
{
  m_filteredOut.clear();
  invalidateFilter();
}

/**
 * Check if index filter is active.
 * @return true if indexes are filtered out
 */
bool FileProxyModel::isFilteringOutIndexes() const
{
  return !m_filteredOut.isEmpty();
}

/**
 * Make filter changes active after adding indexes to be filtered out.
 */
void FileProxyModel::applyFilteringOutIndexes()
{
  invalidateFilter();
}

/**
 * Set filters for included and excluded folders.
 * @param includeFolders wildcard expressions for folders to be included
 * @param excludeFolders wildcard expressions for folders to be excluded
 */
void FileProxyModel::setFolderFilters(const QStringList& includeFolders,
                                      const QStringList& excludeFolders)
{
  QList<QRegularExpression> oldIncludeFolderFilters, oldExcludeFolderFilters;
  m_includeFolderFilters.swap(oldIncludeFolderFilters);
  m_excludeFolderFilters.swap(oldExcludeFolderFilters);
  for (QString filter : includeFolders) {
    filter.replace(QLatin1Char('\\'), QLatin1Char('/'));
#if QT_VERSION >= 0x050f00
    filter = QRegularExpression::wildcardToRegularExpression(filter);
#else
    filter = FileSystemModel::wildcardToRegularExpression(filter);
#endif
    m_includeFolderFilters.append(
          QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption));
  }

  for (QString filter : excludeFolders) {
    filter.replace(QLatin1Char('\\'), QLatin1Char('/'));
#if QT_VERSION >= 0x050f00
    filter = QRegularExpression::wildcardToRegularExpression(filter);
#else
    filter = FileSystemModel::wildcardToRegularExpression(filter);
#endif
    m_excludeFolderFilters.append(
          QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption));
  }

  if (m_includeFolderFilters != oldIncludeFolderFilters ||
      m_excludeFolderFilters != oldExcludeFolderFilters) {
    invalidateFilter();
  }
}

/**
 * Check if a directory path passes the include folder filters.
 * @param dirPath absolute path to directory
 * @return true if path passes filters.
 */
bool FileProxyModel::passesIncludeFolderFilters(const QString& dirPath) const
{
  if (!m_includeFolderFilters.isEmpty()) {
    bool included = false;
    for (auto it = m_includeFolderFilters.constBegin();
         it != m_includeFolderFilters.constEnd();
         ++it) {
      if (it->match(dirPath).hasMatch()) {
        included = true;
        break;
      }
    }
    if (!included) {
      return false;
    }
  }

  return true;
}

/**
 * Check if a directory path passes the include folder filters.
 * @param dirPath absolute path to directory
 * @return true if path passes filters.
 */
bool FileProxyModel::passesExcludeFolderFilters(const QString& dirPath) const
{
  if (!m_excludeFolderFilters.isEmpty()) {
    for (auto it = m_excludeFolderFilters.constBegin();
         it != m_excludeFolderFilters.constEnd();
         ++it) {
      if (it->match(dirPath).hasMatch()) {
        return false;
      }
    }
  }

  return true;
}

/**
 * Get tagged file of model index.
 *
 * @param index model index
 *
 * @return tagged file, 0 is returned if the index does not contain a
 * TaggedFile or if has a TaggedFile which is null.
 */
TaggedFile* FileProxyModel::getTaggedFileOfIndex(const QModelIndex& index) {
  return TaggedFileSystemModel::getTaggedFileOfIndex(index);
}

/**
 * Get directory path if model index is of directory.
 *
 * @param index model index
 *
 * @return directory path, null if not directory
 */
QString FileProxyModel::getPathIfIndexOfDir(const QModelIndex& index) {
  const auto model =
      qobject_cast<const FileProxyModel*>(index.model());
  if (!model || !model->isDir(index))
    return QString();

  return model->filePath(index);
}

/**
 * Read tagged file with ID3v2.4.0.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be newly created tagged file).
 */
TaggedFile* FileProxyModel::readWithId3V24(TaggedFile* taggedFile)
{
  const QPersistentModelIndex& index = taggedFile->getIndex();
  if (TaggedFile* tagLibFile = TaggedFileSystemModel::createTaggedFile(
          TaggedFile::TF_ID3v24, taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(tagLibFile);
      // setData() will not invalidate the model, so this should be safe.
      auto setDataModel = const_cast<QAbstractItemModel*>(
          index.model());
      if (setDataModel) {
        setDataModel->setData(index, data, TaggedFileSystemModel::TaggedFileRole);
      }
    }
    taggedFile = tagLibFile;
    taggedFile->readTags(false);
  }
  return taggedFile;
}
/**
 * Read tagged file with ID3v2.3.0.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be newly created tagged file).
 */
TaggedFile* FileProxyModel::readWithId3V23(TaggedFile* taggedFile)
{
  const QPersistentModelIndex& index = taggedFile->getIndex();
  if (TaggedFile* id3libFile = TaggedFileSystemModel::createTaggedFile(
          TaggedFile::TF_ID3v23, taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(id3libFile);
      // setData() will not invalidate the model, so this should be safe.
      auto setDataModel = const_cast<QAbstractItemModel*>(
          index.model());
      if (setDataModel) {
        setDataModel->setData(index, data, TaggedFileSystemModel::TaggedFileRole);
      }
    }
    taggedFile = id3libFile;
    taggedFile->readTags(false);
  }
  return taggedFile;
}

/**
 * Read file with ID3v2.4 if it has an ID3v2.4 or ID3v2.2 tag.
 * ID3v2.2 files are also read with ID3v2.4 because id3lib corrupts
 * images in ID3v2.2 tags.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new TagLibFile).
 */
TaggedFile* FileProxyModel::readWithId3V24IfId3V24(TaggedFile* taggedFile)
{
  if (taggedFile &&
      (taggedFile->taggedFileFeatures() &
       (TaggedFile::TF_ID3v23 | TaggedFile::TF_ID3v24)) ==
        TaggedFile::TF_ID3v23 &&
      !taggedFile->isChanged() &&
      taggedFile->isTagInformationRead() && taggedFile->hasTag(Frame::Tag_Id3v2)) {
    QString id3v2Version = taggedFile->getTagFormat(Frame::Tag_Id3v2);
    if (id3v2Version.isNull() || id3v2Version == QLatin1String("ID3v2.2.0")) {
      taggedFile = readWithId3V24(taggedFile);
    }
  }
  return taggedFile;
}

/**
 * Read tagged file with Ogg FLAC.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be newly created tagged file).
 */
TaggedFile* FileProxyModel::readWithOggFlac(TaggedFile* taggedFile)
{
  const QPersistentModelIndex& index = taggedFile->getIndex();
  if (TaggedFile* tagLibFile = TaggedFileSystemModel::createTaggedFile(
          TaggedFile::TF_OggFlac, taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(tagLibFile);
      // setData() will not invalidate the model, so this should be safe.
      auto setDataModel = const_cast<QAbstractItemModel*>(
          index.model());
      if (setDataModel) {
        setDataModel->setData(index, data, TaggedFileSystemModel::TaggedFileRole);
      }
    }
    taggedFile = tagLibFile;
    taggedFile->readTags(false);
  }
  return taggedFile;
}

/**
 * Try to read Ogg file with invalid tag detail info as an Ogg FLAC file.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new TagLibFile).
 */
TaggedFile* FileProxyModel::readWithOggFlacIfInvalidOgg(TaggedFile* taggedFile)
{
  if (taggedFile &&
      (taggedFile->taggedFileFeatures() &
       (TaggedFile::TF_OggPictures | TaggedFile::TF_OggFlac)) ==
        TaggedFile::TF_OggPictures &&
      !taggedFile->isChanged() &&
      taggedFile->isTagInformationRead()) {
    TaggedFile::DetailInfo info;
    taggedFile->getDetailInfo(info);
    if (!info.valid) {
      taggedFile = readWithOggFlac(taggedFile);
    }
  }
  return taggedFile;
}

/**
 * Call readTags() on tagged file.
 * Reread file with other metadata plugin if it is not supported by current
 * plugin.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new TaggedFile).
 */
TaggedFile* FileProxyModel::readTagsFromTaggedFile(TaggedFile* taggedFile)
{
  taggedFile->readTags(false);
  taggedFile = readWithId3V24IfId3V24(taggedFile);
  taggedFile = readWithOggFlacIfInvalidOgg(taggedFile);
  return taggedFile;
}

/**
 * Called when the source model emits fileModificationChanged().
 * @param srcIndex source model index
 * @param modified true if file is modified
 */
void FileProxyModel::onFileModificationChanged(const QModelIndex& srcIndex,
                                               bool modified)
{
  auto index = mapFromSource(srcIndex);
  emit fileModificationChanged(index, modified);
  emit dataChanged(index, index);
  bool lastIsModified = isModified();
  if (modified) {
    ++m_numModifiedFiles;
  } else if (m_numModifiedFiles > 0) {
    --m_numModifiedFiles;
  }
  bool newIsModified = isModified();
  if (newIsModified != lastIsModified) {
    emit modifiedChanged(newIsModified);
  }
}

/**
 * Get icon provider.
 * @return icon provider.
 */
CoreTaggedFileIconProvider* FileProxyModel::getIconProvider() const
{
  if (m_fsModel) {
    return m_fsModel->getIconProvider();
  }
  return nullptr;
}

/**
 * Access to tagged file factories.
 * @return reference to tagged file factories.
 */
QList<ITaggedFileFactory*>& FileProxyModel::taggedFileFactories()
{
  return TaggedFileSystemModel::taggedFileFactories();
}

/**
 * Create name-file pattern pairs for all supported types.
 * The order is the same as in createFilterString().
 *
 * @return pairs containing name, pattern, e.g. ("MP3", "*.mp3"), ...,
 * ("All Files", "*").
 */
QList<QPair<QString, QString> > FileProxyModel::createNameFilters()
{
  QStringList extensions;
  const auto factories = taggedFileFactories();
  for (ITaggedFileFactory* factory : factories) {
    const auto keys = factory->taggedFileKeys();
    for (const QString& key : keys) {
      extensions.append(factory->supportedFileExtensions(key));
    }
  }
  // remove duplicates
  extensions.sort();
  QString lastExt(QLatin1String(""));
  for (auto it = extensions.begin(); it != extensions.end();) {
    if (*it == lastExt) {
      it = extensions.erase(it);
    } else {
      lastExt = *it;
      ++it;
    }
  }

  QString allPatterns;
  QList<QPair<QString, QString> > nameFilters;
  for (auto it = extensions.constBegin();
       it != extensions.constEnd();
       ++it) {
    QString text = (*it).mid(1).toUpper();
    QString pattern = QLatin1Char('*') + *it;
    if (!allPatterns.isEmpty()) {
      allPatterns += QLatin1Char(' ');
    }
    allPatterns += pattern;
    nameFilters.append(qMakePair(text, pattern));
  }
  if (!allPatterns.isEmpty()) {
    // Add extensions for playlists.
    allPatterns += QLatin1String(" *.m3u *.pls *.xspf");
    nameFilters.prepend(qMakePair(tr("All Supported Files"), allPatterns));
  }
  nameFilters.append(qMakePair(tr("All Files"), QString(QLatin1Char('*'))));
  return nameFilters;
}
