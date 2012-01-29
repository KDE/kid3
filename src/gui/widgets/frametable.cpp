/**
 * \file frametable.cpp
 * Table to edit frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Sep 2007
 *
 * Copyright (C) 2007-2011  Urs Fleisch
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
#include "frametablemodel.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param model frame table model
 * @param parent parent widget
 */
FrameTable::FrameTable(FrameTableModel* model, QWidget* parent) :
  QTableView(parent), m_currentEditor(0)
{
  setObjectName("FrameTable");
  setModel(model);
  setSelectionMode(SingleSelection);
  horizontalHeader()->setResizeMode(FrameTableModel::CI_Value, QHeaderView::Stretch);
  horizontalHeader()->hide();
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
  int width = fontMetrics().width(i18n("Track Number") + "WW");
  QStyleOptionButton option;
  option.initFrom(this);
  width += style()->subElementRect(
    QStyle::SE_ViewItemCheckIndicator, &option, this).width();
  setColumnWidth(FrameTableModel::CI_Enable, width);

  horizontalHeader()->setResizeMode(FrameTableModel::CI_Value, QHeaderView::Stretch);
  setItemDelegate(new FrameItemDelegate(this));
  setEditTriggers(AllEditTriggers);
  viewport()->installEventFilter(this); // keep track of editors
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(customContextMenu(const QPoint&)));
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
        m_currentEditor = 0;
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
 * This is used to avoid loosing the changes in open editors e.g. when
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
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void FrameTable::contextMenu(int row, int col, const QPoint& pos)
{
  const FrameTableModel* ftModel =
    qobject_cast<const FrameTableModel*>(model());
  if (ftModel && col == 0 && row >= 0) {
    QMenu menu(this);
    QAction* action = menu.addAction(i18n("&Select all"));
    connect(action, SIGNAL(triggered()), ftModel, SLOT(selectAllFrames()));
    action = menu.addAction(i18n("&Deselect all"));
    connect(action, SIGNAL(triggered()), ftModel, SLOT(deselectAllFrames()));
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