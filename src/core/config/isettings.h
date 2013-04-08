/**
 * \file isettings.h
 * Interface for application settings.
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

#ifndef ISETTINGS_H
#define ISETTINGS_H

#include <QVariant>
#include "kid3api.h"

class QString;

/**
 * Interface for application settings.
 */
class KID3_CORE_EXPORT ISettings {
public:
  /**
   * Destructor.
   */
  virtual ~ISettings() = 0;

  /**
   * Use settings subgroup.
   * @param prefix group name
   */
  virtual void beginGroup(const QString& prefix) = 0;

  /**
   * Finnish using settings subgroup.
   */
  virtual void endGroup() = 0;

  /**
   * Set value for setting.
   * @param key name of setting
   * @param value value for setting
   */
  virtual void setValue(const QString& key, const QVariant& value) = 0;

  /**
   * Get value for setting.
   * @param key name of setting
   * @param defaultValue default value
   * @return value of setting as variant.
   */
  virtual QVariant value(const QString& key,
                         const QVariant& defaultValue) const = 0;

  /**
   * Remove setting.
   * @param key name of setting
   */
  virtual void remove(const QString& key) = 0;

  /**
   * Check if setting exists.
   * @param key name of setting
   * @return true if setting exists.
   */
  virtual bool contains(const QString& key) const = 0;

  /**
   * Write unsaved changes to permanent storage.
   */
  virtual void sync() = 0;
};

#endif // ISETTINGS_H
