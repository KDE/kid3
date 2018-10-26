/**
 * \file playlistmodel.h
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

#pragma once

#include <QAbstractProxyModel>
#include "playlistconfig.h"
#include "kid3api.h"

class FileProxyModel;

/**
 * Playlist model.
 */
class KID3_CORE_EXPORT PlaylistModel : public QAbstractProxyModel {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param fsModel file proxy model
   * @param parent parent object
   */
  explicit PlaylistModel(FileProxyModel* fsModel, QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~PlaylistModel() override = default;

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role = Qt::EditRole) override;

  /**
   * Get number of rows.
   * @param parent parent model index, invalid for table models
   * @return number of rows,
   * if parent is valid number of children (0 for table models)
   */
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Get number of columns.
   * @param parent parent model index, invalid for table models
   * @return number of columns,
   * if parent is valid number of children (0 for table models)
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Insert rows.
   * @param row rows are inserted before this row, if 0 at the begin,
   * if rowCount() at the end
   * @param count number of rows to insert
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool insertRows(int row, int count,
                          const QModelIndex& parent = QModelIndex()) override;

  /**
   * Remove rows.
   * @param row rows are removed starting with this row
   * @param count number of rows to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool removeRows(int row, int count,
                          const QModelIndex& parent = QModelIndex()) override;

  /**
   * Get model index of item.
   * @param row row of item
   * @param column column of item
   * @param parent index of parent item
   * @return model index of item
   */
  virtual QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Get parent of item.
   * @param child model index of item
   * @return model index of parent item
   */
  virtual QModelIndex parent(const QModelIndex& child) const override;

  /**
   * Map from index in proxy model to model index.
   * @param proxyIndex index in proxy model
   * @return index in source model.
   */
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

  /**
   * Map from model index to index in proxy model.
   * @param sourceIndex index in source model
   * @return index in proxy model.
   */
  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;

  /**
   * Get supported drop actions.
   * @return supported drop actions.
   */
  virtual Qt::DropActions supportedDropActions() const override;

  /**
   * Get name of playlist file.
   * @return playlist file name.
   */
  QString playlistFileName() const { return m_playlistFileName; }

  /**
   * Set playlist to edit.
   * If the same @a path is already set, nothing is done.
   * An empty @a path can be used to clear the model, so that the playlist
   * will be read from the file when called the next time with a path.
   * Check filesNotFound() to see if some files could not be located.
   * @param path path to playlist file, empty to clear
   */
  void setPlaylistFile(const QString& path);

  /**
   * Get list of files which were not found when setPlaylistFile() was called.
   * @return list of file entries which could not be located.
   */
  QStringList filesNotFound() const { return m_filesNotFound; }

  /**
   * Modification state of playlist.
   * @return true if modified.
   */
  bool isModified() const { return m_modified; }

  /**
   * Set modification state of playlist.
   * If the state is changed, modifiedChanged() is emitted.
   * @param modified true if modified
   */
  void setModified(bool modified);

  /**
   * Get paths to files in playlist.
   * @return list of absolute paths.
   */
  QStringList pathsInPlaylist() const;

  /**
   * Set paths to files in playlist.
   * @param paths list of absolute paths
   * @return true if ok, false if not all @a paths were found and added.
   */
  bool setPathsInPlaylist(const QStringList& paths);

public slots:
  /**
   * Save changes to playlist file.
   * @return true if ok.
   */
  bool save();

signals:
  /**
   * Emitted when isModified() is changed.
   * @param modified true if modified
   */
  void modifiedChanged(bool modified);

private:
  PlaylistConfig m_playlistConfig;
  QString m_playlistFilePath;
  QString m_playlistFileName;
  QList<QPersistentModelIndex> m_items;
  QStringList m_filesNotFound;
  FileProxyModel* m_fsModel;
  bool m_modified;
};
