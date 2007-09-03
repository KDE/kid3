/**
 * \file taggedfile.h
 * Base class for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
 */

#ifndef TAGGEDFILE_H
#define TAGGEDFILE_H

#include <qstring.h>
#include "standardtags.h"
#include "frame.h"

class DirInfo;

/** Base class for tagged files. */
class TaggedFile {
public:
	/**
	 * Constructor.
	 *
	 * @param di directory information
	 * @param fn filename
	 */
	TaggedFile(const DirInfo* di, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~TaggedFile();

	/**
	 * Set file name.
	 *
	 * @param fn file name
	 */
	void setFilename(const QString& fn) { m_newFilename = fn; }

	/**
	 * Get file name.
	 *
	 * @return file name
	 */
	const QString& getFilename() const { return m_newFilename; }

	/**
	 * Get directory name.
	 *
	 * @return directory name
	 */
	QString getDirname() const;

	/**
	 * Get information about directory.
	 *
	 * @return directory information.
	 */
	const DirInfo* getDirInfo() const { return m_dirInfo; }

	/**
	 * Read tags from file.
	 *
	 * @param force true to force reading even if tags were already read.
	 */
	virtual void readTags(bool force) = 0;

	/**
	 * Write tags to file and rename it if necessary.
	 *
	 * @param force   true to force writing even if file was not changed.
	 * @param renamed will be set to true if the file was renamed,
	 *                i.e. the file name is no longer valid, else *renamed
	 *                is left unchanged
	 * @param preserve true to preserve file time stamps
	 *
	 * @return true if ok, false if the file could not be written or renamed.
	 */
	virtual bool writeTags(bool force, bool* renamed, bool preserve) = 0;

	/**
	 * Get ID3v1 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getTitleV1();

	/**
	 * Get ID3v1 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV1();

	/**
	 * Get ID3v1 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV1();

	/**
	 * Get ID3v1 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV1();

	/**
	 * Get ID3v1 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV1();

	/**
	 * Get ID3v1 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV1();

	/**
	 * Get ID3v1 genre.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getGenreV1();

	/**
	 * Remove all ID3v1 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	virtual void removeTagsV1(const StandardTagsFilter& flt);

	/**
	 * Set ID3v1 title.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setTitleV1(const QString& str);

	/**
	 * Set ID3v1 artist.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setArtistV1(const QString& str);

	/**
	 * Set ID3v1 album.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setAlbumV1(const QString& str);

	/**
	 * Set ID3v1 comment.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setCommentV1(const QString& str);

	/**
	 * Set ID3v1 year.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setYearV1(int num);

	/**
	 * Set ID3v1 track.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setTrackNumV1(int num);

	/**
	 * Set ID3v1 genre as text.
	 *
	 * @param str string to set, "" to remove field, QString::null to ignore.
	 */
	virtual void setGenreV1(const QString& str);

	/**
	 * Check if file has an ID3v1 tag.
	 *
	 * @return true if a V1 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV1() const;

	/**
	 * Check if ID3v1 tags are supported by the format of this file.
	 *
	 * @return true if V1 tags are supported.
	 */
	virtual bool isTagV1Supported() const;

	/**
	 * Get ID3v2 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getTitleV2() = 0;

	/**
	 * Get ID3v2 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV2() = 0;

	/**
	 * Get ID3v2 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV2() = 0;

	/**
	 * Get ID3v2 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV2() = 0;

	/**
	 * Get ID3v2 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV2() = 0;

	/**
	 * Get ID3v2 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV2() = 0;

	/**
	 * Remove all ID3v2 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	virtual void removeTagsV2(const StandardTagsFilter& flt) = 0;

	/**
	 * Get ID3v2 genre as text.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getGenreV2() = 0;

	/**
	 * Set ID3v2 title.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setTitleV2(const QString& str) = 0;

	/**
	 * Set ID3v2 artist.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setArtistV2(const QString& str) = 0;

	/**
	 * Set ID3v2 album.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setAlbumV2(const QString& str) = 0;

	/**
	 * Set ID3v2 comment.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setCommentV2(const QString& str) = 0;

	/**
	 * Set ID3v2 year.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setYearV2(int num) = 0;

	/**
	 * Set ID3v2 track.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setTrackNumV2(int num) = 0;

	/**
	 * Set ID3v2 genre as text.
	 *
	 * @param str string to set, "" to remove field, QString::null to ignore.
	 */
	virtual void setGenreV2(const QString& str) = 0;

	/**
	 * Check if file has an ID3v2 tag.
	 *
	 * @return true if a V2 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV2() const = 0;

	/**
	 * Check if tag information has already been read.
	 *
	 * @return true if information is available,
	 *         false if the tags have not been read yet, in which case
	 *         hasTagV1() and hasTagV2() do not return meaningful information.
	 */
	virtual bool isTagInformationRead() const = 0;

	/**
	 * Get technical detail information.
	 *
	 * @return string with detail information,
	 *         "" if no information available.
	 */
	virtual QString getDetailInfo() const = 0;

	/**
	 * Get duration of file.
	 *
	 * @return duration in seconds,
	 *         0 if unknown.
	 */
	virtual unsigned getDuration() const = 0;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension, e.g. ".mp3".
	 */
	virtual QString getFileExtension() const = 0;

	/**
	 * Get the format of tag 1.
	 *
	 * @return string describing format of tag 1,
	 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
	 *         QString::null if unknown.
	 */
	virtual QString getTagFormatV1() const;

	/**
	 * Get the format of tag 2.
	 *
	 * @return string describing format of tag 2,
	 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
	 *         QString::null if unknown.
	 */
	virtual QString getTagFormatV2() const;

	/**
	 * Get a specific frame from the tags 1.
	 *
	 * @param type  frame type
	 * @param frame the frame is returned here
	 *
	 * @return true if ok.
	 */
	virtual bool getFrameV1(Frame::Type type, Frame& frame);

	/**
	 * Set a frame in the tags 1.
	 *
	 * @param frame frame to set.
	 *
	 * @return true if ok.
	 */
	virtual bool setFrameV1(const Frame& frame);

	/**
	 * Get a specific frame from the tags 2.
	 *
	 * @param type  frame type
	 * @param frame the frame is returned here
	 *
	 * @return true if ok.
	 */
	virtual bool getFrameV2(Frame::Type type, Frame& frame);

	/**
	 * Set a frame in the tags 2.
	 *
	 * @param frame frame to set, the index can be set by this method
	 *
	 * @return true if ok.
	 */
	virtual bool setFrameV2(Frame& frame);

	/**
	 * Add a frame in the tags 2.
	 *
	 * @param frame frame to add, a field list may be added by this method
	 *
	 * @return true if ok.
	 */
	virtual bool addFrameV2(Frame& frame);

	/**
	 * Delete a frame in the tags 2.
	 *
	 * @param frame frame to delete.
	 *
	 * @return true if ok.
	 */
	virtual bool deleteFrameV2(const Frame& frame);

	/**
	 * Get all frames in tag 1.
	 *
	 * @return frame collection.
	 */
	virtual FrameCollection getAllFramesV1();

	/**
	 * Get all frames in tag 2.
	 *
	 * @return frame collection.
	 */
	virtual FrameCollection getAllFramesV2();

	/**
	 * Get a list of frame IDs which can be added.
	 *
	 * @return list with frame IDs.
	 */
	virtual QStringList getFrameIds() const;

	/**
	 * Get ID3v1 tags from file.
	 *
	 * @param st tags to put result
	 */
	void getStandardTagsV1(StandardTags* st);

	/**
	 * Get ID3v2 tags from file.
	 *
	 * @param st tags to put result
	 */
	void getStandardTagsV2(StandardTags* st);

	/**
	 * Set ID3v1 tags.
	 *
	 * @param st tags to set
	 * @param flt filter specifying which fields to set
	 */
	void setStandardTagsV1(const StandardTags* st,
												 const StandardTagsFilter& flt);
	/**
	 * Set ID3v2 tags.
	 *
	 * @param st tags to set
	 * @param flt filter specifying which fields to set
	 */
	void setStandardTagsV2(const StandardTags* st,
												 const StandardTagsFilter& flt);
	/**
	 * Get tags from filename.
	 * Supported formats:
	 * artist - album/track song
	 *
	 * @param st  tags to put result
	 * @param fmt format string containing the following codes:
	 *            %s title (song)
	 *            %l album
	 *            %a artist
	 *            %c comment
	 *            %y year
	 *            %t track
	 */
	void getTagsFromFilename(StandardTags* st, QString fmt);

	/**
	 * Create string with tags according to format string.
	 *
	 * @param st  tags to use to build filename
	 * @param fmt format string containing the following codes:
	 *            %s title (song)
	 *            %l album
	 *            %a artist
	 *            %c comment
	 *            %y year
	 *            %t track
	 *            %g genre
	 * @param isDirname true to generate a directory name
	 *
	 * @return format string with format codes replaced by tags.
	 */
	QString formatWithTags(const StandardTags* st, QString fmt,
	                       bool isDirname = false) const;

	/**
	 * Get filename from tags.
	 *
	 * @param st  tags to use to build filename
	 * @param fmt format string containing the following codes:
	 *            %s title (song)
	 *            %l album
	 *            %a artist
	 *            %c comment
	 *            %y year
	 *            %t track
	 *            %g genre
	 */
	void getFilenameFromTags(const StandardTags* st, QString fmt);

	/**
	 * Check if file is changed.
	 *
	 * @return true if file was changed.
	 */
	bool isChanged() const { return m_changedV1 || m_changedV2 ||
			m_newFilename != m_filename; }

	/**
	 * Get absolute filename.
	 *
	 * @return absolute file path.
	 */
	QString getAbsFilename() const;

	/**
	 * Check if tag 2 was changed.
	 * @return true if tag 2 was changed.
	 */
	bool isTag2Changed() const { return m_changedV2; }

	/**
	 * Mark tag 2 as changed.
	 * @param changed true if tag is changed
	 */
	void markTag2Changed(bool changed = true) { m_changedV2 = changed; }

	/**
	 * Get the truncation flags.
	 * @return truncation flags.
	 */
	unsigned getTruncationFlags() const { return m_truncation; }

	/**
	 * Format a time string "h:mm:ss".
	 * If the time is less than an hour, the hour is not put into the
	 * string and the minute is not padded with zeroes.
	 *
	 * @param seconds time in seconds
	 *
	 * @return string with the time in hours, minutes and seconds.
	 */
	static QString formatTime(unsigned seconds);

protected:
	/**
	 * Rename a file.
	 * This methods takes care of case insensitive filesystems.
	 *
	 * @param fnOld old filename
	 * @param fnNew new filename
	 *
	 * @return true if ok.
	 */
	bool renameFile(const QString& fnOld, const QString& fnNew) const;

	/**
	 * Get field name for comment from configuration.
	 *
	 * @return field name.
	 */
	QString getCommentFieldName() const;

 /**
	* Get the total number of tracks if it is enabled.
	*
	* @return total number of tracks,
	*         -1 if disabled or unavailable.
	*/
	int getTotalNumberOfTracksIfEnabled() const;

	/**
	 * Remove the standard ID3v1 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	void removeStandardTagsV1(const StandardTagsFilter& flt);

	/**
	 * Remove the standard ID3v2 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	void removeStandardTagsV2(const StandardTagsFilter& flt);

	/**
	 * Get current filename.
	 * @return existing name.
	 */
	const QString& currentFilename() const { return m_filename; }

	/**
	 * Set current filename to new filename.
	 */
	void updateCurrentFilename() { m_filename = m_newFilename; }

	/**
	 * Check if tag 1 was changed.
	 * @return true if tag 1 was changed.
	 */
	bool isTag1Changed() const { return m_changedV1; }

	/**
	 * Mark tag 1 as changed.
	 * @param changed true if tag is changed
	 */
	void markTag1Changed(bool changed = true) {
		m_changedV1 = changed;
		if (!m_changedV1) clearTrunctionFlags();
	}

	/**
	 * Check if a string has to be truncated.
	 *
	 * @param str  string to be checked
	 * @param flag flag to be set if string has to be truncated
	 * @param len  maximum length of string
	 *
	 * @return str truncated to len characters if necessary, else QString::null.
	 */
	QString checkTruncation(const QString& str,
													StandardTags::TruncationFlag flag, int len = 30);

	/**
	 * Check if a number has to be truncated.
	 *
	 * @param val  value to be checked
	 * @param flag flag to be set if number has to be truncated
	 * @param max  maximum value
	 *
	 * @return val truncated to max if necessary, else -1.
	 */
	int checkTruncation(int val, StandardTags::TruncationFlag flag,
											int max = 255);

	/**
	 * Clear all truncation flags.
	 */
	void clearTrunctionFlags() { m_truncation = 0; }

private:
	TaggedFile(const TaggedFile&);
	TaggedFile& operator=(const TaggedFile&);

	/** Directory information */
	const DirInfo* m_dirInfo;
	/** File name */
	QString m_filename;
	/** New file name */
	QString m_newFilename;
	/** true if ID3v1 tags were changed */
	bool m_changedV1;
	/** true if ID3v2 tags were changed */
	bool m_changedV2;
	/** Truncation flags. */
	unsigned m_truncation;
};

#endif // TAGGEDFILE_H
