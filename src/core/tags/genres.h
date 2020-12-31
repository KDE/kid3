/**
 * \file genres.h
 * Alphabetical list of genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#include <QtGlobal>
#include "kid3api.h"

class QString;

/** Alphabetically sorted list of genres, conversion to/from genre numbers */
class KID3_CORE_EXPORT Genres {
public:
  /**
   * Get name assigned to genre number.
   *
   * @param num genre number
   *
   * @return name, empty string for unknown number.
   */
  static const char* getName(int num);

  /**
   * Get the index in the alphabetically sorted list from the genre number.
   *
   * @param num genre number
   *
   * @return index, 0 for unknown number.
   */
  static int getIndex(int num);

  /**
   * Get the genre number from a string containing a genre text.
   *
   * @param str string with genre
   *
   * @return genre number, 255 for unknown index.
   */
  static int getNumber(const QString& str);

  /**
   * Get a name string from a string with a number or a name.
   * ID3v2 genres can be stored as "9", "(9)", "(9)Metal" or "Metal".
   *
   * @param str genre string, it can also reference multiple ID3v1 genres
   * and have a refinement such as "(9)(138)Viking Metal".
   * Multiple genres can be separated by Frame::stringListSeparator().
   *
   * @return genre name or multiple genre names separated by
   * Frame::stringListSeparator().
   */
  static QString getNameString(const QString& str);

  /**
   * Get a number representation of a genre name if possible.
   *
   * @param str string with genre name, can also contain multiple genres
   * separated by Frame::stringListSeparator()
   * @param parentheses true to put the numbers in parentheses, this will
   * result in an ID3v2.3.0 genre string, which can containing multiple
   * references to ID3v1 genres and optionally a refinement as a genre text
   *
   * @return genre string using numbers where possible. If @a parentheses
   * is true, an ID3v2.3.0 genre string such as "(9)(138)Viking Metal" is
   * returned, else if @a str contains multiple genres, they are returned
   * as numbers (where possible) separated by Frame::stringListSeparator().
   */
  static QString getNumberString(const QString& str, bool parentheses);

  /** Number of genres */
#ifdef Q_OS_WIN32
  enum { count = 192 };
#else
  static const int count = 192;
#endif
  /**
   * Pointer to alphabetic list of genres.
   * NULL terminated, to be used in combo box.
   */
  static const char** s_strList;

private:
  /**
   * Alphabetic list of genres, starts with unknown (empty) entry.
   *
   * 125: Last ID3v1, 142: WinAmp 1.91, 145: WinAmp 1.92, 255: unknown
   */
  static const char* s_genre[Genres::count + 3];
  /**
   * s_genreNum[n] gives the number of the n-th genre
   * in the alphabetically sorted list.
   */
  static const unsigned char s_genreNum[Genres::count + 1];
};
