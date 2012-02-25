/**
 * \file serverimporterconfig.cpp
 * Configuration for server import.
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

#include "serverimporterconfig.h"
#include <QtGlobal>
#include "qtcompatmac.h"

#ifdef CONFIG_USE_KDE
#include <kconfiggroup.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp         configuration group
 * @param cgiPathUsed true to use CgiPath configuration
 * @param additionalTagsUsed true to use AdditionalTags configuration
 */
ServerImporterConfig::ServerImporterConfig(const QString& grp, bool cgiPathUsed,
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
ServerImporterConfig::ServerImporterConfig() : GeneralConfig("Temporary"),
  m_windowWidth(-1), m_windowHeight(-1), m_cgiPathUsed(false),
  m_additionalTagsUsed(false), m_additionalTags(false), m_coverArt(false) {}

/**
 * Destructor.
 */
ServerImporterConfig::~ServerImporterConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void ServerImporterConfig::writeToConfig(Kid3Settings* config) const
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
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
  config->setValue("/Server", QVariant(m_server));
  if (m_cgiPathUsed)
    config->setValue("/CgiPath", QVariant(m_cgiPath));
  if (m_additionalTagsUsed) {
    config->setValue("/AdditionalTags", QVariant(m_additionalTags));
    config->setValue("/CoverArt", QVariant(m_coverArt));
  }
  config->setValue("/WindowWidth", QVariant(m_windowWidth));
  config->setValue("/WindowHeight", QVariant(m_windowHeight));
  config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void ServerImporterConfig::readFromConfig(Kid3Settings* config)
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  m_server = cfg.readEntry("Server", m_server);
  if (m_cgiPathUsed)
    m_cgiPath = cfg.readEntry("CgiPath", m_cgiPath);
  if (m_additionalTagsUsed) {
    m_additionalTags = cfg.readEntry("AdditionalTags",
                                             m_additionalTags);
    m_coverArt = cfg.readEntry("CoverArt", m_coverArt);
  }
  m_windowWidth = cfg.readEntry("WindowWidth", -1);
  m_windowHeight = cfg.readEntry("WindowHeight", -1);
#else
  config->beginGroup("/" + m_group);
  m_server = config->value("/Server", m_server).toString();
  if (m_cgiPathUsed)
    m_cgiPath = config->value("/CgiPath", m_cgiPath).toString();
  if (m_additionalTagsUsed) {
    m_additionalTags = config->value("/AdditionalTags",
                                     m_additionalTags).toBool();
    m_coverArt = config->value("/CoverArt", m_coverArt).toBool();
  }
  m_windowWidth = config->value("/WindowWidth", -1).toInt();
  m_windowHeight = config->value("/WindowHeight", -1).toInt();
  config->endGroup();
#endif
}
