/**
 * \file standardtags.cpp
 * Set of most used tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "standardtags.h"

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
