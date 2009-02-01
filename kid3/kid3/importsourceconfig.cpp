/**
 * \file importsourceconfig.cpp
 * Configuration for import source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "importsourceconfig.h"
#include <qglobal.h>
#include "qtcompatmac.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kconfigskeleton.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp         configuration group
 * @param cgiPathUsed true to use CgiPath configuration
 * @param additionalTagsUsed true to use AdditionalTags configuration
 */
ImportSourceConfig::ImportSourceConfig(const QString& grp, bool cgiPathUsed,
																			 bool additionalTagsUsed) :
	GeneralConfig(grp), m_windowWidth(-1), m_windowHeight(-1),
	m_cgiPathUsed(cgiPathUsed), m_additionalTagsUsed(additionalTagsUsed),
	m_additionalTags(true), m_coverArt(true)
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
	KCM_KConfigGroup(cfg, config, m_group);
	cfg.writeEntry("Server", m_server);
	if (m_cgiPathUsed)
		cfg.writeEntry("CgiPath", m_cgiPath);
	if (m_additionalTagsUsed) {
		cfg.writeEntry("AdditionalTags", m_additionalTags);
		cfg.writeEntry("CoverArt", m_coverArt);
	}
	cfg.writeEntry("WindowWidth", m_windowWidth);
	cfg.writeEntry("WindowHeight", m_windowHeight);
#else
	config->beginGroup("/" + m_group);
	config->QCM_writeEntry("/Server", m_server);
	if (m_cgiPathUsed)
		config->QCM_writeEntry("/CgiPath", m_cgiPath);
	if (m_additionalTagsUsed) {
		config->QCM_writeEntry("/AdditionalTags", m_additionalTags);
		config->QCM_writeEntry("/CoverArt", m_coverArt);
	}
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
	KCM_KConfigGroup(cfg, config, m_group);
	m_server = cfg.readEntry("Server", m_server);
	if (m_cgiPathUsed)
		m_cgiPath = cfg.readEntry("CgiPath", m_cgiPath);
	if (m_additionalTagsUsed) {
		m_additionalTags = cfg.KCM_readBoolEntry("AdditionalTags",
		                                         m_additionalTags);
		m_coverArt = cfg.KCM_readBoolEntry("CoverArt", m_coverArt);
	}
	m_windowWidth = cfg.KCM_readNumEntry("WindowWidth", -1);
	m_windowHeight = cfg.KCM_readNumEntry("WindowHeight", -1);
#else
	config->beginGroup("/" + m_group);
	m_server = config->QCM_readEntry("/Server", m_server);
	if (m_cgiPathUsed)
		m_cgiPath = config->QCM_readEntry("/CgiPath", m_cgiPath);
	if (m_additionalTagsUsed) {
		m_additionalTags = config->QCM_readBoolEntry("/AdditionalTags",
		                                             m_additionalTags);
		m_coverArt = config->QCM_readBoolEntry("/CoverArt", m_coverArt);
	}
	m_windowWidth = config->QCM_readNumEntry("/WindowWidth", -1);
	m_windowHeight = config->QCM_readNumEntry("/WindowHeight", -1);
	config->endGroup();
#endif
}
