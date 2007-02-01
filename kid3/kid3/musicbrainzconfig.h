/**
 * \file musicbrainzconfig.h
 * MusicBrainz configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#ifndef MUSICBRAINZCONFIG_H
#define MUSICBRAINZCONFIG_H

#include "importsourceconfig.h"

/**
 * MusicBrainz configuration.
 */
class MusicBrainzConfig : public ImportSourceConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	MusicBrainzConfig(const QString& grp);

	/**
	 * Destructor.
	 */
	virtual ~MusicBrainzConfig();
};

#endif // MUSICBRAINZCONFIG_H
