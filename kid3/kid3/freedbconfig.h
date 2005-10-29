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

#include <qstringlist.h>
#include "config.h"
#include "generalconfig.h"

/**
 * Freedb configuration.
 */
class FreedbConfig : public GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	FreedbConfig(const QString &grp);
	/**
	 * Constructor.
	 * Use to create temporary configuration.
	 */
	FreedbConfig();
	/**
	 * Destructor.
	 */
	virtual ~FreedbConfig();
	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		Kid3Settings *config
#endif
		) const;
	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		Kid3Settings *config
#endif
		);
	/** true if freedb proxy is used */
	bool useProxy;
	/** proxy used for freedb.org access */
	QString proxy;
	/** freedb.org server */
	QString server;
	/** CGI path used for freedb.org access */
	QString cgiPath;
	/** freedb.org window width */
	int m_windowWidth;
	/** freedb.org window height */
	int m_windowHeight;
};

#endif
