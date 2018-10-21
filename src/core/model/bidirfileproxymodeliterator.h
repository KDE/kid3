/**
 * \file bidirfileproxymodeliterator.h
 * Birdirectional iterator for FileProxyModel.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21-Feb-2014
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

#include <QObject>
#include <QStack>
#include <QPersistentModelIndex>
#include "iabortable.h"
#include "kid3api.h"

class FileProxyModel;

/**
 * Iterator for FileProxyModel.
 * This iterator is like FileProxyModelIterator, but it can traverse the
 * FileProxyModel in both directions and the iteration can be suspended and
 * resumed.
 */
class KID3_CORE_EXPORT BiDirFileProxyModelIterator
    : public QObject, public IAbortable {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param model file proxy model
   * @param parent parent object
   */
  explicit BiDirFileProxyModelIterator(FileProxyModel* model,
                                       QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~BiDirFileProxyModelIterator() override = default;

  /**
   * Abort operation.
   */
  virtual void abort() override;

  /**
   * Check if operation is aborted.
   *
   * @return true if aborted.
   */
  virtual bool isAborted() const override;

  /**
   * Clear state which is reported by isAborted().
   */
  virtual void clearAborted() override;

  /**
   * Set root index of file proxy model.
   *
   * @param rootIdx index of root element
   */
  void setRootIndex(const QPersistentModelIndex& rootIdx);

  /**
   * Set index of current file.
   *
   * @param index index of current file
   */
  void setCurrentIndex(const QPersistentModelIndex& index);

  /**
   * Set direction of iteration.
   * @param backwards true to iterate backwards, false (default) to iterate
   * forwards
   */
  void setDirectionBackwards(bool backwards) {
    m_backwards = backwards;
  }

  /**
   * Start iteration.
   */
  void start();

  /**
   * Suspend iteration.
   * The iteration can be continued with resume().
   */
  void suspend();

  /**
   * Resume iteration which has been suspended with suspend().
   */
  void resume();

signals:
  /**
   * Signaled when the next file node is ready to be processed.
   *
   * @param idx file model index
   */
  void nextReady(const QPersistentModelIndex& idx);

private slots:
  /**
   * Called when the gatherer thread has finished to load.
   */
  void onDirectoryLoaded();

  /**
   * Fetch next index.
   */
  void fetchNext();

private:
  FileProxyModel* m_model;
  QPersistentModelIndex m_rootIndex;
  QPersistentModelIndex m_currentIndex;
  bool m_backwards;
  bool m_aborted;
  bool m_suspended;
};
