/**
 * \file fileproxymodeliterator.h
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

#ifndef FILEPROXYMODELITERATOR_H
#define FILEPROXYMODELITERATOR_H

#include <QObject>
#include <QStack>
#include <QPersistentModelIndex>
#include "iabortable.h"

class QTimer;
class FileProxyModel;

/**
 * Iterator for FileProxyModel.
 * This iterator behaves differently than other iterators, e.g. ModelIterator.
 * The file system model is not completely loaded, subdirectories can be
 * fetched later using fetchMore(). This class fetches directories
 * continuously and waits for them to be fetched. Therefore the routine
 * doing the actual work has to be connected with a slot and will be called
 * when file nodes are available. The iteration will also be suspended after
 * some files so that other slots can be processed and the GUI remains
 * responsive. If the iteration shall stop before all files are processed,
 * abort() shall be called.
 */
class FileProxyModelIterator : public QObject, public IAbortable {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param model file proxy model
   */
  explicit FileProxyModelIterator(FileProxyModel* model);

  /**
   * Destructor.
   */
  virtual ~FileProxyModelIterator();

  /**
   * Abort operation.
   */
  virtual void abort();

  /**
   * Check if operation is aborted.
   *
   * @return true if aborted.
   */
  virtual bool isAborted() const;

  /**
   * Clear state which is reported by isAborted().
   */
  virtual void clearAborted();

  /**
   * Start iteration.
   *
   * @param rootIdx index of root element
   */
  void start(const QPersistentModelIndex& rootIdx);

  /**
   * Start iteration.
   *
   * @param indexes indexes of root directories
   */
  void start(const QList<QPersistentModelIndex>& indexes);

signals:
  /**
   * Signaled when the next file node is ready to be processed.
   *
   * @param idx file model index
   */
  void nextReady(const QPersistentModelIndex& idx);

private slots:
#if QT_VERSION >= 0x040700
  /**
   * Called when the gatherer thread has finished to load the @a path.
   *
   * @param path directory fetched due to fetchMore() call.
   */
  void onDirectoryLoaded(const QString& path);
#else
  /**
   * Check if the directory has been loaded.
   */
  void onRowsInserted();
#endif

  /**
   * Fetch next index.
   */
  void fetchNext();

private:
  QList<QPersistentModelIndex> m_rootIndexes;
  QStack<QPersistentModelIndex> m_nodes;
  FileProxyModel* m_model;
  QPersistentModelIndex m_nextIdx;
  bool m_aborted;
  QTimer* m_timeoutTimer;
};

#endif // FILEPROXYMODELITERATOR_H