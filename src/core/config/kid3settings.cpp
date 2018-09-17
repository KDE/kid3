/**
 * \file kid3settings.cpp
 * Wrapper for Qt application settings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Apr 2013
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

#include "kid3settings.h"
#include <QSettings>
#include <QStringList>

namespace {

void copyOldSettings(QSettings* config)
{
  if (!config->contains(QLatin1String("Tags/MarkTruncations"))) {
    // Configuration missing or not in current format
    QSettings oldSettings(
          QSettings::UserScope, QLatin1String("kid3.sourceforge.net"),
          QLatin1String("Kid3"));
    if (oldSettings.contains(QLatin1String("/kid3/General Options/ExportFormatIdx"))) {
      oldSettings.beginGroup(QLatin1String("/kid3"));
      const auto keys = oldSettings.allKeys();
      for (const QString& key : keys) {
        QString newKey(key);
        newKey.replace(QLatin1String("Recent Files"),
                       QLatin1String("RecentFiles"));
        config->setValue(newKey, oldSettings.value(key));
      }
      qDebug("Copied old settings");
    }
  }
}

}

/**
 * Constructor.
 */
Kid3Settings::Kid3Settings(QSettings* config) : m_config(config)
{
  copyOldSettings(m_config);
  migrateOldSettings();
}

/**
 * Destructor.
 */
Kid3Settings::~Kid3Settings()
{
}

/**
 * Use settings subgroup.
 * @param prefix group name
 */
void Kid3Settings::beginGroup(const QString& prefix)
{
  m_config->beginGroup(prefix);
}

/**
 * Finnish using settings subgroup.
 */
void Kid3Settings::endGroup()
{
  m_config->endGroup();
}

/**
 * Set value for setting.
 * @param key name of setting
 * @param value value for setting
 */
void Kid3Settings::setValue(const QString& key, const QVariant& value)
{
  m_config->setValue(key, value);
}

/**
 * Get value for setting.
 * @param key name of setting
 * @param defaultValue default value
 * @return value of setting as variant.
 */
QVariant Kid3Settings::value(const QString& key,
                             const QVariant& defaultValue) const
{
  return m_config->value(key, defaultValue);
}

/**
 * Remove setting.
 * @param key name of setting
 */
void Kid3Settings::remove(const QString& key)
{
  m_config->remove(key);
}

/**
 * Check if setting exists.
 * @param key name of setting
 * @return true if setting exists.
 */
bool Kid3Settings::contains(const QString& key) const
{
  return m_config->contains(key);
}

/**
 * Write unsaved changes to permanent storage.
 */
void Kid3Settings::sync()
{
  m_config->sync();
}
