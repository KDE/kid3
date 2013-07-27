/**
 * \file networkconfig.h
 * Network related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include "generalconfig.h"
#include "kid3api.h"

/**
 * Network related configuration.
 */
class KID3_CORE_EXPORT NetworkConfig : public StoredConfig<NetworkConfig>
{
public:
  /**
   * Constructor.
   */
  NetworkConfig();

  /**
   * Destructor.
   */
  virtual ~NetworkConfig();

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config);

  /** true if proxy is used */
  bool m_useProxy;
  /** proxy used for access */
  QString m_proxy;
  /** true to use proxy authentication */
  bool m_useProxyAuthentication;
  /** proxy user name */
  QString m_proxyUserName;
  /** proxy password */
  QString m_proxyPassword;
  /** web browser substituted for %b */
  QString m_browser;
};

template<>
KID3_CORE_EXPORT int StoredConfig<NetworkConfig, GeneralConfig>::s_index;

#endif
