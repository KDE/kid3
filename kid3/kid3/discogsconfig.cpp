/**
 * \file discogsconfig.cpp
 * Discogs configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#include "discogsconfig.h"
#include <qstring.h>

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
DiscogsConfig::DiscogsConfig(const QString& grp) : ImportSourceConfig(grp, false)
{
	m_server = "www.discogs.com:80";
}

/**
 * Destructor.
 */
DiscogsConfig::~DiscogsConfig() {}
