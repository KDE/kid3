/**
 * \file musicbrainzconfig.cpp
 * MusicBrainz configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#include "musicbrainzconfig.h"
#ifdef HAVE_TUNEPIMP

#include <qstring.h>

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
MusicBrainzConfig::MusicBrainzConfig(const QString &grp) : GeneralConfig(grp)
{
	m_useProxy = false;
	m_proxy = "";
	m_server = "musicbrainz.org:80";
}

/**
 * Constructor.
 * Use to create temporary configuration.
 */
MusicBrainzConfig::MusicBrainzConfig() : GeneralConfig("Temporary") {}

/**
 * Destructor.
 */
MusicBrainzConfig::~MusicBrainzConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void MusicBrainzConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	Kid3Settings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	config->writeEntry("UseProxy", m_useProxy);
	config->writeEntry("Proxy", m_proxy);
	config->writeEntry("Server", m_server);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/UseProxy", m_useProxy);
	config->writeEntry("/Proxy", m_proxy);
	config->writeEntry("/Server", m_server);
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void MusicBrainzConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	Kid3Settings *config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	m_useProxy = config->readBoolEntry("UseProxy", m_useProxy);
	m_proxy = config->readEntry("Proxy", m_proxy);
	m_server = config->readEntry("Server", m_server);
#else
	config->beginGroup("/" + group);
	m_useProxy = config->readBoolEntry("/UseProxy", m_useProxy);
	m_proxy = config->readEntry("/Proxy", m_proxy);
	m_server = config->readEntry("/Server", m_server);
	config->endGroup();
#endif
}

#endif // HAVE_TUNEPIMP
