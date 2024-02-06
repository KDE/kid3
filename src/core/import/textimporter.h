/**
 * \file textimporter.h
 * Import tags from text.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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

#pragma once

#include <QString>
#include <QScopedPointer>
#include "kid3api.h"

class ImportTrackDataVector;
class ImportParser;
class TrackDataModel;
class TrackData;
/**
 * Import tags from text.
 */
class KID3_CORE_EXPORT TextImporter {
public:
  /**
   * Constructor.
   *
   * @param trackDataModel track data to be filled with imported values
   */
  explicit TextImporter(TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  ~TextImporter();

  /**
   * Update track data list with imported tags.
   *
   * @param text text to import
   * @param headerFormat header format
   * @param trackFormat track format
   *
   * @return true if tags were found.
   */
  bool updateTrackData(const QString& text,
                       const QString& headerFormat, const QString& trackFormat);

  /**
   * Import text from tags to other tags.
   *
   * @param sourceFormat format to create source text
   * @param extractionFormat regular expression to extract other tags
   * @param trackDataVector track data to process
   */
  static void importFromTags(
    const QString& sourceFormat,
    const QString& extractionFormat,
    ImportTrackDataVector& trackDataVector);

  /**
   * Import text from tags to other tags.
   *
   * @param sourceFormat format to create source text
   * @param parser import parser which is initialized with extraction format
   * @param trackData track data to process
   */
  static void importFromTags(
      const QString& sourceFormat, ImportParser& parser, TrackData& trackData);

private:
  Q_DISABLE_COPY(TextImporter)

  /**
   * Look for album specific information (artist, album, year, genre) in
   * a header.
   *
   * @param frames frames to put resulting values in,
   *           fields which are not found are not touched.
   *
   * @return true if one or more field were found.
   */
  bool parseHeader(TrackData& frames);

  /**
   * Get next line as frames from imported file or clipboard.
   *
   * @param frames frames
   * @param start true to start with the first line, false for all
   *              other lines
   *
   * @return true if ok (result in st),
   *         false if end of file reached.
   */
  bool getNextTags(TrackData& frames, bool start);

  /**
   * Get list with track durations.
   *
   * @return list with track durations,
   *         empty if no track durations found.
   */
  QList<int> getTrackDurations() const;

  /** contents of imported file/clipboard */
  QString m_text;
  /** header format */
  QString m_headerFormat;
  /** track format */
  QString m_trackFormat;
  /** header parser object */
  QScopedPointer<ImportParser> m_headerParser;
  /** track parser object */
  QScopedPointer<ImportParser> m_trackParser;
  /** track data */
  TrackDataModel* m_trackDataModel;
};
