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
#include <qtextcodec.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QByteArray>
#endif

#include "genres.h"
#include "dirinfo.h"
#include "kid3.h"
#include "attributedata.h"
#include "pictureframe.h"
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

#if TAGLIB_VERSION <= 0x010400
#include "taglibext/generalencapsulatedobjectframe.h"
#include "taglibext/urllinkframe.h"
#include "taglibext/unsynchronizedlyricsframe.h"
#include "taglibext/speex/speexfile.h"
#include "taglibext/speex/taglib_speexfiletyperesolver.h"
#include "taglibext/trueaudio/ttafile.h"
#include "taglibext/trueaudio/taglib_trueaudiofiletyperesolver.h"
#include "taglibext/wavpack/wvfile.h"
#include "taglibext/wavpack/taglib_wavpackfiletyperesolver.h"
#else
#include <taglib/generalencapsulatedobjectframe.h>
#include <taglib/urllinkframe.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/speexfile.h>
#include <taglib/trueaudiofile.h>
#include <taglib/wavpackfile.h>
#endif

#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
#include <taglib/mp4file.h>
#endif
#ifdef TAGLIB_WITH_ASF
#include <taglib/asffile.h>
#endif
#include <taglib/aifffile.h>
#include <taglib/wavfile.h>
#include <taglib/popularimeterframe.h>
#include <taglib/privateframe.h>
#endif

#if TAGLIB_VERSION > 0x010603
#include <taglib/apefile.h>
#endif

#include "taglibext/aac/aacfiletyperesolver.h"
#include "taglibext/mp2/mp2filetyperesolver.h"


/**
 * Data encoding in ID3v1 tags.
 */
class TextCodecStringHandler : public TagLib::ID3v1::StringHandler {
public:
	/**
	 * Constructor.
	 */
  TextCodecStringHandler() {}

	/**
	 * Destructor.
	 */
	virtual ~TextCodecStringHandler() {}

	/**
	 * Decode a string from data.
	 *
	 * @param data data to decode
	 */
	virtual TagLib::String parse(const TagLib::ByteVector& data) const;

	/**
	 * Encode a byte vector with the data from a string.
	 *
	 * @param s string to encode
	 */
	virtual TagLib::ByteVector render(const TagLib::String& s) const;

	/**
	 * Set text codec.
	 * @param codec text codec, 0 for default behavior (ISO 8859-1)
	 */
	static void setTextCodec(const QTextCodec* codec) { s_codec = codec; }

private:
  static const QTextCodec* s_codec;
};

const QTextCodec* TextCodecStringHandler::s_codec = 0;

/**
 * Decode a string from data.
 *
 * @param data data to decode
 */
TagLib::String TextCodecStringHandler::parse(const TagLib::ByteVector& data) const
{
	return s_codec ?
		QSTRING_TO_TSTRING(s_codec->toUnicode(data.data(), data.size())).stripWhiteSpace() :
		TagLib::String(data, TagLib::String::Latin1).stripWhiteSpace();
}

/**
 * Encode a byte vector with the data from a string.
 *
 * @param s string to encode
 */
TagLib::ByteVector TextCodecStringHandler::render(const TagLib::String& s) const
{
	if (s_codec) {
		QCM_QCString ba(s_codec->fromUnicode(TStringToQString(s)));
		return TagLib::ByteVector(ba.data(), ba.size());
	} else {
		return s.data(TagLib::String::Latin1);
	}
}


/** Default text encoding */
TagLib::String::Type TagLibFile::s_defaultTextEncoding = TagLib::String::Latin1;

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
	QString fileName = getDirInfo()->getDirname() + QDir::separator() + currentFilename();
	QCM_QCString fn = QFile::encodeName(fileName);

	if (force || m_fileRef.isNull()) {
#if TAGLIB_VERSION > 0x010400 && defined _WIN32
		int fnLen = fileName.length();
		wchar_t* fnWs = new wchar_t[fnLen + 1];
		fnWs[fnLen] = 0;
		fileName.toWCharArray(fnWs);
		m_fileRef = TagLib::FileRef(TagLib::FileName(fnWs));
		delete [] fnWs;
#else
		m_fileRef = TagLib::FileRef(fn);
#endif
		m_tagV1 = 0;
		m_tagV2 = 0;
		markTag1Unchanged();
		markTag2Unchanged();
		m_fileRead = true;
	}

	TagLib::File* file;
	if (!m_fileRef.isNull() && (file = m_fileRef.file()) != 0) {
		TagLib::MPEG::File* mpegFile;
		TagLib::FLAC::File* flacFile;
#ifdef MPC_ID3V1
		TagLib::MPC::File* mpcFile;
		TagLib::WavPack::File* wvFile;
#endif
		TagLib::TrueAudio::File* ttaFile;
#if TAGLIB_VERSION > 0x010603
		TagLib::APE::File* apeFile;
#endif
		if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = mpegFile->ID3v1Tag();
				markTag1Unchanged();
			}
			if (!m_tagV2) {
				m_tagV2 = mpegFile->ID3v2Tag();
				markTag2Unchanged();
			}
		} else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = flacFile->ID3v1Tag();
				markTag1Unchanged();
			}
			if (!m_tagV2) {
				m_tagV2 = flacFile->xiphComment();
				markTag2Unchanged();
			}
#ifdef MPC_ID3V1
		} else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = mpcFile->ID3v1Tag();
				markTag1Unchanged();
			}
			if (!m_tagV2) {
				m_tagV2 = mpcFile->APETag();
				markTag2Unchanged();
			}
		} else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = wvFile->ID3v1Tag();
				markTag1Unchanged();
			}
			if (!m_tagV2) {
				m_tagV2 = wvFile->APETag();
				markTag2Unchanged();
			}
#endif
		} else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = ttaFile->ID3v1Tag();
				markTag1Unchanged();
			}
			if (!m_tagV2) {
				m_tagV2 = ttaFile->ID3v2Tag();
				markTag2Unchanged();
			}
