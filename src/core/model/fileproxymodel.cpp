/**
 * \file fileproxymodel.cpp
 * Proxy for filesystem model which filters files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22-Mar-2011
 *
 * Copyright (C) 2011-2017  Urs Fleisch
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
#include <QFileSystemModel>
#include <QTimer>
#include "taggedfileiconprovider.h"
#include "itaggedfilefactory.h"
#include "tagconfig.h"
#include "config.h"

/** Only defined for generation of translation files */
#define NAME_FOR_PO QT_TRANSLATE_NOOP("QFileSystemModel", "Name")
/** Only defined for generation of translation files */
#define SIZE_FOR_PO QT_TRANSLATE_NOOP("QFileSystemModel", "Size")
/** Only defined for generation of translation files */
#define TYPE_FOR_PO QT_TRANSLATE_NOOP("QFileSystemModel", "Type")
/** Only defined for generation of translation files */
#define KIND_FOR_PO QT_TRANSLATE_NOOP("QFileSystemModel", "Kind")
/** Only defined for generation of translation files */
#define DATE_MODIFIED_FOR_PO \
  QT_TRANSLATE_NOOP("QFileSystemModel", "Date Modified")

QList<ITaggedFileFactory*> FileProxyModel::s_taggedFileFactories;

namespace {

QHash<int,QByteArray> getRoleHash()
{
  QHash<int, QByteArray> roles;
  roles[QFileSystemModel::FileNameRole] = "fileName";
  roles[QFileSystemModel::FilePathRole] = "filePath";
  roles[FileProxyModel::IconIdRole] = "iconId";
  roles[FileProxyModel::TruncatedRole] = "truncated";
  roles[FileProxyModel::IsDirRole] = "isDir";
  roles[Qt::CheckStateRole] = "checkState";
  return roles;
}

}

/**
 * Constructor.
 *
 * @param parent parent object
 */
FileProxyModel::FileProxyModel(QObject* parent) : QSortFilterProxyModel(parent),
  m_iconProvider(new TaggedFileIconProvider), m_fsModel(nullptr),
  m_loadTimer(new QTimer(this)), m_sortTimer(new QTimer(this)),
  m_numModifiedFiles(0), m_isLoading(false)
{
  setObjectName(QLatin1String("FileProxyModel"));
  connect(this, &QAbstractItemModel::rowsInserted,
          this, &FileProxyModel::updateInsertedRows);
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
  clearTaggedFileStore();
  delete m_iconProvider;
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
 * Update the TaggedFile contents for rows inserted into the model.
 * @param parent parent model index
 * @param start starting row
 * @param end ending row
 */
void FileProxyModel::updateInsertedRows(const QModelIndex& parent,
                                        int start, int end) {
  const QAbstractItemModel* model = parent.model();
  if (!model)
    return;
  for (int row = start; row <= end; ++row) {
    QModelIndex index(model->index(row, 0, parent));
    initTaggedFileData(index);
  }
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

  return itemFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant FileProxyModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    if (role == TaggedFileRole) {
      return retrieveTaggedFileVariant(index);
    } else if (role == Qt::DecorationRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, 0);
      if (taggedFile) {
        return m_iconProvider->iconForTaggedFile(taggedFile);
      }
    } else if (role == Qt::BackgroundRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, 0);
      if (taggedFile) {
        QColor color = m_iconProvider->backgroundForTaggedFile(taggedFile);
        if (color.isValid())
          return color;
      }
    } else if (role == IconIdRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, 0);
      return taggedFile
          ? m_iconProvider->iconIdForTaggedFile(taggedFile)
          : QByteArray("");
    } else if (role == TruncatedRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, 0);
      return taggedFile &&
          ((TagConfig::instance().markTruncations() &&
            taggedFile->getTruncationFlags(Frame::Tag_Id3v1) != 0) ||
           taggedFile->isMarked());
    } else if (role == IsDirRole && index.column() == 0) {
      return isDir(index);
    }
  }
  return QSortFilterProxyModel::data(index, role);
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool FileProxyModel::setData(const QModelIndex& index, const QVariant& value,
                             int role)
{
  if (index.isValid() && role == TaggedFileRole) {
    return storeTaggedFileVariant(index, value);
  }
  return QSortFilterProxyModel::setData(index, value, role);
}

/**
 * Set source model.
 * @param sourceModel source model, must be QFileSystemModel
 */
void FileProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  auto fsModel = qobject_cast<QFileSystemModel*>(sourceModel);
  Q_ASSERT_X(fsModel != nullptr , "setSourceModel",
             "sourceModel is not QFileSystemModel");
  if (fsModel != m_fsModel) {
    if (m_fsModel) {
      m_isLoading = false;
      disconnect(m_fsModel, &QFileSystemModel::rootPathChanged,
                 this, &FileProxyModel::onStartLoading);
      disconnect(m_fsModel, &QFileSystemModel::directoryLoaded,
                 this, &FileProxyModel::onDirectoryLoaded);
    }
    m_fsModel = fsModel;
    if (m_fsModel) {
      connect(m_fsModel, &QFileSystemModel::rootPathChanged,
              this, &FileProxyModel::onStartLoading);
      connect(m_fsModel, &QFileSystemModel::directoryLoaded,
              this, &FileProxyModel::onDirectoryLoaded);
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
 * This method will directly call QFileSystemModel::sort() on the
 * sourceModel() to take advantage of that specialized behavior. This
 * will change the order in the souce model.
 *
 * @param column column to sort
 * @param order ascending or descending order
 */
void FileProxyModel::sort(int column, Qt::SortOrder order)
{
  QAbstractItemModel* srcModel = nullptr;
  if (rowCount() > 0 && (srcModel = sourceModel()) != nullptr) {
    srcModel->sort(column, order);
  }
}

/**
 * Sets the name filters to apply against the existing files.
 * @param filters list of strings containing wildcards like "*.mp3"
 */
void FileProxyModel::setNameFilters(const QStringList& filters)
{
  QRegExp wildcardRe(QLatin1String("\\.\\w+"));
  QSet<QString> exts;
  for (const QString& filter : filters) {
    int pos = 0;
    while ((pos = wildcardRe.indexIn(filter, pos)) != -1) {
      int len = wildcardRe.matchedLength();
      exts.insert(filter.mid(pos, len).toLower());
      pos += len;
    }
  }
  QStringList oldExtensions(m_extensions);
  m_extensions = exts.toList();
  if (m_extensions != oldExtensions) {
    invalidateFilter();
  }
}

/**
 * Filter out a model index.
 * @param index model index which has to be filtered out
 */
void FileProxyModel::filterOutIndex(const QPersistentModelIndex& index)
{
  m_filteredOut.insert(mapToSource(index));
}

/**
 * Reset internal data of the model.
 * Is called from endResetModel().
 */
void FileProxyModel::resetInternalData()
{
  QSortFilterProxyModel::resetInternalData();
  clearTaggedFileStore();
  m_filteredOut.clear();
  m_loadTimer->stop();
  m_sortTimer->stop();
  m_numModifiedFiles = 0;
  m_isLoading = false;
}

/**
 * Reset the model.
 */
void FileProxyModel::resetModel()
{
  beginResetModel();
  endResetModel();
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
  QList<QRegExp> oldIncludeFolderFilters, oldExcludeFolderFilters;
  m_includeFolderFilters.swap(oldIncludeFolderFilters);
  m_excludeFolderFilters.swap(oldExcludeFolderFilters);
  for (QString filter : includeFolders) {
    filter.replace(QLatin1Char('\\'), QLatin1Char('/'));
    m_includeFolderFilters.append(
          QRegExp(filter, Qt::CaseInsensitive, QRegExp::Wildcard));
  }

  for (QString filter : excludeFolders) {
    filter.replace(QLatin1Char('\\'), QLatin1Char('/'));
    m_excludeFolderFilters.append(
          QRegExp(filter, Qt::CaseInsensitive, QRegExp::Wildcard));
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
      if (it->exactMatch(dirPath)) {
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
      if (it->exactMatch(dirPath)) {
        return false;
      }
    }
  }

  return true;
}

/**
 * Retrieve tagged file for an index.
 * @param index model index
 * @return QVariant with tagged file, invalid QVariant if not found.
 */
QVariant FileProxyModel::retrieveTaggedFileVariant(
    const QPersistentModelIndex& index) const {
  if (m_taggedFiles.contains(index))
    return QVariant::fromValue(m_taggedFiles.value(index));
  return QVariant();
}

/**
 * Store tagged file from variant with index.
 * @param index model index
 * @param value QVariant containing tagged file
 * @return true if index and value valid
 */
bool FileProxyModel::storeTaggedFileVariant(const QPersistentModelIndex& index,
                     QVariant value) {
  if (index.isValid()) {
    if (value.isValid()) {
      if (value.canConvert<TaggedFile*>()) {
        TaggedFile* oldItem = m_taggedFiles.value(index, 0);
        delete oldItem;
        m_taggedFiles.insert(index, value.value<TaggedFile*>());
        return true;
      }
    } else {
      if (TaggedFile* oldFile = m_taggedFiles.value(index, 0)) {
        m_taggedFiles.remove(index);
        delete oldFile;
      }
    }
  }
  return false;
}

/**
 * Clear store with tagged files.
 */
void FileProxyModel::clearTaggedFileStore() {
  qDeleteAll(m_taggedFiles);
  m_taggedFiles.clear();
}

/**
 * Initialize tagged file for model index.
 * @param index model index
 */
void FileProxyModel::initTaggedFileData(const QModelIndex& index) {
  QVariant dat = data(index, TaggedFileRole);
  if (dat.isValid() || isDir(index))
    return;

  dat.setValue(createTaggedFile(fileName(index), index));
  setData(index, dat, TaggedFileRole);
}


/**
 * Get tagged file data of model index.
 *
 * @param index model index
 * @param taggedFile a TaggedFile pointer is returned here
 *
 * @return true if index has a tagged file, *taggedFile is set to the pointer.
 */
bool FileProxyModel::getTaggedFileOfIndex(const QModelIndex& index,
                                          TaggedFile** taggedFile) {
  if (!(index.isValid() && index.model() != nullptr))
    return false;
  QVariant data(index.model()->data(index, FileProxyModel::TaggedFileRole));
  if (!data.canConvert<TaggedFile*>())
    return false;
  *taggedFile = data.value<TaggedFile*>();
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
  if (!(index.isValid() && index.model() != nullptr))
    return nullptr;
  QVariant data(index.model()->data(index, FileProxyModel::TaggedFileRole));
  if (!data.canConvert<TaggedFile*>())
    return nullptr;
  return data.value<TaggedFile*>();
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
  if (TaggedFile* tagLibFile = createTaggedFile(TaggedFile::TF_ID3v24,
          taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(tagLibFile);
      // setData() will not invalidate the model, so this should be safe.
      auto setDataModel = const_cast<QAbstractItemModel*>(
          index.model());
      if (setDataModel) {
        setDataModel->setData(index, data, FileProxyModel::TaggedFileRole);
      }
    }
    taggedFile = tagLibFile;
    taggedFile->readTags(false);
  }
  return taggedFile;
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
TaggedFile* FileProxyModel::createTaggedFile(
    TaggedFile::Feature feature,
    const QString& fileName,
    const QPersistentModelIndex& idx) {
  TaggedFile* taggedFile = nullptr;
  const auto factories = s_taggedFileFactories;
  for (ITaggedFileFactory* factory : factories) {
    const auto keys = factory->taggedFileKeys();
    for (const QString& key : keys) {
      if ((factory->taggedFileFeatures(key) & feature) != 0 &&
          (taggedFile = factory->createTaggedFile(key, fileName, idx,
                                                  feature))
          != nullptr) {
        return taggedFile;
      }
    }
  }
  return nullptr;
}

/**
 * Create a tagged file.
 *
 * @param fileName filename
 * @param idx model index
 *
 * @return tagged file, 0 if not found or type not supported.
 */
TaggedFile* FileProxyModel::createTaggedFile(
    const QString& fileName,
    const QPersistentModelIndex& idx) {
  TaggedFile* taggedFile = nullptr;
  const auto factories = s_taggedFileFactories;
  for (ITaggedFileFactory* factory : factories) {
    const auto keys = factory->taggedFileKeys();
    for (const QString& key : keys) {
      taggedFile = factory->createTaggedFile(key, fileName, idx);
      if (taggedFile) {
        return taggedFile;
      }
    }
  }
  return nullptr;
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
  if (TaggedFile* id3libFile = createTaggedFile(TaggedFile::TF_ID3v23,
          taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(id3libFile);
      // setData() will not invalidate the model, so this should be safe.
      auto setDataModel = const_cast<QAbstractItemModel*>(
          index.model());
      if (setDataModel) {
        setDataModel->setData(index, data, FileProxyModel::TaggedFileRole);
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
  if (TaggedFile* tagLibFile = createTaggedFile(TaggedFile::TF_OggFlac,
          taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(tagLibFile);
      // setData() will not invalidate the model, so this should be safe.
      auto setDataModel = const_cast<QAbstractItemModel*>(
          index.model());
      if (setDataModel) {
        setDataModel->setData(index, data, FileProxyModel::TaggedFileRole);
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
 * Called from tagged file to notify modification state changes.
 * @param index model index
 * @param modified true if file is modified
 */
void FileProxyModel::notifyModificationChanged(const QModelIndex& index,
                                               bool modified)
{
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
 * Called from tagged file to notify changes in extra model data, e.g. the
 * information on which the TaggedFileIconProvider depends.
 * @param index model index
 */
void FileProxyModel::notifyModelDataChanged(const QModelIndex& index)
{
  emit dataChanged(index, index);
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
