/**
 * \file mp3file.h
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef MP3FILE_H
#define MP3FILE_H

#include "taggedfile.h"
#include <id3/globals.h> /* ID3_FrameID */
class ID3_Tag;
class ID3_Field;
class Mp3FrameList;

/** List box item containing MP3 file */
class Mp3File : public TaggedFile {
public:
	/**
	 * Constructor.
	 *
	 * @param dn directory name
	 * @param fn filename
	 */
	Mp3File(const QString& dn, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~Mp3File();

	/**
	 * Read tags from file.
	 *
	 * @param force TRUE to force reading even if tags were already read.
	 */
	virtual void readTags(bool force);

	/**
	 * Write tags to file and rename it if necessary.
	 *
	 * @param force   TRUE to force writing even if file was not changed.
	 * @param renamed will be set to TRUE if the file was renamed,
	 *                i.e. the file name is no longer valid, else *renamed
	 *                is left unchanged
	 * @param preserve true to preserve file time stamps
	 *
	 * @return TRUE if ok, FALSE if the file could not be written or renamed.
	 */
	virtual bool writeTags(bool force, bool *renamed, bool preserve);

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
	virtual QString getTitleV1(void);

	/**
	 * Get ID3v1 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV1(void);

	/**
	 * Get ID3v1 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV1(void);

	/**
	 * Get ID3v1 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV1(void);

	/**
	 * Get ID3v1 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV1(void);

	/**
	 * Get ID3v1 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV1(void);

	/**
	 * Get ID3v1 genre.
	 *
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getGenreNumV1(void);

	/**
	 * Get ID3v2 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getTitleV2(void);

	/**
	 * Get ID3v2 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV2(void);

	/**
	 * Get ID3v2 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV2(void);

	/**
	 * Get ID3v2 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV2(void);

	/**
	 * Get ID3v2 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV2(void);

	/**
	 * Get ID3v2 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV2(void);

	/**
	 * Get ID3v2 genre.
	 *
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getGenreNumV2(void);

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
	 * Set ID3v1 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	virtual void setGenreNumV1(int num);

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
	 * Set ID3v2 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	virtual void setGenreNumV2(int num);

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
	 * Get frame list for this type of tagged file.
	 *
	 * @return frame list.
	 */
	virtual FrameList* getFrameList() const;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension ".mp3".
	 */
	virtual QString getFileExtension() const;

	/**
	 * Get string from text field.
	 *
	 * @param field field
	 *
	 * @return string,
	 *         "" if the field does not exist.
	 */
	static QString getString(ID3_Field* field);

	/**
	 * Set string in text field.
	 *
	 * @param field        field
	 * @param text         text to set
	 */
	static void setString(ID3_Field* field, const QString &text);

	/**
	 * Clean up static resources.
	 */
	static void staticCleanup();

	friend class Mp3FrameList;

private:
	/**
	 * Get text field.
	 *
	 * @param tag ID3 tag
	 * @param id  frame ID
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	static QString getTextField(const ID3_Tag *tag, ID3_FrameID id);

	/**
	 * Get year.
	 *
	 * @param tag ID3 tag
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	static int getYear(const ID3_Tag *tag);

	/**
	 * Get track.
	 *
	 * @param tag ID3 tag
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	static int getTrackNum(const ID3_Tag *tag);

	/**
	 * Get genre.
	 *
	 * @param tag ID3 tag
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	static int getGenreNum(const ID3_Tag *tag);

	/**
	 * Set text field.
	 *
	 * @param tag          ID3 tag
	 * @param id           frame ID
	 * @param text         text to set
	 * @param allowUnicode true to allow setting of Unicode encoding if necessary
	 * @param replace      true to replace an existing field
	 * @param removeEmpty  true to remove a field if text is empty
	 *
	 * @return true if the field was changed.
	 */
	static bool setTextField(ID3_Tag *tag, ID3_FrameID id, const QString &text,
							 bool allowUnicode = false, bool replace = true,
							 bool removeEmpty = true);

	/**
	 * Set year.
	 *
	 * @param tag ID3 tag
	 * @param num number to set, 0 to remove field.
	 *
	 * @return true if the field was changed.
	 */
	static bool setYear(ID3_Tag *tag, int num);

	/**
	 * Set track.
	 *
	 * @param tag ID3 tag
	 * @param num number to set, 0 to remove field.
	 * @param numTracks total number of tracks, -1 to ignore
	 *
	 * @return true if the field was changed.
	 */
	static bool setTrackNum(ID3_Tag *tag, int num, int numTracks = -1);

	/**
	 * Set genre.
	 *
	 * @param tag ID3 tag
	 * @param num number to set, 0xff to remove field.
	 *
	 * @return true if the field was changed.
	 */
	static bool setGenreNum(ID3_Tag *tag, int num);

	/** ID3v1 tags */
	ID3_Tag *tagV1;

	/** ID3v2 tags */
	ID3_Tag *tagV2;

	/** Frame list for MP3 files. */
	static Mp3FrameList* s_mp3FrameList;
};

#endif // MP3FILE_H
