/**
 * \file configurabletreeview.h
 * QTreeView with configurable visibility, order and sort column.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2014
 *
 * Copyright (C) 2014-2017  Urs Fleisch
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

#ifndef CONFIGURABLETREEVIEW_H
#define CONFIGURABLETREEVIEW_H

#include <QTreeView>

/**
 * QTreeView with configurable visibility, order and sort column.
 */
class ConfigurableTreeView : public QTreeView {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit ConfigurableTreeView(QWidget* parent = 0);

  /**
   * Destructor.
   */
  virtual ~ConfigurableTreeView();

  /**
   * Set visible columns.
   * @param columns logical indexes of visible columns
   */
  void setVisibleColumns(const QList<int>& columns);

  /**
   * Get visible columns.
   * @return logical indexes of visible columns.
   */
  QList<int> getVisibleColumns() const;

  /**
   * Get sort column and order.
   * This method returns the values which can be set with sortByColumn().
   *
   * @param column the logical index of the sort column is returned here
   * @param order the sort order is returned here
   */
  void getSortByColumn(int& column, Qt::SortOrder& order) const;

  /**
   * Temporarily disconnect the model to improve performance.
   * The old model state is preserved and will be restored by reconnectModel().
   */
  void disconnectModel();

  /**
   * Reconnect to the model.
   * The state before the call to disconnectModel() is restored.
   */
  void reconnectModel();

private slots:
  /**
   * Show context menu for header.
   * @param pos context menu position
   */
  void showHeaderContextMenu(const QPoint& pos);

  /**
   * Toggle visibility of column.
   * @param visible true to set column visible
   */
  void toggleColumnVisibility(bool visible);

private:
  quint32 m_columnVisibility;

  /** State stored by disconnectModel() and restored by reconnectModel() */
  QAbstractItemModel* m_oldModel;
  QItemSelectionModel* m_oldSelectionModel;
  QPersistentModelIndex m_oldRootIndex;
};

#endif // CONFIGURABLETREEVIEW_H
