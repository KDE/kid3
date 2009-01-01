/**
 * \file musicbrainzreleaseclient.h
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

#ifndef MUSICBRAINZRELEASECLIENT_H
#define MUSICBRAINZRELEASECLIENT_H

#include "importsourceclient.h"

/**
 * MusicBrainz release database client.
 */
class MusicBrainzReleaseClient : public ImportSourceClient
{
public:
	/**
	 * Constructor.
	 */
	MusicBrainzReleaseClient();

	/**
	 * Destructor.
	 */
	virtual ~MusicBrainzReleaseClient();

	/**
	 * Send a query command to search on the server.
	 *
	 * @param cfg      import source configuration
	 * @param artist   artist to search
	 * @param album    album to search
	 */
	virtual void sendFindQuery(
		const ImportSourceConfig* cfg,
		const QString& artist, const QString& album);

	/**
	 * Send a query command to fetch the track list
	 * from the server.
	 *
	 * @param cfg      import source configuration
	 * @param cat      category
	 * @param id       ID
	 */
	virtual void sendTrackListQuery(
		const ImportSourceConfig* cfg, const QString& cat, const QString& id);
};

#endif
