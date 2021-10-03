/**
 * \file serverimporterconfig.cpp
 * Configuration for server import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2018  Urs Fleisch
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
#include "isettings.h"

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp         configuration group
 */
ServerImporterConfig::ServerImporterConfig(const QString& grp)
  : GeneralConfig(grp),
    m_cgiPathUsed(true), m_additionalTagsUsed(false),
    m_standardTags(true), m_additionalTags(true), m_coverArt(true)
{
}

/**
 * Constructor.
 * Used to create temporary configuration.
 */
ServerImporterConfig::ServerImporterConfig()
  : GeneralConfig(QLatin1String("Temporary")),
    m_cgiPathUsed(false),
    m_additionalTagsUsed(false), m_standardTags(false), m_additionalTags(false),
    m_coverArt(false) {}

/**
 * Persist configuration.
 *
 * @param config configuration
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
  QStringList propertiesKv;
  const QList<QByteArray> propertyNames = dynamicPropertyNames();
  for (const QByteArray& propertyName : propertyNames) {
    propertiesKv.append(QString::fromLatin1(propertyName));
    propertiesKv.append(property(propertyName).toString());
  }
  config->setValue(QLatin1String("Properties"), QVariant(propertiesKv));
  config->endGroup();
  config->beginGroup(m_group, true);
  config->setValue(QLatin1String("WindowGeometry"), QVariant(m_windowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
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
  QStringList propertiesKv =
      config->value(QLatin1String("Properties"), QStringList()).toStringList();
  for (auto it = propertiesKv.constBegin();
       it != propertiesKv.constEnd();
       ++it) {
    QString key = *it;
    if (++it == propertiesKv.constEnd()) {
      break;
    }
    setProperty(key.toLatin1(), *it);
  }
  config->endGroup();
  config->beginGroup(m_group, true);
  m_windowGeometry = config->value(QLatin1String("WindowGeometry"),
                                   m_windowGeometry).toByteArray();
  config->endGroup();
}

void ServerImporterConfig::setServer(const QString& server)
{
  if (m_server != server) {
    m_server = server;
    emit serverChanged(m_server);
  }
}

void ServerImporterConfig::setCgiPath(const QString& cgiPath)
{
  if (m_cgiPath != cgiPath) {
    m_cgiPath = cgiPath;
    emit cgiPathChanged(m_cgiPath);
  }
}

void ServerImporterConfig::setWindowGeometry(const QByteArray& windowGeometry)
{
  if (m_windowGeometry != windowGeometry) {
    m_windowGeometry = windowGeometry;
    emit windowGeometryChanged(m_windowGeometry);
  }
}

void ServerImporterConfig::setCgiPathUsed(bool cgiPathUsed)
{
  if (m_cgiPathUsed != cgiPathUsed) {
    m_cgiPathUsed = cgiPathUsed;
    emit cgiPathUsedChanged(m_cgiPathUsed);
  }
}

void ServerImporterConfig::setAdditionalTagsUsed(bool additionalTagsUsed)
{
  if (m_additionalTagsUsed != additionalTagsUsed) {
    m_additionalTagsUsed = additionalTagsUsed;
    emit additionalTagsUsedChanged(m_additionalTagsUsed);
  }
}

void ServerImporterConfig::setStandardTags(bool standardTags)
{
  if (m_standardTags != standardTags) {
    m_standardTags = standardTags;
    emit standardTagsChanged(m_standardTags);
  }
}

void ServerImporterConfig::setAdditionalTags(bool additionalTags)
{
  if (m_additionalTags != additionalTags) {
    m_additionalTags = additionalTags;
    emit additionalTagsChanged(m_additionalTags);
  }
}

void ServerImporterConfig::setCoverArt(bool coverArt)
{
  if (m_coverArt != coverArt) {
    m_coverArt = coverArt;
    emit coverArtChanged(m_coverArt);
  }
}
