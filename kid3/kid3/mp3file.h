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

#include <qstring.h>
#include <qlistbox.h>
#include <id3/globals.h> /* ID3_FrameID */
class QListBox;
class ID3_Tag;
class ID3_Field;
class StandardTags;

 /** List box item containing MP3 file */
class Mp3File : public QListBoxText {
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
	 * Append asterisk to text if file was changed.
	 */
	void refreshText(void);
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
	 * Read tags from file.
	 *
	 * @param force TRUE to force reading even if tags were already read.
	 */
	void readTags(bool force);
	/**
	 * Write tags to file and rename it if necessary.
	 *
	 * @param force TRUE to force writing even if file was not changed.
	 *
	 * @return TRUE if the file was renamed, i.e. the file name is no longer valid.
	 */
	bool writeTags(bool force);
	/**
	 * Remove all ID3v1 tags.
	 */
	void removeTagsV1(void);
	/**
	 * Remove all ID3v2 tags.
	 */
	void removeTagsV2(void);
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
	 * Set string in text field.
	 *
	 * @param field        field
	 * @param text         text to set
	 */
	static void setString(ID3_Field* field, const QString &text);
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
	 *
	 * @return true if the field was changed.
	 */
	static bool setTrackNum(ID3_Tag *tag, int num);
	/**
	 * Set genre.
	 *
	 * @param tag ID3 tag
	 * @param num number to set, 0xff to remove field.
	 *
	 * @return true if the field was changed.
	 */
	static bool setGenreNum(ID3_Tag *tag, int num);
	/**
	 * Get ID3v1 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getTitleV1(void);
	/**
	 * Get ID3v1 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getArtistV1(void);
	/**
	 * Get ID3v1 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getAlbumV1(void);
	/**
	 * Get ID3v1 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getCommentV1(void);
	/**
	 * Get ID3v1 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	int getYearV1(void);
	/**
	 * Get ID3v1 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	int getTrackNumV1(void);
	/**
	 * Get ID3v1 genre.
	 *
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	int getGenreNumV1(void);
	/**
	 * Get ID3v2 title.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getTitleV2(void);
	/**
	 * Get ID3v2 artist.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getArtistV2(void);
	/**
	 * Get ID3v2 album.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getAlbumV2(void);
	/**
	 * Get ID3v2 comment.
	 *
	 * @return string,
	 *         "" if the field does not exist,
	 *         QString::null if the tags do not exist.
	 */
	QString getCommentV2(void);
	/**
	 * Get ID3v2 year.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	int getYearV2(void);
	/**
	 * Get ID3v2 track.
	 *
	 * @return number,
	 *         0 if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	int getTrackNumV2(void);
	/**
	 * Get ID3v2 genre.
	 *
	 * @return number,
	 *         0xff if the field does not exist,
	 *         -1 if the tags do not exist.
	 */
	int getGenreNumV2(void);
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
	 * Set ID3v1 title.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setTitleV1(const QString& str);
	/**
	 * Set ID3v1 artist.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setArtistV1(const QString& str);
	/**
	 * Set ID3v1 album.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setAlbumV1(const QString& str);
	/**
	 * Set ID3v1 comment.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setCommentV1(const QString& str);
	/**
	 * Set ID3v1 year.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	void setYearV1(int num);
	/**
	 * Set ID3v1 track.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	void setTrackNumV1(int num);
	/**
	 * Set ID3v1 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	void setGenreNumV1(int num);
	/**
	 * Set ID3v2 title.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setTitleV2(const QString& str);
	/**
	 * Set ID3v2 artist.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setArtistV2(const QString& str);
	/**
	 * Set ID3v2 album.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setAlbumV2(const QString& str);
	/**
	 * Set ID3v2 comment.
	 *
	 * @param str string to set, "" to remove field.
	 */
	void setCommentV2(const QString& str);
	/**
	 * Set ID3v2 year.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	void setYearV2(int num);
	/**
	 * Set ID3v2 track.
	 *
	 * @param num number to set, 0 to remove field.
	 */
	void setTrackNumV2(int num);
	/**
	 * Set ID3v2 genre.
	 *
	 * @param num number to set, 0xff to remove field.
	 */
	void setGenreNumV2(int num);
	/**
	 * Set ID3v1 tags.
	 *
	 * @param st tags to set
	 */
	void setStandardTagsV1(const StandardTags *st);
	/**
	 * Set ID3v2 tags.
	 *
	 * @param st tags to set
	 */
	void setStandardTagsV2(const StandardTags *st);
	/**
	 * Get tags from filename.
	 * Supported formats:
	 * artist - album/track song.mp3
	 *
	 * @param st tags to put result
	 */
	void getTagsFromFilename(StandardTags *st);
	/**
	 * Get filename from tags.
	 * Supported formats:
	 * artist - album/track song.mp3
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
	bool isChanged(void) { return changedV1 || changedV2 || new_filename != filename; }
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
	 * Update frame list box.
	 *
	 * @param lb list box
	 */
	void updateTagListV2(QListBox *lb);

	/** ID3v2 tags */
	ID3_Tag *tagV2;
	/** TRUE if ID3v2 tags were changed */
	bool changedV2;
	/** List with filename formats */
	static const char **fnFmtList;

 private:
	/**
	 * Get absolute filename.
	 *
	 * @return absolute file path.
	 */
	QString getAbsFilename(void) const;

	/** Directory name */
	const QString& dirname;
	/** File name */
	const QString filename;
	/** New file name */
	QString new_filename;
	/** ID3v1 tags */
	ID3_Tag *tagV1;
	/** TRUE if ID3v1 tags were changed */
	bool changedV1;
	/** TRUE if file is marked selected */
	bool selected;
};

#endif // MP3FILE_H
