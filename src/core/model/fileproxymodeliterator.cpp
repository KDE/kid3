/**
 * \file fileproxymodeliterator.cpp
 * Iterator for FileProxyModel.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03-Feb-2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "fileproxymodeliterator.h"
#include <QTimer>
#include "fileproxymodel.h"

/**
 * GreaterThan functor to sort modex indexes by string data.
 */
class PersistentModelIndexGreaterThan {
public:
  /**
   * Greater than call operator.
   * @param lhs left hand side
   * @param rhs right hand side
   * @return true if string data of @a lhs is greater than string data of
   * @a rhs.
   */
  inline bool operator()(const QPersistentModelIndex& lhs,
                         const QPersistentModelIndex& rhs) const {
    return lhs.data().toString().compare(rhs.data().toString()) > 0;
  }
};

/**
 * Constructor.
 *
 * @param model file proxy model
 */
FileProxyModelIterator::FileProxyModelIterator(FileProxyModel* model) :
  QObject(model), m_model(model), m_numDone(0), m_aborted(false)
{
}

/**
 * Destructor.
 */
FileProxyModelIterator::~FileProxyModelIterator()
{
}

/**
 * Abort operation.
 */
void FileProxyModelIterator::abort()
{
  m_aborted = true;
}

/**
 * Check if operation is aborted.
 *
 * @return true if aborted.
 */
bool FileProxyModelIterator::isAborted() const
{
  return m_aborted;
}

/**
 * Clear state which is reported by isAborted().
 */
void FileProxyModelIterator::clearAborted()
{
  m_aborted = false;
}

/**
 * Start iteration.
 *
 * @param rootIdx index of root element
 */
void FileProxyModelIterator::start(const QPersistentModelIndex& rootIdx)
{
  m_nodes.clear();
  m_rootIndexes.clear();
  m_rootIndexes.append(rootIdx);
  m_numDone = 0;
  m_aborted = false;
  fetchNext();
}

/**
 * Start iteration.
 *
 * @param indexes indexes of root directories
 */
void FileProxyModelIterator::start(const QList<QPersistentModelIndex>& indexes)
{
  m_nodes.clear();
  m_rootIndexes = indexes;
  m_numDone = 0;
  m_aborted = false;
  fetchNext();
}

/**
 * Fetch next index.
 */
void FileProxyModelIterator::fetchNext()
{
  int count = 0;
  while (!m_aborted) {
    if (m_nodes.isEmpty()) {
      if (m_rootIndexes.isEmpty()) {
        break;
      } else {
        m_nodes.push(m_rootIndexes.takeFirst());
      }
    }
    m_nextIdx = m_nodes.top();
    if (m_nextIdx.isValid()) {
      if (m_model->isDir(m_nextIdx) && m_model->canFetchMore(m_nextIdx)) {
        connect(m_model, SIGNAL(sortingFinished()),
                this, SLOT(onDirectoryLoaded()));
        m_model->fetchMore(m_nextIdx);
        return;
      }
      if (++count >= 10) {
        // Avoid spinning too long to keep the GUI responsive.
        QTimer::singleShot(0, this, SLOT(fetchNext()));
        return;
      }
      m_nodes.pop();
      ++m_numDone;
      QStack<QPersistentModelIndex> childNodes;
      for (int row = m_model->rowCount(m_nextIdx) - 1; row >= 0; --row) {
        childNodes.push(m_model->index(row, 0, m_nextIdx));
      }
      qStableSort(childNodes.begin(), childNodes.end(),
                  PersistentModelIndexGreaterThan());
      m_nodes += childNodes;
      emit nextReady(m_nextIdx);
    }
  }
  m_nodes.clear();
  m_rootIndexes.clear();
  m_nextIdx = QPersistentModelIndex();
  emit nextReady(m_nextIdx);
}

/**
 * Called when the gatherer thread has finished to load.
 */
void FileProxyModelIterator::onDirectoryLoaded()
{
  disconnect(m_model, SIGNAL(sortingFinished()),
             this, SLOT(onDirectoryLoaded()));
  fetchNext();
}
