/**
 * \file commandformatreplacer.h
 * Replaces context command format codes in a string.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Aug 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef COMMANDFORMATREPLACER_H
#define COMMANDFORMATREPLACER_H

#include "frame.h"

/**
 * Replaces context command format codes in a string.
 */
class KID3_CORE_EXPORT CommandFormatReplacer : public FrameFormatReplacer {
public:
  /**
   * Constructor.
   *
   * @param frames frame collection
   * @param str    string with format codes
   * @param files  file list
   * @param isDir  true if directory
   */
  explicit CommandFormatReplacer(
    const FrameCollection& frames, const QString& str,
    const QStringList& files, bool isDir);

  /**
   * Destructor.
   */
  virtual ~CommandFormatReplacer() override = default;

  /**
   * Get help text for supported format codes.
   *
   * @param onlyRows if true only the tr elements are returned,
   *                 not the surrounding table
   *
   * @return help text.
   */
  static QString getToolTip(bool onlyRows = false);

protected:
  /**
   * Replace a format code (one character %c or multiple characters %{chars}).
   * Supported format fields:
   * Those supported by FrameFormatReplacer::getReplacement()
   * %f %{file} filename
   * %d %{directory} directory name
   * %b %{browser} the web browser set in the configuration
   * %q %{qmlpath} base directory for QML files
   *
   * @param code format code
   *
   * @return replacement string,
   *         QString::null if code not found.
   */
  virtual QString getReplacement(const QString& code) const override;

private:
  const QStringList& m_files;
  const bool m_isDir;
};

#endif // COMMANDFORMATREPLACER_H
