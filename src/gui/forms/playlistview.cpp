/**
 * \file playlistview.cpp
 * List view for playlist items.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2018
 *
 * Copyright (C) 2018  Urs Fleisch
 *
 * This file is part of Kid3.
 * Contains adapted Python code from
 * http://stackoverflow.com/questions/26227885/drag-and-drop-rows-within-qtablewidget
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

#include "playlistview.h"
#include <algorithm>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QFileSystemModel>
#include <QAction>
#include <QUrl>

PlaylistView::PlaylistView(QWidget* parent)
  : QListView(parent), m_dropRole(QFileSystemModel::FilePathRole)
{
  auto deleteAction = new QAction(this);
  deleteAction->setShortcut(QKeySequence::Delete);
  deleteAction->setShortcutContext(Qt::WidgetShortcut);
  connect(deleteAction, &QAction::triggered,
          this, &PlaylistView::deleteCurrentRow);
  addAction(deleteAction);

  auto moveUpAction = new QAction(this);
  moveUpAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
  moveUpAction->setShortcutContext(Qt::WidgetShortcut);
  connect(moveUpAction, &QAction::triggered,
          this, &PlaylistView::moveUpCurrentRow);
  addAction(moveUpAction);

  auto moveDownAction = new QAction(this);
  moveDownAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
  moveDownAction->setShortcutContext(Qt::WidgetShortcut);
  connect(moveDownAction, &QAction::triggered,
          this, &PlaylistView::moveDownCurrentRow);
  addAction(moveDownAction);
}

/**
 * Check if the drop index if child of a dropped item.
 * @param event drop event
 * @param index drop index
 * @return true if dropping on itself.
 */
bool PlaylistView::droppingOnItself(QDropEvent* event, const QModelIndex& index)
{
  Qt::DropAction dropAction = event->dropAction();
  if (dragDropMode() == InternalMove) {
    dropAction = Qt::MoveAction;
  }
  if (event->source() == this &&
      (event->possibleActions() & Qt::MoveAction) != 0 &&
      dropAction == Qt::MoveAction) {
    QModelIndexList selIndexes = selectedIndexes();
    QModelIndex child = index;
    QModelIndex root = rootIndex();
    while (child.isValid() && child != root) {
      if (selIndexes.contains(child)) {
        return true;
      }
      child = child.parent();
    }
  }
  return false;
}

/**
 * @brief Get row, column and index where item is dropped.
 * @param event drop event
 * @param dropRow the row is returned here
 * @param dropCol the column is returned here
 * @param dropIndex the parent model index is returned here
 * @return true if drop supported and not on itself.
 */
bool PlaylistView::dropOn(
    QDropEvent* event, int* dropRow, int* dropCol, QModelIndex* dropIndex)
{
  if (event->isAccepted())
    return false;

  QModelIndex index;
  QModelIndex root = rootIndex();
  if (viewport()->rect().contains(event->pos())) {
    index = indexAt(event->pos());
    if (!index.isValid() || !visualRect(index).contains(event->pos())) {
      index = root;
    }
  }

  if (model()->supportedDropActions() & event->dropAction()) {
    int row = -1;
    int col = -1;
    if (index != root) {
      DropIndicatorPosition dropIndicatorPosition =
          position(event->pos(), visualRect(index), index);
      switch (dropIndicatorPosition) {
      case QAbstractItemView::AboveItem:
        row = index.row();
        col = index.column();
        index = index.parent();
        break;
      case QAbstractItemView::BelowItem:
        row = index.row() + 1;
        col = index.column();
        index = index.parent();
        break;
      case QAbstractItemView::OnItem:
      case QAbstractItemView::OnViewport:
        row = index.row();
        col = index.column();
        index = index.parent();
        break;
      }
    }
    *dropIndex = index;
    *dropRow = row;
    *dropCol = col;
    if (!droppingOnItself(event, index)) {
      return true;
    }
  }
  return false;
}

/**
 * Get drop indicator position.
 * @param pos drop position
 * @param rect visual rectangle of @a idx
 * @param idx parent index
 * @return drop indicator position.
 */
QAbstractItemView::DropIndicatorPosition PlaylistView::position(
    const QPoint& pos, const QRect& rect, const QModelIndex& idx) const
{
  QAbstractItemView::DropIndicatorPosition r = QAbstractItemView::OnViewport;
  const int margin = 2;
  if (pos.y() - rect.top() < margin) {
    r = QAbstractItemView::AboveItem;
  } else if (rect.bottom() - pos.y() < margin) {
    r = QAbstractItemView::BelowItem;
  } else if (rect.contains(pos, true)) {
    r = QAbstractItemView::OnItem;
  }

  if (r == QAbstractItemView::OnItem &&
      !(model()->flags(idx) & Qt::ItemIsDropEnabled)) {
    r = pos.y() < rect.center().y()
        ? QAbstractItemView::AboveItem : QAbstractItemView::BelowItem;
  }
  return r;
}

