/**
 * \file configurabletreeview.cpp
 * QTreeView with configurable visibility, order and sort column.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "configurabletreeview.h"
#include <QHeaderView>
#include <QMenu>
#include <QAction>

/**
 * Constructor.
 * @param parent parent widget
 */
ConfigurableTreeView::ConfigurableTreeView(QWidget* parent) : QTreeView(parent),
  m_columnVisibility(0xffffffff)
{
  QHeaderView* headerView = header();
  setSortingEnabled(true);
#if QT_VERSION >= 0x050000
  headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
  headerView->setResizeMode(QHeaderView::ResizeToContents);
#endif
  headerView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header(), SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(showHeaderContextMenu(QPoint)));
}

/**
 * Destructor.
 */
ConfigurableTreeView::~ConfigurableTreeView()
{
}

/**
 * Show context menu for header.
 * @param pos context menu position
 */
void ConfigurableTreeView::showHeaderContextMenu(const QPoint& pos)
{
  QHeaderView* headerView = header();
  QMenu menu(headerView);
  for (int column = 1; column < headerView->count(); ++column) {
    const quint32 mask = 1 << column;
    QAction* action = new QAction(&menu);
    action->setText(model()->headerData(column, Qt::Horizontal).toString());
    action->setData(column);
    action->setCheckable(true);
    action->setChecked((m_columnVisibility & mask) != 0);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(toggleColumnVisibility(bool)));
    menu.addAction(action);
  }
  menu.setMouseTracking(true);
  menu.exec(headerView->mapToGlobal(pos));
}

/**
 * Toggle visibility of column.
 * @param visible true to set column visible
 */
void ConfigurableTreeView::toggleColumnVisibility(bool visible)
{
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    bool ok;
    int column = action->data().toInt(&ok);
    if (ok) {
      if (visible) {
        m_columnVisibility |= 1 << column;
      } else {
        m_columnVisibility &= ~(1 << column);
      }
      setColumnHidden(column, !visible);
    }
  }
}

/**
 * Set visible columns.
 * @param columns logical indexes of visible columns
 */
void ConfigurableTreeView::setVisibleColumns(const QList<int>& columns)
{
  QHeaderView* headerView = header();
  if (!columns.isEmpty()) {
    m_columnVisibility = 0;
    for (int visualIdx = 0; visualIdx < columns.size(); ++visualIdx) {
      int logicalIdx = columns.at(visualIdx);
      int oldVisualIdx = headerView->visualIndex(logicalIdx);
      headerView->moveSection(oldVisualIdx, visualIdx);
      headerView->showSection(logicalIdx);
      m_columnVisibility |= 1 << logicalIdx;
    }
    for (int visualIdx = columns.size(); visualIdx < headerView->count();
         ++visualIdx) {
      headerView->hideSection(headerView->logicalIndex(visualIdx));
    }
  } else {
    m_columnVisibility = 0xffffffff;
  }
}

/**
 * Get visible columns.
 * @return logical indexes of visible columns.
 */
QList<int> ConfigurableTreeView::getVisibleColumns() const
{
  QList<int> columns;
  const QHeaderView* headerView = header();
  for (int visualIdx = 0; visualIdx < headerView->count(); ++visualIdx) {
    int logicalIdx = headerView->logicalIndex(visualIdx);
    if (!headerView->isSectionHidden(logicalIdx)) {
      columns.append(logicalIdx);
    }
  }
  return columns;
}

/**
 * Get sort column and order.
 * This method returns the values which can be set with sortByColumn().
 *
 * @param column the logical index of the sort column is returned here
 * @param order the sort order is returned here
 */
void ConfigurableTreeView::getSortByColumn(int& column,
                                           Qt::SortOrder& order) const {
  const QHeaderView* headerView = header();
  column = headerView->sortIndicatorSection();
  order = headerView->sortIndicatorOrder();
}
