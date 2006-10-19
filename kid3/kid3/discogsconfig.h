/**
 * \file discogsconfig.h
 * Discogs configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#ifndef DISCOGSCONFIG_H
#define DISCOGSCONFIG_H

#include "importsourceconfig.h"

/**
 * Discogs configuration.
 */
class DiscogsConfig : public ImportSourceConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	DiscogsConfig(const QString &grp);

	/**
	 * Destructor.
	 */
	virtual ~DiscogsConfig();
};

#endif // DISCOGSCONFIG_H
