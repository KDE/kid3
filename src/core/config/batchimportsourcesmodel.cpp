/**
 * \file batchimportsourcesmodel.cpp
 * Context menu commands configuration table model.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "batchimportsourcesmodel.h"

/** Column indices. */
enum ColumnIndex {
  CI_Name,
  CI_Accuracy,
  CI_StandardTags,
  CI_AdditionalTags,
  CI_CoverArt,
  CI_NumColumns
};

/**
 * Constructor.
 * @param parent parent widget
 */
BatchImportSourcesModel::BatchImportSourcesModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  setObjectName(QLatin1String("BatchImportSourcesModel"));
}

/**
 * Destructor.
 */
BatchImportSourcesModel::~BatchImportSourcesModel()
{
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags BatchImportSourcesModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid()) {
    theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == CI_StandardTags ||
        index.column() == CI_AdditionalTags ||
        index.column() == CI_CoverArt) {
      theFlags |= Qt::ItemIsUserCheckable;
    }
  }
  return theFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant BatchImportSourcesModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_sources.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return QVariant();
  const BatchImportProfile::Source& item = m_sources.at(index.row());
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
    case CI_Name:
      return item.getName();
    case CI_Accuracy:
      return item.getRequiredAccuracy();
    default: ;
    }
  }
  if (role == Qt::CheckStateRole) {
    switch (index.column()) {
    case CI_StandardTags:
      return item.standardTagsEnabled() ? Qt::Checked : Qt::Unchecked;
    case CI_AdditionalTags:
      return item.additionalTagsEnabled() ? Qt::Checked : Qt::Unchecked;
    case CI_CoverArt:
      return item.coverArtEnabled() ? Qt::Checked : Qt::Unchecked;
    default: ;
    }
  }
  return QVariant();
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool BatchImportSourcesModel::setData(const QModelIndex& index,
                                      const QVariant& value, int role)
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_sources.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return false;
  BatchImportProfile::Source& item = m_sources[index.row()];
  if (role == Qt::EditRole) {
    switch (index.column()) {
    case CI_Name:
      item.setName(value.toString());
      break;
    case CI_Accuracy:
      item.setRequiredAccuracy(value.toInt());
      break;
    default:
      return false;
    }
  } else if (role == Qt::CheckStateRole) {
    switch (index.column()) {
    case CI_StandardTags:
      item.enableStandardTags(value.toInt() == Qt::Checked);
      break;
    case CI_AdditionalTags:
      item.enableAdditionalTags(value.toInt() == Qt::Checked);
      break;
    case CI_CoverArt:
      item.enableCoverArt(value.toInt() == Qt::Checked);
      break;
    default:
      return false;
    }
  } else {
    return false;
  }
  emit dataChanged(index, index);
  return true;
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant BatchImportSourcesModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    switch (section) {
    case CI_Name:
      return tr("Server");
    case CI_Accuracy:
      return tr("Accuracy");
    case CI_StandardTags:
      return tr("Standard Tags");
    case CI_AdditionalTags:
      return tr("Additional Tags");
    case CI_CoverArt:
      return tr("Cover Art");
    default:
      return section + 1;
    }
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index, invalid for table models
 * @return number of rows,
 * if parent is valid number of children (0 for table models)
 */
int BatchImportSourcesModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_sources.size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int BatchImportSourcesModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : CI_NumColumns;
}

/**
 * Insert rows.
 * @param row rows are inserted before this row, if 0 at the begin,
 * if rowCount() at the end
 * @param count number of rows to insert
 * @return true if successful
 */
bool BatchImportSourcesModel::insertRows(int row, int count,
                                         const QModelIndex&)
{
  if (count > 0) {
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_sources.insert(row, BatchImportProfile::Source());
    endInsertRows();
  }
  return true;
}

/**
 * Remove rows.
 * @param row rows are removed starting with this row
 * @param count number of rows to remove
 * @return true if successful
 */
bool BatchImportSourcesModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_sources.removeAt(row);
    endRemoveRows();
  }
  return true;
}

/**
 * Set batch import source of a given @a row.
 * @param row number of row to set
 * @param source batch import source
 */
void BatchImportSourcesModel::setBatchImportSource(
    int row, const BatchImportProfile::Source& source)
{
  if (row >= 0 && row < m_sources.size()) {
    m_sources[row] = source;
    emit dataChanged(index(row, 0), index(row, CI_NumColumns - 1));
  }
}

/**
 * Get batch import source of a given @a row.
 * @param row number of row to get
 * @param source the batch import source is returned here
 */
void BatchImportSourcesModel::getBatchImportSource(
    int row, BatchImportProfile::Source& source)
{
  if (row >= 0 && row < m_sources.size()) {
    source = m_sources.at(row);
  }
}

/**
 * Set the model from the import sources.
 * @param sources batch import sources
 */
void BatchImportSourcesModel::setBatchImportSources(
    const QList<BatchImportProfile::Source>& sources)
{
#if QT_VERSION >= 0x040600
  beginResetModel();
  m_sources = sources;
  endResetModel();
#else
  m_sources = sources;
  reset();
#endif
}

/**
 * Get the import sources from the model.
 * @return batch import sources.
 */
QList<BatchImportProfile::Source> BatchImportSourcesModel::getBatchImportSources() const
{
  return m_sources;
}
