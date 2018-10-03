/**
 * \file formatreplacer.h
 * Replaces format codes in a string.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Jul 2008
 *
 * Copyright (C) 2008-2013  Urs Fleisch
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

#ifndef FORMATREPLACER_H
#define FORMATREPLACER_H

#include <QString>
#include "kid3api.h"

/**
 * Replaces format codes in a string.
 */
class KID3_CORE_EXPORT FormatReplacer {
public:
  /** Flags for replacePercentCodes(). */
  enum FormatStringFlags {
    FSF_SupportUrlEncode  = (1 << 0),
    FSF_ReplaceSeparators = (1 << 1),
    FSF_SupportHtmlEscape = (1 << 2)
  };

  /**
   * Constructor.
   *
   * @param str string with format codes
   */
  explicit FormatReplacer(const QString& str = QString());

  /**
   * Destructor.
   */
  virtual ~FormatReplacer() = default;

  /**
   * Set string with format codes.
   * @param str string with format codes
   */
  void setString(const QString& str) { m_str = str; }

  /**
   * Get string.
   * The string set with setString() can be modified using
   * replaceEscapedChars() and replacePercentCodes().
   * @return string.
   */
  QString getString() const { return m_str; }

  /**
   * Replace escaped characters.
   * Replaces the escaped characters ("\n", "\t", "\r", "\\", "\a", "\b",
   * "\f", "\v") with the corresponding characters.
   */
  void replaceEscapedChars();

  /**
   * Replace percent codes.
   *
   * @param flags flags: FSF_SupportUrlEncode to support modifier u
   *              (with code c "%uc") to URL encode,
   *              FSF_ReplaceSeparators to replace directory separators
   *              ('/', '\\', ':') in tags,
   *              FSF_SupportHtmlEscape to support modifier h
   *              (with code c "%hc") to replace HTML metacharacters
   *              ('<', '>', '&', '"', ''', non-ascii) in tags.
   */
  void replacePercentCodes(unsigned flags = 0);

  /**
   * Converts the plain text string @a plain to a HTML string with
   * HTML metacharacters replaced by HTML entities.
   * @param plain plain text
   * @return html text with HTML entities.
   */
  static QString escapeHtml(const QString& plain);

protected:
  /**
   * Replace a format code (one character %c or multiple characters %{chars}).
   *
   * @param code format code
   *
   * @return replacement string,
   *         QString::null if code not found.
   */
  virtual QString getReplacement(const QString& code) const = 0;

private:
  QString m_str;
};

#endif // FORMATREPLACER_H
