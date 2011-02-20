/**
 * \file importsourceclient.cpp
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

#include "importsourceclient.h"
#include <qregexp.h>
#include <qstatusbar.h>
#include <qurl.h>

#include "importsourceconfig.h"
#include "kid3.h"

/**
 * Constructor.
 */
ImportSourceClient::ImportSourceClient() :
 m_requestType(RT_None)
{
	connect(this, SIGNAL(bytesReceived(const QByteArray&)),
					this, SLOT(requestFinished(const QByteArray&)));
}

/**
 * Destructor.
 */
ImportSourceClient::~ImportSourceClient()
{
}

/**
 * Find keyword on server.
 *
 * @param cfg    import source configuration
 * @param artist artist to search
 * @param album  album to search
 */
void ImportSourceClient::find(const ImportSourceConfig* cfg,
															const QString& artist, const QString& album)
{
	sendFindQuery(cfg, artist, album);
	m_requestType = RT_Find;
}

/**
 * Handle response when request is finished.
 * The data is sent to other objects via signals.
 *
 * @param rcvStr received data
 */
void ImportSourceClient::requestFinished(const QByteArray& rcvStr)
{
	switch (m_requestType) {
		case RT_Album:
			emit albumFinished(rcvStr);
			break;
		case RT_Find:
			emit findFinished(rcvStr);
			break;
		default:
			qWarning("Unknown import request type");
	}
}

/**
 * Request track list from server.
 *
 * @param cfg import source configuration
 * @param cat category
 * @param id  ID
 */
void ImportSourceClient::getTrackList(const ImportSourceConfig* cfg, QString cat, QString id)
{
	sendTrackListQuery(cfg, cat, id);
	m_requestType = RT_Album;
}

/**
 * Encode a query in an URL.
 * The query is percent-encoded with spaces collapsed and replaced by '+'.
 *
 * @param query query to encode
 *
 * @return encoded query.
 */
QString ImportSourceClient::encodeUrlQuery(const QString& query)
{
	QString result(query);
	result.replace(QRegExp(" +"), " "); // collapse spaces
	QCM_QUrl_encode(result);
	result.replace("%20", "+"); // replace spaces by '+'
	return result;
}
