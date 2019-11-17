/**
 * \file configtablemodel.cpp
 * Model for table with context menu to add and remove rows.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Mar 2011
 *
 * Copyright (C) 2005-2018  Urs Fleisch
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

#include "configtablemodel.h"
#include "modelsectionresizemode.h"

/**
 * Constructor.
 * @param parent parent widget
 */
ConfigTableModel::ConfigTableModel(QObject* parent)
  : QAbstractTableModel(parent)
{
  setObjectName(QLatin1String("ConfigTableModel"));
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags ConfigTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid())
    theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  return theFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant ConfigTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_keyValues.size() ||
      index.column() < 0 || index.column() >= 2)
    return QVariant();
  const QPair<QString, QString>& keyValue = m_keyValues.at(index.row());
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == 0)
      return keyValue.first;
    else
      return keyValue.second;
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
bool ConfigTableModel::setData(const QModelIndex& index,
                                 const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole ||
      index.row() < 0 || index.row() >= m_keyValues.size() ||
      index.column() < 0 || index.column() >= 2)
    return false;
  QPair<QString, QString>& keyValue = m_keyValues[index.row()]; // clazy:exclude=detaching-member
  if (index.column() == 0) {
    keyValue.first = value.toString();
  } else {
    keyValue.second = value.toString();
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
QVariant ConfigTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal && section < m_labels.size()) {
    return m_labels[section];
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index, invalid for table models
 * @return number of rows,
 * if parent is valid number of children (0 for table models)
 */
int ConfigTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_keyValues.size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int ConfigTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

/**
 * Insert rows.
 * @param row rows are inserted before this row, if 0 at the begin,
 * if rowCount() at the end
 * @param count number of rows to insert
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool ConfigTableModel::insertRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_keyValues.insert(row, QPair<QString, QString>());
    endInsertRows();
  }
  return true;
}

/**
 * Remove rows.
 * @param row rows are removed starting with this row
 * @param count number of rows to remove
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool ConfigTableModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_keyValues.removeAt(row);
    endRemoveRows();
  }
  return true;
}

/**
 * Get the resize modes to be used for the columns.
 * @return list of resize modes for the columns
 */
QList<ModelSectionResizeMode>
    ConfigTableModel::getHorizontalResizeModes() const
{
  return {ModelSectionResizeMode::Stretch, ModelSectionResizeMode::Stretch};
}

/**
 * Set the column labels.
 * @param labels column labels
 */
void ConfigTableModel::setLabels(const QStringList& labels)
{
  beginResetModel();
  m_labels = labels;
  endResetModel();
}

/**
 * Set the model from a map.
 *
 * @param map list with keys and values
 */
void ConfigTableModel::setMap(const QList<QPair<QString, QString>>& map)
{
  beginResetModel();
  m_keyValues = map;
  // make sure that at least one line is in the table
  if (m_keyValues.isEmpty())
    m_keyValues.append({QString(), QString()});
  endResetModel();
}

/**
 * Get map from the model.
 * @return list with keys and values
 */
QList<QPair<QString, QString>> ConfigTableModel::getMap() const
{
  return m_keyValues;
}
