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
class KID3_CORE_EXPORT NetworkConfig : public StoredConfig<NetworkConfig> {
  Q_OBJECT
  /** proxy used for access */
  Q_PROPERTY(QString proxy READ proxy WRITE setProxy NOTIFY proxyChanged)
  /** proxy user name */
  Q_PROPERTY(QString proxyUserName READ proxyUserName WRITE setProxyUserName NOTIFY proxyUserNameChanged)
  /** proxy password */
  Q_PROPERTY(QString proxyPassword READ proxyPassword WRITE setProxyPassword NOTIFY proxyPasswordChanged)
  /** web browser substituted for %b */
  Q_PROPERTY(QString browser READ browser WRITE setBrowser NOTIFY browserChanged)
  /** true if proxy is used */
  Q_PROPERTY(bool useProxy READ useProxy WRITE setUseProxy NOTIFY useProxyChanged)
  /** true to use proxy authentication */
  Q_PROPERTY(bool useProxyAuthentication READ useProxyAuthentication WRITE setUseProxyAuthentication NOTIFY useProxyAuthenticationChanged)

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

  /** Get proxy used for access. */
  QString proxy() const { return m_proxy; }

  /** Set proxy used for access. */
  void setProxy(const QString& proxy);

  /** Get proxy user name. */
  QString proxyUserName() const { return m_proxyUserName; }

  /** Set proxy user name. */
  void setProxyUserName(const QString& proxyUserName);

  /** Get proxy password. */
  QString proxyPassword() const { return m_proxyPassword; }

  /** Set proxy password. */
  void setProxyPassword(const QString& proxyPassword);

  /** Get web browser substituted for %b. */
  QString browser() const { return m_browser; }

  /** Set web browser substituted for %b. */
  void setBrowser(const QString& browser);

  /** Check if proxy is used. */
  bool useProxy() const { return m_useProxy; }

  /** Set if proxy is used. */
  void setUseProxy(bool useProxy);

  /** Check if proxy authentication is used. */
  bool useProxyAuthentication() const { return m_useProxyAuthentication; }

  /** Set if proxy authentication is used. */
  void setUseProxyAuthentication(bool useProxyAuthentication);

  /**
   * Set default web browser.
   */
  void setDefaultBrowser();

signals:
  /** Emitted when @a proxy changed. */
  void proxyChanged(const QString& proxy);

  /** Emitted when @a proxyUserName changed. */
  void proxyUserNameChanged(const QString& proxyUserName);

  /** Emitted when @a proxyPassword changed. */
  void proxyPasswordChanged(const QString& proxyPassword);

  /** Emitted when @a browser changed. */
  void browserChanged(const QString& browser);

  /** Emitted when @a useProxy changed. */
  void useProxyChanged(bool useProxy);

  /** Emitted when @a useProxyAuthentication changed. */
  void useProxyAuthenticationChanged(bool useProxyAuthentication);

private:
  friend NetworkConfig& StoredConfig<NetworkConfig>::instance();

  QString m_proxy;
  QString m_proxyUserName;
  QString m_proxyPassword;
  QString m_browser;
  bool m_useProxy;
  bool m_useProxyAuthentication;

  /** Index in configuration storage */
  static int s_index;
};

#endif
