/**
 * \file dummysettings.cpp
 * Application settings stub for tests.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03 Jun 2013
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

#include "dummysettings.h"

/**
 * Constructor.
 */
DummySettings::DummySettings()
{
}

/**
 * Use settings subgroup.
 * @param prefix group name
 */
void DummySettings::beginGroup(const QString& prefix)
{
  Q_UNUSED(prefix)
}

/**
 * Finnish using settings subgroup.
 */
void DummySettings::endGroup()
{
}

/**
 * Set value for setting.
 * @param key name of setting
 * @param value value for setting
 */
void DummySettings::setValue(const QString& key, const QVariant& value)
{
  Q_UNUSED(key)
  Q_UNUSED(value)
}

/**
 * Get value for setting.
 * @param key name of setting
 * @param defaultValue default value
 * @return value of setting as variant.
 */
QVariant DummySettings::value(const QString& key,
                              const QVariant& defaultValue) const
{
  Q_UNUSED(key)
  return defaultValue;
}

/**
 * Remove setting.
 * @param key name of setting
 */
void DummySettings::remove(const QString& key)
{
  Q_UNUSED(key)
}

/**
 * Check if setting exists.
 * @param key name of setting
 * @return true if setting exists.
 */
bool DummySettings::contains(const QString& key) const
{
  Q_UNUSED(key)
  return false;
}

/**
 * Write unsaved changes to permanent storage.
 */
void DummySettings::sync()
{
}
