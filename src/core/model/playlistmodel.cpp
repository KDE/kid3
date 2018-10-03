/**
 * \file playlistmodel.cpp
 * Model containing files in playlist.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Aug 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

#include "playlistmodel.h"
#include <QFileInfo>
#include <QFileSystemModel>
#include "fileproxymodel.h"
#include "playlistcreator.h"
#include "fileconfig.h"

PlaylistModel::PlaylistModel(FileProxyModel* fsModel, QObject* parent)
  : QAbstractProxyModel(parent),
    m_fsModel(fsModel), m_modified(false)
{
  setObjectName(QLatin1String("PlaylistModel"));
  setSourceModel(m_fsModel);
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

  return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled |
      Qt::ItemIsDragEnabled;
}

bool PlaylistModel::setData(const QModelIndex& index,
                            const QVariant& value, int role)
{
  if (role == QFileSystemModel::FilePathRole &&
      index.isValid() &&
      index.row() >= 0 && index.row() < m_items.size() &&
      index.column() == 0) {
    QModelIndex idx = m_fsModel->index(value.toString());
    if (idx.isValid()) {
      QPersistentModelIndex& itemIdx = m_items[index.row()];
      if (itemIdx != idx) {
        itemIdx = idx;
        emit dataChanged(index, index);
        setModified(true);
        return true;
      }
    }
  }
  return false;
}

int PlaylistModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_items.size();
}

int PlaylistModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 1;
}

bool PlaylistModel::insertRows(int row, int count,
                               const QModelIndex& parent)
{
  if (count <= 0 || row < 0 || row > rowCount(parent))
    return false;
  beginInsertRows(parent, row, row + count - 1);
  for (int i = 0; i < count; ++i) {
    m_items.insert(row, QPersistentModelIndex());
  }
  endInsertRows();
  setModified(true);
  return true;
}

bool PlaylistModel::removeRows(int row, int count,
                               const QModelIndex& parent)
{
  if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
    return false;
  beginRemoveRows(parent, row, row + count - 1);
  for (int i = 0; i < count; ++i) {
    m_items.removeAt(row);
  }
  endRemoveRows();
  setModified(true);
  return true;
}

QModelIndex PlaylistModel::index(int row, int column,
                                 const QModelIndex& parent) const
{
  QModelIndex result = !parent.isValid() && row >= 0 && row < m_items.size() &&
      column == 0
      ? createIndex(row, column)
      : QModelIndex();
  return result;
}

QModelIndex PlaylistModel::parent(const QModelIndex& child) const
{
  Q_UNUSED(child)
  return QModelIndex();
}

QModelIndex PlaylistModel::mapToSource(const QModelIndex& proxyIndex) const
{
  QModelIndex result;
  if (!proxyIndex.parent().isValid() && proxyIndex.row() >= 0 &&
      proxyIndex.row() < m_items.size() && proxyIndex.column() == 0) {
    result = m_items.at(proxyIndex.row());
  }
  return result;
}

QModelIndex PlaylistModel::mapFromSource(const QModelIndex& sourceIndex) const
{
  QModelIndex result;
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items.at(i) == sourceIndex) {
      result = index(i, sourceIndex.column());
      break;
    }
  }
  return result;
}

Qt::DropActions PlaylistModel::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}

void PlaylistModel::setPlaylistFile(const QString& path)
{
  if (m_playlistFilePath == path)
    return;

  if (path.isEmpty()) {
    m_playlistFilePath.clear();
    m_playlistFileName.clear();
    beginResetModel();
    m_items.clear();
    endResetModel();
    setModified(false);
    return;
  }

  m_playlistConfig = PlaylistConfig::instance();
  PlaylistCreator creator(QString(), m_playlistConfig);
  QStringList filePaths;
  PlaylistConfig::PlaylistFormat format;
  bool useFullPath;
  bool writeInfo;

  QFileInfo fileInfo(path);
  m_playlistFileName = fileInfo.fileName();
  m_playlistFilePath = fileInfo.absoluteDir().filePath(m_playlistFileName);
  if (creator.read(path, filePaths, format, useFullPath, writeInfo)) {
    beginResetModel();
    m_items.clear();
    const auto constFilePaths = filePaths;
    for (const QString& filePath :  constFilePaths) {
      QModelIndex index = m_fsModel->index(filePath);
      if (index.isValid()) {
        m_items.append(index);
      }
    }
    endResetModel();

    m_playlistConfig.setFormat(format);
    m_playlistConfig.setUseFullPath(useFullPath);
    m_playlistConfig.setWriteInfo(writeInfo);
  } else {
    // File does not exist yet, prepare model to be populated with
    // setPathsInPlaylist().
    beginResetModel();
    m_items.clear();
    endResetModel();

    m_playlistConfig.setFormat(PlaylistConfig::formatFromFileExtension(path));
  }
  setModified(false);
}

void PlaylistModel::setModified(bool modified)
{
  if (m_modified != modified) {
    m_modified = modified;
    emit modifiedChanged(m_modified);
  }
}

bool PlaylistModel::save()
{
  PlaylistCreator creator(QString(), m_playlistConfig);
  if (creator.write(m_playlistFilePath, m_items)) {
    setModified(false);
    return true;
  }
  return false;
}

QStringList PlaylistModel::pathsInPlaylist() const
{
  QStringList paths;
  const auto idxs = m_items;
  for (const QPersistentModelIndex& idx : idxs) {
    if (const auto model =
        qobject_cast<const FileProxyModel*>(idx.model())) {
      paths.append(model->filePath(idx));
    }
  }
  return paths;
}

bool PlaylistModel::setPathsInPlaylist(const QStringList& paths)
{
  bool ok = true;
  beginResetModel();
  m_items.clear();
  for (const QString& filePath : paths) {
    QModelIndex index = m_fsModel->index(filePath);
    if (index.isValid()) {
      m_items.append(index);
    } else {
      ok = false;
    }
  }
  endResetModel();
  setModified(true);
  return ok;
}
