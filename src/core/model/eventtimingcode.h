/**
 * \file eventtimingcode.h
 * Event timing code to string conversion.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include <QString>
#include <QStringList>
#include "kid3api.h"

/**
 * Event timing code.
 */
class KID3_CORE_EXPORT EventTimeCode {
public:
  /**
   * Constructor.
   * @param code code ID3v2 ETCO code
   */
  explicit EventTimeCode(int code) : m_code(code) {}

  /**
   * Get code.
   * @return code.
   */
  int getCode() const { return m_code; }

  /**
   * Check if code is valid.
   * @return true if valid.
   */
  bool isValid() const { return m_code != -1; }

  /**
   * Get string representation.
   * @return code description.
   */
  QString toString() const;

  /**
   * Get translated string representation.
   * @return code description.
   */
  QString toTranslatedString() const;

  /**
   * Get index of code in list of strings.
   * @return index.
   */
  int toIndex() const;

  /**
   * Create from string.
   * @param str untranslated string
   * @return event time code.
   */
  static EventTimeCode fromString(const char* str);

  /**
   * Create from index.
   * @param index index
   * @return event time code.
   */
  static EventTimeCode fromIndex(int index);

  /**
   * Get list of translated strings.
   * @return code descriptions.
   */
  static QStringList getTranslatedStrings();

private:
  int m_code;
};
