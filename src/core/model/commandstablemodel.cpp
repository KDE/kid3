/**
 * \file commandstablemodel.cpp
 * Context menu commands configuration table model.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Mar 2011
 *
 * Copyright (C) 2005-2011  Urs Fleisch
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

#include "commandstablemodel.h"
#include "commandformatreplacer.h"
#include "qtcompatmac.h"

/** Column indices. */
enum ColumnIndex {
  CI_Confirm,
  CI_Output,
  CI_Name,
  CI_Command,
  CI_NumColumns
};

/**
 * Constructor.
 * @param parent parent widget
 */
CommandsTableModel::CommandsTableModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  setObjectName("CommandsTableModel");
}

/**
 * Destructor.
 */
CommandsTableModel::~CommandsTableModel()
{
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags CommandsTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid()) {
    theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == CI_Confirm || index.column() == CI_Output) {
      theFlags |= Qt::ItemIsUserCheckable;
    } else {
      theFlags |= Qt::ItemIsEditable;
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
QVariant CommandsTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_cmdList.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return QVariant();
  const MiscConfig::MenuCommand& item = m_cmdList.at(index.row());
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
    case CI_Name:
      return item.getName();
    case CI_Command:
      return item.getCommand();
    default: ;
    }
  }
  if (role == Qt::CheckStateRole) {
    switch (index.column()) {
    case CI_Confirm:
      return item.mustBeConfirmed() ? Qt::Checked : Qt::Unchecked;
    case CI_Output:
      return item.outputShown() ? Qt::Checked : Qt::Unchecked;
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
bool CommandsTableModel::setData(const QModelIndex& index,
                                 const QVariant& value, int role)
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_cmdList.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return false;
  MiscConfig::MenuCommand& item = m_cmdList[index.row()];
  if (role == Qt::EditRole) {
    switch (index.column()) {
    case CI_Name:
      item.setName(value.toString());
      break;
    case CI_Command:
      item.setCommand(value.toString());
      break;
    default:
      return false;
    }
  } else if (role == Qt::CheckStateRole) {
    switch (index.column()) {
    case CI_Confirm:
      item.setMustBeConfirmed(value.toInt() == Qt::Checked);
      break;
    case CI_Output:
      item.setOutputShown(value.toInt() == Qt::Checked);
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
QVariant CommandsTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::ToolTipRole && orientation == Qt::Horizontal &&
      section == CI_Command)
    return CommandFormatReplacer::getToolTip();
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    switch (section) {
    case CI_Confirm:
      return i18n("Confirm");
    case CI_Output:
      return i18n("Output");
    case CI_Name:
      return i18n("Name");
    case CI_Command:
      return i18n("Command");
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
int CommandsTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_cmdList.size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int CommandsTableModel::columnCount(const QModelIndex& parent) const
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
bool CommandsTableModel::insertRows(int row, int count,
                        const QModelIndex&)
{
  beginInsertRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
    m_cmdList.insert(row, MiscConfig::MenuCommand());
  endInsertRows();
  return true;
}

/**
 * Remove rows.
 * @param row rows are removed starting with this row
 * @param count number of rows to remove
 * @return true if successful
 */
bool CommandsTableModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
    m_cmdList.removeAt(row);
  endRemoveRows();
  return true;
}

/**
 * Get the resize modes to be used for the columns.
 * @return list of resize modes for the columns
 */
QList<QHeaderView::ResizeMode>
    CommandsTableModel::getHorizontalResizeModes() const
{
  QList<QHeaderView::ResizeMode> resizeModes;
  for (int i = 0; i < CI_NumColumns; ++i) {
    QHeaderView::ResizeMode mode = QHeaderView::Interactive;
    if (i == CI_Confirm || i == CI_Output)
      mode = QHeaderView::ResizeToContents;
    else if (i == CI_Command)
      mode = QHeaderView::Stretch;
    resizeModes.append(mode);
  }
  return resizeModes;
}

/**
 * Set the model from the command list.
 * @param cmdList command list
 */
void CommandsTableModel::setCommandList(
  const QList<MiscConfig::MenuCommand>& cmdList)
{
  m_cmdList = cmdList;
  reset();
}

/**
 * Get the command list from the model.
 * @return command list
 */
QList<MiscConfig::MenuCommand> CommandsTableModel::getCommandList() const
{
  QList<MiscConfig::MenuCommand> cmdList;
  for (QList<MiscConfig::MenuCommand>::const_iterator it = m_cmdList.constBegin();
       it != m_cmdList.constEnd();
       ++it) {
    if (!it->getName().isEmpty()) {
      cmdList.append(*it);
    }
  }
  if (cmdList.isEmpty()) {
    // Make sure that their is at least one entry, so that new entries can
    // be added.
    cmdList.append(MiscConfig::MenuCommand());
  }
  return cmdList;
}
