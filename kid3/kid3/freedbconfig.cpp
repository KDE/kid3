/**
 * \file freedbconfig.cpp
 * General configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include <qstring.h>
#include "freedbconfig.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
FreedbConfig::FreedbConfig(const QString &grp) : GeneralConfig(grp)
{
	useProxy = false;
	proxy = "";
	server = "freedb.freedb.org:80";
	cgiPath = "/~cddb/cddb.cgi";
	m_windowWidth = -1;
	m_windowHeight = -1;
}

/**
 * Constructor.
 * Use to create temporary configuration.
 */
FreedbConfig::FreedbConfig() : GeneralConfig("Temporary") {}

/**
 * Destructor.
 */
FreedbConfig::~FreedbConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void FreedbConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	Kid3Settings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	config->writeEntry("UseProxy", useProxy);
	config->writeEntry("Proxy", proxy);
	config->writeEntry("Server", server);
	config->writeEntry("CgiPath", cgiPath);
	config->writeEntry("FreedbWindowWidth", m_windowWidth);
	config->writeEntry("FreedbWindowHeight", m_windowHeight);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/UseProxy", useProxy);
	config->writeEntry("/Proxy", proxy);
	config->writeEntry("/Server", server);
	config->writeEntry("/CgiPath", cgiPath);
	config->writeEntry("/FreedbWindowWidth", m_windowWidth);
	config->writeEntry("/FreedbWindowHeight", m_windowHeight);
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void FreedbConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	Kid3Settings *config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	useProxy = config->readBoolEntry("UseProxy", useProxy);
	proxy = config->readEntry("Proxy", proxy);
	server = config->readEntry("Server", server);
	cgiPath = config->readEntry("CgiPath", cgiPath);
	m_windowWidth = config->readNumEntry("FreedbWindowWidth", -1);
	m_windowHeight = config->readNumEntry("FreedbWindowHeight", -1);
#else
	config->beginGroup("/" + group);
	useProxy = config->readBoolEntry("/UseProxy", useProxy);
	proxy = config->readEntry("/Proxy", proxy);
	server = config->readEntry("/Server", server);
	cgiPath = config->readEntry("/CgiPath", cgiPath);
	m_windowWidth = config->readNumEntry("/FreedbWindowWidth", -1);
	m_windowHeight = config->readNumEntry("/FreedbWindowHeight", -1);
	config->endGroup();
#endif
}
