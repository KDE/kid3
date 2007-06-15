/**
 * \file importparser.h
 * Import parser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef IMPORTPARSER_H
#define IMPORTPARSER_H

#include <qstring.h>
#include <qregexp.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ValueList>
#else
#include <qvaluelist.h>
#endif

class StandardTags;

/**
 * Import parser.
 */
class ImportParser {
public:
	/**
	 * Set import format.
	 *
	 * @param fmt format regexp
	 * @param enableTrackIncr enable automatic track increment if no %t is found
	 */
	void setFormat(const QString& fmt, bool enableTrackIncr = false);

	/**
	 * Get next tags in text buffer.
	 *
	 * @param text text buffer containing data from file or clipboard
	 * @param st   standard tags for output
	 * @param pos  current position in buffer, will be updated to point
	 *             behind current match (to be used for next call)
	 * @return true if tags found (pos is index behind match).
	 */
	bool getNextTags(const QString& text, StandardTags& st, int& pos);

	/**
	 * Get list with track durations.
	 *
	 * @return list with track durations.
	 */
	Q3ValueList<int>* getTrackDurations() { return &m_trackDuration; }

private:
	/** track regexp pattern */
	QString m_pattern;
	/** regexp object */
	QRegExp m_re;
	/** true if automatic track number incrementing is used */
	bool m_trackIncrEnabled;
	/** automatically incremented track number */
	int m_trackIncrNr;
	/** numbers of fields within regexp captures, -1 if not in pattern */
	int m_titlePos;
	int m_albumPos;
	int m_artistPos;
	int m_commentPos;
	int m_yearPos;
	int m_trackPos;
	int m_genrePos;
	int m_durationPos;
	Q3ValueList<int> m_trackDuration;
};

#endif
