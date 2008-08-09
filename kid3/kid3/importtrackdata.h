/**
 * \file importtrackdata.h
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jul 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#ifndef IMPORTTRACKDATA_H
#define IMPORTTRACKDATA_H

#include "standardtags.h"
#include "frame.h"
#include <qglobal.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVector>
#else
#include <qvaluevector.h>
#endif

/**
 * Track data used for import.
 */
class ImportTrackData : public FrameCollection {
public:
	/**
	 * Constructor.
	 * @param absFilename  absolute filename
	 * @param fileDuration duration in seconds
	 */
	explicit ImportTrackData(const QString& absFilename = QString::null,
													 int fileDuration = 0) :
		m_fileDuration(fileDuration), m_importDuration(0),
		m_absFilename(absFilename) {}

	/**
	 * Destructor.
	 */
	~ImportTrackData() {}

	/**
	 * Get duration of file.
	 * @return duration of file.
	 */
	int getFileDuration() const { return m_fileDuration; }

	/**
	 * Set duration of file.
	 * @param duration duration of file
	 */
	void setFileDuration(int duration) { m_fileDuration = duration; }

	/**
	 * Get duration of import.
	 * @return duration of import.
	 */
	int getImportDuration() const { return m_importDuration; }

	/**
	 * Set duration of import.
	 * @param duration duration of import
	 */
	void setImportDuration(int duration) { m_importDuration = duration; }

	/**
	 * Get absolute filename.
	 *
	 * @return absolute file path.
	 */
	QString getAbsFilename() const { return m_absFilename; }

	/**
	 * Format a string from track data.
	 * Supported format fields:
	 * Those supported by TrackDataFormatReplacer::getReplacement()
	 *
	 * @param format format specification
	 *
	 * @return formatted string.
	 */
	QString formatString(const QString& format, unsigned numTracks = 0) const;

	/**
	 * Get frames.
	 * @return frames.
	 */
	FrameCollection& getFrameCollection() {
		return *(static_cast<FrameCollection*>(this));
	}

	/**
	 * Set frames.
	 * @param frames frames
	 */
	void setFrameCollection(const FrameCollection& frames) {
		*(static_cast<FrameCollection*>(this)) = frames;
	}

	/**
	 * Get help text for format codes supported by formatString().
	 *
	 * @param onlyRows if true only the tr elements are returned,
	 *                 not the surrounding table
	 *
	 * @return help text.
	 */
	static QString getFormatToolTip(bool onlyRows = false);

private:
	int m_fileDuration;
	int m_importDuration;
	QString m_absFilename;
};

/**
 * Vector containing tracks to import and artist, album names.
 */
class ImportTrackDataVector : public
#if QT_VERSION >= 0x040000
QVector<ImportTrackData>
#else
QValueVector<ImportTrackData>
#endif
{
public:
	/**
	 * Get album artist.
	 * @return album artist.
	 */
	QString getArtist() const { return m_artist; }

	/**
	 * Set album artist.
	 * @param artist artist
	 */
	void setArtist(const QString& artist) { m_artist = artist; }

	/**
	 * Get album title.
	 * @return album title.
	 */
	QString getAlbum() const { return m_album; }

	/**
	 * Set album title.
	 * @param album album
	 */
	void setAlbum(const QString& album) { m_album = album; }

private:
	QString m_artist;
	QString m_album;
};


/**
 * Replaces track data format codes in a string.
 */
class TrackDataFormatReplacer : public FrameFormatReplacer {
public:
	/**
	 * Constructor.
	 *
	 * @param trackData track data
	 * @param numTracks number of tracks in album
	 * @param str       string with format codes
	 */
	explicit TrackDataFormatReplacer(
		const ImportTrackData& trackData, unsigned numTracks = 0,
		const QString& str = QString());

	/**
	 * Destructor.
	 */
	virtual ~TrackDataFormatReplacer();

	/**
	 * Get help text for supported format codes.
	 *
	 * @param onlyRows if true only the tr elements are returned,
	 *                 not the surrounding table
	 *
	 * @return help text.
	 */
	static QString getToolTip(bool onlyRows = false);

protected:
	/**
	 * Replace a format code (one character %c or multiple characters %{chars}).
	 * Supported format fields:
	 * Those supported by FrameFormatReplacer::getReplacement()
	 * %f filename
	 * %p path to file
	 * %u URL of file
	 * %d duration in minutes:seconds
	 * %D duration in seconds
	 * %n number of tracks
	 *
	 * @param code format code
	 *
	 * @return replacement string,
	 *         QString::null if code not found.
	 */
	virtual QString getReplacement(const QString& code) const;

private:
	const ImportTrackData& m_trackData;
	const unsigned m_numTracks;
};

#endif // IMPORTTRACKDATA_H
