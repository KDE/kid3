/**
 * \file taggedfile.cpp
 * Handling of tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
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

#include <qdir.h>
#include <qstring.h>
#include <qregexp.h>

#include "standardtags.h"
#include "kid3.h"
#include "genres.h"
#include "dirinfo.h"
#include "taggedfile.h"
#include <sys/stat.h>

#if QT_VERSION >= 0x040000
QList<const TaggedFile::Resolver*> TaggedFile::s_resolvers;
#else
QValueList<const TaggedFile::Resolver*> TaggedFile::s_resolvers;
#endif

/**
 * Constructor.
 *
 * @param di directory information
 * @param fn filename
 */
TaggedFile::TaggedFile(const DirInfo* di, const QString& fn) :
	m_dirInfo(di), m_filename(fn), m_newFilename(fn),
	m_changedV1(false), m_changedV2(false), m_truncation(0)
{
}

/**
 * Destructor.
 */
TaggedFile::~TaggedFile()
{
}

/**
 * Get directory name.
 *
 * @return directory name
 */
QString TaggedFile::getDirname() const
{
	return m_dirInfo->getDirname();
}

/**
 * Get ID3v1 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getTitleV1()
{
	return QString::null;
}

/**
 * Get ID3v1 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getArtistV1()
{
	return QString::null;
}

/**
 * Get ID3v1 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getAlbumV1()
{
	return QString::null;
}

/**
 * Get ID3v1 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getCommentV1()
{
	return QString::null;
}

/**
 * Get ID3v1 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TaggedFile::getYearV1()
{
	return -1;
}

/**
 * Get ID3v1 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TaggedFile::getTrackNumV1()
{
	return -1;
}

/**
 * Get ID3v1 genre.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */
QString TaggedFile::getGenreV1()
{
	return QString::null;
}

/**
 * Remove ID3v1 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void TaggedFile::deleteFramesV1(const FrameFilter& flt)
{
	if (flt.isEnabled(Frame::FT_Title))   setTitleV1("");
	if (flt.isEnabled(Frame::FT_Artist))  setArtistV1("");
	if (flt.isEnabled(Frame::FT_Album))   setAlbumV1("");
	if (flt.isEnabled(Frame::FT_Comment)) setCommentV1("");
	if (flt.isEnabled(Frame::FT_Date))    setYearV1(0);
	if (flt.isEnabled(Frame::FT_Track))   setTrackNumV1(0);
	if (flt.isEnabled(Frame::FT_Genre))   setGenreV1("");
}

/**
 * Set ID3v1 title.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setTitleV1(const QString&)
{
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setArtistV1(const QString&)
{
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setAlbumV1(const QString&)
{
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */
void TaggedFile::setCommentV1(const QString&)
{
}

/**
 * Set ID3v1 year.
 *
 * @param num number to set, 0 to remove field.
 */
void TaggedFile::setYearV1(int)
{
}

/**
 * Set ID3v1 track.
 *
 * @param num number to set, 0 to remove field.
 */
void TaggedFile::setTrackNumV1(int)
{
}

/**
 * Set ID3v1 genre as text.
 *
 * @param str string to set, "" to remove field, QString::null to ignore.
 */
void TaggedFile::setGenreV1(const QString&)
{
}

/**
 * Check if file has an ID3v1 tag.
 *
 * @return true if a V1 tag is available.
 * @see isTagInformationRead()
 */
bool TaggedFile::hasTagV1() const
{
	return false;
}

/**
 * Check if ID3v1 tags are supported by the format of this file.
 *
 * @return true if V1 tags are supported.
 */
