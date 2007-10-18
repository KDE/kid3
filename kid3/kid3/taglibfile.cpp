/**
 * \file taglibfile.cpp
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#include "taglibfile.h"
#ifdef HAVE_TAGLIB

#include <qdir.h>
#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QByteArray>
#endif

#include "standardtags.h"
#include "genres.h"
#include "dirinfo.h"
#include "kid3.h"
#include <sys/stat.h>
#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

// Just using include <oggfile.h>, include <flacfile.h> as recommended in the
// TagLib documentation does not work, as there are files with these names
// in this directory.
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/flacfile.h>
#include <taglib/mpcfile.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2header.h>
#include <taglib/apetag.h>
#include <taglib/textidentificationframe.h>
#include <taglib/commentsframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/uniquefileidentifierframe.h>

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
#include <taglib/generalencapsulatedobjectframe.h>
#endif
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
#include <taglib/urllinkframe.h>
#else
#include "taglibext/urllinkframe.h"
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
#include <taglib/unsynchronizedlyricsframe.h>
#else
#include "taglibext/unsynchronizedlyricsframe.h"
#endif

/**
 * Constructor.
 *
 * @param di directory information
 * @param fn filename
 */
TagLibFile::TagLibFile(const DirInfo* di, const QString& fn) :
	TaggedFile(di, fn), m_tagV1(0), m_tagV2(0), m_fileRead(false)
{
}

/**
 * Destructor.
 */
TagLibFile::~TagLibFile()
{
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void TagLibFile::readTags(bool force)
{
	QCM_QCString fn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());

	if (force || m_fileRef.isNull()) {
		m_fileRef = TagLib::FileRef(fn);
		m_tagV1 = 0;
		m_tagV2 = 0;
		markTag1Changed(false);
		markTag2Changed(false);
		m_fileRead = true;
	}

	TagLib::File* file;
	if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
		TagLib::MPEG::File* mpegFile;
		TagLib::FLAC::File* flacFile;
#ifdef MPC_ID3V1
		TagLib::MPC::File* mpcFile;
#endif
		if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = mpegFile->ID3v1Tag();
				markTag1Changed(false);
			}
			if (!m_tagV2) {
				m_tagV2 = mpegFile->ID3v2Tag();
				markTag2Changed(false);
			}
		} else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = flacFile->ID3v1Tag();
				markTag1Changed(false);
			}
			if (!m_tagV2) {
				m_tagV2 = flacFile->xiphComment();
				markTag2Changed(false);
			}
#ifdef MPC_ID3V1
		} else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = mpcFile->ID3v1Tag();
				markTag1Changed(false);
			}
			if (!m_tagV2) {
				m_tagV2 = mpcFile->APETag();
				markTag2Changed(false);
			}
#endif
		} else {
			m_tagV1 = 0;
			markTag1Changed(false);
			if (!m_tagV2) {
				m_tagV2 = m_fileRef.tag();
				markTag2Changed(false);
			}
		}
	}

	if (force) {
		setFilename(currentFilename());
	}
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force    true to force writing even if file was not changed.
 * @param renamed  will be set to true if the file was renamed,
 *                 i.e. the file name is no longer valid, else *renamed
 *                 is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool TagLibFile::writeTags(bool force, bool* renamed, bool preserve)
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

	bool fileChanged = false;
	TagLib::File* file;
	if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
		TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(file);
		if (mpegFile) {
			if (m_tagV1 && (force || isTag1Changed()) && m_tagV1->isEmpty()) {
				mpegFile->strip(TagLib::MPEG::File::ID3v1);
				fileChanged = true;
				markTag1Changed(false);
				m_tagV1 = 0;
			}
			if (m_tagV2 && (force || isTag2Changed()) && m_tagV2->isEmpty()) {
				mpegFile->strip(TagLib::MPEG::File::ID3v2);
				fileChanged = true;
				markTag2Changed(false);
				m_tagV2 = 0;
			}
			int saveMask = 0;
			if (m_tagV1 && (force || isTag1Changed()) && !m_tagV1->isEmpty()) {
				saveMask |= TagLib::MPEG::File::ID3v1;
			}
			if (m_tagV2 && (force || isTag2Changed()) && !m_tagV2->isEmpty()) {
				saveMask |= TagLib::MPEG::File::ID3v2;
			}
			if (saveMask != 0) {
				if (mpegFile->save(saveMask, false)) {
					fileChanged = true;
					if (saveMask & TagLib::MPEG::File::ID3v1) {
						markTag1Changed(false);
					}
					if (saveMask & TagLib::MPEG::File::ID3v2) {
						markTag2Changed(false);
					}
				}
			}
		} else {
			if ((m_tagV2 && (force || isTag2Changed())) ||
					(m_tagV1 && (force || isTag1Changed()))) {
				TagLib::MPC::File* mpcFile = dynamic_cast<TagLib::MPC::File*>(file);
#ifndef MPC_ID3V1
				// it does not work if there is also an ID3 tag (bug in TagLib?)
				if (mpcFile) {
					mpcFile->remove(TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2);
					fileChanged = true;
				}
#endif
				if (m_fileRef.save()) {
					fileChanged = true;
					markTag1Changed(false);
					markTag2Changed(false);
				}
			}
		}
	}

	// If the file was changed, make sure it is written to disk.
	// This is done when the file is closed, which is only done
	// in the TagLib::File destructor. To force destruction, a new
	// file reference is assigned, later readTags() is called.
	// If the file is not properly closed, doubled tags can be
	// written if the file is finally closed!
	// This can be reproduced with an untagged MP3 file, then add
	// an ID3v2 title, save, add an ID3v2 artist, save, reload
	// => double ID3v2 tags.
	// On Windows it is necessary to close the file before renaming it,
	// so it is done even if the file is not changed.
#ifndef WIN32
	if (fileChanged)
#endif
		m_fileRef = TagLib::FileRef();

	// restore time stamp
	if (setUtime) {
		::utime(fn, &times);
	}

	if (getFilename() != currentFilename()) {
		if (!renameFile(currentFilename(), getFilename())) {
			return false;
		}
		updateCurrentFilename();
		*renamed = true;
	}

#ifndef WIN32
	if (fileChanged)
#endif
		readTags(true);
	return true;
}

/**
 * Remove ID3v1 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void TagLibFile::deleteFramesV1(const FrameFilter& flt)
{
	if (m_tagV1) {
		TaggedFile::deleteFramesV1(flt);
	}
}

/**
 * Get ID3v1 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getTitleV1()
{
	if (m_tagV1) {
		TagLib::String str = m_tagV1->title();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v1 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getArtistV1()
{
	if (m_tagV1) {
		TagLib::String str = m_tagV1->artist();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v1 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getAlbumV1()
{
	if (m_tagV1) {
		TagLib::String str = m_tagV1->album();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v1 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getCommentV1()
{
	if (m_tagV1) {
		TagLib::String str = m_tagV1->comment();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v1 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TagLibFile::getYearV1()
{
	if (m_tagV1) {
		return m_tagV1->year();
	} else {
		return -1;
	}
}

/**
 * Get ID3v1 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TagLibFile::getTrackNumV1()
{
	if (m_tagV1) {
		return m_tagV1->track();
	} else {
		return -1;
	}
}

/**
 * Get ID3v1 genre.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getGenreV1()
{
	if (m_tagV1) {
		TagLib::String str = m_tagV1->genre();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v2 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getTitleV2()
{
	if (m_tagV2) {
		TagLib::String str = m_tagV2->title();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v2 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getArtistV2()
{
	if (m_tagV2) {
		TagLib::String str = m_tagV2->artist();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v2 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getAlbumV2()
{
	if (m_tagV2) {
		TagLib::String str = m_tagV2->album();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v2 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getCommentV2()
{
	if (m_tagV2) {
		TagLib::String str = m_tagV2->comment();
		return str.isNull() ? QString("") : TStringToQString(str);
	} else {
		return QString::null;
	}
}

/**
 * Get ID3v2 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TagLibFile::getYearV2()
{
	if (m_tagV2) {
		return m_tagV2->year();
	} else {
		return -1;
	}
}

/**
 * Get ID3v2 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TagLibFile::getTrackNumV2()
{
	if (m_tagV2) {
		return m_tagV2->track();
	} else {
		return -1;
	}
}

/**
 * Get a genre string from a string which can contain the genre itself,
 * or only the genre number or the genre number in parenthesis.
 *
 * @param str genre string
 *
 * @return genre.
 */
