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
	void setFormat(const QString &fmt, bool enableTrackIncr = false);
	/**
	 * Get next tags in text buffer.
	 *
	 * @param text text buffer containing data from file or clipboard
	 * @param st   standard tags for output
	 * @param pos  current position in buffer, will be updated to point
	 *             behind current match (to be used for next call)
	 * @return true if tags found (pos is index behind match).
	 */
	bool getNextTags(const QString &text, StandardTags &st, int &pos);

private:
	/** track regexp pattern */
	QString pattern;
	/** regexp object */
	QRegExp re;
	/** true if automatic track number incrementing is used */
	bool trackIncrEnabled;
	/** automatically incremented track number */
	int trackIncrNr;
	/** numbers of fields within regexp captures, -1 if not in pattern */
	int titlePos;
	int albumPos;
	int artistPos;
	int commentPos;
	int yearPos;
	int trackPos;
	int genrePos;
};

#endif