bool TaggedFile::isTagV1Supported() const
{
	return false;
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */
QString TaggedFile::getAbsFilename() const
{
	QDir dir(m_dirInfo->getDirname());
	return QDir::QCM_cleanPath(dir.QCM_absoluteFilePath(m_newFilename));
}

/**
 * Get ID3v1 tags from file.
 *
 * @param st tags to put result
 */
void TaggedFile::getStandardTagsV1(StandardTags* st)
{
	st->title = getTitleV1();
	st->artist = getArtistV1();
	st->album = getAlbumV1();
	st->comment = getCommentV1();
	st->year = getYearV1();
	st->track = getTrackNumV1();
	st->genre = getGenreV1();
}

/**
 * Get ID3v2 tags from file.
 *
 * @param st tags to put result
 */
void TaggedFile::getStandardTagsV2(StandardTags* st)
{
	st->title = getTitleV2();
	st->artist = getArtistV2();
	st->album = getAlbumV2();
	st->comment = getCommentV2();
	st->year = getYearV2();
	st->track = getTrackNumV2();
	st->genre = getGenreV2();
}

/**
 * Set ID3v1 tags.
 *
 * @param st tags to set
 * @param flt filter specifying which fields to set
 */
void TaggedFile::setStandardTagsV1(const StandardTags* st,
																	 const StandardTagsFilter& flt)
{
	StandardTags oldst;
	getStandardTagsV1(&oldst);
	if (flt.m_enableTitle && st->title != oldst.title) {
		setTitleV1(st->title);
	}
	if (flt.m_enableArtist && st->artist != oldst.artist) {
		setArtistV1(st->artist);
	}
	if (flt.m_enableAlbum && st->album != oldst.album) {
		setAlbumV1(st->album);
	}
	if (flt.m_enableComment && st->comment != oldst.comment) {
		setCommentV1(st->comment);
	}
	if (flt.m_enableYear && st->year != oldst.year) {
		setYearV1(st->year);
	}
	if (flt.m_enableTrack && st->track != oldst.track) {
		setTrackNumV1(st->track);
	}
	if (flt.m_enableGenre && st->genre != oldst.genre) {
		setGenreV1(st->genre);
	}
}

/**
 * Set ID3v2 tags.
 *
 * @param st tags to set
 * @param flt filter specifying which fields to set
 */
void TaggedFile::setStandardTagsV2(const StandardTags* st,
																	 const StandardTagsFilter& flt)
{
	StandardTags oldst;
	getStandardTagsV2(&oldst);
	if (flt.m_enableTitle && st->title != oldst.title) {
		setTitleV2(st->title);
	}
	if (flt.m_enableArtist && st->artist != oldst.artist) {
		setArtistV2(st->artist);
	}
	if (flt.m_enableAlbum && st->album != oldst.album) {
		setAlbumV2(st->album);
	}
	if (flt.m_enableComment && st->comment != oldst.comment) {
		setCommentV2(st->comment);
	}
	if (flt.m_enableYear && st->year != oldst.year) {
		setYearV2(st->year);
	}
	if (flt.m_enableTrack && st->track != oldst.track) {
		setTrackNumV2(st->track);
	}
	if (flt.m_enableGenre && st->genre != oldst.genre) {
		setGenreV2(st->genre);
	}
}

/**
 * Remove artist part from album string.
 * This is used when only the album is needed, but the regexp in
 * getTagsFromFilename() matched a "artist - album" string.
 *
 * @param album album string
 */
static void remove_artist(QString& album)
{
	int pos = album.QCM_indexOf(" - ");
	if (pos != -1) {
		album.remove(0, pos + 3);
	}
}

/**
 * Get tags from filename.
 * Supported formats:
 * with QT3:
 * album/track - artist - song
 * artist - album/track song
 * /artist - album - track - song
 * album/artist - track - song
 * artist/album/track song
 * album/artist - song
 *
 * with QT2 (only weak regexp support):
 * artist - album/track song
 *
 * @param st  tags to put result
 * @param fmt format string containing the following codes:
 *            %s title (song)
 *            %l album
 *            %a artist
 *            %c comment
 *            %y year
 *            %t track
 */
void TaggedFile::getTagsFromFilename(StandardTags* st, QString fmt)
{
	QRegExp re;
	QString fn(getAbsFilename());

	// construct regular expression from format string

	// if the format does not contain a '_', they are replaced by spaces
	// in the filename.
	QString fileName(fn);
	if (!fmt.contains('_')) {
#if QT_VERSION >= 0x030100
		fileName.replace(QChar('_'), QChar(' '));
#else
		fileName.replace(QRegExp("_"), QChar(' '));
#endif
	}

	// escape regexp characters
	QString pattern;
	uint fmtLen = fmt.length();
	static const QString escChars("+?.*^$()[]{}|\\");
	for (uint i = 0; i < fmtLen; ++i) {
		const QChar ch = fmt.at(i);
		if (escChars.contains(ch)) {
			pattern += '\\';
		}
		pattern += ch;
	}
	// and finally a dot followed by 3 or 4 characters for the extension
	pattern += "\\..{3,4}$";

	// replace %-codes with regexp capture strings:
	// %s title, %l album, %a artist, %c comment: string ([^-_\\./ ][^/]*[^-_/ ])
	// %t track, %y year: number (\\d{1,4})
	static const int numTagCodes = 6;
	static const char tagCodes[numTagCodes] = {'s', 'l', 'a', 'c', 't', 'y' };
	int idxOfCode[numTagCodes];
	for (int k = 0; k < numTagCodes; ++k) {
		idxOfCode[k] = 0;
	}
	int pos = 0;
	for (int j = 0; j < numTagCodes; ++j) {
		pos = pattern.QCM_indexOf('%', pos);
		if (pos == -1) break;
		++pos;
		for (int k = 0; k < numTagCodes; ++k) {
			if (pattern[pos] == tagCodes[k]) {
				// code found, insert regexp capture
				if (k < 4) {
					static const char capStr[] = "([^-_\\./ ][^/]*[^-_/ ])";
					pattern.replace(--pos, 2, capStr);
					pos += sizeof(capStr) - 1;
				} else {
					static const char capStr[] = "(\\d{1,4})";
					pattern.replace(--pos, 2, capStr);
					pos += sizeof(capStr) - 1;
				}
				idxOfCode[k] = j + 1;
				break;
			}
		}
	}

	re.setPattern(pattern);
	if (re.QCM_indexIn(fileName) != -1) {
		if (idxOfCode[0])
			st->title = re.cap(idxOfCode[0]);
		if (idxOfCode[1])
			st->album = re.cap(idxOfCode[1]);
		if (idxOfCode[2])
			st->artist = re.cap(idxOfCode[2]);
		if (idxOfCode[3])
			st->comment = re.cap(idxOfCode[3]);
		if (idxOfCode[4])
			st->track = re.cap(idxOfCode[4]).toInt();
		if (idxOfCode[5])
			st->year = re.cap(idxOfCode[5]).toInt();
		return;
	}

	// album/track - artist - song
	re.setPattern("([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)[_ ]-[_ ]([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.QCM_indexIn(fn) != -1) {
		st->album = re.cap(1);
		st->track = re.cap(2).toInt();
		st->artist = re.cap(3);
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// artist - album/track song
	re.setPattern("([^/]+)[_ ]-[_ ]([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.QCM_indexIn(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// /artist - album - track - song
	re.setPattern("/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+[^-_/ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.QCM_indexIn(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// album/artist - track - song
	re.setPattern("([^/]+)/([^/]+[^-_\\./ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.QCM_indexIn(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// artist/album/track song
	re.setPattern("([^/]+)/([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.QCM_indexIn(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// album/artist - song
	re.setPattern("([^/]+)/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+)\\..{3,4}$");
	if (re.QCM_indexIn(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->title = re.cap(3);
		remove_artist(st->album);
		return;
	}
}

/**
 * Create string with tags according to format string.
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
 * @param isDirname true to generate a directory name
 *
 * @return format string with format codes replaced by tags.
 */
QString TaggedFile::formatWithTags(const StandardTags* st, QString fmt,
								   bool isDirname) const
{
	if (!isDirname) {
		// first remove directory part from fmt
		const int sepPos = fmt.QCM_lastIndexOf('/');
		if (sepPos >= 0) {
			fmt.remove(0, sepPos + 1);
		}
		// add extension to fmt
		fmt += getFileExtension();
	}

	return st->formatString(fmt, isDirname ?
													StandardTags::FSF_ReplaceSeparators : 0);
}

/**
 * Get filename from tags.
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
void TaggedFile::getFilenameFromTags(const StandardTags* st, QString fmt)
{
	m_newFilename = formatWithTags(st, fmt);
}

/**
 * Format a time string "h:mm:ss".
 * If the time is less than an hour, the hour is not put into the
 * string and the minute is not padded with zeroes.
 *
 * @param seconds time in seconds
 *
 * @return string with the time in hours, minutes and seconds.
 */
QString TaggedFile::formatTime(unsigned seconds)
{
	unsigned hours = seconds / 3600;
	seconds %= 3600;
	unsigned minutes = seconds / 60;
	seconds %= 60;
	QString timeStr;
	if (hours > 0) {
		timeStr.sprintf("%u:%02u:%02u", hours, minutes, seconds);
	} else {
		timeStr.sprintf("%u:%02u", minutes, seconds);
	}
	return timeStr;
}

/**
 * Rename a file.
 * This methods takes care of case insensitive filesystems.
 *
 * @param fnOld old filename
 * @param fnNew new filename
 *
 * @return true if ok.
 */
bool TaggedFile::renameFile(const QString& fnOld, const QString& fnNew) const
{
	QString dirname = m_dirInfo->getDirname();
	if (fnNew.QCM_toLower() == fnOld.QCM_toLower()) {
		// If the filenames only differ in case, the new file is reported to
		// already exist on case insensitive filesystems (e.g. Windows),
		// so it is checked if the new file is really the old file by
		// comparing inodes and devices. If the files are not the same,
		// another file would be overwritten and an error is reported.
		if (QFile::exists(dirname + QDir::separator() + fnNew)) {
			struct stat statOld, statNew;
			if (::stat((dirname + QDir::separator() + fnOld).QCM_latin1(), &statOld) == 0 &&
					::stat((dirname + QDir::separator() + fnNew).QCM_latin1(), &statNew) == 0 &&
					!(statOld.st_ino == statNew.st_ino &&
						statOld.st_dev == statNew.st_dev)) {
				qDebug("rename(%s, %s): %s already exists", fnOld.QCM_latin1(),
							 fnNew.QCM_latin1(), fnNew.QCM_latin1());
				return false;
			}
		}

		// if the filenames only differ in case, first rename to a
		// temporary filename, so that it works also with case
		// insensitive filesystems (e.g. Windows).
		QString temp_filename(fnNew);
		temp_filename.append("_CASE");
		if (!QDir(dirname).rename(fnOld, temp_filename)) {
			qDebug("rename(%s, %s) failed", fnOld.QCM_latin1(),
					   temp_filename.QCM_latin1());
			return false;
		}
		if (!QDir(dirname).rename(temp_filename, fnNew)) {
			qDebug("rename(%s, %s) failed", temp_filename.QCM_latin1(),
					   fnNew.QCM_latin1());
			return false;
		}
	} else if (QFile::exists(dirname + QDir::separator() + fnNew)) {
		qDebug("rename(%s, %s): %s already exists", fnOld.QCM_latin1(),
					 fnNew.QCM_latin1(), fnNew.QCM_latin1());
		return false;
	} else if (!QDir(dirname).rename(fnOld, fnNew)) {
		qDebug("rename(%s, %s) failed", fnOld.QCM_latin1(),
					 fnNew.QCM_latin1());
		return false;
	}
	return true;
}

/**
 * Get field name for comment from configuration.
 *
 * @return field name.
 */
QString TaggedFile::getCommentFieldName() const
{
	return Kid3App::s_miscCfg.m_commentName;
}

/**
 * Get the total number of tracks if it is enabled.
 *
 * @return total number of tracks,
 *         -1 if disabled or unavailable.
 */
int TaggedFile::getTotalNumberOfTracksIfEnabled() const
{
	int numTracks = -1;
	if (Kid3App::s_miscCfg.m_enableTotalNumberOfTracks) {
		numTracks = m_dirInfo->getNumFiles();
	}
	return numTracks;
}

/**
 * Add the total number of tracks to the track number if enabled.
 *
 * @param value string containing track number,
 *              "/t" with t = total number of tracks will be appended
 *              if enabled and value contains a number
 */
void TaggedFile::addTotalNumberOfTracksIfEnabled(QString& value) const
{
	int numTracks = getTotalNumberOfTracksIfEnabled();
	if (numTracks > 0) {
		bool ok;
		int trackNr = value.toInt(&ok);
		if (ok && trackNr > 0) {
			value = QString("%1/%2").arg(trackNr).arg(numTracks);
		}
	}
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void TaggedFile::deleteFramesV2(const FrameFilter& flt)
{
	if (flt.isEnabled(Frame::FT_Title))   setTitleV2("");
	if (flt.isEnabled(Frame::FT_Artist))  setArtistV2("");
	if (flt.isEnabled(Frame::FT_Album))   setAlbumV2("");
	if (flt.isEnabled(Frame::FT_Comment)) setCommentV2("");
	if (flt.isEnabled(Frame::FT_Date))    setYearV2(0);
	if (flt.isEnabled(Frame::FT_Track))   setTrackNumV2(0);
	if (flt.isEnabled(Frame::FT_Genre))   setGenreV2("");
}

/**
 * Get the format of tag 1.
 *
 * @return string describing format of tag 1,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TaggedFile::getTagFormatV1() const
{
	return QString::null;
}

/**
 * Get the format of tag 2.
 *
 * @return string describing format of tag 2,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TaggedFile::getTagFormatV2() const
{
	return QString::null;
}

/**
 * Check if a string has to be truncated.
 *
 * @param str  string to be checked
 * @param flag flag to be set if string has to be truncated
 * @param len  maximum length of string
 *
 * @return str truncated to len characters if necessary, else QString::null.
 */
QString TaggedFile::checkTruncation(
	const QString& str, StandardTags::TruncationFlag flag, int len)
{
	if (static_cast<int>(str.length()) > len) {
		QString s = str;
		s.truncate(len);
		m_truncation |= flag;
		return s;
	} else {
		m_truncation &= ~flag;
		return QString::null;
	}
}

/**
 * Check if a number has to be truncated.
 *
 * @param val  value to be checked
 * @param flag flag to be set if number has to be truncated
 * @param max  maximum value
 *
 * @return val truncated to max if necessary, else -1.
 */
int TaggedFile::checkTruncation(int val, StandardTags::TruncationFlag flag,
																int max)
{
	if (val > max) {
		m_truncation |= flag;
		return max;
	} else {
		m_truncation &= ~flag;
		return -1;
	}
}

/**
 * Get a specific frame from the tags 1.
 *
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool TaggedFile::getFrameV1(Frame::Type type, Frame& frame)
{
	int n = -1;
	bool number = false;

	switch (type) {
		case Frame::FT_Album:
			frame.m_value = getAlbumV1();
			break;
		case Frame::FT_Artist:
			frame.m_value = getArtistV1();
			break;
		case Frame::FT_Comment:
			frame.m_value = getCommentV1();
			break;
		case Frame::FT_Date:
			n = getYearV1();
			number = true;
			break;
		case Frame::FT_Genre:
			frame.m_value = getGenreV1();
			break;
		case Frame::FT_Title:
			frame.m_value = getTitleV1();
			break;
		case Frame::FT_Track:
			n = getTrackNumV1();
			number = true;
			break;
		default:
			// maybe handled in a subclass
			return false;
	}
	if (number) {
		if (n == -1) {
			frame.m_value = QString();
		} else if (n == 0) {
			frame.m_value = QString("");
		} else {
			frame.m_value.setNum(n);
		}
	}
	frame.m_type = type;
	return true;
}

/**
 * Set a frame in the tags 1.
 *
 * @param frame frame to set.
 *
 * @return true if ok.
 */
bool TaggedFile::setFrameV1(const Frame& frame)
{
	int n = -1;
	if (frame.m_type == Frame::FT_Date ||
			frame.m_type == Frame::FT_Track) {
		if (frame.isInactive()) {
			n = -1;
		} else if (frame.isEmpty()) {
			n = 0;
		} else {
			n = frame.m_value.toInt();
		} 
	}
	switch (frame.m_type) {
		case Frame::FT_Album:
			setAlbumV1(frame.m_value);
			break;
		case Frame::FT_Artist:
			setArtistV1(frame.m_value);
			break;
		case Frame::FT_Comment:
			setCommentV1(frame.m_value);
			break;
		case Frame::FT_Date:
			setYearV1(n);
			break;
		case Frame::FT_Genre:
			setGenreV1(frame.m_value);
			break;
		case Frame::FT_Title:
			setTitleV1(frame.m_value);
			break;
		case Frame::FT_Track:
			setTrackNumV1(n);
			break;
		default:
			// maybe handled in a subclass
			return false;
	}
	return true;
}

/**
 * Get a specific frame from the tags 2.
 *
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool TaggedFile::getFrameV2(Frame::Type type, Frame& frame)
{
	int n = -1;
	bool number = false;

	switch (type) {
		case Frame::FT_Album:
			frame.m_value = getAlbumV2();
			break;
		case Frame::FT_Artist:
			frame.m_value = getArtistV2();
			break;
		case Frame::FT_Comment:
			frame.m_value = getCommentV2();
			break;
		case Frame::FT_Date:
			n = getYearV2();
			number = true;
			break;
		case Frame::FT_Genre:
			frame.m_value = getGenreV2();
			break;
		case Frame::FT_Title:
			frame.m_value = getTitleV2();
			break;
		case Frame::FT_Track:
			n = getTrackNumV2();
			number = true;
			break;
		default:
			// maybe handled in a subclass
			return false;
	}
	if (number) {
		if (n == -1) {
			frame.m_value = QString();
		} else if (n == 0) {
			frame.m_value = QString("");
		} else {
			frame.m_value.setNum(n);
		}
	}
	frame.m_type = type;
	return true;
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool TaggedFile::setFrameV2(const Frame& frame)
{
	int n = -1;
	if (frame.m_type == Frame::FT_Date ||
			frame.m_type == Frame::FT_Track) {
		if (frame.isInactive()) {
			n = -1;
		} else if (frame.isEmpty()) {
			n = 0;
		} else {
			n = frame.m_value.toInt();
		} 
	}
	switch (frame.m_type) {
		case Frame::FT_Album:
			setAlbumV2(frame.m_value);
			break;
		case Frame::FT_Artist:
			setArtistV2(frame.m_value);
			break;
		case Frame::FT_Comment:
			setCommentV2(frame.m_value);
			break;
		case Frame::FT_Date:
			setYearV2(n);
			break;
		case Frame::FT_Genre:
			setGenreV2(frame.m_value);
			break;
		case Frame::FT_Title:
			setTitleV2(frame.m_value);
			break;
		case Frame::FT_Track:
			setTrackNumV2(n);
			break;
		default:
			// maybe handled in a subclass
			return false;
	}
	return true;
}

/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool TaggedFile::addFrameV2(Frame& frame)
{
	return TaggedFile::setFrameV2(frame);
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool TaggedFile::deleteFrameV2(const Frame& frame)
{
	Frame emptyFrame(frame);
	emptyFrame.setValue("");
	return setFrameV2(emptyFrame);
}

/**
 * Get all frames in tag 1.
 *
 * @param frames frame collection to set.
 */
void TaggedFile::getAllFramesV1(FrameCollection& frames)
{
	frames.clear();
	Frame frame;
	for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
		if (getFrameV1(static_cast<Frame::Type>(i), frame)) {
			frames.insert(frame);
		}
	}
}

/**
 * Set frames in tag 1.
 *
 * @param frames      frame collection
 * @param onlyChanged only frames with value marked as changed are set
 */
void TaggedFile::setFramesV1(const FrameCollection& frames, bool onlyChanged)
{
	for (FrameCollection::const_iterator it = frames.begin();
			 it != frames.end();
			 ++it) {
		if (!onlyChanged || it->isValueChanged()) {
				setFrameV1(*it);
				markTag1Changed();
		}
	}
}

/**
 * Get all frames in tag 2.
 * This generic implementation only supports the standard tags and should
 * be reimplemented in derived classes.
 *
 * @param frames frame collection to set.
 */
void TaggedFile::getAllFramesV2(FrameCollection& frames)
{
	frames.clear();
	Frame frame;
	for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastV1Frame; ++i) {
		if (getFrameV2(static_cast<Frame::Type>(i), frame)) {
			frames.insert(frame);
		}
	}
}

/**
 * Set frames in tag 2.
 *
 * @param frames      frame collection
 * @param onlyChanged only frames with value marked as changed are set
 */
void TaggedFile::setFramesV2(const FrameCollection& frames, bool onlyChanged)
{
	bool myFramesValid = false;
	FrameCollection myFrames;

	for (FrameCollection::const_iterator it = frames.begin();
			 it != frames.end();
			 ++it) {
		if (!onlyChanged || it->isValueChanged()) {
			if (it->getIndex() != -1) {
				// The frame has an index, so the original tag can be modified
				setFrameV2(*it);
				markTag2Changed();
			} else {
				// The frame does not have an index
				if (it->getType() <= Frame::FT_LastV1Frame) {
					// Standard tags can be handled with the basic method
					TaggedFile::setFrameV2(*it);
					markTag2Changed();
				} else {
					// The frame has to be looked up and modified
					if (!myFramesValid) {
						getAllFramesV2(myFrames);
						myFramesValid = true;
					}
					FrameCollection::iterator myIt = myFrames.find(*it);
					if (myIt != myFrames.end()) {
						Frame myFrame(*it);
						myFrame.setIndex(myIt->getIndex());
						setFrameV2(myFrame);
						markTag2Changed();
					} else {
						// Such a frame does not exist, add a new one.
						Frame myFrame(*it);
						addFrameV2(myFrame);
						setFrameV2(myFrame);
						markTag2Changed();
					}
				}
			}
		}
	}
}

/**
 * Get a list of frame IDs which can be added.
 *
 * @return list with frame IDs.
 */
QStringList TaggedFile::getFrameIds() const
{
	QStringList lst;
	for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastFrame; ++i) {
		lst.append(QCM_translate(Frame::getNameFromType(static_cast<Frame::Type>(i))));
	}
	return lst;
}


/**
 * Add a file type resolver to the end of a list of resolvers.
 *
 * @param resolver file type resolver to add
 */
void TaggedFile::addResolver(const Resolver* resolver)
{
	s_resolvers.push_back(resolver);
}

/**
 * Create a TaggedFile subclass using the first successful resolver.
 * @see addResolver()
 *
 * @param di directory information
 * @param fn filename
 *
 * @return tagged file, 0 if type not supported.
 */
TaggedFile* TaggedFile::createFile(const DirInfo* di, const QString& fn)
{
	TaggedFile* taggedFile = 0;
	for (
#if QT_VERSION >= 0x040000
		QList<const Resolver*>::const_iterator
#else
		QValueList<const Resolver*>::const_iterator
#endif
			it = s_resolvers.begin(); it != s_resolvers.end(); ++it) {
		taggedFile = (*it)->createFile(di, fn);
		if (taggedFile) break;
	}
	return taggedFile;
}

/**
 * Get a list with all extensions (e.g. ".mp3") supported by the resolvers.
 * @see addResolver()
 *
 * @return list of file extensions.
 */
QStringList TaggedFile::getSupportedFileExtensions()
{
	QStringList extensions;
	for (
#if QT_VERSION >= 0x040000
		QList<const Resolver*>::const_iterator
#else
		QValueList<const Resolver*>::const_iterator
#endif
			it = s_resolvers.begin(); it != s_resolvers.end(); ++it) {
		extensions += (*it)->getSupportedFileExtensions();
	}

	// remove duplicates
	extensions.sort();
	QString lastExt("");
	for (QStringList::iterator it = extensions.begin();
			 it != extensions.end();) {
		if (*it == lastExt) {
			it = extensions.erase(it);
		} else {
			lastExt = *it;
			++it;
		}
	}

	return extensions;
}

/**
 * Free static resources.
 */
void TaggedFile::staticCleanup()
{
#if QT_VERSION >= 0x040000
	qDeleteAll(s_resolvers);
#else
	for (QValueList<const Resolver*>::const_iterator it = s_resolvers.begin();
			 it != s_resolvers.end();
			 ++it) {
		delete *it;
	}
#endif
	s_resolvers.clear();
}
