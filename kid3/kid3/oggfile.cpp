/**
 * \file oggfile.cpp
 * Handling of Ogg files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
 */

#include "oggfile.h"
#if defined HAVE_VORBIS || defined HAVE_FLAC

#include "standardtags.h"
#include "genres.h"
#include "oggframelist.h"
#include "dirinfo.h"
#include <qfile.h>
#include <qdir.h>
#if QT_VERSION >= 0x040000
#include <Q3CString>
#endif
#include <sys/stat.h>
#ifdef WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <stdio.h>
#include <math.h>
#ifdef HAVE_VORBIS
#include "vcedit.h"
#include <vorbis/vorbisfile.h>
#endif

/**
 * Constructor.
 *
 * @param di directory information
 * @param fn filename
 */
OggFile::OggFile(const DirInfo* di, const QString& fn) :
	TaggedFile(di, fn), m_fileRead(false)
{
}

/**
 * Destructor.
 */
OggFile::~OggFile()
{
}

#ifdef HAVE_VORBIS
/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void OggFile::readTags(bool force)
{
	if (force || !m_fileRead) {
		m_comments.clear();
		markTag2Changed(false);
		m_fileRead = true;
		Q3CString fnIn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());

		if (m_fileInfo.read(fnIn)) {
			FILE* fpIn = ::fopen(fnIn, "rb");
			if (fpIn) {
				vcedit_state* state = ::vcedit_new_state();
				if (state) {
					if (::vcedit_open(state, fpIn) >= 0) {
						vorbis_comment* vc = ::vcedit_comments(state);
						if (vc) {
							for (int i = 0; i < vc->comments; ++i) {
								QString userComment =
									QString::fromUtf8(vc->user_comments[i],
																		vc->comment_lengths[i]);
								int equalPos = userComment.find('=');
								if (equalPos != -1) {
									QString name(
										userComment.left(equalPos).stripWhiteSpace().upper());
									QString value(
										userComment.mid(equalPos + 1).stripWhiteSpace());
									if (!value.isEmpty()) {
										m_comments.push_back(CommentField(name, value));
									}
								}
							}
						}
					}
					::vcedit_clear(state);
				}
				::fclose(fpIn);
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
 * @param force   true to force writing even if file was not changed.
 * @param renamed will be set to true if the file was renamed,
 *                i.e. the file name is no longer valid, else *renamed
 *                is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool OggFile::writeTags(bool force, bool* renamed, bool preserve)
{
	QString dirname = getDirInfo()->getDirname();
	if (isChanged() &&
		!QFileInfo(dirname + QDir::separator() + currentFilename()).isWritable()) {
		return false;
	}

	if (m_fileRead && (force || isTag2Changed())) {
		bool writeOk = false;
		QString tempFilename(currentFilename());
		Q3CString fnIn;
		if (getFilename() == currentFilename()) {
			// we have to rename the original file and delete it afterwards
			tempFilename += "_KID3";
			if (!renameFile(currentFilename(), tempFilename)) {
				return false;
			}
			fnIn = QFile::encodeName(dirname + QDir::separator() + tempFilename);
		} else {
			fnIn = QFile::encodeName(dirname + QDir::separator() + currentFilename());
		}
		Q3CString fnOut = QFile::encodeName(dirname + QDir::separator() +
																			 getFilename());
		FILE* fpIn = ::fopen(fnIn, "rb");
		if (fpIn) {

			// store time stamp if it has to be preserved
			bool setUtime = false;
			struct utimbuf times;
			if (preserve) {
				int fd = fileno(fpIn);
				if (fd >= 0) {
					struct stat fileStat;
					if (::fstat(fd, &fileStat) == 0) {
						times.actime  = fileStat.st_atime;
						times.modtime = fileStat.st_mtime;
						setUtime = true;
					}
				}
			}

			FILE* fpOut = ::fopen(fnOut, "wb");
			if (fpOut) {
				vcedit_state* state = ::vcedit_new_state();
				if (state) {
					if (::vcedit_open(state, fpIn) >= 0) {
						vorbis_comment* vc = ::vcedit_comments(state);
						if (vc) {
							::vorbis_comment_clear(vc);
							::vorbis_comment_init(vc);
							CommentList::iterator it = m_comments.begin();
							while (it != m_comments.end()) {
								QString name((*it).getName());
								QString value((*it).getValue());
								if (!value.isEmpty()) {
									::vorbis_comment_add_tag(
										vc,
										const_cast<char*>(name.latin1()),
										const_cast<char*>((const char*)value.utf8()));
									++it;
								} else {
									it = m_comments.erase(it);
								}
							}
							if (::vcedit_write(state, fpOut) >= 0) {
								writeOk = true;
							}
						}
					}
					::vcedit_clear(state);
				}
				::fclose(fpOut);
			}
			::fclose(fpIn);

			// restore time stamp
			if (setUtime) {
				::utime(fnOut, &times);
			}
		}
		if (!writeOk) {
			return false;
		}
		markTag2Changed(false);
		if (getFilename() == currentFilename()) {
			QDir(dirname).remove(tempFilename);
		} else {
			QDir(dirname).remove(currentFilename());
			updateCurrentFilename();
			*renamed = true;
		}
	} else if (getFilename() != currentFilename()) {
		// tags not changed, but file name
		if (!renameFile(currentFilename(), getFilename())) {
			return false;
		}
		updateCurrentFilename();
		*renamed = true;
	}
	return true;
}
#else // HAVE_VORBIS
void OggFile::readTags(bool) {}
bool OggFile::writeTags(bool, bool*, bool) { return false; }
#endif // HAVE_VORBIS

/**
 * Remove all ID3v2 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void OggFile::removeTagsV2(const StandardTagsFilter& flt)
{
	if (flt.areAllTrue()) {
		m_comments.clear();
		markTag2Changed();
	} else {
		removeStandardTagsV2(flt);
	}
}

/**
 * Get ID3v2 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString OggFile::getTitleV2()
{
	return getTextField("TITLE");
}

/**
 * Get ID3v2 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString OggFile::getArtistV2()
{
	return getTextField("ARTIST");
}

/**
 * Get ID3v2 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString OggFile::getAlbumV2()
{
	return getTextField("ALBUM");
}

/**
 * Get ID3v2 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString OggFile::getCommentV2()
{
	return getTextField(getCommentFieldName());
}

/**
 * Get ID3v2 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int OggFile::getYearV2()
{
	QString str = getTextField("DATE");
	if (str.isNull()) return -1;
	if (str.isEmpty()) return 0;
	return str.toInt();
}

/**
 * Get ID3v2 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int OggFile::getTrackNumV2()
{
	QString str = getTextField("TRACKNUMBER");
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
 * Get ID3v2 genre as text.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString OggFile::getGenreV2()
{
	return getTextField("GENRE");
}

/**
 * Get text field.
 *
 * @param name name
 * @return value, "" if not found,
 *         QString::null if the tags have not been read yet.
 */
QString OggFile::getTextField(const QString& name) const
{
	if (m_fileRead) {
		return m_comments.getValue(name);
	}
	return QString::null;
}

/**
 * Set text field.
 * If value is null if the tags have not been read yet, nothing is changed.
 * If value is different from the current value, tag 2 is marked as changed.
 *
 * @param name name
 * @param value value, "" to remove, QString::null to do nothing
 */
void OggFile::setTextField(const QString& name, const QString& value)
{
	if (m_fileRead && !value.isNull() &&
			m_comments.setValue(name, value)) {
		markTag2Changed();
	}
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setTitleV2(const QString& str)
{
	setTextField("TITLE", str);
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setArtistV2(const QString& str)
{
	setTextField("ARTIST", str);
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setAlbumV2(const QString& str)
{
	setTextField("ALBUM", str);
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setCommentV2(const QString& str)
{
	setTextField(getCommentFieldName(), str);
}

/**
 * Set ID3v2 year.
 *
 * @param num number to set, 0 to remove field, < 0 to ignore.
 */
void OggFile::setYearV2(int num)
{
	if (num >= 0) {
		QString str;
		if (num != 0) {
			str.setNum(num);
		} else {
			str = "";
		}
		setTextField("DATE", str);
	}
}

/**
 * Set ID3v2 track.
 *
 * @param num number to set, 0 to remove field, < 0 to ignore.
 */
void OggFile::setTrackNumV2(int num)
{
	if (num >= 0) {
		QString str;
		int numTracks = -1;
		if (num != 0) {
			numTracks = getTotalNumberOfTracksIfEnabled();
			str.setNum(num);
		} else {
			str = "";
		}
		setTextField("TRACKNUMBER", str);
		if (numTracks > 0) {
			str.setNum(numTracks);
			setTextField("TRACKTOTAL", str);
		}
	}
}

/**
 * Set ID3v2 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setGenreV2(const QString& str)
{
	setTextField("GENRE", str);
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTagV1() and hasTagV2() do not return meaningful information.
 */
bool OggFile::isTagInformationRead() const
{
	return m_fileRead;
}

/**
 * Check if file has an ID3v2 tag.
 *
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool OggFile::hasTagV2() const
{
	return !m_comments.empty();
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".ogg".
 */
QString OggFile::getFileExtension() const
{
	return ".ogg";
}

#ifdef HAVE_VORBIS
/**
 * Get technical detail information.
 *
 * @return string with detail information,
 *         "" if no information available.
 */
QString OggFile::getDetailInfo() const
{
	QString str;
	if (m_fileRead && m_fileInfo.valid) {
		str = QString("Ogg Vorbis %1 kbps %2 Hz %3 Channels ").
			arg(m_fileInfo.bitrate / 1000).
			arg(m_fileInfo.sampleRate).
			arg(m_fileInfo.channels);
		if (m_fileInfo.duration > 0) {
			str += formatTime(m_fileInfo.duration);
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
unsigned OggFile::getDuration() const
{
	if (m_fileRead && m_fileInfo.valid) {
		return m_fileInfo.duration;
	}
	return 0;
}

/**
 * Get the format of tag 2.
 *
 * @return "Vorbis".
 */
QString OggFile::getTagFormatV2() const
{
	return hasTagV2() ? QString("Vorbis") : QString::null;
}

/** Frame list for Ogg files. */
OggFrameList* OggFile::s_oggFrameList = 0;

/**
 * Get frame list for this type of tagged file.
 *
 * @return frame list.
 */
FrameList* OggFile::getFrameList() const
{
	if (!s_oggFrameList) {
		s_oggFrameList = new OggFrameList();
	}
	return s_oggFrameList;
}

/**
 * Clean up static resources.
 */
void OggFile::staticCleanup()
{
	delete s_oggFrameList;
	s_oggFrameList = 0;
}


/**
 * Read information about an Ogg/Vorbis file.
 * @param fn file name
 * @return true if ok.
 */
bool OggFile::FileInfo::read(const char* fn)
{
	valid = false;
	FILE* fp = ::fopen(fn, "rb");
	if (fp) {
		OggVorbis_File vf;
		if (::ov_open(fp, &vf, NULL, 0) == 0) {
			vorbis_info* vi = ::ov_info(&vf, -1);
			if (vi) {
				valid = true;
				version = vi->version;
				channels = vi->channels;
				sampleRate = vi->rate;
				bitrate = vi->bitrate_nominal;
				if (bitrate <= 0) {
					bitrate = vi->bitrate_upper;
				}
				if (bitrate <= 0) {
					bitrate = vi->bitrate_lower;
				}
			}
#ifdef WIN32
			duration = (long)::ov_time_total(&vf, -1);
#else
			duration = ::lrint(::ov_time_total(&vf, -1));
#endif
			::ov_clear(&vf); // closes file, do not use ::fclose()
		} else {
			::fclose(fp);
		}
	}
	return valid;
}
#else // HAVE_VORBIS
QString OggFile::getDetailInfo() const { return ""; }
unsigned OggFile::getDuration() const { return 0; }
FrameList* OggFile::getFrameList() const { return 0; }
void OggFile::staticCleanup() {}
#endif // HAVE_VORBIS

/**
 * Get value.
 * @param name name
 * @return value, "" if not found.
 */
QString OggFile::CommentList::getValue(const QString& name) const
{
	for (const_iterator it = begin(); it != end(); ++it) {
		if ((*it).getName() == name) {
			return (*it).getValue();
		}
	}
	return "";
}

/**
 * Set value.
 * @param name name
 * @param value value
 * @return true if value was changed.
 */
bool OggFile::CommentList::setValue(const QString& name, const QString& value)
{
	for (iterator it = begin(); it != end(); ++it) {
		if ((*it).getName() == name) {
			QString oldValue = (*it).getValue();
			if (value != oldValue) {
				(*it).setValue(value);
				return true;
			} else {
				return false;
			}
		}
	}
	if (!value.isEmpty()) {
		CommentField cf(name, value);
		push_back(cf);
		return true;
	} else {
		return false;
	}
}

#endif // HAVE_VORBIS || define HAVE_FLAC
