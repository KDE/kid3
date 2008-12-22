/**
 * \file oggfile.hpp
 * Handling of Ogg files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
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

#ifndef OGGFILE_H
#define OGGFILE_H

#include "config.h"
#if defined HAVE_VORBIS || defined HAVE_FLAC

#include "taggedfile.h"
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QList>
#else
#include <qvaluelist.h>
#endif


 /** List box item containing OGG file */
class OggFile : public TaggedFile {
public:
	/** File type resolution. */
	class Resolver : public TaggedFile::Resolver {
		/**
		 * Create an OggFile object if it supports the filename's extension.
		 *
		 * @param di directory information
		 * @param fn filename
		 *
		 * @return tagged file, 0 if type not supported.
		 */
		virtual TaggedFile* createFile(const DirInfo* di, const QString& fn) const;

		/**
		 * Get a list with all extensions supported by OggFile.
		 *
		 * @return list of file extensions.
		 */
		virtual QStringList getSupportedFileExtensions() const;
	};


	/**
	 * Constructor.
	 *
	 * @param di directory information
	 * @param fn filename
	 */
	OggFile(const DirInfo* di, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~OggFile();

	/**
	 * Read tags from file.
	 *
	 * @param force true to force reading even if tags were already read.
	 */
	virtual void readTags(bool force);

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
	virtual bool writeTags(bool force, bool* renamed, bool preserve);

	/**
	 * Remove ID3v2 frames.
	 *
	 * @param flt filter specifying which frames to remove
	 */
	virtual void deleteFramesV2(const FrameFilter& flt);

	/**
	 * Get ID3v2 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getTitleV2();

	/**
	 * Get ID3v2 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV2();

	/**
	 * Get ID3v2 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV2();

	/**
	 * Get ID3v2 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV2();

	/**
	 * Get ID3v2 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV2();

	/**
	 * Get ID3v2 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV2();

	/**
	 * Get ID3v2 genre as text.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getGenreV2();

	/**
	 * Set ID3v2 title.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setTitleV2(const QString& str);

	/**
	 * Set ID3v2 artist.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setArtistV2(const QString& str);

	/**
	 * Set ID3v2 album.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setAlbumV2(const QString& str);

	/**
	 * Set ID3v2 comment.
	 *
	 * @param str string to set, "" to remove field.
	 */
	virtual void setCommentV2(const QString& str);

	/**
	 * Set ID3v2 year.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setYearV2(int num);

	/**
	 * Set ID3v2 track.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	virtual void setTrackNumV2(int num);

	/**
	 * Set ID3v2 genre as text.
	 *
	 * @param str string to set, "" to remove field, QString::null to ignore.
	 */
	virtual void setGenreV2(const QString& str);

	/**
	 * Check if tag information has already been read.
	 *
	 * @return true if information is available,
	 *         false if the tags have not been read yet, in which case
	 *         hasTagV1() and hasTagV2() do not return meaningful information.
	 */
	virtual bool isTagInformationRead() const;

	/**
	 * Check if file has an ID3v2 tag.
	 *
	 * @return true if a V2 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV2() const;

	/**
	 * Get technical detail information.
	 *
	 * @param info the detail information is returned here
	 */
	virtual void getDetailInfo(DetailInfo& info) const;

	/**
	 * Get duration of file.
	 *
	 * @return duration in seconds,
	 *         0 if unknown.
	 */
	virtual unsigned getDuration() const;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension ".ogg".
	 */
	virtual QString getFileExtension() const;

	/**
	 * Get the format of tag 2.
	 *
	 * @return "Vorbis".
	 */
	virtual QString getTagFormatV2() const;

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
	 * @param frame frame to add
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
	 * Get all frames in tag 2.
	 *
	 * @param frames frame collection to set.
	 */
	virtual void getAllFramesV2(FrameCollection& frames);

	/**
	 * Get a list of frame IDs which can be added.
	 *
	 * @return list with frame IDs.
	 */
	virtual QStringList getFrameIds() const;

protected:
	/** Vorbis comment field. */
	class CommentField {
	public:
		/** Constructor. */
		CommentField(const QString& name = QString::null,
								 const QString& value = QString::null) :
			m_name(name), m_value(value) {}
		/** Destructor. */
		~CommentField() {}
		/**
		 * Get name.
		 * @return name.
		 */
		QString getName() const { return m_name; }
		/**
		 * Get value.
		 * @return value.
		 */
		QString getValue() const { return m_value; }
		/**
		 * Set value.
		 * @param value value
		 */
		void setValue(const QString& value) { m_value = value; }

	private:
		QString m_name;
		QString m_value;
	};

	/** Vorbis comment list. */
	class CommentList : public
#if QT_VERSION >= 0x040000
	QList<CommentField>
#else
	QValueList<CommentField>
#endif
	{
	public:
		/** Constructor. */
		CommentList() {}
		/** Destructor. */
		~CommentList() {}
		/**
		 * Get value.
		 * @param name name
		 * @return value, "" if not found.
		 */
		QString getValue(const QString& name) const;
		/**
		 * Set value.
		 * @param name name
		 * @param value value
		 * @return true if value was changed.
		 */
		bool setValue(const QString& name, const QString& value);
	};

	/**
	 * Get text field.
	 *
	 * @param name name
	 * @return value, "" if not found,
	 *         QString::null if the tags have not been read yet.
	 */
	QString getTextField(const QString& name) const;

	/**
	 * Set text field.
	 * If value is null or the tags have not been read yet, nothing is changed.
	 * If value is different from the current value, changedV2 is set.
	 *
	 * @param name name
	 * @param value value, "" to remove, QString::null to do nothing
	 * @param type frame type
	 */
	void setTextField(const QString& name, const QString& value,
	                  Frame::Type type);

	/** Comments of this file. */
	CommentList m_comments;
	/** true if file has been read. */
	bool m_fileRead;
	/** true if comments are changed. */
	bool m_changed;

private:
	OggFile(const OggFile&);
	OggFile& operator=(const OggFile&);

#ifdef HAVE_VORBIS
	/** Information about Ogg/Vorbis file. */
	struct FileInfo {
		/**
		 * Read information about an Ogg/Vorbis file.
		 * @param fn file name
		 * @return true if ok.
		 */
		bool read(const char* fn);

		bool valid;      /**< true if read() was successful */
		int version;     /**< vorbis encoder version */
		int channels;    /**< number of channels */
		long sampleRate; /**< sample rate in Hz */
		long bitrate;    /**< bitrate in bits/s */
		long duration;   /**< duration in seconds */
	};

	/** Info about file. */
	FileInfo m_fileInfo;
#endif // HAVE_VORBIS
};

#endif // HAVE_VORBIS || define HAVE_FLAC

#endif // OGGFILE_H
