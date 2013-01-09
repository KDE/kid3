/**
 * \file configtable.cpp
 * Table with context menu to add, delete and clear rows.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Oct 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
ConfigTable::ConfigTable(QWidget* parent) :
  QTableView(parent)
{
  setObjectName("ConfigTable");
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(customContextMenu(const QPoint&)));
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
  QHeaderView* header = horizontalHeader();
  int col = 0;
  foreach (QHeaderView::ResizeMode mode, resizeModes)
#if QT_VERSION >= 0x050000
    header->setSectionResizeMode(col++, mode);
#else
    header->setResizeMode(col++, mode);
#endif
}

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void ConfigTable::addRow(int row)
{
  model()->insertRow(row + 1);
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void ConfigTable::deleteRow(int row)
{
  if (model()->rowCount() <= 1)
    return;
  model()->removeRow(row);
}

/**
 * Clear a row in the table.
 *
 * @param row row to clear
 */
void ConfigTable::clearRow(int row)
{
  if (row < model()->rowCount() && model()->removeRow(row))
    model()->insertRow(row);
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
    action = menu.addAction(i18n("&Insert row"));
    if (action) action->setData((row << 2) | 0);
  }
  if (row >= 0) {
    action = menu.addAction(i18n("&Delete row"));
    if (action) action->setData((row << 2) | 1);
  }
  if (row >= 0) {
    action = menu.addAction(i18n("&Clear row"));
    if (action) action->setData((row << 2) | 2);
  }
  connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
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
  QModelIndex index = indexAt(pos);
  if (index.isValid()) {
    contextMenu(index.row(), index.column(), mapToGlobal(pos));
  }
}
