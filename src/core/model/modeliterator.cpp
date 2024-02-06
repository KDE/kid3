/**
 * \file modeliterator.cpp
 * Iterator for Qt models.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26-Mar-2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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

#include "modeliterator.h"
#include <QItemSelectionModel>
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param rootIdx root of model to iterate
 */
ModelIterator::ModelIterator(const QPersistentModelIndex& rootIdx)
    : m_model(rootIdx.model())
{
  m_nodes.push(rootIdx);
  next();
}

/**
 * Check if a next item exists.
 * @return true if there is a next index
 */
bool ModelIterator::hasNext() const
{
  return m_model && m_nextIdx.isValid();
}

/**
 * Advance iterator and return next item.
 * @return next index
 */
QPersistentModelIndex ModelIterator::next()
{
  if (!m_model)
    return QPersistentModelIndex();
  QPersistentModelIndex result = m_nextIdx;
  if (!m_nodes.isEmpty()) {
    m_nextIdx = m_nodes.pop();
    if (m_nextIdx.isValid()) {
      for (int row = m_model->rowCount(m_nextIdx) - 1; row >= 0; --row) {
        m_nodes.push(m_model->index(row, 0, m_nextIdx));
      }
    }
  } else {
    m_nextIdx = QPersistentModelIndex();
  }
  return result;
}

/**
 * Get next item without moving iterator.
 * @return next index
 */
QPersistentModelIndex ModelIterator::peekNext() const
{
  if (!m_model)
    return QPersistentModelIndex();
  return m_nextIdx;
}

/**
 * Advance iterator and return data of next index.
 * @param role model item role to get
 * @return data of next index
 */
QVariant ModelIterator::nextData(int role) {
  if (!m_model)
    return QVariant();
  return m_model->data(next(), role);
}

/**
 * Get data of next item without moving iterator.
 * @param role model item role to get
 * @return data of next index
 */
QVariant ModelIterator::peekNextData(int role) const {
  if (!m_model)
    return QVariant();
  return m_model->data(m_nextIdx, role);
}


/**
 * Constructor.
 *
 * @param rootIdx root of model to iterate
 */
ModelBfsIterator::ModelBfsIterator(const QPersistentModelIndex& rootIdx)
    : m_model(rootIdx.model()), m_nextIdx(rootIdx), m_parentIdx(rootIdx), m_row(0)
{
}

/**
 * Check if a next item exists.
 * @return true if there is a next index
 */
bool ModelBfsIterator::hasNext() const
{
  return m_model && m_nextIdx.isValid();
}

/**
 * Advance iterator and return next item.
 * @return next index
 */
QPersistentModelIndex ModelBfsIterator::next()
{
  if (!m_model)
    return QPersistentModelIndex();
  QPersistentModelIndex result = m_nextIdx;
  forever {
    if (m_parentIdx.isValid() && m_row < m_model->rowCount(m_parentIdx)) {
      m_nextIdx = m_model->index(m_row, 0, m_parentIdx);
      m_nodes.enqueue(m_nextIdx);
      ++m_row;
      break;
    }
    if (!m_nodes.isEmpty()) {
      m_parentIdx = m_nodes.dequeue();
      m_row = 0;
    } else {
      m_nextIdx = QPersistentModelIndex();
      break;
    }
  }
  return result;
}

/**
 * Get next item without moving iterator.
 * @return next index
 */
QPersistentModelIndex ModelBfsIterator::peekNext() const
{
  if (!m_model)
    return QPersistentModelIndex();
  return m_nextIdx;
}

/**
 * Advance iterator and return data of next index.
 * @param role model item role to get
 * @return data of next index
 */
QVariant ModelBfsIterator::nextData(int role) {
  if (!m_model)
    return QVariant();
  return m_model->data(next(), role);
}

/**
 * Get data of next item without moving iterator.
 * @param role model item role to get
 * @return data of next index
 */
QVariant ModelBfsIterator::peekNextData(int role) const {
  if (!m_model)
    return QVariant();
  return m_model->data(m_nextIdx, role);
}


/**
 * Destructor.
 */
AbstractTaggedFileIterator::~AbstractTaggedFileIterator()
{
}


/**
 * Constructor.
 *
 * @param rootIdx root of model to iterate
 */
TaggedFileIterator::TaggedFileIterator(const QPersistentModelIndex& rootIdx)
    : m_it(rootIdx), m_nextFile(nullptr)
{
  next();
}

