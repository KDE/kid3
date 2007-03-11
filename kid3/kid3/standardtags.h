/**
 * \file standardtags.h
 * Set of most used tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef STANDARDTAGS_H
#define STANDARDTAGS_H

#include <qstring.h>

/** Set of standard tags */
class StandardTags {
 public:
	/**
	 * Constructor.
	 */
	StandardTags();

	/**
	 * Set values which are different inactive.
	 *
	 * @param st tags to compare
	 */
	void filterDifferent(const StandardTags& st);

	/**
	 * Set tags inactive.
	 */
	void setInactive();

	/**
	 * Copy all tags which are not inactive.
	 *
	 * @param dest standard tags to copy into
	 */
	void copyActiveTags(StandardTags& dest) const;

	/**
	 * Set tags empty.
	 */
	void setEmpty();

	/**
	 * Copy tags which are empty or inactive from other tags.
	 * This can be used to merge two tags.
	 *
	 * @param st other tags
	 */
	void merge(const StandardTags& st);

	/**
	 * Format a string from tag data.
	 * Supported format fields:
	 * %s title (song)
	 * %l album
	 * %a artist
	 * %c comment
	 * %y year
	 * %t track, two digits, i.e. leading zero if < 10
	 * %T track, without leading zeroes
	 * %g genre
	 *
	 * @param format format specification
	 *
	 * @return formatted string.
	 */
	QString formatString(const QString& format) const;

	/**
	 * Replace escaped characters in a string.
	 *
	 * @param format string with escaped two-character-sequences
	 *               ("\n", "\t", "\r", "\\", "\a", "\b", "\f", "\v")
	 *
	 * @return string with escaped sequences replaced by characters.
	 */
	static QString replaceEscapedChars(const QString& format);

	/**
	 * Replace percent codes in a string.
	 *
	 * @param format string with percent codes
	 *               (% followed by a single character)
	 * @param codes        characters following percent
	 * @param replacements strings with replacements for codes
	 * @param numCodes     number of elements in codes and replacements
	 *
	 * @return string with percent codes replaced
	 */
	static QString replacePercentCodes(
		const QString& format, const QChar* codes,
		const QString* replacements, int numCodes);

	/** Title, empty if "", inactive if QString::null */
	QString title;
	/** Artist, empty if "", inactive if QString::null */
	QString artist;
	/** Album, empty if "", inactive if QString::null */
	QString album;
	/** Comment, empty if "", inactive if QString::null */
	QString comment;
	/** Year, empty if 0, inactive if -1 */
	int year;
	/** Track, empty if 0, inactive if -1 */
	int track;
	/** Genre, empty if 0xff, inactive if -1 */
	int genre;
	/** Genre without number, inactive if QString::null or genre != 0xff,
			empty if "" */
	QString genreStr;
};

/**
 * Filter to enable fields.
 */
class StandardTagsFilter {
public:
	/**
	 * If all fields are false set all true.
	 */
	void allFalseToAllTrue();

	/**
	 * Set all fields true.
	 */
	void setAllTrue();

  /**
   * Check if all fields are true.
   *
   * @return true if all fields are true.
   */
  bool areAllTrue() const;

	bool m_enableTitle;   /**< true if title enabled */
	bool m_enableArtist;  /**< true if artist enabled */
	bool m_enableAlbum;   /**< true if album enabled */
	bool m_enableComment; /**< true if comment enabled */
	bool m_enableYear;    /**< true if year enabled */
	bool m_enableTrack;   /**< true if track enabled */
	bool m_enableGenre;   /**< true if genre enabled */
};

#endif // STANDARDTAGS_H
