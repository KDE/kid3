/**
 * \file mp3file.cpp
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include <qdir.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qregexp.h>

#include <id3/tag.h>
#if defined WIN32 && defined _DEBUG
#include <id3.h> /* ID3TagIterator_Delete() */
#endif

#include "standardtags.h"
#include "genres.h"
#include "mp3file.h"

#ifdef WIN32
/* ID3LIB_ symbols not found on Windows ?! */
#define UNICODE_SUPPORT_BUGGY 1
#else
#define UNICODE_SUPPORT_BUGGY ((((ID3LIB_MAJOR_VERSION) << 16) + ((ID3LIB_MINOR_VERSION) << 8) + (ID3LIB_PATCH_VERSION)) <= 0x030803)
#endif

/**
 * Constructor.
 *
 * @param dn directory name
 * @param fn filename
 */

Mp3File::Mp3File(const QString& dn, const QString& fn) :
	QListBoxText(fn), dirname(dn), filename(fn), new_filename(fn)
{
	setInSelection(FALSE);
	tagV1 = 0;
	tagV2 = 0;
	changedV1 = FALSE;
	changedV2 = FALSE;
}

/**
 * Destructor.
 */

Mp3File::~Mp3File(void)
{
	if (tagV1) {
		delete tagV1;
	}
	if (tagV2) {
		delete tagV2;
	}
}

/**
 * Append asterisk to text if file was changed.
 */

