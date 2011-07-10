/**
 * \file configstore.h
 * Configuration storage.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
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

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "generalconfig.h" // Kid3Settings
#include "formatconfig.h"
#include "importconfig.h"
#include "miscconfig.h"
#include "freedbconfig.h"
#include "discogsconfig.h"
#include "amazonconfig.h"
#include "musicbrainzconfig.h"
#include "filterconfig.h"
#include "playlistconfig.h"

/**
 * Configuration storage.
 */
class ConfigStore {
public:
	/**
	 * Constructor.
	 */
	ConfigStore();

	/**
	 * Destructor.
	 */
	~ConfigStore();

	/**
	 * Persist configuration.
	 */
	void writeToConfig();

	/**
	 * Read persisted configuration.
	 */
	void readFromConfig();

	/**
	 * Get settings.
	 * @return settings.
	 */
	Kid3Settings* getSettings() const { return m_config; }

	/** Filename format configuration */
	static FormatConfig s_fnFormatCfg;
	/** ID3 format configuration */
	static FormatConfig s_id3FormatCfg;
	/** Import configuration */
	static ImportConfig s_genCfg;
	/** Miscellaneous configuration */
	static MiscConfig s_miscCfg;
	/** Freedb configuration */
	static FreedbConfig s_freedbCfg;
	/** TrackType configuration */
	static FreedbConfig s_trackTypeCfg;
	/** Discogs configuration */
	static DiscogsConfig s_discogsCfg;
	/** Amazon configuration */
	static AmazonConfig s_amazonCfg;
	/** MusicBrainz configuration */
	static MusicBrainzConfig s_musicBrainzCfg;
	/** Filter configuration */
	static FilterConfig s_filterCfg;
	/** Playlist configuration */
	static PlaylistConfig s_playlistCfg;

private:
	Kid3Settings* m_config;
};

#endif // CONFIGSTORE_H
