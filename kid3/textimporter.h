/**
 * \file textimporter.h
 * Import tags from text.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
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

#ifndef TEXTIMPORTER_H
#define TEXTIMPORTER_H

#include <QString>

class ImportTrackDataVector;
class ImportParser;
class FrameCollection;

/**
 * Import tags from text.
 */
class TextImporter {
public:
	/**
	 * Constructor.
	 *
	 * @param trackDataVector track data to be filled with imported values
	 */
	explicit TextImporter(ImportTrackDataVector& trackDataVector);

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

private:
	/**
	 * Look for album specific information (artist, album, year, genre) in
	 * a header.
	 *
	 * @param frames frames to put resulting values in,
	 *           fields which are not found are not touched.
	 *
	 * @return true if one or more field were found.
	 */
	bool parseHeader(FrameCollection& frames);

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
	bool getNextTags(FrameCollection& frames, bool start);

	/**
	 * Get list with track durations.
	 *
	 * @return list with track durations,
	 *         empty if no track durations found.
	 */
	QList<int> getTrackDurations();

	/** contents of imported file/clipboard */
	QString m_text;
	/** header format */
	QString m_headerFormat;
	/** track format */
	QString m_trackFormat;
	/** header parser object */
	ImportParser* m_headerParser;
	/** track parser object */
	ImportParser* m_trackParser;
	/** track data */
	ImportTrackDataVector& m_trackDataVector;
};

#endif
