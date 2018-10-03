/**
 * \file playlistcreator.h
 * Playlist creator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2009
 *
 * Copyright (C) 2009-2011  Urs Fleisch
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

#ifndef PLAYLISTCREATOR_H
#define PLAYLISTCREATOR_H

#include <QString>
#include <QMap>
#include <QScopedPointer>
#include "playlistconfig.h"

class QModelIndex;
class QPersistentModelIndex;
class TaggedFile;
class ImportTrackData;

/**
 * Playlist creator.
 * Creates playlists from added items according to a playlist configuration.
 */
class PlaylistCreator {
public:
  /**
   * An item from the file list which can be added to a playlist.
   * The item will only be added to the playlist if add() is called.
   */
  class Item {
  public:
    /**
     * Constructor.
     *
     * @param index model index
     * @param ctr  associated playlist creator
     */
    Item(const QModelIndex& index, PlaylistCreator& ctr);

    /**
     * Destructor.
     */
    ~Item() = default;

    /**
     * Check if item is a directory.
     * @return true if item is directory.
     */
    bool isDir() const { return m_isDir; }

    /**
     * Check if item is a tagged file.
     * @return true if item is file.
     */
    bool isFile() const { return m_taggedFile != nullptr; }

    /**
     * Get the directory of the item.
     * @return directory path with trailing separator.
     */
    QString getDirName() const { return m_dirName; }

    /**
     * Add item to playlist.
     * This operation will write a playlist if the configuration is set to write
     * a playlist in every directory and a new directory is entered.
     *
     * @return true if ok.
     */
    bool add();

    /**
     * Get additional information for item.
     * @param info additional information is returned here
     * @param duration the duration of the track is returned here
     */
    void getInfo(QString& info, unsigned long& duration);

  private:
    /**
     * Format string using tags and properties of item.
     *
     * @param format format string
     *
     * @return string with percent codes replaced.
     */
    QString formatString(const QString& format);

    PlaylistCreator& m_ctr;
    TaggedFile* m_taggedFile;
    QScopedPointer<ImportTrackData> m_trackData;
    QString m_dirName;
    bool m_isDir;
  };

  /**
   * Constructor.
   *
   * @param topLevelDir top-level directory of playlist
   * @param cfg         playlist configuration
   */
  PlaylistCreator(const QString& topLevelDir, const PlaylistConfig& cfg);

  /**
   * Write playlist containing added Entry elements.
   *
   * @return true if ok.
   */
  bool write();

  /**
   * Write a playlist from a list of model indexes.
   * @param playlistPath file path to be used for playlist
   * @param indexes indexes in FileProxyModel
   * @return true if ok.
   */
  bool write(const QString& playlistPath,
             const QList<QPersistentModelIndex>& indexes);

  /**
   * Read playlist from file
   * @param playlistPath path to playlist file
   * @param filePaths absolute paths to the playlist files are returned here
   * @param format the playlist format is returned here
   * @param hasFullPath true is returned here if the files use absolute paths
   * @param hasInfo true is returned here if the playlist contains additional
   *                information
   * @return true if ok.
   */
  bool read(const QString& playlistPath, QStringList& filePaths,
            PlaylistConfig::PlaylistFormat& format,
            bool& hasFullPath, bool& hasInfo) const;

private:
  friend class Item;

  struct Entry {
    Entry() : duration(0) {}
    unsigned long duration;
    QString filePath;
    QString info;
  };

  bool write(const QList<Entry>& entries);

  const PlaylistConfig& m_cfg;
  QString m_playlistDirName;
  QString m_playlistFileName;
  QMap<QString, Entry> m_entries;
};

#endif // PLAYLISTCREATOR_H
