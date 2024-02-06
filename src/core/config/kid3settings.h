/**
 * \file kid3settings.h
 * Wrapper for Qt application settings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Apr 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#pragma once

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
  ~Kid3Settings() override = default;

  /**
   * Use settings subgroup.
   * @param prefix group name
   * @param forState true if this group stores state information
   */
  void beginGroup(const QString& prefix, bool forState = false) override;

  /**
   * Finnish using settings subgroup.
   */
  void endGroup() override;

  /**
   * Set value for setting.
   * @param key name of setting
   * @param value value for setting
   */
  void setValue(const QString& key, const QVariant& value) override;

  /**
   * Get value for setting.
   * @param key name of setting
   * @param defaultValue default value
   * @return value of setting as variant.
   */
  QVariant value(const QString& key,
                 const QVariant& defaultValue) const override;

  /**
   * Remove setting.
   * @param key name of setting
   */
  void remove(const QString& key) override;

  /**
   * Check if setting exists.
   * @param key name of setting
   * @return true if setting exists.
   */
  bool contains(const QString& key) const override;

  /**
   * Write unsaved changes to permanent storage.
   */
  void sync() override;

private:
  QSettings* m_config;
};
