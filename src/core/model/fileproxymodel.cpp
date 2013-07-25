/**
 * \file fileproxymodel.cpp
 * Proxy for filesystem model which filters files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22-Mar-2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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
#include "taggedfileiconprovider.h"
#include "config.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

/**
 * Constructor.
 *
 * @param parent parent object
 */
FileProxyModel::FileProxyModel(QObject* parent) : QSortFilterProxyModel(parent),
  m_iconProvider(new TaggedFileIconProvider), m_fsModel(0)
{
  setObjectName(QLatin1String("FileProxyModel"));
  connect(this, SIGNAL(rowsInserted(QModelIndex, int, int)),
          this, SLOT(updateInsertedRows(QModelIndex,int,int)));
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
 * Destructor.
 */
FileProxyModel::~FileProxyModel()
{
  clearTaggedFileStore();
  delete m_iconProvider;
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int FileProxyModel::columnCount(const QModelIndex&) const
{
  return 1;
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant FileProxyModel::headerData(int, Qt::Orientation, int) const
{
  return QVariant();
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
    if (m_extensions.isEmpty() || !m_fsModel || m_fsModel->isDir(srcIndex))
      return true;
    for (QStringList::const_iterator it = m_extensions.begin();
         it != m_extensions.end();
         ++it) {
      if (item.endsWith(*it, Qt::CaseInsensitive))
        return true;
    }
  }
  return false;
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
  m_fsModel = qobject_cast<QFileSystemModel*>(sourceModel);
  Q_ASSERT_X(m_fsModel != 0 , "setSourceModel",
             "sourceModel is not QFileSystemModel");
#if QT_VERSION >= 0x040700
  disconnect(this, SIGNAL(directoryLoaded(QString)));
  connect(m_fsModel, SIGNAL(directoryLoaded(QString)),
          this, SIGNAL(directoryLoaded(QString)));
#endif
  QSortFilterProxyModel::setSourceModel(sourceModel);
}

/**
 * Sets the name filters to apply against the existing files.
 * @param filters list of strings containing wildcards like "*.mp3"
 */
void FileProxyModel::setNameFilters(const QStringList& filters)
{
  QRegExp wildcardRe(QLatin1String("\\.\\w+"));
  QSet<QString> exts;
  foreach (QString filter, filters) {
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
 * Emit dataChanged() to the model to force an update of the connected views,
 * e.g. when the modification state changes.
  * @param topLeft top left item changed
  * @param bottomRight bottom right item changed
 */
void FileProxyModel::emitDataChanged(const QModelIndex& topLeft,
                                     const QModelIndex& bottomRight)
{
  emit dataChanged(topLeft, bottomRight);
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

  QFileInfo info = fileInfo(index);
  dat.setValue(TaggedFile::createFile(info.path(), info.fileName(), index));
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
  if (!(index.isValid() && index.model() != 0))
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
  if (!(index.isValid() && index.model() != 0))
    return 0;
  QVariant data(index.model()->data(index, FileProxyModel::TaggedFileRole));
  if (!data.canConvert<TaggedFile*>())
    return 0;
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
  const FileProxyModel* model =
      qobject_cast<const FileProxyModel*>(index.model());
  if (!model || !model->isDir(index))
    return QString();

  return model->filePath(index);
}

/**
 * Read tagged file with TagLib.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new TagLibFile).
 */
TaggedFile* FileProxyModel::readWithTagLib(TaggedFile* taggedFile)
{
#ifdef HAVE_TAGLIB
  const QPersistentModelIndex& index = taggedFile->getIndex();
  TagLibFile* tagLibFile = new TagLibFile(
        taggedFile->getDirname(), taggedFile->getFilename(), index);
  if (index.isValid()) {
    QVariant data;
    data.setValue(static_cast<TaggedFile*>(tagLibFile));
    // setData() will not invalidate the model, so this should be safe.
    QAbstractItemModel* setDataModel = const_cast<QAbstractItemModel*>(
        index.model());
    if (setDataModel) {
      setDataModel->setData(index, data, FileProxyModel::TaggedFileRole);
    }
  }
  taggedFile = tagLibFile;
  taggedFile->readTags(false);
#endif
  return taggedFile;
}

/**
 * Read tagged file with id3lib.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new Mp3File).
 */
TaggedFile* FileProxyModel::readWithId3Lib(TaggedFile* taggedFile)
{
#ifdef HAVE_ID3LIB
  const QPersistentModelIndex& index = taggedFile->getIndex();
  Mp3File* id3libFile = new Mp3File(
        taggedFile->getDirname(), taggedFile->getFilename(), index);
  if (index.isValid()) {
    QVariant data;
    data.setValue(static_cast<TaggedFile*>(id3libFile));
    // setData() will not invalidate the model, so this should be safe.
    QAbstractItemModel* setDataModel = const_cast<QAbstractItemModel*>(
        index.model());
    if (setDataModel) {
      setDataModel->setData(index, data, FileProxyModel::TaggedFileRole);
    }
  }
  taggedFile = id3libFile;
  taggedFile->readTags(false);
#endif
  return taggedFile;
}

/**
 * Read file with TagLib if it has an ID3v2.4 or ID3v2.2 tag.
 * ID3v2.2 files are also read with TagLib because id3lib corrupts
 * images in ID3v2.2 tags.
 *
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new TagLibFile).
 */
TaggedFile* FileProxyModel::readWithTagLibIfId3V24(TaggedFile* taggedFile)
{
  if (taggedFile &&
      taggedFile->taggedFileKey() == QLatin1String("Id3libMetadata") &&
      !taggedFile->isChanged() &&
      taggedFile->isTagInformationRead() && taggedFile->hasTagV2()) {
    QString id3v2Version = taggedFile->getTagFormatV2();
    if (id3v2Version.isNull() || id3v2Version == QLatin1String("ID3v2.2.0")) {
      taggedFile = readWithTagLib(taggedFile);
    }
  }
  return taggedFile;
}
