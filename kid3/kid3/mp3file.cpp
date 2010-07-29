/**
 * \file mp3file.cpp
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "mp3file.h"
#ifdef HAVE_ID3LIB

#include <qdir.h>
#include <qstring.h>
#include <qtextcodec.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QByteArray>
#endif

#include <id3/tag.h>
#ifdef WIN32
#include <id3.h>
#endif

#include "genres.h"
#include "dirinfo.h"
#include "kid3.h"
#include "attributedata.h"
#include <cstring>
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

/** Text codec for ID3v1 tags, 0 to use default (ISO 8859-1) */
const QTextCodec* Mp3File::s_textCodecV1 = 0;

/** Default text encoding */
ID3_TextEnc Mp3File::s_defaultTextEncoding = ID3TE_ISO8859_1;

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
	QCM_QCString fn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());

	if (force && m_tagV1) {
		m_tagV1->Clear();
		m_tagV1->Link(fn, ID3TT_ID3V1);
		markTag1Unchanged();
	}
	if (!m_tagV1) {
		m_tagV1 = new ID3_Tag;
		if (m_tagV1) {
			m_tagV1->Link(fn, ID3TT_ID3V1);
			markTag1Unchanged();
		}
	}

	if (force && m_tagV2) {
		m_tagV2->Clear();
		m_tagV2->Link(fn, ID3TT_ID3V2);
		markTag2Unchanged();
	}
	if (!m_tagV2) {
		m_tagV2 = new ID3_Tag;
		if (m_tagV2) {
			m_tagV2->Link(fn, ID3TT_ID3V2);
			markTag2Unchanged();
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
	QCM_QCString fn;
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
		markTag1Unchanged();
	}
	// Even after removing all frames, HasV2Tag() still returns true,
	// so we strip the whole header.
	if (m_tagV2 && (force || isTag2Changed()) && (m_tagV2->NumFrames() == 0)) {
		m_tagV2->Strip(ID3TT_ID3V2);
		markTag2Unchanged();
	}
	// There seems to be a bug in id3lib: If I update an ID3v1 and then
	// strip the ID3v2 the ID3v1 is removed too and vice versa, so I
	// first make any stripping and then the updating.
	if (m_tagV1 && (force || isTag1Changed()) && (m_tagV1->NumFrames() > 0)) {
		m_tagV1->Update(ID3TT_ID3V1);
		markTag1Unchanged();
	}
	if (m_tagV2 && (force || isTag2Changed()) && (m_tagV2->NumFrames() > 0)) {
		m_tagV2->Update(ID3TT_ID3V2);
		markTag2Unchanged();
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
 * Remove ID3v1 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void Mp3File::deleteFramesV1(const FrameFilter& flt)
{
	if (m_tagV1) {
		if (flt.areAllEnabled()) {
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
			markTag1Changed(Frame::FT_UnknownFrame);
			clearTrunctionFlags();
		} else {
			TaggedFile::deleteFramesV1(flt);
		}
	}
}

/**
 * Fix up a unicode string from id3lib.
 *
 * @param str      unicode string
 * @param numChars number of characters in str
 *
 * @return string as QString.
 */
static QString fixUpUnicode(const unicode_t* str, size_t numChars)
{
	QString text;
	if (numChars > 0 && str && *str) {
		QChar* qcarray = new QChar[numChars];
		if (qcarray) {
			// Unfortunately, Unicode support in id3lib is rather buggy
			// in the current version: The codes are mirrored.
			// In the hope that my patches will be included, I try here
			// to work around these bugs.
			size_t i;
			size_t numZeroes = 0;
			for (i = 0; i < numChars; i++) {
				qcarray[i] =
					UNICODE_SUPPORT_BUGGY ?
					(ushort)(((str[i] & 0x00ff) << 8) |
									 ((str[i] & 0xff00) >> 8)) :
					(ushort)str[i];
				if (qcarray[i].isNull()) { ++numZeroes; }
			}
			// remove a single trailing zero character
			if (numZeroes == 1 && qcarray[numChars - 1].isNull()) {
				--numChars;
			}
			text = QString(qcarray, numChars);
			delete [] qcarray;
		}
	}
	return text;
}

/**
 * Get string from text field.
 *
 * @param field field
 * @param codec text codec to use, 0 for default
 *
 * @return string,
 *         "" if the field does not exist.
 */
static QString getString(ID3_Field* field, const QTextCodec* codec = 0)
{
	QString text("");
	if (field != NULL) {
		ID3_TextEnc enc = field->GetEncoding();
		if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
			size_t numItems = field->GetNumTextItems();
			if (numItems <= 1) {
				text = fixUpUnicode(field->GetRawUnicodeText(),
				                    field->Size() / sizeof(unicode_t));
			} else {
				// if there are multiple items, put them into one string
				// separated by a special separator.
				// ID3_Field::GetRawUnicodeTextItem() returns a pointer to a temporary
				// object, so I do not use it.
				text = fixUpUnicode(field->GetRawUnicodeText(),
				                    field->Size() / sizeof(unicode_t));
				text.replace('\0', Frame::stringListSeparator());
			}
		} else {
			// (ID3TE_IS_SINGLE_BYTE_ENC(enc))
			// (enc == ID3TE_ISO8859_1 || enc == ID3TE_UTF8)
			size_t numItems = field->GetNumTextItems();
			if (numItems <= 1) {
				text = codec ?
					codec->toUnicode(field->GetRawText(), field->Size()) :
					QString(field->GetRawText());
			} else {
				// if there are multiple items, put them into one string
				// separated by a special separator.
				for (size_t itemNr = 0; itemNr < numItems; ++itemNr) {
					if (itemNr == 0) {
						text = field->GetRawTextItem(0);
					} else {
						text += Frame::stringListSeparator();
						text += field->GetRawTextItem(itemNr);
					}
				}
			}
		}
	}
	return text;
}

/**
 * Get text field.
 *
 * @param tag ID3 tag
 * @param id  frame ID
 * @param codec text codec to use, 0 for default
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
static QString getTextField(const ID3_Tag* tag, ID3_FrameID id,
														const QTextCodec* codec = 0)
{
	if (!tag) {
		return QString::null;
	}
	QString str("");
	ID3_Field* fld;
	ID3_Frame* frame = tag->Find(id);
	if (frame && ((fld = frame->GetField(ID3FN_TEXT)) != NULL)) {
		str = getString(fld, codec);
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
static int getYear(const ID3_Tag* tag)
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
static int getTrackNum(const ID3_Tag* tag)
{
	QString str = getTextField(tag, ID3FID_TRACKNUM);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0;
	// handle "track/total number of tracks" format
	int slashPos = str.QCM_indexOf('/');
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
static int getGenreNum(const ID3_Tag* tag)
{
	QString str = getTextField(tag, ID3FID_CONTENTTYPE);
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0xff;
	int cpPos = 0, n = 0xff;
	if ((str[0] == '(') && ((cpPos = str.QCM_indexOf(')', 2)) > 1)) {
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
 * Allocate a fixed up a unicode string for id3lib.
 *
 * @param text string
 *
 * @return new allocated unicode string, has to be freed with delete [].
 */
static unicode_t* newFixedUpUnicode(const QString& text)
{
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
	}
	return unicode;
}

