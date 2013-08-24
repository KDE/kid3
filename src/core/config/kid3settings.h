/**
 * \file kid3settings.h
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

#ifndef KID3SETTINGS_H
#define KID3SETTINGS_H

#include "isettings.h"

class QSettings;

/**
 * Wrapper for KDE application settings.
 */
class KID3_CORE_EXPORT Kid3Settings : public ISettings {
public:
  /**
   * Constructor.
   * @param config Qt settings
   */
  explicit Kid3Settings(QSettings* config);

  /**
   * Destructor.
   */
  virtual ~Kid3Settings();

  /**
   * Use settings subgroup.
   * @param prefix group name
   */
  virtual void beginGroup(const QString& prefix);

  /**
   * Finnish using settings subgroup.
   */
  virtual void endGroup();

  /**
   * Set value for setting.
   * @param key name of setting
   * @param value value for setting
   */
  virtual void setValue(const QString& key, const QVariant& value);

  /**
   * Get value for setting.
   * @param key name of setting
   * @param defaultValue default value
   * @return value of setting as variant.
   */
  virtual QVariant value(const QString& key,
                         const QVariant& defaultValue) const;

  /**
   * Remove setting.
   * @param key name of setting
   */
  virtual void remove(const QString& key);

  /**
   * Check if setting exists.
   * @param key name of setting
   * @return true if setting exists.
   */
  virtual bool contains(const QString& key) const;

  /**
   * Write unsaved changes to permanent storage.
   */
  virtual void sync();

private:
  QSettings* m_config;
};

#endif // KID3SETTINGS_H
