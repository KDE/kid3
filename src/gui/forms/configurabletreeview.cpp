/**
 * \file configurabletreeview.cpp
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

#include "configurabletreeview.h"
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>

/**
 * Constructor.
 * @param parent parent widget
 */
ConfigurableTreeView::ConfigurableTreeView(QWidget* parent) : QTreeView(parent),
  m_columnVisibility(0xffffffff), m_oldModel(nullptr), m_oldSelectionModel(nullptr)
{
  QHeaderView* headerView = header();
  setSortingEnabled(true);
  headerView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header(), &QWidget::customContextMenuRequested,
          this, &ConfigurableTreeView::showHeaderContextMenu);

  m_columnActionGroup = new QActionGroup(this);
  m_autoColumnAction = new QAction(m_columnActionGroup);
  m_autoColumnAction->setText(tr("Automatic Column Widths"));
  m_autoColumnAction->setCheckable(true);
  connect(m_autoColumnAction, &QAction::triggered,
          this, [this](bool checked){
    setCustomColumnWidthsEnabled(!checked);
  });
  m_customColumnAction = new QAction(m_columnActionGroup);
  m_customColumnAction->setText(tr("Custom Column Widths"));
  m_customColumnAction->setCheckable(true);
  connect(m_customColumnAction, &QAction::triggered,
          this, [this](bool checked) {
    setCustomColumnWidthsEnabled(checked);
  });
  setCustomColumnWidthsEnabled(false);
}

/**
 * Reimplemented to go to parent item with Left key and
 * make Return/Enter send activated() also on the Mac.
 * @param event key event
 */
void ConfigurableTreeView::keyPressEvent(QKeyEvent* event)
{
  if ((state() != EditingState || hasFocus()) &&
      (!m_openParentKey.isEmpty() || !m_openCurrentKey.isEmpty())) {
    // First create a key sequence from the key event and modifiers.
    int keyCode = event->key();
    Qt::Key key = static_cast<Qt::Key>(keyCode);
    if (key != Qt::Key_unknown && key != Qt::Key_Control &&
        key != Qt::Key_Shift && key != Qt::Key_Alt && key != Qt::Key_Meta) {
      Qt::KeyboardModifiers modifiers = event->modifiers();
      if (modifiers & Qt::ShiftModifier) {
        keyCode += Qt::SHIFT;
      }
      if (modifiers & Qt::ControlModifier) {
        keyCode += Qt::CTRL;
      }
      if (modifiers & Qt::AltModifier) {
        keyCode += Qt::ALT;
      }
      if (modifiers & Qt::MetaModifier) {
        keyCode += Qt::META;
      }
      QKeySequence keySequence(keyCode);

      // Open the parent folder if the "open_parent" key
      // (Ctrl+Up by default) is pressed.
      if (keySequence.matches(m_openParentKey) == QKeySequence::ExactMatch) {
        QModelIndex idx = rootIndex();
        if (idx.isValid()) {
          emit parentActivated(idx);
        }
        event->ignore();
        return;
      }
      // Open the current folder if the "open_current" key
      // (Ctrl+Down by default) is pressed.
      if (keySequence.matches(m_openCurrentKey) == QKeySequence::ExactMatch) {
        QModelIndex idx = currentIndex();
        if (idx.isValid()) {
          emit activated(idx);
        }
        event->ignore();
        return;
      }
    }
  }

  switch (event->key()) {
  // When the left arrow key is pressed on an item without children,
  // go to its parent item.
  case Qt::Key_Left:
    if (state() != EditingState || hasFocus()) {
      QPersistentModelIndex oldCurrent = currentIndex();
      QAbstractItemModel* mdl = model();
      QItemSelectionModel* selMdl = selectionModel();
      if (mdl && selMdl && oldCurrent.isValid() &&
          mdl->rowCount(oldCurrent) == 0) {
        QPersistentModelIndex newCurrent = mdl->parent(oldCurrent);
        if (newCurrent.isValid() && newCurrent != rootIndex()) {
          setCurrentIndex(newCurrent);
          event->accept();
          return;
        }
      }
    }
    break;
#ifdef Q_OS_MAC
  case Qt::Key_Enter:
  case Qt::Key_Return:
    if (state() != EditingState || hasFocus()) {
      QModelIndex idx = currentIndex();
      if (idx.isValid()) {
        emit activated(idx);
      }
      event->ignore();
    }
    break;
#endif
  }
  QTreeView::keyPressEvent(event);
}

/**
 * Set keyboard shortcuts for the open parent and open current actions.
 * @param map map of action names to key sequences
 */
