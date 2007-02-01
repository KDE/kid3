/**
 * \file mp3file.cpp
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "mp3file.h"
#ifdef HAVE_ID3LIB

#include <qdir.h>
#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
#include <Q3CString>
#else
#include <qlistbox.h>
#endif

#include <id3/tag.h>
#ifdef WIN32
#include <id3.h>
#endif

#include "standardtags.h"
#include "mp3framelist.h"
#include "genres.h"
#include "dirinfo.h"
#include <sys/stat.h>
#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#ifdef WIN32
/**
 * This will be set for id3lib versions with Unicode bugs.
 * ID3LIB_ symbols cannot be found on Windows ?!
 */
#define UNICODE_SUPPORT_BUGGY 1
#else
/** This will be set for id3lib versions with Unicode bugs. */
#define UNICODE_SUPPORT_BUGGY ((((ID3LIB_MAJOR_VERSION) << 16) + ((ID3LIB_MINOR_VERSION) << 8) + (ID3LIB_PATCH_VERSION)) <= 0x030803)
#endif

/**
 * Constructor.
 *
 * @param di directory information
 * @param fn filename
 */
Mp3File::Mp3File(const DirInfo* di, const QString& fn) :
	TaggedFile(di, fn), m_tagV1(0), m_tagV2(0)
{
}

/**
 * Destructor.
 */
Mp3File::~Mp3File()
{
	if (m_tagV1) {
		delete m_tagV1;
	}
	if (m_tagV2) {
		delete m_tagV2;
	}
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void Mp3File::readTags(bool force)
{
	Q3CString fn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());

	if (force && m_tagV1) {
		m_tagV1->Clear();
		m_tagV1->Link(fn, ID3TT_ID3V1);
		markTag1Changed(false);
	}
	if (!m_tagV1) {
		m_tagV1 = new ID3_Tag;
		if (m_tagV1) {
			m_tagV1->Link(fn, ID3TT_ID3V1);
			markTag1Changed(false);
		}
	}

	if (force && m_tagV2) {
		m_tagV2->Clear();
		m_tagV2->Link(fn, ID3TT_ID3V2);
		markTag2Changed(false);
	}
	if (!m_tagV2) {
		m_tagV2 = new ID3_Tag;
		if (m_tagV2) {
			m_tagV2->Link(fn, ID3TT_ID3V2);
			markTag2Changed(false);
		}
	}

	if (force) {
		setFilename(currentFilename());
	}
}

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
bool Mp3File::writeTags(bool force, bool* renamed, bool preserve)
{
	QString fnStr(getDirInfo()->getDirname() + QDir::separator() + currentFilename());
	if (isChanged() && !QFileInfo(fnStr).isWritable()) {
		return false;
	}

	// store time stamp if it has to be preserved
	Q3CString fn;
	bool setUtime = false;
	struct utimbuf times;
	if (preserve) {
		fn = QFile::encodeName(fnStr);
		struct stat fileStat;
		if (::stat(fn, &fileStat) == 0) {
			times.actime  = fileStat.st_atime;
			times.modtime = fileStat.st_mtime;
			setUtime = true;
		}
	}

	// There seems to be a bug in id3lib: The V1 genre is not
	// removed. So we check here and strip the whole header
	// if there are no frames.
	if (m_tagV1 && (force || isTag1Changed()) && (m_tagV1->NumFrames() == 0)) {
		m_tagV1->Strip(ID3TT_ID3V1);
		markTag1Changed(false);
	}
	// Even after removing all frames, HasV2Tag() still returns true,
	// so we strip the whole header.
	if (m_tagV2 && (force || isTag2Changed()) && (m_tagV2->NumFrames() == 0)) {
		m_tagV2->Strip(ID3TT_ID3V2);
		markTag2Changed(false);
	}
	// There seems to be a bug in id3lib: If I update an ID3v1 and then
	// strip the ID3v2 the ID3v1 is removed too and vice versa, so I
	// first make any stripping and then the updating.
	if (m_tagV1 && (force || isTag1Changed()) && (m_tagV1->NumFrames() > 0)) {
		m_tagV1->Update(ID3TT_ID3V1);
		markTag1Changed(false);
	}
	if (m_tagV2 && (force || isTag2Changed()) && (m_tagV2->NumFrames() > 0)) {
		m_tagV2->Update(ID3TT_ID3V2);
		markTag2Changed(false);
	}

	// restore time stamp
	if (setUtime) {
		::utime(fn, &times);
	}

	if (getFilename() != currentFilename()) {
		if (!renameFile(currentFilename(), getFilename())) {
			return false;
		}
		updateCurrentFilename();
		// link tags to new file name
		readTags(true);
		*renamed = true;
	}
	return true;
}

