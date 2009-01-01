/**
 * \file freedbclient.cpp
 * freedb.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 *
 * Copyright (C) 2004-2009  Urs Fleisch
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

#include "freedbclient.h"
#include "importsourceconfig.h"

static const char gnudbServer[] = "www.gnudb.org:80";

/**
 * Constructor.
 */
FreedbClient::FreedbClient()
{
}

/**
 * Destructor.
 */
FreedbClient::~FreedbClient()
{
}

/**
 * Send a query command in to search on the server.
 *
 * @param cfg      import source configuration
 * @param artist   artist to search
 * @param album    album to search
 */
void FreedbClient::sendFindQuery(
	const ImportSourceConfig*,
	const QString& artist, const QString& album)
{
	// At the moment, only www.gnudb.org has a working search
	// so we always use this server for find queries.
	sendRequest(gnudbServer, QString("/search/") +
							encodeUrlQuery(artist + " " + album));
}

/**
 * Send a query command to fetch the track list
 * from the server.
 *
 * @param cfg      import source configuration
 * @param cat      category
 * @param id       ID
 */
void FreedbClient::sendTrackListQuery(
	const ImportSourceConfig* cfg, const QString& cat, const QString& id)
{
	sendRequest(cfg->m_server,
							cfg->m_cgiPath + "?cmd=cddb+read+" + cat + "+" + id +
							"&hello=noname+localhost+Kid3+" VERSION "&proto=6");
}
