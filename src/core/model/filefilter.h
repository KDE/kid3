/**
 * \file filefilter.h
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

#ifndef FILEFILTER_H
#define FILEFILTER_H

#include "expressionparser.h"
#include "trackdata.h"
#include <QObject>
#include <QString>

class TaggedFile;

/**
 * Filter for tagged files.
 */
class KID3_CORE_EXPORT FileFilter : public QObject {
Q_OBJECT
public:
  /** Type of filter event. */
  enum FilterEventType {
    ParseError, FilePassed, FileFilteredOut
  };

  /**
   * Constructor.
   */
  FileFilter();

  /**
   * Destructor.
   */
  virtual ~FileFilter();

  /**
   * Set filter expression.
   * @param filterExpression filter expression
   */
  void setFilterExpression(const QString& filterExpression) {
    m_filterExpression = filterExpression;
  }

  /**
   * Check if filter expression is empty.
   * @return true if filter expression is empty.
   */
  bool isEmptyFilterExpression() const { return m_filterExpression.isEmpty(); }

  /**
   * Initialize the parser.
   * This method has to be called before the first call to parse()
   * and afterwards when the expression has been changed.
   */
  void initParser();

  /**
   * Check if file passes through filter.
   *
   * @param taggedFile file to check
   * @param ok         if not 0, false is returned here when parsing fails
   *
   * @return true if file passes through filter.
   */
  bool filter(TaggedFile& taggedFile, bool* ok = 0);

  /**
   * Clear abort flag.
   */
  void clearAbortFlag() { m_aborted = false; }

  /**
   * Check if dialog was aborted.
   * @return true if aborted.
   */
  bool getAbortFlag() { return m_aborted; }

  /**
   * Get help text for format codes supported by formatString().
   *
   * @param onlyRows if true only the tr elements are returned,
   *                 not the surrounding table
   *
   * @return help text.
   */
  static QString getFormatToolTip(bool onlyRows = false);

public slots:
  /**
   * Set abort flag.
   */
  void setAbortFlag() { m_aborted = true; }

private:
  /**
   * Format a string from tag data.
   *
   * @param format format specification
   *
   * @return formatted string.
   */
  QString formatString(const QString& format);

  /**
   * Evaluate the expression to a boolean result.
   * @see initParser()
   * @return result of expression.
   */
  bool parse();

  QString m_filterExpression;
  ExpressionParser m_parser;
  ImportTrackData m_trackData1;
  ImportTrackData m_trackData2;
  ImportTrackData m_trackData12;
  bool m_aborted;
};

#endif
