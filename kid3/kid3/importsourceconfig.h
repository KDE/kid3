/**
 * \file importsourceconfig.h
 * Configuration for import source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 */

#ifndef IMPORTSOURCECONFIG_H
#define IMPORTSOURCECONFIG_H

#include "generalconfig.h"
#include <qstring.h>

/**
 * Freedb configuration.
 */
class ImportSourceConfig : public GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp         configuration group
	 * @param cgiPathUsed true to use CgiPath configuration
	 */
	ImportSourceConfig(const QString& grp, bool cgiPathUsed = true);

	/**
	 * Constructor.
	 * Used to create temporary configuration.
	 */
	ImportSourceConfig();

	/**
	 * Destructor.
	 */
	virtual ~ImportSourceConfig();

	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		) const;

	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		);

	/** server */
	QString m_server;

	/** CGI path used for access */
	QString m_cgiPath;

	/** window width */
	int m_windowWidth;

	/** window height */
	int m_windowHeight;

	/** true if CgiPath configuration is used */
	bool m_cgiPathUsed;
};

#endif
