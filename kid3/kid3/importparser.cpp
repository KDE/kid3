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
#if QT_VERSION >= 300
	if (pattern == "freedb_header") {
		// special pattern for parsing of freedb.org album data
		parseFreedbTrackDurations(text);
		int dtitlePos, extdYearPos, extdId3gPos;
		QRegExp fdre("DTITLE=\\s*(\\S[^\\r\\n]*\\S)\\s*/\\s*(\\S[^\\r\\n]*\\S)[\\r\\n]");
		if ((dtitlePos = fdre.search(text, pos)) != -1) {
			st.artist = fdre.cap(1);
			st.album = fdre.cap(2);
			dtitlePos += fdre.matchedLength();
		}
		fdre.setPattern("EXTD=[^\\r\\n]*YEAR:\\s*(\\d+)\\D");
		if ((extdYearPos = fdre.search(text, pos)) != -1) {
			st.year = fdre.cap(1).toInt();
			extdYearPos += fdre.matchedLength();
		}
		fdre.setPattern("EXTD=[^\\r\\n]*ID3G:\\s*(\\d+)\\D");
		if ((extdId3gPos = fdre.search(text, pos)) != -1) {
			st.genre = fdre.cap(1).toInt();
			extdId3gPos += fdre.matchedLength();
		}
		if (dtitlePos > pos) pos = dtitlePos;
		if (extdYearPos > pos) pos = extdYearPos;
		if (extdId3gPos > pos) pos = extdId3gPos;
		return (pos > oldpos);
	}
	if (pattern == "freedb_tracks") {
		// special pattern for parsing of freedb.org track data
		trackDuration.clear();
		static int tracknr = 0;
		// assume search for 1st track if pos is 0
		if (pos == 0) {
			tracknr = 0;
		} else {
			++tracknr;
		}
		QRegExp fdre(QString("TTITLE%1=([^\\r\\n]+)[\\r\\n]").arg(tracknr));
		QString title;
		while ((idx = fdre.search(text, pos)) != -1) {
			title += fdre.cap(1);
			pos = idx + fdre.matchedLength();
		}
		if (pos > oldpos) { /* avoid endless loop */
			st.track = tracknr + 1;
			st.title = title;
			return true;
		} else {
			return false;
		}
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
#else
	/* no regexp capture in old Qt versions */
	if (pattern == "freedb_header") {
		// special pattern for parsing of freedb.org album data
		parseFreedbTrackDurations(text);
		int dtitlePos, dextPos, extdYearPos = -1, extdId3gPos = -1;
		if ((dtitlePos = text.find("DTITLE=", pos)) != -1) {
			dtitlePos += 7;
			int slashPos = text.find('/', dtitlePos);
			if (slashPos != -1) {
				st.artist = text.mid(dtitlePos, slashPos - dtitlePos).stripWhiteSpace();
				dtitlePos = slashPos + 1;
				int lfPos = text.find('\n', dtitlePos);
				if (lfPos != -1) {
					st.album = text.mid(dtitlePos, lfPos - dtitlePos).stripWhiteSpace();
					dtitlePos = lfPos + 1;
				}
			}
		}
		if ((dextPos = text.find("EXTD=", pos)) != -1) {
			dextPos += 5;
			int dextEnd = text.find('\n', dextPos);
			if (dextEnd != -1) {
				QString dextStr(text.mid(dextPos, dextEnd - dextPos).stripWhiteSpace());
				int len;
				extdYearPos = QRegExp("YEAR:\\s*\\d+").match(dextStr, 0, &len);
				if (extdYearPos != -1) {
					extdYearPos += 5;
					st.year = dextStr.mid(extdYearPos, len - 5).toInt();
				}
				extdId3gPos = QRegExp("ID3G:\\s*\\d+").match(dextStr, 0, &len);
				if (extdId3gPos != -1) {
					extdId3gPos += 5;
					st.genre = dextStr.mid(extdId3gPos, len - 5).toInt();
				}
			}
		}
		if (dtitlePos > pos) pos = dtitlePos;
		if (extdYearPos > pos) pos = extdYearPos;
		if (extdId3gPos > pos) pos = extdId3gPos;
		return (pos > oldpos);
	}
	if (pattern == "freedb_tracks") {
		// special pattern for parsing of freedb.org track data
		trackDuration.clear();
		static int tracknr = 0;
		// assume search for 1st track if pos is 0
		if (pos == 0) {
			tracknr = 0;
		} else {
			++tracknr;
		}
		QString ttitleStr(QString("TTITLE%1=").arg(tracknr));
		QString title;
		while ((idx = text.find(ttitleStr, pos)) != -1) {
			pos = idx + ttitleStr.length();
			int crPos = text.find('\r', pos);
			if (crPos != -1) {
				title += text.mid(pos, crPos - pos);
				pos = crPos + 1;
			}
		}
		if (pos > oldpos) { /* avoid endless loop */
			st.track = tracknr + 1;
			st.title = title;
			return true;
		} else {
			return false;
		}
	}
	/* hack: use yearPos to determine whether header or track parsing
	   is required */
	if (yearPos == -1) {
		/* track parsing */
		idx = QRegExp("[\\r\\n]\\s*\\d+[.\\s]").match(text, pos);
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
			if (genre != 0xff) {
				st.genre = genre;
				st.genreStr = QString::null;
			} else if (!genreStr.isEmpty()) {
				st.genre = 0xff;
				st.genreStr = genreStr;
			}
			pos = idx + len;
		}
	}
	if (pos > oldpos) { /* avoid endless loop */
		return true;
	}
#endif
	return false;
}

/**
 * Parse the track durations from freedb.org.
 *
 * @param text          text buffer containing data from freedb.org
 */
void ImportParser::parseFreedbTrackDurations(const QString &text)
{
/* Example freedb format:
   # Track frame offsets:
   # 150
   # 2390
   # 23387
   # 44650
   # 61322
   # 94605
   # 121710
   # 144637
   # 176820
   # 187832
   # 218930
   #
   # Disc length: 3114 seconds
*/
	trackDuration.clear();
	int discLenPos, len;
	discLenPos = QRegExp("Disc length:\\s*\\d+").match(text, 0, &len);
	if (discLenPos != -1) {
		discLenPos += 12;
		int discLen = text.mid(discLenPos, len - 12).toInt();
		int trackOffsetPos = text.find("Track frame offsets", 0);
		if (trackOffsetPos != -1) {
			QRegExp re("#\\s*\\d+");
			int lastOffset = -1;
			while ((trackOffsetPos = re.match(text, trackOffsetPos, &len)) != -1 &&
				   trackOffsetPos < discLenPos) {
				trackOffsetPos += 1;
				int trackOffset = text.mid(trackOffsetPos, len - 1).toInt();
				if (lastOffset != -1) {
					int duration = (trackOffset - lastOffset) / 75;
					trackDuration.append(duration);
				}
				lastOffset = trackOffset;
			}
			if (lastOffset != -1) {
				int duration = (discLen * 75 - lastOffset) / 75;
				trackDuration.append(duration);
			}
		}
	}
}