void ConfigurableTreeView::setShortcuts(const QMap<QString, QKeySequence>& map)
{
  auto it = map.constFind(QLatin1String("open_parent"));
  if (it != map.constEnd()) {
    m_openParentKey = *it;
  }
  it = map.constFind(QLatin1String("open_current"));
  if (it != map.constEnd()) {
    m_openCurrentKey = *it;
  }
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
    auto action = new QAction(&menu);
    action->setText(model()->headerData(column, Qt::Horizontal).toString());
    action->setData(column);
    action->setCheckable(true);
    action->setChecked((m_columnVisibility & mask) != 0);
    connect(action, &QAction::triggered,
            this, &ConfigurableTreeView::toggleColumnVisibility);
    menu.addAction(action);
  }
  menu.addSeparator();
  menu.addAction(m_autoColumnAction);
  menu.addAction(m_customColumnAction);

  menu.setMouseTracking(true);
  menu.exec(headerView->mapToGlobal(pos));
}

/**
 * Toggle visibility of column.
 * @param visible true to set column visible
 */
void ConfigurableTreeView::toggleColumnVisibility(bool visible)
{
  if (auto action = qobject_cast<QAction*>(sender())) {
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
      columns.append(logicalIdx); // clazy:exclude=reserve-candidates
    }
  }
  return columns;
}

/**
 * Set if custom column widths are enabled.
 * @param enable true to enable custom column widths, false for automatic
 * column widths
 */
void ConfigurableTreeView::setCustomColumnWidthsEnabled(bool enable)
{
  m_customColumnAction->setChecked(enable);
  m_autoColumnAction->setChecked(!enable);
  if (QHeaderView* hdr = header()) {
    hdr->setSectionResizeMode(
          enable ? QHeaderView::Interactive : QHeaderView::ResizeToContents);
  }
  if (enable) {
    resizeColumnWidths();
  }
}

/**
 * Check if custom column widths are enabled.
 * @return true if custom column widths are enabled.
 */
bool ConfigurableTreeView::areCustomColumnWidthsEnabled() const
{
  return m_customColumnAction->isChecked();
}

/**
 * Set column widths to custom column widths set with setColumnWidths().
 * @return true if custom column widths settings could be applied.
 */
bool ConfigurableTreeView::resizeColumnWidths()
{
  if (QHeaderView* hdr = header()) {
    if (m_columnWidths.size() == hdr->count()) {
      int logicalIdx = 0;
      for (auto it = m_columnWidths.constBegin();
           it != m_columnWidths.constEnd();
           ++it, ++logicalIdx) {
        int width = *it;
        hdr->resizeSection(logicalIdx, width);
      }
      return true;
    }
  }
  return false;
}

/**
 * Initialize custom column widths from contents if not yet valid.
 * @param minimumWidth minimum width for column, -1 if not used
 * @return size of first section, -1 when initialization not necessary.
 */
int ConfigurableTreeView::initializeColumnWidthsFromContents(int minimumWidth)
{
  if (QHeaderView* hdr = header()) {
    if (areCustomColumnWidthsEnabled() &&
        m_columnWidths.size() != hdr->count()) {
      int firstSectionSize = 0;
      for (int logicalIdx = 0; logicalIdx < hdr->count(); ++logicalIdx) {
        if (!hdr->isSectionHidden(logicalIdx)) {
          resizeColumnToContents(logicalIdx);
          if (firstSectionSize <= 0) {
            firstSectionSize = hdr->sectionSize(logicalIdx);
            if (firstSectionSize < minimumWidth) {
              hdr->resizeSection(logicalIdx, minimumWidth);
            }
          }
        }
      }
      m_columnWidths = getColumnWidths();
      return firstSectionSize;
    }
  }
  return -1;
}

/**
 * Set column widths.
 * @param columnWidths column widths
 */
void ConfigurableTreeView::setColumnWidths(const QList<int>& columnWidths)
{
  m_columnWidths = columnWidths;
  if (areCustomColumnWidthsEnabled()) {
    resizeColumnWidths();
  }
}

/**
 * Get column widths.
 * @return column widths.
 */
QList<int> ConfigurableTreeView::getColumnWidths() const
{
  QList<int> columnWidths;
  if (QHeaderView* hdr = header()) {
    columnWidths.reserve(hdr->count());
    for (int logicalIndex = 0; logicalIndex < hdr->count(); ++logicalIndex) {
      columnWidths.append(hdr->sectionSize(logicalIndex));
    }
  }
  return columnWidths;
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

/**
 * Temporarily disconnect the model to improve performance.
 * The old model state is preserved and will be restored by reconnectModel().
 */
void ConfigurableTreeView::disconnectModel()
{
  if (!m_oldModel) {
    m_oldRootIndex = rootIndex();
    m_oldSelectionModel = selectionModel();
    m_oldModel = model();
    setModel(nullptr);
  }
}


/**
 * Reconnect to the model.
 * The state before the call to disconnectModel() is restored.
 */
void ConfigurableTreeView::reconnectModel()
{
  if (m_oldModel) {
    setModel(m_oldModel);
    setSelectionModel(m_oldSelectionModel);
    setRootIndex(QModelIndex());
    setRootIndex(m_oldRootIndex);
    m_oldRootIndex = QPersistentModelIndex();
    m_oldSelectionModel = nullptr;
    m_oldModel = nullptr;
  }
}
