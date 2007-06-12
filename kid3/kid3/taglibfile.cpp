/**
 * \file taglibfile.cpp
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 */

#include "taglibfile.h"
#ifdef HAVE_TAGLIB

#include <qdir.h>
#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3CString>
#endif

#include "standardtags.h"
#include "taglibframelist.h"
#include "genres.h"
#include "dirinfo.h"
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
	Q3CString fn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());

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
 * Remove all ID3v1 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void TagLibFile::removeTagsV1(const StandardTagsFilter& flt)
{
	if (m_tagV1) {
		removeStandardTagsV1(flt);
	}
}

/**
 * Remove all ID3v2 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void TagLibFile::removeTagsV2(const StandardTagsFilter& flt)
{
	if (m_tagV2) {
		TagLib::ID3v2::Tag* id3v2Tag;
		TagLib::Ogg::XiphComment* oggTag;
		TagLib::APE::Tag* apeTag;
		if (flt.areAllTrue()) {
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
				removeStandardTagsV2(flt);
			}
		} else {
			removeStandardTagsV2(flt);
		}
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
		if (qs[0] == '(' && (cpPos = qs.find(')', 2)) > 1) {
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV1->title())) {
			QString s = checkTruncation(str, StandardTags::TF_Title);
			if (!s.isNull())
				m_tagV1->setTitle(QStringToTString(s));
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV1->artist())) {
			QString s = checkTruncation(str, StandardTags::TF_Artist);
			if (!s.isNull())
				m_tagV1->setArtist(QStringToTString(s));
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV1->album())) {
			QString s = checkTruncation(str, StandardTags::TF_Album);
			if (!s.isNull())
				m_tagV1->setAlbum(QStringToTString(s));
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV1->comment())) {
			QString s = checkTruncation(str, StandardTags::TF_Comment, 28);
			if (!s.isNull())
				m_tagV1->setComment(QStringToTString(s));
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
			TagLib::String::null : QStringToTString(str);
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
		bool needsUnicode = false;
		uint unicodeSize = qstr.length();
		const QChar* qcarray = qstr.unicode();
		for (uint i = 0; i < unicodeSize; ++i) {
			if (qcarray[i].latin1() == 0) {
				needsUnicode = true;
				break;
			}
		}
		if (needsUnicode) {
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
			TagLib::String::null : QStringToTString(str);
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV2->artist())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "TPE1")) {
			}
			m_tagV2->setArtist(tstr);
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV2->album())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "TALB")) {
			}
			m_tagV2->setAlbum(tstr);
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
			TagLib::String::null : QStringToTString(str);
		if (!(tstr == m_tagV2->comment())) {
			if (!setId3v2Unicode(m_tagV2, str, tstr, "COMM")) {
			}
			m_tagV2->setComment(tstr);
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
			TagLib::String::null : QStringToTString(str);
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

/** Frame list for MP3 files. */
TagLibFrameList* TagLibFile::s_tagLibFrameList = 0;

/**
 * Get frame list for this type of tagged file.
 *
 * @return frame list.
 */
FrameList* TagLibFile::getFrameList() const
{
	if (!s_tagLibFrameList) {
		s_tagLibFrameList = new TagLibFrameList();
	}
	return s_tagLibFrameList;
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

/**
 * Clean up static resources.
 */
void TagLibFile::staticCleanup()
{
	delete s_tagLibFrameList;
	s_tagLibFrameList = 0;
}

#endif // HAVE_TAGLIB