static QString getGenreString(const TagLib::String& str)
{
	if (!str.isNull()) {
		QString qs = TStringToQString(str);
		int cpPos = 0, n = 0xff;
		bool ok = false;
		if (qs[0] == '(' && (cpPos = qs.QCM_indexOf(')', 2)) > 1) {
			n = qs.mid(1, cpPos - 1).toInt(&ok);
			if (!ok || n > 0xff) {
				n = 0xff;
			}
			return Genres::getName(n);
		} else if ((n = qs.toInt(&ok)) >= 0 && n <= 0xff && ok) {
			return Genres::getName(n);
		} else {
			return qs;
		}
	} else {
		return "";
	}
}

/**
 * Get ID3v2 genre as text.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TagLibFile::getGenreV2()
{
	if (m_tagV2) {
		return getGenreString(m_tagV2->genre());
	} else {
		return QString::null;
	}
}

/**
 * Create m_tagV1 if it does not already exists so that it can be set.
 *
 * @return true if m_tagV1 can be set.
 */
bool TagLibFile::makeTagV1Settable()
{
	if (!m_tagV1) {
		TagLib::File* file;
		if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
			TagLib::MPEG::File* mpegFile;
			TagLib::FLAC::File* flacFile;
#ifdef MPC_ID3V1
			TagLib::MPC::File* mpcFile;
#endif
			if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
				m_tagV1 = mpegFile->ID3v1Tag(true);
			} else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
				m_tagV1 = flacFile->ID3v1Tag(true);
#ifdef MPC_ID3V1
			} else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
				m_tagV1 = mpcFile->ID3v1Tag(true);
#endif
			}
		}
	}
	return (m_tagV1 != 0);
}

/**
 * Create m_tagV2 if it does not already exist so that it can be set.
 *
 * @return true if m_tagV2 can be set.
 */
bool TagLibFile::makeTagV2Settable()
{
	if (!m_tagV2) {
		TagLib::File* file;
		if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
			TagLib::MPEG::File* mpegFile;
			TagLib::FLAC::File* flacFile;
			TagLib::MPC::File* mpcFile;
			if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
				m_tagV2 = mpegFile->ID3v2Tag(true);
			} else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
				m_tagV2 = flacFile->xiphComment(true);
			} else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
				m_tagV2 = mpcFile->APETag(true);
			}
		}
	}
	return (m_tagV2 != 0);
}

/**
 * Set ID3v1 title.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setTitleV1(const QString& str)
{
	if (makeTagV1Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV1->title())) {
			QString s = checkTruncation(str, StandardTags::TF_Title);
			if (!s.isNull())
				m_tagV1->setTitle(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setTitle(tstr);
			markTag1Changed();
		}
	}
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setArtistV1(const QString& str)
{
	if (makeTagV1Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV1->artist())) {
			QString s = checkTruncation(str, StandardTags::TF_Artist);
			if (!s.isNull())
				m_tagV1->setArtist(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setArtist(tstr);
			markTag1Changed();
		}
	}
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setAlbumV1(const QString& str)
{
	if (makeTagV1Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV1->album())) {
			QString s = checkTruncation(str, StandardTags::TF_Album);
			if (!s.isNull())
				m_tagV1->setAlbum(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setAlbum(tstr);
			markTag1Changed();
		}
	}
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setCommentV1(const QString& str)
{
	if (makeTagV1Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV1->comment())) {
			QString s = checkTruncation(str, StandardTags::TF_Comment, 28);
			if (!s.isNull())
				m_tagV1->setComment(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setComment(tstr);
			markTag1Changed();
		}
	}
}

/**
 * Set ID3v1 year.
 *
 * @param num number to set, 0 to remove field.
 */
void TagLibFile::setYearV1(int num)
{
	if (makeTagV1Settable() && num >= 0) {
		if (num != static_cast<int>(m_tagV1->year())) {
			m_tagV1->setYear(num);
			markTag1Changed();
		}
	}
}

/**
 * Set ID3v1 track.
 *
 * @param num number to set, 0 to remove field.
 */
void TagLibFile::setTrackNumV1(int num)
{
	if (makeTagV1Settable() && num >= 0) {
		if (num != static_cast<int>(m_tagV1->track())) {
			int n = checkTruncation(num, StandardTags::TF_Track);
			if (n != -1)
				m_tagV1->setTrack(n);
			else
				m_tagV1->setTrack(num);
			markTag1Changed();
		}
	}
}

/**
 * Set ID3v1 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void TagLibFile::setGenreV1(const QString& str)
{
	if (makeTagV1Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV1->genre())) {
			m_tagV1->setGenre(tstr);
			markTag1Changed();
		}
		// if the string cannot be converted to a number, set the truncation flag
		checkTruncation(!str.isEmpty() && Genres::getNumber(str) == 0xff ? 1 : 0,
										StandardTags::TF_Genre, 0);
	}
}

/**
 * Check if string needs Unicode encoding.
 *
 * @return true if Unicode needed,
 *         false if Latin-1 sufficient.
 */
static bool needsUnicode(const QString& qstr)
{
	bool result = false;
	uint unicodeSize = qstr.length();
	const QChar* qcarray = qstr.unicode();
	for (uint i = 0; i < unicodeSize; ++i) {
#if QT_VERSION >= 0x040000
		if (qcarray[i].toLatin1() == 0)
#else
		if (qcarray[i].latin1() == 0)
#endif
		{
			result = true;
			break;
		}
	}
	return result;
}

/**
 * Write a Unicode field if the tag is ID3v2 and Latin-1 is not sufficient.
 *
 * @param tag     tag
 * @param qstr    text as QString
 * @param tstr    text as TagLib::String
 * @param frameId ID3v2 frame ID
 *
 * @return true if an ID3v2 Unicode field was written.
 */
bool setId3v2Unicode(TagLib::Tag* tag, const QString& qstr, const TagLib::String& tstr, const char* frameId)
{
	TagLib::ID3v2::Tag* id3v2Tag;
	if (tag && (id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) != 0) {
		// first check if this string needs to be stored as unicode
		if (needsUnicode(qstr)) {
			TagLib::ByteVector id(frameId);
			id3v2Tag->removeFrames(id);
			if (!tstr.isEmpty()) {
				TagLib::ID3v2::Frame* frame;
				if (frameId[0] != 'C') {
					frame = new TagLib::ID3v2::TextIdentificationFrame(id, TagLib::String::UTF16);
				} else {
					frame = new TagLib::ID3v2::CommentsFrame(TagLib::String::UTF16);
				}
				if (!frame) {
					return false;
				}
				frame->setText(tstr);
#ifdef WIN32
				// freed in Windows DLL => must be allocated in the same DLL
				TagLib::ID3v2::Frame* dllAllocatedFrame =
					TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
				if (dllAllocatedFrame) {
					id3v2Tag->addFrame(dllAllocatedFrame);
				}
				delete frame;
#else
				id3v2Tag->addFrame(frame);
#endif
			}
			return true;
		}
	}
	return false;
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setTitleV2(const QString& str)
{
	if (makeTagV2Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV2->title())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "TIT2")) {
				m_tagV2->setTitle(tstr);
			}
			markTag2Changed();
		}
	}
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setArtistV2(const QString& str)
{
	if (makeTagV2Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV2->artist())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "TPE1")) {
				m_tagV2->setArtist(tstr);
			}
			markTag2Changed();
		}
	}
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setAlbumV2(const QString& str)
{
	if (makeTagV2Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV2->album())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "TALB")) {
				m_tagV2->setAlbum(tstr);
			}
			markTag2Changed();
		}
	}
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field.
 */
void TagLibFile::setCommentV2(const QString& str)
{
	if (makeTagV2Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV2->comment())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "COMM")) {
				m_tagV2->setComment(tstr);
			}
			markTag2Changed();
		}
	}
}

/**
 * Set ID3v2 year.
 *
 * @param num number to set, 0 to remove field.
 */
void TagLibFile::setYearV2(int num)
{
	if (makeTagV2Settable() && num >= 0) {
		if (num != static_cast<int>(m_tagV2->year())) {
			m_tagV2->setYear(num);
			markTag2Changed();
		}
	}
}

/**
 * Set ID3v2 track.
 *
 * @param num number to set, 0 to remove field.
 */
