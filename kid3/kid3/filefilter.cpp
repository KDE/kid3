/**
 * \file filefilter.cpp
 * Filter for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
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

#include "filefilter.h"
#include "taggedfile.h"
#include <qregexp.h>
#include "qtcompatmac.h"

/**
 * Constructor.
 */
FileFilter::FileFilter() :
	m_parser(QStringList() << "equals" << "contains" << "matches")
{
}

/**
 * Destructor.
 */
FileFilter::~FileFilter()
{
}

/**
 * Initialize the parser.
 * This method has to be called before the first call to parse()
 * and afterwards when the expression has been changed.
 */
void FileFilter::initParser()
{
	m_parser.tokenizeRpn(m_filterExpression);
}

/**
 * Format a string from tag data.
 *
 * @param format format specification
 *
 * @return formatted string.
 */
QString FileFilter::formatString(const QString& format)
{
	if (format.QCM_indexOf('%') == -1) {
		return format;
	}
	QString str(format);
	str.replace(QString("%1"), QString("\v1"));
	str.replace(QString("%2"), QString("\v2"));
	str = m_trackData12.formatString(str);
	if (str.QCM_indexOf('\v') != -1) {
		str.replace(QString("\v2"), QString("%"));
		str = m_trackData2.formatString(str);
		if (str.QCM_indexOf('\v') != -1) {
			str.replace(QString("\v1"), QString("%"));
			str = m_trackData1.formatString(str);
		}
	}
	return str;
}

/**
 * Get help text for format codes supported by formatString().
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString FileFilter::getFormatToolTip(bool onlyRows)
{
	QString str;
	if (!onlyRows) str += "<table>\n";
	str += TrackDataFormatReplacer::getToolTip(true);

	str += "<tr><td>%1a...</td><td>%1{artist}...</td><td>";
	str += QCM_translate("Tag 1");
	str += " ";
	str += QCM_translate("Artist");
	str += "</td></tr>\n";

	str += "<tr><td>%2a...</td><td>%2{artist}...</td><td>";
	str += QCM_translate("Tag 2");
	str += " ";
	str += QCM_translate("Artist");
	str += "</td></tr>\n";

	str += "<tr><td></td><td>equals</td><td>";
	str += QCM_translate(I18N_NOOP("True if strings are equal"));
	str += "</td></tr>\n";

	str += "<tr><td></td><td>contains</td><td>";
	str += QCM_translate(I18N_NOOP("True if string contains substring"));
	str += "</td></tr>\n";

	str += "<tr><td></td><td>matches</td><td>";
	str += QCM_translate(I18N_NOOP("True if string matches regexp"));
	str += "</td></tr>\n";

	str += "<tr><td></td><td>and</td><td>";
	str += QCM_translate(I18N_NOOP("Logical AND"));
	str += "</td></tr>\n";

	str += "<tr><td></td><td>or</td><td>";
	str += QCM_translate(I18N_NOOP("Logical OR"));
	str += "</td></tr>\n";

	str += "<tr><td></td><td>not</td><td>";
	str += QCM_translate(I18N_NOOP("Logical negation"));
	str += "</td></tr>\n";

	if (!onlyRows) str += "</table>\n";
	return str;
}

/**
 * Evaluate the expression to a boolean result.
 * @see initParser()
 * @return result of expression.
 */
bool FileFilter::parse()
{
	QString op, var1, var2;
	bool result = false;
	m_parser.clearEvaluation();
	while (m_parser.evaluate(op, var1, var2)) {
		var1 = formatString(var1);
		var2 = formatString(var2);
		if (op == "equals") {
			m_parser.pushBool(var1 == var2);
		} else if (op == "contains") {
			m_parser.pushBool(var2.QCM_indexOf(var1) >= 0);
		} else if (op == "matches") {
			m_parser.pushBool(QRegExp(var1).exactMatch(var2));
		}
	}
	if (!m_parser.hasError()) {
		m_parser.popBool(result);
	}
	return result;
}

/**
 * Check if file passes through filter.
 *
 * @param taggedFile file to check
 * @param ok         if not 0, false is returned here when parsing fails
 *
 * @return true if file passes through filter.
 */
bool FileFilter::filter(TaggedFile& taggedFile, bool* ok)
{
	if (m_filterExpression.isEmpty()) {
		if (ok) *ok = true;
		return true;
	}
	m_trackData1.clear();
	m_trackData1.setAbsFilename(taggedFile.getAbsFilename());
	m_trackData1.setFileDuration(taggedFile.getDuration());
	m_trackData1.setFileExtension(taggedFile.getFileExtension());
	m_trackData1.setTagFormatV1(taggedFile.getTagFormatV1());
	m_trackData1.setTagFormatV2(taggedFile.getTagFormatV2());
	m_trackData2 = m_trackData1;
	taggedFile.getAllFramesV1(m_trackData1);
	taggedFile.getAllFramesV2(m_trackData2);
	m_trackData12 = m_trackData2;
	m_trackData12.merge(m_trackData1);
	TaggedFile::DetailInfo info;
	taggedFile.getDetailInfo(info);
	m_trackData12.setDetailInfo(info);

	bool result = parse();
	if (m_parser.hasError()) {
		if (ok) *ok = false;
		return false;
	} else {
		if (ok) *ok = true;
		return result;
	}
}
