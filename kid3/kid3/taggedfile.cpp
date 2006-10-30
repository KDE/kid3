/**
 * \file taggedfile.cpp
 * Handling of tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
 */

#include <qdir.h>
#include <qstring.h>
#include <qregexp.h>

#include "standardtags.h"
#include "filelist.h"
#include "kid3.h"
#include "genres.h"
#include "taggedfile.h"

/**
 * Constructor.
 *
 * @param dn directory name
 * @param fn filename
 */
TaggedFile::TaggedFile(const QString& dn, const QString& fn) :
	dirname(dn), filename(fn), new_filename(fn)
{
	changedV1 = FALSE;
	changedV2 = FALSE;
}

/**
 * Destructor.
 */
TaggedFile::~TaggedFile()
{
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
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */
int TaggedFile::getGenreNumV1()
{
	return -1;
}

/**
 * Remove all ID3v1 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void TaggedFile::removeTagsV1(const StandardTagsFilter&)
{
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
 * Set ID3v1 genre.
 *
 * @param num number to set, 0xff to remove field.
 */
void TaggedFile::setGenreNumV1(int)
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

QString TaggedFile::getAbsFilename(void) const
{
	QDir dir(dirname);
	return QDir::cleanDirPath(dir.absFilePath(new_filename));
}

/**
 * Get ID3v1 tags from file.
 *
 * @param st tags to put result
 */

void TaggedFile::getStandardTagsV1(StandardTags *st)
{
	st->title = getTitleV1();
	st->artist = getArtistV1();
	st->album = getAlbumV1();
	st->comment = getCommentV1();
	st->year = getYearV1();
	st->track = getTrackNumV1();
	st->genre = getGenreNumV1();
	st->genreStr = QString::null;
}

/**
 * Get ID3v2 tags from file.
 *
 * @param st tags to put result
 */

void TaggedFile::getStandardTagsV2(StandardTags *st)
{
	st->title = getTitleV2();
	st->artist = getArtistV2();
	st->album = getAlbumV2();
	st->comment = getCommentV2();
	st->year = getYearV2();
	st->track = getTrackNumV2();
	st->genre = getGenreNumV2();
	st->genreStr = st->genre == 0xff ? getGenreV2() : QString::null;
}

/**
 * Set ID3v1 tags.
 *
 * @param st tags to set
 * @param flt filter specifying which fields to set
 */

void TaggedFile::setStandardTagsV1(const StandardTags *st,
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
		setGenreNumV1(st->genre);
	}
}

/**
 * Set ID3v2 tags.
 *
 * @param st tags to set
 * @param flt filter specifying which fields to set
 */

void TaggedFile::setStandardTagsV2(const StandardTags *st,
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
	if (flt.m_enableGenre &&
			(st->genre != oldst.genre || st->genreStr != oldst.genreStr)) {
		if (st->genre != 0xff) {
			setGenreNumV2(st->genre);
		} else {
			setGenreV2(st->genreStr);
		}
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
	int pos = album.find(" - ");
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
void TaggedFile::getTagsFromFilename(StandardTags *st, QString fmt)
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
		pos = pattern.find('%', pos);
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
	if (re.search(fileName) != -1) {
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
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->track = re.cap(2).toInt();
		st->artist = re.cap(3);
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// artist - album/track song
	re.setPattern("([^/]+)[_ ]-[_ ]([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// /artist - album - track - song
	re.setPattern("/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+[^-_/ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// album/artist - track - song
	re.setPattern("([^/]+)/([^/]+[^-_\\./ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// artist/album/track song
	re.setPattern("([^/]+)/([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\..{3,4}$");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// album/artist - song
	re.setPattern("([^/]+)/([^/]+[^-_/ ])[_ ]-[_ ]([^-_/ ][^/]+)\\..{3,4}$");
	if (re.search(fn) != -1) {
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
QString TaggedFile::formatWithTags(const StandardTags *st, QString fmt,
								   bool isDirname) const
{
	if (!isDirname) {
		// first remove directory part from fmt
		const int sepPos = fmt.findRev('/');
		if (sepPos >= 0) {
			fmt.remove(0, sepPos + 1);
		}
		// add extension to fmt
		fmt += getFileExtension();
	}

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
		insert_str[i] = "";
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
	for (int k = 0; k < i; ++k) {
		fmt = fmt.arg(insert_str[k]);
	}
	return fmt;
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
void TaggedFile::getFilenameFromTags(const StandardTags *st, QString fmt)
{
	new_filename = formatWithTags(st, fmt);
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
	if (fnNew.lower() == fnOld.lower()) {
		// if the filenames only differ in case, first rename to a
		// temporary filename, so that it works also with case
		// insensitive filesystems (e.g. Windows).
		QString temp_filename(fnNew);
		temp_filename.append("_CASE");
		if (!QDir(dirname).rename(fnOld, temp_filename)) {
			qDebug("rename(%s, %s) failed", fnOld.latin1(),
					   temp_filename.latin1());
			return false;
		}
		if (!QDir(dirname).rename(temp_filename, fnNew)) {
			qDebug("rename(%s, %s) failed", temp_filename.latin1(),
					   fnNew.latin1());
			return false;
		}
	} else if (!QFile::exists(dirname + QDir::separator() + fnNew)) {
		if (!QDir(dirname).rename(fnOld, fnNew)) {
			qDebug("rename(%s, %s) failed", fnOld.latin1(),
					   fnNew.latin1());
			return false;
		}
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
		numTracks = FileList::getNumberOfFiles();
	}
	return numTracks;
}

/**
 * Remove the standard ID3v1 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void TaggedFile::removeStandardTagsV1(const StandardTagsFilter& flt)
{
	if (flt.m_enableTitle)   setTitleV1("");
	if (flt.m_enableArtist)  setArtistV1("");
	if (flt.m_enableAlbum)   setAlbumV1("");
	if (flt.m_enableComment) setCommentV1("");
	if (flt.m_enableYear)    setYearV1(0);
	if (flt.m_enableTrack)   setTrackNumV1(0);
	if (flt.m_enableGenre)   setGenreNumV1(0xff);
}

/**
 * Remove the standard ID3v2 tags.
 *
 * @param flt filter specifying which fields to remove
 */
void TaggedFile::removeStandardTagsV2(const StandardTagsFilter& flt)
{
	if (flt.m_enableTitle)   setTitleV2("");
	if (flt.m_enableArtist)  setArtistV2("");
	if (flt.m_enableAlbum)   setAlbumV2("");
	if (flt.m_enableComment) setCommentV2("");
	if (flt.m_enableYear)    setYearV2(0);
	if (flt.m_enableTrack)   setTrackNumV2(0);
	if (flt.m_enableGenre)   setGenreNumV2(0xff);
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
