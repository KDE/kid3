/**
 * \file musicbrainzconfig.cpp
 * MusicBrainz configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#include "musicbrainzconfig.h"
#include <qstring.h>

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
MusicBrainzConfig::MusicBrainzConfig(const QString &grp) : ImportSourceConfig(grp, false)
{
	m_server = "musicbrainz.org:80";
}

/**
 * Destructor.
 */
MusicBrainzConfig::~MusicBrainzConfig() {}
