/**
 * \file modeliterator.h
 * Iterator for Qt models.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26-Mar-2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef MODELITERATOR_H
#define MODELITERATOR_H

#include <QModelIndex>
#include <QStack>
#include <QQueue>
#include "kid3api.h"

class QItemSelectionModel;
class TaggedFile;
class FileProxyModel;

/**
 * Generic Java-style iterator for Qt models.
 * Iterates using preorder traversal. Supports only one column.
 *
 * Typical usage:
 * @code
 * // get rootIndex...
 * ModelIterator it(rootIndex);
 * while (it.hasNext()) {
 *   TaggedFile* taggedFile;
 *   if (ModelIterator::getTaggedFileOfIndex(it.peekNext(), &taggedFile)
 *       && taggedFile) {
 *     // do something with taggedFile...
 *   }
 *   it.next();
 * }
 * @endcode
 */
class ModelIterator {
public:
  /**
   * Constructor.
   *
   * @param rootIdx root of model to iterate
   */
  explicit ModelIterator(const QModelIndex& rootIdx);

  /**
   * Check if a next item exists.
   * @return true if there is a next index
   */
  bool hasNext() const;

  /**
   * Advance iterator and return next item.
   * @return next index
   */
  QModelIndex next();

  /**
   * Get next item without moving iterator.
   * @return next index
   */
  QModelIndex peekNext() const;

  /**
   * Advance iterator and return data of next index.
   * @param role model item role to get
   * @return data of next index
   */
  QVariant nextData(int role);

  /**
   * Get data of next item without moving iterator.
   * @param role model item role to get
   * @return data of next index
   */
  QVariant peekNextData(int role) const;

private:
  QStack<QModelIndex> m_nodes;
  const QAbstractItemModel* m_model;
  QModelIndex m_nextIdx;
};

/**
 * Generic Java-style iterator for Qt models.
 * Iterates using breadth-first-search. Supports only one column.
 */
class ModelBfsIterator {
public:
  /**
   * Constructor.
   *
   * @param rootIdx root of model to iterate
   */
  explicit ModelBfsIterator(const QModelIndex& rootIdx);

  /**
   * Check if a next item exists.
   * @return true if there is a next index
   */
  bool hasNext() const;

  /**
   * Advance iterator and return next item.
   * @return next index
   */
  QModelIndex next();

  /**
   * Get next item without moving iterator.
   * @return next index
   */
  QModelIndex peekNext() const;

  /**
   * Advance iterator and return data of next index.
   * @param role model item role to get
   * @return data of next index
   */
  QVariant nextData(int role);

  /**
   * Get data of next item without moving iterator.
   * @param role model item role to get
   * @return data of next index
   */
  QVariant peekNextData(int role) const;

private:
  QQueue<QModelIndex> m_nodes;
  const QAbstractItemModel* m_model;
  QModelIndex m_nextIdx;
  QModelIndex m_parentIdx;
  int m_row;
};

/**
 * Abstract base class for tagged file iterators.
 */
class KID3_CORE_EXPORT AbstractTaggedFileIterator {
public:
  /**
   * Destructor.
   */
  virtual ~AbstractTaggedFileIterator() = 0;

  /**
   * Check if a next item exists.
   * @return true if there is a next file
   */
  virtual bool hasNext() const = 0;

  /**
   * Advance iterator and return next item.
   * @return next file
   */
  virtual TaggedFile* next() = 0;

  /**
   * Get next item without moving iterator.
   * @return next file
   */
  virtual TaggedFile* peekNext() const = 0;
};

/**
 * Iterator to iterate over model indexes with tagged files.
 * All TaggedFiles returned while hasNext() is true are not null.
 *
 * Typical usage:
 * @code
 * // get rootIndex...
 * TaggedFileIterator it(rootIndex);
 * while (it.hasNext()) {
 *   TaggedFile* taggedFile = it.next();
 *   // do something with taggedFile...
 * }
 * @endcode
 */
class KID3_CORE_EXPORT TaggedFileIterator : public AbstractTaggedFileIterator {
public:
  /**
   * Constructor.
   *
   * @param rootIdx root of model to iterate
   */
  explicit TaggedFileIterator(const QModelIndex& rootIdx);

  /**
   * Check if a next item exists.
   * @return true if there is a next file
   */
  virtual bool hasNext() const { return m_nextFile != 0; }

  /**
   * Advance iterator and return next item.
   * @return next file
   */
  virtual TaggedFile* next();

  /**
   * Get next item without moving iterator.
   * @return next file
   */
  virtual TaggedFile* peekNext() const { return m_nextFile; }

private:
  ModelIterator m_it;
  TaggedFile* m_nextFile;
};

/**
 * Iterator to iterate over model indexes with selected tagged files.
 * All TaggedFiles returned while hasNext() is true are not null.
 *
 * Typical usage:
 * @code
 * // get rootIndex, selectionModel...
 * SelectedTaggedFileIterator it(rootIndex, selectionModel, false);
 * while (it.hasNext()) {
 *   TaggedFile* taggedFile = it.next();
 *   // do something with taggedFile...
 * }
 * @endcode
 */
