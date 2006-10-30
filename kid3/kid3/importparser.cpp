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
		genrePos = durationPos = -1;
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
			case 'd':
				durationPos = nr;
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
#if QT_VERSION >= 0x030100
	pattern.remove(QRegExp("%[slacytgd]"));
#else
	pattern.replace(QRegExp("%[slacytgd]"), "");
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
	int idx, oldpos = pos;
	if (pattern.isEmpty()) {
		trackDuration.clear();
		return false;
	}
	if (durationPos == -1) {
		trackDuration.clear();
	} else if (pos == 0) {
		trackDuration.clear();
		int dsp = 0; // "duration search pos"
		int lastDsp = dsp;
		while ((idx = re.search(text, dsp)) != -1) {
			QString durationStr = re.cap(durationPos);
			int duration;
			QRegExp durationRe("(\\d+):(\\d+)");
			if (durationRe.search(durationStr) != -1) {
				duration = durationRe.cap(1).toInt() * 60 +
					durationRe.cap(2).toInt();
			} else {
				duration = durationStr.toInt();
			}
			trackDuration.append(duration);

			dsp = idx + re.matchedLength();
			if (dsp > lastDsp) { /* avoid endless loop */
				lastDsp = dsp;
			} else {
				break;
			}
		}
	}
	if ((idx = re.search(text, pos)) != -1) {
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
			if (genre != 0xff) {
				st.genre = genre;
				st.genreStr = QString::null;
			} else if (!genreStr.isEmpty()) {
				st.genre = 0xff;
				st.genreStr = genreStr;
			}
		}
		pos = idx + re.matchedLength();
		if (pos > oldpos) { /* avoid endless loop */
			return true;
		}
	}
	return false;
}
