/**
 * \file configstore.cpp
 * Configuration storage.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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
#include "isettings.h"

namespace {

/**
 * Current configuration file version.
 * 0: <= 3.0.2
 * 1: 3.1
 * 2: 3.2
 * 3: 3.3
 * 4: 3.7
 * 5: 3.8.3
 * 6: 3.8.5
 * 7: 3.9.0
 * 8: 3.9.3
 */
const int CONFIG_VERSION = 8;

}

/** Version of configuration file read, -1 if not read yet. */
int ConfigStore::s_configVersion = -1;


ConfigStore* ConfigStore::s_self = nullptr;

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
  const auto cfgs = m_configurations;
  for (const GeneralConfig* cfg : cfgs) {
    cfg->writeToConfig(m_config);
  }
  m_config->beginGroup(QLatin1String("ConfigStore"));
  m_config->setValue(QLatin1String("ConfigVersion"), QVariant(
      s_configVersion > CONFIG_VERSION ? s_configVersion : CONFIG_VERSION));
  m_config->endGroup();
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
  if (!cfg)
    return -1;

  if (s_configVersion == -1) {
    m_config->beginGroup(QLatin1String("ConfigStore"));
    s_configVersion =
        m_config->value(QLatin1String("ConfigVersion"), 0).toInt();
    m_config->endGroup();
  }
  int index = m_configurations.size();
  m_configurations.append(cfg);
  cfg->readFromConfig(m_config);
  return index;
}
