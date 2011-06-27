/**
 * \file trackdata.h
 * Track data, frames with association to tagged file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jul 2005
 *
 * Copyright (C) 2005-2011  Urs Fleisch
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

#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <QVector>
#include "frame.h"
#include "taggedfile.h"

/**
 * Track data, frames with association to tagged file.
 */
class TrackData : public FrameCollection {
public:
	/** Tag version contained in track data. */
	enum TagVersion {
		TagNone = 0, /**< Empty or imported and not from a tag */
		TagV1 = 1,   /**< From tag 1 */
		TagV2 = 2,   /**< From tag 2 */
		TagV2V1 = 3  /**< Merged from tag 2 and tag 1 (where tag 2 is not set) */
	};

	/**
	 * Constructor.
	 */
	TrackData();

	/**
	 * Constructor.
	 * All fields are set from the tagged file,
	 * which should be read using readTags() before.
	 *
	 * @param taggedFile tagged file providing track data
	 * @param tagVersion source of frames
	 */
	TrackData(TaggedFile& taggedFile, TagVersion tagVersion);

	/**
	 * Get duration of file.
	 * @return duration of file.
	 */
	int getFileDuration() const;

	/**
	 * Get absolute filename.
	 *
	 * @return absolute file path.
	 */
	QString getAbsFilename() const;

	/**
	 * Get filename.
	 *
	 * @return filename.
	 */
	QString getFilename() const;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension, e.g. ".mp3".
	 */
	QString getFileExtension() const;

	/**
	 * Get the total number of tracks in the directory.
	 *
	 * @return total number of tracks, -1 if unavailable.
	 */
	int getTotalNumberOfTracksInDir() const;

	/**
	 * Get the format of tag 1.
	 *
	 * @return string describing format of tag 1,
	 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
	 *         QString::null if unknown.
	 */
	QString getTagFormatV1() const;

	/**
	 * Get the format of tag 2.
	 *
	 * @return string describing format of tag 2,
	 *         e.g. "ID3v2.3", "Vorbis", "APE",
	 *         QString::null if unknown.
	 */
	QString getTagFormatV2() const;

	/**
	 * Get detail info.
	 * @param info the detail information is returned here
	 */
	void getDetailInfo(TaggedFile::DetailInfo& info) const;

	/**
	 * Format a string from track data.
	 * Supported format fields:
	 * Those supported by TrackDataFormatReplacer::getReplacement()
	 *
	 * @param format    format specification
	 *
	 * @return formatted string.
	 */
	QString formatString(const QString& format) const;

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
	 * Get tagged file associated with this track data.
	 * @return tagged file, 0 if none assigned.
	 */
	TaggedFile* getTaggedFile() const;

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
	QPersistentModelIndex m_taggedFileIndex;
};

/**
 * Track data used for import.
 */
class ImportTrackData : public TrackData {
public:
	/**
	 * Constructor.
	 */
	ImportTrackData() : m_importDuration(0) {}

	/**
	 * Constructor.
	 * All fields except the import duration are set from the tagged file,
	 * which should be read using readTags() before.
	 *
	 * @param taggedFile tagged file providing track data
	 * @param tagVersion source of frames
	 */
	ImportTrackData(TaggedFile& taggedFile, TagVersion tagVersion) :
		TrackData(taggedFile, tagVersion), m_importDuration(0), m_enabled(true) {}

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
	 * Check if track is enabled.
	 * @return true if enabled (true is default).
	 */
	bool isEnabled() const { return m_enabled; }

	/**
	 * Enable or disable track.
	 * @param enabled true to enable
	 */
	void setEnabled(bool enabled) { m_enabled = enabled; }

private:
	int m_importDuration;
	bool m_enabled;
};

/**
 * Vector containing tracks to import and artist, album names.
 */
class ImportTrackDataVector : public QVector<ImportTrackData> {
public:
	/**
	 * Clear vector and associated data.
	 */
	void clearData();

	/**
	 * Get album artist.
	 * @return album artist.
	 */
	QString getArtist() const;

	/**
	 * Get album title.
	 * @return album title.
	 */
	QString getAlbum() const;

	/**
	 * Check if tag 1 is supported in the first track.
	 * @return true if tag 1 is supported.
	 */
	bool isTagV1Supported() const;

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

	/**
	 * Read the tags from the files.
	 * This can be used to fill the track data with another tag version.
	 *
	 * @param tagVersion tag version to read
	 */
	void readTags(ImportTrackData::TagVersion tagVersion);

private:
	/**
	 * Get frame from first track.
	 * @param type frame type
	 * @return value of frame.
	 */
	QString getFrame(Frame::Type type) const;

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
	 * @param str       string with format codes
	 */
	explicit TrackDataFormatReplacer(
		const TrackData& trackData,
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
	const TrackData& m_trackData;
};

#endif // TRACKDATA_H
