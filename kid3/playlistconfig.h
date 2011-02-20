/**
 * \file playlistconfig.h
 * Configuration for playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Sep 2009
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

#ifndef PLAYLISTCONFIG_H
#define PLAYLISTCONFIG_H

#include <qstring.h>
#include "config.h"
#include "generalconfig.h"

/**
 * Playlist configuration.
 */
class PlaylistConfig : public GeneralConfig {
public:
	/**
	 * Playlist format.
	 */
	enum PlaylistFormat {
		PF_M3U, /**< M3U */
		PF_PLS, /**< PLS */
		PF_XSPF /**< XSPF */
	};

	/**
	 * Location to create playlist.
	 */
	enum PlaylistLocation {
		PL_CurrentDirectory, /**< create in current directory */
		PL_EveryDirectory,   /**< create in every directory */
		PL_TopLevelDirectory /**< create in top-level directory */
	};

	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	PlaylistConfig(const QString& grp = "Playlist");

	/**
	 * Destructor.
	 */
	virtual ~PlaylistConfig();

	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		) const;

	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		);

	/** Use file name format if true, else directory name */
	bool m_useFileNameFormat;
	/** Include only selected files if true, else all files */
	bool m_onlySelectedFiles;
	/** Sort by tag field if true, else file name */
	bool m_useSortTagField;
	/** Use full path for files in playlist if true, else relative path */
	bool m_useFullPath;
	/** Write info format, else only list of files */
	bool m_writeInfo;
	/** Playlist location */
	PlaylistLocation m_location;
	/** Playlist format */
	PlaylistFormat m_format;
	/** Playlist file name format */
	QString m_fileNameFormat;
	/** Tag field used for sorting */
	QString m_sortTagField;
	/** Format for additional information */
	QString m_infoFormat;
};

#endif // PLAYLISTCONFIG_H
