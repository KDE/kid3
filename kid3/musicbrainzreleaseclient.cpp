/**
 * \file musicbrainzreleaseclient.cpp
 * MusicBrainz release database client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 *
 * Copyright (C) 2006-2009  Urs Fleisch
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

#include "musicbrainzreleaseclient.h"
#include "importsourceconfig.h"

/**
 * Constructor.
 */
MusicBrainzReleaseClient::MusicBrainzReleaseClient()
{
}

/**
 * Destructor.
 */
MusicBrainzReleaseClient::~MusicBrainzReleaseClient()
{
}

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void MusicBrainzReleaseClient::sendFindQuery(
	const ImportSourceConfig* cfg,
	const QString& artist, const QString& album)
{
	/*
	 * Query looks like this:
	 * http://musicbrainz.org/ws/1/release/?type=xml&artist=wizard&title=odin
	 */
	sendRequest(cfg->m_server,
							QString("/ws/1/release/?type=xml&artist=") +
							encodeUrlQuery(artist) +
							"&title=" + encodeUrlQuery(album));
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void MusicBrainzReleaseClient::sendTrackListQuery(
	const ImportSourceConfig* cfg, const QString& cat, const QString& id)
{
	/*
	 * Query looks like this:
	 * http://musicbrainz.org/ws/1/release/978c7ed1-a854-4ef2-bd4e-e7c1317be854/?type=xml&inc=artist+tracks
	 */
	QString path("/ws/1/");
	path += cat;
	path += '/';
	path += id;
	path += "/?type=xml&inc=artist+tracks";
	if (cfg->m_additionalTags) {
		path += "+release-events+artist-rels+release-rels+track-rels+"
			"track-level-rels+labels";
	}
	if (cfg->m_coverArt) {
		path += "+url-rels";
	}
	sendRequest(cfg->m_server, path);
}
