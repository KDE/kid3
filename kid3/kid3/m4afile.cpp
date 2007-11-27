/**
 * \file m4afile.cpp
 * Handling of MPEG-4 audio files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Oct 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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

#include "m4afile.h"
#ifdef HAVE_MP4V2

#include "standardtags.h"
#include "dirinfo.h"
#include "genres.h"
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
#include <mp4.h>

#if defined MPEG4IP_MAJOR_VERSION && defined MPEG4IP_MINOR_VERSION
#define MPEG4IP_MAJOR_MINOR_VERSION ((MPEG4IP_MAJOR_VERSION << 8) | MPEG4IP_MINOR_VERSION)
#else
#define MPEG4IP_MAJOR_MINOR_VERSION 0x0009
#endif

/**
 * Constructor.
 *
 * @param di directory information
 * @param fn filename
 */
M4aFile::M4aFile(const DirInfo* di, const QString& fn) :
	TaggedFile(di, fn), m_fileRead(false)
{
}

/**
 * Destructor.
 */
M4aFile::~M4aFile()
{
}

/** Mapping between frame types and field names. */
static const struct {
	const char* name;
	Frame::Type type;
} nameTypes[] = {
	{ "\251nam", Frame::FT_Title },
	{ "\251ART", Frame::FT_Artist },
	{ "\251wrt", Frame::FT_Composer },
	{ "\251alb", Frame::FT_Album },
	{ "\251day", Frame::FT_Date },
	{ "\251too", Frame::FT_EncodedBy },
	{ "\251cmt", Frame::FT_Comment },
	{ "\251gen", Frame::FT_Genre },
	{ "trkn", Frame::FT_Track },
	{ "disk", Frame::FT_Disc },
	{ "gnre", Frame::FT_Genre },
	{ "cpil", Frame::FT_Other },
	{ "tmpo", Frame::FT_Bpm },
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0105
	{ "\251grp", Frame::FT_Other },
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
	{ "aART", Frame::FT_Other },
	{ "pgap", Frame::FT_Other },
#endif
	{ "covr", Frame::FT_Other }
};

/**
 * Get the predefined field name for a type.
 *
 * @param type frame type
 *
 * @return field name, QString::null if not defined.
 */
static QString getNameForType(Frame::Type type)
{
	static QMap<Frame::Type, QString> typeNameMap;
	if (typeNameMap.empty()) {
		// first time initialization
		for (unsigned i = 0; i < sizeof(nameTypes) / sizeof(nameTypes[0]); ++i) {
			if (nameTypes[i].type != Frame::FT_Other) {
				typeNameMap.insert(nameTypes[i].type, nameTypes[i].name);
			}
		}
	}
	if (type != Frame::FT_Other) {
		QMap<Frame::Type, QString>::const_iterator it = typeNameMap.find(type);
		if (it != typeNameMap.end()) {
			return *it;
		}
	}
	return QString::null;
}

/**
 * Get the type for a predefined field name.
 *
 * @param name           field name
 * @param onlyPredefined if true, FT_Unknown is returned for fields which
 *                       are not predefined, else FT_Other
 *
 * @return type, FT_Other for "cpil", "covr",
 *         FT_Unknown or FT_Other if not predefined field.
 */
static Frame::Type getTypeForName(const QString& name,
																	bool onlyPredefined = false)
{
	if (name.length() == 4) {
		static QMap<QString, Frame::Type> nameTypeMap;
		if (nameTypeMap.empty()) {
			// first time initialization
			for (unsigned i = 0; i < sizeof(nameTypes) / sizeof(nameTypes[0]); ++i) {
				nameTypeMap.insert(nameTypes[i].name, nameTypes[i].type);
			}
		}
		QMap<QString, Frame::Type>::const_iterator it = nameTypeMap.find(name);
		if (it != nameTypeMap.end()) {
			return *it;
		}
	}
	return onlyPredefined ? Frame::FT_UnknownFrame : Frame::FT_Other;
}

#ifndef HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG
/**
 * Check if a name is a free form field.
 *
 * @param hFile handle
 * @param name  field name
 *
 * @return true if a free form field.
 */
