/**
 * \file taggedfile.h
 * Base class for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
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

#ifndef TAGGEDFILE_H
#define TAGGEDFILE_H

#include <qstring.h>
#include <qstringlist.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QList>
#else
#include <qvaluelist.h>
#endif
#include "frame.h"

class DirInfo;

/** Base class for tagged files. */
class TaggedFile {
public:
	/** Abstract base class for pluggable file type resolution. */
	class Resolver {
	public:
		/**
		 * Constructor.
		 */
		Resolver() {}

		/**
		 * Destructor.
		 */
		virtual ~Resolver() {}

		/**
		 * Create a TaggedFile subclass depending on the file extension.
		 *
		 * @param di directory information
		 * @param fn filename
		 *
		 * @return tagged file, 0 if type not supported.
		 */
		virtual TaggedFile* createFile(const DirInfo* di, const QString& fn) const = 0;

		/**
		 * Get a list with all extensions (e.g. ".mp3") supported by TaggedFile subclass.
		 *
		 * @return list of file extensions.
		 */
		virtual QStringList getSupportedFileExtensions() const = 0;
	};


	/** Information about file. */
	struct DetailInfo {
		/** Channel mode. */
		enum ChannelMode { CM_None, CM_Stereo, CM_JointStereo };

		/** Constructor. */
		DetailInfo() : valid(false), vbr(false), channelMode(CM_None), channels(0),
									 sampleRate(0), bitrate(0), duration(0) {}

		bool valid;              /**< true if information valid */
		bool vbr;                /**< true if variable bitrate */
		QString format;          /**< format description */
		ChannelMode channelMode; /**< channel mode */
		unsigned channels;       /**< number of channels > 0 */
		unsigned sampleRate;     /**< sample rate in Hz > 0 */
		unsigned bitrate;        /**< 0 < bitrate in kbps < 999 */
		unsigned long duration;  /**< duration in seconds > 0 */
	};


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
	 * Remove ID3v1 frames.
	 *
	 * @param flt filter specifying which frames to remove
	 */
	virtual void deleteFramesV1(const FrameFilter& flt);

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
	 * Remove ID3v2 frames.
	 *
	 * @param flt filter specifying which frames to remove
	 */
	virtual void deleteFramesV2(const FrameFilter& flt);

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
	 * @param info the detail information is returned here
	 */
	virtual void getDetailInfo(DetailInfo& info) const = 0;

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
	 * @param frame frame to set
	 *
	 * @return true if ok.
	 */
	virtual bool setFrameV2(const Frame& frame);

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
	 * Get a list of frame IDs which can be added.
	 *
	 * @return list with frame IDs.
	 */
	virtual QStringList getFrameIds() const = 0;

	/**
	 * Get all frames in tag 1.
	 *
	 * @param frames frame collection to set.
	 */
	virtual void getAllFramesV1(FrameCollection& frames);

	/**
	 * Get all frames in tag 2.
	 * This generic implementation only supports the standard tags and should
	 * be reimplemented in derived classes.
	 *
	 * @param frames frame collection to set.
	 */
	virtual void getAllFramesV2(FrameCollection& frames);

	/**
	 * Set frames in tag 1.
	 *
	 * @param frames      frame collection
	 * @param onlyChanged only frames with value marked as changed are set
	 */
	void setFramesV1(const FrameCollection& frames, bool onlyChanged = true);

	/**
	 * Set frames in tag 2.
	 *
	 * @param frames      frame collection
	 * @param onlyChanged only frames with value marked as changed are set
	 */
	void setFramesV2(const FrameCollection& frames, bool onlyChanged = true);

	/**
	 * Get tags from filename.
	 * Supported formats:
	 * album/track - artist - song
	 * artist - album/track song
	 * /artist - album - track - song
	 * album/artist - track - song
	 * artist/album/track song
	 * album/artist - song
	 *
	 * @param frames frames to put result
	 * @param fmt format string containing the following codes:
	 *            %s title (song)
	 *            %l album
	 *            %a artist
	 *            %c comment
	 *            %y year
	 *            %t track
	 */
	void getTagsFromFilename(FrameCollection& frames, const QString& fmt);

