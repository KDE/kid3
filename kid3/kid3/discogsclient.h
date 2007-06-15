/**
 * \file discogsclient.h
 * Discogs client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 */

#ifndef DISCOGSCLIENT_H
#define DISCOGSCLIENT_H

#include "importsourceclient.h"

/**
 * Discogs client.
 */
class DiscogsClient : public ImportSourceClient
{
public:
	/**
	 * Constructor.
	 */
	DiscogsClient();

	/**
	 * Destructor.
	 */
	virtual ~DiscogsClient();

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

	/**
	 * Construct a query command in m_request to fetch the track list
	 * from the server.
	 *
	 * @param cfg      import source configuration
	 * @param cat      category
	 * @param id       ID
	 * @param dest     the server to connect to is returned here
	 * @param destPort the port of the server is returned here
	 */
	virtual void constructTrackListQuery(
		const ImportSourceConfig* cfg, const QString& cat, const QString& id,
		QString& dest, int& destPort);
};

#endif