#if TAGLIB_VERSION > 0x010603
		} else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != 0) {
			if (!m_tagV1) {
				m_tagV1 = apeFile->ID3v1Tag();
				markTag1Unchanged();
			}
			if (!m_tagV2) {
				m_tagV2 = apeFile->APETag();
				markTag2Unchanged();
			}
#endif
		} else {
			m_tagV1 = 0;
			markTag1Unchanged();
			if (!m_tagV2) {
				m_tagV2 = m_fileRef.tag();
				markTag2Unchanged();
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
				markTag1Unchanged();
				m_tagV1 = 0;
			}
			if (m_tagV2 && (force || isTag2Changed()) && m_tagV2->isEmpty()) {
				mpegFile->strip(TagLib::MPEG::File::ID3v2);
				fileChanged = true;
				markTag2Unchanged();
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
						markTag1Unchanged();
					}
					if (saveMask & TagLib::MPEG::File::ID3v2) {
						markTag2Unchanged();
					}
				}
			}
		} else {
			if ((m_tagV2 && (force || isTag2Changed())) ||
					(m_tagV1 && (force || isTag1Changed()))) {
				TagLib::TrueAudio::File* ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file);
#if TAGLIB_VERSION > 0x010603
				TagLib::APE::File* apeFile = dynamic_cast<TagLib::APE::File*>(file);
#endif
#ifndef MPC_ID3V1
				// it does not work if there is also an ID3 tag (bug in TagLib?)
				TagLib::MPC::File* mpcFile = dynamic_cast<TagLib::MPC::File*>(file);
				TagLib::WavPack::File* wvFile = dynamic_cast<TagLib::WavPack::File*>(file);
				if (mpcFile) {
					mpcFile->remove(TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2);
					fileChanged = true;
				} else if (wvFile) {
					wvFile->strip(TagLib::WavPack::File::ID3v1);
					fileChanged = true;
				} else
#endif
				if (ttaFile) {
					if (m_tagV1 && (force || isTag1Changed()) && m_tagV1->isEmpty()) {
						ttaFile->strip(TagLib::TrueAudio::File::ID3v1);
						fileChanged = true;
						markTag1Unchanged();
						m_tagV1 = 0;
					}
					if (m_tagV2 && (force || isTag2Changed()) && m_tagV2->isEmpty()) {
						ttaFile->strip(TagLib::TrueAudio::File::ID3v2);
						fileChanged = true;
						markTag2Unchanged();
						m_tagV2 = 0;
					}
				}
#if TAGLIB_VERSION > 0x010603
				if (apeFile) {
					if (m_tagV1 && (force || isTag1Changed()) && m_tagV1->isEmpty()) {
						apeFile->strip(TagLib::APE::File::ID3v1);
						fileChanged = true;
						markTag1Unchanged();
						m_tagV1 = 0;
					}
					if (m_tagV2 && (force || isTag2Changed()) && m_tagV2->isEmpty()) {
						apeFile->strip(TagLib::APE::File::APE);
						fileChanged = true;
						markTag2Unchanged();
						m_tagV2 = 0;
					}
				}
#endif
				if (m_fileRef.save()) {
					fileChanged = true;
					markTag1Unchanged();
					markTag2Unchanged();
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
		if (str.isNull()) {
			return QString("");
		} else {
			QString qstr(TStringToQString(str));
			qstr.truncate(28);
			return qstr;
		}
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
			TagLib::WavPack::File* wvFile;
#endif
			TagLib::TrueAudio::File* ttaFile;
#if TAGLIB_VERSION > 0x010603
			TagLib::APE::File* apeFile;
#endif
			if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
				m_tagV1 = mpegFile->ID3v1Tag(true);
			} else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
				m_tagV1 = flacFile->ID3v1Tag(true);
#ifdef MPC_ID3V1
			} else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
				m_tagV1 = mpcFile->ID3v1Tag(true);
			} else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != 0) {
				m_tagV1 = wvFile->ID3v1Tag(true);
#endif
			} else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != 0) {
				m_tagV1 = ttaFile->ID3v1Tag(true);
#if TAGLIB_VERSION > 0x010603
			} else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != 0) {
				m_tagV1 = apeFile->ID3v1Tag(true);
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
			TagLib::WavPack::File* wvFile;
			TagLib::TrueAudio::File* ttaFile;
#if TAGLIB_VERSION > 0x010603
			TagLib::APE::File* apeFile;
#endif
			if ((mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) != 0) {
				m_tagV2 = mpegFile->ID3v2Tag(true);
			} else if ((flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) != 0) {
				m_tagV2 = flacFile->xiphComment(true);
			} else if ((mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) != 0) {
				m_tagV2 = mpcFile->APETag(true);
			} else if ((wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) != 0) {
				m_tagV2 = wvFile->APETag(true);
			} else if ((ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) != 0) {
				m_tagV2 = ttaFile->ID3v2Tag(true);
#if TAGLIB_VERSION > 0x010603
			} else if ((apeFile = dynamic_cast<TagLib::APE::File*>(file)) != 0) {
				m_tagV2 = apeFile->APETag(true);
#endif
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
			QString s = checkTruncation(str, 1 << Frame::FT_Title);
			if (!s.isNull())
				m_tagV1->setTitle(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setTitle(tstr);
			markTag1Changed(Frame::FT_Title);
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
			QString s = checkTruncation(str, 1 << Frame::FT_Artist);
			if (!s.isNull())
				m_tagV1->setArtist(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setArtist(tstr);
			markTag1Changed(Frame::FT_Artist);
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
			QString s = checkTruncation(str, 1 << Frame::FT_Album);
			if (!s.isNull())
				m_tagV1->setAlbum(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setAlbum(tstr);
			markTag1Changed(Frame::FT_Album);
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
			QString s = checkTruncation(str, 1 << Frame::FT_Comment, 28);
			if (!s.isNull())
				m_tagV1->setComment(QSTRING_TO_TSTRING(s));
			else
				m_tagV1->setComment(tstr);
			markTag1Changed(Frame::FT_Comment);
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
			markTag1Changed(Frame::FT_Date);
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
			int n = checkTruncation(num, 1 << Frame::FT_Track);
			if (n != -1)
				m_tagV1->setTrack(n);
			else
				m_tagV1->setTrack(num);
			markTag1Changed(Frame::FT_Track);
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
			markTag1Changed(Frame::FT_Genre);
		}
		// if the string cannot be converted to a number, set the truncation flag
		checkTruncation(!str.isEmpty() && Genres::getNumber(str) == 0xff ? 1 : 0,
										1 << Frame::FT_Genre, 0);
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
		char ch = qcarray[i].
#if QT_VERSION >= 0x040000
			toLatin1()
#else
			latin1()
#endif
			;
		if (ch == 0 || (ch & 0x80) != 0) {
			result = true;
			break;
		}
	}
	return result;
}

/**
 * Get the configured text encoding.
 *
 * @param unicode true if unicode is required
 *
 * @return text encoding.
 */
static TagLib::String::Type getTextEncodingConfig(bool unicode)
{
	TagLib::String::Type enc = TagLibFile::getDefaultTextEncoding();
	if (unicode && enc == TagLib::String::Latin1) {
		enc = TagLib::String::UTF8;
	}
	return enc;
}

/**
 * Remove the first COMM frame with an empty description.
 *
 * @param id3v2Tag ID3v2 tag
 */
static void removeCommentFrame(TagLib::ID3v2::Tag* id3v2Tag)
{
	const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList("COMM");
	for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
			 it != frameList.end();
			 ++it) {
		TagLib::ID3v2::CommentsFrame* id3Frame =
			dynamic_cast<TagLib::ID3v2::CommentsFrame*>(*it);
		if (id3Frame && id3Frame->description().isEmpty()) {
			id3v2Tag->removeFrame(id3Frame, true);
			break;
		}
	}
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
		TagLib::String::Type enc = getTextEncodingConfig(needsUnicode(qstr));
		TagLib::ByteVector id(frameId);
		if (enc != TagLib::String::Latin1 || id == "COMM") {
			if (id == "COMM") {
				removeCommentFrame(id3v2Tag);
			} else {
				id3v2Tag->removeFrames(id);
			}
			if (!tstr.isEmpty()) {
				TagLib::ID3v2::Frame* frame;
				if (frameId[0] != 'C') {
					frame = new TagLib::ID3v2::TextIdentificationFrame(id, enc);
				} else {
					frame = new TagLib::ID3v2::CommentsFrame(enc);
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
			markTag2Changed(Frame::FT_Title);
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
			markTag2Changed(Frame::FT_Artist);
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
			markTag2Changed(Frame::FT_Album);
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
			markTag2Changed(Frame::FT_Comment);
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
			if (getDefaultTextEncoding() == TagLib::String::Latin1) {
				m_tagV2->setYear(num);
			} else {
				QString str;
				if (num != 0) {
					str.setNum(num);
				} else {
					str = "";
				}
				TagLib::String tstr = str.isEmpty() ?
					TagLib::String::null : QSTRING_TO_TSTRING(str);
				if (!setId3v2Unicode(m_tagV2, str, tstr, "TDRC")) {
					m_tagV2->setYear(num);
				}
			}
			markTag2Changed(Frame::FT_Date);
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
			TagLib::ID3v2::Tag* id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2);
			if (id3v2Tag) {
				TagLib::ID3v2::TextIdentificationFrame* frame;
				QString str;
				if (num != 0) {
					str.setNum(num);
					formatTrackNumberIfEnabled(str, true);
				} else {
					str = "";
				}
				TagLib::String tstr = str.isEmpty() ?
					TagLib::String::null : QSTRING_TO_TSTRING(str);
				if (!setId3v2Unicode(m_tagV2, str, tstr, "TRCK")) {
					if ((frame = new TagLib::ID3v2::TextIdentificationFrame(
								 "TRCK", getDefaultTextEncoding())) != 0) {
						frame->setText(tstr);
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
				}
			} else {
				m_tagV2->setTrack(num);
			}
			markTag2Changed(Frame::FT_Track);
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
			if (!setId3v2Unicode(m_tagV2, str, tstr, "TCON")) {
				TagLib::ID3v2::TextIdentificationFrame* frame;
				TagLib::ID3v2::Tag* id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2);
				if (id3v2Tag && Kid3App::s_miscCfg.m_genreNotNumeric &&
						(frame = new TagLib::ID3v2::TextIdentificationFrame(
							"TCON", getDefaultTextEncoding())) != 0) {
					frame->setText(tstr);
					id3v2Tag->removeFrames("TCON");
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
					m_tagV2->setGenre(tstr);
				}
			}
			markTag2Changed(Frame::FT_Genre);
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
					 dynamic_cast<TagLib::FLAC::File*>(file) != 0 ||
					 dynamic_cast<TagLib::TrueAudio::File*>(file) != 0
#ifdef MPC_ID3V1
					 || dynamic_cast<TagLib::MPC::File*>(file)  != 0
					 || dynamic_cast<TagLib::WavPack::File*>(file) != 0
#endif
#if TAGLIB_VERSION > 0x010603
					 || dynamic_cast<TagLib::APE::File*>(file) != 0
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
 * @param info the detail information is returned here
 */
void TagLibFile::getDetailInfo(DetailInfo& info) const
{
	TagLib::AudioProperties* audioProperties;
	if (!m_fileRef.isNull() &&
			(audioProperties = m_fileRef.audioProperties()) != 0) {
		TagLib::MPEG::Properties* mpegProperties;
		TagLib::Vorbis::Properties* oggProperties;
		TagLib::FLAC::Properties* flacProperties;
		TagLib::MPC::Properties* mpcProperties;
		TagLib::Ogg::Speex::Properties* speexProperties;
		TagLib::TrueAudio::Properties* ttaProperties;
		TagLib::WavPack::Properties* wvProperties;
#if TAGLIB_VERSION > 0x010603
		TagLib::APE::Properties* apeProperties;
#endif
		info.valid = true;
		if ((mpegProperties =
				 dynamic_cast<TagLib::MPEG::Properties*>(audioProperties)) != 0) {
			if (getFilename().right(4).QCM_toLower() == ".aac") {
				info.format = "AAC";
				return;
			}
			switch (mpegProperties->version()) {
				case TagLib::MPEG::Header::Version1:
					info.format = "MPEG 1 ";
					break;
				case TagLib::MPEG::Header::Version2:
					info.format = "MPEG 2 ";
					break;
				case TagLib::MPEG::Header::Version2_5:
					info.format = "MPEG 2.5 ";
					break;
			}
			int layer = mpegProperties->layer();
			if (layer >= 1 && layer <= 3) {
				info.format += "Layer ";
				info.format += QString::number(layer);
			}
			switch (mpegProperties->channelMode()) {
				case TagLib::MPEG::Header::Stereo:
					info.channelMode = DetailInfo::CM_Stereo;
					info.channels = 2;
					break;
				case TagLib::MPEG::Header::JointStereo:
					info.channelMode = DetailInfo::CM_JointStereo;
					info.channels = 2;
					break;
				case TagLib::MPEG::Header::DualChannel:
					info.channels = 2;
					break;
				case TagLib::MPEG::Header::SingleChannel:
					info.channels = 1;
					break;
			}
		} else if ((oggProperties =
								dynamic_cast<TagLib::Vorbis::Properties*>(audioProperties)) !=
							 0) {
			info.format = "Ogg Vorbis";
		} else if ((flacProperties =
								dynamic_cast<TagLib::FLAC::Properties*>(audioProperties)) !=
							 0) {
			info.format = "FLAC";
		} else if ((mpcProperties =
								dynamic_cast<TagLib::MPC::Properties*>(audioProperties)) != 0) {
			info.format = "MPC";
		} else if ((speexProperties =
								dynamic_cast<TagLib::Ogg::Speex::Properties*>(audioProperties)) != 0) {
			info.format = QString("Speex %1").arg(speexProperties->speexVersion());
		} else if ((ttaProperties =
								dynamic_cast<TagLib::TrueAudio::Properties*>(audioProperties)) != 0) {
			info.format = "True Audio ";
			info.format += QString::number(ttaProperties->ttaVersion());
			info.format += " ";
			info.format += QString::number(ttaProperties->bitsPerSample());
			info.format += " bit";
		} else if ((wvProperties =
								dynamic_cast<TagLib::WavPack::Properties*>(audioProperties)) != 0) {
			info.format = "WavPack ";
			info.format += QString::number(wvProperties->version(), 16);
			info.format += " ";
			info.format += QString::number(wvProperties->bitsPerSample());
			info.format += " bit";
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if (dynamic_cast<TagLib::MP4::Properties*>(audioProperties) != 0) {
			info.format = "MP4";
#endif
#ifdef TAGLIB_WITH_ASF
		} else if (dynamic_cast<TagLib::ASF::Properties*>(audioProperties) != 0) {
			info.format = "ASF";
#endif
		} else if (dynamic_cast<TagLib::RIFF::AIFF::Properties*>(audioProperties) != 0) {
			info.format = "AIFF";
		} else if (dynamic_cast<TagLib::RIFF::WAV::Properties*>(audioProperties) != 0) {
			info.format = "WAV";
#endif
#if TAGLIB_VERSION > 0x010603
		} else if ((apeProperties =
								dynamic_cast<TagLib::APE::Properties*>(audioProperties)) != 0) {
			info.format = QString("APE %1.%2 %3 bit").
				arg(apeProperties->version() / 1000).
				arg(apeProperties->version() % 1000).
				arg(apeProperties->bitsPerSample());
#endif
		}
		info.bitrate = audioProperties->bitrate();
		info.sampleRate = audioProperties->sampleRate();
		if (audioProperties->channels() > 0) {
			info.channels = audioProperties->channels();
		}
		info.duration = audioProperties->length();
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
		} else if (dynamic_cast<TagLib::Ogg::Speex::File*>(file) != 0) {
			return ".spx";
		} else if (dynamic_cast<TagLib::WavPack::File*>(file) != 0) {
			return ".wv";
		} else if (dynamic_cast<TagLib::TrueAudio::File*>(file) != 0) {
			return ".tta";
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if (dynamic_cast<TagLib::MP4::File*>(file) != 0) {
			return ".m4a";
#endif
#ifdef TAGLIB_WITH_ASF
		} else if (dynamic_cast<TagLib::ASF::File*>(file) != 0) {
			return ".wma";
#endif
		} else if (dynamic_cast<TagLib::RIFF::AIFF::File*>(file) != 0) {
			return ".aiff";
		} else if (dynamic_cast<TagLib::RIFF::WAV::File*>(file) != 0) {
			return ".wav";
#endif
#if TAGLIB_VERSION > 0x010603
		} else if (dynamic_cast<TagLib::APE::File*>(file) != 0) {
			return ".ape";
#endif
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
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if (dynamic_cast<const TagLib::MP4::Tag*>(tag) != 0) {
			return "MP4";
#endif
#ifdef TAGLIB_WITH_ASF
		} else if (dynamic_cast<const TagLib::ASF::Tag*>(tag) != 0) {
			return "ASF";
#endif
#endif
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
	{ Frame::FT_Picture,        I18N_NOOP("APIC - Attached picture"), true },
	{ Frame::FT_Other,          I18N_NOOP("ASPI - Audio seek point index"), false },
	{ Frame::FT_Comment,        I18N_NOOP("COMM - Comments"), true },
	{ Frame::FT_Other,          I18N_NOOP("COMR - Commercial"), false },
	{ Frame::FT_Other,          I18N_NOOP("ENCR - Encryption method registration"), false },
	{ Frame::FT_Other,          I18N_NOOP("EQU2 - Equalisation (2)"), false },
	{ Frame::FT_Other,          I18N_NOOP("ETCO - Event timing codes"), false },
	{ Frame::FT_Other,          I18N_NOOP("GEOB - General encapsulated object"), true },
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
	{ Frame::FT_Arranger,       I18N_NOOP("TIPL - Involved people list"), true },
	{ Frame::FT_Grouping,       I18N_NOOP("TIT1 - Content group description"), true },
	{ Frame::FT_Title,          I18N_NOOP("TIT2 - Title/songname/content description"), true },
	{ Frame::FT_Subtitle,       I18N_NOOP("TIT3 - Subtitle/Description refinement"), true },
	{ Frame::FT_Other,          I18N_NOOP("TKEY - Initial key"), true },
	{ Frame::FT_Language,       I18N_NOOP("TLAN - Language(s)"), true },
	{ Frame::FT_Other,          I18N_NOOP("TLEN - Length"), true },
	{ Frame::FT_Performer,      I18N_NOOP("TMCL - Musician credits list"), true },
	{ Frame::FT_Media,          I18N_NOOP("TMED - Media type"), true },
	{ Frame::FT_Other,          I18N_NOOP("TMOO - Mood"), true },
	{ Frame::FT_OriginalAlbum,  I18N_NOOP("TOAL - Original album/movie/show title"), true },
	{ Frame::FT_Other,          I18N_NOOP("TOFN - Original filename"), true },
	{ Frame::FT_Author,         I18N_NOOP("TOLY - Original lyricist(s)/text writer(s)"), true },
	{ Frame::FT_OriginalArtist, I18N_NOOP("TOPE - Original artist(s)/performer(s)"), true },
	{ Frame::FT_Other,          I18N_NOOP("TOWN - File owner/licensee"), true },
	{ Frame::FT_Artist,         I18N_NOOP("TPE1 - Lead performer(s)/Soloist(s)"), true },
	{ Frame::FT_AlbumArtist,    I18N_NOOP("TPE2 - Band/orchestra/accompaniment"), true },
	{ Frame::FT_Conductor,      I18N_NOOP("TPE3 - Conductor/performer refinement"), true },
	{ Frame::FT_Remixer,        I18N_NOOP("TPE4 - Interpreted, remixed, or otherwise modified by"), true },
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
	{ Frame::FT_Lyrics,         I18N_NOOP("USLT - Unsynchronized lyric/text transcription"), true },
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
		// if there are multiple items, put them into one string
		// separated by a special separator.
		text = TStringToQString(tFrame->fieldList().toString(Frame::stringListSeparator()));
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
	field.m_value = QString(
#if QT_VERSION >= 0x040000
		QByteArray(bvLang.data(), bvLang.size())
#else
		QCString(bvLang.data(), bvLang.size() + 1)
#endif
		);
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

	if (!ba.isEmpty()) {
		QString text(ba);
		if (ba.size() - text.length() <= 1 &&
				AttributeData::isHexString(text, 'Z')) {
			return text;
		}
	}
	return QString();
}

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

/**
 * Get the fields from a URL link frame.
 *
 * @param wFrame URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromUrlFrame(
	const TagLib::ID3v2::UrlLinkFrame* wFrame, Frame::FieldList& fields)
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
	const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame, Frame::FieldList& fields)
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
	const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame,
	Frame::FieldList& fields)
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

#if TAGLIB_VERSION >= 0x010600
/**
 * Get the fields from a private frame.
 *
 * @param privFrame private frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromPrivFrame(
	const TagLib::ID3v2::PrivateFrame* privFrame,
	Frame::FieldList& fields)
{
	QString owner;
	Frame::Field field;
	field.m_id = Frame::Field::ID_Owner;
	owner = TStringToQString(privFrame->owner());
	field.m_value = owner;
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Data;
	TagLib::ByteVector data = privFrame->data();
	QByteArray ba;
	QCM_duplicate(ba, data.data(), data.size());
	field.m_value = ba;
	fields.push_back(field);

	if (!owner.isEmpty() && !ba.isEmpty()) {
		QString str;
		if (AttributeData(owner).toString(ba, str)) {
			return str;
		}
	}
	return QString();
}

/**
 * Get the fields from a popularimeter frame.
 *
 * @param popmFrame popularimeter frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
static QString getFieldsFromPopmFrame(
	const TagLib::ID3v2::PopularimeterFrame* popmFrame,
	Frame::FieldList& fields)
{
	Frame::Field field;
	field.m_id = Frame::Field::ID_Email;
	field.m_value = TStringToQString(popmFrame->email());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Rating;
	field.m_value = popmFrame->rating();
	QString text(field.m_value.toString());
	fields.push_back(field);

	field.m_id = Frame::Field::ID_Counter;
	field.m_value = popmFrame->counter();
	fields.push_back(field);

	return text;
}
#endif

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
		const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
		const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
		const TagLib::ID3v2::UrlLinkFrame* wFrame;
		const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
#if TAGLIB_VERSION >= 0x010600
		const TagLib::ID3v2::PrivateFrame* privFrame;
		const TagLib::ID3v2::PopularimeterFrame* popmFrame;
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
		} else if ((geobFrame =
								dynamic_cast<const TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
									frame)) != 0) {
			return getFieldsFromGeobFrame(geobFrame, fields);
		} else if ((wxxxFrame = dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame*>(
									frame)) != 0) {
			return getFieldsFromUserUrlFrame(wxxxFrame, fields);
		} else if ((wFrame = dynamic_cast<const TagLib::ID3v2::UrlLinkFrame*>(
									frame)) != 0) {
			return getFieldsFromUrlFrame(wFrame, fields);
		} else if ((usltFrame = dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
								frame)) != 0) {
			return getFieldsFromUsltFrame(usltFrame, fields);
#if TAGLIB_VERSION >= 0x010600
		} else if ((privFrame = dynamic_cast<const TagLib::ID3v2::PrivateFrame*>(
								frame)) != 0) {
			return getFieldsFromPrivFrame(privFrame, fields);
		} else if ((popmFrame = dynamic_cast<const TagLib::ID3v2::PopularimeterFrame*>(
								frame)) != 0) {
			return getFieldsFromPopmFrame(popmFrame, fields);
#endif
		} else {
			TagLib::ByteVector id = frame->frameID();
#ifndef TAGLIB_SUPPORTS_URLLINK_FRAMES
			if (id.startsWith("WXXX")) {
				TagLib::ID3v2::UserUrlLinkFrame userUrlLinkFrame(frame->render());
				return getFieldsFromUserUrlFrame(&userUrlLinkFrame, fields);
			} else if (id.startsWith("W")) {
				TagLib::ID3v2::UrlLinkFrame urlLinkFrame(frame->render());
				return getFieldsFromUrlFrame(&urlLinkFrame, fields);
			} else
#endif
#ifndef TAGLIB_SUPPORTS_USLT_FRAMES
			if (id.startsWith("USLT")) {
				TagLib::ID3v2::UnsynchronizedLyricsFrame usltFrame(frame->render());
				return getFieldsFromUsltFrame(&usltFrame, fields);
			} else
#endif
#ifndef TAGLIB_SUPPORTS_GEOB_FRAMES
			if (id.startsWith("GEOB")) {
				TagLib::ID3v2::GeneralEncapsulatedObjectFrame geobFrame(frame->render());
				return getFieldsFromGeobFrame(&geobFrame, fields);
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
 * The following template functions are used to uniformly set fields
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

template <>
void setTextEncoding(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UserUrlLinkFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
										 TagLib::String::Type enc)
{
	f->setTextEncoding(enc);
}


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

template <>
void setDescription(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
										const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
										const Frame::Field& fld)
{
	f->setDescription(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <class T>
void setMimeType(T*, const Frame::Field&) {}

template <>
void setMimeType(TagLib::ID3v2::AttachedPictureFrame* f,
								 const Frame::Field& fld)
{
	f->setMimeType(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <>
void setMimeType(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
								 const Frame::Field& fld)
{
	f->setMimeType(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

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

template <>
void setData(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
						 const Frame::Field& fld)
{
	QByteArray ba(fld.m_value.toByteArray());
	f->setObject(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
						 const Frame::Field& fld)
{
	QByteArray ba(fld.m_value.toByteArray());
	f->setIdentifier(TagLib::ByteVector(ba.data(), ba.size()));
}

template <class T>
void setLanguage(T*, const Frame::Field&) {}

template <>
void setLanguage(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
	f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <>
void setLanguage(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
								 const Frame::Field& fld)
{
	f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <class T>
void setOwner(T*, const Frame::Field&) {}

template <>
void setOwner(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
							const Frame::Field& fld)
{
	f->setOwner(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

#if TAGLIB_VERSION >= 0x010600
template <>
void setOwner(TagLib::ID3v2::PrivateFrame* f,
							const Frame::Field& fld)
{
	f->setOwner(QSTRING_TO_TSTRING(fld.m_value.toString()));
}
#endif

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

template <>
void setFilename(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
								 const Frame::Field& fld)
{
	f->setFileName(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <class T>
void setUrl(T*, const Frame::Field&) {}

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

template <>
void setValue(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
              const TagLib::String& text)
{
	f->setDescription(text);
}

static void setStringOrList(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
	if (text.find(Frame::stringListSeparator()) == -1) {
		f->setText(text);
	} else {
		f->setText(TagLib::StringList::split(text, Frame::stringListSeparator()));
	}
}

template <>
void setValue(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
	setStringOrList(f, text);
}

template <>
void setValue(TagLib::ID3v2::UniqueFileIdentifierFrame* f, const TagLib::String& text)
{
	if (AttributeData::isHexString(TStringToQString(text), 'Z')) {
		TagLib::ByteVector data(text.data(TagLib::String::Latin1));
		data.append('\0');
		f->setIdentifier(data);
	}
}

#if TAGLIB_VERSION >= 0x010600
template <>
void setValue(TagLib::ID3v2::PrivateFrame* f, const TagLib::String& text)
{
	QByteArray newData;
	TagLib::String owner = f->owner();
	if (!owner.isEmpty() &&
			AttributeData(TStringToQString(owner)).
			toByteArray(TStringToQString(text), newData)) {
		f->setData(TagLib::ByteVector(newData.data(), newData.size()));
	}
}

template <>
void setValue(TagLib::ID3v2::PopularimeterFrame* f, const TagLib::String& text)
{
	f->setRating(text.toInt());
}
#endif

template <class T>
void setText(T* f, const TagLib::String& text)
{
	f->setText(text);
}

template <>
void setText(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
	setStringOrList(f, text);
}

#if TAGLIB_VERSION >= 0x010600
template <class T>
void setEmail(T*, const Frame::Field&) {}

template <>
void setEmail(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
	f->setEmail(QSTRING_TO_TSTRING(fld.m_value.toString()));
}

template <class T>
void setRating(T*, const Frame::Field&) {}

template <>
void setRating(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
	f->setRating(fld.m_value.toInt());
}

template <class T>
void setCounter(T*, const Frame::Field&) {}

template <>
void setCounter(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
	f->setCounter(fld.m_value.toUInt());
}
#endif

/**
 * Set the fields in a TagLib ID3v2 frame.
 *
 * @param self   this TagLibFile instance
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
			if (!Kid3App::s_miscCfg.m_genreNotNumeric) {
				text = Genres::getNumberString(text, false);
			}
		} else if (frame.getType() == Frame::FT_Track) {
			self->formatTrackNumberIfEnabled(text, true);
		}
		setValue(tFrame, QSTRING_TO_TSTRING(text));
		setTextEncoding(tFrame, getTextEncodingConfig(needsUnicode(text)));
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
						if (!Kid3App::s_miscCfg.m_genreNotNumeric) {
							value = Genres::getNumberString(value, false);
						}
					} else if (frame.getType() == Frame::FT_Track) {
						self->formatTrackNumberIfEnabled(value, true);
					}
					setText(tFrame, QSTRING_TO_TSTRING(value));
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
#if TAGLIB_VERSION >= 0x010600
				case Frame::Field::ID_Email:
					setEmail(tFrame, fld);
					break;
				case Frame::Field::ID_Rating:
					setRating(tFrame, fld);
					break;
				case Frame::Field::ID_Counter:
					setCounter(tFrame, fld);
					break;
#endif
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
		TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame;
		TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame;
		TagLib::ID3v2::UrlLinkFrame* wFrame;
		TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame;
#if TAGLIB_VERSION >= 0x010600
		TagLib::ID3v2::PrivateFrame* privFrame;
		TagLib::ID3v2::PopularimeterFrame* popmFrame;
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
		} else if ((geobFrame =
								dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, geobFrame, frame);
		} else if ((wxxxFrame = dynamic_cast<TagLib::ID3v2::UserUrlLinkFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, wxxxFrame, frame);
		} else if ((wFrame = dynamic_cast<TagLib::ID3v2::UrlLinkFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, wFrame, frame);
		} else if ((usltFrame =
								dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, usltFrame, frame);
#if TAGLIB_VERSION >= 0x010600
		} else if ((privFrame = dynamic_cast<TagLib::ID3v2::PrivateFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, privFrame, frame);
		} else if ((popmFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(
									id3Frame)) != 0) {
			setTagLibFrame(this, popmFrame, frame);
#endif
		} else {
			TagLib::ByteVector id(id3Frame->frameID());
			// create temporary objects for frames not known by TagLib,
			// an UnknownFrame copy will be created by the edit method.
#ifndef TAGLIB_SUPPORTS_URLLINK_FRAMES
			if (id.startsWith("WXXX")) {
				TagLib::ID3v2::UserUrlLinkFrame userUrlLinkFrame(id3Frame->render());
				setTagLibFrame(this, &userUrlLinkFrame, frame);
				id3Frame->setData(userUrlLinkFrame.render());
			} else if (id.startsWith("W")) {
				TagLib::ID3v2::UrlLinkFrame urlLinkFrame(id3Frame->render());
				setTagLibFrame(this, &urlLinkFrame, frame);
				id3Frame->setData(urlLinkFrame.render());
			} else
#endif
#ifndef TAGLIB_SUPPORTS_USLT_FRAMES
			if (id.startsWith("USLT")) {
				TagLib::ID3v2::UnsynchronizedLyricsFrame usltFrame(id3Frame->render());
				setTagLibFrame(this, &usltFrame, frame);
				id3Frame->setData(usltFrame.render());
			} else
#endif
#ifndef TAGLIB_SUPPORTS_GEOB_FRAMES
			if (id.startsWith("GEOB")) {
				TagLib::ID3v2::GeneralEncapsulatedObjectFrame geobFrame(id3Frame->render());
				setTagLibFrame(this, &geobFrame, frame);
				id3Frame->setData(geobFrame.render());
			} else
#endif
			{
				setTagLibFrame(this, id3Frame, frame);
			}
		}
	}
}

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
static const char* getVorbisNameFromType(Frame::Type type)
{
  static const char* const names[] = {
		"TITLE",           // FT_Title,
		"ARTIST",          // FT_Artist,
		"ALBUM",           // FT_Album,
		"COMMENT",         // FT_Comment,
		"DATE",            // FT_Date,
		"TRACKNUMBER",     // FT_Track,
		"GENRE",           // FT_Genre,
		                   // FT_LastV1Frame = FT_Track,
		"ALBUMARTIST",     // FT_AlbumArtist,
		"ARRANGER",        // FT_Arranger,
		"AUTHOR",          // FT_Author,
		"BPM",             // FT_Bpm,
		"COMPOSER",        // FT_Composer,
		"CONDUCTOR",       // FT_Conductor,
		"COPYRIGHT",       // FT_Copyright,
		"DISCNUMBER",      // FT_Disc,
		"ENCODED-BY",      // FT_EncodedBy,
		"GROUPING",        // FT_Grouping,
		"ISRC",            // FT_Isrc,
		"LANGUAGE",        // FT_Language,
		"LYRICIST",        // FT_Lyricist,
		"LYRICS",          // FT_Lyrics,
		"SOURCEMEDIA",     // FT_Media,
		"ORIGINALALBUM",   // FT_OriginalAlbum,
		"ORIGINALARTIST",  // FT_OriginalArtist,
		"ORIGINALDATE",    // FT_OriginalDate,
		"PART",            // FT_Part,
		"PERFORMER",       // FT_Performer,
		"UNKNOWN",         // FT_Picture,
		"PUBLISHER",       // FT_Publisher,
		"REMIXER",         // FT_Remixer,
		"SUBTITLE",        // FT_Subtitle,
		"WEBSITE",         // FT_Website,
		                   // FT_LastFrame = FT_Website
	};
	class not_used { int array_size_check[
			sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1
			? 1 : -1 ]; };
	return type <= Frame::FT_LastFrame ? names[type] : "UNKNOWN";
}

/**
 * Get the frame type for a Vorbis name.
 *
 * @param name Vorbis tag name
 *
 * @return frame type.
 */
static Frame::Type getTypeFromVorbisName(QString name)
{
	static QMap<QString, int> strNumMap;
	if (strNumMap.empty()) {
		// first time initialization
		for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
			Frame::Type type = static_cast<Frame::Type>(i);
			strNumMap.insert(getVorbisNameFromType(type), type);
		}
		strNumMap.insert("DESCRIPTION", Frame::FT_Comment);
	}
	QMap<QString, int>::const_iterator it =
		strNumMap.find(name.remove(' ').QCM_toUpper());
	if (it != strNumMap.end()) {
		return static_cast<Frame::Type>(*it);
	}
	return Frame::FT_Other;
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
	Frame::Type type = getTypeFromVorbisName(name);
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
 * Get internal name of a Vorbis frame.
 *
 * @param frame frame
 *
 * @return Vorbis key.
 */
static QString getVorbisName(const Frame& frame)
{
	Frame::Type type = frame.getType();
	if (type == Frame::FT_Comment) {
		return "DESCRIPTION";
	} else if (type <= Frame::FT_LastFrame) {
		return getVorbisNameFromType(type);
	} else {
		return frame.getName().remove(' ').QCM_toUpper();
	}
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
	Frame::Type type = frame.getType();
	if (type == Frame::FT_Date) {
		return "YEAR";
	} else if (type == Frame::FT_Track) {
		return "TRACK";
	} else if (type <= Frame::FT_LastFrame) {
		return getVorbisNameFromType(type);
	} else {
		return frame.getName().QCM_toUpper();
	}
}

#ifdef TAGLIB_WITH_MP4
/** Type of data in MP4 frame. */
enum Mp4ValueType {
	MVT_ByteArray,
#if TAGLIB_VERSION >= 0x010602
	MVT_CoverArt,
#endif
	MVT_String,
	MVT_Bool,
	MVT_Int,
	MVT_IntPair
};

/** MP4 name, frame type and value type. */
struct Mp4NameTypeValue {
	const char* name;
	Frame::Type type;
	Mp4ValueType value;
};

/** Mapping between frame types and field names. */
static const Mp4NameTypeValue mp4NameTypeValues[] = {
	{ "\251nam", Frame::FT_Title, MVT_String },
	{ "\251ART", Frame::FT_Artist, MVT_String },
	{ "\251wrt", Frame::FT_Composer, MVT_String },
	{ "\251alb", Frame::FT_Album, MVT_String },
	{ "\251day", Frame::FT_Date, MVT_String },
	{ "\251too", Frame::FT_EncodedBy, MVT_String },
	{ "\251cmt", Frame::FT_Comment, MVT_String },
	{ "\251gen", Frame::FT_Genre, MVT_String },
	{ "trkn", Frame::FT_Track, MVT_IntPair },
	{ "disk", Frame::FT_Disc, MVT_IntPair },
	{ "gnre", Frame::FT_Genre, MVT_String },
	{ "cpil", Frame::FT_Other, MVT_Bool },
	{ "tmpo", Frame::FT_Bpm, MVT_Int },
	{ "\251grp", Frame::FT_Grouping, MVT_String },
	{ "aART", Frame::FT_AlbumArtist, MVT_String },
	{ "pgap", Frame::FT_Other, MVT_Bool },
	{ "cprt", Frame::FT_Copyright, MVT_String },
	{ "\251lyr", Frame::FT_Lyrics, MVT_String },
	{ "tvsh", Frame::FT_Other, MVT_String },
	{ "tvnn", Frame::FT_Other, MVT_String },
	{ "tven", Frame::FT_Other, MVT_String },
	{ "tvsn", Frame::FT_Other, MVT_ByteArray },
	{ "tves", Frame::FT_Other, MVT_ByteArray },
	{ "desc", Frame::FT_Other, MVT_String },
	{ "ldes", Frame::FT_Other, MVT_String },
	{ "sonm", Frame::FT_Other, MVT_String },
	{ "soar", Frame::FT_Other, MVT_String },
	{ "soaa", Frame::FT_Other, MVT_String },
	{ "soal", Frame::FT_Other, MVT_String },
	{ "soco", Frame::FT_Other, MVT_String },
	{ "sosn", Frame::FT_Other, MVT_String },
	{ "\251enc", Frame::FT_Other, MVT_String },
	{ "purd", Frame::FT_Other, MVT_String },
	{ "pcst", Frame::FT_Other, MVT_Bool },
	{ "keyw", Frame::FT_Other, MVT_String },
	{ "catg", Frame::FT_Other, MVT_String },
	{ "hdvd", Frame::FT_Other, MVT_ByteArray },
	{ "stik", Frame::FT_Other, MVT_ByteArray },
	{ "rtng", Frame::FT_Other, MVT_ByteArray },
	{ "apID", Frame::FT_Other, MVT_String },
	{ "akID", Frame::FT_Other, MVT_ByteArray },
	{ "sfID", Frame::FT_Other, MVT_ByteArray },
	{ "cnID", Frame::FT_Other, MVT_ByteArray },
	{ "atID", Frame::FT_Other, MVT_ByteArray },
	{ "plID", Frame::FT_Other, MVT_ByteArray },
	{ "geID", Frame::FT_Other, MVT_ByteArray },
#if TAGLIB_VERSION >= 0x010602
	{ "covr", Frame::FT_Picture, MVT_CoverArt },
#else
	{ "covr", Frame::FT_Picture, MVT_ByteArray },
#endif
	{ "ARRANGER", Frame::FT_Arranger, MVT_String },
	{ "AUTHOR", Frame::FT_Author, MVT_String },
	{ "CONDUCTOR", Frame::FT_Conductor, MVT_String },
	{ "ISRC", Frame::FT_Isrc, MVT_String },
	{ "LANGUAGE", Frame::FT_Language, MVT_String },
	{ "LYRICIST", Frame::FT_Lyricist, MVT_String },
	{ "SOURCEMEDIA", Frame::FT_Media, MVT_String },
	{ "ORIGINALALBUM", Frame::FT_OriginalAlbum, MVT_String },
	{ "ORIGINALARTIST", Frame::FT_OriginalArtist, MVT_String },
	{ "ORIGINALDATE", Frame::FT_OriginalDate, MVT_String },
	{ "PART", Frame::FT_Part, MVT_String },
	{ "PERFORMER", Frame::FT_Performer, MVT_String },
	{ "PUBLISHER", Frame::FT_Publisher, MVT_String },
	{ "REMIXER", Frame::FT_Remixer, MVT_String },
	{ "SUBTITLE", Frame::FT_Subtitle, MVT_String },
	{ "WEBSITE", Frame::FT_Website, MVT_String }
};

/**
 * Get MP4 name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
static void getMp4NameForType(Frame::Type type, TagLib::String& name,
                              Mp4ValueType& value)
{
	static QMap<Frame::Type, unsigned> typeNameMap;
	if (typeNameMap.empty()) {
		// first time initialization
		for (unsigned i = 0;
		     i < sizeof(mp4NameTypeValues) / sizeof(mp4NameTypeValues[0]); ++i) {
			if (mp4NameTypeValues[i].type != Frame::FT_Other) {
				typeNameMap.insert(mp4NameTypeValues[i].type, i);
			}
		}
	}
	name = "";
	value = MVT_String;
	if (type != Frame::FT_Other) {
		QMap<Frame::Type, unsigned>::const_iterator it = typeNameMap.find(type);
		if (it != typeNameMap.end()) {
			name = mp4NameTypeValues[*it].name;
			value = mp4NameTypeValues[*it].value;
		}
	}
}

/**
 * Get MP4 value type and frame type for an MP4 name.
 *
 * @param name  MP4 name
 * @param type  the frame type is returned here
 * @param value the MP4 value type is returned here
 *
 * @return true if free-form frame.
 */
static bool getMp4TypeForName(const TagLib::String& name, Frame::Type& type,
                              Mp4ValueType& value)
{
	static QMap<TagLib::String, unsigned> nameTypeMap;
	if (nameTypeMap.empty()) {
		// first time initialization
		for (unsigned i = 0;
		     i < sizeof(mp4NameTypeValues) / sizeof(mp4NameTypeValues[0]); ++i) {
			nameTypeMap.insert(mp4NameTypeValues[i].name, i);
		}
	}
	QMap<TagLib::String, unsigned>::const_iterator it = nameTypeMap.find(name);
	if (it != nameTypeMap.end()) {
		type = mp4NameTypeValues[*it].type;
		value = mp4NameTypeValues[*it].value;
		return name[0] >= 'A' && name[0] <= 'Z';
	} else {
		type = Frame::FT_Other;
		value = MVT_String;
		return true;
	}
}

/**
 * Strip free form prefix from MP4 frame name.
 *
 * @param name MP4 frame name to be stripped
 */
static void stripMp4FreeFormName(TagLib::String& name)
{
	if (name.startsWith("----")) {
		int nameStart = name.rfind(":");
		if (nameStart == -1) { 
			nameStart = 5;
		} else {
			++nameStart;
		}
		name = name.substr(nameStart);

		Frame::Type type;
		Mp4ValueType valueType;
		if (!getMp4TypeForName(name, type, valueType)) {
			// not detected as free form => mark with ':' as first character
			name = ':' + name;
		}
	}
}

/**
 * Prepend free form prefix to MP4 frame name.
 * Only names starting with a capital letter or ':' are prefixed.
 *
 * @param name MP4 frame name to be prefixed.
 */
static void prefixMp4FreeFormName(TagLib::String& name)
{
	if (!name.startsWith("----")) {
		Frame::Type type;
		Mp4ValueType valueType;
		if (getMp4TypeForName(name, type, valueType)) {
			// free form
			if (name[0] == ':') name = name.substr(1);
			name = "----:com.apple.iTunes:" + name;
		}
	}
}

/**
 * Get an MP4 type for a frame.
 *
 * @param frame frame
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
static void getMp4TypeForFrame(const Frame& frame, TagLib::String& name,
                               Mp4ValueType& value)
{
	if (frame.getType() != Frame::FT_Other) {
		getMp4NameForType(frame.getType(), name, value);
		if (name.isEmpty()) {
			name = QSTRING_TO_TSTRING(frame.getName(true));
		}
	} else {
		Frame::Type type;
		name = QSTRING_TO_TSTRING(frame.getName(true));
		getMp4TypeForName(name, type, value);
	}
}

/**
 * Get an MP4 item for a frame.
 *
 * @param frame frame
 * @param name  the name for the item is returned here
 *
 * @return MP4 item, an invalid item is returned if not supported.
 */
static TagLib::MP4::Item getMp4ItemForFrame(const Frame& frame, TagLib::String& name)
{
	Mp4ValueType valueType;
	getMp4TypeForFrame(frame, name, valueType);
	prefixMp4FreeFormName(name);
	switch (valueType) {
		case MVT_String:
			return TagLib::MP4::Item(QSTRING_TO_TSTRING(frame.getValue()));
		case MVT_Bool:
			return TagLib::MP4::Item(frame.getValue().toInt() != 0);
		case MVT_Int:
			return TagLib::MP4::Item(frame.getValue().toInt());
		case MVT_IntPair:
		{
			QString str1 = frame.getValue(), str2 = "0";
			int slashPos = str1.QCM_indexOf('/');
			if (slashPos != -1) {
				str1.truncate(slashPos);
				str2 = str1.mid(slashPos + 1);
			}
			return TagLib::MP4::Item(str1.toInt(), str2.toInt());
		}
#if TAGLIB_VERSION >= 0x010602
		case MVT_CoverArt:
		{
			QByteArray ba;
			TagLib::MP4::CoverArt::Format format = TagLib::MP4::CoverArt::JPEG;
			if (PictureFrame::getData(frame, ba)) {
				QString mimeType;
				if (PictureFrame::getMimeType(frame, mimeType) && 
						mimeType == "image/png") {
					format = TagLib::MP4::CoverArt::PNG;
				}
			}
			TagLib::MP4::CoverArt coverArt(format,
																		 TagLib::ByteVector(ba.data(), ba.size()));
			TagLib::MP4::CoverArtList coverArtList;
			coverArtList.append(coverArt);
			return TagLib::MP4::Item(coverArtList);
		}
#endif
		case MVT_ByteArray:
		default:
			// binary data and album art are not handled by TagLib
			return TagLib::MP4::Item();
	}
}
#endif

#ifdef TAGLIB_WITH_ASF
/** Indices of fixed ASF frames. */
enum AsfFrameIndex {
	AFI_Title,
	AFI_Artist,
	AFI_Comment,
	AFI_Copyright,
	AFI_Rating,
	AFI_Attributes
};

/** ASF name, frame type and value type. */
struct AsfNameTypeValue {
	const char* name;
	Frame::Type type;
	TagLib::ASF::Attribute::AttributeTypes value;
};

/** Mapping between frame types and field names. */
static const AsfNameTypeValue asfNameTypeValues[] = {
	{ "Title", Frame::FT_Title, TagLib::ASF::Attribute::UnicodeType },
	{ "Author", Frame::FT_Artist, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/AlbumTitle", Frame::FT_Album, TagLib::ASF::Attribute::UnicodeType },
	{ "Description", Frame::FT_Comment, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Year", Frame::FT_Date, TagLib::ASF::Attribute::UnicodeType },
	{ "Copyright", Frame::FT_Copyright, TagLib::ASF::Attribute::UnicodeType },
	{ "Rating", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/TrackNumber", Frame::FT_Track, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Track", Frame::FT_Track, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Genre", Frame::FT_Genre, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/GenreID", Frame::FT_Genre, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/AlbumArtist", Frame::FT_AlbumArtist, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Producer", Frame::FT_Arranger, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/BeatsPerMinute", Frame::FT_Bpm, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Composer", Frame::FT_Composer, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Conductor", Frame::FT_Conductor, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/PartOfSet", Frame::FT_Disc, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/EncodedBy", Frame::FT_EncodedBy, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/ContentGroupDescription", Frame::FT_Grouping, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/ISRC", Frame::FT_Isrc, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Language", Frame::FT_Language, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Writer", Frame::FT_Lyricist, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Lyrics", Frame::FT_Lyrics, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/AudioSourceURL", Frame::FT_Media, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/OriginalAlbumTitle", Frame::FT_OriginalAlbum, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/OriginalArtist", Frame::FT_OriginalArtist, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/OriginalReleaseYear", Frame::FT_OriginalDate, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/SubTitleDescription", Frame::FT_Part, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/Picture", Frame::FT_Picture, TagLib::ASF::Attribute::BytesType },
	{ "WM/Publisher", Frame::FT_Publisher, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/ModifiedBy", Frame::FT_Remixer, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/SubTitle", Frame::FT_Subtitle, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/AuthorURL", Frame::FT_Website, TagLib::ASF::Attribute::UnicodeType },
	{ "AverageLevel", Frame::FT_Other, TagLib::ASF::Attribute::DWordType },
	{ "PeakValue", Frame::FT_Other, TagLib::ASF::Attribute::DWordType },
	{ "WM/AudioFileURL", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/EncodingSettings", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/EncodingTime", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
	{ "WM/InitialKey", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	// incorrect WM/Lyrics_Synchronised data make file inaccessible in Windows
	// { "WM/Lyrics_Synchronised", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
	{ "WM/MCDI", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
	{ "WM/MediaClassPrimaryID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
	{ "WM/MediaClassSecondaryID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
	{ "WM/Mood", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/OriginalFilename", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/OriginalLyricist", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/PromotionURL", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/SharedUserRating", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
	{ "WM/WMCollectionGroupID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
	{ "WM/WMCollectionID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
	{ "WM/WMContentID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType }
};

/**
 * Get ASF name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the ASF name is returned here
 * @param value the ASF value type is returned here
 */
static void getAsfNameForType(Frame::Type type, TagLib::String& name,
                              TagLib::ASF::Attribute::AttributeTypes& value)
{
	static QMap<Frame::Type, unsigned> typeNameMap;
	if (typeNameMap.empty()) {
		// first time initialization
		for (unsigned i = 0;
		     i < sizeof(asfNameTypeValues) / sizeof(asfNameTypeValues[0]); ++i) {
			if (asfNameTypeValues[i].type != Frame::FT_Other &&
					!typeNameMap.contains(asfNameTypeValues[i].type)) {
				typeNameMap.insert(asfNameTypeValues[i].type, i);
			}
		}
	}
	name = "";
	value = TagLib::ASF::Attribute::UnicodeType;
	if (type != Frame::FT_Other) {
		QMap<Frame::Type, unsigned>::const_iterator it = typeNameMap.find(type);
		if (it != typeNameMap.end()) {
			name = asfNameTypeValues[*it].name;
			value = asfNameTypeValues[*it].value;
		}
	}
}

/**
 * Get ASF value type and frame type for an ASF name.
 *
 * @param name  ASF name
 * @param type  the frame type is returned here
 * @param value the ASF value type is returned here
 */
static void getAsfTypeForName(const TagLib::String& name, Frame::Type& type,
															TagLib::ASF::Attribute::AttributeTypes& value)
{
	static QMap<TagLib::String, unsigned> nameTypeMap;
	if (nameTypeMap.empty()) {
		// first time initialization
		for (unsigned i = 0;
		     i < sizeof(asfNameTypeValues) / sizeof(asfNameTypeValues[0]); ++i) {
			nameTypeMap.insert(asfNameTypeValues[i].name, i);
		}
	}
	QMap<TagLib::String, unsigned>::const_iterator it = nameTypeMap.find(name);
	if (it != nameTypeMap.end()) {
		type = asfNameTypeValues[*it].type;
		value = asfNameTypeValues[*it].value;
	} else {
		type = Frame::FT_Other;
		value = TagLib::ASF::Attribute::UnicodeType;
	}
}

/**
 * Get an ASF type for a frame.
 *
 * @param frame frame
 * @param name  the name for the attribute is returned here
 * @param value the ASF value type is returned here
 */
static void getAsfTypeForFrame(const Frame& frame, TagLib::String& name,
															 TagLib::ASF::Attribute::AttributeTypes& value)
{
	if (frame.getType() != Frame::FT_Other) {
		getAsfNameForType(frame.getType(), name, value);
		if (name.isEmpty()) {
			name = QSTRING_TO_TSTRING(frame.getName(true));
		}
	} else {
		Frame::Type type;
		name = QSTRING_TO_TSTRING(frame.getName(true));
		getAsfTypeForName(name, type, value);
	}
}

#if TAGLIB_VERSION >= 0x010602
/**
 * Get a picture frame from the bytes in a WM/Picture frame.
 * The WM/Picture frame has the following data:
 * 1 byte picture type, 4 bytes (little endian) size of picture data,
 * UTF16_LE mime type, UTF16_LE description, picture data.
 *
 * @param data bytes in WM/Picture frame
 * @param frame the picture frame is returned here
 *
 * @return true if ok.
 */
static bool parseAsfPicture(const TagLib::ByteVector& data, Frame& frame)
{
	uint len = data.size();
	if (len < 10)
		return false;
	char pictureType = data[0];
	if (pictureType < 0 || pictureType > 20)
		return false;
	uint picSize =
		 ((unsigned char)data[1] & 0xff)        |
		(((unsigned char)data[2] & 0xff) << 8)  |
		(((unsigned char)data[3] & 0xff) << 16) |
		(((unsigned char)data[4] & 0xff) << 24);
	if (picSize > len - 9)
		return false;
	uint offset = 5;
#if QT_VERSION >= 0x040000
	QString mimeType = QString::fromUtf16(
		reinterpret_cast<const ushort*>(data.mid(offset, len - offset).data()));
	offset += mimeType.length() * 2 + 2;
	if (offset >= len - 1)
		return false;
	QString description = QString::fromUtf16(
		reinterpret_cast<const ushort*>(data.mid(offset, len - offset).data()));
	offset += description.length() * 2 + 2;
	if (offset > len)
		return false;
	TagLib::ByteVector picture = data.mid(offset, len - offset);
	PictureFrame::setFields(frame, Frame::Field::TE_ISO8859_1, "JPG",
													mimeType,
													static_cast<PictureFrame::PictureType>(pictureType),
													description,
													QByteArray(picture.data(), picture.size()));
#else
	QString mimeType, description;
	const ushort* unicode = reinterpret_cast<const ushort*>(data.data() + offset);
	uint unicodeLen = 0;
	while (unicode[unicodeLen] != 0) {
		++unicodeLen;
		if (unicodeLen > (len - offset) / 2 - 2) {
			return false;
		}
	}
	mimeType.setUnicodeCodes(unicode, unicodeLen);
	offset += unicodeLen * 2 + 2;
	unicode += unicodeLen + 1;
	unicodeLen = 0;
	while (unicode[unicodeLen] != 0) {
		++unicodeLen;
		if (unicodeLen > (len - offset) / 2) {
			return false;
		}
	}
	description.setUnicodeCodes(unicode, unicodeLen);
	if (description.isNull()) {
		description = "";
	}
	offset += unicodeLen * 2 + 2;
	QByteArray picture;
	picture.duplicate(data.data() + offset, len - offset);
	PictureFrame::setFields(frame, Frame::Field::TE_ISO8859_1, "JPG",
													mimeType,
													static_cast<PictureFrame::PictureType>(pictureType),
													description,
													picture);
#endif
	frame.setType(Frame::FT_Picture);
	return true;
}

/**
 * Render the bytes of a WM/Picture frame from a picture frame.
 *
 * @param frame picture frame
 * @param data  the bytes for the WM/Picture are returned here
 */
static void renderAsfPicture(const Frame& frame, TagLib::ByteVector& data)
{
	Frame::Field::TextEncoding enc;
	PictureFrame::PictureType pictureType;
	QByteArray picture;
	QString imgFormat, mimeType, description;
	PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
	                        description, picture);

	data.resize(5);
	data[0] = pictureType;
	uint size = picture.size();
	data[1] = size & 0xff;
	size >>= 8;
	data[2] = size & 0xff;
	size >>= 8;
	data[3] = size & 0xff;
	size >>= 8;
	data[4] = size & 0xff;
	data.append(QSTRING_TO_TSTRING(mimeType).data(TagLib::String::UTF16LE));
	data.append(TagLib::ByteVector(2, 0));
	data.append(QSTRING_TO_TSTRING(description).data(TagLib::String::UTF16LE));
	data.append(TagLib::ByteVector(2, 0));
	data.append(TagLib::ByteVector(picture.data(), picture.size()));
}
#endif

/**
 * Get an ASF attribute for a frame.
 *
 * @param frame     frame
 * @param valueType ASF value type
 *
 * @return ASF attribute, an empty attribute is returned if not supported.
 */
static TagLib::ASF::Attribute getAsfAttributeForFrame(
	const Frame& frame,
	TagLib::ASF::Attribute::AttributeTypes valueType)
{
	switch (valueType) {
		case TagLib::ASF::Attribute::UnicodeType:
			return TagLib::ASF::Attribute(QSTRING_TO_TSTRING(frame.getValue()));
		case TagLib::ASF::Attribute::BoolType:
			return TagLib::ASF::Attribute(frame.getValue() == "1");
		case TagLib::ASF::Attribute::WordType:
			return TagLib::ASF::Attribute(frame.getValue().toUShort());
		case TagLib::ASF::Attribute::DWordType:
			return TagLib::ASF::Attribute(frame.getValue().toUInt());
		case TagLib::ASF::Attribute::QWordType:
			return TagLib::ASF::Attribute(frame.getValue().toULongLong());
		case TagLib::ASF::Attribute::BytesType:
		case TagLib::ASF::Attribute::GuidType:
		default:
			if (frame.getType() != Frame::FT_Picture) {
				QByteArray ba;
				if (AttributeData(frame.getName(true)).toByteArray(frame.getValue(), ba)) {
					return TagLib::ASF::Attribute(TagLib::ByteVector(ba.data(), ba.size()));
				}
				QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Data);
				if (fieldValue.isValid()) {
					ba = fieldValue.toByteArray();
					return TagLib::ASF::Attribute(TagLib::ByteVector(ba.data(), ba.size()));
				}
			}
#if TAGLIB_VERSION >= 0x010602
			else {
				TagLib::ByteVector bv;
				renderAsfPicture(frame, bv); 
				return TagLib::ASF::Attribute(bv);
			}
#endif
	}
	return TagLib::ASF::Attribute();
}
#endif

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
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
		TagLib::ASF::Tag* asfTag;
#endif
#endif
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (index < static_cast<int>(frameList.size())) {
				// This is a hack. The frameList should not be modified directly.
				// However when removing the old frame and adding a new frame,
				// the indices of all frames get invalid.
				setId3v2Frame(frameList[index], frame);
				markTag2Changed(frame.getType());
				return true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			if (frame.getType() == Frame::FT_Picture) {
				return false;
			}
			TagLib::String key = QSTRING_TO_TSTRING(getVorbisName(frame));
			TagLib::String value = QSTRING_TO_TSTRING(frame.getValue());
#if TAGLIB_VERSION <= 0x010400
			// Remove all fields with that key, because TagLib <= 1.4 crashes
			// using an invalidated iterator after calling erase().
			oggTag->addField(key, value, true);
#else
			const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
			if (fieldListMap.contains(key) && fieldListMap[key].size() > 1) {
				int i = 0;
				TagLib::String oldValue(TagLib::String::null);
				for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
						 it != fieldListMap.end();
						 ++it) {
					TagLib::StringList stringList = (*it).second;
					for (TagLib::StringList::ConstIterator slit = stringList.begin();
							 slit != stringList.end();
							 ++slit) {
						if (i++ == index) {
							oldValue = *slit;
							break;
						}
					}
				}
				oggTag->removeField(key, oldValue);
				oggTag->addField(key, value, false);
			} else {
				oggTag->addField(key, value, true);
			}
#endif
			if (frame.getType() == Frame::FT_Track) {
				int numTracks = getTotalNumberOfTracksIfEnabled();
				if (numTracks > 0) {
					oggTag->addField("TRACKTOTAL", TagLib::String::number(numTracks), true);
				}
			}
			markTag2Changed(frame.getType());
			return true;
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			if (frame.getType() == Frame::FT_Picture) {
				return false;
			}
			apeTag->addValue(QSTRING_TO_TSTRING(getApeName(frame)),
											 QSTRING_TO_TSTRING(frame.getValue()));
			markTag2Changed(frame.getType());
			return true;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
			TagLib::String name;
			TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
			if (item.isValid()) {
				mp4Tag->itemListMap()[name] = item;
				markTag2Changed(frame.getType());
			}
			return true;
#endif
#ifdef TAGLIB_WITH_ASF
		} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
#if !(TAGLIB_VERSION >= 0x010602)
			if (frame.getType() == Frame::FT_Picture) {
				return false;
			}
#endif
			switch (index) {
				case AFI_Title:
					asfTag->setTitle(QSTRING_TO_TSTRING(frame.getValue()));
					break;
				case AFI_Artist:
					asfTag->setArtist(QSTRING_TO_TSTRING(frame.getValue()));
					break;
				case AFI_Comment:
					asfTag->setComment(QSTRING_TO_TSTRING(frame.getValue()));
					break;
				case AFI_Copyright:
					asfTag->setCopyright(QSTRING_TO_TSTRING(frame.getValue()));
					break;
				case AFI_Rating:
					asfTag->setRating(QSTRING_TO_TSTRING(frame.getValue()));
					break;
				case AFI_Attributes:
				default:
				{
					TagLib::String name;
					TagLib::ASF::Attribute::AttributeTypes valueType;
					getAsfTypeForFrame(frame, name, valueType);
					TagLib::ASF::Attribute attribute =
						getAsfAttributeForFrame(frame, valueType);
					TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
					if (attrListMap.contains(name) && attrListMap[name].size() > 1) {
						int i = AFI_Attributes;
						bool found = false;
						for (TagLib::ASF::AttributeListMap::Iterator it = attrListMap.begin();
								 it != attrListMap.end();
								 ++it) {
							TagLib::ASF::AttributeList& attrList = (*it).second;
							for (TagLib::ASF::AttributeList::Iterator ait = attrList.begin();
									 ait != attrList.end();
									 ++ait) {
								if (i++ == index) {
									found = true;
									*ait = attribute;
									break;
								}
							}
							if (found) {
								break;
							}
						}
					} else {
						asfTag->setAttribute(name, attribute);
					}
				}
			}
			markTag2Changed(frame.getType());
			return true;
#endif
#endif
		}
	}

	// Try the superclass method
	return TaggedFile::setFrameV2(frame);
}

/**
 * Check if an ID3v2.4.0 frame ID is valid.
 *
 * @param frameId frame ID (4 characters)
 *
 * @return true if frame ID is valid.
 */
static bool isFrameIdValid(const QString& frameId)
{
	Frame::Type type;
	const char* str;
	getTypeStringForFrameId(TagLib::ByteVector(frameId.QCM_latin1(), 4), type, str);
	return type != Frame::FT_UnknownFrame;
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
	TagLib::String::Type enc = getDefaultTextEncoding();
	// Add a new frame.
	if (makeTagV2Settable()) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
		TagLib::ASF::Tag* asfTag;
#endif
#endif
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			QString name = frame.getType() != Frame::FT_Other ?
				QString(getStringForType(frame.getType())) :
				frame.getName();
			QString frameId = name;
			frameId.truncate(4);
			TagLib::ID3v2::Frame* id3Frame = 0;
			if (frameId.startsWith("T")) {
				if (frameId == "TXXX") {
					id3Frame = new TagLib::ID3v2::UserTextIdentificationFrame(enc);
				} else if (isFrameIdValid(frameId)) {
					id3Frame = new TagLib::ID3v2::TextIdentificationFrame(
						TagLib::ByteVector(frameId.QCM_latin1(), frameId.length()), enc);
					id3Frame->setText(""); // is necessary for createFrame() to work
				}
			} else if (frameId == "COMM") {
				id3Frame = new TagLib::ID3v2::CommentsFrame(enc);
			} else if (frameId == "APIC") {
				id3Frame = new TagLib::ID3v2::AttachedPictureFrame;
				if (id3Frame) {
					((TagLib::ID3v2::AttachedPictureFrame*)id3Frame)->setTextEncoding(enc);
					((TagLib::ID3v2::AttachedPictureFrame*)id3Frame)->setMimeType(
						"image/jpeg");
					((TagLib::ID3v2::AttachedPictureFrame*)id3Frame)->setType(
						TagLib::ID3v2::AttachedPictureFrame::FrontCover);
				}
			} else if (frameId == "UFID") {
				// the bytevector must not be empty
				id3Frame = new TagLib::ID3v2::UniqueFileIdentifierFrame(
					TagLib::String(), TagLib::ByteVector(" "));
			} else if (frameId == "GEOB") {
				id3Frame = new TagLib::ID3v2::GeneralEncapsulatedObjectFrame;
				if (id3Frame) {
					((TagLib::ID3v2::GeneralEncapsulatedObjectFrame*)id3Frame)->setTextEncoding(enc);
				}
			} else if (frameId.startsWith("W")) {
				if (frameId == "WXXX") {
					id3Frame = new TagLib::ID3v2::UserUrlLinkFrame(enc);
				} else if (isFrameIdValid(frameId)) {
					id3Frame = new TagLib::ID3v2::UrlLinkFrame(
						TagLib::ByteVector(frameId.QCM_latin1(), frameId.length()));
					id3Frame->setText("http://"); // is necessary for createFrame() to work
				}
			} else if (frameId == "USLT") {
				id3Frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(enc);
				if (id3Frame) {
					((TagLib::ID3v2::UnsynchronizedLyricsFrame*)id3Frame)->setLanguage("eng");
				}
			}
			if (!id3Frame) {
				TagLib::ID3v2::UserTextIdentificationFrame* txxxFrame =
					new TagLib::ID3v2::UserTextIdentificationFrame(enc);
				txxxFrame->setDescription(QSTRING_TO_TSTRING(frame.getName()));
				id3Frame = txxxFrame;
				frame.setInternalName("TXXX - User defined text information");
			} else {
				frame.setInternalName(name);
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
				frame.setIndex(id3v2Tag->frameList().size() - 1);
				if (frame.fieldList().empty()) {
					// add field list to frame
					getFieldsFromId3Frame(id3Frame, frame.fieldList(), frame.getType());
					frame.setFieldListFromValue();
				}
#ifdef WIN32
				delete id3Frame;
#endif
				markTag2Changed(frame.getType());
				return true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			if (frame.getType() == Frame::FT_Picture) {
				return false;
			}
			QString name(getVorbisName(frame));
			TagLib::String tname = QSTRING_TO_TSTRING(name);
			TagLib::String tvalue = QSTRING_TO_TSTRING(frame.getValue());
			if (tvalue.isEmpty()) {
				tvalue = " "; // empty values are not added by TagLib
			}
#if TAGLIB_VERSION <= 0x010400
			oggTag->addField(tname, tvalue);
#else
			oggTag->addField(tname, tvalue, false);
#endif
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
				index += (*it).second.size();
			}
			frame.setIndex(found ? index : -1);
			markTag2Changed(frame.getType());
			return true;
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			if (frame.getType() == Frame::FT_Picture) {
				return false;
			}
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
			markTag2Changed(frame.getType());
			return true;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
#if TAGLIB_VERSION >= 0x010602
			if (frame.getType() == Frame::FT_Picture &&
					frame.getFieldList().empty()) {
				PictureFrame::setFields(frame);
			}
#endif
			TagLib::String name;
			TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
			if (!item.isValid()) {
				return false;
			}
			frame.setInternalName(TStringToQString(name));
			prefixMp4FreeFormName(name);
			mp4Tag->itemListMap()[name] = item;
			const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
			int index = 0;
			bool found = false;
			for (TagLib::MP4::ItemListMap::ConstIterator it = itemListMap.begin();
					 it != itemListMap.end();
					 ++it) {
				if ((*it).first == name) {
					found = true;
					break;
				}
				++index;
			}
			frame.setIndex(found ? index : -1);
			markTag2Changed(frame.getType());
			return true;
#endif
#ifdef TAGLIB_WITH_ASF
		} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
#if TAGLIB_VERSION >= 0x010602
			if (frame.getType() == Frame::FT_Picture &&
					frame.getFieldList().empty()) {
				PictureFrame::setFields(frame);
			}
#else
			if (frame.getType() == Frame::FT_Picture) {
				return false;
			}
#endif
			TagLib::String name;
			TagLib::ASF::Attribute::AttributeTypes valueType;
			getAsfTypeForFrame(frame, name, valueType);
			if (valueType == TagLib::ASF::Attribute::BytesType &&
					frame.getType() != Frame::FT_Picture) {
				Frame::Field field;
				field.m_id = Frame::Field::ID_Data;
				field.m_value = QByteArray();
				frame.fieldList().push_back(field);
			}
			TagLib::ASF::Attribute attribute = getAsfAttributeForFrame(frame, valueType);
			asfTag->addAttribute(name, attribute);
			frame.setInternalName(TStringToQString(name));

			const TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
			int index = AFI_Attributes;
			bool found = false;
			for (TagLib::ASF::AttributeListMap::ConstIterator it = attrListMap.begin();
					 it != attrListMap.end();
					 ++it) {
				if ((*it).first == name) {
					index += (*it).second.size() - 1;
					found = true;
					break;
				}
				index += (*it).second.size();
			}
			frame.setIndex(found ? index : -1);
			markTag2Changed(frame.getType());
			return true;
#endif
#endif
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
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
		TagLib::ASF::Tag* asfTag;
#endif
#endif
		if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
			const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
			if (index < static_cast<int>(frameList.size())) {
				id3v2Tag->removeFrame(frameList[index]);
				markTag2Changed(frame.getType());
				return true;
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			TagLib::String key =
				QSTRING_TO_TSTRING(frame.getName(true));
#if TAGLIB_VERSION <= 0x010400
			// Remove all fields with that key, because TagLib <= 1.4 crashes
			// using an invalidated iterator after calling erase().
			oggTag->removeField(key);
#else
			oggTag->removeField(key, QSTRING_TO_TSTRING(frame.getValue()));
#endif
			markTag2Changed(frame.getType());
			return true;
		} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
			TagLib::String key = QSTRING_TO_TSTRING(frame.getName(true));
			apeTag->removeItem(key);
			markTag2Changed(frame.getType());
			return true;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
			TagLib::String name = QSTRING_TO_TSTRING(frame.getName(true));
			prefixMp4FreeFormName(name);
			mp4Tag->itemListMap().erase(name);
			markTag2Changed(frame.getType());
			return true;
#endif
#ifdef TAGLIB_WITH_ASF
		} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
			switch (index) {
				case AFI_Title:
					asfTag->setTitle("");
					break;
				case AFI_Artist:
					asfTag->setArtist("");
					break;
				case AFI_Comment:
					asfTag->setComment("");
					break;
				case AFI_Copyright:
					asfTag->setCopyright("");
					break;
				case AFI_Rating:
					asfTag->setRating("");
					break;
				case AFI_Attributes:
				default:
				{
					TagLib::String name = QSTRING_TO_TSTRING(frame.getName(true));
					TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
					if (attrListMap.contains(name) && attrListMap[name].size() > 1) {
						int i = AFI_Attributes;
						bool found = false;
						for (TagLib::ASF::AttributeListMap::Iterator it = attrListMap.begin();
								 it != attrListMap.end();
								 ++it) {
							TagLib::ASF::AttributeList& attrList = (*it).second;
							for (TagLib::ASF::AttributeList::Iterator ait = attrList.begin();
									 ait != attrList.end();
									 ++ait) {
								if (i++ == index) {
									found = true;
									attrList.erase(ait);
									break;
								}
							}
							if (found) {
								break;
							}
						}
					} else {
						asfTag->removeItem(name);
					}
				}
			}
			markTag2Changed(frame.getType());
			return true;
#endif
#endif
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
void TagLibFile::deleteFramesV2(const FrameFilter& flt)
{
	if (m_tagV2) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
		TagLib::ASF::Tag* asfTag;
#endif
#endif
		if (flt.areAllEnabled()) {
			if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
				const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
				for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
						 it != frameList.end();) {
					id3v2Tag->removeFrame(*it++, true);
				}
				markTag2Changed(Frame::FT_UnknownFrame);
			} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) !=
								 0) {
				const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
				for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
						 it != fieldListMap.end();) {
					oggTag->removeField((*it++).first);
				}
				markTag2Changed(Frame::FT_UnknownFrame);
			} else if ((apeTag = dynamic_cast<TagLib::APE::Tag*>(m_tagV2)) != 0) {
				const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
				for (TagLib::APE::ItemListMap::ConstIterator it = itemListMap.begin();
						 it != itemListMap.end();) {
					apeTag->removeItem((*it++).first);
				}
				markTag2Changed(Frame::FT_UnknownFrame);
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
			} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
				mp4Tag->itemListMap().clear();
				markTag2Changed(Frame::FT_UnknownFrame);
#endif
#ifdef TAGLIB_WITH_ASF
			} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
				asfTag->setTitle("");
				asfTag->setArtist("");
				asfTag->setComment("");
				asfTag->setCopyright("");
				asfTag->setRating("");
				asfTag->attributeListMap().clear();
				markTag2Changed(Frame::FT_UnknownFrame);
#endif
#endif
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
				markTag2Changed(Frame::FT_UnknownFrame);
			} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) !=
								 0) {
				const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
				for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
						 it != fieldListMap.end();) {
					QString name(TStringToQString((*it).first));
					if (flt.isEnabled(getTypeFromVorbisName(name), name)) {
						oggTag->removeField((*it++).first);
					} else {
						++it;
					}
				}
				markTag2Changed(Frame::FT_UnknownFrame);
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
				markTag2Changed(Frame::FT_UnknownFrame);
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
			} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
				TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
				Frame::Type type;
				Mp4ValueType valueType;
				for (TagLib::MP4::ItemListMap::Iterator it = itemListMap.begin();
						 it != itemListMap.end();) {
					getMp4TypeForName((*it).first, type, valueType);
					QString name(TStringToQString((*it).first));
					if (flt.isEnabled(type, name)) {
						itemListMap.erase(it++);
					} else {
						++it;
					}
				}
				markTag2Changed(Frame::FT_UnknownFrame);
#endif
#ifdef TAGLIB_WITH_ASF
			} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
				if (flt.isEnabled(Frame::FT_Title))
					asfTag->setTitle("");
				if (flt.isEnabled(Frame::FT_Artist))
					asfTag->setArtist("");
				if (flt.isEnabled(Frame::FT_Comment))
					asfTag->setComment("");
				if (flt.isEnabled(Frame::FT_Copyright))
					asfTag->setCopyright("");
				if (flt.isEnabled(Frame::FT_Other, "Rating"))
					asfTag->setRating("");

				TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
				Frame::Type type;
				TagLib::ASF::Attribute::AttributeTypes valueType;
				for (TagLib::ASF::AttributeListMap::Iterator it = attrListMap.begin();
						 it != attrListMap.end();) {
					getAsfTypeForName((*it).first, type, valueType);
					QString name(TStringToQString((*it).first));
					if (flt.isEnabled(type, name)) {
						attrListMap.erase(it++);
					} else {
						++it;
					}
				}
				markTag2Changed(Frame::FT_UnknownFrame);
#endif
#endif
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
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
		TagLib::ASF::Tag* asfTag;
#endif
#endif
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
				if ((*it)->frameID().mid(1, 3) == "XXX" ||
						type == Frame::FT_Comment) {
					QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Description);
					if (fieldValue.isValid()) {
						QString description = fieldValue.toString();
						if (!description.isEmpty()) {
							if (description.startsWith("QuodLibet::")) {
								// remove ExFalso/QuodLibet "namespace"
								description = description.mid(11);
							}
							frame.setInternalName(QString(name) + '\n' + description);
							frame.setType(Frame::FT_Other);
						}
					}
#if TAGLIB_VERSION >= 0x010600
				} else if ((*it)->frameID().startsWith("PRIV")) {
					QVariant fieldValue = frame.getFieldValue(Frame::Field::ID_Owner);
					if (fieldValue.isValid()) {
						QString owner = fieldValue.toString();
						if (!owner.isEmpty()) {
							frame.setInternalName(QString(name) + '\n' + owner);
						}
					}
#endif
				}
				frames.insert(frame);
			}
		} else if ((oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(m_tagV2)) != 0) {
			const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
			int i = 0;
			for (TagLib::Ogg::FieldListMap::ConstIterator it = fieldListMap.begin();
					 it != fieldListMap.end();
					 ++it) {
				QString name = TStringToQString((*it).first);
				Frame::Type type = getTypeFromVorbisName(name);
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
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
			const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
			int i = 0;
			for (TagLib::MP4::ItemListMap::ConstIterator it = itemListMap.begin();
					 it != itemListMap.end();
					 ++it) {
				TagLib::String name = (*it).first;
				stripMp4FreeFormName(name);
				Frame::Type type;
				Mp4ValueType valueType;
				getMp4TypeForName(name, type, valueType);
				QString value;
				bool frameAlreadyInserted = false;
				switch (valueType) {
					case MVT_String:
					{
						TagLib::StringList strings = (*it).second.toStringList();
						value = strings.size() > 0 ? TStringToQString(strings.front()) : "";
						break;
					}
					case MVT_Bool:
						value = (*it).second.toBool() ? "1" : "0";
						break;
					case MVT_Int:
						value.setNum((*it).second.toInt());
						break;
					case MVT_IntPair:
					{
						TagLib::MP4::Item::IntPair intPair = (*it).second.toIntPair();
						value.setNum(intPair.first);
						if (intPair.second != 0) {
							value += '/';
							value += QString::number(intPair.second);
						}
						break;
					}
#if TAGLIB_VERSION >= 0x010602
					case MVT_CoverArt:
					{
						TagLib::MP4::CoverArtList coverArtList = (*it).second.toCoverArtList();
						if (coverArtList.size() > 0) {
							const TagLib::MP4::CoverArt& coverArt = coverArtList.front();
							TagLib::ByteVector bv = coverArt.data();
							Frame frame(type, "", TStringToQString(name), i++);
							QByteArray ba;
							QCM_duplicate(ba, bv.data(), bv.size());
							PictureFrame::setFields(
								frame, Frame::Field::TE_ISO8859_1,
								coverArt.format() == TagLib::MP4::CoverArt::PNG ? "PNG" : "JPG",
								coverArt.format() == TagLib::MP4::CoverArt::PNG ?
								"image/png" : "image/jpeg",
								PictureFrame::PT_CoverFront, "", ba);
							frames.insert(frame);
							frameAlreadyInserted = true;
						}
						break;
					}
#endif
					case MVT_ByteArray:
					default:
						// binary data and album art are not handled by TagLib
						value = "";
				}
				if (!frameAlreadyInserted)
					frames.insert(
						Frame(type, value, TStringToQString(name), i++));
			}
#endif
#ifdef TAGLIB_WITH_ASF
		} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
			TagLib::String name;
			TagLib::ASF::Attribute::AttributeTypes valueType;
			Frame::Type type = Frame::FT_Title;
			getAsfNameForType(type, name, valueType);
			QString value = TStringToQString(asfTag->title());
			frames.insert(Frame(type, value, TStringToQString(name), AFI_Title));

			type = Frame::FT_Artist;
			getAsfNameForType(type, name, valueType);
			value = TStringToQString(asfTag->artist());
			frames.insert(Frame(type, value, TStringToQString(name), AFI_Artist));

			type = Frame::FT_Comment;
			getAsfNameForType(type, name, valueType);
			value = TStringToQString(asfTag->comment());
			frames.insert(Frame(type, value, TStringToQString(name), AFI_Comment));

			type = Frame::FT_Copyright;
			getAsfNameForType(type, name, valueType);
			value = TStringToQString(asfTag->copyright());
			frames.insert(Frame(type, value, TStringToQString(name), AFI_Copyright));

			name = "Rating";
			getAsfTypeForName(name, type, valueType);
			value = TStringToQString(asfTag->rating());
			frames.insert(Frame(type, value, TStringToQString(name), AFI_Rating));

			int i = AFI_Attributes;
			QByteArray ba;
			const TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
			for (TagLib::ASF::AttributeListMap::ConstIterator it = attrListMap.begin();
					 it != attrListMap.end();
					 ++it) {
				name = (*it).first;
				getAsfTypeForName(name, type, valueType);
				for (TagLib::ASF::AttributeList::ConstIterator ait = (*it).second.begin();
						 ait != (*it).second.end();
						 ++ait) {
					switch ((*ait).type()) {
						case TagLib::ASF::Attribute::UnicodeType:
							value = TStringToQString((*ait).toString());
							break;
						case TagLib::ASF::Attribute::BoolType:
							value = (*ait).toBool() ? "1" : "0";
							break;
						case TagLib::ASF::Attribute::DWordType:
							value.setNum((*ait).toUInt());
							break;
						case TagLib::ASF::Attribute::QWordType:
							value.setNum((*ait).toULongLong());
							break;
						case TagLib::ASF::Attribute::WordType:
							value.setNum((*ait).toUShort());
							break;
						case TagLib::ASF::Attribute::BytesType:
						case TagLib::ASF::Attribute::GuidType:
						default:
						{
							TagLib::ByteVector bv = (*ait).toByteVector();
							QCM_duplicate(ba, bv.data(), bv.size());
							value = "";
							AttributeData(TStringToQString(name)).toString(ba, value);
						}
					}
					Frame frame(type, value, TStringToQString(name), i);
					if ((*ait).type() == TagLib::ASF::Attribute::BytesType &&
							valueType == TagLib::ASF::Attribute::BytesType) {
						Frame::Field field;
						field.m_id = Frame::Field::ID_Data;
						field.m_value = ba;
						frame.fieldList().push_back(field);
					}
#if TAGLIB_VERSION >= 0x010602
					++i;
					if (type == Frame::FT_Picture) {
						parseAsfPicture((*ait).toByteVector(), frame);
					}
					frames.insert(frame);
#else
					if (type != Frame::FT_Picture) {
						++i;
						frames.insert(frame);
					}
#endif
				}
			}
#endif
#endif
		} else {
			TaggedFile::getAllFramesV2(frames);
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
	TagLib::ID3v2::Tag* id3v2Tag;
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		TagLib::MP4::Tag* mp4Tag;
#endif
#ifdef TAGLIB_WITH_ASF
		TagLib::ASF::Tag* asfTag;
#endif
#endif
	QStringList lst;
	if ((id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(m_tagV2)) != 0) {
		for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
			lst.append(QCM_translate(Frame::getNameFromType(static_cast<Frame::Type>(k))));
		}
		for (unsigned i = 0; i < sizeof(typeStrOfId) / sizeof(typeStrOfId[0]); ++i) {
			const TypeStrOfId& ts = typeStrOfId[i];
			if (ts.type == Frame::FT_Other && ts.supported && ts.str) {
				lst.append(QCM_translate(ts.str));
			}
		}
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
	} else if ((mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(m_tagV2)) != 0) {
		TagLib::String name;
		Mp4ValueType valueType;
		Frame::Type type;
		for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
			name = "";
			type = static_cast<Frame::Type>(k);
			getMp4NameForType(type, name, valueType);
			if (!name.isEmpty() && valueType != MVT_ByteArray &&
					!(name[0] >= 'A' && name[0] <= 'Z')) {
				lst.append(QCM_translate(Frame::getNameFromType(type)));
			}
		}
		for (unsigned i = 0; i < sizeof(mp4NameTypeValues) / sizeof(mp4NameTypeValues[0]); ++i) {
			if (mp4NameTypeValues[i].type == Frame::FT_Other &&
					mp4NameTypeValues[i].value != MVT_ByteArray &&
					!(mp4NameTypeValues[i].name[0] >= 'A' &&
						mp4NameTypeValues[i].name[0] <= 'Z')) {
				lst.append(mp4NameTypeValues[i].name);
			}
		}
#endif
#ifdef TAGLIB_WITH_ASF
	} else if ((asfTag = dynamic_cast<TagLib::ASF::Tag*>(m_tagV2)) != 0) {
		TagLib::String name;
		TagLib::ASF::Attribute::AttributeTypes valueType;
		Frame::Type type;
		for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
			name = "";
			type = static_cast<Frame::Type>(k);
			getAsfNameForType(type, name, valueType);
			if (!name.isEmpty()
#if !(TAGLIB_VERSION >= 0x010602)
					&& type != Frame::FT_Picture
#endif
				) {
				lst.append(QCM_translate(Frame::getNameFromType(type)));
			}
		}
		for (unsigned i = 0; i < sizeof(asfNameTypeValues) / sizeof(asfNameTypeValues[0]); ++i) {
			if (asfNameTypeValues[i].type == Frame::FT_Other) {
				lst.append(asfNameTypeValues[i].name);
			}
		}
