/**
 * \file flacfile.h
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 */

#ifndef FLACFILE_H
#define FLACFILE_H

#include "config.h"
#ifdef HAVE_FLAC

#include "oggfile.h"

class FlacFrameList;
namespace FLAC {
	namespace Metadata {
		class Chain;
		class VorbisComment;
		class StreamInfo;
	};
};

 /** List box item containing FLAC file */
class FlacFile : public OggFile {
public:
	/**
	 * Constructor.
	 *
	 * @param dn directory name
	 * @param fn filename
	 */
	FlacFile(const QString& dn, const QString& fn);

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
	 * @return string with detail information,
	 *         "" if no information available.
	 */
	virtual QString getDetailInfo() const;

	/**
	 * Get duration of file.
	 *
	 * @return duration in seconds,
	 *         0 if unknown.
	 */
	virtual unsigned getDuration() const;

	/**
	 * Get frame list for this type of tagged file.
	 *
	 * @return frame list.
	 */
	virtual FrameList* getFrameList() const;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension ".flac".
	 */
	virtual QString getFileExtension() const;

	/**
	 * Clean up static resources.
	 */
	static void staticCleanup();

	friend class FlacFrameList;

private:
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

	/** Info about file. */
	FileInfo m_fileInfo;

	/** Frame list for FLAC files. */
	static FlacFrameList* s_flacFrameList;

	/** FLAC metadata chain. */
	FLAC::Metadata::Chain* m_chain;
};

#endif // HAVE_FLAC

#endif // FLACFILE_H
