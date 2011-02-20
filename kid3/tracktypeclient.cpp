/**
 * \file tracktypeclient.cpp
 * TrackType.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Apr 2007
 *
 * Copyright (C) 2007-2009  Urs Fleisch
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

#include "tracktypeclient.h"
#include "importsourceconfig.h"

static const char trackTypeServer[] = "tracktype.org:80";

/**
 * Constructor.
 */
TrackTypeClient::TrackTypeClient()
{
}

/**
 * Destructor.
 */
TrackTypeClient::~TrackTypeClient()
{
}

/**
 * Send a query command to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void TrackTypeClient::sendFindQuery(
	const ImportSourceConfig* cfg,
	const QString& artist, const QString& album)
{
	// At the moment, only TrackType.org recognizes cddb album commands,
	// so we always use this server for find queries.
	sendRequest(trackTypeServer,
							cfg->m_cgiPath + "?cmd=cddb+album+" +
							encodeUrlQuery(artist + " / " + album) +
							"&hello=noname+localhost+Kid3+" VERSION "&proto=6");
}