/**
 * Remove all ID3v1 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void Mp3File::removeTagsV1(const StandardTagsFilter& flt)
{
	if (m_tagV1) {
		if (flt.areAllTrue()) {
			ID3_Tag::Iterator* iter = m_tagV1->CreateIterator();
			ID3_Frame* frame;
			while ((frame = iter->GetNext()) != NULL) {
				m_tagV1->RemoveFrame(frame);
			}
#ifdef WIN32
			/* allocated in Windows DLL => must be freed in the same DLL */
			ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
			delete iter;
#endif
			markTag1Changed();
		} else {
			removeStandardTagsV1(flt);
		}
	}
}

/**
 * Remove all ID3v2 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void Mp3File::removeTagsV2(const StandardTagsFilter& flt)
{
	if (m_tagV2) {
		if (flt.areAllTrue()) {
			ID3_Tag::Iterator* iter = m_tagV2->CreateIterator();
			ID3_Frame* frame;
			while ((frame = iter->GetNext()) != NULL) {
				m_tagV2->RemoveFrame(frame);
			}
#ifdef WIN32
			/* allocated in Windows DLL => must be freed in the same DLL */
			ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
			delete iter;
#endif
			markTag2Changed();
		} else {
			removeStandardTagsV2(flt);
		}
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
			const unicode_t* txt = field->GetRawUnicodeText();
			uint unicode_size = field->Size() / sizeof(unicode_t);
			if (unicode_size && txt) {
				QChar* qcarray = new QChar[unicode_size];
				if (qcarray) {
					// Unfortunately, Unicode support in id3lib is rather buggy
					// in the current version: The codes are mirrored.
					// In the hope that my patches will be included, I try here
					// to work around these bugs.
					uint i;
					uint numZeroes = 0;
					for (i = 0; i < unicode_size; i++) {
						qcarray[i] =
							UNICODE_SUPPORT_BUGGY ?
							(ushort)(((txt[i] & 0x00ff) << 8) |
									 ((txt[i] & 0xff00) >> 8)) :
							(ushort)txt[i];
						if (qcarray[i].isNull()) { ++numZeroes; }
					}
					// remove a single trailing zero character
					if (numZeroes == 1 && qcarray[unicode_size - 1].isNull()) {
						--unicode_size;
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
QString Mp3File::getTextField(const ID3_Tag* tag, ID3_FrameID id)
{
	if (!tag) {
		return QString::null;
	}
	QString str("");
	ID3_Field* fld;
	ID3_Frame* frame = tag->Find(id);
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
int Mp3File::getYear(const ID3_Tag* tag)
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
int Mp3File::getTrackNum(const ID3_Tag* tag)
{
	QString str = getTextField(tag, ID3FID_TRACKNUM);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0;
	// handle "track/total number of tracks" format
	int slashPos = str.find('/');
	if (slashPos != -1) {
		str.truncate(slashPos);
	}
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
int Mp3File::getGenreNum(const ID3_Tag* tag)
{
	QString str = getTextField(tag, ID3FID_CONTENTTYPE);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0xff;
	int cpPos = 0, n = 0xff;
	if ((str[0] == '(') && ((cpPos = str.find(')', 2)) > 1)) {
		bool ok;
		n = str.mid(1, cpPos - 1).toInt(&ok);
		if (!ok || n > 0xff) {
			n = 0xff;
		}
	} else {
		// ID3v2 genres can be stored as "(9)", "(9)Metal" or "Metal".
		// If the string does not start with '(', try to get the genre number
		// from a string containing a genre text.
		n = Genres::getNumber(str);
	}
	return n;
}

/**
 * Set string in text field.
 *
 * @param field        field
 * @param text         text to set
 */
void Mp3File::setString(ID3_Field* field, const QString& text)
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
		const QChar* qcarray = text.unicode();
		uint unicode_size = text.length();
		unicode_t* unicode = new unicode_t[unicode_size + 1];
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
	} else if (enc == ID3TE_UTF8) {
		field->Set(text.utf8().data());
	} else {
		// enc == ID3TE_ISO8859_1
		field->Set(text.latin1());
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
bool Mp3File::setTextField(ID3_Tag* tag, ID3_FrameID id, const QString& text,
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
						const QChar* qcarray = text.unicode();
						for (i = 0; i < unicode_size; i++) {
							if (qcarray[i].latin1() == 0) {
								ID3_Field* encfld = frame->GetField(ID3FN_TEXTENC);
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
bool Mp3File::setYear(ID3_Tag* tag, int num)
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
 * @param numTracks total number of tracks, -1 to ignore
 *
 * @return true if the field was changed.
 */
bool Mp3File::setTrackNum(ID3_Tag* tag, int num, int numTracks)
{
	bool changed = false;
	if (num >= 0) {
		QString str;
		if (num != 0) {
			str.setNum(num);
			if (numTracks > 0) {
				str += '/';
				str += QString::number(numTracks);
			}
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
bool Mp3File::setGenreNum(ID3_Tag* tag, int num)
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
QString Mp3File::getTitleV1()
{
	return getTextField(m_tagV1, ID3FID_TITLE);
}

/**
 * Get ID3v1 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getArtistV1()
{
	return getTextField(m_tagV1, ID3FID_LEADARTIST);
}

/**
 * Get ID3v1 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getAlbumV1()
{
	return getTextField(m_tagV1, ID3FID_ALBUM);
}

/**
 * Get ID3v1 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getCommentV1()
{
	return getTextField(m_tagV1, ID3FID_COMMENT);
}

/**
 * Get ID3v1 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int Mp3File::getYearV1()
{
	return getYear(m_tagV1);
}

/**
 * Get ID3v1 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int Mp3File::getTrackNumV1()
{
	return getTrackNum(m_tagV1);
}

/**
 * Get ID3v1 genre.
 *
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */
int Mp3File::getGenreNumV1()
{
	return getGenreNum(m_tagV1);
}

/**
 * Get ID3v2 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getTitleV2()
{
	return getTextField(m_tagV2, ID3FID_TITLE);
}

/**
 * Get ID3v2 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getArtistV2()
{
	return getTextField(m_tagV2, ID3FID_LEADARTIST);
}

/**
 * Get ID3v2 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getAlbumV2()
{
	return getTextField(m_tagV2, ID3FID_ALBUM);
}

/**
 * Get ID3v2 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getCommentV2()
{
	return getTextField(m_tagV2, ID3FID_COMMENT);
}

/**
 * Get ID3v2 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int Mp3File::getYearV2()
{
	return getYear(m_tagV2);
}

/**
 * Get ID3v2 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int Mp3File::getTrackNumV2()
{
	return getTrackNum(m_tagV2);
}

/**
 * Get ID3v2 genre.
 *
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */
int Mp3File::getGenreNumV2()
{
	return getGenreNum(m_tagV2);
}

/**
 * Get ID3v2 genre as text.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getGenreV2()
{
	int num = getGenreNumV2();
	if (num != 0xff && num != -1) {
		return Genres::getName(num);
	} else {
		return getTextField(m_tagV2, ID3FID_CONTENTTYPE);
	}
}

/**
 * Set ID3v1 title.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setTitleV1(const QString& str)
{
	if (setTextField(m_tagV1, ID3FID_TITLE, str)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setArtistV1(const QString& str)
{
	if (setTextField(m_tagV1, ID3FID_LEADARTIST, str)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setAlbumV1(const QString& str)
{
	if (setTextField(m_tagV1, ID3FID_ALBUM, str)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setCommentV1(const QString& str)
{
	if (setTextField(m_tagV1, ID3FID_COMMENT, str)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v1 year.
 *
 * @param num number to set, 0 to remove field.
 */
void Mp3File::setYearV1(int num)
{
	if (setYear(m_tagV1, num)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v1 track.
 *
 * @param num number to set, 0 to remove field.
 */
void Mp3File::setTrackNumV1(int num)
{
	if (setTrackNum(m_tagV1, num)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v1 genre.
 *
 * @param num number to set, 0xff to remove field.
 */
void Mp3File::setGenreNumV1(int num)
{
	if (setGenreNum(m_tagV1, num)) {
		markTag1Changed();
	}
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setTitleV2(const QString& str)
{
	if (setTextField(m_tagV2, ID3FID_TITLE, str, true)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setArtistV2(const QString& str)
{
	if (setTextField(m_tagV2, ID3FID_LEADARTIST, str, true)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setAlbumV2(const QString& str)
{
	if (setTextField(m_tagV2, ID3FID_ALBUM, str, true)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setCommentV2(const QString& str)
{
	if (setTextField(m_tagV2, ID3FID_COMMENT, str, true)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 year.
 *
 * @param num number to set, 0 to remove field.
 */
void Mp3File::setYearV2(int num)
{
	if (setYear(m_tagV2, num)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 track.
 *
 * @param num number to set, 0 to remove field.
 */
void Mp3File::setTrackNumV2(int num)
{
	int numTracks = getTotalNumberOfTracksIfEnabled();
	if (setTrackNum(m_tagV2, num, numTracks)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 genre.
 *
 * @param num number to set, 0xff to remove field.
 */
void Mp3File::setGenreNumV2(int num)
{
	if (setGenreNum(m_tagV2, num)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void Mp3File::setGenreV2(const QString& str)
{
	if (setTextField(m_tagV2, ID3FID_CONTENTTYPE, str, true)) {
		markTag2Changed();
	}
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTagV1() and hasTagV2() do not return meaningful information.
 */
bool Mp3File::isTagInformationRead() const
{
	return m_tagV1 || m_tagV2;
}

/**
 * Check if file has an ID3v1 tag.
 *
 * @return true if a V1 tag is available.
 * @see isTagInformationRead()
 */
bool Mp3File::hasTagV1() const
{
	return m_tagV1 && m_tagV1->HasV1Tag();
}

/**
 * Check if ID3v1 tags are supported by the format of this file.
 *
 * @return true.
 */
bool Mp3File::isTagV1Supported() const
{
	return true;
}

/**
 * Check if file has an ID3v2 tag.
 *
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool Mp3File::hasTagV2() const
{
	return m_tagV2 && m_tagV2->HasV2Tag();
}

/**
 * Get technical detail information.
 *
 * @return string with detail information,
 *         "" if no information available.
 */
QString Mp3File::getDetailInfo() const {
	QString str("");
	const Mp3_Headerinfo* info = NULL;
	if (m_tagV1) {
		info = m_tagV1->GetMp3HeaderInfo();
	} else if (m_tagV2) {
		info = m_tagV2->GetMp3HeaderInfo();
	}
	if (info) {
		switch (info->version) {
			case MPEGVERSION_1:
				str.append("MPEG 1 ");
				break;
			case MPEGVERSION_2:
				str.append("MPEG 2 ");
				break;
			case MPEGVERSION_2_5:
				str.append("MPEG 2.5 ");
				break;
			default:
				; // nothing
		}
		switch (info->layer) {
			case MPEGLAYER_I:
				str.append("Layer 1 ");
				break;
			case MPEGLAYER_II:
				str.append("Layer 2 ");
				break;
			case MPEGLAYER_III:
				str.append("Layer 3 ");
				break;
			default:
				; // nothing
		}
		int kb = info->bitrate;
#ifndef HAVE_NO_ID3LIB_VBR
		if (info->vbr_bitrate > 1000) {
			str.append("VBR ");
			kb = info->vbr_bitrate;
		}
#endif
		if (kb > 1000 && kb < 999000) {
			kb /= 1000;
			QString kbStr;
			kbStr.setNum(kb);
			kbStr.append(" kbps ");
			str.append(kbStr);
		}
		int hz = info->frequency;
		if (hz > 0) {
			QString hzStr;
			hzStr.setNum(hz);
			hzStr.append(" Hz ");
			str.append(hzStr);
		}
		switch (info->channelmode) {
			case MP3CHANNELMODE_STEREO:
				str.append("Stereo ");
				break;
			case MP3CHANNELMODE_JOINT_STEREO:
				str.append("Joint Stereo ");
				break;
			case MP3CHANNELMODE_DUAL_CHANNEL:
				str.append("Dual ");
				break;
			case MP3CHANNELMODE_SINGLE_CHANNEL:
				str.append("Single ");
				break;
			default:
				; // nothing
		}
		if (info->time > 0) {
			str.append(formatTime(info->time));
		}
	}
	return str;
}

/**
 * Get duration of file.
 *
 * @return duration in seconds,
 *         0 if unknown.
 */
unsigned Mp3File::getDuration() const
{
	unsigned duration = 0;
	const Mp3_Headerinfo* info = NULL;
	if (m_tagV1) {
		info = m_tagV1->GetMp3HeaderInfo();
	} else if (m_tagV2) {
		info = m_tagV2->GetMp3HeaderInfo();
	}
	if (info && info->time > 0) {
		duration = info->time;
	}
	return duration;
}

/** Frame list for MP3 files. */
Mp3FrameList* Mp3File::s_mp3FrameList = 0;

/**
 * Get frame list for this type of tagged file.
 *
 * @return frame list.
 */
FrameList* Mp3File::getFrameList() const
{
	if (!s_mp3FrameList) {
		s_mp3FrameList = new Mp3FrameList();
	}
	return s_mp3FrameList;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".mp3".
 */
QString Mp3File::getFileExtension() const
{
	return ".mp3";
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1".
 */
QString Mp3File::getTagFormatV1() const
{
	return hasTagV1() ? QString("ID3v1.1") : QString::null;
}

/**
 * Get the format of tag 2.
 *
 * @return string describing format of tag 2,
 *         e.g. "ID3v2.3", "ID3v2.4".
 */
QString Mp3File::getTagFormatV2() const
{
	if (m_tagV2 && m_tagV2->HasV2Tag()) {
		switch (m_tagV2->GetSpec()) {
			case ID3V2_3_0:
				return "ID3v2.3.0";
			case ID3V2_4_0:
				return "ID3v2.4.0";
			case ID3V2_2_0:
				return "ID3v2.2.0";
			case ID3V2_2_1:
				return "ID3v2.2.1";
			default:
				break;
		}
	}
	return QString::null;
}

/**
 * Clean up static resources.
 */
void Mp3File::staticCleanup()
{
	delete s_mp3FrameList;
	s_mp3FrameList = 0;
}

#endif // HAVE_ID3LIB