/**
 * Get sorted list of selected rows.
 * @return list of rows.
 */
QList<int> PlaylistView::getSelectedRows() const
{
  QSet<int> selRows;
  const auto idxs = selectedIndexes();
  for (const QModelIndex& idx : idxs) {
    selRows.insert(idx.row());
  }

  QList<int> result = selRows.toList();
  std::sort(result.begin(), result.end());
  return result;
}

void PlaylistView::dropEvent(QDropEvent* event)
{
  if (event->dropAction() == Qt::MoveAction ||
      event->dropAction() == Qt::CopyAction ||
      dragDropMode() == InternalMove) {
    if (event->source() == this) {
      // Internal drop.
      QModelIndex index;
      int col = -1;
      int row = -1;
      if (dropOn(event, &row, &col, &index)) {
        if (QAbstractItemModel* mdl = model()) {
          const QList<int> selRows = getSelectedRows();
          if (!selRows.isEmpty()) {
            int top = selRows.first();
            int dropRow = row;
            if (dropRow == -1) {
              dropRow = mdl->rowCount(index);
            }
            int offset = dropRow - top;
            for (int theRow : selRows) {
              int r = theRow + offset;
              if (r > mdl->rowCount(index) || r < 0) {
                r = 0;
              }
              mdl->insertRow(r, index);
            }

            const QList<int> newSelRows = getSelectedRows();
            if (!newSelRows.isEmpty()) {
              top = newSelRows.first();
              offset = dropRow - top;
              for (int theRow : newSelRows) {
                int r = theRow + offset;
                if (r > mdl->rowCount(index) || r < 0) {
                  r = 0;
                }
                for (int j = 0; j < mdl->columnCount(index); ++j) {
                  QVariant source = mdl->index(theRow, j, index)
                      .data(m_dropRole);
                  mdl->setData(mdl->index(r, j, index), source, m_dropRole);
                }
              }
              event->accept();
            }
          }
        }
      } else {
        QListView::dropEvent(event);
      }
    } else if (event->mimeData()->hasUrls()) {
      // External file drop.
      int row, col;
      QModelIndex index;
      if (dropOn(event, &row, &col, &index)) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (QAbstractItemModel* mdl = model()) {
          if (row == -1) {
            row = mdl->rowCount(index);
          }
          if (!urls.isEmpty()) {
            QListIterator<QUrl> it(urls);
            it.toBack();
            while (it.hasPrevious()) {
              const QUrl& url = it.previous();
              if (url.isLocalFile()) {
                QString path = url.toLocalFile();
                mdl->insertRow(row, index);
                QModelIndex idx = mdl->index(row, 0, index);
                mdl->setData(idx, path, m_dropRole);
                if (idx.data(m_dropRole).toString() != path) {
                  qWarning("Playlist: Failed to set path %s", qPrintable(path));
                  mdl->removeRow(row, index);
                }
              }
            }
            event->accept();
          }
        }
      }
    }
  }
}

void PlaylistView::dragEnterEvent(QDragEnterEvent* event)
{
  QListView::dragEnterEvent(event);
  if (!event->isAccepted() && event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void PlaylistView::dragMoveEvent(QDragMoveEvent* event)
{
  QListView::dragMoveEvent(event);
  if (!event->isAccepted() && event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void PlaylistView::dragLeaveEvent(QDragLeaveEvent* event)
{
  event->accept();
}

void PlaylistView::deleteCurrentRow()
{
  if (QAbstractItemModel* mdl = model()) {
    QModelIndex idx = currentIndex();
    if (idx.isValid()) {
      int row = idx.row();
      mdl->removeRow(row);
      int numRows = mdl->rowCount();
      if (row < numRows) {
        setCurrentIndex(mdl->index(row, 0));
      } else if (row > 0 && row - 1 < numRows) {
        setCurrentIndex(mdl->index(row - 1, 0));
      }
    }
  }
}

void PlaylistView::moveUpCurrentRow()
{
  swapRows(-1, 0);
}

void PlaylistView::moveDownCurrentRow()
{
  swapRows(0, 1);
}

void PlaylistView::swapRows(int offset1, int offset2)
{
  if (QAbstractItemModel* mdl = model()) {
    QModelIndex idx = currentIndex();
    if (idx.isValid()) {
      int row1 = idx.row() + offset1;
      int row2 = idx.row() + offset2;
      int numRows = mdl->rowCount();
      if (row1 >= 0 && row2 >= 0 && row1 < numRows && row2 < numRows) {
        QModelIndex idx1 = mdl->index(row1, 0);
        QModelIndex idx2 = mdl->index(row2, 0);
        QVariant val1 = idx1.data(m_dropRole);
        QVariant val2 = idx2.data(m_dropRole);
        mdl->setData(idx1, val2, m_dropRole);
        mdl->setData(idx2, val1, m_dropRole);
        if (offset1 == 0) {
          setCurrentIndex(idx2);
        } else if (offset2 == 0) {
          setCurrentIndex(idx1);
        }
      }
    }
  }
}
