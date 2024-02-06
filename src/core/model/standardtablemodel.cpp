/**
 * \file standardtablemodel.h
 * Table model with containing values for multiple roles.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Jul 2019
 *
 * Copyright (C) 2019-2024  Urs Fleisch
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

#include "standardtablemodel.h"

StandardTableModel::StandardTableModel(QObject* parent)
  : QAbstractTableModel(parent),
    m_numColumns(1)
{
}

Qt::ItemFlags StandardTableModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable |
      Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant StandardTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
      section >= 0 && section < m_horizontalHeaderLabels.size()) {
    return m_horizontalHeaderLabels.at(section);
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

bool StandardTableModel::setHeaderData(
    int section, Qt::Orientation orientation, const QVariant& value, int role)
{
  if (orientation == Qt::Horizontal &&
      (role == Qt::DisplayRole || role == Qt::EditRole) &&
      section >= 0 && section < columnCount()) {
    if (section >= m_horizontalHeaderLabels.size()) {
      m_horizontalHeaderLabels.resize(section + 1);
    }
    m_horizontalHeaderLabels[section] = value.toString();
    return true;
  }
  return false;
}

Qt::DropActions StandardTableModel::supportedDropActions() const
{
  return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

int StandardTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_cont.size();
}

int StandardTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_numColumns;
}

QVariant StandardTableModel::data(const QModelIndex& index, int role) const
{
  const int row = index.row(), column = index.column();
  if (row < 0 || row >= m_cont.size() ||
      column < 0 || column >= m_numColumns)
    return QVariant();

  if (role == Qt::EditRole)
    role = Qt::DisplayRole;

  const auto& r = m_cont.at(row);
  return column < r.size() ? r.at(column).value(role) : QVariant();
}

bool StandardTableModel::setData(const QModelIndex& index,
                                      const QVariant& value, int role)
{
  const int row = index.row(), column = index.column();
  if (row < 0 || row >= m_cont.size() ||
      column < 0 || column >= m_numColumns)
    return false;

  if (role == Qt::EditRole)
    role = Qt::DisplayRole;

  auto& r = m_cont[row];
  if (column >= r.size())
    r.resize(m_numColumns);

  auto& c = r[column];
  if (auto it = c.find(role); it != c.end()) {
    if (*it != value) {
      *it = value;
      emit dataChanged(index, index);
    }
  } else {
    c.insert(role, value);
  }
  return true;
}

bool StandardTableModel::insertRows(int row, int count,
                                   const QModelIndex& parent)
{
  if (count < 1 || row < 0 || row > rowCount(parent))
    return false;
  beginInsertRows(QModelIndex(), row, row + count - 1);
  m_cont.insert(row, count, {});
  endInsertRows();
  return true;
}

bool StandardTableModel::removeRows(int row, int count,
                                   const QModelIndex& parent)
{
  if (count <= 0 || row < 0 || row + count > rowCount(parent))
    return false;
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  m_cont.remove(row, count);
  endRemoveRows();
  return true;
}

void StandardTableModel::setColumnCount(int columns)
{
  if (m_numColumns < columns) {
    beginInsertColumns(QModelIndex(), m_numColumns, columns - 1);
    m_numColumns = columns;
    endInsertColumns();
  } else if (m_numColumns > columns) {
    beginRemoveColumns(QModelIndex(), columns, m_numColumns - 1);
    m_numColumns = columns;
    endRemoveColumns();
  }
}

void StandardTableModel::setHorizontalHeaderLabels(
    const QStringList& labels)
{
  if (labels.size() <= columnCount()) {
    m_horizontalHeaderLabels = labels.toVector();
  }
}

void StandardTableModel::clear()
{
  if (m_cont.size() > 0) {
    beginRemoveRows(QModelIndex(), 0, m_cont.size() - 1);
    m_cont.clear();
    endRemoveRows();
  }
}
