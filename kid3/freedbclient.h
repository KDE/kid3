/**
 * \file freedbclient.h
 * freedb.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 *
 * Copyright (C) 2004-2011  Urs Fleisch
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

#ifndef FREEDBCLIENT_H
#define FREEDBCLIENT_H

#include "importsource.h"

/**
 * freedb.org client.
 */
class FreedbClient : public ImportSource
{
public:
	/**
	 * Constructor.
	 *
	 * @param parent          parent object
	 * @param trackDataVector track data to be filled with imported values
	 */
	FreedbClient(QObject* parent,
							 ImportTrackDataVector& trackDataVector);

	/**
	 * Destructor.
	 */
	virtual ~FreedbClient();

	/**
	 * Name of import source.
	 * @return name.
	 */
	virtual QString name() const;

	/** NULL-terminated array of server strings, 0 if not used */
	virtual const char** serverList() const;

	/** default server, 0 to disable */
	virtual const char* defaultServer() const;

	/** default CGI path, 0 to disable */
	virtual const char* defaultCgiPath() const;

	/** anchor to online help, 0 to disable */
	virtual const char* helpAnchor() const;

	/** configuration, 0 if not used */
	virtual ImportSourceConfig* cfg() const;

	/**
	 * Process finished findCddbAlbum request.
	 *
	 * @param searchStr search data received
	 */
	virtual void parseFindResults(const QByteArray& searchStr);

	/**
	 * Parse result of album request and populate m_trackDataVector with results.
	 *
	 * @param albumStr album data received
	 */
	virtual void parseAlbumResults(const QByteArray& albumStr);

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
