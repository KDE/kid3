/**
 * \file tracktypeclient.h
 * TrackType.org client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Apr 2007
 */

#ifndef TRACKTYPECLIENT_H
#define TRACKTYPECLIENT_H

#include "freedbclient.h"

/**
 * TrackType.org client.
 */
class TrackTypeClient : public FreedbClient
{
public:
	/**
	 * Constructor.
	 */
	TrackTypeClient();

	/**
	 * Destructor.
	 */
	virtual ~TrackTypeClient();

	/**
	 * Construct a query command in m_request to search on the server.
	 *
	 * @param cfg      import source configuration
	 * @param artist   artist to search
	 * @param album    album to search
	 * @param dest     the server to connect to is returned here
	 * @param destPort the port of the server is returned here
	 */
	virtual void constructFindQuery(
		const ImportSourceConfig* cfg,
		const QString& artist, const QString& album,
		QString& dest, int& destPort);
};

#endif