void TagLibFile::setTrackNumV2(int num)
{
	if (makeTagV2Settable() && num >= 0) {
		if (num != static_cast<int>(m_tagV2->track())) {
			int numTracks;
			TagLib::ID3v2::TextIdentificationFrame* frame;
			TagLib::ID3v2::Tag* id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2);
			if (id3v2Tag &&
					(numTracks = getTotalNumberOfTracksIfEnabled()) > 0 &&
					num > 0 &&
					(frame = new TagLib::ID3v2::TextIdentificationFrame(
						"TRCK", TagLib::String::Latin1)) != 0) {
				TagLib::String str = TagLib::String::number(num);
				str += '/';
				str += TagLib::String::number(numTracks);
				frame->setText(str);
				id3v2Tag->removeFrames("TRCK");
#ifdef WIN32
				// freed in Windows DLL => must be allocated in the same DLL
				TagLib::ID3v2::Frame* dllAllocatedFrame =
					TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
				if (dllAllocatedFrame) {
					id3v2Tag->addFrame(dllAllocatedFrame);
				}
				delete frame;
#else
				id3v2Tag->addFrame(frame);
#endif
			} else {
				m_tagV2->setTrack(num);
			}
			markTag2Changed();
		}
	}
}

/**
 * Set ID3v2 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void TagLibFile::setGenreV2(const QString& str)
{
	if (makeTagV2Settable() && !str.isNull()) {
		TagLib::String tstr = str.isEmpty() ?
			TagLib::String::null : QSTRING_TO_TSTRING(str);
		if (!(tstr == m_tagV2->genre())) {
			m_tagV2->setGenre(tstr);
			markTag2Changed();
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
bool TagLibFile::isTagInformationRead() const
{
	return m_fileRead;
}

/**
 * Check if file has an ID3v1 tag.
 *
 * @return true if a V1 tag is available.
 * @see isTagInformationRead()
 */
bool TagLibFile::hasTagV1() const
{
	return m_tagV1 && !m_tagV1->isEmpty();
}

/**
 * Check if ID3v1 tags are supported by the format of this file.
 *
 * @return true.
 */
bool TagLibFile::isTagV1Supported() const
{
	TagLib::File* file;
	return (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0 &&
					(dynamic_cast<TagLib::MPEG::File*>(file) != 0 ||
					 dynamic_cast<TagLib::FLAC::File*>(file) != 0
#ifdef MPC_ID3V1
					 || dynamic_cast<TagLib::MPC::File*>(file)  != 0
#endif
						));
}

/**
 * Check if file has an ID3v2 tag.
 *
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool TagLibFile::hasTagV2() const
{
	return m_tagV2 && !m_tagV2->isEmpty();
}

/**
 * Get technical detail information.
 *
 * @return string with detail information,
 *         "" if no information available.
 */
