/**
 * \file taggedfilesystemmodel.cpp
 * Filesystem model which additional columns.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 08-Aug-2021
 *
 * Copyright (C) 2021  Urs Fleisch
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

#include "taggedfilesystemmodel.h"
#include "coretaggedfileiconprovider.h"
#include "filesystemmodel.h"
#include "itaggedfilefactory.h"
#include "tagconfig.h"
#include "saferename.h"

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

QList<ITaggedFileFactory*> TaggedFileSystemModel::s_taggedFileFactories;

TaggedFileSystemModel::TaggedFileSystemModel(
    CoreTaggedFileIconProvider* iconProvider, QObject* parent)
  : FileSystemModel(parent), m_iconProvider(iconProvider)
{
  setObjectName(QLatin1String("TaggedFileSystemModel"));
  connect(this, &QAbstractItemModel::rowsInserted,
          this, &TaggedFileSystemModel::updateInsertedRows);
  m_tagFrameColumnTypes
      << Frame::FT_Title << Frame::FT_Artist << Frame::FT_Album
      << Frame::FT_Comment << Frame::FT_Date << Frame::FT_Track
      << Frame::FT_Genre;
}

TaggedFileSystemModel::~TaggedFileSystemModel()
{
  clearTaggedFileStore();
}

QModelIndex TaggedFileSystemModel::sibling(int row, int column,
                                           const QModelIndex&idx) const
{
  if (row == idx.row() &&
      column >= NUM_FILESYSTEM_COLUMNS &&
      column < NUM_FILESYSTEM_COLUMNS + m_tagFrameColumnTypes.size()) {
    return createIndex(row, column, idx.internalPointer());
  } else {
    return FileSystemModel::sibling(row, column, idx);
  }
}

int TaggedFileSystemModel::columnCount(const QModelIndex &parent) const
{
  return parent.column() > 0
      ? 0 : NUM_FILESYSTEM_COLUMNS + m_tagFrameColumnTypes.size();
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant TaggedFileSystemModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    if (role == TaggedFileRole) {
      return retrieveTaggedFileVariant(index);
    } else if (role == Qt::DecorationRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, nullptr);
      if (taggedFile) {
        return m_iconProvider->iconForTaggedFile(taggedFile);
      }
    } else if (role == Qt::BackgroundRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, nullptr);
      if (taggedFile) {
        QVariant color = m_iconProvider->backgroundForTaggedFile(taggedFile);
        if (!color.isNull())
          return color;
      }
    } else if (role == IconIdRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, nullptr);
      return taggedFile
          ? m_iconProvider->iconIdForTaggedFile(taggedFile)
          : QByteArray("");
    } else if (role == TruncatedRole && index.column() == 0) {
      TaggedFile* taggedFile = m_taggedFiles.value(index, nullptr);
      return taggedFile &&
          ((TagConfig::instance().markTruncations() &&
            taggedFile->getTruncationFlags(Frame::Tag_Id3v1) != 0) ||
           taggedFile->isMarked());
    } else if (role == IsDirRole && index.column() == 0) {
      return isDir(index);
    } else if ((role == Qt::DisplayRole || role == Qt::EditRole) &&
               index.column() >= NUM_FILESYSTEM_COLUMNS &&
               index.column() <
               NUM_FILESYSTEM_COLUMNS + m_tagFrameColumnTypes.size()) {
      QPersistentModelIndex taggedFileIdx = index.sibling(index.row(), 0);
      auto it = m_taggedFiles.constFind(taggedFileIdx);
      if (it != m_taggedFiles.constEnd()) {
        if (TaggedFile* taggedFile = *it) {
          Frame frame;
          Frame::Type type = m_tagFrameColumnTypes.at(index.column() -
                                                      NUM_FILESYSTEM_COLUMNS);
          if (taggedFile->getFrame(Frame::Tag_2, type, frame)) {
            QString value = frame.getValue();
            if (type == Frame::FT_Track) {
              bool ok;
              int intValue = value.toInt(&ok);
              if (ok) {
                return intValue;
              }
            }
            return value;
          }
        }
      }
      return QVariant();
    } else if (index.column() >= NUM_FILESYSTEM_COLUMNS) {
      return QVariant();
    }
  }
  return FileSystemModel::data(index, role);
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool TaggedFileSystemModel::setData(const QModelIndex& index,
                                    const QVariant& value, int role)
{
  if (index.isValid()) {
    if (role == TaggedFileRole) {
      return storeTaggedFileVariant(index, value);
    } else if ((role == Qt::DisplayRole || role == Qt::EditRole) &&
               index.column() >= NUM_FILESYSTEM_COLUMNS &&
               index.column() <
               NUM_FILESYSTEM_COLUMNS + m_tagFrameColumnTypes.size()) {
      QPersistentModelIndex taggedFileIdx = index.sibling(index.row(), 0);
      auto it = m_taggedFiles.constFind(taggedFileIdx);
      if (it != m_taggedFiles.constEnd()) {
        if (TaggedFile* taggedFile = *it) {
          Frame frame;
          if (taggedFile->getFrame(
                Frame::Tag_2,
                m_tagFrameColumnTypes.at(index.column() -
                                         NUM_FILESYSTEM_COLUMNS),
                frame)) {
            frame.setValue(value.toString());
            return taggedFile->setFrame(Frame::Tag_2, frame);
          }
        }
      }
      return false;
    } else if (index.column() >= NUM_FILESYSTEM_COLUMNS) {
      return false;
    }
  }
  return FileSystemModel::setData(index, value, role);
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant TaggedFileSystemModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
      section >= NUM_FILESYSTEM_COLUMNS &&
      section < NUM_FILESYSTEM_COLUMNS + m_tagFrameColumnTypes.size()) {
    return Frame::ExtendedType(
          m_tagFrameColumnTypes.at(section - NUM_FILESYSTEM_COLUMNS))
        .getTranslatedName();
  }
  return FileSystemModel::headerData(section, orientation, role);
}

/**
 * Rename file or directory of @a index to @a newName.
 * @return true if ok
 */
