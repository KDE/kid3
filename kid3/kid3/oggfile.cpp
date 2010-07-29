/**
 * \file oggfile.cpp
 * Handling of Ogg files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#include "oggfile.hpp"
#if defined HAVE_VORBIS || defined HAVE_FLAC

#include "dirinfo.h"
#include "pictureframe.h"
#include "kid3.h"
#include <qfile.h>
#include <qdir.h>
#if QT_VERSION >= 0x040000
#include <QByteArray>
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
		markTag2Unchanged();
		m_fileRead = true;
		QCM_QCString fnIn = QFile::encodeName(getDirInfo()->getDirname() + QDir::separator() + currentFilename());

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
								int equalPos = userComment.QCM_indexOf('=');
								if (equalPos != -1) {
									QString name(
										userComment.left(equalPos).QCM_trimmed().QCM_toUpper());
									QString value(
										userComment.mid(equalPos + 1).QCM_trimmed());
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
		// we have to rename the original file and delete it afterwards
		QString tempFilename(currentFilename() + "_KID3");
		if (!renameFile(currentFilename(), tempFilename)) {
			return false;
		}
		QCM_QCString fnIn = QFile::encodeName(dirname + QDir::separator() +
																					tempFilename);
		QCM_QCString fnOut = QFile::encodeName(dirname + QDir::separator() +
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
										const_cast<char*>(name.QCM_latin1()),
										const_cast<char*>((const char*)value.QCM_toUtf8().data()));
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
		markTag2Unchanged();
		QDir(dirname).remove(tempFilename);
		if (getFilename() != currentFilename()) {
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
		"METADATA_BLOCK_PICTURE", // FT_Picture,
		"PUBLISHER",       // FT_Publisher,
		"REMIXER",         // FT_Remixer,
		"SUBTITLE",        // FT_Subtitle,
		"WEBSITE",         // FT_Website,
		                   // FT_LastFrame = FT_Website
	};
	class not_used { int array_size_check[
			sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1
			? 1 : -1 ]; };
	if (type == Frame::FT_Picture &&
			Kid3App::s_miscCfg.m_pictureNameItem == MiscConfig::VP_COVERART) {
		return "COVERART";
	}
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
		strNumMap.insert("COVERART", Frame::FT_Picture);
	}
	QMap<QString, int>::const_iterator it =
		strNumMap.find(name.remove(' ').QCM_toUpper());
	if (it != strNumMap.end()) {
		return static_cast<Frame::Type>(*it);
	}
	return Frame::FT_Other;
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
	if (type <= Frame::FT_LastFrame) {
		return getVorbisNameFromType(type);
	} else {
		return frame.getName().remove(' ').QCM_toUpper();
	}
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void OggFile::deleteFramesV2(const FrameFilter& flt)
{
	if (flt.areAllEnabled()) {
		m_comments.clear();
		markTag2Changed(Frame::FT_UnknownFrame);
	} else {
		bool changed = false;
		for (OggFile::CommentList::iterator it = m_comments.begin();
				 it != m_comments.end();) {
			QString name((*it).getName());
			if (flt.isEnabled(getTypeFromVorbisName(name), name)) {
				it = m_comments.erase(it);
				changed = true;
			} else {
				++it;
			}
		}
		if (changed) {
			markTag2Changed(Frame::FT_UnknownFrame);
		}
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
	int slashPos = str.QCM_indexOf('/');
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
 * @param type frame type
 */
