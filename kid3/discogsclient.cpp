/**
 * \file discogsclient.cpp
 * Discogs client.
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

#include "discogsclient.h"
#include "importsourceconfig.h"

static const char discogsServer[] = "www.discogs.com:80";

/**
 * Constructor.
 */
DiscogsClient::DiscogsClient()
{
}

/**
 * Destructor.
 */
DiscogsClient::~DiscogsClient()
{
}

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void DiscogsClient::sendFindQuery(
	const ImportSourceConfig*,
	const QString& artist, const QString& album)
{
	/*
	 * Query looks like this:
	 * http://www.discogs.com/search?type=releases&q=amon+amarth+avenger&btn=Search
	 */
	sendRequest(discogsServer,
							QString("/search?type=releases&q=") +
							encodeUrlQuery(artist + " " + album) + "&btn=Search");
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void DiscogsClient::sendTrackListQuery(
	const ImportSourceConfig*, const QString& cat, const QString& id)
{
	/*
	 * Query looks like this:
	 * http://www.discogs.com/release/761529
	 */
	sendRequest(discogsServer, QString("/") + cat + '/' + id);
}