/**
 * Set string list in text field.
 *
 * @param field field
 * @param lst   string list to set
 */
static void setStringList(ID3_Field* field, const QStringList& lst)
{
	ID3_TextEnc enc = field->GetEncoding();
	bool first = true;
	for (QStringList::const_iterator it = lst.begin(); it != lst.end(); ++it) {
		if (first) {
			if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
				unicode_t* unicode = newFixedUpUnicode(*it);
				if (unicode) {
					field->Set(unicode);
					delete [] unicode;
				}
			} else if (enc == ID3TE_UTF8) {
				field->Set((*it).QCM_toUtf8().data());
			} else {
				// enc == ID3TE_ISO8859_1
				field->Set((*it).QCM_latin1());
			}
			first = false;
		} else {
			// This will not work with buggy id3lib. A BOM 0xfffe is written before
			// the first string, but not before the subsequent strings. Prepending a
			// BOM or changing the byte order does not help when id3lib rewrites
			// this field when another frame is changed. So you cannot use
			// string lists with Unicode encoding.
			if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
				unicode_t* unicode = newFixedUpUnicode(*it);
				if (unicode) {
					field->Add(unicode);
					delete [] unicode;
				}
			} else if (enc == ID3TE_UTF8) {
				field->Add((*it).QCM_toUtf8().data());
			} else {
				// enc == ID3TE_ISO8859_1
				field->Add((*it).QCM_latin1());
			}
		}
	}
}

/**
 * Set string in text field.
 *
 * @param field        field
 * @param text         text to set
 * @param codec        text codec to use, 0 for default
 */