class SelectedTaggedFileIterator : public AbstractTaggedFileIterator {
public:
  /**
   * Constructor.
   *
   * @param rootIdx root of model to iterate
   * @param selectModel selection model
   * @param allIfNoneSelected treat all files as selected when nothing is
   * selected
   */
  SelectedTaggedFileIterator(const QModelIndex& rootIdx,
                             const QItemSelectionModel* selectModel,
                             bool allIfNoneSelected);

  /**
   * Check if a next item exists.
   * @return true if there is a next file
   */
  virtual bool hasNext() const { return m_nextFile != 0; }

  /**
   * Advance iterator and return next item.
   * @return next file
   */
  virtual TaggedFile* next();

  /**
   * Get next item without moving iterator.
   * @return next file
   */
  virtual TaggedFile* peekNext() const { return m_nextFile; }

  /**
   * Check if nothing is selected.
   * @return true if nothing is selected.
   */
  bool hasNoSelection() const;

private:
  ModelIterator m_it;
  TaggedFile* m_nextFile;
  const QItemSelectionModel* m_selectModel;
  bool m_allSelected;
};

/**
 * Iterator to iterate tagged files from a single directory.
 * All TaggedFiles returned while hasNext() is true are not null.
 *
 * Typical usage:
 * @code
 * // get currentIndex...
 * TaggedFileOfDirectoryIterator it(currentIndex);
 * while (it.hasNext()) {
 *   TaggedFile* taggedFile = it.next();
 *   // do something with taggedFile...
 * }
 * @endcode
 */
class KID3_CORE_EXPORT TaggedFileOfDirectoryIterator : public AbstractTaggedFileIterator {
public:
  /**
   * Constructor.
   *
   * @param index of the directory or a file in it
   */
  explicit TaggedFileOfDirectoryIterator(const QModelIndex& index);

  /**
   * Check if a next item exists.
   * @return true if there is a next file
   */
  virtual bool hasNext() const;

  /**
   * Advance iterator and return next item.
   * @return next file
   */
  virtual TaggedFile* next();

  /**
   * Get next item without moving iterator.
   * @return next file
   */
  virtual TaggedFile* peekNext() const;

  /**
   * Get first tagged file in directory.
   * @param index of the directory or a file in it
   * @return first tagged file in directory, 0 if none.
   */
  static TaggedFile* first(const QModelIndex& index);

private:
  int m_row;
  const QAbstractItemModel* m_model;
  QModelIndex m_parentIdx;
  TaggedFile* m_nextFile;
};

/**
 * Iterator to iterate selected tagged files from a single directory.
 * All TaggedFiles returned while hasNext() is true are not null.
 *
 * Typical usage:
 * @code
 * // get currentIndex, selectionModel...
 * SelectedTaggedFileOfDirectoryIterator it(currentIndex, selectionModel, false);
 * while (it.hasNext()) {
 *   TaggedFile* taggedFile = it.next();
 *   // do something with taggedFile...
 * }
 * @endcode
 */
class SelectedTaggedFileOfDirectoryIterator :
    public AbstractTaggedFileIterator {
public:
  /**
   * Constructor.
   *
   * @param index of the directory or a file in it
   * @param selectModel selection model
   * @param allIfNoneSelected treat all files as selected when nothing is
   * selected
   */
  SelectedTaggedFileOfDirectoryIterator(
    const QModelIndex& index,
    const QItemSelectionModel* selectModel,
    bool allIfNoneSelected);

  /**
   * Check if a next item exists.
   * @return true if there is a next file
   */
  virtual bool hasNext() const;

  /**
   * Advance iterator and return next item.
   * @return next file
   */
  virtual TaggedFile* next();

  /**
   * Get next item without moving iterator.
   * @return next file
   */
  virtual TaggedFile* peekNext() const;

private:
  int m_row;
  const QAbstractItemModel* m_model;
  QModelIndex m_parentIdx;
  TaggedFile* m_nextFile;
  const QItemSelectionModel* m_selectModel;
  bool m_allSelected;
};

/**
 * Iterator to iterate all tagged files from selected directories.
 * All TaggedFiles returned while hasNext() is true are not null.
 *
 * Typical usage:
 * @code
 * // get selectionModel...
 * TaggedFileOfSelectedDirectoriesIterator it(selectionModel);
 * while (it.hasNext()) {
 *   TaggedFile* taggedFile = it.next();
 *   // do something with taggedFile...
 * }
 * @endcode
 */
class TaggedFileOfSelectedDirectoriesIterator :
    public AbstractTaggedFileIterator {
public:
  /**
   * Constructor.
   *
   * @param selectModel selection model
   */
  explicit TaggedFileOfSelectedDirectoriesIterator(
    const QItemSelectionModel* selectModel);

  /**
   * Check if a next item exists.
   * @return true if there is a next file
   */
  virtual bool hasNext() const;

  /**
   * Advance iterator and return next item.
   * @return next file
   */
  virtual TaggedFile* next();

  /**
   * Get next item without moving iterator.
   * @return next file
   */
  virtual TaggedFile* peekNext() const;

private:
  /**
   * Get indexes of directory and recursively all subdirectories.
   * @param dirIndex index of directory
   * @return list with dirIndex and its subdirectories.
   */
  QModelIndexList getIndexesOfDirWithSubDirs(const QModelIndex& dirIndex);

  const FileProxyModel* m_model;
  QModelIndexList m_dirIndexes;
  int m_dirIdx;
  int m_row;
  TaggedFile* m_nextFile;
};

#endif // MODELITERATOR_H