/**
 * Advance iterator and return next item.
 * @return next file
 */
TaggedFile* TaggedFileIterator::next()
{
  TaggedFile* result = m_nextFile;
  m_nextFile = nullptr;
  while (m_it.hasNext()) {
    if (QPersistentModelIndex index = m_it.next();
        (m_nextFile = FileProxyModel::getTaggedFileOfIndex(index)) != nullptr)
      break;
  }
  return result;
}

/**
 * Try to close the file handles.
 *
 * @param index root of model to iterate
 */
void TaggedFileIterator::closeFileHandles(const QPersistentModelIndex& index)
{
  TaggedFileIterator it(index);
  while (it.hasNext()) {
    it.next()->closeFileHandle();
  }
}


/**
 * Constructor.
 *
 * @param rootIdx root of model to iterate
 * @param selectModel selection model
 * @param allIfNoneSelected treat all files as selected when nothing is
 * selected
 */
SelectedTaggedFileIterator::SelectedTaggedFileIterator(
    const QPersistentModelIndex& rootIdx,
    const QItemSelectionModel* selectModel,
    bool allIfNoneSelected)
    : m_it(rootIdx), m_nextFile(nullptr), m_selectModel(selectModel),
      m_allSelected(!m_selectModel ||
                    (allIfNoneSelected && !m_selectModel->hasSelection()))
{
  next();
}

/**
 * Advance iterator and return next item.
 * @return next file
 */
TaggedFile* SelectedTaggedFileIterator::next()
{
  TaggedFile* result = m_nextFile;
  m_nextFile = nullptr;
  while (m_it.hasNext()) {
    if (QPersistentModelIndex index = m_it.next();
        (m_nextFile = FileProxyModel::getTaggedFileOfIndex(index)) != nullptr &&
        (m_allSelected || m_selectModel->isSelected(index)))
      break;
    m_nextFile = nullptr;
  }
  return result;
}

/**
 * Check if nothing is selected.
 * @return true if nothing is selected.
 */
bool SelectedTaggedFileIterator::hasNoSelection() const
{
  return m_selectModel && !m_selectModel->hasSelection();
}

/**
 * Constructor.
 *
 * @param index of the directory or a file in it
 */
TaggedFileOfDirectoryIterator::TaggedFileOfDirectoryIterator(
    const QPersistentModelIndex& index)
    : m_row(0), m_model(index.model()),
      m_parentIdx(m_model && m_model->hasChildren(index)
                  ? index
                  : QPersistentModelIndex(index.parent())),
      m_nextFile(nullptr)
{
  next();
}

/**
 * Check if a next item exists.
 * @return true if there is a next file
 */
bool TaggedFileOfDirectoryIterator::hasNext() const
{
  return m_model && m_nextFile != nullptr;
}

/**
 * Advance iterator and return next item.
 * @return next file
 */
TaggedFile* TaggedFileOfDirectoryIterator::next() {
  if (!m_model)
    return nullptr;
  TaggedFile* result = m_nextFile;
  m_nextFile = nullptr;
  while (m_row < m_model->rowCount(m_parentIdx)) {
    if (QModelIndex index = m_model->index(m_row++, 0, m_parentIdx);
        (m_nextFile = FileProxyModel::getTaggedFileOfIndex(index)) != nullptr)
      break;
  }
  return result;
}

/**
 * Get next item without moving iterator.
 * @return next file
 */
TaggedFile* TaggedFileOfDirectoryIterator::peekNext() const
{
  if (!m_model)
    return nullptr;
  return m_nextFile;
}

/**
 * Get first tagged file in directory.
 * @param index of the directory or a file in it
 * @return first tagged file in directory, 0 if none.
 */
TaggedFile* TaggedFileOfDirectoryIterator::first(
    const QPersistentModelIndex& index)
{
  if (TaggedFileOfDirectoryIterator it(index); it.hasNext())
    return it.peekNext();
  return nullptr;
}


/**
 * Constructor.
 *
 * @param index of the directory or a file in it
 * @param selectModel selection model
 * @param allIfNoneSelected treat all files as selected when nothing is
 * selected
 */
