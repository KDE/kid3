/**
 * \file checkablestringlistmodel.cpp
 * String list model with checkable items.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Dec 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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

#include "checkablestringlistmodel.h"

/**
 * Constructor.
 * @param parent parent widget
 */
CheckableStringListModel::CheckableStringListModel(QObject* parent) :
  QStringListModel(parent)
{
}

/**
 * Destructor.
 */
CheckableStringListModel::~CheckableStringListModel()
{
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags CheckableStringListModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QStringListModel::flags(index);
  if (index.isValid()) {
    theFlags &= ~(Qt::ItemIsEditable | Qt::ItemIsDropEnabled);
    theFlags |= Qt::ItemIsUserCheckable;
  }
  return theFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant CheckableStringListModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::CheckStateRole &&
      index.isValid() && index.column() == 0 &&
      index.row() >= 0 && index.row() < 64) {
    return m_bitMask & (1ULL << index.row()) ? Qt::Checked : Qt::Unchecked;
  }
  return QStringListModel::data(index, role);
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool CheckableStringListModel::setData(const QModelIndex& index,
                                       const QVariant& value, int role)
{
  if (role == Qt::CheckStateRole &&
      index.isValid() && index.column() == 0 &&
      index.row() >= 0 && index.row() < 64) {
    quint64 mask = 1ULL << index.row();
    if (value == Qt::Checked) {
      m_bitMask |= mask;
    } else if (value == Qt::Unchecked) {
      m_bitMask &= ~mask;
    }
    return true;
  }
  return QStringListModel::setData(index, value, role);
}

/**
 * Insert rows.
 * @param row first row
 * @param count number of rows to insert
 * @param parent parent model index
 * @return true if rows were successfully inserted.
 */
bool CheckableStringListModel::insertRows(int row, int count,
                                          const QModelIndex& parent)
{
  quint64 mask = (1ULL << row) - 1;
  m_bitMask = (m_bitMask & mask) | ((m_bitMask & ~mask) << count);
  return QStringListModel::insertRows(row, count, parent);
}

/**
 * removeRows
 * @param row first row
 * @param count number of rows to remove
 * @param parent parent model index
 * @return true if rows were successfully removed.
 */
bool CheckableStringListModel::removeRows(int row, int count,
                                          const QModelIndex& parent)
{
  m_bitMask = (m_bitMask & ((1ULL << row) - 1)) |
      ((m_bitMask & ~((1ULL << (row + count)) - 1)) >> count);
  return QStringListModel::removeRows(row, count, parent);
}
