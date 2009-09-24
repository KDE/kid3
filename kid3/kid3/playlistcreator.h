/**
 * \file playlistcreator.h
 * Playlist creator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#include <qstring.h>
#include <qmap.h>

class FileListItem;
class DirInfo;
class TaggedFile;
class ImportTrackData;
class PlaylistConfig;

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
		 * @param item item in file list
		 * @param ctr  associated playlist creator
		 */
		Item(FileListItem* item, PlaylistCreator& ctr);

		/**
		 * Destructor.
		 */
		~Item();

		/**
		 * Check if item is a directory.
		 * @return true if item is directory.
		 */
		bool isDir() const { return m_dirInfo != 0; }

		/**
		 * Check if item is a tagged file.
		 * @return true if item is file.
		 */
		bool isFile() const { return m_taggedFile != 0; }

		/**
		 * Get the directory of the item.
		 * @return directory path with trailing separator.
		 */
		const QString& getDirName() { return m_dirName; }

		/**
		 * Add item to playlist.
		 * This operation will write a playlist if the configuration is set to write
		 * a playlist in every directory and a new directory is entered.
		 *
		 * @return true if ok.
		 */
		bool add();

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
		FileListItem* m_item;
		const DirInfo* m_dirInfo;
		TaggedFile* m_taggedFile;
		ImportTrackData* m_trackData;
		QString m_dirName;
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

private:
	friend class Item;

	struct Entry {
		unsigned long duration;
		QString filePath;
		QString info;
	};

	const PlaylistConfig& m_cfg;
	QString m_playlistDirName;
	QString m_playlistFileName;
	QMap<QString, Entry> m_entries;
};

#endif // PLAYLISTCREATOR_H
