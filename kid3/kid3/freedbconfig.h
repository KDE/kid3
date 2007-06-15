/**
 * \file freedbconfig.h
 * Freedb configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Jan 2004
 */

#ifndef FREEDBCONFIG_H
#define FREEDBCONFIG_H

#include "importsourceconfig.h"

/**
 * Freedb configuration.
 */
class FreedbConfig : public ImportSourceConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	FreedbConfig(const QString& grp);

	/**
	 * Destructor.
	 */
	virtual ~FreedbConfig();
};

#endif
