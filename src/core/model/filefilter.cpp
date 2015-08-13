/**
 * \file filefilter.cpp
 * Filter for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jan 2008
 *
 * Copyright (C) 2008-2013  Urs Fleisch
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
#include <QRegExp>
#include <QCoreApplication>

/**
 * Constructor.
 * @param parent parent object
 */
FileFilter::FileFilter(QObject* parent) : QObject(parent),
  m_parser(QStringList() << QLatin1String("equals")
                         << QLatin1String("contains")
                         << QLatin1String("matches")),
  m_aborted(false)
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
  if (format.indexOf(QLatin1Char('%')) == -1) {
    return format;
  }
  QString str(format);
  str.replace(QLatin1String("%1"), QLatin1String("\v1"));
  str.replace(QLatin1String("%2"), QLatin1String("\v2"));
  str = m_trackData12.formatString(str);
  if (str.indexOf(QLatin1Char('\v')) != -1) {
    str.replace(QLatin1String("\v2"), QLatin1String("%"));
    str = m_trackData2.formatString(str);
    if (str.indexOf(QLatin1Char('\v')) != -1) {
      str.replace(QLatin1String("\v1"), QLatin1String("%"));
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
  if (!onlyRows) str += QLatin1String("<table>\n");
  str += TrackDataFormatReplacer::getToolTip(true);

  str += QLatin1String("<tr><td>%1a...</td><td>%1{artist}...</td><td>");
  str += QCoreApplication::translate("@default", "Tag 1");
  str += QLatin1Char(' ');
  str += QCoreApplication::translate("@default", "Artist");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%2a...</td><td>%2{artist}...</td><td>");
  str += QCoreApplication::translate("@default", "Tag 2");
  str += QLatin1Char(' ');
  str += QCoreApplication::translate("@default", "Artist");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>equals</td><td>");
  const char* const trueIfStringsAreEqualStr =
      QT_TRANSLATE_NOOP("@default", "True if strings are equal");
  str += QCoreApplication::translate("@default", trueIfStringsAreEqualStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>contains</td><td>");
  const char* const trueIfStringContainsSubstringStr =
      QT_TRANSLATE_NOOP("@default", "True if string contains substring");
  str += QCoreApplication::translate("@default",
                                     trueIfStringContainsSubstringStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>matches</td><td>");
  const char* const trueIfStringMatchesRegexpStr =
      QT_TRANSLATE_NOOP("@default", "True if string matches regexp");
  str += QCoreApplication::translate("@default", trueIfStringMatchesRegexpStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>and</td><td>");
  const char* const logicalAndStr =
      QT_TRANSLATE_NOOP("@default", "Logical AND");
  str += QCoreApplication::translate("@default", logicalAndStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>or</td><td>");
  const char* const logicalOrStr = QT_TRANSLATE_NOOP("@default", "Logical OR");
  str += QCoreApplication::translate("@default", logicalOrStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td></td><td>not</td><td>");
  const char* const logicalNegationStr =
      QT_TRANSLATE_NOOP("@default", "Logical negation");
  str += QCoreApplication::translate("@default", logicalNegationStr);
  str += QLatin1String("</td></tr>\n");

  if (!onlyRows) str += QLatin1String("</table>\n");
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
    if (op == QLatin1String("equals")) {
      m_parser.pushBool(var1 == var2);
    } else if (op == QLatin1String("contains")) {
      m_parser.pushBool(var2.indexOf(var1) >= 0);
    } else if (op == QLatin1String("matches")) {
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
  m_trackData1 = ImportTrackData(taggedFile, Frame::TagV1);
  m_trackData2 = ImportTrackData(taggedFile, Frame::TagV2);
  m_trackData12 = ImportTrackData(taggedFile, Frame::TagV2V1);

  bool result = parse();
  if (m_parser.hasError()) {
    if (ok) *ok = false;
    return false;
  } else {
    if (ok) *ok = true;
    return result;
  }
}

/**
 * Clear abort flag.
 */
void FileFilter::clearAborted()
{
  m_aborted = false;
}

/**
 * Check if dialog was aborted.
 * @return true if aborted.
 */
bool FileFilter::isAborted() const
{
  return m_aborted;
}

/**
 * Set abort flag.
 */
void FileFilter::abort()
{
  m_aborted = true;
}