QString TagLibFile::getDetailInfo() const {
	QString str;
	TagLib::AudioProperties* audioProperties;
	if (!m_fileRef.isNull() &&
			(audioProperties = m_fileRef.audioProperties()) != 0) {
		const char* channelModeStr = 0;
		TagLib::MPEG::Properties* mpegProperties;
		TagLib::Vorbis::Properties* oggProperties;
		TagLib::FLAC::Properties* flacProperties;
		TagLib::MPC::Properties* mpcProperties;
		if ((mpegProperties =
				 dynamic_cast<TagLib::MPEG::Properties*>(audioProperties)) != 0) {
			switch (mpegProperties->version()) {
				case TagLib::MPEG::Header::Version1:
					str += "MPEG 1 ";
					break;
				case TagLib::MPEG::Header::Version2:
					str += "MPEG 2 ";
					break;
				case TagLib::MPEG::Header::Version2_5:
					str += "MPEG 2.5 ";
					break;
					//! @todo is there information about VBR.
			}
			int layer = mpegProperties->layer();
			if (layer >= 1 && layer <= 3) {
				str += "Layer ";
				str += QString::number(layer);
				str += ' ';
			}
			switch (mpegProperties->channelMode()) {
				case TagLib::MPEG::Header::Stereo:
					channelModeStr = "Stereo ";
					break;
				case TagLib::MPEG::Header::JointStereo:
					channelModeStr = "Joint Stereo ";
					break;
				case TagLib::MPEG::Header::DualChannel:
					channelModeStr = "Dual ";
					break;
				case TagLib::MPEG::Header::SingleChannel:
					channelModeStr = "Single ";
					break;
			}
		} else if ((oggProperties =
								dynamic_cast<TagLib::Vorbis::Properties*>(audioProperties)) !=
							 0) {
			str += "Ogg Vorbis ";
		} else if ((flacProperties =
								dynamic_cast<TagLib::FLAC::Properties*>(audioProperties)) !=
							 0) {
			str += "FLAC ";
		} else if ((mpcProperties =
								dynamic_cast<TagLib::MPC::Properties*>(audioProperties)) != 0) {
			str += "MPC ";
		}
		int bitrate = audioProperties->bitrate();
		if (bitrate > 0 && bitrate < 999) {
			str += QString::number(bitrate);
			str += " kbps ";
		}
		int sampleRate = audioProperties->sampleRate();
		if (sampleRate > 0) {
			str += QString::number(sampleRate);
			str += " Hz ";
		}
		if (channelModeStr) {
			str += channelModeStr;
		} else {
			int channels = audioProperties->channels();
			if (channels > 0) {
				str += QString::number(channels);
				str += " Channels ";
			}
		}
		int length = audioProperties->length();
		if (length > 0) {
			str += formatTime(length);
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
unsigned TagLibFile::getDuration() const
{
	TagLib::AudioProperties* audioProperties;
	return
		((!m_fileRef.isNull() &&
			(audioProperties = m_fileRef.audioProperties()) != 0)) ?
		audioProperties->length() : 0;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".mp3".
 */
QString TagLibFile::getFileExtension() const
{
	TagLib::FLAC::File f("test.flac");

	TagLib::File* file = m_fileRef.file();
	if (file) {
		if (dynamic_cast<TagLib::MPEG::File*>(file) != 0) {
			return ".mp3";
		} else if (dynamic_cast<TagLib::Vorbis::File*>(file) != 0) {
			return ".ogg";
		} else if (dynamic_cast<TagLib::FLAC::File*>(file) != 0) {
			return ".flac";
		} else if (dynamic_cast<TagLib::MPC::File*>(file) != 0) {
			return ".mpc";
		}
	}
	return ".mp3";
}

/**
 * Get the format of a tag.
 *
 * @param tag tag, 0 if no tag available
 *
 * @return string describing format of tag,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
static QString getTagFormat(const TagLib::Tag* tag)
{
	if (tag && !tag->isEmpty()) {
		const TagLib::ID3v1::Tag* id3v1Tag;
		const TagLib::ID3v2::Tag* id3v2Tag;
		const TagLib::Ogg::XiphComment* oggTag;
		const TagLib::APE::Tag* apeTag;
		if ((id3v1Tag = dynamic_cast<const TagLib::ID3v1::Tag*>(tag)) != 0) {
			return "ID3v1.1";
		} else if ((id3v2Tag = dynamic_cast<const TagLib::ID3v2::Tag*>(tag)) != 0) {
			TagLib::ID3v2::Header* header = id3v2Tag->header();
			if (header) {
				uint majorVersion = header->majorVersion();
				uint revisionNumber = header->revisionNumber();
#if (TAGLIB_VERSION <= 0x010400)
				// A wrong majorVersion is returned if a new ID3v2.4.0 tag is created.
				if (majorVersion == 0 && revisionNumber == 0) {
					majorVersion = 4;
				}
#endif
				return QString("ID3v2.%1.%2").
					arg(majorVersion).arg(revisionNumber);
			} else {
				return "ID3v2";
			}
		} else if ((oggTag = dynamic_cast<const TagLib::Ogg::XiphComment*>(tag)) != 0) {
			return "Vorbis";
		} else if ((apeTag = dynamic_cast<const TagLib::APE::Tag*>(tag)) != 0) {
			return "APE";
		}
	}
	return QString::null;
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TagLibFile::getTagFormatV1() const
{
	return getTagFormat(m_tagV1);
}

/**
 * Get the format of tag 2.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TagLibFile::getTagFormatV2() const
{
	return getTagFormat(m_tagV2);
}


/** Types and descriptions for id3lib frame IDs */
static const struct TypeStrOfId {
	Frame::Type type;
	const char* str;
	bool supported;
} typeStrOfId[] = {
	{ Frame::FT_Other,          I18N_NOOP("AENC - Audio encryption"), false },
	{ Frame::FT_Other,          I18N_NOOP("APIC - Attached picture"), true },
	{ Frame::FT_Other,          I18N_NOOP("ASPI - Audio seek point index"), false },
	{ Frame::FT_Comment,        I18N_NOOP("COMM - Comments"), true },
	{ Frame::FT_Other,          I18N_NOOP("COMR - Commercial"), false },
	{ Frame::FT_Other,          I18N_NOOP("ENCR - Encryption method registration"), false },
	{ Frame::FT_Other,          I18N_NOOP("EQU2 - Equalisation (2)"), false },
	{ Frame::FT_Other,          I18N_NOOP("ETCO - Event timing codes"), false },
	{ Frame::FT_Other,          I18N_NOOP("GEOB - General encapsulated object"),
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		true
#else
		false
#endif
	},
	{ Frame::FT_Other,          I18N_NOOP("GRID - Group identification registration"), false },
	{ Frame::FT_Other,          I18N_NOOP("LINK - Linked information"), false },
	{ Frame::FT_Other,          I18N_NOOP("MCDI - Music CD identifier"), false },
	{ Frame::FT_Other,          I18N_NOOP("MLLT - MPEG location lookup table"), false },
	{ Frame::FT_Other,          I18N_NOOP("OWNE - Ownership frame"), false },
	{ Frame::FT_Other,          I18N_NOOP("PRIV - Private frame"), false },
	{ Frame::FT_Other,          I18N_NOOP("PCNT - Play counter"), false },
	{ Frame::FT_Other,          I18N_NOOP("POPM - Popularimeter"), false },
	{ Frame::FT_Other,          I18N_NOOP("POSS - Position synchronisation frame"), false },
	{ Frame::FT_Other,          I18N_NOOP("RBUF - Recommended buffer size"), false },
	{ Frame::FT_Other,          I18N_NOOP("RVA2 - Relative volume adjustment (2)"), false },
	{ Frame::FT_Other,          I18N_NOOP("RVRB - Reverb"), false },
	{ Frame::FT_Other,          I18N_NOOP("SEEK - Seek frame"), false },
	{ Frame::FT_Other,          I18N_NOOP("SIGN - Signature frame"), false },
	{ Frame::FT_Other,          I18N_NOOP("SYLT - Synchronized lyric/text"), false },
	{ Frame::FT_Other,          I18N_NOOP("SYTC - Synchronized tempo codes"), false },
	{ Frame::FT_Album,          I18N_NOOP("TALB - Album/Movie/Show title"), true },
	{ Frame::FT_Bpm,            I18N_NOOP("TBPM - BPM (beats per minute)"), true },
	{ Frame::FT_Composer,       I18N_NOOP("TCOM - Composer"), true },
	{ Frame::FT_Genre,          I18N_NOOP("TCON - Content type"), true },
	{ Frame::FT_Copyright,      I18N_NOOP("TCOP - Copyright message"), true },
	{ Frame::FT_Other,          I18N_NOOP("TDEN - Encoding time"), true },
	{ Frame::FT_Other,          I18N_NOOP("TDLY - Playlist delay"), true },
	{ Frame::FT_OriginalDate,   I18N_NOOP("TDOR - Original release time"), true },
	{ Frame::FT_Date,           I18N_NOOP("TDRC - Recording time"), true },
	{ Frame::FT_Other,          I18N_NOOP("TDRL - Release time"), true },
	{ Frame::FT_Other,          I18N_NOOP("TDTG - Tagging time"), true },
	{ Frame::FT_EncodedBy,      I18N_NOOP("TENC - Encoded by"), true },
	{ Frame::FT_Lyricist,       I18N_NOOP("TEXT - Lyricist/Text writer"), true },
	{ Frame::FT_Other,          I18N_NOOP("TFLT - File type"), true },
	{ Frame::FT_Other,          I18N_NOOP("TIPL - Involved people list"), true },
	{ Frame::FT_Other,          I18N_NOOP("TIT1 - Content group description"), true },
	{ Frame::FT_Title,          I18N_NOOP("TIT2 - Title/songname/content description"), true },
	{ Frame::FT_Subtitle,       I18N_NOOP("TIT3 - Subtitle/Description refinement"), true },
	{ Frame::FT_Other,          I18N_NOOP("TKEY - Initial key"), true },
	{ Frame::FT_Language,       I18N_NOOP("TLAN - Language(s)"), true },
	{ Frame::FT_Other,          I18N_NOOP("TLEN - Length"), true },
	{ Frame::FT_Other,          I18N_NOOP("TMCL - Musician credits list"), true },
	{ Frame::FT_Other,          I18N_NOOP("TMED - Media type"), true },
	{ Frame::FT_Other,          I18N_NOOP("TMOO - Mood"), true },
	{ Frame::FT_OriginalAlbum,  I18N_NOOP("TOAL - Original album/movie/show title"), true },
	{ Frame::FT_Other,          I18N_NOOP("TOFN - Original filename"), true },
	{ Frame::FT_Author,         I18N_NOOP("TOLY - Original lyricist(s)/text writer(s)"), true },
	{ Frame::FT_OriginalArtist, I18N_NOOP("TOPE - Original artist(s)/performer(s)"), true },
	{ Frame::FT_Other,          I18N_NOOP("TOWN - File owner/licensee"), true },
	{ Frame::FT_Artist,         I18N_NOOP("TPE1 - Lead performer(s)/Soloist(s)"), true },
	{ Frame::FT_Performer,      I18N_NOOP("TPE2 - Band/orchestra/accompaniment"), true },
	{ Frame::FT_Conductor,      I18N_NOOP("TPE3 - Conductor/performer refinement"), true },
	{ Frame::FT_Arranger,       I18N_NOOP("TPE4 - Interpreted, remixed, or otherwise modified by"), true },
	{ Frame::FT_Disc,           I18N_NOOP("TPOS - Part of a set"), true },
	{ Frame::FT_Other,          I18N_NOOP("TPRO - Produced notice"), true },
	{ Frame::FT_Publisher,      I18N_NOOP("TPUB - Publisher"), true },
	{ Frame::FT_Track,          I18N_NOOP("TRCK - Track number/Position in set"), true },
	{ Frame::FT_Other,          I18N_NOOP("TRSN - Internet radio station name"), true },
	{ Frame::FT_Other,          I18N_NOOP("TRSO - Internet radio station owner"), true },
	{ Frame::FT_Other,          I18N_NOOP("TSOA - Album sort order"), true },
	{ Frame::FT_Other,          I18N_NOOP("TSOP - Performer sort order"), true },
	{ Frame::FT_Other,          I18N_NOOP("TSOT - Title sort order"), true },
	{ Frame::FT_Isrc,           I18N_NOOP("TSRC - ISRC (international standard recording code)"), true },
	{ Frame::FT_Other,          I18N_NOOP("TSSE - Software/Hardware and settings used for encoding"), true },
	{ Frame::FT_Part,           I18N_NOOP("TSST - Set subtitle"), true },
	{ Frame::FT_Other,          I18N_NOOP("TXXX - User defined text information"), true },
	{ Frame::FT_Other,          I18N_NOOP("UFID - Unique file identifier"), true },
	{ Frame::FT_Other,          I18N_NOOP("USER - Terms of use"), false },
	{ Frame::FT_Other,          I18N_NOOP("USLT - Unsynchronized lyric/text transcription"), true },
	{ Frame::FT_Other,          I18N_NOOP("WCOM - Commercial information"), true },
	{ Frame::FT_Other,          I18N_NOOP("WCOP - Copyright/Legal information"), true },
	{ Frame::FT_Other,          I18N_NOOP("WOAF - Official audio file webpage"), true },
	{ Frame::FT_Website,        I18N_NOOP("WOAR - Official artist/performer webpage"), true },
	{ Frame::FT_Other,          I18N_NOOP("WOAS - Official audio source webpage"), true },
	{ Frame::FT_Other,          I18N_NOOP("WORS - Official internet radio station homepage"), true },
	{ Frame::FT_Other,          I18N_NOOP("WPAY - Payment"), true },
	{ Frame::FT_Other,          I18N_NOOP("WPUB - Official publisher webpage"), true },
	{ Frame::FT_Other,          I18N_NOOP("WXXX - User defined URL link"), true }
};

/**
 * Get type and description of frame.
 *
 * @param id   ID of frame
 * @param type the type is returned here
 * @param str  the description is returned here
 */
static void getTypeStringForFrameId(TagLib::ByteVector id, Frame::Type& type,
																		const char*& str)
{
	static TagLib::Map<TagLib::ByteVector, unsigned> idIndexMap;
	if (idIndexMap.isEmpty()) {
		for (unsigned i = 0;
				 i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]);
				 ++i) {
			idIndexMap.insert(TagLib::ByteVector(typeStrOfId[i].str, 4), i);
		}
	}
	if (idIndexMap.contains(id)) {
		const TypeStrOfId& ts = typeStrOfId[idIndexMap[id]];
		type = ts.type;
		str = ts.str;
	} else {
		type = Frame::FT_UnknownFrame;
		str = "????";
	}
}

/**
 * Get string description starting with 4 bytes ID.
 *
 * @param type type of frame
 *
 * @return string.
 */
static const char* getStringForType(Frame::Type type)
{
	if (type != Frame::FT_Other) {
		for (unsigned i = 0;
				 i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]);
				 ++i) {
			const TypeStrOfId& ts = typeStrOfId[i];
			if (ts.type == type) {
				return ts.str;
			}
		}
	}
	return "????";
}