static void setString(ID3_Field* field, const QString& text,
											const QTextCodec* codec = 0)
{
	if (text.QCM_indexOf(Frame::stringListSeparator()) == -1) {
		ID3_TextEnc enc = field->GetEncoding();
		// (ID3TE_IS_DOUBLE_BYTE_ENC(enc))
		if (enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
			unicode_t* unicode = newFixedUpUnicode(text);
			if (unicode) {
				field->Set(unicode);
				delete [] unicode;
			}
		} else if (enc == ID3TE_UTF8) {
			field->Set(text.QCM_toUtf8().data());
		} else {
			// enc == ID3TE_ISO8859_1
			field->Set(codec ? codec->fromUnicode(text).data() : text.QCM_latin1());
		}
	} else {
		setStringList(field, QCM_split(Frame::stringListSeparator(), text));
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
 * @param codec        text codec to use, 0 for default
 *
 * @return true if the field was changed.
 */
static bool setTextField(ID3_Tag* tag, ID3_FrameID id, const QString& text,
												 bool allowUnicode = false, bool replace = true,
												 bool removeEmpty = true, const QTextCodec* codec = 0)
{
	bool changed = false;
	if (tag && !text.isNull()) {
		ID3_Frame* frame = NULL;
		bool removeOnly = removeEmpty && text.isEmpty();
		if (replace || removeOnly) {
			if (id == ID3FID_COMMENT && tag->HasV2Tag()) {
				frame = tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, "");
			} else {
				frame = tag->Find(id);
			}
			if (frame) {
				frame = tag->RemoveFrame(frame);
				delete frame;
				changed = true;
			}
		}
		if (!removeOnly && (replace || tag->Find(id) == NULL)) {
			frame = new ID3_Frame(id);
			if (frame) {
				ID3_Field* fld = frame->GetField(ID3FN_TEXT);
				if (fld) {
					ID3_TextEnc enc = tag->HasV2Tag() ?
						Mp3File::getDefaultTextEncoding() : ID3TE_ISO8859_1;
					if (allowUnicode && enc == ID3TE_ISO8859_1) {
						// check if information is lost if the string is not unicode
						uint i, unicode_size = text.length();
						const QChar* qcarray = text.unicode();
						for (i = 0; i < unicode_size; i++) {
							char ch = qcarray[i].
#if QT_VERSION >= 0x040000
								toLatin1()
#else
								latin1()
#endif
								;
							if (ch == 0 || (ch & 0x80) != 0) {
								enc = ID3TE_UTF16;
								break;
							}
						}
					}
					ID3_Field* encfld = frame->GetField(ID3FN_TEXTENC);
					if (encfld) {
						encfld->Set(enc);
					}
					fld->SetEncoding(enc);
					setString(fld, text, codec);
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
static bool setYear(ID3_Tag* tag, int num)
{
	bool changed = false;
	if (num >= 0) {
		QString str;
		if (num != 0) {
			str.setNum(num);
		} else {
			str = "";
		}
		changed = getTextField(tag, ID3FID_YEAR) != str &&
		          setTextField(tag, ID3FID_YEAR, str);
	}
	return changed;
}

/**
 * Set track.
 *
 * @param tag ID3 tag
 * @param num number to set, 0 to remove field.
 * @param numTracks total number of tracks, -1 to ignore
 * @param numDigits number of digits, -1 to ignore
 *
 * @return true if the field was changed.
 */
static bool setTrackNum(ID3_Tag* tag, int num, int numTracks = -1,
                        int numDigits = -1)
{
	bool changed = false;
	if (num >= 0 && getTrackNum(tag) != num) {
		QString str;
		if (num != 0) {
			if (numDigits > 0) {
				str.sprintf("%0*d", numDigits, num);
			} else {
				str.setNum(num);
			}
			if (numTracks > 0) {
				str += '/';
				if (numDigits > 0) {
					str += QString().sprintf("%0*d", numDigits, numTracks);
				} else {
					str += QString::number(numTracks);
				}
			}
		} else {
			str = "";
		}
		changed = getTextField(tag, ID3FID_TRACKNUM) != str &&
		          setTextField(tag, ID3FID_TRACKNUM, str);
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
static bool setGenreNum(ID3_Tag* tag, int num)
{
	bool changed = false;
	if (num >= 0) {
		QString str;
		if (num != 0xff) {
			str = QString("(%1)").arg(num);
		} else {
			str = "";
		}
		changed = getTextField(tag, ID3FID_CONTENTTYPE) != str &&
		          setTextField(tag, ID3FID_CONTENTTYPE, str);
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
	return getTextField(m_tagV1, ID3FID_TITLE, s_textCodecV1);
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
	return getTextField(m_tagV1, ID3FID_LEADARTIST, s_textCodecV1);
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
	return getTextField(m_tagV1, ID3FID_ALBUM, s_textCodecV1);
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
	return getTextField(m_tagV1, ID3FID_COMMENT, s_textCodecV1);
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
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getGenreV1()
{
	int num = getGenreNum(m_tagV1);
	if (num == -1) {
		return QString::null;
	} else if (num == 0xff) {
		return "";
	} else {
		return Genres::getName(num);
	}
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
 * Get ID3v2 genre as text.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString Mp3File::getGenreV2()
{
	int num = getGenreNum(m_tagV2);
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
	if (getTextField(m_tagV1, ID3FID_TITLE, s_textCodecV1) != str &&
			setTextField(m_tagV1, ID3FID_TITLE, str, false, true, true, s_textCodecV1)) {
		markTag1Changed(Frame::FT_Title);
		QString s = checkTruncation(str, 1 << Frame::FT_Title);
		if (!s.isNull()) setTextField(m_tagV1, ID3FID_TITLE, s, false, true, true, s_textCodecV1);
	}
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setArtistV1(const QString& str)
{
	if (getTextField(m_tagV1, ID3FID_LEADARTIST, s_textCodecV1) != str &&
			setTextField(m_tagV1, ID3FID_LEADARTIST, str, false, true, true, s_textCodecV1)) {
		markTag1Changed(Frame::FT_Artist);
		QString s = checkTruncation(str, 1 << Frame::FT_Artist);
		if (!s.isNull()) setTextField(m_tagV1, ID3FID_LEADARTIST, s, false, true, true, s_textCodecV1);
	}
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setAlbumV1(const QString& str)
{
	if (getTextField(m_tagV1, ID3FID_ALBUM, s_textCodecV1) != str &&
			setTextField(m_tagV1, ID3FID_ALBUM, str, false, true, true, s_textCodecV1)) {
		markTag1Changed(Frame::FT_Album);
		QString s = checkTruncation(str, 1 << Frame::FT_Album);
		if (!s.isNull()) setTextField(m_tagV1, ID3FID_ALBUM, s, false, true, true, s_textCodecV1);
	}
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setCommentV1(const QString& str)
{
	if (getTextField(m_tagV1, ID3FID_COMMENT, s_textCodecV1) != str &&
			setTextField(m_tagV1, ID3FID_COMMENT, str, false, true, true, s_textCodecV1)) {
		markTag1Changed(Frame::FT_Comment);
		QString s = checkTruncation(str, 1 << Frame::FT_Comment, 28);
		if (!s.isNull()) setTextField(m_tagV1, ID3FID_COMMENT, s, false, true, true, s_textCodecV1);
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
		markTag1Changed(Frame::FT_Date);
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
		markTag1Changed(Frame::FT_Track);
		int n = checkTruncation(num, 1 << Frame::FT_Track);
		if (n != -1) setTrackNum(m_tagV1, n);
	}
}

/**
 * Set ID3v1 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void Mp3File::setGenreV1(const QString& str)
{
	if (!str.isNull()) {
		int num = Genres::getNumber(str);
		if (setGenreNum(m_tagV1, num)) {
			markTag1Changed(Frame::FT_Genre);
		}
		// if the string cannot be converted to a number, set the truncation flag
		checkTruncation(num == 0xff && !str.isEmpty() ? 1 : 0,
										1 << Frame::FT_Genre, 0);
	}
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setTitleV2(const QString& str)
{
	if (getTextField(m_tagV2, ID3FID_TITLE) != str &&
			setTextField(m_tagV2, ID3FID_TITLE, str, true)) {
		markTag2Changed(Frame::FT_Title);
	}
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setArtistV2(const QString& str)
{
	if (getTextField(m_tagV2, ID3FID_LEADARTIST) != str &&
			setTextField(m_tagV2, ID3FID_LEADARTIST, str, true)) {
		markTag2Changed(Frame::FT_Artist);
	}
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setAlbumV2(const QString& str)
{
	if (getTextField(m_tagV2, ID3FID_ALBUM) != str &&
			setTextField(m_tagV2, ID3FID_ALBUM, str, true)) {
		markTag2Changed(Frame::FT_Album);
	}
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field.
 */
void Mp3File::setCommentV2(const QString& str)
{
	if (getTextField(m_tagV2, ID3FID_COMMENT) != str &&
			setTextField(m_tagV2, ID3FID_COMMENT, str, true)) {
		markTag2Changed(Frame::FT_Comment);
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
		markTag2Changed(Frame::FT_Date);
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
	int numDigits = getTrackNumberDigits();
	if (setTrackNum(m_tagV2, num, numTracks, numDigits)) {
		markTag2Changed(Frame::FT_Track);
	}
}

/**
 * Set ID3v2 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void Mp3File::setGenreV2(const QString& str)
{
	if (!str.isNull()) {
		int num = 0xff;
		if (!Kid3App::s_miscCfg.m_genreNotNumeric) {
			num = Genres::getNumber(str);
		}
		if (num >= 0 && num != 0xff) {
			if (getGenreNum(m_tagV2) != num &&
					setGenreNum(m_tagV2, num)) {
				markTag2Changed(Frame::FT_Genre);
			}
		} else {
			if (getTextField(m_tagV2, ID3FID_CONTENTTYPE) != str &&
					setTextField(m_tagV2, ID3FID_CONTENTTYPE, str, true)) {
				markTag2Changed(Frame::FT_Genre);
			}
		}
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
 * @param info the detail information is returned here
 */
void Mp3File::getDetailInfo(DetailInfo& info) const
{
	if (getFilename().right(4).QCM_toLower() == ".aac") {
		info.valid = true;
		info.format = "AAC";
		return;
	}

	const Mp3_Headerinfo* headerInfo = 0;
	if (m_tagV2) {
		headerInfo = m_tagV2->GetMp3HeaderInfo();
	}
	if (!headerInfo && m_tagV1) {
		headerInfo = m_tagV1->GetMp3HeaderInfo();
	}
	if (headerInfo) {
		info.valid = true;
		switch (headerInfo->version) {
			case MPEGVERSION_1:
				info.format = "MPEG 1 ";
				break;
			case MPEGVERSION_2:
				info.format = "MPEG 2 ";
				break;
			case MPEGVERSION_2_5:
				info.format = "MPEG 2.5 ";
				break;
			default:
				; // nothing
		}
		switch (headerInfo->layer) {
			case MPEGLAYER_I:
				info.format += "Layer 1";
				break;
			case MPEGLAYER_II:
				info.format += "Layer 2";
				break;
			case MPEGLAYER_III:
				info.format += "Layer 3";
				break;
			default:
				; // nothing
		}
		info.bitrate = headerInfo->bitrate / 1000;
#ifndef HAVE_NO_ID3LIB_VBR
		if (headerInfo->vbr_bitrate > 1000) {
			info.vbr = true;
			info.bitrate = headerInfo->vbr_bitrate / 1000;
		}
#endif
		info.sampleRate = headerInfo->frequency;
		switch (headerInfo->channelmode) {
			case MP3CHANNELMODE_STEREO:
				info.channelMode = DetailInfo::CM_Stereo;
				info.channels = 2;
				break;
			case MP3CHANNELMODE_JOINT_STEREO:
				info.channelMode = DetailInfo::CM_JointStereo;
				info.channels = 2;
				break;
			case MP3CHANNELMODE_DUAL_CHANNEL:
				info.channels = 2;
				break;
			case MP3CHANNELMODE_SINGLE_CHANNEL:
				info.channels = 1;
				break;
			default:
				; // nothing
		}
		info.duration = headerInfo->time;
	} else {
		info.valid = false;
	}
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
	if (m_tagV2) {
		info = m_tagV2->GetMp3HeaderInfo();
	}
	if (!info && m_tagV1) {
		info = m_tagV1->GetMp3HeaderInfo();
	}
	if (info && info->time > 0) {
		duration = info->time;
	}
	return duration;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".mp3".
 */
QString Mp3File::getFileExtension() const
{
	QString ext(getFilename().right(4).QCM_toLower());
	if (ext == ".aac" || ext == ".mp2")
		return ext;
	else
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

/** Types and descriptions for id3lib frame IDs */
static const struct TypeStrOfId {
	Frame::Type type;
	const char* str;
} typeStrOfId[] = {
	{ Frame::FT_UnknownFrame,   0 },                                                                   /* ???? */
	{ Frame::FT_Other,          I18N_NOOP("AENC - Audio encryption") },                                /* AENC */
	{ Frame::FT_Picture,        I18N_NOOP("APIC - Attached picture") },                                /* APIC */
	{ Frame::FT_Other,          0 },                                                                   /* ASPI */
	{ Frame::FT_Comment,        I18N_NOOP("COMM - Comments") },                                        /* COMM */
	{ Frame::FT_Other,          I18N_NOOP("COMR - Commercial") },                                      /* COMR */
	{ Frame::FT_Other,          I18N_NOOP("ENCR - Encryption method registration") },                  /* ENCR */
	{ Frame::FT_Other,          0 },                                                                   /* EQU2 */
	{ Frame::FT_Other,          I18N_NOOP("EQUA - Equalization") },                                    /* EQUA */
	{ Frame::FT_Other,          I18N_NOOP("ETCO - Event timing codes") },                              /* ETCO */
	{ Frame::FT_Other,          I18N_NOOP("GEOB - General encapsulated object") },                     /* GEOB */
	{ Frame::FT_Other,          I18N_NOOP("GRID - Group identification registration") },               /* GRID */
	{ Frame::FT_Arranger,       I18N_NOOP("IPLS - Involved people list") },                            /* IPLS */
	{ Frame::FT_Other,          I18N_NOOP("LINK - Linked information") },                              /* LINK */
	{ Frame::FT_Other,          I18N_NOOP("MCDI - Music CD identifier") },                             /* MCDI */
	{ Frame::FT_Other,          I18N_NOOP("MLLT - MPEG location lookup table") },                      /* MLLT */
	{ Frame::FT_Other,          I18N_NOOP("OWNE - Ownership frame") },                                 /* OWNE */
	{ Frame::FT_Other,          I18N_NOOP("PRIV - Private frame") },                                   /* PRIV */
	{ Frame::FT_Other,          I18N_NOOP("PCNT - Play counter") },                                    /* PCNT */
	{ Frame::FT_Other,          I18N_NOOP("POPM - Popularimeter") },                                   /* POPM */
	{ Frame::FT_Other,          I18N_NOOP("POSS - Position synchronisation frame") },                  /* POSS */
	{ Frame::FT_Other,          I18N_NOOP("RBUF - Recommended buffer size") },                         /* RBUF */
	{ Frame::FT_Other,          0 },                                                                   /* RVA2 */
	{ Frame::FT_Other,          I18N_NOOP("RVAD - Relative volume adjustment") },                      /* RVAD */
	{ Frame::FT_Other,          I18N_NOOP("RVRB - Reverb") },                                          /* RVRB */
	{ Frame::FT_Other,          0 },                                                                   /* SEEK */
	{ Frame::FT_Other,          0 },                                                                   /* SIGN */
	{ Frame::FT_Other,          I18N_NOOP("SYLT - Synchronized lyric/text") },                         /* SYLT */
	{ Frame::FT_Other,          I18N_NOOP("SYTC - Synchronized tempo codes") },                        /* SYTC */
	{ Frame::FT_Album,          I18N_NOOP("TALB - Album/Movie/Show title") },                          /* TALB */
	{ Frame::FT_Bpm,            I18N_NOOP("TBPM - BPM (beats per minute)") },                          /* TBPM */
	{ Frame::FT_Composer,       I18N_NOOP("TCOM - Composer") },                                        /* TCOM */
	{ Frame::FT_Genre,          I18N_NOOP("TCON - Content type") },                                    /* TCON */
	{ Frame::FT_Copyright,      I18N_NOOP("TCOP - Copyright message") },                               /* TCOP */
	{ Frame::FT_Other,          I18N_NOOP("TDAT - Date") },                                            /* TDAT */
	{ Frame::FT_Other,          0 },                                                                   /* TDEN */
	{ Frame::FT_Other,          I18N_NOOP("TDLY - Playlist delay") },                                  /* TDLY */
	{ Frame::FT_Other,          0 },                                                                   /* TDOR */
	{ Frame::FT_Other,          0 },                                                                   /* TDRC */
	{ Frame::FT_Other,          0 },                                                                   /* TDRL */
	{ Frame::FT_Other,          0 },                                                                   /* TDTG */
	{ Frame::FT_Other,          0 },                                                                   /* TIPL */
	{ Frame::FT_EncodedBy,      I18N_NOOP("TENC - Encoded by") },                                      /* TENC */
	{ Frame::FT_Lyricist,       I18N_NOOP("TEXT - Lyricist/Text writer") },                            /* TEXT */
	{ Frame::FT_Other,          I18N_NOOP("TFLT - File type") },                                       /* TFLT */
	{ Frame::FT_Other,          I18N_NOOP("TIME - Time") },                                            /* TIME */
	{ Frame::FT_Grouping,       I18N_NOOP("TIT1 - Content group description") },                       /* TIT1 */
	{ Frame::FT_Title,          I18N_NOOP("TIT2 - Title/songname/content description") },              /* TIT2 */
	{ Frame::FT_Subtitle,       I18N_NOOP("TIT3 - Subtitle/Description refinement") },                 /* TIT3 */
	{ Frame::FT_Other,          I18N_NOOP("TKEY - Initial key") },                                     /* TKEY */
	{ Frame::FT_Language,       I18N_NOOP("TLAN - Language(s)") },                                     /* TLAN */
	{ Frame::FT_Other,          I18N_NOOP("TLEN - Length") },                                          /* TLEN */
	{ Frame::FT_Other,          0 },                                                                   /* TMCL */
	{ Frame::FT_Media,          I18N_NOOP("TMED - Media type") },                                      /* TMED */
	{ Frame::FT_Other,          0 },                                                                   /* TMOO */
	{ Frame::FT_OriginalAlbum,  I18N_NOOP("TOAL - Original album/movie/show title") },                 /* TOAL */
	{ Frame::FT_Other,          I18N_NOOP("TOFN - Original filename") },                               /* TOFN */
	{ Frame::FT_Author,         I18N_NOOP("TOLY - Original lyricist(s)/text writer(s)") },             /* TOLY */
	{ Frame::FT_OriginalArtist, I18N_NOOP("TOPE - Original artist(s)/performer(s)") },                 /* TOPE */
	{ Frame::FT_OriginalDate,   I18N_NOOP("TORY - Original release year") },                           /* TORY */
	{ Frame::FT_Other,          I18N_NOOP("TOWN - File owner/licensee") },                             /* TOWN */
	{ Frame::FT_Artist,         I18N_NOOP("TPE1 - Lead performer(s)/Soloist(s)") },                    /* TPE1 */
	{ Frame::FT_AlbumArtist,    I18N_NOOP("TPE2 - Band/orchestra/accompaniment") },                    /* TPE2 */
	{ Frame::FT_Conductor,      I18N_NOOP("TPE3 - Conductor/performer refinement") },                  /* TPE3 */
	{ Frame::FT_Remixer,        I18N_NOOP("TPE4 - Interpreted, remixed, or otherwise modified by") },  /* TPE4 */
	{ Frame::FT_Disc,           I18N_NOOP("TPOS - Part of a set") },                                   /* TPOS */
	{ Frame::FT_Other,          0 },                                                                   /* TPRO */
	{ Frame::FT_Publisher,      I18N_NOOP("TPUB - Publisher") },                                       /* TPUB */
	{ Frame::FT_Track,          I18N_NOOP("TRCK - Track number/Position in set") },                    /* TRCK */
	{ Frame::FT_Other,          I18N_NOOP("TRDA - Recording dates") },                                 /* TRDA */
	{ Frame::FT_Other,          I18N_NOOP("TRSN - Internet radio station name") },                     /* TRSN */
	{ Frame::FT_Other,          I18N_NOOP("TRSO - Internet radio station owner") },                    /* TRSO */
	{ Frame::FT_Other,          I18N_NOOP("TSIZ - Size") },                                            /* TSIZ */
	{ Frame::FT_Other,          0 },                                                                   /* TSOA */
	{ Frame::FT_Other,          0 },                                                                   /* TSOP */
	{ Frame::FT_Other,          0 },                                                                   /* TSOT */
	{ Frame::FT_Isrc,           I18N_NOOP("TSRC - ISRC (international standard recording code)") },    /* TSRC */
	{ Frame::FT_Other,          I18N_NOOP("TSSE - Software/Hardware and settings used for encoding") },/* TSSE */
	{ Frame::FT_Part,           0 },                                                                   /* TSST */
	{ Frame::FT_Other,          I18N_NOOP("TXXX - User defined text information") },                   /* TXXX */
	{ Frame::FT_Date,           I18N_NOOP("TYER - Year") },                                            /* TYER */
	{ Frame::FT_Other,          I18N_NOOP("UFID - Unique file identifier") },                          /* UFID */
	{ Frame::FT_Other,          I18N_NOOP("USER - Terms of use") },                                    /* USER */
	{ Frame::FT_Lyrics,         I18N_NOOP("USLT - Unsynchronized lyric/text transcription") },         /* USLT */
	{ Frame::FT_Other,          I18N_NOOP("WCOM - Commercial information") },                          /* WCOM */
	{ Frame::FT_Other,          I18N_NOOP("WCOP - Copyright/Legal information") },                     /* WCOP */
	{ Frame::FT_Other,          I18N_NOOP("WOAF - Official audio file webpage") },                     /* WOAF */
	{ Frame::FT_Website,        I18N_NOOP("WOAR - Official artist/performer webpage") },               /* WOAR */
	{ Frame::FT_Other,          I18N_NOOP("WOAS - Official audio source webpage") },                   /* WOAS */
	{ Frame::FT_Other,          I18N_NOOP("WORS - Official internet radio station homepage") },        /* WORS */
	{ Frame::FT_Other,          I18N_NOOP("WPAY - Payment") },                                         /* WPAY */
	{ Frame::FT_Other,          I18N_NOOP("WPUB - Official publisher webpage") },                      /* WPUB */
	{ Frame::FT_Other,          I18N_NOOP("WXXX - User defined URL link") }                            /* WXXX */
};

/** Not instantiated class to check array size at compilation time. */
class not_used { int array_size_check[
		sizeof(typeStrOfId) / sizeof(typeStrOfId[0]) == ID3FID_WWWUSER + 1
		? 1 : -1 ]; };

/**
 * Get type and description of frame.
 *
 * @param id ID of frame
 * @param type the type is returned here
 * @param str  the description is returned here, 0 if not available
 */
static void getTypeStringForId3libFrameId(ID3_FrameID id, Frame::Type& type,
																					const char*& str)
{
	const TypeStrOfId& ts = typeStrOfId[id <= ID3FID_WWWUSER ? id : 0];
	type = ts.type;
	str = ts.str;
}

/**
 * Get id3lib frame ID for frame type.
 *
 * @param type frame type
 *
 * @return id3lib frame ID or ID3FID_NOFRAME if type not found.
 */
static ID3_FrameID getId3libFrameIdForType(Frame::Type type)
{
	// IPLS is mapped to FT_Arranger and FT_Performer
	if (type == Frame::FT_Performer) {
		return ID3FID_INVOLVEDPEOPLE;
	}

	static int typeIdMap[Frame::FT_LastFrame + 1] = { -1, };
	if (typeIdMap[0] == -1) {
		// first time initialization
		for (int i = 0; i <= ID3FID_WWWUSER; ++i) {
			int t = typeStrOfId[i].type;
			if (t <= Frame::FT_LastFrame) {
				typeIdMap[t] = i;
			}
		}
	}
	return type <= Frame::FT_LastFrame ?
		static_cast<ID3_FrameID>(typeIdMap[type]) : ID3FID_NOFRAME;
}

/**
 * Get id3lib frame ID for frame name.
 *
 * @param name frame name
 *
 * @return id3lib frame ID or ID3FID_NOFRAME if name not found.
 */
static ID3_FrameID getId3libFrameIdForName(const QString& name)
{
	if (name.length() >= 4) {
		const char* nameStr = name.QCM_latin1();
		for (int i = 0; i <= ID3FID_WWWUSER; ++i) {
			const char* s = typeStrOfId[i].str;
			if (s && ::strncmp(s, nameStr, 4) == 0) {
				return static_cast<ID3_FrameID>(i);
			}
		}
	}
	return ID3FID_NOFRAME;
}

/**
 * Get the fields from an ID3v2 tag.
 *
 * @param id3Frame frame
 * @param fields   the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromId3Frame(ID3_Frame* id3Frame,
																		 Frame::FieldList& fields)
{
	QString text;
	ID3_Frame::Iterator* iter = id3Frame->CreateIterator();
	ID3_FrameID id3Id = id3Frame->GetID();
	ID3_Field* id3Field;
	Frame::Field field;
	while ((id3Field = iter->GetNext()) != 0) {
		ID3_FieldID id = id3Field->GetID();
		ID3_FieldType type = id3Field->GetType();
		field.m_id = id;
		if (type == ID3FTY_INTEGER) {
			field.m_value = id3Field->Get();
		}
		else if (type == ID3FTY_BINARY) {
			QByteArray ba;
			QCM_duplicate(ba,
										(const char *)id3Field->GetRawBinary(),
										(unsigned int)id3Field->Size());
			field.m_value = ba;
		}
		else if (type == ID3FTY_TEXTSTRING) {
			if (id == ID3FN_TEXT || id == ID3FN_DESCRIPTION || id == ID3FN_URL) {
				text = getString(id3Field);
				if (id3Id == ID3FID_CONTENTTYPE) {
					text = Genres::getNameString(text);
				}
				field.m_value = text;
			} else {
				field.m_value = getString(id3Field);
			}
		} else {
			field.m_value.clear();
		}
		fields.push_back(field);
	}
#ifdef WIN32
	/* allocated in Windows DLL => must be freed in the same DLL */
	ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
	delete iter;
#endif
	return text;
}

/**
 * Get ID3v2 frame by index.
 *
 * @param tag   ID3v2 tag
 * @param index index
 *
 * @return frame, 0 if index invalid.
 */
static ID3_Frame* getId3v2Frame(ID3_Tag* tag, int index)
{
	ID3_Frame* result = 0;
	if (tag) {
		ID3_Frame* frame;
		ID3_Tag::Iterator* iter = tag->CreateIterator();
		int i = 0;
		while ((frame = iter->GetNext()) != 0) {
			if (i == index) {
				result = frame;
				break;
			}
			++i;
		}
#ifdef WIN32
		/* allocated in Windows DLL => must be freed in the same DLL */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
	}
	return result;
}

/**
 * Set the fields in an id3lib frame from the field in the frame.
 *
 * @param id3Frame id3lib frame
 * @param frame    frame with fields
 */
void Mp3File::setId3v2Frame(ID3_Frame* id3Frame, const Frame& frame) const
{
	ID3_Frame::Iterator* iter = id3Frame->CreateIterator();
	ID3_FrameID id3Id = id3Frame->GetID();
	ID3_Field* id3Field;
	ID3_TextEnc enc = ID3TE_NONE;
	for (Frame::FieldList::const_iterator fldIt = frame.getFieldList().begin();
			 fldIt != frame.getFieldList().end();
			 ++fldIt) {
		id3Field = iter->GetNext();
		if (!id3Field) {
			qDebug("early end of ID3 fields");
			break;
		}
		const Frame::Field& fld = *fldIt;
		switch (fld.m_value.type()) {
			case QVariant::Int:
			case QVariant::UInt:
			{
				int intVal = fld.m_value.toInt();
				if (fld.m_id == ID3FN_TEXTENC) {
					if (intVal == ID3TE_UTF8) intVal = ID3TE_UTF16;
					enc = static_cast<ID3_TextEnc>(intVal);
				}
				id3Field->Set(intVal);
				break;
			}

			case QVariant::String:
			{
				if (enc != ID3TE_NONE) {
					id3Field->SetEncoding(enc);
				}
				QString value(fld.m_value.toString());
				if (id3Id == ID3FID_CONTENTTYPE) {
					if (!Kid3App::s_miscCfg.m_genreNotNumeric) {
						value = Genres::getNumberString(value, true);
					}
				} else if (id3Id == ID3FID_TRACKNUM) {
					formatTrackNumberIfEnabled(value, true);
				}
				setString(id3Field, value);
				break;
			}

			case QVariant::ByteArray:
			{
				const QByteArray& ba = fld.m_value.toByteArray();
				id3Field->Set(reinterpret_cast<const unsigned char*>(ba.data()),
											ba.size());
				break;
			}

			default:
				qDebug("Unknown type %d in field %d", fld.m_value.type(), fld.m_id);
		}
	}
#ifdef WIN32
	/* allocated in Windows DLL => must be freed in the same DLL */
	ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
	delete iter;
#endif
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool Mp3File::setFrameV2(const Frame& frame)
{
	// If the frame has an index, change that specific frame
	int index = frame.getIndex();
	if (index != -1 && m_tagV2) {
		ID3_Frame* id3Frame = getId3v2Frame(m_tagV2, index);
		if (id3Frame) {
			// If value is changed or field list is empty,
			// set from value, else from FieldList.
			if (frame.isValueChanged() || frame.getFieldList().empty()) {
				QString value(frame.getValue());
				ID3_Field* fld;
				if ((fld = id3Frame->GetField(ID3FN_URL)) != 0) {
					if (getString(fld) != value) {
						fld->Set(value.QCM_latin1());
						markTag2Changed(frame.getType());
					}
					return true;
				} else if ((fld = id3Frame->GetField(ID3FN_TEXT)) != 0 ||
						(fld = id3Frame->GetField(ID3FN_DESCRIPTION)) != 0) {
					if (id3Frame->GetID() == ID3FID_CONTENTTYPE) {
						if (!Kid3App::s_miscCfg.m_genreNotNumeric) {
							value = Genres::getNumberString(value, true);
						}
					} else if (id3Frame->GetID() == ID3FID_TRACKNUM) {
						formatTrackNumberIfEnabled(value, true);
					}
					if (fld->GetEncoding() == ID3TE_ISO8859_1) {
						// check if information is lost if the string is not unicode
						uint i, unicode_size = value.length();
						const QChar* qcarray = value.unicode();
						for (i = 0; i < unicode_size; i++) {
							char ch = qcarray[i].
#if QT_VERSION >= 0x040000
								toLatin1()
#else
								latin1()
#endif
								;
							if (ch == 0 || (ch & 0x80) != 0) {
								ID3_Field* encfld = id3Frame->GetField(ID3FN_TEXTENC);
								if (encfld) {
									encfld->Set(ID3TE_UTF16);
								}
								fld->SetEncoding(ID3TE_UTF16);
								break;
							}
						}
					}
					if (getString(fld) != value) {
						setString(fld, value);
						markTag2Changed(frame.getType());
					}
					return true;
				} else if (id3Frame->GetID() == ID3FID_PRIVATE &&
					         (fld = id3Frame->GetField(ID3FN_DATA)) != 0) {
					ID3_Field* ownerFld = id3Frame->GetField(ID3FN_OWNER);
					QString owner;
					QByteArray newData, oldData;
					if (ownerFld && !(owner = getString(ownerFld)).isEmpty() &&
							AttributeData(owner).toByteArray(value, newData)) {
						QCM_duplicate(oldData,
						              (const char *)fld->GetRawBinary(),
						              (unsigned int)fld->Size());
						if (newData != oldData) {
							fld->Set(reinterpret_cast<const unsigned char*>(newData.data()),
							         newData.size());
							markTag2Changed(frame.getType());
						}
						return true;
					}
				} else if (id3Frame->GetID() == ID3FID_CDID &&
					         (fld = id3Frame->GetField(ID3FN_DATA)) != 0) {
					QByteArray newData, oldData;
					if (AttributeData::isHexString(value, 'F', "+") &&
							AttributeData(AttributeData::Utf16).toByteArray(value, newData)) {
						QCM_duplicate(oldData,
						              (const char *)fld->GetRawBinary(),
						              (unsigned int)fld->Size());
						if (newData != oldData) {
							fld->Set(reinterpret_cast<const unsigned char*>(newData.data()),
							         newData.size());
							markTag2Changed(frame.getType());
						}
						return true;
					}
				} else if (id3Frame->GetID() == ID3FID_UNIQUEFILEID &&
					         (fld = id3Frame->GetField(ID3FN_DATA)) != 0) {
					QByteArray newData, oldData;
					if (AttributeData::isHexString(value, 'Z')) {
#if QT_VERSION >= 0x040000
						newData = (value + '\0').toLatin1();
#else
						newData.duplicate((value + '\0').latin1(), value.length() + 1);
#endif
						QCM_duplicate(oldData,
						              (const char *)fld->GetRawBinary(),
						              (unsigned int)fld->Size());
						if (newData != oldData) {
							fld->Set(reinterpret_cast<const unsigned char*>(newData.data()),
							         newData.size());
							markTag2Changed(frame.getType());
						}
						return true;
					}
				} else if (id3Frame->GetID() == ID3FID_POPULARIMETER &&
					         (fld = id3Frame->GetField(ID3FN_RATING)) != 0) {
					if (getString(fld) != value) {
						fld->Set(value.toInt());
						markTag2Changed(frame.getType());
					}
					return true;
				}
			} else {
				setId3v2Frame(id3Frame, frame);
				markTag2Changed(frame.getType());
				return true;
			}
		}
	}

	// Try the superclass method
	return TaggedFile::setFrameV2(frame);
}


/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool Mp3File::addFrameV2(Frame& frame)
{
	// Add a new frame.
	ID3_FrameID id;
	if (frame.getType() != Frame::FT_Other) {
		id = getId3libFrameIdForType(frame.getType());
	} else {
		id = getId3libFrameIdForName(frame.getName());
		if (id == ID3FID_NOFRAME) {
			id = ID3FID_USERTEXT;
		}
	}
	if (id != ID3FID_NOFRAME && id != ID3FID_SETSUBTITLE && m_tagV2) {
		ID3_Frame* id3Frame = new ID3_Frame(id);
		if (id3Frame) {
			ID3_Field* fld = id3Frame->GetField(ID3FN_TEXT);
			if (fld) {
				ID3_TextEnc enc = getDefaultTextEncoding();
				ID3_Field* encfld = id3Frame->GetField(ID3FN_TEXTENC);
				if (encfld) {
					encfld->Set(enc);
				}
				fld->SetEncoding(enc);
			}
			if (id == ID3FID_USERTEXT && !frame.getName().startsWith("TXXX")) {
				fld = id3Frame->GetField(ID3FN_DESCRIPTION);
				if (fld) {
					setString(fld, frame.getName());
				}
			} else if (id == ID3FID_PICTURE) {
				fld = id3Frame->GetField(ID3FN_MIMETYPE);
				if (fld) {
					setString(fld, "image/jpeg");
				}
				fld = id3Frame->GetField(ID3FN_PICTURETYPE);
				if (fld) {
					fld->Set(ID3PT_COVERFRONT);
				}
			}
			if (!frame.fieldList().empty()) {
				setId3v2Frame(id3Frame, frame);
			}
			Frame::Type type;
			const char* name;
			getTypeStringForId3libFrameId(id, type, name);
			m_tagV2->AttachFrame(id3Frame);
			frame.setInternalName(name);
			frame.setIndex(m_tagV2->NumFrames() - 1);
			if (frame.fieldList().empty()) {
				// add field list to frame
				getFieldsFromId3Frame(id3Frame, frame.fieldList());
				frame.setFieldListFromValue();
			}
			markTag2Changed(frame.getType());
			return true;
		}
	}

	// Try the superclass method
	return TaggedFile::addFrameV2(frame);
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool Mp3File::deleteFrameV2(const Frame& frame)
{
	// If the frame has an index, delete that specific frame
	int index = frame.getIndex();
	if (index != -1 && m_tagV2) {
		ID3_Frame* id3Frame = getId3v2Frame(m_tagV2, index);
		if (id3Frame) {
			m_tagV2->RemoveFrame(id3Frame);
			markTag2Changed(frame.getType());
			return true;
		}
	}

	// Try the superclass method
	return TaggedFile::deleteFrameV2(frame);
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void Mp3File::deleteFramesV2(const FrameFilter& flt)
{
	if (m_tagV2) {
		if (flt.areAllEnabled()) {
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
			markTag2Changed(Frame::FT_UnknownFrame);
		} else {
			ID3_Tag::Iterator* iter = m_tagV2->CreateIterator();
			ID3_Frame* frame;
			while ((frame = iter->GetNext()) != NULL) {
				Frame::Type type;
				const char* name;
				getTypeStringForId3libFrameId(frame->GetID(), type, name);
				if (flt.isEnabled(type, name)) {
					m_tagV2->RemoveFrame(frame);
				}
			}
#ifdef WIN32
			/* allocated in Windows DLL => must be freed in the same DLL */
			ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
			delete iter;
#endif
			markTag2Changed(Frame::FT_UnknownFrame);
		}
	}
}

/**
 * Get all frames in tag 2.
 *
 * @param frames frame collection to set.
 */
void Mp3File::getAllFramesV2(FrameCollection& frames)
{
	frames.clear();
	if (m_tagV2) {
		ID3_Tag::Iterator* iter = m_tagV2->CreateIterator();
		ID3_Frame* id3Frame;
		int i = 0;
		Frame::Type type;
		const char* name;
		while ((id3Frame = iter->GetNext()) != 0) {
			getTypeStringForId3libFrameId(id3Frame->GetID(), type, name);
			Frame frame(type, "", name, i++);
			frame.setValue(getFieldsFromId3Frame(id3Frame, frame.fieldList()));
			if (id3Frame->GetID() == ID3FID_USERTEXT ||
					id3Frame->GetID() == ID3FID_WWWUSER ||
					id3Frame->GetID() == ID3FID_COMMENT) {
				QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Description);
				if (fieldValue.isValid()) {
					QString description = fieldValue.toString();
					if (!description.isEmpty()) {
						frame.setInternalName(QString(name) + '\n' + description);
						frame.setType(Frame::FT_Other);
					}
				}
			} else if (id3Frame->GetID() == ID3FID_PRIVATE) {
				const Frame::FieldList& fields = frame.getFieldList();
				QString owner;
				QByteArray data;
				for (Frame::FieldList::const_iterator it = fields.begin();
				     it != fields.end();
				     ++it) {
					if ((*it).m_id == Frame::Field::ID_Owner) {
						owner = (*it).m_value.toString();
						if (!owner.isEmpty()) {
							frame.setInternalName(QString(name) + '\n' + owner);
						}
					} else if ((*it).m_id == Frame::Field::ID_Data) {
						data = (*it).m_value.toByteArray();
					}
				}
				if (!owner.isEmpty() && !data.isEmpty()) {
					QString str;
					if (AttributeData(owner).toString(data, str)) {
						frame.setValue(str);
					}
				}
			} else if (id3Frame->GetID() == ID3FID_CDID) {
				QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Data);
				if (fieldValue.isValid()) {
					QString str;
					if (AttributeData(AttributeData::Utf16).toString(fieldValue.toByteArray(), str) &&
							AttributeData::isHexString(str, 'F', "+")) {
						frame.setValue(str);
					}
				}
			} else if (id3Frame->GetID() == ID3FID_UNIQUEFILEID) {
				QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Data);
				if (fieldValue.isValid()) {
					QByteArray ba(fieldValue.toByteArray());
					QString str(ba);
					if (ba.size() - str.length() <= 1 &&
							AttributeData::isHexString(str, 'Z')) {
						frame.setValue(str);
					}
				}
			} else if (id3Frame->GetID() == ID3FID_POPULARIMETER) {
				QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Rating);
				if (fieldValue.isValid()) {
					QString str(fieldValue.toString());
					if (!str.isEmpty()) {
						frame.setValue(str);
					}
				}
			}
			frames.insert(frame);
		}
#ifdef WIN32
		/* allocated in Windows DLL => must be freed in the same DLL */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
	}
	frames.addMissingStandardFrames();
}

/**
 * Get a list of frame IDs which can be added.
 *
 * @return list with frame IDs.
 */
QStringList Mp3File::getFrameIds() const
{
	QStringList lst;
	for (int type = Frame::FT_FirstFrame; type <= Frame::FT_LastFrame; ++type) {
		if (type != Frame::FT_Part) {
			lst.append(QCM_translate(Frame::getNameFromType(
																 static_cast<Frame::Type>(type))));
		}
	}
	for (int i = 0; i <= ID3FID_WWWUSER; ++i) {
		if (typeStrOfId[i].type == Frame::FT_Other) {
			const char* s = typeStrOfId[i].str;
			if (s) {
				lst.append(QCM_translate(s));
			}
		}
	}
	return lst;
}

/**
 * Set the text codec to be used for tag 1.
 *
 * @param codec text codec, 0 to use default (ISO 8859-1)
 */
void Mp3File::setTextCodecV1(const QTextCodec* codec)
{
	s_textCodecV1 = codec;
}

/**
 * Set the default text encoding.
 *
 * @param textEnc default text encoding
 */
void Mp3File::setDefaultTextEncoding(MiscConfig::TextEncoding textEnc)
{
	// UTF8 encoding is buggy, so UTF16 is used when UTF8 is configured
	s_defaultTextEncoding = textEnc == MiscConfig::TE_ISO8859_1 ?
		ID3TE_ISO8859_1 : ID3TE_UTF16;
}


/**
 * Create an Mp3File object if it supports the filename's extension.
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* Mp3File::Resolver::createFile(const DirInfo* di,
																					const QString& fn) const
{
	QString ext = fn.right(4).QCM_toLower();
	if ((ext == ".mp3" || ext == ".mp2" || ext == ".aac")
#ifdef HAVE_TAGLIB
			&& Kid3App::s_miscCfg.m_id3v2Version != MiscConfig::ID3v2_4_0
#endif
		)
		return new Mp3File(di, fn);
	else
		return 0;
}

/**
 * Get a list with all extensions supported by Mp3File.
 *
 * @return list of file extensions.
 */
QStringList Mp3File::Resolver::getSupportedFileExtensions() const
{
	return QStringList() << ".mp3" << ".mp2" << ".aac";
}

#endif // HAVE_ID3LIB
