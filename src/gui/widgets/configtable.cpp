/**
 * \file configtable.cpp
 * Table with context menu to add, delete and clear rows.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Oct 2005
 *
 * Copyright (C) 2005-2013  Urs Fleisch
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

#include "configtable.h"
#include <QToolTip>
#include <QMenu>

/**
 * Constructor.
 *
 * @param parent parent widget
 */
ConfigTable::ConfigTable(QAbstractItemModel* model, QWidget* parent) :
  AbstractListEdit(m_tableView = new QTableView, model, parent)
{
  setObjectName(QLatin1String("ConfigTable"));
  setAddButtonText(tr("&Add"));
  hideEditButton();
  m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tableView, &QWidget::customContextMenuRequested,
      this, &ConfigTable::customContextMenu);
}

/**
 * Destructor.
 */
ConfigTable::~ConfigTable() {}

/**
 * Set the resize modes to be used for the columns.
 * @param resizeModes list of resize modes for the columns
 */
void ConfigTable::setHorizontalResizeModes(
  const QList<QHeaderView::ResizeMode>& resizeModes)
{
  QHeaderView* header = m_tableView->horizontalHeader();
  int col = 0;
  for (QHeaderView::ResizeMode mode : resizeModes)
    header->setSectionResizeMode(col++, mode);
}

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void ConfigTable::addRow(int row)
{
  m_tableView->model()->insertRow(row + 1);
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void ConfigTable::deleteRow(int row)
{
  if (m_tableView->model()->rowCount() <= 1)
    return;
  m_tableView->model()->removeRow(row);
}

/**
 * Clear a row in the table.
 *
 * @param row row to clear
 */
void ConfigTable::clearRow(int row)
{
  if (row < m_tableView->model()->rowCount() && m_tableView->model()->removeRow(row))
    m_tableView->model()->insertRow(row);
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
void ConfigTable::executeAction(QAction* action)
{
  if (action) {
    int row = action->data().toInt();
    int cmd = row & 3;
    row >>= 2;
    switch (cmd) {
      case 0:
        addRow(row);
        break;
      case 1:
        deleteRow(row);
        break;
      case 2:
      default:
        clearRow(row);
        break;
    }
  }
}

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void ConfigTable::contextMenu(int row, int /* col */, const QPoint& pos)
{
  QMenu menu(this);
  QAction* action;
  if (row >= -1) {
    action = menu.addAction(tr("&Insert row"));
    if (action) action->setData((row << 2) | 0);
  }
  if (row >= 0) {
    action = menu.addAction(tr("&Delete row"));
    if (action) action->setData((row << 2) | 1);
  }
  if (row >= 0) {
    action = menu.addAction(tr("&Clear row"));
    if (action) action->setData((row << 2) | 2);
  }
  connect(&menu, &QMenu::triggered, this, &ConfigTable::executeAction);
  menu.setMouseTracking(true);
  menu.exec(pos);
}

/**
 * Display custom context menu.
 *
 * @param pos position where context menu is drawn on screen
 */
void ConfigTable::customContextMenu(const QPoint& pos)
{
  QModelIndex index = m_tableView->indexAt(pos);
  if (index.isValid()) {
    contextMenu(index.row(), index.column(), mapToGlobal(pos));
  }
}

/**
 * Add a new item.
 */
void ConfigTable::addItem()
{
  addRow(getItemView()->model()->rowCount());
}

/**
 * Edit the selected item.
 */
void ConfigTable::editItem()
{
}
