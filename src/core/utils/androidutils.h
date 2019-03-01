/**
 * \file androidutils.h
 * Platform utility functions for Android.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 27 Feb 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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

#include <QObject>
#include "kid3api.h"

/**
 * Communication with the Java part of the Android app.
 * A single instance of this class shall be created and then be accessed
 * using the instance() method.
 */
class KID3_CORE_EXPORT AndroidUtils : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit AndroidUtils(QObject* parent);

  /**
   * Check if an intent was received before the application was ready.
   */
  void checkPendingIntents();

  /**
   * Emit signal when a view or edit intent to open a file is received.
   * @param path path of file
   */
  void emitFilePathReceived(const QString& path);

  /**
   * Get a pointer to the application's config store instance.
   * @return config store, 0 if no instance has been allocated.
   */
  static AndroidUtils* instance() { return s_self; }

signals:
  /**
   * Emitted when a view or edit intent to open a file is received.
   * @param path path to file
   */
  void filePathReceived(const QString& path);

private:
  static AndroidUtils* s_self;
};
