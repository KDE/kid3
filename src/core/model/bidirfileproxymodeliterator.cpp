/**
 * \file bidirfileproxymodeliterator.cpp
 * Birdirectional iterator for FileProxyModel.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21-Feb-2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#include "bidirfileproxymodeliterator.h"
#include <QTimer>
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param model file proxy model
 * @param parent parent object
 */
BiDirFileProxyModelIterator::BiDirFileProxyModelIterator(FileProxyModel* model,
                                                         QObject *parent)
  : QObject(parent), m_model(model), m_backwards(false),
    m_aborted(false), m_suspended(false)
{
}

/**
 * Abort operation.
 */
void BiDirFileProxyModelIterator::abort()
{
  m_aborted = true;
}

/**
 * Check if operation is aborted.
 *
 * @return true if aborted.
 */
bool BiDirFileProxyModelIterator::isAborted() const
{
  return m_aborted;
}

/**
 * Clear state which is reported by isAborted().
 */
void BiDirFileProxyModelIterator::clearAborted()
{
  m_aborted = false;
}

/**
 * Set root index of file proxy model.
 *
 * @param rootIdx index of root element
 */
void BiDirFileProxyModelIterator::setRootIndex(
    const QPersistentModelIndex& rootIdx)
{
  m_rootIndex = rootIdx;
}

/**
 * Set index of current file.
 *
 * @param index index of current file
 */
void BiDirFileProxyModelIterator::setCurrentIndex(
    const QPersistentModelIndex& index)
{
  m_currentIndex = index;
}

/**
 * Start iteration.
 */
void BiDirFileProxyModelIterator::start()
{
  m_aborted = false;
  m_suspended = false;
  if (m_currentIndex.isValid()) {
    emit nextReady(m_currentIndex);
  }
  fetchNext();
}

/**
 * Fetch next index.
 */
void BiDirFileProxyModelIterator::fetchNext()
{
  int count = 0;
  while (!m_aborted) {
    if (m_suspended) {
      return;
    }
    QModelIndex next;
    if (!m_backwards) {
      if (!m_currentIndex.isValid()) {
        m_currentIndex = m_rootIndex;
      }
      if (m_model->rowCount(m_currentIndex) > 0) {
        // to first child
        next = m_model->index(0, 0, m_currentIndex);
      } else {
        QModelIndex parent = m_currentIndex;
        while (!next.isValid() && parent.isValid()) {
          // to next sibling or next sibling of parent
          int row = parent.row();
          if (parent == m_rootIndex) {
            // do not move beyond root index
            break;
          }
          parent = parent.parent();
          if (row + 1 < m_model->rowCount(parent)) {
            // to next sibling
            next = m_model->index(row + 1, 0, parent);
          }
        }
      }
    } else {
      if (m_currentIndex.isValid()) {
        if (int row = m_currentIndex.row() - 1; row >= 0) {
          // to last leafnode of previous sibling
          next = m_currentIndex.sibling(row, 0);
          row = m_model->rowCount(next) - 1;
          while (row >= 0) {
            next = m_model->index(row, 0, next);
            row = m_model->rowCount(next) - 1;
          }
        } else {
          // to parent
          next = m_currentIndex.parent();
        }
        if (next == m_rootIndex)
          next = QPersistentModelIndex();
      } else {
        // to last node
        int row;
        QModelIndex last = m_rootIndex;
        while ((row = m_model->rowCount(last)) > 0 &&
               (last = m_model->index(row - 1, 0, last)).isValid()) {
          next = last;
        }
      }
    }
    if (next.isValid()) {
      if (m_model->isDir(next) && m_model->canFetchMore(next)) {
        connect(m_model, &FileProxyModel::sortingFinished,
                this, &BiDirFileProxyModelIterator::onDirectoryLoaded);
        m_model->fetchMore(next);
        return;
      }
      if (++count >= 10) {
        // Avoid spinning too long to keep the GUI responsive.
        QTimer::singleShot(0, this, &BiDirFileProxyModelIterator::fetchNext);
        return;
      }
      m_currentIndex = next;
      emit nextReady(m_currentIndex);
    } else {
      break;
    }
  }
  m_currentIndex = QPersistentModelIndex();
  emit nextReady(m_currentIndex);
}

/**
 * Called when the gatherer thread has finished to load.
 */
void BiDirFileProxyModelIterator::onDirectoryLoaded()
{
  disconnect(m_model, &FileProxyModel::sortingFinished,
             this, &BiDirFileProxyModelIterator::onDirectoryLoaded);
  fetchNext();
}

/**
 * Suspend iteration.
 * The iteration can be continued with resume().
 */
void BiDirFileProxyModelIterator::suspend()
{
  m_suspended = true;
}

/**
 * Resume iteration which has been suspended with suspend().
 */
void BiDirFileProxyModelIterator::resume()
{
  m_suspended = false;
  fetchNext();
}
