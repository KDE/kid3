/**
 * \file frametable.cpp
 * Table to edit frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Sep 2007
 *
 * Copyright (C) 2007-2018  Urs Fleisch
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

#include "frametable.h"
#include <QAction>
#include <QPoint>
#include <QHeaderView>
#include <QMenu>
#include <QChildEvent>
#include <QLineEdit>
#include "frametablemodel.h"
#include "frameitemdelegate.h"

/**
 * Constructor.
 *
 * @param model frame table model
 * @param genreModel genre model
 * @param parent parent widget
 */
FrameTable::FrameTable(FrameTableModel* model, GenreModel* genreModel,
                       QWidget* parent) :
  QTableView(parent), m_currentEditor(nullptr)
{
  setObjectName(QLatin1String("FrameTable"));
  setModel(model);
  setSelectionMode(SingleSelection);
  horizontalHeader()->setSectionResizeMode(FrameTableModel::CI_Value, QHeaderView::Stretch);
  // Set a small height instead of hiding the header, so that the column
  // widths can still be resized by the user.
  horizontalHeader()->setFixedHeight(2);
  horizontalHeader()->setStyleSheet(QLatin1String("color: rgba(0, 0, 0, 0);"));
  verticalHeader()->hide();
  if (model->isId3v1()) {
    bool insertTemporaryRow = model->rowCount() < 1;
    if (insertTemporaryRow)
      model->insertRow(0);
    setMinimumHeight((Frame::FT_LastV1Frame + 1) * (rowHeight(0) + 1));
    if (insertTemporaryRow)
      model->removeRow(0);
  }
  // Set width of first column
  int width = fontMetrics().width(tr("WWW Audio Source") + QLatin1String("WW"));
  QStyleOptionButton option;
  option.initFrom(this);
  width += style()->subElementRect(
    QStyle::SE_ViewItemCheckIndicator, &option, this).width();
  setColumnWidth(FrameTableModel::CI_Enable, width);

  horizontalHeader()->setSectionResizeMode(FrameTableModel::CI_Value, QHeaderView::Stretch);
  setItemDelegate(new FrameItemDelegate(genreModel, this));
  setEditTriggers(AllEditTriggers);
  viewport()->installEventFilter(this); // keep track of editors
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QWidget::customContextMenuRequested,
      this, &FrameTable::customContextMenu);
}

/**
 * Destructor.
 */
FrameTable::~FrameTable() {}

/**
 * Filters events if this object has been installed as an event filter
 * for the watched object.
 * This method is reimplemented to keep track of the current open editor.
 * It has to be installed on the viewport of the table.
 * @param event event
 * @return true to filter event out.
 */
bool FrameTable::eventFilter(QObject*, QEvent* event)
{
  if (event) {
    QEvent::Type type = event->type();
    if (type == QEvent::ChildAdded) {
      QObject* obj = ((QChildEvent*)event)->child();
      if (obj && obj->isWidgetType()) {
        m_currentEditor = (QWidget*)obj;
      }
    } else if (type == QEvent::ChildRemoved) {
      if (m_currentEditor == ((QChildEvent*)event)->child()) {
        m_currentEditor = nullptr;
      }
    } else if (type == QEvent::WindowDeactivate) {
      // this is done to avoid losing focus when the window is deactivated
      // while editing a cell (i.e. the cell is not closed by pressing Enter)
      if ((state() == QAbstractItemView::EditingState) && m_currentEditor) {
        commitData(m_currentEditor);
        closeEditor(m_currentEditor, QAbstractItemDelegate::EditPreviousItem);
      }
    }
  }
  return false;
}

/**
 * Commit data from the current editor.
 * This is used to avoid losing the changes in open editors e.g. when
 * the file is changed using Alt-Up or Alt-Down.
 *
 * @return true if data was committed.
 */
bool FrameTable::acceptEdit()
{
  if ((state() == QAbstractItemView::EditingState) && m_currentEditor) {
    commitData(m_currentEditor);
    //  close editor to avoid being stuck in QAbstractItemView::NoState
    closeEditor(m_currentEditor, QAbstractItemDelegate::NoHint);
    return true;
  }
  return false;
}

/**
 * Get current editor widget if the table is currently in edit state.
 * @return current editor widget, 0 if not in edit state.
 */
const QWidget* FrameTable::getCurrentEditor() const
{
  return state() == EditingState ? m_currentEditor : nullptr;
}

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void FrameTable::contextMenu(int row, int col, const QPoint& pos)
{
  const auto ftModel =
    qobject_cast<const FrameTableModel*>(model());
  if (ftModel && col == 0 && row >= 0) {
    QMenu menu(this);
    QAction* action = menu.addAction(tr("&Select all"));
    connect(action, &QAction::triggered, ftModel, &FrameTableModel::selectAllFrames);
    action = menu.addAction(tr("&Deselect all"));
    connect(action, &QAction::triggered, ftModel, &FrameTableModel::deselectAllFrames);
    menu.setMouseTracking(true);
    menu.exec(pos);
  }
}

/**
 * Display custom context menu.
 *
 * @param pos position where context menu is drawn on screen
 */
void FrameTable::customContextMenu(const QPoint& pos)
{
  QModelIndex index = indexAt(pos);
  if (index.isValid()) {
    contextMenu(index.row(), index.column(), mapToGlobal(pos));
  }
}

/**
 * Select in the editor of a value row.
 * @param row row number
 * @param start start position
 * @param length number of characters to select
 */
void FrameTable::setValueSelection(int row, int start, int length)
{
  if (const auto ftModel =
      qobject_cast<const FrameTableModel*>(model())) {
    QModelIndex idx = ftModel->index(row, FrameTableModel::CI_Value);
    if (idx.isValid()) {
      scrollTo(idx);
      setCurrentIndex(idx);
      edit(idx);
      if (auto le = qobject_cast<QLineEdit*>(indexWidget(idx))) {
        le->setSelection(start, length);
      }
    }
  }
}
