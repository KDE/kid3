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
	void copyActiveTags(StandardTags &dest) const;
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
};

#endif // STANDARDTAGS_H
