/**
 * \file configtable.h
 * Table with context menu to add, delete and clear rows.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Oct 2005
 *
 * Copyright (C) 2005-2024  Urs Fleisch
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

#pragma once

#include <QTableView>
#include <QList>
#include <QHeaderView>
#include "abstractlistedit.h"

enum class ModelSectionResizeMode : int;

/**
 * Context menu commands configuration table.
 */
class ConfigTable : public AbstractListEdit {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param model item model
   * @param parent parent widget
   */
  explicit ConfigTable(QAbstractItemModel* model, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  ~ConfigTable() override = default;

  /**
   * Set the resize modes to be used for the columns.
   * @param resizeModes list of resize modes for the columns
   */
  void setHorizontalResizeModes(const QList<ModelSectionResizeMode>& resizeModes);

public slots:
  /**
   * Add a new item.
   */
  void addItem() override;

  /**
   * Edit the selected item.
   */
  void editItem() override;

  /**
   * Insert a new row into the table.
   *
   * @param row the new row is inserted after this row
   */
  void addRow(int row);

  /**
   * Delete a row from the table.
   *
   * @param row row to delete
   */
  void deleteRow(int row);

  /**
   * Clear a row in the table.
   *
   * @param row row to clear
   */
  void clearRow(int row);

  /**
   * Execute a context menu action.
   *
   * @param action action of selected menu
   */
  void executeAction(const QAction* action);

  /**
   * Display context menu.
   *
   * @param row row at which context menu is displayed
   * @param col column at which context menu is displayed
   * @param pos position where context menu is drawn on screen
   */
  void contextMenu(int row, int col, const QPoint& pos);

  /**
   * Display custom context menu.
   *
   * @param pos position where context menu is drawn on screen
   */
  void customContextMenu(const QPoint& pos);

private:
  QTableView* m_tableView;
};