/**
 * Get the fields from a text identification frame.
 *
 * @param tFrame text identification frame
 * @param fields the fields are appended to this list
 * @param type   frame type
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromTextFrame(
	const TagLib::ID3v2::TextIdentificationFrame* tFrame,
	Frame::FieldList& fields, Frame::Type type)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_TextEnc;
	field.m_value = tFrame->textEncoding();
	fields.push_back(field);

	const TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame;
	if (tFrame &&
			(txxxFrame =
			 dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(tFrame))
			!= 0) {
		field.m_id = Frame::Field::ID_Description;
		field.m_value = TStringToQString(txxxFrame->description());
		fields.push_back(field);

		TagLib::StringList slText = tFrame->fieldList();
		text = slText.size() > 1 ? TStringToQString(slText[1]) : "";
	} else {
		text = TStringToQString(tFrame->toString());
	}
	field.m_id = Frame::Field::ID_Text;
	if (type == Frame::FT_Genre) {
		text = Genres::getNameString(text);
	}
	field.m_value = text;
	fields.push_back(field);

	return text;
}

/**
 * Get the fields from an attached picture frame.
 *
 * @param apicFrame attached picture frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromApicFrame(
	const TagLib::ID3v2::AttachedPictureFrame* apicFrame,
	Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_TextEnc;
	field.m_value = apicFrame->textEncoding();
	fields.push_back(field);

	// for compatibility with ID3v2.3 id3lib
	field.m_id = Frame::Field::ID_ImageFormat;
	field.m_value = QString("");
	fields.push_back(field);

	field.m_id = Frame::Field::ID_MimeType;
	field.m_value = TStringToQString(apicFrame->mimeType());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_PictureType;
	field.m_value = apicFrame->type();
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Description;
	text = TStringToQString(apicFrame->description());
	field.m_value = text;
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Data;
	TagLib::ByteVector pic = apicFrame->picture();
	QByteArray ba;
	QCM_duplicate(ba, pic.data(), pic.size());
	field.m_value = ba;
	fields.push_back(field);

	return text;
}

/**
 * Get the fields from a comments frame.
 *
 * @param commFrame comments frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromCommFrame(
	const TagLib::ID3v2::CommentsFrame* commFrame, Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_TextEnc;
	field.m_value = commFrame->textEncoding();
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Language;
	TagLib::ByteVector bvLang = commFrame->language();
	field.m_value = QString(QCM_QCString(bvLang.data(), bvLang.size() + 1));
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Description;
	field.m_value = TStringToQString(commFrame->description());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Text;
	text = TStringToQString(commFrame->toString());
	field.m_value = text;
	fields.push_back(field);

	return text;
}

/**
 * Get the fields from a unique file identifier frame.
 *
 * @param ufidFrame unique file identifier frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUfidFrame(
	const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame,
	Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_Owner;
	field.m_value = TStringToQString(ufidFrame->owner());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Id;
	TagLib::ByteVector id = ufidFrame->identifier();
	QByteArray ba;
	QCM_duplicate(ba, id.data(), id.size());
	field.m_value = ba;
	fields.push_back(field);

	return text;
}

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
/**
 * Get the fields from a general encapsulated object frame.
 *
 * @param geobFrame general encapsulated object frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromGeobFrame(
	const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame,
	Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_TextEnc;
	field.m_value = geobFrame->textEncoding();
	fields.push_back(field);

	field.m_id = Frame::Field::ID_MimeType;
	field.m_value = TStringToQString(geobFrame->mimeType());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Filename;
	field.m_value = TStringToQString(geobFrame->fileName());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Description;
	text = TStringToQString(geobFrame->description());
	field.m_value = text;
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Data;
	TagLib::ByteVector obj = geobFrame->object();
	QByteArray ba;
	QCM_duplicate(ba, obj.data(), obj.size());
	field.m_value = ba;
	fields.push_back(field);

	return text;
}
#endif // TAGLIB_SUPPORTS_GEOB_FRAMES

/**
 * Get the fields from a URL link frame.
 *
 * @param wFrame URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUrlFrame(
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
	const TagLib::ID3v2::UrlLinkFrame* wFrame
#else
	const UrlLinkFrame* wFrame
#endif
	, Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_Url;
	text = TStringToQString(wFrame->url());
	field.m_value = text;
	fields.push_back(field);

	return text;
}

/**
 * Get the fields from a user URL link frame.
 *
 * @param wxxxFrame user URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUserUrlFrame(
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
	const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame
#else
	const UserUrlLinkFrame* wxxxFrame
#endif
	, Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_TextEnc;
	field.m_value = wxxxFrame->textEncoding();
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Description;
	field.m_value = TStringToQString(wxxxFrame->description());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Url;
	text = TStringToQString(wxxxFrame->url());
	field.m_value = text;
	fields.push_back(field);

	return text;
}

/**
 * Get the fields from an unsynchronized lyrics frame.
 * This is copy-pasted from editCommFrame().
 *
 * @param usltFrame unsynchronized frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUsltFrame(
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
	const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame
#else
	const UnsynchronizedLyricsFrame* usltFrame
#endif
	, Frame::FieldList& fields)
{
	QString text;
	Frame::Field field;
	field.m_id = Frame::Field::ID_TextEnc;
	field.m_value = usltFrame->textEncoding();
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Language;
	TagLib::ByteVector bvLang = usltFrame->language();
	field.m_value = QString(QCM_QCString(bvLang.data(), bvLang.size() + 1));
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Description;
	field.m_value = TStringToQString(usltFrame->description());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Text;
	text = TStringToQString(usltFrame->toString());
	field.m_value = text;
	fields.push_back(field);

	return text;
}

/**
 * Get the fields from an unknown frame.
 *
 * @param unknownFrame unknown frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUnknownFrame(
	const TagLib::ID3v2::Frame* unknownFrame, Frame::FieldList& fields)
{
	Frame::Field field;
	field.m_id = Frame::Field::ID_Data;
	TagLib::ByteVector dat = unknownFrame->render();
	QByteArray ba;
	QCM_duplicate(ba, dat.data(), dat.size());
	field.m_value = ba;
	fields.push_back(field);
	return QString();
}

/**
 * Get the fields from an ID3v2 tag.
 *
 * @param frame  frame
 * @param fields the fields are appended to this list
 * @param type   frame type
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromId3Frame(const TagLib::ID3v2::Frame* frame,
																		 Frame::FieldList& fields, Frame::Type type)
{
	if (frame) {
		const TagLib::ID3v2::TextIdentificationFrame* tFrame;
		const TagLib::ID3v2::AttachedPictureFrame* apicFrame;
		const TagLib::ID3v2::CommentsFrame* commFrame;
		const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame;
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
#endif
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
		const TagLib::ID3v2::UrlLinkFrame* wFrame;
#else
		const UserUrlLinkFrame* wxxxFrame;
		const UrlLinkFrame* wFrame;
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
		const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
#else
		const UnsynchronizedLyricsFrame* usltFrame;
#endif
		if ((tFrame =
				 dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame*>(frame)) !=
				0) {
			return getFieldsFromTextFrame(tFrame, fields, type);
		} else if ((apicFrame =
								dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(frame))
							 != 0) {
			return getFieldsFromApicFrame(apicFrame, fields);
		} else if ((commFrame = dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(
									frame)) != 0) {
			return getFieldsFromCommFrame(commFrame, fields);
		} else if ((ufidFrame =
								dynamic_cast<const TagLib::ID3v2::UniqueFileIdentifierFrame*>(
									frame)) != 0) {
			return getFieldsFromUfidFrame(ufidFrame, fields);
		}
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		else if ((geobFrame =
							dynamic_cast<const TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
								frame)) != 0) {
			return getFieldsFromGeobFrame(geobFrame, fields);
		}
#endif
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		else if ((wxxxFrame = dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame*>(
								frame)) != 0) {
			return getFieldsFromUserUrlFrame(wxxxFrame, fields);
		} else if ((wFrame = dynamic_cast<const TagLib::ID3v2::UrlLinkFrame*>(
									frame)) != 0) {
			return getFieldsFromUrlFrame(wFrame, fields);
		}
#else
		else if ((wxxxFrame = dynamic_cast<const UserUrlLinkFrame*>(frame)) != 0) {
			return getFieldsFromUserUrlFrame(wxxxFrame, fields);
		} else if ((wFrame = dynamic_cast<const UrlLinkFrame*>(frame)) != 0) {
			return getFieldsFromUrlFrame(wFrame, fields);
		}
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
		else if ((usltFrame = dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
								frame)) != 0) {
			return getFieldsFromUsltFrame(usltFrame, fields);
		}
#else
		else if ((usltFrame = dynamic_cast<const UnsynchronizedLyricsFrame*>(
								frame)) != 0) {
			return getFieldsFromUsltFrame(usltFrame, fields);
		}
#endif
		else {
			TagLib::ByteVector id = frame->frameID();
#ifndef TAGLIB_SUPPORTS_URLLINK_FRAMES
			if (id.startsWith("WXXX")) {
				UserUrlLinkFrame userUrlLinkFrame(frame->render());
				return getFieldsFromUserUrlFrame(&userUrlLinkFrame, fields);
			} else if (id.startsWith("W")) {
				UrlLinkFrame urlLinkFrame(frame->render());
				return getFieldsFromUrlFrame(&urlLinkFrame, fields);
			} else
#endif
#ifndef TAGLIB_SUPPORTS_USLT_FRAMES
				if (id.startsWith("USLT")) {
					UnsynchronizedLyricsFrame usltFrame(frame->render());
					return getFieldsFromUsltFrame(&usltFrame, fields);
				} else
#endif
					return getFieldsFromUnknownFrame(frame, fields);
		}
	}
	return QString();
}

/**
 * Convert a string to a language code byte vector.
 *
 * @param str string containing language code.
 *
 * @return 3 byte vector with language code.
 */
