/**
 * \file configstore.h
 * Configuration storage.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include <QObject>
#include <QList>
#include "kid3api.h"

class ISettings;
class GeneralConfig;

/**
 * Configuration storage.
 */
class KID3_CORE_EXPORT ConfigStore : public QObject {
public:
  /**
   * Constructor.
   * @param config application settings
   */
  explicit ConfigStore(ISettings* config);

  /**
   * Destructor.
   */
  virtual ~ConfigStore() override;

  /**
   * Persist all added configurations.
   */
  void writeToConfig();

  /**
   * Add a configuration.
   * The configuration will be read from the application settings.
   *
   * @param cfg configuration, ownership is taken
   * @return index of configuration.
   */
  int addConfiguration(GeneralConfig* cfg);

  /**
   * Access to configuration.
   * @param index index of configuration
   * @return configuration, 0 if not found.
   */
  GeneralConfig* configuration(int index) const {
    return m_configurations.at(index);
  }

  /**
   * Get a pointer to the application's config store instance.
   * @return config store, 0 if no instance has been allocated.
   */
  static ConfigStore* instance() { return s_self; }

  /**
   * Get the version number of the configuration which was read.
   * @return version number.
   */
  static int getConfigVersion() { return s_configVersion; }

private:
  ISettings* m_config;
  QList<GeneralConfig*> m_configurations;
  static ConfigStore* s_self;
  static int s_configVersion;
};

#endif // CONFIGSTORE_H
