/**
 * \file dirlist.cpp
 * List of directories to operate on.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Jul 2005
 *
 * Copyright (C) 2005-2011  Urs Fleisch
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

#include "dirlist.h"
#include <QAbstractProxyModel>

/**
 * Constructor.
 * @param parent parent widget
 */
DirList::DirList(QWidget* parent) :
  QListView(parent)
{}

/**
 * Destructor.
 */
DirList::~DirList() {}

/**
 * Fill the dirlist with the directories found in a directory.
 *
 * @param index index of path in filesystem model
 * @return false if name is not directory path, else true.
 */
bool DirList::readDir(const QModelIndex& index) {
  QAbstractProxyModel* proxyModel = qobject_cast<QAbstractProxyModel*>(model());
  QModelIndex rootIndex = proxyModel ? proxyModel->mapFromSource(index) : index;
  if (rootIndex.isValid()) {
    setRootIndex(rootIndex);
    if (m_entryToSelect.isValid()) {
      setCurrentIndex(m_entryToSelect);
    }
    return true;
  }
  return false;
}