void OggFile::setTextField(const QString& name, const QString& value,
                           Frame::Type type)
{
	if (m_fileRead && !value.isNull() &&
			m_comments.setValue(name, value)) {
		markTag2Changed(type);
	}
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setTitleV2(const QString& str)
{
	setTextField("TITLE", str, Frame::FT_Title);
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setArtistV2(const QString& str)
{
	setTextField("ARTIST", str, Frame::FT_Artist);
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setAlbumV2(const QString& str)
{
	setTextField("ALBUM", str, Frame::FT_Album);
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void OggFile::setCommentV2(const QString& str)
{
	setTextField(getCommentFieldName(), str, Frame::FT_Comment);
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
		setTextField("DATE", str, Frame::FT_Date);
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
			formatTrackNumberIfEnabled(str, false);
		} else {
			str = "";
		}
		setTextField("TRACKNUMBER", str, Frame::FT_Track);
		if (numTracks > 0) {
			str.setNum(numTracks);
			formatTrackNumberIfEnabled(str, false);
			setTextField("TRACKTOTAL", str, Frame::FT_Other);
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
	setTextField("GENRE", str, Frame::FT_Genre);
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
 * @param info the detail information is returned here
 */
void OggFile::getDetailInfo(DetailInfo& info) const
{
	if (m_fileRead && m_fileInfo.valid) {
		info.valid = true;
		info.format = "Ogg Vorbis";
		info.bitrate = m_fileInfo.bitrate / 1000;
		info.sampleRate = m_fileInfo.sampleRate;
		info.channels = m_fileInfo.channels;
		info.duration = m_fileInfo.duration;
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

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool OggFile::setFrameV2(const Frame& frame)
{
	if (frame.getType() == Frame::FT_Track) {
		int numTracks = getTotalNumberOfTracksIfEnabled();
		if (numTracks > 0) {
			QString numTracksStr = QString::number(numTracks);
			formatTrackNumberIfEnabled(numTracksStr, false);
			if (getTextField("TRACKTOTAL") != numTracksStr) {
				setTextField("TRACKTOTAL", numTracksStr, Frame::FT_Other);
				markTag2Changed(Frame::FT_Other);
			}
		}
	}

	// If the frame has an index, change that specific frame
	int index = frame.getIndex();
	if (index != -1 && index < static_cast<int>(m_comments.size())) {
		QString value = frame.getValue();
		if (frame.getType() == Frame::FT_Picture) {
#ifdef HAVE_BASE64_ENCODING
			PictureFrame::getFieldsToBase64(frame, value);
			if (!value.isEmpty() && frame.getName(true) == "COVERART") {
				QString mimeType;
				PictureFrame::getMimeType(frame, mimeType);
				setTextField("COVERARTMIME", mimeType, Frame::FT_Other);
			}
#else
			return false;
#endif
		} else if (frame.getType() == Frame::FT_Track) {
			formatTrackNumberIfEnabled(value, false);
		}
#if QT_VERSION >= 0x040000
		if (m_comments[index].getValue() != value) {
			m_comments[index].setValue(value);
			markTag2Changed(frame.getType());
		}
		return true;
#else
		CommentList::iterator it = m_comments.at(index);
		if (it != m_comments.end()) {
			if ((*it).getValue() != value) {
				(*it).setValue(value);
				markTag2Changed(frame.getType());
			}
			return true;
		}
#endif
	}

	// Try the superclass method
	return TaggedFile::setFrameV2(frame);
}

/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add
 *
 * @return true if ok.
 */
bool OggFile::addFrameV2(Frame& frame)
{
	// Add a new frame.
	QString name(getVorbisName(frame));
	QString value(frame.getValue());
	if (frame.getType() == Frame::FT_Picture) {
#ifdef HAVE_BASE64_ENCODING
		if (frame.getFieldList().empty()) {
			PictureFrame::setFields(
				frame, Frame::Field::TE_ISO8859_1, "", "image/jpeg",
				PictureFrame::PT_CoverFront, "", QByteArray());
		}
		frame.setInternalName(name);
		PictureFrame::getFieldsToBase64(frame, value);
#else
		return false;
#endif
	}
	m_comments.push_back(OggFile::CommentField(name, value));
	frame.setInternalName(name);
	frame.setIndex(m_comments.size() - 1);
	markTag2Changed(frame.getType());
	return true;
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool OggFile::deleteFrameV2(const Frame& frame)
{
	// If the frame has an index, delete that specific frame
	int index = frame.getIndex();
	if (index != -1 && index < static_cast<int>(m_comments.size())) {
#if QT_VERSION >= 0x040000
		m_comments.removeAt(index);
#else
		OggFile::CommentList::iterator it = m_comments.at(index);
		m_comments.erase(it);
#endif
		markTag2Changed(frame.getType());
		return true;
	}

	// Try the superclass method
	return TaggedFile::deleteFrameV2(frame);
}

/**
 * Get all frames in tag 2.
 *
 * @param frames frame collection to set.
 */
void OggFile::getAllFramesV2(FrameCollection& frames)
{
	frames.clear();
	QString name;
	int i = 0;
	for (OggFile::CommentList::const_iterator it = m_comments.begin();
			 it != m_comments.end();
			 ++it) {
		name = (*it).getName();
		Frame::Type type = getTypeFromVorbisName(name);
#ifdef HAVE_BASE64_ENCODING
		if (type == Frame::FT_Picture) {
			Frame frame(type, "", name, i++);
			PictureFrame::setFieldsFromBase64(frame, (*it).getValue());
			if (name == "COVERART") {
				PictureFrame::setMimeType(frame, getTextField("COVERARTMIME"));
			}
			frames.insert(frame);
		} else
#endif
		frames.insert(Frame(type, (*it).getValue(), name, i++));
	}
	frames.addMissingStandardFrames();
}

/**
 * Get a list of frame IDs which can be added.
 *
 * @return list with frame IDs.
 */
QStringList OggFile::getFrameIds() const
{
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

	QStringList lst;
	for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
#ifndef HAVE_BASE64_ENCODING
		if (k != Frame::FT_Picture)
#endif
			lst.append(QCM_translate(Frame::getNameFromType(static_cast<Frame::Type>(k))));
	}
	for (unsigned i = 0; i < sizeof(fieldNames) / sizeof(fieldNames[0]); ++i) {
		lst.append(fieldNames[i]);
	}
	return lst;
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
void OggFile::getDetailInfo(DetailInfo& info) const { info.valid = false; }
unsigned OggFile::getDuration() const { return 0; }
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


/**
 * Create an OggFile object if it supports the filename's extension.
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* OggFile::Resolver::createFile(const DirInfo* di,
																					const QString& fn) const
{
	QString ext = fn.right(4).QCM_toLower();
	if (ext == ".oga" || ext == ".ogg")
		return new OggFile(di, fn);
	else
		return 0;
}

/**
 * Get a list with all extensions supported by OggFile.
 *
 * @return list of file extensions.
 */
QStringList OggFile::Resolver::getSupportedFileExtensions() const
{
	return QStringList() << ".oga" << ".ogg";
}

#endif // HAVE_VORBIS || define HAVE_FLAC
