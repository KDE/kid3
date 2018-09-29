/**
 * \file dirproxymodel.cpp
 * Proxy for filesystem model which filters directories.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19-Mar-2011
 *
 * Copyright (C) 2011-2014  Urs Fleisch
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

#include "dirproxymodel.h"
#include <QFileSystemModel>
#include <QDateTime>

/**
 * Constructor.
 *
 * @param parent parent object
 */
DirProxyModel::DirProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
  setObjectName(QLatin1String("DirProxyModel"));
}

/**
 * Check if row should be included in model.
 *
 * @param srcRow source row
 * @param srcParent source parent
 *
 * @return true to include row.
 */
bool DirProxyModel::filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const
{
  auto srcModel = qobject_cast<QFileSystemModel*>(sourceModel());
  if (srcModel) {
    return srcModel->isDir(srcModel->index(srcRow, 0, srcParent));
  }
  return false;
}

/**
 * Sort comparison function.
 * @param left index of left item in source model
 * @param right index of right item in source model
 * @return true if left is less than right.
 */
bool DirProxyModel::lessThan(const QModelIndex& left,
                             const QModelIndex& right) const
{
  // "." and ".." shall be in the first and second row.
  bool orderIsAscending = sortOrder() == Qt::AscendingOrder;
  QString leftName = left.sibling(left.row(), 0).data().toString();
  if (leftName == QLatin1String(".")) {
    return orderIsAscending;
  }
  QString rightName = right.sibling(right.row(), 0).data().toString();
  if (rightName == QLatin1String(".")) {
    return !orderIsAscending;
  }
  if (leftName == QLatin1String("..")) {
    return orderIsAscending;
  }
  if (rightName == QLatin1String("..")) {
    return !orderIsAscending;
  }

  // The data() in QFileSystemModel are String QVariants, therefore
  // QSortFilterProxyModel::lessThan() is of no use here, custom sorting
  // has to be used.
  Q_ASSERT_X(sourceModel()->metaObject() == &QFileSystemModel::staticMetaObject,
             "lessThan", "source model must be QFileSystemModel");
  auto fsModel = static_cast<QFileSystemModel*>(sourceModel());
  switch (sortColumn()) {
  case 0:
    return left.data().toString().compare(right.data().toString()) < 0;
  case 1:
    return fsModel->size(left) < fsModel->size(right);
  case 2:
    return fsModel->type(left) < fsModel->type(right);
  case 3:
    return fsModel->lastModified(left) < fsModel->lastModified(right);
  }
  qWarning("DirProxyModel: Invalid sort column %d",
           sortColumn());
  return QSortFilterProxyModel::lessThan(left, right);
}

/**
 * Reset the model.
 */
void DirProxyModel::resetModel()
{
  beginResetModel();
  endResetModel();
}
