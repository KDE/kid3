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
#include <qlistbox.h>
#include <qpixmap.h>
class QListBox;
class QPainter;
class StandardTags;
class StandardTagsFilter;
class FrameList;

/** List box item containing tagged file */
class TaggedFile : public QListBoxItem {
public:
	/**
	 * Constructor.
	 *
	 * @param dn directory name
	 * @param fn filename
	 */
	TaggedFile(const QString& dn, const QString& fn);

	/**
	 * Destructor.
	 */
	virtual ~TaggedFile();

	/**
	 * Set file name.
	 *
	 * @param fn file name
	 */
	void setFilename(const QString& fn) { new_filename = fn; }

	/**
	 * Get file name.
	 *
	 * @return file name
	 */
	QString getFilename(void) const { return new_filename; }

	/**
	 * Get directory name.
	 *
	 * @return directory name
	 */
	QString getDirname(void) const { return dirname; }

	/**
	 * Read tags from file.
	 *
	 * @param force TRUE to force reading even if tags were already read.
	 */
	virtual void readTags(bool force) = 0;

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
	virtual bool writeTags(bool force, bool *renamed, bool preserve) = 0;

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
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getGenreNumV1();

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
	 * Set ID3v1 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	virtual void setGenreNumV1(int num);

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
	virtual QString getTitleV2(void) = 0;

	/**
	 * Get ID3v2 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getArtistV2(void) = 0;

	/**
	 * Get ID3v2 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getAlbumV2(void) = 0;

	/**
	 * Get ID3v2 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	virtual QString getCommentV2(void) = 0;

	/**
	 * Get ID3v2 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getYearV2(void) = 0;

	/**
	 * Get ID3v2 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getTrackNumV2(void) = 0;

	/**
	 * Get ID3v2 genre.
	 *
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	virtual int getGenreNumV2(void) = 0;

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
	 * Set ID3v2 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	virtual void setGenreNumV2(int num) = 0;

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
	 * Get frame list for this type of tagged file.
	 *
	 * @return frame list.
	 */
	virtual FrameList* getFrameList() const = 0;

	/**
	 * Get file extension including the dot.
	 *
	 * @return file extension, e.g. ".mp3".
	 */
	virtual QString getFileExtension() const = 0;

	/**
	 * Get ID3v1 tags from file.
	 *
	 * @param st tags to put result
	 */
	void getStandardTagsV1(StandardTags *st);

	/**
	 * Get ID3v2 tags from file.
	 *
	 * @param st tags to put result
	 */
	void getStandardTagsV2(StandardTags *st);

	/**
	 * Set ID3v1 tags.
	 *
	 * @param st tags to set
	 * @param flt filter specifying which fields to set
	 */
	void setStandardTagsV1(const StandardTags *st,
												 const StandardTagsFilter& flt);
	/**
	 * Set ID3v2 tags.
	 *
	 * @param st tags to set
	 * @param flt filter specifying which fields to set
	 */
	void setStandardTagsV2(const StandardTags *st,
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
	void getTagsFromFilename(StandardTags *st, QString fmt);

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
	QString formatWithTags(const StandardTags *st, QString fmt,
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
	void getFilenameFromTags(const StandardTags *st, QString fmt);

	/**
	 * Check if file is changed.
	 *
	 * @return TRUE if file was changed.
	 */
	bool isChanged(void) const { return changedV1 || changedV2 ||
																 new_filename != filename; }

	/**
	 * Mark file as selected.
	 *
	 * @param val TRUE to set file selected.
	 */
	void setInSelection(bool val) { selected = val; }

	/**
	 * Check if file is marked selected.
	 *
	 * @return TRUE if file is marked selected.
	 */
	bool isInSelection(void) { return selected; }

	/**
	 * Get height of item.
	 *
	 * @param lb listbox containing the item
	 *
	 * @return height.
	 */
	int height(const QListBox* lb) const;

	/**
	 * Get width of item.
	 *
	 * @param lb listbox containing the item
	 *
	 * @return width.
	 */
	int width(const QListBox* lb) const;

	/**
	 * Get absolute filename.
	 *
	 * @return absolute file path.
	 */
	QString getAbsFilename(void) const;

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

	/** TRUE if ID3v2 tags were changed */
	bool changedV2;

protected:
	/**
	 * Paint item.
	 *
	 * @param painter painter used
	 */
	void paint(QPainter *painter);

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

	/** Directory name */
	const QString& dirname;
	/** File name */
	const QString filename;
	/** New file name */
	QString new_filename;
	/** TRUE if ID3v1 tags were changed */
	bool changedV1;

private:
	/** TRUE if file is marked selected */
	bool selected;
	/** pointer to pixmap for modified file */
	static QPixmap *modifiedPixmap;
	/** pointer to empty pixmap */
	static QPixmap *nullPixmap;
	/** pointer to V1V2 pixmap */
	static QPixmap *v1v2Pixmap;
	/** pointer to V1 pixmap */
	static QPixmap *v1Pixmap;
	/** pointer to V2 pixmap */
	static QPixmap *v2Pixmap;
	/** pointer to "no tag" pixmap */
	static QPixmap *notagPixmap;
};

#endif // TAGGEDFILE_H
