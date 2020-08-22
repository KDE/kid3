/**
 * \file configurabletreeview.h
 * QTreeView with configurable visibility, order and sort column.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include <QTreeView>

class QActionGroup;

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
  explicit ConfigurableTreeView(QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~ConfigurableTreeView() override = default;

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
   * Set if custom column widths are enabled.
   * @param enable true to enable custom column widths, false for automatic
   * column widths
   */
  void setCustomColumnWidthsEnabled(bool enable);

  /**
   * Check if custom column widths are enabled.
   * @return true if custom column widths are enabled.
   */
  bool areCustomColumnWidthsEnabled() const;

  /**
   * Initialize custom column widths from contents if not yet valid.
   * @param minimumWidth minimum width for column, -1 if not used
   * @return size of first section, -1 when initialization not necessary.
   */
  int initializeColumnWidthsFromContents(int minimumWidth);

  /**
   * Set column widths.
   * @param columnWidths column widths
   */
  void setColumnWidths(const QList<int>& columnWidths);

  /**
   * Get column widths.
   * @return column widths.
   */
  QList<int> getColumnWidths() const;

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

signals:
  /**
   * Emitted when the parent shall be activated.
   * This is emitted when Command + Up is pressed to mimic the shortcut of the
   * macOS Finder.
   * @param index root index of item view
   */
  void parentActivated(const QModelIndex& index);

protected:
  /**
   * Reimplemented to go to parent item with Left key and
   * make Return/Enter send activated() also on the Mac.
   * @param event key event
   */
  virtual void keyPressEvent(QKeyEvent* event) override;

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
  /**
   * Set column widths to custom column widths set with setColumnWidths().
   * @return true if custom column widths settings could be applied.
   */
  bool resizeColumnWidths();

  quint32 m_columnVisibility;

  /** State stored by disconnectModel() and restored by reconnectModel() */
  QAbstractItemModel* m_oldModel;
  QItemSelectionModel* m_oldSelectionModel;
  QPersistentModelIndex m_oldRootIndex;
  QList<int> m_columnWidths;
  QActionGroup* m_columnActionGroup;
  QAction* m_autoColumnAction;
  QAction* m_customColumnAction;
};