static TagLib::ByteVector languageCodeByteVector(QString str)
{
	uint len = str.length();
	if (len > 3) {
		str.truncate(3);
	} else if (len < 3) {
		for (uint i = len; i < 3; ++i) {
			str += ' ';
		}
	}
	return TagLib::ByteVector(str.QCM_latin1(), str.length());
}

/**
 * The follwoing template functions are used to uniformly set fields
 * in the different ID3v2 frames.
 */
template <class T>
void setTextEncoding(T*, TagLib::String::Type) {}

template <>
void setTextEncoding(TagLib::ID3v2::TextIdentificationFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::AttachedPictureFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::CommentsFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
template <>
void setTextEncoding(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}
#endif

#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
template <>
void setTextEncoding(TagLib::ID3v2::UserUrlLinkFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}
#else
template <>
void setTextEncoding(UserUrlLinkFrame* f, TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}
#endif

#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
template <>
void setTextEncoding(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}
#else
template <>
void setTextEncoding(UnsynchronizedLyricsFrame* f, TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}
#endif


template <class T>
void setDescription(T*, const Frame::Field&) {}

template <>
void setDescription(TagLib::ID3v2::UserTextIdentificationFrame* f,
										const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::AttachedPictureFrame* f,
										const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
template <>
void setDescription(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
										const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
template <>
void setDescription(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#else
template <>
void setDescription(UserUrlLinkFrame* f, const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
template <>
void setDescription(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
										const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#else
template <>
void setDescription(UnsynchronizedLyricsFrame* f, const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

template <class T>
void setMimeType(T*, const Frame::Field&) {}

template <>
void setMimeType(TagLib::ID3v2::AttachedPictureFrame* f,
								 const Frame::Field& fld)
{
	f->setMimeType(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
template <>
void setMimeType(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
								 const Frame::Field& fld)
{
	f->setMimeType(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

template <class T>
void setPictureType(T*, const Frame::Field&) {}

template <>
void setPictureType(TagLib::ID3v2::AttachedPictureFrame* f,
										const Frame::Field& fld)
{
	f->setType(
		static_cast<TagLib::ID3v2::AttachedPictureFrame::Type>(
			fld.m_value.toInt()));
}

template <class T>
void setData(T*, const Frame::Field&) {}

template <>
void setData(TagLib::ID3v2::Frame* f, const Frame::Field& fld)
{
	QByteArray ba(fld.m_value.toByteArray());
	f->setData(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::AttachedPictureFrame* f, const Frame::Field& fld)
{
	QByteArray ba(fld.m_value.toByteArray());
	f->setPicture(TagLib::ByteVector(ba.data(), ba.size()));
}

template <class T>
void setLanguage(T*, const Frame::Field&) {}

template <>
void setLanguage(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
	f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
template <>
void setLanguage(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
								 const Frame::Field& fld)
{
	f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}
#else
template <>
void setLanguage(UnsynchronizedLyricsFrame* f, const Frame::Field& fld)
{
	f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}
#endif

template <class T>
void setOwner(T*, const Frame::Field&) {}

template <>
void setOwner(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
							const Frame::Field& fld)
{
	f->setOwner(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <class T>
void setIdentifier(T*, const Frame::Field&) {}

template <>
void setIdentifier(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
									 const Frame::Field& fld)
{
	QByteArray ba(fld.m_value.toByteArray());
	f->setIdentifier(TagLib::ByteVector(ba.data(), ba.size()));
}

template <class T>
void setFilename(T*, const Frame::Field&) {}

#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
template <>
void setFilename(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
								 const Frame::Field& fld)
{
	f->setFileName(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

template <class T>
void setUrl(T*, const Frame::Field&) {}

#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
template <>
void setUrl(TagLib::ID3v2::UrlLinkFrame* f, const Frame::Field& fld)
{
	f->setUrl(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setUrl(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
	f->setUrl(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#else
template <>
void setUrl(UrlLinkFrame* f, const Frame::Field& fld)
{
	f->setUrl(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setUrl(UserUrlLinkFrame* f, const Frame::Field& fld)
{
	f->setUrl(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

template <class T>
void setValue(T* f, const TagLib::String& text)
{
	f->setText(text);
}

template <>
void setValue(TagLib::ID3v2::AttachedPictureFrame* f, const TagLib::String& text)
{
	f->setDescription(text);
}

/**
 * Set the fields in a TagLib ID3v2 frame.
 *
 * @param tFrame TagLib frame to set
 * @param frame  frame with field values
 */
template <class T>
void setTagLibFrame(const TagLibFile* self, T* tFrame, const Frame& frame)
{
	// If value is changed or field list is empty,
	// set from value, else from FieldList.
	if (frame.isValueChanged() || frame.getFieldList().empty()) {
		QString text(frame.getValue());
		if (frame.getType() == Frame::FT_Genre) {
			text = Genres::getNumberString(text, false);
		} else if (frame.getType() == Frame::FT_Track) {
			self->addTotalNumberOfTracksIfEnabled(text);
		}
		setValue(tFrame, QSTRING_TO_TSTRING(text));
		setTextEncoding(tFrame, needsUnicode(text) ?
										TagLib::String::UTF16 : TagLib::String::Latin1);
	} else {
		for (Frame::FieldList::const_iterator fldIt = frame.getFieldList().begin();
				 fldIt != frame.getFieldList().end();
				 ++fldIt) {
			const Frame::Field& fld = *fldIt;
			switch (fld.m_id) {
				case Frame::Field::ID_Text:
				{
					QString value(fld.m_value.toString());
					if (frame.getType() == Frame::FT_Genre) {
						value = Genres::getNumberString(value, false);
					} else if (frame.getType() == Frame::FT_Track) {
						self->addTotalNumberOfTracksIfEnabled(value);
					}
					tFrame->setText(QSTRING_TO_TSTRING(value));
					break;
				}
				case Frame::Field::ID_TextEnc:
					setTextEncoding(tFrame, static_cast<TagLib::String::Type>(
														fld.m_value.toInt()));
					break;
				case Frame::Field::ID_Description:
					setDescription(tFrame, fld);
					break;
				case Frame::Field::ID_MimeType:
					setMimeType(tFrame, fld);
					break;
				case Frame::Field::ID_PictureType:
					setPictureType(tFrame, fld);
					break;
				case Frame::Field::ID_Data:
					setData(tFrame, fld);
					break;
				case Frame::Field::ID_Language:
					setLanguage(tFrame, fld);
					break;
				case Frame::Field::ID_Owner:
					setOwner(tFrame, fld);
					break;
				case Frame::Field::ID_Id:
					setIdentifier(tFrame, fld);
					break;
				case Frame::Field::ID_Filename:
					setFilename(tFrame, fld);
					break;
				case Frame::Field::ID_Url:
					setUrl(tFrame, fld);
					break;
			}
		}
	}
}

/**
 * Modify an ID3v2 frame.
 *
 * @param id3Frame original ID3v2 frame
 * @param frame    frame with fields to set in new frame
 */
void TagLibFile::setId3v2Frame(
	TagLib::ID3v2::Frame* id3Frame, const Frame& frame) const
{
	if (id3Frame) {
		TagLib::ID3v2::TextIdentificationFrame* tFrame;
		TagLib::ID3v2::AttachedPictureFrame* apicFrame;
		TagLib::ID3v2::CommentsFrame* commFrame;
		TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame;
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
#endif
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
		TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
		TagLib::ID3v2::UrlLinkFrame* wFrame;
#else
		UserUrlLinkFrame* wxxxFrame;
		UrlLinkFrame* wFrame;
#endif
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
		TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
#else
		UnsynchronizedLyricsFrame* usltFrame;
#endif
		if ((tFrame =
				 dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(id3Frame))
				!= 0) {
			TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame =
				dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(id3Frame);
			if (txxxFrame) {
				setTagLibFrame(this, txxxFrame, frame);
			} else {
				setTagLibFrame(this, tFrame, frame);
			}
		} else if ((apicFrame =
								dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame))
							 != 0) {
			setTagLibFrame(this, apicFrame, frame);
		} else if ((commFrame = dynamic_cast<TagLib::ID3v2::CommentsFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, commFrame, frame);
		} else if ((ufidFrame =
								dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, ufidFrame, frame);
		}
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
		else if ((geobFrame =
							dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
								id3Frame)) != 0) {
			setTagLibFrame(this, geobFrame, frame);
		}
#endif
		else if ((wxxxFrame = dynamic_cast<
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
							TagLib::ID3v2::UserUrlLinkFrame*
#else
							UserUrlLinkFrame*
#endif
							>(
								id3Frame)) != 0) {
			setTagLibFrame(this, wxxxFrame, frame);
		}
		else if ((wFrame = dynamic_cast<
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
							TagLib::ID3v2::UrlLinkFrame*
#else
							UrlLinkFrame*
#endif
							>(
								id3Frame)) != 0) {
			setTagLibFrame(this, wFrame, frame);
		}
		else if ((usltFrame =
							dynamic_cast<
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
							TagLib::ID3v2::UnsynchronizedLyricsFrame*
#else
							UnsynchronizedLyricsFrame*
#endif
							>(
								id3Frame)) != 0) {
			setTagLibFrame(this, usltFrame, frame);
		}
		else {
			TagLib::ByteVector id(id3Frame->frameID());
			// create temporary objects for frames not known by TagLib,
			// an UnknownFrame copy will be created by the edit method.
#ifndef TAGLIB_SUPPORTS_URLLINK_FRAMES
			if (id.startsWith("WXXX")) {
				UserUrlLinkFrame userUrlLinkFrame(id3Frame->render());
				setTagLibFrame(this, &userUrlLinkFrame, frame);
				id3Frame->setData(userUrlLinkFrame.render());
			} else if (id.startsWith("W")) {
				UrlLinkFrame urlLinkFrame(id3Frame->render());
				setTagLibFrame(this, &urlLinkFrame, frame);
				id3Frame->setData(urlLinkFrame.render());
			} else
#endif
#ifndef TAGLIB_SUPPORTS_USLT_FRAMES
			if (id.startsWith("USLT")) {
				UnsynchronizedLyricsFrame usltFrame(id3Frame->render());
				setTagLibFrame(this, &usltFrame, frame);
				id3Frame->setData(usltFrame.render());
			} else
#endif
			{
				setTagLibFrame(this, id3Frame, frame);
			}
		}
	}
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool TagLibFile::setFrameV2(const Frame& frame)
{
	// If the frame has an index, change that specific frame
	int index = frame.getIndex();
	if (index != -1 && m_tagV2) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (index < static_cast<int>(frameList.size())) {
				// This is a hack. The frameList should not be modified directly.
				// However when removing the old frame and adding a new frame,
				// the indices of all frames get invalid.
				setId3v2Frame(frameList[index], frame);
				return true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			TagLib::String key = QSTRING_TO_TSTRING(frame.getName().remove(' ').QCM_toUpper());
			TagLib::String value = QSTRING_TO_TSTRING(frame.getValue());
#ifdef TAGLIB_XIPHCOMMENT_REMOVEFIELD_CRASHES
			oggTag->addField(key, value, true);
#else
			// This will crash because TagLib uses an invalidated iterator
			// after calling erase(). I hope this will be fixed in the next
			// version. Until then, remove all fields with that key.
			oggTag->removeField(key, oldValue);
			oggTag->addField(key, value, false);
#endif
			if (frame.getType() == Frame::FT_Track) {
				int numTracks = getTotalNumberOfTracksIfEnabled();
				if (numTracks > 0) {
					oggTag->addField("TRACKTOTAL", TagLib::String::number(numTracks), true);
				}
			}
			return true;
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			apeTag->addValue(QSTRING_TO_TSTRING(frame.getName(true)),
											 QSTRING_TO_TSTRING(frame.getValue()));
			return true;
		}
	}

	// Try the superclass method
	return TaggedFile::setFrameV2(frame);
}

/**
 * Get internal name of an APE frame.
 *
 * @param frame frame
 *
 * @return APE key.
 */
static QString getApeName(const Frame& frame)
{
	switch (frame.getType()) {
		case Frame::FT_Date:
			return "YEAR";
		case Frame::FT_Track:
			return "TRACK";
		default:
			return frame.getName().QCM_toUpper();
	}
}

/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool TagLibFile::addFrameV2(Frame& frame)
{
	// Add a new frame.
	if (makeTagV2Settable()) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			QString name = frame.getType() != Frame::FT_Other ?
				QString(getStringForType(frame.getType())) :
				frame.getName();
			QString frameId = name;
			frameId.truncate(4);
			TagLib::ID3v2::Frame* id3Frame = 0;
			if (frameId.startsWith("T")) {
				if (frameId == "TXXX") {
					id3Frame = new TagLib::ID3v2::UserTextIdentificationFrame;
				} else {
					id3Frame = new TagLib::ID3v2::TextIdentificationFrame(
						TagLib::ByteVector(frameId.QCM_latin1(), frameId.length()),
						TagLib::String::Latin1);
					id3Frame->setText(""); // is necessary for createFrame() to work
				}
			} else if (frameId == "COMM") {
				id3Frame = new TagLib::ID3v2::CommentsFrame;
			} else if (frameId == "APIC") {
				id3Frame = new TagLib::ID3v2::AttachedPictureFrame;
			} else if (frameId == "UFID") {
				// the bytevector must not be empty
				id3Frame = new TagLib::ID3v2::UniqueFileIdentifierFrame(
					TagLib::String(), TagLib::ByteVector(" "));
			}
#ifdef TAGLIB_SUPPORTS_GEOB_FRAMES
			else if (frameId == "GEOB") {
				id3Frame = new TagLib::ID3v2::GeneralEncapsulatedObjectFrame;
			}
#endif
			else if (frameId.startsWith("W")) {
#ifdef TAGLIB_SUPPORTS_URLLINK_FRAMES
				if (frameId == "WXXX") {
					id3Frame = new TagLib::ID3v2::UserUrlLinkFrame;
				} else {
					id3Frame = new TagLib::ID3v2::UrlLinkFrame(
						TagLib::ByteVector(frameId.QCM_latin1(), frameId.length()));
					frame->setText("http://"); // is necessary for createFrame() to work
				}
#else
				if (frameId == "WXXX") {
					id3Frame = new UserUrlLinkFrame;
				} else {
					id3Frame = new UrlLinkFrame(
						TagLib::ByteVector(frameId.QCM_latin1(), frameId.length()));
					id3Frame->setText("http://"); // is necessary for createFrame() to work
				}
#endif
			} else if (frameId == "USLT") {
#ifdef TAGLIB_SUPPORTS_USLT_FRAMES
				id3Frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame;
#else
				id3Frame = new UnsynchronizedLyricsFrame;
#endif
			}
			if (id3Frame) {
				if (!frame.fieldList().empty()) {
					frame.setValueFromFieldList();
					setId3v2Frame(id3Frame, frame);
				}
#ifdef WIN32
				// freed in Windows DLL => must be allocated in the same DLL
				TagLib::ID3v2::Frame* dllAllocatedFrame =
					TagLib::ID3v2::FrameFactory::instance()->createFrame(id3Frame->render());
				if (dllAllocatedFrame) {
					id3v2Tag->addFrame(dllAllocatedFrame);
				}
#else
				id3v2Tag->addFrame(id3Frame);
#endif
				frame.setInternalName(name);
				frame.setIndex(id3v2Tag->frameList().size() - 1);
				if (frame.fieldList().empty()) {
					// add field list to frame
					getFieldsFromId3Frame(id3Frame, frame.fieldList(), frame.getType());
					frame.setFieldListFromValue();
				}
#ifdef WIN32
				delete id3Frame;
#endif
				return true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			QString name(frame.getName().remove(' ').QCM_toUpper());
			TagLib::String tname = QSTRING_TO_TSTRING(name);
			oggTag->addField(tname, QSTRING_TO_TSTRING(frame.getValue()));
			frame.setInternalName(name);

			const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
			int index = 0;
			bool found = false;
			for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
					 it != fieldListMap.end();
					 ++it) {
				if ((*it).first == tname) {
					index += (*it).second.size() - 1;
					found = true;
					break;
				}
				++index;
			}
			frame.setIndex(found ? index : -1);
			return true;
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			QString name(getApeName(frame));
			TagLib::String tname = QSTRING_TO_TSTRING(name);
			TagLib::String tvalue = QSTRING_TO_TSTRING(frame.getValue());
			if (tvalue.isEmpty()) {
				tvalue = " "; // empty values are not added by TagLib
			}
			apeTag->addValue(tname, tvalue, true);
			frame.setInternalName(name);

			const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
			int index = 0;
			bool found = false;
			for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
					 it != itemListMap.end();
					 ++it) {
				if ((*it).first == tname) {
					found = true;
					break;
				}
				++index;
			}
			frame.setIndex(found ? index : -1);
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
bool TagLibFile::deleteFrameV2(const Frame& frame)
{
	// If the frame has an index, delete that specific frame
	int index = frame.getIndex();
	if (index != -1 && m_tagV2) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (index < static_cast<int>(frameList.size())) {
				id3v2Tag->removeFrame(frameList[index]);
				return true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			TagLib::String key =
				QSTRING_TO_TSTRING(frame.getName().remove(' ').QCM_toUpper());
#ifdef TAGLIB_XIPHCOMMENT_REMOVEFIELD_CRASHES
			oggTag->removeField(key);
#else
			// This will crash because TagLib uses an invalidated iterator
			// after calling erase(). I hope this will be fixed in the next
			// version. Until then, remove all fields with that key.
			oggTag->removeField(key, QSTRING_TO_TSTRING(frame.getValue()));
#endif
			return true;
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			TagLib::String key = QSTRING_TO_TSTRING(frame.getName(true));
			apeTag->removeItem(key);
			return true;
		}
	}

	// Try the superclass method
	return TaggedFile::deleteFrameV2(frame);
}

/**
 * Get the frame type for an APE name.
 *
 * @param name APE tag name
 *
 * @return frame type.
 */
static Frame::Type getTypeFromApeName(const QString& name)
{
	Frame::Type type = Frame::getTypeFromName(name);
	if (type == Frame::FT_Other) {
		if (name == "YEAR") {
			type = Frame::FT_Date;
		} else if (name == "TRACK") {
			type = Frame::FT_Track;
		} else if (name == "ENCODED BY") {
			type = Frame::FT_EncodedBy;
		}
	}
	return type;
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void TagLibFile::deleteFramesV2(const FrameFilter& flt)
{
	if (m_tagV2) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if (flt.areAllEnabled()) {
			if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
				const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
				for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
						 it != frameList.end();) {
					id3v2Tag->removeFrame(*it++, true);
				}
				markTag2Changed();
			} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) !=
								 0) {
				const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
				for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
						 it != fieldListMap.end();) {
					oggTag->removeField((*it++).first);
				}
				markTag2Changed();
			} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
				const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
				for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
						 it != itemListMap.end();) {
					apeTag->removeItem((*it++).first);
				}
				markTag2Changed();
			} else {
				TaggedFile::deleteFramesV2(flt);
			}
		} else {
			if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
				const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
				Frame::Type type;
				const char* name;
				for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
						 it != frameList.end();) {
					getTypeStringForFrameId((*it)->frameID(), type, name);
					if (flt.isEnabled(type, name)) {
						id3v2Tag->removeFrame(*it++, true);
					} else {
						++it;
					}
				}
				markTag2Changed();
			} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) !=
								 0) {
				const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
				for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
						 it != fieldListMap.end();) {
					QString name(TStringToQString((*it).first));
					if (flt.isEnabled(Frame::getTypeFromName(name), name)) {
						oggTag->removeField((*it++).first);
					} else {
						++it;
					}
				}
				markTag2Changed();
			} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
				const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
				for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
						 it != itemListMap.end();) {
					QString name(TStringToQString((*it).first));
					if (flt.isEnabled(getTypeFromApeName(name), name)) {
						apeTag->removeItem((*it++).first);
					} else {
						++it;
					}
				}
				markTag2Changed();
			} else {
				TaggedFile::deleteFramesV2(flt);
			}
		}
	}
}

/**
 * Get all frames in tag 2.
 *
 * @param frames frame collection to set.
 */
void TagLibFile::getAllFramesV2(FrameCollection& frames)
{
	frames.clear();
	if (m_tagV2) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			int i = 0;
			Frame::Type type;
			const char* name;
			for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
					 it != frameList.end();
					 ++it) {
				getTypeStringForFrameId((*it)->frameID(), type, name);
				Frame frame(type, TStringToQString((*it)->toString()), name, i++);
				frame.setValue(getFieldsFromId3Frame(*it, frame.fieldList(), type));
				frames.insert(frame);
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
			int i = 0;
			for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
					 it != fieldListMap.end();
					 ++it) {
				QString name = TStringToQString((*it).first);
				Frame::Type type = Frame::getTypeFromName(name);
				TagLib::StringList stringList = (*it).second;
				for (TagLib::StringList::ConstIterator slit = stringList.begin();
						 slit != stringList.end();
						 ++slit) {
					frames.insert(Frame(type, TStringToQString(TagLib::String(*slit)),
																 name, i++));
				}
			}
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
			int i = 0;
			for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
					 it != itemListMap.end();
					 ++it) {
				QString name = TStringToQString((*it).first);
				TagLib::StringList values = (*it).second.toStringList();
				Frame::Type type = getTypeFromApeName(name);
				frames.insert(
					Frame(type, values.size() > 0 ? TStringToQString(values.front()) : "",
								name, i++));
			}
		}
	}
	frames.addMissingStandardFrames();
}

/**
 * Get a list of frame IDs which can be added.
 *
 * @return list with frame IDs.
 */
QStringList TagLibFile::getFrameIds() const
{
	QStringList lst(TaggedFile::getFrameIds());
	TagLib::ID3v2::Tag* id3v2Tag;
	if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
		for (unsigned i = 0; i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]); ++i) {
			const TypeStrOfId& ts = typeStrOfId[i];
			if (ts.type == Frame::FT_Other && ts.supported && ts.str) {
				lst.append(ts.str);
			}
		}
	} else {
		static const char* const fieldNames[] = {
			"ALBUMARTIST",
			"CATALOGNUMBER",
			"CONTACT",
			"DESCRIPTION",
			"EAN/UPN",
			"ENCODING",
			"ENGINEER",
			"ENSEMBLE",
			"GUEST ARTIST",
			"LABEL",
			"LABELNO",
			"LICENSE",
			"LOCATION",
			"OPUS",
			"ORGANIZATION",
			"PARTNUMBER",
			"PRODUCER",
			"PRODUCTNUMBER",
			"RECORDINGDATE",
			"RELEASE DATE",
			"REMIXER",
			"SOURCE ARTIST",
			"SOURCE MEDIUM",
			"SOURCE WORK",
			"SOURCEMEDIA",
			"SPARS",
			"TRACKTOTAL",
			"VERSION",
			"VOLUME"
		};
		for (unsigned i = 0; i < sizeof(fieldNames) / sizeof(fieldNames[0]); ++i) {
			lst.append(fieldNames[i]);
		}
	}
	return lst;
}


/**
 * Create an TagLibFile object if it supports the filename's extension.
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* TagLibFile::Resolver::createFile(const DirInfo* di,
																					const QString& fn) const
{
	QString ext = fn.right(4).QCM_toLower();
	if ((ext == ".mp3"
#ifdef HAVE_ID3LIB
			 && Kid3App::s_miscCfg.m_id3v2Version == MiscConfig::ID3v2_4_0
#endif
				)
			|| ext == ".mpc" || ext == ".ogg" || ext == "flac")
		return new TagLibFile(di, fn);
	else
		return 0;
}

/**
 * Get a list with all extensions supported by TagLibFile.
 *
 * @return list of file extensions.
 */
QStringList TagLibFile::Resolver::getSupportedFileExtensions() const
{
	return QStringList() << ".flac" << ".mp3" << ".mpc" << ".ogg";
}

#endif // HAVE_TAGLIB
