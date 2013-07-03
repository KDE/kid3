/**
 * \file configstore.cpp
 * Configuration storage.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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

#include "configstore.h"
#include "generalconfig.h"

ConfigStore* ConfigStore::s_self = 0;

/**
 * Constructor.
 * @param config application settings
 */
ConfigStore::ConfigStore(ISettings* config) : m_config(config)
{
  Q_ASSERT_X(!s_self, "ConfigStore", "there should be only one config store");
  s_self = this;
}

/**
 * Destructor.
 */
ConfigStore::~ConfigStore()
{
  qDeleteAll(m_configurations);
}

/**
 * Persist all added configurations.
 */
void ConfigStore::writeToConfig()
{
  foreach (GeneralConfig* cfg, m_configurations) {
    cfg->writeToConfig(m_config);
  }
}

/**
 * Add a configuration.
 * The configuration will be read from the application settings.
 *
 * @param cfg configuration, ownership is taken
 * @return index of configuration.
 */
int ConfigStore::addConfiguration(GeneralConfig* cfg)
{
  Q_ASSERT(cfg);
  int index = m_configurations.size();
  m_configurations.append(cfg);
  cfg->readFromConfig(m_config);
  return index;
}
