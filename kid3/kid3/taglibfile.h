/**
 * \file taglibfile.h
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 */

#ifndef TAGLIBFILE_H
#define TAGLIBFILE_H

#include "config.h"
#ifdef HAVE_TAGLIB

#include "taggedfile.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <qglobal.h>

/** TagLib version in with 8 bits for major, minor and patch version. */
#define TAGLIB_VERSION (((TAGLIB_MAJOR_VERSION) << 16) + \
												((TAGLIB_MINOR_VERSION) << 8) + (TAGLIB_PATCH_VERSION))

#if (TAGLIB_VERSION <= 0x010400)
/** Defined if TagLib::Ogg::XiphComment::removeField() crashes */
#define TAGLIB_XIPHCOMMENT_REMOVEFIELD_CRASHES
//#else
//#define TAGLIB_SUPPORTS_GEOB_FRAMES
//#define TAGLIB_SUPPORTS_URLLINK_FRAMES
//#define TAGLIB_SUPPORTS_USLT_FRAMES
#endif

#if QT_VERSION >= 0x040000
#define QSTRING_TO_TSTRING(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8)
#else
#define QSTRING_TO_TSTRING(s) TagLib::String(s.utf8().data(), TagLib::String::UTF8)
#endif


/** List box item containing tagged file. */
class TagLibFile : public TaggedFile {
public:
	/**
	 * Constructor.
	 *
	 * @param di directory information
	 * @param fn filename
	 */
	TagLibFile(const DirInfo* di, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~TagLibFile();

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
	 * Remove all ID3v1 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	virtual void removeTagsV1(const StandardTagsFilter& flt);

	/**
	 * Remove all ID3v2 tags.
	 *
	 * @param flt filter specifying which fields to remove
	 */
	virtual void removeTagsV2(const StandardTagsFilter& flt);

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
	 * Check if file has an ID3v1 tag.
	 *
	 * @return true if a V1 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV1() const;

	/**
	 * Check if file has an ID3v2 tag.
	 *
	 * @return true if a V2 tag is available.
	 * @see isTagInformationRead()
	 */
	virtual bool hasTagV2() const;

	/**
	 * Check if ID3v1 tags are supported by the format of this file.
	 *
	 * @return true.
	 */
	virtual bool isTagV1Supported() const;

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
	 * Get file extension including the dot.
	 *
	 * @return file extension ".mp3".
	 */
	virtual QString getFileExtension() const;

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

private:
	TagLibFile(const TagLibFile&);
	TagLibFile& operator=(const TagLibFile&);

	/**
	 * Create m_tagV1 if it does not already exist so that it can be set.
	 *
	 * @return true if m_tagV1 can be set.
	 */
	bool makeTagV1Settable();

	/**
	 * Create m_tagV2 if it does not already exist so that it can be set.
	 *
	 * @return true if m_tagV2 can be set.
	 */
	bool makeTagV2Settable();

	TagLib::FileRef m_fileRef; /**< file reference */
	TagLib::Tag* m_tagV1;      /**< ID3v1 tags */
	TagLib::Tag* m_tagV2;      /**< ID3v2 tags */
	bool m_fileRead;           /**< true if file has been read */
};

#endif // HAVE_TAGLIB

#endif // TAGLIBFILE_H