	/**
	 * Create string with tags according to format string.
	 *
	 * @param frames    frames to use to build filename
	 * @param str       format string containing codes supported by
	 *                  FrameFormatReplacer::getReplacement()
	 * @param isDirname true to generate a directory name
	 *
	 * @return format string with format codes replaced by tags.
	 */
	QString formatWithTags(const FrameCollection& frames, QString str,
	                       bool isDirname = false) const;

	/**
	 * Get filename from tags.
	 *
	 * @param frames    frames to use to build filename
	 * @param fmt       format string containing codes supported by
	 *                  FrameFormatReplacer::getReplacement()
	 */
	void getFilenameFromTags(const FrameCollection& frames, QString fmt);

	/**
	 * Check if file is changed.
	 *
	 * @return true if file was changed.
	 */
	bool isChanged() const { return m_changedV1 || m_changedV2 ||
			m_newFilename != m_filename; }

	/**
	 * Check if filename is changed.
	 *
	 * @return true if filename was changed.
	 */
	bool isFilenameChanged() const { return m_newFilename != m_filename; }

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
	 *
	 * @param type type of changed frame
	 */
	void markTag2Changed(Frame::Type type);

	/**
	 * Mark tag 2 as unchanged.
	 */
	void markTag2Unchanged() { m_changedV2 = false; m_changedFramesV2 = 0; }

	/**
	 * Get the mask of the frame types changed in tag 1.
	 * @return mask of frame types.
	 */
	unsigned long getChangedFramesV1() const { return m_changedFramesV1; }

	/**
	 * Get the mask of the frame types changed in tag 2.
	 * @return mask of frame types.
	 */
	unsigned long getChangedFramesV2() const { return m_changedFramesV2; }

	/**
	 * Get the truncation flags.
	 * @return truncation flags.
	 */
	unsigned getTruncationFlags() const { return m_truncation; }

	/**
	 * Format the track number (digits, total number of tracks) if enabled.
	 *
	 * @param value    string containing track number, will be modified
	 * @param addTotal true to add total number of tracks if enabled
	 *                 "/t" with t = total number of tracks will be appended
	 *                 if enabled and value contains a number
	 */
	void formatTrackNumberIfEnabled(QString& value, bool addTotal) const;

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

	/**
	 * Add a file type resolver to the end of a list of resolvers.
	 *
	 * @param resolver file type resolver to add
	 */
	static void addResolver(const Resolver* resolver);

	/**
	 * Create a TaggedFile subclass using the first successful resolver.
	 * @see addResolver()
	 *
	 * @param di directory information
	 * @param fn filename
	 *
	 * @return tagged file, 0 if type not supported.
	 */
	static TaggedFile* createFile(const DirInfo* di, const QString& fn);

	/**
	 * Get a list with all extensions (e.g. ".mp3") supported by the resolvers.
	 * @see addResolver()
	 *
	 * @return list of file extensions.
	 */
	static QStringList getSupportedFileExtensions();

	/**
	 * Free static resources.
	 */
	static void staticCleanup();

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
	 * Get the number of track number digits configured.
	 *
	 * @return track number digits,
	 *         1 if invalid or unavailable.
	 */
	int getTrackNumberDigits() const;

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
	 *
	 * @param type type of changed frame
	 */
	void markTag1Changed(Frame::Type type);

	/**
	 * Mark tag 1 as unchanged.
	 */
	void markTag1Unchanged() {
		m_changedV1 = false;
		m_changedFramesV1 = 0;
		clearTrunctionFlags();
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
													unsigned flag, int len = 30);

	/**
	 * Check if a number has to be truncated.
	 *
	 * @param val  value to be checked
	 * @param flag flag to be set if number has to be truncated
	 * @param max  maximum value
	 *
	 * @return val truncated to max if necessary, else -1.
	 */
	int checkTruncation(int val, unsigned flag,
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
	/** changed tag 1 frame types */
	unsigned long m_changedFramesV1;
	/** true if ID3v2 tags were changed */
	bool m_changedV2;
	/** changed tag 2 frame types */
	unsigned long m_changedFramesV2;
	/** Truncation flags. */
	unsigned m_truncation;

#if QT_VERSION >= 0x040000
	static QList<const Resolver*> s_resolvers;
#else
	static QValueList<const Resolver*> s_resolvers;
#endif
};

#endif // TAGGEDFILE_H
