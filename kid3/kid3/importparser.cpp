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
void ImportParser::setFormat(const QString& fmt, bool enableTrackIncr)
{
	int percentIdx = 0, nr = 1, lastIdx = fmt.length() - 1;
	m_pattern = fmt;
	m_titlePos = m_albumPos = m_artistPos = m_commentPos = m_yearPos = m_trackPos =
		m_genrePos = m_durationPos = -1;
	while (((percentIdx = m_pattern.find('%', percentIdx)) >= 0) &&
		   (percentIdx < lastIdx)) {
		switch (m_pattern[percentIdx + 1].latin1()) {
			case 's':
				m_titlePos = nr;
				break;
			case 'l':
				m_albumPos = nr;
				break;
			case 'a':
				m_artistPos = nr;
				break;
			case 'c':
				m_commentPos = nr;
				break;
			case 'y':
				m_yearPos = nr;
				break;
			case 't':
				m_trackPos = nr;
				break;
			case 'g':
				m_genrePos = nr;
				break;
			case 'd':
				m_durationPos = nr;
				break;
			default:
				++percentIdx;
				continue;
		}
		++nr;
		percentIdx += 2;
	}
	if (enableTrackIncr && m_trackPos == -1) {
		m_trackIncrEnabled = true;
		m_trackIncrNr = 1;
	} else {
		m_trackIncrEnabled = false;
		m_trackIncrNr = 0;
	}
#if QT_VERSION >= 0x030100
	m_pattern.remove(QRegExp("%[slacytgd]"));
#else
	m_pattern.replace(QRegExp("%[slacytgd]"), "");
#endif
	m_re.setPattern(m_pattern);
}

/** Get ID3 text field if it is in format */
#define SET_TEXT_FIELD(name) \
if (m_##name##Pos != -1) st.name = m_re.cap(m_##name##Pos)

/** Get ID3 integer field if it is in format */
#define SET_INT_FIELD(name) \
if (m_##name##Pos != -1) st.name = m_re.cap(m_##name##Pos).toInt()

/**
 * Get next tags in text buffer.
 *
 * @param text text buffer containing data from file or clipboard
 * @param st   standard tags for output
 * @param pos  current position in buffer, will be updated to point
 *             behind current match (to be used for next call)
 * @return true if tags found (pos is index behind match).
 */
bool ImportParser::getNextTags(const QString& text, StandardTags& st, int& pos)
{
	int idx, oldpos = pos;
	if (m_pattern.isEmpty()) {
		m_trackDuration.clear();
		return false;
	}
	if (m_durationPos == -1) {
		m_trackDuration.clear();
	} else if (pos == 0) {
		m_trackDuration.clear();
		int dsp = 0; // "duration search pos"
		int lastDsp = dsp;
		while ((idx = m_re.search(text, dsp)) != -1) {
			QString durationStr = m_re.cap(m_durationPos);
			int duration;
			QRegExp durationRe("(\\d+):(\\d+)");
			if (durationRe.search(durationStr) != -1) {
				duration = durationRe.cap(1).toInt() * 60 +
					durationRe.cap(2).toInt();
			} else {
				duration = durationStr.toInt();
			}
			m_trackDuration.append(duration);

			dsp = idx + m_re.matchedLength();
			if (dsp > lastDsp) { /* avoid endless loop */
				lastDsp = dsp;
			} else {
				break;
			}
		}
	}
	if ((idx = m_re.search(text, pos)) != -1) {
		SET_TEXT_FIELD(title);
		SET_TEXT_FIELD(album);
		SET_TEXT_FIELD(artist);
		SET_TEXT_FIELD(comment);
		SET_INT_FIELD(year);
		SET_INT_FIELD(track);
		if (m_trackIncrEnabled) {
			st.track = m_trackIncrNr++;
		}
		SET_TEXT_FIELD(genre);
		pos = idx + m_re.matchedLength();
		if (pos > oldpos) { /* avoid endless loop */
			return true;
		}
	}
	return false;
}
