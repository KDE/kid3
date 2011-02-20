/**
 * \file genres.h
 * Alphabetical list of genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#ifndef GENRES_H
#define GENRES_H

class QString;

/** Alphabetically sorted list of genres, conversion to/from genre numbers */
class Genres {
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
	 * Get the index in the alphabethically sorted list from the genre number.
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
	 * @param str genre string.
	 */
	static QString getNameString(const QString& str);

	/**
	 * Get a number representation of a genre name if possible.
	 *
	 * @param str         string with genre name
	 * @param parentheses true to put the number in parentheses
	 *
	 * @return genre string.
	 */
	static QString getNumberString(const QString& str, bool parentheses);

	/** Number of genres */
#if defined _WIN32 || defined WIN32
	enum { count = 148 };
#else
	static const int count = 148;
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

#endif /* GENRES_H */
