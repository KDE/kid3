/**
 * \file dirproxymodel.cpp
 * Proxy for filesystem model which filters directories.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19-Mar-2011
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

#include "dirproxymodel.h"
#include <QFileSystemModel>

/**
 * Constructor.
 *
 * @param parent parent object
 */
DirProxyModel::DirProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
  setObjectName("DirProxyModel");
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
  QFileSystemModel* srcModel = qobject_cast<QFileSystemModel*>(sourceModel());
  if (srcModel) {
    return srcModel->isDir(srcModel->index(srcRow, 0, srcParent));
  }
  return false;
}
