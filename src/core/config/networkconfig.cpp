/**
 * \file networkconfig.cpp
 * Network related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "networkconfig.h"
#include <cstdlib>

namespace {

/** Default value for web browser */
#ifdef Q_OS_MAC
const char* const defaultBrowser = "open";
#else
const char* const defaultBrowser = "xdg-open";
#endif

}

int NetworkConfig::s_index = -1;

/**
 * Constructor.
 */
NetworkConfig::NetworkConfig()
  : StoredConfig<NetworkConfig>(QLatin1String("Network")),
    m_useProxy(false),
    m_useProxyAuthentication(false)
{
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void NetworkConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("UseProxy"), QVariant(m_useProxy));
  config->setValue(QLatin1String("Proxy"), QVariant(m_proxy));
  config->setValue(QLatin1String("UseProxyAuthentication"),
                   QVariant(m_useProxyAuthentication));
  config->setValue(QLatin1String("ProxyUserName"), QVariant(m_proxyUserName));
  config->setValue(QLatin1String("ProxyPassword"), QVariant(m_proxyPassword));
  config->setValue(QLatin1String("Browser"), QVariant(m_browser));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void NetworkConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_useProxy = config->value(QLatin1String("UseProxy"), m_useProxy).toBool();
  m_proxy = config->value(QLatin1String("Proxy"), m_proxy).toString();
  m_useProxyAuthentication = config->value(QLatin1String("UseProxyAuthentication"),
                                           m_useProxyAuthentication).toBool();
  m_proxyUserName = config->value(QLatin1String("ProxyUserName"),
                                  m_proxyUserName).toString();
  m_proxyPassword = config->value(QLatin1String("ProxyPassword"),
                                  m_proxyPassword).toString();
  m_browser = config->value(QLatin1String("Browser"), QString()).toString();
  if (m_browser.isEmpty()) {
    setDefaultBrowser();
  }
  config->endGroup();
}

void NetworkConfig::setDefaultBrowser()
{
#ifdef Q_OS_WIN32
  if (m_browser.isEmpty()) {
    m_browser = QString::fromLocal8Bit(qgetenv("ProgramFiles"));
    m_browser += QLatin1String("\\Internet Explorer\\IEXPLORE.EXE");
  }
#else
  m_browser = QString::fromLatin1(defaultBrowser);
#endif
}

void NetworkConfig::setProxy(const QString& proxy)
{
  if (m_proxy != proxy) {
    m_proxy = proxy;
    emit proxyChanged(m_proxy);
  }
}

void NetworkConfig::setProxyUserName(const QString& proxyUserName)
{
  if (m_proxyUserName != proxyUserName) {
    m_proxyUserName = proxyUserName;
    emit proxyUserNameChanged(m_proxyUserName);
  }
}

void NetworkConfig::setProxyPassword(const QString& proxyPassword)
{
  if (m_proxyPassword != proxyPassword) {
    m_proxyPassword = proxyPassword;
    emit proxyPasswordChanged(m_proxyPassword);
  }
}

void NetworkConfig::setBrowser(const QString& browser)
{
  if (m_browser != browser) {
    m_browser = browser;
    emit browserChanged(m_browser);
  }
}

void NetworkConfig::setUseProxy(bool useProxy)
{
  if (m_useProxy != useProxy) {
    m_useProxy = useProxy;
    emit useProxyChanged(m_useProxy);
  }
}

void NetworkConfig::setUseProxyAuthentication(bool useProxyAuthentication)
{
  if (m_useProxyAuthentication != useProxyAuthentication) {
    m_useProxyAuthentication = useProxyAuthentication;
    emit useProxyAuthenticationChanged(m_useProxyAuthentication);
  }
}
