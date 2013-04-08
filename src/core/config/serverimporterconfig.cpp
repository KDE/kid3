/**
 * \file serverimporterconfig.cpp
 * Configuration for server import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2013  Urs Fleisch
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
  GeneralConfig(grp),
  m_cgiPathUsed(cgiPathUsed), m_additionalTagsUsed(additionalTagsUsed),
  m_standardTags(true), m_additionalTags(true), m_coverArt(true)
{
}

/**
 * Constructor.
 * Used to create temporary configuration.
 */
ServerImporterConfig::ServerImporterConfig() : GeneralConfig(QLatin1String("Temporary")),
  m_cgiPathUsed(false),
  m_additionalTagsUsed(false), m_standardTags(false), m_additionalTags(false),
  m_coverArt(false) {}

/**
 * Destructor.
 */
ServerImporterConfig::~ServerImporterConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void ServerImporterConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("Server"), QVariant(m_server));
  if (m_cgiPathUsed)
    config->setValue(QLatin1String("CgiPath"), QVariant(m_cgiPath));
  if (m_additionalTagsUsed) {
    config->setValue(QLatin1String("StandardTags"), QVariant(m_standardTags));
    config->setValue(QLatin1String("AdditionalTags"), QVariant(m_additionalTags));
    config->setValue(QLatin1String("CoverArt"), QVariant(m_coverArt));
  }
  config->setValue(QLatin1String("WindowGeometry"), QVariant(m_windowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void ServerImporterConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_server = config->value(QLatin1String("Server"), m_server).toString();
  if (m_cgiPathUsed)
    m_cgiPath = config->value(QLatin1String("CgiPath"), m_cgiPath).toString();
  if (m_additionalTagsUsed) {
    m_standardTags = config->value(QLatin1String("StandardTags"),
                                   m_standardTags).toBool();
    m_additionalTags = config->value(QLatin1String("AdditionalTags"),
                                     m_additionalTags).toBool();
    m_coverArt = config->value(QLatin1String("CoverArt"), m_coverArt).toBool();
  }
  m_windowGeometry = config->value(QLatin1String("WindowGeometry"),
                                   m_windowGeometry).toByteArray();
  config->endGroup();
}
