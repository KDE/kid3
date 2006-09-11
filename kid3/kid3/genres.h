/**
 * \file genres.h
 * Alphabetical list of genres.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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
	static const char *getName(int num);
	/**
	 * Get the index in the alphabethically sorted list from the genre number.
	 *
	 * @param num genre number
	 *
	 * @return index, 0 for unknown number.
	 */
	static int getIndex(int num);
	/**
	 * Get the genre number from the index in the alphabethically sorted list.
	 *
	 * @param index index in alphabethically sorted list
	 *
	 * @return genre number, 255 for unknown index.
	 */
	static int getNumber(int index);
	/**
	 * Get the genre number from a string containing a genre text.
	 *
	 * @param str   string with genre
	 *
	 * @return genre number, 255 for unknown index.
	 */
	static int getNumber(const QString &str);
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
	static const char **strList;

 private:
	/**
	 * Alphabetic list of genres, starts with unknown (empty) entry.
	 *
	 * 125: Last ID3v1, 142: WinAmp 1.91, 145: WinAmp 1.92, 255: unknown
	 */
	static const char *genre[Genres::count + 2];
	/**
	 * genre_num[n] gives the number of the n-th genre
	 * in the alphabetically sorted list.
	 */
	static const unsigned char genre_num[Genres::count + 1];
};

#endif /* GENRES_H */
