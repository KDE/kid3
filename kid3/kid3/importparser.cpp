/**
 * \file importparser.cpp
 * Import parser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "standardtags.h"
#include "genres.h"
#include "importparser.h"

/**
 * Set import format.
 *
 * @param fmt format regexp
 * @param enableTrackIncr enable automatic track increment if no %t is found
 */
void ImportParser::setFormat(const QString &fmt, bool enableTrackIncr)
{
	int percentIdx = 0, nr = 1, lastIdx = fmt.length() - 1;
	pattern = fmt;
	titlePos = albumPos = artistPos = commentPos = yearPos = trackPos =
		genrePos = -1;
	while (((percentIdx = pattern.find('%', percentIdx)) >= 0) &&
		   (percentIdx < lastIdx)) {
		switch (pattern[percentIdx + 1].latin1()) {
			case 's':
				titlePos = nr;
				break;
			case 'l':
				albumPos = nr;
				break;
			case 'a':
				artistPos = nr;
				break;
			case 'c':
				commentPos = nr;
				break;
			case 'y':
				yearPos = nr;
				break;
			case 't':
				trackPos = nr;
				break;
			case 'g':
				genrePos = nr;
				break;
			default:
				++percentIdx;
				continue;
		}
		++nr;
		percentIdx += 2;
	}
	if (enableTrackIncr && trackPos == -1) {
		trackIncrEnabled = true;
		trackIncrNr = 1;
	} else {
		trackIncrEnabled = false;
		trackIncrNr = 0;
	}
#if QT_VERSION >= 300
	pattern.remove(QRegExp("%[slacytg]"));
#else
	pattern.replace(QRegExp("%[slacytg]"), "");
#endif
	re.setPattern(pattern);
}

/** Get ID3 text field if it is in format */
#define SET_TEXT_FIELD(name) \
if (name##Pos != -1) st.name = re.cap(name##Pos)

/** Get ID3 integer field if it is in format */
#define SET_INT_FIELD(name) \
if (name##Pos != -1) st.name = re.cap(name##Pos).toInt()

/**
 * Get next tags in text buffer.
 *
 * @param text text buffer containing data from file or clipboard
 * @param st   standard tags for output
 * @param pos  current position in buffer, will be updated to point
 *             behind current match (to be used for next call)
 * @return true if tags found (pos is index behind match).
 */
bool ImportParser::getNextTags(const QString &text, StandardTags &st, int &pos)
{
#if QT_VERSION >= 300
	int matchpos, oldpos = pos;
	if (pattern.isEmpty()) {
		return false;
	}
	if ((matchpos = re.search(text, pos, QRegExp::CaretAtOffset)) != -1) {
		SET_TEXT_FIELD(title);
		SET_TEXT_FIELD(album);
		SET_TEXT_FIELD(artist);
		SET_TEXT_FIELD(comment);
		SET_INT_FIELD(year);
		SET_INT_FIELD(track);
		if (trackIncrEnabled) {
			st.track = trackIncrNr++;
		}
		if (genrePos != -1) {
			QString genreStr = re.cap(genrePos);
			int genre = Genres::getNumber(genreStr);
			if (genre != 0xff) st.genre = genre;
		}
		pos = matchpos + re.matchedLength();
		if (pos > oldpos) { /* avoid endless loop */
			return true;
		}
	}
	return false;
#else
	/* no regexp capture in old Qt versions */
	int idx, oldpos = pos;
	/* hack: use yearPos to determine whether header or track parsing
	   is required */
	if (yearPos == -1) {
		/* track parsing */
		idx = QRegExp("[\r\n]\\s*\\d+[.\\s]").match(text, pos);
		if (idx == -1) return false;
		int trackLen, spaceLen, titleLen, titleIdx;
		int trackIdx = QRegExp("\\d+").match(text, idx + 1, &trackLen, true);
		if (trackIdx == -1) return false;
		if (!((QRegExp("^[.\\s]\\s*\\d+:\\d+\\s+").match(text, trackIdx + trackLen, &spaceLen, true) != -1) ||
			  (QRegExp("^[.\\s]\\s*").match(text, trackIdx + trackLen, &spaceLen, true) != -1)))
			return false;
		titleIdx = QRegExp("^[^\\s][^\\r\\n]*[^\\s]").match(text, trackIdx + trackLen + spaceLen, &titleLen, true);
		if (titleIdx == -1) return false;
		st.track = text.mid(trackIdx, trackLen).toInt();
		st.title = text.mid(titleIdx, titleLen);
		pos = titleIdx + titleLen;
	} else {
		/* header parsing */
		/* I know, this is done rather complicated, but I only use QRegExp of
		   Qt 2.3, so that it runs also on Windows. */
		int len, artistIdx, artistLen, albumIdx, albumLen;
		idx = QRegExp("[^\\s][^\\r\\n/]+\\s*/\\s*[^\\r\\n]*[^\\s][\\r\\n]+\\s*tracks:\\s+\\d+").match(text, pos);
		if (idx == -1) return false;

		artistIdx = QRegExp("^[^\\s][^\\r\\n/]+\\s*/").match(text, idx, &artistLen, true);
		if (artistIdx == -1) return false;

		albumIdx = QRegExp("^\\s*[^\\r\\n]*[^\\s][\\r\\n]").match(text, artistIdx + artistLen, &albumLen, true);
		if (albumIdx == -1) return false;
		pos = albumIdx + albumLen;
		st.artist = text.mid(artistIdx, artistLen - 1).stripWhiteSpace();
		st.album = text.mid(albumIdx, albumLen - 1).stripWhiteSpace();

		idx = QRegExp("year:\\s*\\d+").match(text, pos, &len);
		if (idx != -1) {
			bool ok;
			int year = text.mid(idx + 5, len - 5).stripWhiteSpace().toInt(&ok);
			if (ok) st.year = year;
			pos = idx + len;
		}

		idx = QRegExp("genre:\\s*[^\\r\\n]+[\\r\\n]").match(text, pos, &len);
		if (idx != -1) {
			QString genreStr(text.mid(idx + 6, len - 7).stripWhiteSpace());
			int genre = Genres::getNumber(genreStr);
			if (genre != 0xff) st.genre = genre;
			pos = idx + len;
		}
	}
	if (pos > oldpos) { /* avoid endless loop */
		return true;
	}
	return false;
#endif
}