#endif
#endif
	} else {
		static const char* const fieldNames[] = {
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
			"SOURCE ARTIST",
			"SOURCE MEDIUM",
			"SOURCE WORK",
			"SPARS",
			"TRACKTOTAL",
			"VERSION",
			"VOLUME"
		};
		for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
			if (k != Frame::FT_Picture) {
				lst.append(QCM_translate(Frame::getNameFromType(static_cast<Frame::Type>(k))));
			}
		}
		for (unsigned i = 0; i < sizeof(fieldNames) / sizeof(fieldNames[0]); ++i) {
			lst.append(fieldNames[i]);
		}
	}
	return lst;
}

/**
 * Static initialization.
 * Registers file types.
 */
void TagLibFile::staticInit()
{
#if TAGLIB_VERSION <= 0x010400
	TagLib::FileRef::addFileTypeResolver(new SpeexFileTypeResolver);
	TagLib::FileRef::addFileTypeResolver(new WavPackFileTypeResolver);
	TagLib::FileRef::addFileTypeResolver(new TTAFileTypeResolver);
#endif
	TagLib::FileRef::addFileTypeResolver(new AACFileTypeResolver);
	TagLib::FileRef::addFileTypeResolver(new MP2FileTypeResolver);

	TagLib::ID3v1::Tag::setStringHandler(new TextCodecStringHandler);
}