void Mp3File::refreshText(void)
{
	QString txt(filename);
	if (isChanged()) {
		txt.append(" *");
	}
	if (text() != txt) {
		setText(txt);
	}
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */

QString Mp3File::getAbsFilename(void) const
{
	QDir dir(dirname);
	return QDir::cleanDirPath(dir.absFilePath(new_filename));
}

/**
 * Read tags from file.
 *
 * @param force TRUE to force reading even if tags were already read.
 */

void Mp3File::readTags(bool force)
{
	QString absfn = dirname + QDir::separator() + filename;
	const char *fn = absfn.latin1();

	if (force && tagV1) {
		tagV1->Clear();
		tagV1->Link(fn, ID3TT_ID3V1);
		changedV1 = FALSE;
	}
	if (!tagV1) {
		tagV1 = new ID3_Tag;
		if (tagV1) {
			tagV1->Link(fn, ID3TT_ID3V1);
			changedV1 = FALSE;
		}
	}

	if (force && tagV2) {
		tagV2->Clear();
		tagV2->Link(fn, ID3TT_ID3V2);
		changedV2 = FALSE;
	}
	if (!tagV2) {
		tagV2 = new ID3_Tag;
		if (tagV2) {
			tagV2->Link(fn, ID3TT_ID3V2);
			changedV2 = FALSE;
		}
	}

	if (force) {
		new_filename = filename;
	}
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force TRUE to force writing even if file was not changed.
 *
 * @return TRUE if the file was renamed, i.e. the file name is no longer valid.
 */

bool Mp3File::writeTags(bool force)
{
	if (tagV1 && (force || changedV1)) {
		tagV1->Update(ID3TT_ID3V1);
		// There seems to be a bug in id3lib: The V1 genre is not
		// removed. So we check here and strip the whole header
		// if there are no frames.
		if (tagV1->NumFrames() == 0) {
			tagV1->Strip(ID3TT_ID3V1);
		}
		changedV1 = FALSE;
	}
	if (tagV2 && (force || changedV2)) {
		tagV2->Update(ID3TT_ID3V2);
		changedV2 = FALSE;
	}
	if ((new_filename != filename) &&
		!QFile::exists(dirname + QDir::separator() + new_filename)) {
		if (!QDir(dirname).rename(filename, new_filename)) {
			qDebug("rename(%s, %s) failed", filename.latin1(),
			       new_filename.latin1());
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Remove all ID3v1 tags.
 */

void Mp3File::removeTagsV1(void)
{
	if (tagV1) {
		ID3_Tag::Iterator* iter = tagV1->CreateIterator();
		ID3_Frame* frame;
		while ((frame = iter->GetNext()) != NULL) {
			tagV1->RemoveFrame(frame);
		}
#if defined WIN32 && defined _DEBUG
		/* just to avoid user breakpoint in VC++ */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
		changedV1 = TRUE;
	}
}

/**
 * Remove all ID3v2 tags.
 */

void Mp3File::removeTagsV2(void)
{
	if (tagV2) {
		ID3_Tag::Iterator* iter = tagV2->CreateIterator();
		ID3_Frame* frame;
		while ((frame = iter->GetNext()) != NULL) {
			tagV2->RemoveFrame(frame);
		}
#if defined WIN32 && defined _DEBUG
		/* just to avoid user breakpoint in VC++ */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
		changedV2 = TRUE;
	}
}

/**
 * Get string from text field.
 *
 * @param field field
 *
 * @return string,
 *         "" if the field does not exist.
 */

QString Mp3File::getString(ID3_Field* field)
{
	QString text("");
	if (field != NULL) {
		ID3_TextEnc enc = field->GetEncoding();
		if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
			const unicode_t *txt = field->GetRawUnicodeText();
			uint unicode_size = field->Size() / sizeof(unicode_t);
			if (unicode_size && txt) {
				QChar *qcarray = new QChar[unicode_size];
				if (qcarray) {
					// Unfortunately, Unicode support in id3lib is rather buggy
					// in the current version: The codes are mirrored.
					// In the hope that my patches will be included, I try here
					// to work around these bugs.
					uint i;
					for (i = 0; i < unicode_size; i++) {
						qcarray[i] =
							UNICODE_SUPPORT_BUGGY ?
							(ushort)(((txt[i] & 0x00ff) << 8) |
									 ((txt[i] & 0xff00) >> 8)) :
							(ushort)txt[i];
					}
					text = QString(qcarray, unicode_size);
					delete [] qcarray;
				}
			}
		} else {
			// (ID3TE_IS_SINGLE_BYTE_ENC(enc))
			// (enc == ID3TE_ISO8859_1 || enc == ID3TE_UTF8)
			text = QString(field->GetRawText());
		}
	}
	return text;
}

/**
 * Get text field.
 *
 * @param tag ID3 tag
 * @param id  frame ID
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getTextField(const ID3_Tag *tag, ID3_FrameID id)
{
	if (!tag) {
		return QString::null;
	}
	QString str("");
	ID3_Field* fld;
	ID3_Frame *frame = tag->Find(id);
	if (frame && ((fld = frame->GetField(ID3FN_TEXT)) != NULL)) {
		str = getString(fld);
	}
	return str;
}

/**
 * Get year.
 *
 * @param tag ID3 tag
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getYear(const ID3_Tag *tag)
{
	QString str = getTextField(tag, ID3FID_YEAR);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0;
	return str.toInt();
}

/**
 * Get track.
 *
 * @param tag ID3 tag
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getTrackNum(const ID3_Tag *tag)
{
	QString str = getTextField(tag, ID3FID_TRACKNUM);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0;
	return str.toInt();
}

/**
 * Get genre.
 *
 * @param tag ID3 tag
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getGenreNum(const ID3_Tag *tag)
{
	QString str = getTextField(tag, ID3FID_CONTENTTYPE);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0xff;
	int cpPos, n = 0xff;
	if ((str[0] == '(') && ((cpPos = str.find(')', 2)) > 1)) {
		bool ok;
		n = str.mid(1, cpPos - 1).toInt(&ok);
		if (!ok || n > 0xff) {
			n = 0xff;
		}
	}
	return n;
}

/**
 * Set string in text field.
 *
 * @param field        field
 * @param text         text to set
 */

void Mp3File::setString(ID3_Field* field, const QString &text)
{
	ID3_TextEnc enc = field->GetEncoding();
	// (ID3TE_IS_DOUBLE_BYTE_ENC(enc))
	if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
		// Unfortunately, Unicode support in id3lib is rather buggy in the
		// current version: The codes are mirrored, a second different
		// BOM may be added, if the LSB >= 0x80, the MSB is set to 0xff.
		// If iconv is used (id3lib on Linux), the character do not come
		// back mirrored, but with a second (different)! BOM 0xfeff and
		// they are still written in the wrong order (big endian).
		// In the hope that my patches will be included, I try here to
		// work around these bugs, but there is no solution for the
		// LSB >= 0x80 bug.
		const QChar *qcarray = text.unicode();
		uint unicode_size = text.length();
		unicode_t *unicode = new unicode_t[unicode_size + 1];
		if (unicode) {
			uint i;
			for (i = 0; i < unicode_size; i++) {
				unicode[i] = (ushort)qcarray[i].unicode();
				if (UNICODE_SUPPORT_BUGGY) {
					unicode[i] = (ushort)(((unicode[i] & 0x00ff) << 8) |
										  ((unicode[i] & 0xff00) >> 8));
				}
			}
			unicode[unicode_size] = 0;
			field->Set(unicode);
			delete [] unicode;
		}
	} else {
		// (ID3TE_IS_SINGLE_BYTE_ENC(enc))
		// (enc == ID3TE_ISO8859_1 || enc == ID3TE_UTF8)
		field->Set(text);
	}
}

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

bool Mp3File::setTextField(ID3_Tag *tag, ID3_FrameID id, const QString &text,
						   bool allowUnicode, bool replace, bool removeEmpty)
{
	bool changed = false;
	if (tag && !text.isNull()) {
		ID3_Frame* frame = NULL;
		bool removeOnly = removeEmpty && text.isEmpty();
		if (replace || removeOnly) {
			frame = tag->Find(id);
			frame = tag->RemoveFrame(frame);
			delete frame;
			changed = true;
		}
		if (!removeOnly && (replace || tag->Find(id) == NULL)) {
			frame = new ID3_Frame(id);
			if (frame) {
				ID3_Field* fld = frame->GetField(ID3FN_TEXT);
				if (fld) {
					if (allowUnicode && fld->GetEncoding() == ID3TE_ISO8859_1) {
						// check if information is lost if the string is not unicode
						uint i, unicode_size = text.length();
						const QChar *qcarray = text.unicode();
						for (i = 0; i < unicode_size; i++) {
							if (qcarray[i].latin1() == 0) {
								ID3_Field *encfld = frame->GetField(ID3FN_TEXTENC);
								if (encfld) {
									encfld->Set(ID3TE_UTF16);
								}
								fld->SetEncoding(ID3TE_UTF16);
								break;
							}
						}
					}
					setString(fld, text);
					tag->AttachFrame(frame);
				}
			}
			changed = true;
		}
	}
	return changed;
}

/**
 * Set year.
 *
 * @param tag ID3 tag
 * @param num number to set, 0 to remove field.
 *
 * @return true if the field was changed.
 */

bool Mp3File::setYear(ID3_Tag *tag, int num)
{
	bool changed = false;
	if (num >= 0) {
		QString str;
		if (num != 0) {
			str.setNum(num);
		} else {
			str = "";
		}
		changed = setTextField(tag, ID3FID_YEAR, str);
	}
	return changed;
}

/**
 * Set track.
 *
 * @param tag ID3 tag
 * @param num number to set, 0 to remove field.
 *
 * @return true if the field was changed.
 */

bool Mp3File::setTrackNum(ID3_Tag *tag, int num)
{
	bool changed = false;
	if (num >= 0) {
		QString str;
		if (num != 0) {
			str.setNum(num);
		} else {
			str = "";
		}
		changed = setTextField(tag, ID3FID_TRACKNUM, str);
	}
	return changed;
}

/**
 * Set genre.
 *
 * @param tag ID3 tag
 * @param num number to set, 0xff to remove field.
 *
 * @return true if the field was changed.
 */

bool Mp3File::setGenreNum(ID3_Tag *tag, int num)
{
	bool changed = false;
	if (num >= 0) {
		QString str;
		if (num != 0xff) {
			str = QString("(%1)").arg(num);
		} else {
			str = "";
		}
		changed = setTextField(tag, ID3FID_CONTENTTYPE, str);
	}
	return changed;
}

/**
 * Get ID3v1 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getTitleV1(void)
{
	return getTextField(tagV1, ID3FID_TITLE);
}

/**
 * Get ID3v1 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getArtistV1(void)
{
	return getTextField(tagV1, ID3FID_LEADARTIST);
}

/**
 * Get ID3v1 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getAlbumV1(void)
{
	return getTextField(tagV1, ID3FID_ALBUM);
}

/**
 * Get ID3v1 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getCommentV1(void)
{
	return getTextField(tagV1, ID3FID_COMMENT);
}

/**
 * Get ID3v1 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getYearV1(void)
{
	return getYear(tagV1);
}

/**
 * Get ID3v1 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getTrackNumV1(void)
{
	return getTrackNum(tagV1);
}

/**
 * Get ID3v1 genre.
 *
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getGenreNumV1(void)
{
	return getGenreNum(tagV1);
}

/**
 * Get ID3v2 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getTitleV2(void)
{
	return getTextField(tagV2, ID3FID_TITLE);
}

/**
 * Get ID3v2 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getArtistV2(void)
{
	return getTextField(tagV2, ID3FID_LEADARTIST);
}

/**
 * Get ID3v2 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getAlbumV2(void)
{
	return getTextField(tagV2, ID3FID_ALBUM);
}

/**
 * Get ID3v2 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getCommentV2(void)
{
	return getTextField(tagV2, ID3FID_COMMENT);
}

/**
 * Get ID3v2 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getYearV2(void)
{
	return getYear(tagV2);
}

/**
 * Get ID3v2 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getTrackNumV2(void)
{
	return getTrackNum(tagV2);
}

/**
 * Get ID3v2 genre.
 *
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getGenreNumV2(void)
{
	return getGenreNum(tagV2);
}

/**
 * Get ID3v1 tags from file.
 *
 * @param st tags to put result
 */

void Mp3File::getStandardTagsV1(StandardTags *st)
{
	st->title = getTitleV1();
	st->artist = getArtistV1();
	st->album = getAlbumV1();
	st->comment = getCommentV1();
	st->year = getYearV1();
	st->track = getTrackNumV1();
	st->genre = getGenreNumV1();
}

/**
 * Get ID3v2 tags from file.
 *
 * @param st tags to put result
 */

void Mp3File::getStandardTagsV2(StandardTags *st)
{
	st->title = getTitleV2();
	st->artist = getArtistV2();
	st->album = getAlbumV2();
	st->comment = getCommentV2();
	st->year = getYearV2();
	st->track = getTrackNumV2();
	st->genre = getGenreNumV2();
}

/**
 * Set ID3v1 title.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setTitleV1(const QString& str)
{
	if (setTextField(tagV1, ID3FID_TITLE, str)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setArtistV1(const QString& str)
{
	if (setTextField(tagV1, ID3FID_LEADARTIST, str)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setAlbumV1(const QString& str)
{
	if (setTextField(tagV1, ID3FID_ALBUM, str)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setCommentV1(const QString& str)
{
	if (setTextField(tagV1, ID3FID_COMMENT, str)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v1 year.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setYearV1(int num)
{
	if (setYear(tagV1, num)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v1 track.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setTrackNumV1(int num)
{
	if (setTrackNum(tagV1, num)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v1 genre.
 *
 * @param num number to set, 0xff to remove field.
 */

void Mp3File::setGenreNumV1(int num)
{
	if (setGenreNum(tagV1, num)) {
		changedV1 = true;
	}
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setTitleV2(const QString& str)
{
	if (setTextField(tagV2, ID3FID_TITLE, str, true)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setArtistV2(const QString& str)
{
	if (setTextField(tagV2, ID3FID_LEADARTIST, str, true)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setAlbumV2(const QString& str)
{
	if (setTextField(tagV2, ID3FID_ALBUM, str, true)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setCommentV2(const QString& str)
{
	if (setTextField(tagV2, ID3FID_COMMENT, str, true)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v2 year.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setYearV2(int num)
{
	if (setYear(tagV2, num)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v2 track.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setTrackNumV2(int num)
{
	if (setTrackNum(tagV2, num)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v2 genre.
 *
 * @param num number to set, 0xff to remove field.
 */

void Mp3File::setGenreNumV2(int num)
{
	if (setGenreNum(tagV2, num)) {
		changedV2 = true;
	}
}

/**
 * Set ID3v1 tags.
 *
 * @param st tags to set
 */

void  Mp3File::setStandardTagsV1(const StandardTags *st)
{
	StandardTags oldst;
	getStandardTagsV1(&oldst);
	if (st->title != oldst.title) {
		setTitleV1(st->title);
	}
	if (st->artist != oldst.artist) {
		setArtistV1(st->artist);
	}
	if (st->album != oldst.album) {
		setAlbumV1(st->album);
	}
	if (st->comment != oldst.comment) {
		setCommentV1(st->comment);
	}
	if (st->year != oldst.year) {
		setYearV1(st->year);
	}
	if (st->track != oldst.track) {
		setTrackNumV1(st->track);
	}
	if (st->genre != oldst.genre) {
		setGenreNumV1(st->genre);
	}
}

/**
 * Set ID3v2 tags.
 *
 * @param st tags to set
 */

void  Mp3File::setStandardTagsV2(const StandardTags *st)
{
	StandardTags oldst;
	getStandardTagsV2(&oldst);
	if (st->title != oldst.title) {
		setTitleV2(st->title);
	}
	if (st->artist != oldst.artist) {
		setArtistV2(st->artist);
	}
	if (st->album != oldst.album) {
		setAlbumV2(st->album);
	}
	if (st->comment != oldst.comment) {
		setCommentV2(st->comment);
	}
	if (st->year != oldst.year) {
		setYearV2(st->year);
	}
	if (st->track != oldst.track) {
		setTrackNumV2(st->track);
	}
	if (st->genre != oldst.genre) {
		setGenreNumV2(st->genre);
	}
}

#if QT_VERSION >= 300
/**
 * Remove artist part from album string.
 * This is used when only the album is needed, but the regexp in
 * getTagsFromFilename() matched a "artist - album" string.
 *
 * @param album album string
 */

static void remove_artist(QString& album)
{
	int pos = album.find(" - ");
	if (pos != -1) {
		album.remove(0, pos + 3);
	}
}
#endif

/**
 * Get tags from filename.
 * Supported formats:
 * with QT3:
 * album/track - artist - song.mp3
 * artist - album/track song.mp3
 * /artist - album - track - song.mp3
 * album/artist - track - song.mp3
 * album/artist - song.mp3
 *
 * with QT2 (only weak regexp support):
 * artist - album/track song.mp3
 *
 * @param st tags to put result
 */

void Mp3File::getTagsFromFilename(StandardTags *st)
{
#if QT_VERSION < 300
	int start, end;
	const QString fn(getAbsFilename());
	QRegExp re("[^/]+ - [^/]+/\\d\\d?\\d?[- ]+[^/]+\\.mp3");
	start = re.match(fn);
	if (start != -1) {
		end = fn.find(" - ", start);
		st->artist = fn.mid(start, end - start);
		start = end + 3;
		end = fn.find("/", start);
		st->album = fn.mid(start, end - start);
		start = end + 1;
		for (end = start + 1;; end++) {
			if (fn[end] < '0' || fn[end] > '9') break;
		}
		st->track = fn.mid(start, end - start).toInt();
		for (;; end++) {
			if (fn[end] != ' ' && fn[end] != '-') break;
		}
		start = end;
		end = fn.find(".mp3", start);
		st->title = fn.mid(start, end - start);
	}
#else
	QRegExp re;
	QString fn(getAbsFilename());
	// album/track - artist - song.mp3
	re.setPattern("([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+) - ([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->track = re.cap(2).toInt();
		st->artist = re.cap(3);
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// artist - album/track song.mp3
	re.setPattern("([^/]+) - ([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// /artist - album - track - song.mp3
	re.setPattern("/([^/]+[^-_/ ]) - ([^-_/ ][^/]+[^-_/ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// album/artist - track - song.mp3
	re.setPattern("([^/]+)/([^/]+[^-_\\./ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// album/artist - song.mp3
	re.setPattern("([^/]+)/([^/]+[^-_/ ]) - ([^-_/ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->title = re.cap(3);
		remove_artist(st->album);
		return;
	}
#endif
}

static const char *fnFmt[] = {
	"%t %s.mp3",
	"%t. %s.mp3",
	"%a - %s.mp3",
	"%a-%s.mp3",
	"%a_%s.mp3",
	"(%a) %s.mp3",
	"%t. %a - %s.mp3",
	"%a - %t - %s.mp3",
	"%a - %l - %t - %s.mp3",
	0
};

const char **Mp3File::fnFmtList = &fnFmt[0];

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

void Mp3File::getFilenameFromTags(const StandardTags *st, QString fmt)
{
	const int num_tag_codes = 7;
	const QChar tag_code[num_tag_codes] = {
	    's', 'l', 'a', 'c', 'y', 't', 'g'};
	QString tag_str[num_tag_codes];
	QString insert_str[num_tag_codes];
	QString year, track;
	year.sprintf("%d", st->year);
	track.sprintf("%02d", st->track);
	tag_str[0] = st->title;
	tag_str[1] = st->album;
	tag_str[2] = st->artist;
	tag_str[3] = st->comment;
	tag_str[4] = year;
	tag_str[5] = track;
	tag_str[6] = Genres::getName(st->genre);
	int pos = 0, i;
	for (i = 0;; ++i) {
		pos = fmt.find('%', pos);
		if (pos == -1) break;
		if (i >= num_tag_codes) {
			// maximum of insert strings reached,
			// remove rest of string
			fmt.truncate(pos);
			break;
		}
		++pos;
		for (int k = 0;; ++k) {
			if (k >= num_tag_codes) {
				// invalid code at pos, remove it
				fmt.remove(--pos, 2);
				break;
			}
			if (fmt[pos] == tag_code[k]) {
				// code found, prepare format and string for sprintf
				fmt[pos] = i + '0';
				insert_str[i] = tag_str[k];
				++pos;
				break;
			}
		}
	}
	new_filename = fmt;
	for (int k = 0; k < i; ++k) {
		new_filename = new_filename.arg(insert_str[k]);
	}
}

/**
 * Update frame list box.
 *
 * @param lb list box
 */

void Mp3File::updateTagListV2(QListBox *lb)
{
	if (tagV2) {
		ID3_Tag::Iterator* iter = tagV2->CreateIterator();
		ID3_Frame* frame;
		lb->clear();
		while ((frame = iter->GetNext()) != NULL) {
			lb->insertItem(frame->GetTextID());
		}
#if defined WIN32 && defined _DEBUG
		/* just to avoid user breakpoint in VC++ */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
	}
}
