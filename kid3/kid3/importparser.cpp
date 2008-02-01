/**
 * \file importparser.cpp
 * Import parser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
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

#include "standardtags.h"
#include "genres.h"
#include "importparser.h"

/**
 * Get help text for format codes supported by setFormat().
 *
 * @return help text.
 */
QString ImportParser::getFormatToolTip()
{
	QString str;
	str += "<table>\n";

	str += "<tr><td>%s</td><td>%{title}</td><td>";
	str += QCM_translate("Title");
	str += "</td></tr>\n";

	str += "<tr><td>%l</td><td>%{album}</td><td>";
	str += QCM_translate("Album");
	str += "</td></tr>\n";

	str += "<tr><td>%a</td><td>%{artist}</td><td>";
	str += QCM_translate("Artist");
	str += "</td></tr>\n";

	str += "<tr><td>%c</td><td>%{comment}</td><td>";
	str += QCM_translate("Comment");
	str += "</td></tr>\n";

	str += "<tr><td>%y</td><td>%{year}</td><td>";
	str += QCM_translate(I18N_NOOP("Year"));
	str += "</td></tr>\n";

	str += "<tr><td>%t</td><td>%{track}</td><td>";
	str += QCM_translate("Track");
	str += "</td></tr>\n";

	str += "<tr><td>%g</td><td>%{genre}</td><td>";
	str += QCM_translate("Genre");
	str += "</td></tr>\n";

	str += "<tr><td>%d</td><td>%{duration}</td><td>";
	str += QCM_translate(I18N_NOOP("Length"));
	str += "</td></tr>\n";

	str += "</table>\n";
	return str;
}

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
	m_pattern.replace("%{title}", "%s");
	m_pattern.replace("%{album}", "%l");
	m_pattern.replace("%{artist}", "%a");
	m_pattern.replace("%{comment}", "%c");
	m_pattern.replace("%{year}", "%y");
	m_pattern.replace("%{track}", "%t");
	m_pattern.replace("%{tracknumber}", "%t");
	m_pattern.replace("%{genre}", "%g");
	m_pattern.replace("%{duration}", "%d");
	m_titlePos = m_albumPos = m_artistPos = m_commentPos = m_yearPos = m_trackPos =
		m_genrePos = m_durationPos = -1;
	while (((percentIdx = m_pattern.QCM_indexOf('%', percentIdx)) >= 0) &&
		   (percentIdx < lastIdx)) {
		switch (
#if QT_VERSION >= 0x040000
			m_pattern[percentIdx + 1].toLatin1()
#else
			m_pattern[percentIdx + 1].latin1()
#endif
			) {
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
		while ((idx = m_re.QCM_indexIn(text, dsp)) != -1) {
			QString durationStr = m_re.cap(m_durationPos);
			int duration;
			QRegExp durationRe("(\\d+):(\\d+)");
			if (durationRe.QCM_indexIn(durationStr) != -1) {
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
	if ((idx = m_re.QCM_indexIn(text, pos)) != -1) {
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