/**
 * Set the text codec to be used for tag 1.
 *
 * @param codec text codec, 0 to use default (ISO 8859-1)
 */
void TagLibFile::setTextCodecV1(const QTextCodec* codec)
{
	TextCodecStringHandler::setTextCodec(codec);
}

/**
 * Set the default text encoding.
 *
 * @param textEnc default text encoding
 */
void TagLibFile::setDefaultTextEncoding(MiscConfig::TextEncoding textEnc)
{
	// Do not use TagLib::ID3v2::FrameFactory::setDefaultTextEncoding(),
	// it will change the encoding of existing frames read in, not only
	// of newly created frames, which is really not what we want!
	switch (textEnc) {
		case MiscConfig::TE_ISO8859_1:
			s_defaultTextEncoding = TagLib::String::Latin1;
			break;
		case MiscConfig::TE_UTF16:
			s_defaultTextEncoding = TagLib::String::UTF16;
			break;
		case MiscConfig::TE_UTF8:
		default:
			s_defaultTextEncoding = TagLib::String::UTF8;
	}
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
	if (((ext == ".mp3" || ext == ".mp2" || ext == ".aac")
#ifdef HAVE_ID3LIB
			 && Kid3App::s_miscCfg.m_id3v2Version == MiscConfig::ID3v2_4_0
#endif
				)
			|| ext == ".mpc" || ext == ".oga" || ext == ".ogg" || ext == "flac"
			|| ext == ".spx" || ext == ".tta"
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
			|| ext == ".m4a" || ext ==  ".m4b" || ext ==  ".m4p" || ext ==  ".mp4"
#endif
#ifdef TAGLIB_WITH_ASF
			|| ext == ".wma" || ext ==  ".asf"
#endif
			|| ext == ".aif" || ext ==  "aiff" || ext ==  ".wav"
#endif
#if TAGLIB_VERSION > 0x010603
			|| ext == ".ape"
#endif
			|| ext.right(3) == ".wv")
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
	return QStringList() << ".flac" << ".mp3" << ".mpc" << ".oga" << ".ogg" <<
		".spx" << ".tta" << ".aac" << ".mp2" <<
#if TAGLIB_VERSION >= 0x010600
#ifdef TAGLIB_WITH_MP4
		".m4a" << ".m4b" << ".m4p" << ".mp4" <<
#endif
#ifdef TAGLIB_WITH_ASF
		".wma" << ".asf" <<
#endif
		".aif" << ".aiff" << ".wav" <<
#endif
#if TAGLIB_VERSION > 0x010603
		".ape" <<
#endif
		".wv";
}

#endif // HAVE_TAGLIB
