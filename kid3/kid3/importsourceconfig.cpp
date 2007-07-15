/**
 * \file importsourceconfig.cpp
 * Configuration for import source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 */

#include "importsourceconfig.h"
#include <qglobal.h>
#include "qtcompatmac.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp         configuration group
 * @param cgiPathUsed true to use CgiPath configuration
 */
ImportSourceConfig::ImportSourceConfig(const QString& grp, bool cgiPathUsed) :
	GeneralConfig(grp), m_windowWidth(-1), m_windowHeight(-1),
	m_cgiPathUsed(cgiPathUsed)
{
}

/**
 * Constructor.
 * Used to create temporary configuration.
 */
ImportSourceConfig::ImportSourceConfig() : GeneralConfig("Temporary") {}

/**
 * Destructor.
 */
ImportSourceConfig::~ImportSourceConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void ImportSourceConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(m_group);
	config->writeEntry("Server", m_server);
	if (m_cgiPathUsed)
		config->writeEntry("CgiPath", m_cgiPath);
	config->writeEntry("WindowWidth", m_windowWidth);
	config->writeEntry("WindowHeight", m_windowHeight);
#else
	config->beginGroup("/" + m_group);
	config->QCM_writeEntry("/Server", m_server);
	if (m_cgiPathUsed)
		config->QCM_writeEntry("/CgiPath", m_cgiPath);
	config->QCM_writeEntry("/WindowWidth", m_windowWidth);
	config->QCM_writeEntry("/WindowHeight", m_windowHeight);
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void ImportSourceConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	config->setGroup(m_group);
	m_server = config->readEntry("Server", m_server);
	if (m_cgiPathUsed)
		m_cgiPath = config->readEntry("CgiPath", m_cgiPath);
	m_windowWidth = config->readNumEntry("WindowWidth", -1);
	m_windowHeight = config->readNumEntry("WindowHeight", -1);
#else
	config->beginGroup("/" + m_group);
	m_server = config->QCM_readEntry("/Server", m_server);
	if (m_cgiPathUsed)
		m_cgiPath = config->QCM_readEntry("/CgiPath", m_cgiPath);
	m_windowWidth = config->QCM_readNumEntry("/WindowWidth", -1);
	m_windowHeight = config->QCM_readNumEntry("/WindowHeight", -1);
	config->endGroup();
#endif
}
