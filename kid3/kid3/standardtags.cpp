/**
 * \file standardtags.cpp
 * Set of most used tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "standardtags.h"

StandardTags::StandardTags()
{
	setInactive();
}

/**
 * Set values which are different inactive.
 *
 * @param st tags to compare
 */

void StandardTags::filterDifferent(const StandardTags& st)
{
	if (title != st.title) {
		title = QString::null;
	}
	if (artist != st.artist) {
		artist = QString::null;
	}
	if (album != st.album) {
		album = QString::null;
	}
	if (comment != st.comment) {
		comment = QString::null;
	}
	if (year != st.year) {
		year = -1;
	}
	if (track != st.track) {
		track = -1;
	}
	if (genre != st.genre) {
		genre = -1;
	}
}

/**
 * Set tags inactive.
 */

void StandardTags::setInactive()
{
	title = QString::null;
	artist = QString::null;
	album = QString::null;
	comment = QString::null;
	year = track = genre = -1;
}

/**
 * Copy all tags which are not inactive.
 *
 * @param dest standard tags to copy into
 */
void StandardTags::copyActiveTags(StandardTags &dest) const
{
	if (title != QString::null) dest.title = title;
	if (artist != QString::null) dest.artist = artist;
	if (album != QString::null) dest.album = album;
	if (comment != QString::null) dest.comment = comment;
	if (year != -1) dest.year = year;
	if (track != -1) dest.track = track;
	if (genre != -1) dest.genre = genre;

}

/**
 * Set tags empty.
 */

void StandardTags::setEmpty(void)
{
	title = "";
	artist = "";
	album = "";
	comment = "";
	year = 0;
	track = 0;
	genre = 0xff;
}
