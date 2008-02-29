/**
 * \file standardtags.h
 * Set of most used tags.
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

#ifndef STANDARDTAGS_H
#define STANDARDTAGS_H

#include <qstring.h>

class QStringList;

/** Set of standard tags */
class StandardTags {
 public:
	/** Flags for formatString(), replacePercentCodes(). */
	enum FormatStringFlags {
		FSF_SupportUrlEncode  = (1 << 0),
		FSF_ReplaceSeparators = (1 << 1)
	};

	/** Flags for truncation. */
	enum TruncationFlag {
		TF_Title   = (1 << 0),
		TF_Artist  = (1 << 1),
		TF_Album   = (1 << 2),
		TF_Comment = (1 << 3),
		TF_Year    = (1 << 4),
		TF_Track   = (1 << 5),
		TF_Genre   = (1 << 6)
	};

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
	 * Check if the tags are empty or inactive.
	 *
	 * @return true if empty or inactive.
	 */
	bool isEmptyOrInactive() const;

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
	 * @param flags  flags: FSF_SupportUrlEncode to support modifier u
	 *               (with code c "%uc") to URL encode,
	 *               FSF_ReplaceSeparators to replace directory separators
	 *               ('/', '\\', ':') in tags.
	 *
	 * @return formatted string.
	 */
	QString formatString(const QString& format, unsigned flags = 0) const;

	/**
	 * Get help text for format codes supported by formatString().
	 *
	 * @param onlyRows if true only the tr elements are returned,
	 *                 not the surrounding table
	 *
	 * @return help text.
	 */
	static QString getFormatToolTip(bool onlyRows = false);

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
	 *               (% followed by a single character or %{word})
	 * @param shortCodes   characters following percent
	 * @param longCodes    code words in braces following percent
	 * @param replacements strings with replacements for codes
	 * @param numCodes     number of elements in codes and replacements
	 * @param flags        flags: FSF_SupportUrlEncode to support modifier u
	 *                     (with code c "%uc") to URL encode,
	 *                     FSF_ReplaceSeparators to replace directory separators
	 *                     ('/', '\\', ':') in tags.
	 *
	 * @return string with percent codes replaced
	 */
	static QString replacePercentCodes(
		const QString& format, const QChar* shortCodes,
		const QStringList& longCodes,
		const QString* replacements, int numCodes,
		unsigned flags = 0);

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
	/** Genre, inactive if QString::null, empty if "" */
	QString genre;
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
