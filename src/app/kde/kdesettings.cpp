/**
 * \file kdesettings.cpp
 * Wrapper for KDE application settings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Apr 2013
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

#include "kdesettings.h"
#include <QtConfig>
#include <KConfigGroup>

/**
 * Constructor.
 * @param config KDE settings
 * @param stateConfig state information
 */
KdeSettings::KdeSettings(KSharedConfigPtr config, KSharedConfigPtr stateConfig)
  : m_config(config), m_stateConfig(stateConfig)
{
  migrateOldSettings();
}

/**
 * Destructor.
 */
KdeSettings::~KdeSettings()
{
  // Must not be inline because of forward declared QScopedPointer.
}

/**
 * Use settings subgroup.
 * @param prefix group name
 * @param forState true if this group stores state information
 */
void KdeSettings::beginGroup(const QString& prefix, bool forState)
{
  m_group.reset(new KConfigGroup(forState ? m_stateConfig : m_config, prefix));
}

/**
 * Finnish using settings subgroup.
 */
void KdeSettings::endGroup()
{
  m_group.reset();
}

/**
 * Set value for setting.
 * @param key name of setting
 * @param value value for setting
 */
void KdeSettings::setValue(const QString& key, const QVariant& value)
{
  if (m_group) {
    m_group->writeEntry(key, value);
  }
}

/**
 * Get value for setting.
 * @param key name of setting
 * @param defaultValue default value
 * @return value of setting as variant.
 */
QVariant KdeSettings::value(const QString& key,
                            const QVariant& defaultValue) const
{
  if (m_group) {
    return m_group->readEntry(key, defaultValue);
  }
  return QVariant();
}

/**
 * Remove setting.
 * @param key name of setting
 */
void KdeSettings::remove(const QString& key)
{
  if (m_group) {
    m_group->deleteEntry(key);
  }
}

/**
 * Check if setting exists.
 * @param key name of setting
 * @return true if setting exists.
 */
bool KdeSettings::contains(const QString& key) const
{
  if (m_group) {
    return m_group->hasKey(key);
  }
  return false;
}

/**
 * Write unsaved changes to permanent storage.
 */
void KdeSettings::sync()
{
  m_config->sync();
  m_stateConfig->sync();
}