static bool isFreeFormMetadata(MP4FileHandle hFile, const char* name)
{
	bool result = false;
	if (getTypeForName(name, true) == Frame::FT_UnknownFrame) {
		u_int8_t* pValue = 0;
		u_int32_t valueSize = 0;
		result = MP4GetMetadataFreeForm(hFile, const_cast<char*>(name),
																		&pValue, &valueSize);
		if (pValue && valueSize > 0) {
			free(pValue);
		}
	}
	return result;
}
#endif

/**
 * Get a byte array for a value.
 *
 * @param name  field name
 * @param value field value
 * @param size  size of value in bytes
 *
 * @return byte array with string representation.
 */
static QByteArray getValueByteArray(const char* name,
																		const u_int8_t* value, u_int32_t size)
{
	QCM_QCString str;
	if (name[0] == '\251') {
		QCM_duplicate(str, reinterpret_cast<const char*>(value), size);
	} else if (std::strcmp(name, "trkn") == 0) {
		if (size >= 6) {
			unsigned track = value[3] + (value[2] << 8);
			unsigned totalTracks = value[5] + (value[4] << 8);
			str.setNum(track);
			if (totalTracks > 0) {
				str += '/';
				str += QCM_QCString().setNum(totalTracks);
			}
		}
	} else if (std::strcmp(name, "disk") == 0) {
		if (size >= 6) {
			unsigned disk = value[3] + (value[2] << 8);
			unsigned totalDisks = value[5] + (value[4] << 8);
			str.setNum(disk);
			if (totalDisks > 0) {
				str += '/';
				str += QCM_QCString().setNum(totalDisks);
			}
		}
	} else if (std::strcmp(name, "gnre") == 0) {
		if (size >= 2) {
			unsigned genreNum = value[1] + (value[0] << 8);
			if (genreNum > 0) {
				str = Genres::getName(genreNum - 1);
			}
		}
	} else if (std::strcmp(name, "cpil") == 0) {
		if (size >= 1) {
			str.setNum(value[0]);
		}
	} else if (std::strcmp(name, "tmpo") == 0) {
		if (size >= 2) {
			unsigned bpm = value[1] + (value[0] << 8);
			if (bpm > 0) {
				str.setNum(bpm);
			}
		}
	} else if (std::strcmp(name, "covr") == 0) {
		QByteArray ba;
		QCM_duplicate(ba, reinterpret_cast<const char*>(value), size);
		return ba;
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
	} else if (std::strcmp(name, "pgap") == 0) {
		if (size >= 1) {
			str.setNum(value[0]);
		}
#endif
	} else {
		QCM_duplicate(str, reinterpret_cast<const char*>(value), size);
	}
	return str;
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void M4aFile::readTags(bool force)
{
	if (force || !m_fileRead) {
		m_metadata.clear();
		markTag2Changed(false);
		m_fileRead = true;
		QCM_QCString fnIn = QFile::encodeName(
			getDirInfo()->getDirname() + QDir::separator() + currentFilename());

		MP4FileHandle handle = MP4Read(fnIn);
		if (handle != MP4_INVALID_FILE_HANDLE) {
			m_fileInfo.read(handle);
#ifdef HAVE_MP4V2_MP4GETMETADATABYINDEX_CHARPP_ARG
			static char notFreeFormStr[] = "NOFF";
			static char freeFormStr[] = "----";
			char* ppName;
			u_int8_t* ppValue = 0;
			u_int32_t pValueSize = 0;
			u_int32_t index = 0;
			unsigned numEmptyEntries = 0;
			for (index = 0; index < 64; ++index) {
				ppName = notFreeFormStr;
				bool ok = MP4GetMetadataByIndex(handle, index,
																				&ppName, &ppValue, &pValueSize);
				if (ok && ppName && memcmp(ppName, "----", 4) == 0) {
					// free form tagfield
					free(ppName);
					free(ppValue);
					ppName = freeFormStr;
					ppValue = 0;
					pValueSize = 0;
					ok = MP4GetMetadataByIndex(handle, index,
																		 &ppName, &ppValue, &pValueSize);
				}
				if (ok) {
					numEmptyEntries = 0;
					if (ppName) {
						QString key(ppName);
						QByteArray ba;
						if (ppValue && pValueSize > 0) {
							ba = getValueByteArray(ppName, ppValue, pValueSize);
						}
						m_metadata[key] = ba;
						free(ppName);
					}
					free(ppValue);
					ppName = 0;
					ppValue = 0;
					pValueSize = 0;
				} else {
					// There are iTunes files with invalid fields in between,
					// so we stop after 3 invalid indices.
					if (++numEmptyEntries >= 3) {
						break;
					}
				}
			}
#else
			const char* ppName = 0;
			u_int8_t* ppValue = 0;
			u_int32_t pValueSize = 0;
			u_int32_t index = 0;
			unsigned numEmptyEntries = 0;
			for (index = 0; index < 64; ++index) {
				if (MP4GetMetadataByIndex(handle, index,
																	&ppName, &ppValue, &pValueSize)) {
					numEmptyEntries = 0;
					if (ppName) {
						QString key(ppName);
						QByteArray ba;
						if (ppValue && pValueSize > 0) {
							ba = getValueByteArray(ppName, ppValue, pValueSize);
						}
						m_metadata[key] = ba;

						// If the field is free form, there are two memory leaks in mp4v2.
						// The first is not accessible, the second can be freed.
						if (isFreeFormMetadata(handle, ppName)) {
							free(const_cast<char*>(ppName));
						}
					}
					free(ppValue);
					ppName = 0;
					ppValue = 0;
					pValueSize = 0;
				} else {
					// There are iTunes files with invalid fields in between,
					// so we stop after 3 invalid indices.
					if (++numEmptyEntries >= 3) {
						break;
					}
				}
			}
#endif
			MP4Close(handle);
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
bool M4aFile::writeTags(bool force, bool* renamed, bool preserve)
{
	bool ok = true;
	QString fnStr(getDirInfo()->getDirname() + QDir::separator() +
								currentFilename());
	if (isChanged() && !QFileInfo(fnStr).isWritable()) {
		return false;
	}

	if (m_fileRead && (force || isTag2Changed())) {
		QCM_QCString fn = QFile::encodeName(fnStr);

		// store time stamp if it has to be preserved
		bool setUtime = false;
		struct utimbuf times;
		if (preserve) {
			struct stat fileStat;
			if (::stat(fn, &fileStat) == 0) {
				times.actime  = fileStat.st_atime;
				times.modtime = fileStat.st_mtime;
				setUtime = true;
			}
		}

    MP4FileHandle handle = MP4Modify(fn);
    if (handle != MP4_INVALID_FILE_HANDLE) {
			// return code is not checked because it will fail if no metadata exists
			MP4MetadataDelete(handle);

			for (MetadataMap::const_iterator it = m_metadata.begin();
					 it != m_metadata.end();
					 ++it) {
				const QByteArray& value = *it;
				if (!value.isEmpty()) {
					const QString& name = it.key();
#if QT_VERSION >= 0x040000
					const QByteArray& str = value;
#else
					QCString str(value.data(), value.size() + 1);
#endif
					bool setOk;
					if (name == "\251nam") {
						setOk = MP4SetMetadataName(handle, str);
					} else if (name == "\251ART") {
						setOk = MP4SetMetadataArtist(handle, str);
					} else if (name == "\251wrt") {
						setOk = MP4SetMetadataWriter(handle, str);
					} else if (name == "\251cmt") {
						setOk = MP4SetMetadataComment(handle, str);
					} else if (name == "\251too") {
						setOk = MP4SetMetadataTool(handle, str);
					} else if (name == "\251day") {
						unsigned short year = str.toUShort();
						if (year > 0) {
							if (year < 1000) year += 2000;
							else if (year > 9999) year = 9999;
							setOk = MP4SetMetadataYear(handle, QCM_QCString().setNum(year));
							if (setOk) setYearV2(year);
						} else {
							setOk = true;
						}
					} else if (name == "\251alb") {
						setOk = MP4SetMetadataAlbum(handle, str);
					} else if (name == "trkn") {
						u_int16_t track = 0, totalTracks = 0;
						int slashPos = str.QCM_indexOf('/');
						if (slashPos != -1) {
							totalTracks = str.mid(slashPos + 1).toUShort();
							track = str.mid(0, slashPos).toUShort();
						} else {
							track = str.toUShort();
						}
						setOk = MP4SetMetadataTrack(handle, track, totalTracks);
					} else if (name == "disk") {
						u_int16_t disk = 0, totalDisks = 0;
						int slashPos = str.QCM_indexOf('/');
						if (slashPos != -1) {
							totalDisks = str.mid(slashPos + 1).toUShort();
							disk = str.mid(0, slashPos).toUShort();
						} else {
							disk = str.toUShort();
						}
						setOk = MP4SetMetadataDisk(handle, disk, totalDisks);
					} else if (name == "\251gen" || name == "gnre") {
						setOk = MP4SetMetadataGenre(handle, str);
					} else if (name == "tmpo") {
						u_int16_t tempo = str.toUShort();
						setOk = MP4SetMetadataTempo(handle, tempo);
					} else if (name == "cpil") {
						u_int8_t cpl = str.toUShort();
						setOk = MP4SetMetadataCompilation(handle, cpl);
					} else if (name == "covr") {
						setOk = MP4SetMetadataCoverArt(
							handle,
							reinterpret_cast<u_int8_t*>(const_cast<char*>(value.data())),
							value.size());
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0105
					} else if (name == "\251grp") {
						setOk = MP4SetMetadataGrouping(handle, str);
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
					} else if (name == "aART") {
						setOk = MP4SetMetadataAlbumArtist(handle, str);
					} else if (name == "pgap") {
						u_int8_t pgap = str.toUShort();
						setOk = MP4SetMetadataPartOfGaplessAlbum(handle, pgap);
#endif
					} else {
						setOk = MP4SetMetadataFreeForm(
							handle, const_cast<char*>(name.QCM_toUtf8().data()),
							reinterpret_cast<u_int8_t*>(const_cast<char*>(value.data())),
							value.size());
					}
					if (!setOk) {
						qDebug("MP4SetMetadata %s failed", name.QCM_latin1());
						ok = false;
					}
				}
			}

			MP4Close(handle);
			if (ok) {
				// without this, old tags stay in the file marked as free
				MP4Optimize(fn);
				markTag2Changed(false);
			}

			// restore time stamp
			if (setUtime) {
				::utime(fn, &times);
			}
		} else {
			qDebug("MP4Modify failed");
			ok = false;
		}
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
	return ok;
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void M4aFile::deleteFramesV2(const FrameFilter& flt)
{
	if (flt.areAllEnabled()) {
		m_metadata.clear();
		markTag2Changed();
	} else {
		bool changed = false;
		for (MetadataMap::iterator it = m_metadata.begin();
				 it != m_metadata.end();) {
			QString name(it.key());
			Frame::Type type = getTypeForName(name);
			if (flt.isEnabled(type, name)) {
#if QT_VERSION >= 0x040000
				it = m_metadata.erase(it);
#else
				m_metadata.erase(it);
				++it;
#endif
				changed = true;
			} else {
				++it;
			}
		}
		if (changed) {
			markTag2Changed();
		}
	}
}

/**
 * Get metadata field as string.
 *
 * @param name field name
 *
 * @return value as string, "" if not found,
 *         QString::null if the tags have not been read yet.
 */
QString M4aFile::getTextField(const QString& name) const
{
	if (m_fileRead) {
		MetadataMap::const_iterator it = m_metadata.find(name);
		if (it != m_metadata.end()) {
			return QString::fromUtf8((*it).data(), (*it).size());
		}
		return "";
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
void M4aFile::setTextField(const QString& name, const QString& value)
{
	if (m_fileRead && !value.isNull()) {
		QByteArray str = value.QCM_toUtf8();
		MetadataMap::iterator it = m_metadata.find(name);
		if (it != m_metadata.end()) {
			if (*it != str) {
				*it = str;
				markTag2Changed();
			}
		} else {
			m_metadata.insert(name, str);
			markTag2Changed();
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
QString M4aFile::getTitleV2()
{
	return getTextField("\251nam");
}

/**
 * Get ID3v2 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString M4aFile::getArtistV2()
{
	return getTextField("\251ART");
}

/**
 * Get ID3v2 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString M4aFile::getAlbumV2()
{
	return getTextField("\251alb");
}

/**
 * Get ID3v2 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString M4aFile::getCommentV2()
{
	return getTextField("\251cmt");
}

/**
 * Get ID3v2 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int M4aFile::getYearV2()
{
	QString str(getTextField("\251day"));
	if (!str.isNull()) {
		return str.toInt();
	}
	return -1;
}

/**
 * Get ID3v2 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int M4aFile::getTrackNumV2()
{
	QString str(getTextField("trkn"));
	if (!str.isNull()) {
		int slashPos = str.QCM_indexOf('/');
		if (slashPos != -1) {
			str.truncate(slashPos);
		}
		return str.toInt();
	}
	return -1;
}

/**
 * Get ID3v2 genre as text.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString M4aFile::getGenreV2()
{
	QString str(getTextField("\251gen"));
	if (str.isEmpty()) {
		str = getTextField("gnre");
	}
	return str;
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void M4aFile::setTitleV2(const QString& str)
{
	setTextField("\251nam", str);
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void M4aFile::setArtistV2(const QString& str)
{
	setTextField("\251ART", str);
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void M4aFile::setAlbumV2(const QString& str)
{
	setTextField("\251alb", str);
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void M4aFile::setCommentV2(const QString& str)
{
	setTextField("\251cmt", str);
}

/**
 * Set ID3v2 year.
 *
 * @param num number to set, 0 to remove field, < 0 to ignore.
 */
void M4aFile::setYearV2(int num)
{
	if (num >= 0) {
		setTextField("\251day", num != 0 ? QString::number(num) : "");
	}
}

/**
 * Set ID3v2 track.
 *
 * @param num number to set, 0 to remove field, < 0 to ignore.
 */
void M4aFile::setTrackNumV2(int num)
{
	if (num >= 0) {
		QString str;
		if (num != 0) {
			str.setNum(num);
			int numTracks = getTotalNumberOfTracksIfEnabled();
			if (numTracks > 0) {
				str += '/';
				str += QString::number(numTracks);
			}
		} else {
			str = "";
		}
		setTextField("trkn", str);
	}
}

/**
 * Set ID3v2 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void M4aFile::setGenreV2(const QString& str)
{
	if (str != getGenreV2()) {
		int genreNum = Genres::getNumber(str);
		if (genreNum != 255) {
			setTextField("gnre", str);
			m_metadata.remove("\251gen");
		} else {
			setTextField("\251gen", str);
			m_metadata.remove("gnre");
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
bool M4aFile::isTagInformationRead() const
{
	return m_fileRead;
}

/**
 * Check if file has an ID3v2 tag.
 *
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool M4aFile::hasTagV2() const
{
	return !m_metadata.empty();
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".m4a".
 */
QString M4aFile::getFileExtension() const
{
	return ".m4a";
}

/**
 * Get technical detail information.
 *
 * @return string with detail information,
 *         "" if no information available.
 */
QString M4aFile::getDetailInfo() const
{
	QString str;
	if (m_fileRead && m_fileInfo.valid) {
		str = "MP4 ";
		if (m_fileInfo.bitrate > 0 && m_fileInfo.bitrate < 999) {
			str += QString::number(m_fileInfo.bitrate);
			str += " kbps ";
		}
		if (m_fileInfo.sampleRate > 0) {
			str += QString::number(m_fileInfo.sampleRate);
			str += " Hz ";
		}
		if (m_fileInfo.channels > 0) {
			str += QString::number(m_fileInfo.channels);
			str += " Channels ";
		}
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
unsigned M4aFile::getDuration() const
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
QString M4aFile::getTagFormatV2() const
{
	return hasTagV2() ? QString("MP4") : QString::null;
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool M4aFile::setFrameV2(const Frame& frame)
{
	QString name(frame.getName(true));
	MetadataMap::iterator it = m_metadata.find(name);
	if (it != m_metadata.end()) {
		if (name != "covr") {
			*it = frame.getValue().QCM_toUtf8();
		} else {
			if (!frame.getFieldList().empty()) {
				const Frame::Field& fld = frame.getFieldList().front();
				if (fld.m_id == Frame::Field::ID_Data) {
					*it = fld.m_value.toByteArray();
				}
			}
		}
		return true;
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
bool M4aFile::addFrameV2(Frame& frame)
{
	Frame::Type type = frame.getType();
	QString name;
	if (type != Frame::FT_Other) {
		name = getNameForType(type);
		if (!name.isEmpty()) {
			frame.setInternalName(name);
		}
	}
	name = frame.getName(true);
	if (name == "covr") {
		if (!frame.getFieldList().empty() &&
				frame.getFieldList().front().m_id == Frame::Field::ID_Data) {
			m_metadata[name] = frame.getFieldList().front().m_value.toByteArray();
		} else {
			Frame::Field coverField;
			coverField.m_id = Frame::Field::ID_Data;
			coverField.m_value = QByteArray();
			frame.fieldList().clear();
			frame.fieldList().push_back(coverField);
			m_metadata[name] = QByteArray();
		}
	} else {
		m_metadata[name] = frame.getValue().QCM_toUtf8();
	}
	markTag2Changed();
	return true;
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool M4aFile::deleteFrameV2(const Frame& frame)
{
	QString name(frame.getName(true));
	MetadataMap::iterator it = m_metadata.find(name);
	if (it != m_metadata.end()) {
		m_metadata.erase(it);
		markTag2Changed();
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
void M4aFile::getAllFramesV2(FrameCollection& frames)
{
	frames.clear();
	QString name;
	QString value;
	for (MetadataMap::const_iterator it = m_metadata.begin();
			 it != m_metadata.end();
			 ++it) {
		name = it.key();
		Frame::Type type = getTypeForName(name);
		if (name != "covr") {
			value = QString::fromUtf8((*it).data(), (*it).size());
			frames.insert(Frame(type, value, name, -1));
		} else {
			Frame frame(type, "", name, -1);
			Frame::Field coverField;
			coverField.m_id = Frame::Field::ID_Data;
			coverField.m_value = QByteArray(*it);
			frame.fieldList().clear();
			frame.fieldList().push_back(coverField);
			frames.insert(frame);
		}
	}
	frames.addMissingStandardFrames();
}

/**
 * Get a list of frame IDs which can be added.
 *
 * @return list with frame IDs.
 */
QStringList M4aFile::getFrameIds() const
{
	static const Frame::Type types[] = {
		Frame::FT_Title,
		Frame::FT_Artist,
		Frame::FT_Album,
		Frame::FT_Comment,
		Frame::FT_Date,
		Frame::FT_Track,
		Frame::FT_Genre,
		Frame::FT_Bpm,
		Frame::FT_Composer,
		Frame::FT_Disc,
		Frame::FT_EncodedBy
	};

	QStringList lst;
	for (unsigned i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
		lst.append(QCM_translate(Frame::getNameFromType(types[i])));
	}
	lst << "covr" << "cpil";
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0105
	lst << "\251grp";
#endif
#if MPEG4IP_MAJOR_MINOR_VERSION >= 0x0106
	lst << "aART" << "pgap";
#endif
	return lst;
}


/**
 * Read information about an MPEG-4 file.
 * @param fn file name
 * @return true if ok.
 */
bool M4aFile::FileInfo::read(MP4FileHandle handle)
{
	valid = false;
	u_int32_t numTracks = MP4GetNumberOfTracks(handle);
	for (u_int32_t i = 0; i < numTracks; ++i) {
		MP4TrackId trackId = MP4FindTrackId(handle, i);
		const char* trackType = MP4GetTrackType(handle, trackId);
		if (std::strcmp(trackType, MP4_AUDIO_TRACK_TYPE) == 0) {
			valid = true;
			bitrate = (MP4GetTrackBitRate(handle, trackId) + 500) / 1000;
			sampleRate = MP4GetTrackTimeScale(handle, trackId);
			duration = MP4ConvertFromTrackDuration(
				handle, trackId,
				MP4GetTrackDuration(handle, trackId), MP4_MSECS_TIME_SCALE) / 1000;
			channels = 2;
			break;
		}
	}
	return valid;
}


/**
 * Create an M4aFile object if it supports the filename's extension.
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* M4aFile::Resolver::createFile(const DirInfo* di,
																					const QString& fn) const
{
	QString ext = fn.right(4).QCM_toLower();
	if (ext == ".m4a" || ext == ".m4b" || ext == ".m4p" || ext == ".mp4" ||
			ext == ".m4v" || ext == "mp4v")
		return new M4aFile(di, fn);
	else
		return 0;
}

/**
 * Get a list with all extensions supported by M4aFile.
 *
 * @return list of file extensions.
 */
QStringList M4aFile::Resolver::getSupportedFileExtensions() const
{
	return QStringList() << ".m4a" << ".m4b" << ".m4p" << ".mp4";
}

#endif // HAVE_MP4V2