SelectedTaggedFileOfDirectoryIterator::SelectedTaggedFileOfDirectoryIterator(
  const QPersistentModelIndex& index,
  const QItemSelectionModel* selectModel,
  bool allIfNoneSelected)
  : m_row(0), m_model(index.model()),
    m_parentIdx(m_model && m_model->hasChildren(index)
                ? index : QPersistentModelIndex(index.parent())),
    m_nextFile(nullptr),
    m_selectModel(selectModel),
    m_allSelected(!m_selectModel ||
                  (allIfNoneSelected && !m_selectModel->hasSelection()))
{
  next();
}

/**
 * Check if a next item exists.
 * @return true if there is a next file
 */
bool SelectedTaggedFileOfDirectoryIterator::hasNext() const
{
  return m_model && m_nextFile != nullptr;
}

/**
 * Advance iterator and return next item.
 * @return next file
 */
TaggedFile* SelectedTaggedFileOfDirectoryIterator::next() {
  if (!m_model)
    return nullptr;
  TaggedFile* result = m_nextFile;
  m_nextFile = nullptr;
  while (m_row < m_model->rowCount(m_parentIdx)) {
    if (QModelIndex index = m_model->index(m_row++, 0, m_parentIdx);
        (m_nextFile = FileProxyModel::getTaggedFileOfIndex(index)) != nullptr &&
        (m_allSelected || m_selectModel->isSelected(index)))
      break;
    m_nextFile = nullptr;
  }
  return result;
}

/**
 * Get next item without moving iterator.
 * @return next file
 */
TaggedFile* SelectedTaggedFileOfDirectoryIterator::peekNext() const
{
  if (!m_model)
    return nullptr;
  return m_nextFile;
}


/**
 * Constructor.
 *
 * @param selectModel selection model
 */
TaggedFileOfSelectedDirectoriesIterator::TaggedFileOfSelectedDirectoriesIterator(
  const QItemSelectionModel* selectModel) : m_model(nullptr), m_dirIdx(0), m_row(0),
  m_nextFile(nullptr)
{
  if (selectModel &&
      (m_model = qobject_cast<const FileProxyModel*>(selectModel->model()))
      != nullptr) {
    const auto indexes = selectModel->selectedRows();
    for (const QModelIndex& index : indexes) {
      if (m_model->isDir(index)) {
        m_dirIndexes.append(getIndexesOfDirWithSubDirs(index));
      }
    }
  }
  next();
}

/**
 * Get indexes of directory and recursively all subdirectories.
 * @param dirIndex index of directory
 * @return list with dirIndex and its subdirectories.
 */
QList<QPersistentModelIndex>
TaggedFileOfSelectedDirectoriesIterator::getIndexesOfDirWithSubDirs(
  const QModelIndex& dirIndex) const
{
  QList<QPersistentModelIndex> dirs;
  dirs.append(dirIndex);
  for (int dirsPos = 0; dirsPos < dirs.size(); ++dirsPos) {
    QPersistentModelIndex parentIndex(dirs.at(dirsPos));
    for (int row = 0; row < m_model->rowCount(parentIndex); ++row) {
      if (QModelIndex index(m_model->index(row, 0, parentIndex));
          m_model->isDir(index)) {
        dirs.append(index);
      }
    }
  }
  return dirs;
}

/**
 * Check if a next item exists.
 * @return true if there is a next file
 */
bool TaggedFileOfSelectedDirectoriesIterator::hasNext() const
{
  return m_model && m_nextFile != nullptr;
}

/**
 * Advance iterator and return next item.
 * @return next file
 */
TaggedFile* TaggedFileOfSelectedDirectoriesIterator::next()
{
  if (!m_model)
    return nullptr;
  TaggedFile* result = m_nextFile;
  m_nextFile = nullptr;
  while (!m_nextFile) {
    if (m_dirIdx >= m_dirIndexes.size())
      break;
    QPersistentModelIndex parentIdx(m_dirIndexes.at(m_dirIdx));
    while (m_row < m_model->rowCount(parentIdx)) {
      if (QModelIndex index = m_model->index(m_row++, 0, parentIdx);
          (m_nextFile = FileProxyModel::getTaggedFileOfIndex(index)) != nullptr)
        break;
    }
    if (m_row >= m_model->rowCount(parentIdx)) {
      ++m_dirIdx;
      m_row = 0;
    }
  }
  return result;
}

/**
 * Get next item without moving iterator.
 * @return next file
 */
TaggedFile* TaggedFileOfSelectedDirectoriesIterator::peekNext() const
{
  if (!m_model)
    return nullptr;
  return m_nextFile;
}
