/**
 * \file importsourceclient.h
 * Client to connect to server with import data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
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

#ifndef IMPORTSOURCECLIENT_H
#define IMPORTSOURCECLIENT_H

#include "httpclient.h"

class ImportSourceConfig;

/**
 * Client to connect to server with import data.
 */
class ImportSourceClient : public HttpClient
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	ImportSourceClient();

	/**
	 * Destructor.
	 */
	virtual ~ImportSourceClient();

	/**
	 * Send a query command to search on the server.
	 * This method has to be reimplemented for the specific search command.
	 *
	 * @param cfg      import source configuration
	 * @param artist   artist to search
	 * @param album    album to search
	 */
	virtual void sendFindQuery(
		const ImportSourceConfig* cfg,
		const QString& artist, const QString& album) = 0;

	/**
	 * Send a query command to fetch the track list
	 * from the server.
	 * This method has to be reimplemented for the specific server.
	 *
	 * @param cfg      import source configuration
	 * @param cat      category
	 * @param id       ID
	 */
	virtual void sendTrackListQuery(
		const ImportSourceConfig* cfg, const QString& cat, const QString& id) = 0;

	/**
	 * Find artist, album on server.
	 *
	 * @param cfg    import source configuration
	 * @param artist artist to search
	 * @param album  album to search
	 */
	void find(const ImportSourceConfig* cfg,
						const QString& artist, const QString& album);

	/**
	 * Request track list from server.
	 *
	 * @param cfg import source configuration
	 * @param cat category
	 * @param id  ID
	 */
	void getTrackList(const ImportSourceConfig* cfg, QString cat, QString id);

 /**
	* Encode a query in an URL.
	* The query is percent-encoded with spaces collapsed and replaced by '+'.
	*
	* @param query query to encode
	*
	* @return encoded query.
	*/
	static QString encodeUrlQuery(const QString& query);

signals:
	/**
	 * Emitted when find request finished.
	 * Parameter: text containing result of find request
	 */
	void findFinished(const QByteArray&);

	/**
	 * Emitted when album track data request finished.
	 * Parameter: text containing result of album request
	 */
	void albumFinished(const QByteArray&);

private slots:
	/**
	 * Handle response when request is finished.
	 * The data is sent to other objects via signals.
	 *
	 * @param rcvStr received data
	 */
	void requestFinished(const QByteArray& rcvStr);

private:
	/** type of current request */
	enum RequestType {
		RT_None,
		RT_Find,
		RT_Album
	} m_requestType;
};

#endif
