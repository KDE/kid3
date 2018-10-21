/**
 * \file importparser.h
 * Import parser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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
#include <QRegularExpression>
#include <QMap>
#include <QList>
#include "kid3api.h"

class FrameCollection;

/**
 * Import parser.
 */
class KID3_CORE_EXPORT ImportParser {
public:
  /**
   * Constructor.
   */
  ImportParser();

  /**
   * Set import format.
   *
   * @param fmt format regexp
   * @param enableTrackIncr enable automatic track increment if no %t is found
   */
  void setFormat(const QString& fmt, bool enableTrackIncr = false);

  /**
   * Get next tags in text buffer.
   *
   * @param text text buffer containing data from file or clipboard
   * @param frames frames for output
   * @param pos  current position in buffer, will be updated to point
   *             behind current match (to be used for next call)
   * @return true if tags found (pos is index behind match).
   */
  bool getNextTags(const QString& text, FrameCollection& frames, int& pos);

  /**
   * Get list with track durations.
   *
   * @return list with track durations.
   */
  QList<int> getTrackDurations() const { return m_trackDuration; }

  /**
   * Get help text for format codes supported by setFormat().
   *
   * @return help text.
   */
  static QString getFormatToolTip();

private:
  /** track regexp pattern */
  QString m_pattern;
  /** regexp object */
  QRegularExpression m_re;
  /** automatically incremented track number */
  int m_trackIncrNr;
  QMap<QString, int> m_codePos;
  QList<int> m_trackDuration;
  /** true if automatic track number incrementing is used */
  bool m_trackIncrEnabled;
};
