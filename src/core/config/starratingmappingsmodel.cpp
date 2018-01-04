/**
 * \file starratingmappingsmodel.cpp
 * Star rating mappings configuration table model.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

#include "starratingmappingsmodel.h"

/** Column indices. */
enum ColumnIndex {
  CI_Name = 0,
  CI_NumColumns = 6
};

/**
 * Constructor.
 * @param parent parent widget
 */
StarRatingMappingsModel::StarRatingMappingsModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  setObjectName(QLatin1String("StarRatingMappingsModel"));
}

/**
 * Destructor.
 */
StarRatingMappingsModel::~StarRatingMappingsModel()
{
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags StarRatingMappingsModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid()) {
    theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }
  return theFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant StarRatingMappingsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_maps.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return QVariant();
  const QPair<QString, QVector<int> >& item = m_maps.at(index.row());
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == CI_Name) {
      return item.first;
    } else if (item.second.size() >= index.column()) {
      return item.second.at(index.column() - 1);
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
bool StarRatingMappingsModel::setData(const QModelIndex& index,
                                      const QVariant& value, int role)
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_maps.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return false;
  QPair<QString, QVector<int> >& item = m_maps[index.row()];
  bool changed = false;
  if (role == Qt::EditRole) {
    if (index.column() == CI_Name) {
      item.first = value.toString();
      changed = true;
    } else if (item.second.size() >= index.column()) {
      item.second[index.column() - 1] = value.toInt();
      changed = true;
    }
    if (changed) {
      makeRowValid(index.row());
      emit dataChanged(index, index);
    }
  }
  return changed;
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant StarRatingMappingsModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    if (section == CI_Name) {
      return tr("Name");
    } else if (section < CI_NumColumns) {
      return section;
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
int StarRatingMappingsModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_maps.size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int StarRatingMappingsModel::columnCount(const QModelIndex& parent) const
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
bool StarRatingMappingsModel::insertRows(int row, int count,
                                         const QModelIndex&)
{
  if (count > 0) {
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
      m_maps.insert(row, qMakePair(QString(), QVector<int>(CI_NumColumns - 1)));
      makeRowValid(row);
    }
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
bool StarRatingMappingsModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_maps.removeAt(row);
    endRemoveRows();
  }
  return true;
}

/**
 * Set the model from the star count mappings.
 * @param maps star count mappings
 */
void StarRatingMappingsModel::setMappings(
    const QList<QPair<QString, QVector<int> > >& maps)
{
  beginResetModel();
  m_maps = maps;
  endResetModel();
}

/**
 * Get the start count mappings from the model.
 * @return star count mappings
 */
QList<QPair<QString, QVector<int> > > StarRatingMappingsModel::getMappings() const
{
  return m_maps;
}

/**
 * Make sure that @a row contains valid values.
 */
void StarRatingMappingsModel::makeRowValid(int row)
{
  QString& type = m_maps[row].first;
  type = type.trimmed();
  if (type == QLatin1String("POPM.")) {
    type.truncate(4);
  }
  QVector<int>& values = m_maps[row].second;
  int previousValue = 0;
  for (QVector<int>::iterator it = values.begin(); it != values.end(); ++it) {
    if (*it <= previousValue) {
      *it = previousValue + 1;
    }
    previousValue = *it;
  }
}
