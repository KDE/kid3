/**
 * \file flacfile.hpp
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
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

#ifndef FLACFILE_H
#define FLACFILE_H

#include "config.h"
#ifdef HAVE_FLAC

#include "oggfile.hpp"

namespace FLAC {
	namespace Metadata {
		class Chain;
		class VorbisComment;
		class StreamInfo;
		class Picture;
	};
};

 /** List box item containing FLAC file */
class FlacFile : public OggFile {
public:
	/** File type resolution. */
	class Resolver : public TaggedFile::Resolver {
		/**
		 * Create an FlacFile object if it supports the filename's extension.
		 *
		 * @param di directory information
		 * @param fn filename
		 *
		 * @return tagged file, 0 if type not supported.
		 */
		virtual TaggedFile* createFile(const DirInfo* di, const QString& fn) const;

		/**
		 * Get a list with all extensions supported by FlacFile.
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
	FlacFile(const DirInfo* di, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~FlacFile();

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
	 * @return file extension ".flac".
	 */
	virtual QString getFileExtension() const;

#ifdef HAVE_FLAC_PICTURE
	/**
	 * Check if file has an ID3v2 tag.
	 *
	 * @return true if a V2 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV2() const;

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
	 * Remove ID3v2 frames.
	 *
	 * @param flt filter specifying which frames to remove
	 */
	virtual void deleteFramesV2(const FrameFilter& flt);

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
#endif // HAVE_FLAC_PICTURE

private:
	FlacFile(const FlacFile&);
	FlacFile& operator=(const FlacFile&);

	/** Information about a FLAC file. */
	struct FileInfo {
		/**
		 * Read information about a FLAC file.
		 * @param si stream info
		 * @return true if ok.
		 */
		bool read(FLAC::Metadata::StreamInfo* si);

		bool valid;             /**< true if read() was successful */
		unsigned channels;      /**< number of channels */
		unsigned sampleRate;    /**< sample rate in Hz */
		unsigned long bitrate;  /**< bitrate in bits/s */
		unsigned long duration; /**< duration in seconds */
	};

	/**
	 * Set the vorbis comment block with the comments.
	 *
	 * @param vc vorbis comment block to set
	 */
	void setVorbisComment(FLAC::Metadata::VorbisComment* vc);

#ifdef HAVE_FLAC_PICTURE
	/** Pictures */
#if QT_VERSION >= 0x040000
	typedef QList<Frame> PictureList;
#else
	typedef QValueList<Frame> PictureList;
#endif
	PictureList m_pictures;
#endif // HAVE_FLAC_PICTURE

	/** Info about file. */
	FileInfo m_fileInfo;

	/** FLAC metadata chain. */
	FLAC::Metadata::Chain* m_chain;
};

#endif // HAVE_FLAC

#endif // FLACFILE_H
