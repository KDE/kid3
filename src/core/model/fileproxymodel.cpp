/**
 * \file fileproxymodel.cpp
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

#include "fileproxymodel.h"
#include <QFileSystemModel>
#include <QTimer>
#include "taggedfileiconprovider.h"
#include "itaggedfilefactory.h"
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

/**
 * Constructor.
 *
 * @param parent parent object
 */
FileProxyModel::FileProxyModel(QObject* parent) : QSortFilterProxyModel(parent),
  m_iconProvider(new TaggedFileIconProvider), m_fsModel(0),
  m_loadTimer(new QTimer(this)), m_sortTimer(new QTimer(this)),
  m_isLoading(false)
{
  setObjectName(QLatin1String("FileProxyModel"));
  connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
          this, SLOT(updateInsertedRows(QModelIndex,int,int)));
  m_loadTimer->setSingleShot(true);
  m_loadTimer->setInterval(1000);
  connect(m_loadTimer, SIGNAL(timeout()), this, SLOT(onDirectoryLoaded()));
  m_sortTimer->setSingleShot(true);
  m_sortTimer->setInterval(100);
  connect(m_sortTimer, SIGNAL(timeout()), this, SLOT(emitSortingFinished()));
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
 * Destructor.
 */
FileProxyModel::~FileProxyModel()
{
  clearTaggedFileStore();
  delete m_iconProvider;
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
  QFileSystemModel* fsModel = qobject_cast<QFileSystemModel*>(sourceModel);
  Q_ASSERT_X(fsModel != 0 , "setSourceModel",
             "sourceModel is not QFileSystemModel");
  if (fsModel != m_fsModel) {
    if (m_fsModel) {
      m_isLoading = false;
      disconnect(m_fsModel, SIGNAL(rootPathChanged(QString)),
                 this, SLOT(onStartLoading()));
#if QT_VERSION >= 0x040700
      disconnect(m_fsModel, SIGNAL(directoryLoaded(QString)),
                 this, SLOT(onDirectoryLoaded()));
#else
      disconnect(m_fsModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                 this, SLOT(onDirectoryLoaded()));
#endif
    }
    m_fsModel = fsModel;
    if (m_fsModel) {
      connect(m_fsModel, SIGNAL(rootPathChanged(QString)),
              this, SLOT(onStartLoading()));
#if QT_VERSION >= 0x040700
      connect(m_fsModel, SIGNAL(directoryLoaded(QString)),
              this, SLOT(onDirectoryLoaded()));
#else
      // Qt < 4.7 does not have a directoryLoaded() signal, so
      // rowsInserted() and a slow timeout for empty directories
      // are used.
      connect(m_fsModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
              this, SLOT(onDirectoryLoaded()));
#endif
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
  QAbstractItemModel* srcModel = 0;
  if (rowCount() > 0 && (srcModel = sourceModel()) != 0) {
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
  foreach (const QString& filter, filters) {
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
  dat.setValue(createTaggedFile(info.path(), info.fileName(), index));
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
          taggedFile->getDirname(), taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(tagLibFile);
      // setData() will not invalidate the model, so this should be safe.
      QAbstractItemModel* setDataModel = const_cast<QAbstractItemModel*>(
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
 * @param dirName directory name
 * @param fileName filename
 * @param idx model index
 *
 * @return tagged file, 0 if feature not found or type not supported.
 */
TaggedFile* FileProxyModel::createTaggedFile(
    TaggedFile::Feature feature,
    const QString& dirName, const QString& fileName,
    const QPersistentModelIndex& idx) {
  TaggedFile* taggedFile = 0;
  foreach (ITaggedFileFactory* factory, s_taggedFileFactories) {
    foreach (const QString& key, factory->taggedFileKeys()) {
      if ((factory->taggedFileFeatures(key) & feature) != 0 &&
          (taggedFile = factory->createTaggedFile(key, dirName, fileName, idx,
                                                  feature))
          != 0) {
        return taggedFile;
      }
    }
  }
  return 0;
}

/**
 * Create a tagged file.
 *
 * @param dirName directory name
 * @param fileName filename
 * @param idx model index
 *
 * @return tagged file, 0 if not found or type not supported.
 */
TaggedFile* FileProxyModel::createTaggedFile(
    const QString& dirName, const QString& fileName,
    const QPersistentModelIndex& idx) {
  TaggedFile* taggedFile = 0;
  foreach (ITaggedFileFactory* factory, s_taggedFileFactories) {
    foreach (const QString& key, factory->taggedFileKeys()) {
      taggedFile = factory->createTaggedFile(key, dirName, fileName, idx);
      if (taggedFile) {
        return taggedFile;
      }
    }
  }
  return 0;
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
          taggedFile->getDirname(), taggedFile->getFilename(), index)) {
    if (index.isValid()) {
      QVariant data;
      data.setValue(id3libFile);
      // setData() will not invalidate the model, so this should be safe.
      QAbstractItemModel* setDataModel = const_cast<QAbstractItemModel*>(
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
      taggedFile->isTagInformationRead() && taggedFile->hasTagV2()) {
    QString id3v2Version = taggedFile->getTagFormatV2();
    if (id3v2Version.isNull() || id3v2Version == QLatin1String("ID3v2.2.0")) {
      taggedFile = readWithId3V24(taggedFile);
    }
  }
  return taggedFile;
}
