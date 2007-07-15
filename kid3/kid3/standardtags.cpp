/**
 * \file standardtags.cpp
 * Set of most used tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "standardtags.h"
#include "genres.h"
#include <qurl.h>
#include "qtcompatmac.h"

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
		genre = QString::null;
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
	year = track = -1;
	genre = QString::null;
}

/**
 * Copy all tags which are not inactive.
 *
 * @param dest standard tags to copy into
 */
void StandardTags::copyActiveTags(StandardTags& dest) const
{
	if (title != QString::null) dest.title = title;
	if (artist != QString::null) dest.artist = artist;
	if (album != QString::null) dest.album = album;
	if (comment != QString::null) dest.comment = comment;
	if (year != -1) dest.year = year;
	if (track != -1) dest.track = track;
	if (genre != QString::null) dest.genre = genre;
}

/**
 * Set tags empty.
 */
void StandardTags::setEmpty()
{
	title = "";
	artist = "";
	album = "";
	comment = "";
	year = 0;
	track = 0;
	genre = "";
}

/**
 * Copy tags which are empty or inactive from other tags.
 * This can be used to merge two tags.
 *
 * @param st other tags
 */
void StandardTags::merge(const StandardTags& st)
{
	if (title.isEmpty())   title = st.title;
	if (artist.isEmpty())  artist = st.artist;
	if (album.isEmpty())   album = st.album;
	if (comment.isEmpty()) comment = st.comment;
	if (year <= 0)         year = st.year;
	if (track <= 0)        track = st.track;
	if (genre.isEmpty())   genre = st.genre;
}

/**
 * Check if the tags are empty or inactive.
 *
 * @return true if empty or inactive.
 */
bool StandardTags::isEmptyOrInactive() const
{
return 
	title.isEmpty() &&
	artist.isEmpty() &&
	album.isEmpty() && 
	comment.isEmpty() &&
	year <= 0 &&
	track <= 0 &&
	genre.isEmpty();
}

/**
 * Replace escaped characters in a string.
 *
 * @param format string with escaped two-character-sequences
 *               ("\n", "\t", "\r", "\\", "\a", "\b", "\f", "\v")
 *
 * @return string with escaped sequences replaced by characters.
 */
QString StandardTags::replaceEscapedChars(const QString& format)
{
	QString fmt(format);
	if (!fmt.isEmpty()) {
		const int numEscCodes = 8;
		const QChar escCode[numEscCodes] = {
			'n', 't', 'r', '\\', 'a', 'b', 'f', 'v'};
		const char escChar[numEscCodes] = {
			'\n', '\t', '\r', '\\', '\a', '\b', '\f', '\v'};

		for (int pos = 0; pos < static_cast<int>(fmt.length());) {
			pos = fmt.QCM_indexOf('\\', pos);
			if (pos == -1) break;
			++pos;
			for (int k = 0;; ++k) {
				if (k >= numEscCodes) {
					// invalid code at pos
					++pos;
					break;
				}
				if (fmt[pos] == escCode[k]) {
					// code found, replace it
					fmt.replace(pos - 1, 2, escChar[k]);
					break;
				}
			}
		}
	}
	return fmt;
}

/**
 * Replace percent codes in a string.
 *
 * @param format string with percent codes
 *               (% followed by a single character)
 * @param codes        characters following percent
 * @param replacements strings with replacements for codes
 * @param numCodes     number of elements in codes and replacements
 * @param flags        flags: FSF_SupportUrlEncode to support modifier u
 *                     (with code c "%uc") to URL encode
 *
 * @return string with percent codes replaced
 */
QString StandardTags::replacePercentCodes(
	const QString& format, const QChar* codes,
	const QString* replacements, int numCodes,
	unsigned flags)
{
	QString fmt(format);
	if (!fmt.isEmpty()) {
		for (int pos = 0; pos < static_cast<int>(fmt.length());) {
			pos = fmt.QCM_indexOf('%', pos);
			if (pos == -1) break;

			for (int k = 0;; ++k) {
				if (k >= numCodes) {
					// invalid code at pos
					++pos;
					break;
				}
				if ((flags & FSF_SupportUrlEncode) &&
						fmt[pos + 1] == 'u' && fmt[pos + 2] == codes[k]) {
					// code found, replace it URL encoded
					QString enc = replacements[k];
					QCM_QUrl_encode(enc);
					fmt.replace(pos, 3, enc);
					pos += enc.length();
					break;
				}
				if (fmt[pos + 1] == codes[k]) {
					// code found, replace it
					fmt.replace(pos, 2, replacements[k]);
					pos += replacements[k].length();
					break;
				}
			}
		}
	}
	return fmt;
}

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
 *               (with code c "%uc") to URL encode
 *
 * @return formatted string.
 */
QString StandardTags::formatString(const QString& format, unsigned flags) const
{
	if (!format.isEmpty()) {
		const int numTagCodes = 8;
		const QChar tagCode[numTagCodes] = {
	    's', 'l', 'a', 'c', 'y', 't', 'T', 'g'
		};
		QString tagStr[numTagCodes];

		QString yearStr, trackStr;
		yearStr.sprintf("%d", year);
		trackStr.sprintf("%02d", track);
		tagStr[0] = title;
		tagStr[1] = album;
		tagStr[2] = artist;
		tagStr[3] = comment;
		tagStr[4] = yearStr;
		tagStr[5] = trackStr;
		tagStr[6] = QString::number(track);
		tagStr[7] = genre;

		return replacePercentCodes(format, tagCode, tagStr, numTagCodes, flags);
	}
	return format;
}


/**
 * If all fields are false set all true.
 */
void StandardTagsFilter::allFalseToAllTrue()
{
	if (!m_enableTitle && !m_enableArtist && !m_enableAlbum &&
			!m_enableComment && !m_enableYear && !m_enableTrack && !m_enableGenre) {
		setAllTrue();
	}
}

/**
 * Set all fields true.
 */
void StandardTagsFilter::setAllTrue()
{
	m_enableTitle = true;
	m_enableArtist = true;
	m_enableAlbum = true;
	m_enableComment = true;
	m_enableYear = true;
	m_enableTrack = true;
	m_enableGenre = true;
}

/**
 * Check if all fields are true.
 *
 * @return true if all fields are true.
 */
bool StandardTagsFilter::areAllTrue() const
{
	return m_enableTitle && m_enableArtist && m_enableAlbum &&
		m_enableComment && m_enableYear && m_enableTrack && m_enableGenre;
}
