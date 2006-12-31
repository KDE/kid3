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
