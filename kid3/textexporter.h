/**
 * \file textexporter.h
 * Export tags as text.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef TEXTEXPORTER_H
#define TEXTEXPORTER_H

#include <QObject>
#include "trackdata.h"

/**
 * Export text from tags.
 */
class TextExporter : public QObject {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit TextExporter(QObject* parent = 0);

  /**
   * Destructor.
   */
	virtual ~TextExporter();

  /**
   * Set data to be exported.
   *
   * @param trackDataVector data to export
   */
  void setTrackData(const ImportTrackDataVector& trackDataVector) {
    m_trackDataVector = trackDataVector;
  }

	/**
	 * Reread the tags in the track data.
	 * @param tagVersion tag version
	 */
	void readTagsInTrackData(TrackData::TagVersion tagVersion) {
		m_trackDataVector.readTags(tagVersion);
	}

  /**
   * Get exported text.
   * @return exported text.
   */
  QString getText() const { return m_text; }

  /**
   * Update text from tags.
   *
   * @param headerFormat header format
   * @param trackFormat track format
   * @param trailerFormat trailer format
   */
  void updateText(const QString& headerFormat, const QString& trackFormat,
                  const QString& trailerFormat);

  /**
   * Update text from tags using formats from the configuration.
   *
   * int fmtIdx index of format
   */
  void updateTextUsingConfig(int fmtIdx);

  /**
   * Export to a file.
   *
   * @param fn file name
   *
   * @return true if ok.
   */
  bool exportToFile(const QString& fn);

private:
  ImportTrackDataVector m_trackDataVector;
  QString m_text;
};

#endif // TEXTEXPORTER_H