bool TaggedFileSystemModel::rename(const QModelIndex& index,
                                   const QString& newName)
{
  if (Utils::hasIllegalFileNameCharacters(newName))
    return false;

  return setData(index, newName);
}

/**
 * Called from tagged file to notify modification state changes.
 * @param index model index
 * @param modified true if file is modified
 */
void TaggedFileSystemModel::notifyModificationChanged(const QModelIndex& index,
                                                      bool modified)
{
  emit fileModificationChanged(index, modified);
}

/**
 * Called from tagged file to notify changes in extra model data, e.g. the
 * information on which the CoreTaggedFileIconProvider depends.
 * @param index model index
 */
void TaggedFileSystemModel::notifyModelDataChanged(const QModelIndex& index)
{
  emit dataChanged(index, index);
}

/**
 * Update the TaggedFile contents for rows inserted into the model.
 * @param parent parent model index
 * @param start starting row
 * @param end ending row
 */
void TaggedFileSystemModel::updateInsertedRows(const QModelIndex& parent,
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
 * Reset internal data of the model.
 * Is called from endResetModel().
 */
void TaggedFileSystemModel::resetInternalData()
{
  FileSystemModel::resetInternalData();
  clearTaggedFileStore();
}

/**
 * Retrieve tagged file for an index.
 * @param index model index
 * @return QVariant with tagged file, invalid QVariant if not found.
 */
QVariant TaggedFileSystemModel::retrieveTaggedFileVariant(
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
bool TaggedFileSystemModel::storeTaggedFileVariant(
    const QPersistentModelIndex& index, const QVariant& value) {
  if (index.isValid()) {
    if (value.isValid()) {
      if (value.canConvert<TaggedFile*>()) {
        TaggedFile* oldItem = m_taggedFiles.value(index, nullptr);
        delete oldItem;
        m_taggedFiles.insert(index, value.value<TaggedFile*>());
        return true;
      }
    } else {
      if (TaggedFile* oldFile = m_taggedFiles.value(index, nullptr)) {
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
void TaggedFileSystemModel::clearTaggedFileStore() {
  qDeleteAll(m_taggedFiles);
  m_taggedFiles.clear();
}

/**
 * Initialize tagged file for model index.
 * @param index model index
 */
void TaggedFileSystemModel::initTaggedFileData(const QModelIndex& index) {
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
bool TaggedFileSystemModel::getTaggedFileOfIndex(const QModelIndex& index,
                                                 TaggedFile** taggedFile) {
  if (!(index.isValid() && index.model() != nullptr))
    return false;
  QVariant data(index.model()->data(index, TaggedFileRole));
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
TaggedFile* TaggedFileSystemModel::getTaggedFileOfIndex(
    const QModelIndex& index) {
  if (!(index.isValid() && index.model() != nullptr))
    return nullptr;
  QVariant data(index.model()->data(index, TaggedFileRole));
  if (!data.canConvert<TaggedFile*>())
    return nullptr;
  return data.value<TaggedFile*>();
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
TaggedFile* TaggedFileSystemModel::createTaggedFile(
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
TaggedFile* TaggedFileSystemModel::createTaggedFile(
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
