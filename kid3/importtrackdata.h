/**
 * \file importtrackdata.h
 * Track data used for import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jul 2005
 *
 * Copyright (C) 2005-2009  Urs Fleisch
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

#include "frame.h"
#include "taggedfile.h"
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
	 * Constructor.
	 * All fields except the import duration are set from the tagged file,
	 * which should be read using readTags() before. The frames are merged
	 * from tag 2 and tag 1 (where tag 2 is not set).
	 *
	 * @param taggedFile tagged file providing track data
	 */
	ImportTrackData(TaggedFile& taggedFile);

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
	 * Set absolute filename.
	 *
	 * @param absFilename absolute file path
	 */
	void setAbsFilename(const QString& absFilename) {
		m_absFilename = absFilename;
	}

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension, e.g. ".mp3".
	 */
	QString getFileExtension() const;

	/**
	 * Set file extension.
	 * @param fileExtension file extension
	 */
	void setFileExtension(const QString& fileExtension) {
		m_fileExtension = fileExtension;
	}

	/**
	 * Get the format of tag 1.
	 *
	 * @return string describing format of tag 1,
	 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
	 *         QString::null if unknown.
	 */
	QString getTagFormatV1() const { return m_tagFormatV1; }

	/**
	 * Set the format of tag 1.
	 * @param tagFormatV1 string describing format of tag 1
	 */
	void setTagFormatV1(const QString& tagFormatV1) { m_tagFormatV1 = tagFormatV1; }

	/**
	 * Get the format of tag 2.
	 *
	 * @return string describing format of tag 2,
	 *         e.g. "ID3v2.3", "Vorbis", "APE",
	 *         QString::null if unknown.
	 */
	QString getTagFormatV2() const { return m_tagFormatV2; }

	/**
	 * Set the format of tag 2.
	 * @param tagFormatV2 string describing format of tag 2
	 */
	void setTagFormatV2(const QString& tagFormatV2) { m_tagFormatV2 = tagFormatV2; }

	/**
	 * Get detail info.
	 * @return detail info.
	 */
	const TaggedFile::DetailInfo& getDetailInfo() const { return m_detailInfo; }

	/**
	 * Set detail info.
	 * @param detailInfo detail info
	 */
	void setDetailInfo(const TaggedFile::DetailInfo& detailInfo) { m_detailInfo = detailInfo; }

	/**
	 * Format a string from track data.
	 * Supported format fields:
	 * Those supported by TrackDataFormatReplacer::getReplacement()
	 *
	 * @param format    format specification
	 * @param numTracks number of tracks in album
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
	QString m_fileExtension;
	QString m_tagFormatV1;
	QString m_tagFormatV2;
	TaggedFile::DetailInfo m_detailInfo;
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
	 * Clear vector and associated data.
	 */
	void clearData() {
		clear();
		m_artist = QString();
		m_album = QString();
		m_coverArtUrl = QString();
	}

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

	/**
	 * Get cover art URL.
	 * @return cover art URL.
	 */
	QString getCoverArtUrl() const { return m_coverArtUrl; }

	/**
	 * Set cover art URL.
	 * @param coverArtUrl cover art URL
	 */
	void setCoverArtUrl(const QString& coverArtUrl) { m_coverArtUrl = coverArtUrl; }

private:
	QString m_artist;
	QString m_album;
	QString m_coverArtUrl;
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
