/**
 * \file generalconfig.h
 * General configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#ifndef GENERALCONFIG_H
#define GENERALCONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "isettings.h"
#include "configstore.h"

/**
 * Abstract base class for configurations.
 */
class KID3_CORE_EXPORT GeneralConfig : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp configuration group
   */
  explicit GeneralConfig(const QString& grp);

  /**
   * Destructor.
   */
  virtual ~GeneralConfig() override;

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  virtual void writeToConfig(ISettings* config) const = 0;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(ISettings* config) = 0;

  /**
   * String list of available text codecs.
   *
   * @return list of codec names.
   */
  Q_INVOKABLE static QStringList getTextCodecNames();

protected:
  /**
   * Convert list of integers to list of strings.
   * @param intList list of integers
   * @return list of strings.
   */
  static QStringList intListToStringList(const QList<int>& intList);

  /**
   * Convert list of strings to list of integers.
   * @param strList list of strings
   * @return list of integers.
   */
  static QList<int> stringListToIntList(const QStringList& strList);

  /**
   * Remove aliases in braces from text encoding name.
   *
   * @param comboEntry text encoding name
   *
   * @return codec name.
   */
  static QString getTextCodecName(const QString& comboEntry);

  /**
   * Get index of text encoding in getTextCodecNames().
   * @param textEncoding text encoding name
   * @return index of encoding.
   */
  static int indexFromTextCodecName(const QString& textEncoding);

  /**
   * Get text encoding name from index in getTextCodecNames().
   * @param index index of encoding
   * @return text encoding name, null if index invalid.
   */
  static QString indexToTextCodecName(int index);

  /** Configuration group. */
  QString m_group;
};

/**
 * Template to inject a static instance() method into a configuration class.
 * This is an application of the "curiously recurring template pattern" so that
 * the instance() method returns the type of the derived class.
 * A typical usage is
 * @code
 * class SpecializedConfig : public StoredConfig<SpecializedConfig> {
 * public:
 *   explicit SpecializedConfig(const QString& grp) : StoredConfig(grp) {
 *     (..)
 *   }
 * };
 * @endcode
 *
 * SpecializedConfig::instance() returns a reference to a stored instance
 * of this class. There can only be one such instance per class.
 *
 * @tparam Derived derived class
 * @tparam Base base class, default is GeneralConfig
 */
template <class Derived, class Base = GeneralConfig>
class StoredConfig : public Base {
public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp configuration group
   */
  explicit StoredConfig(const QString& grp) : Base(grp) {}

  /**
   * Get stored instance of class.
   *
   * @return instance.
   */
  static Derived& instance();
};

template <class Derived, class Base>
Derived& StoredConfig<Derived, Base>::instance() {
  Derived* obj = nullptr;
  ConfigStore* store = ConfigStore::instance();
  if (Derived::s_index >= 0) {
    obj = static_cast<Derived*>(store->configuration(Derived::s_index));
  } else {
    obj = new Derived;
    obj->setParent(store);
    Derived::s_index = store->addConfiguration(obj);
  }
  return *obj;
}

#endif
