/**
 * \file mp3file.cpp
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include <qdir.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qregexp.h>

#include <id3/tag.h>
#include <id3/misc_support.h>

#include "standardtags.h"
#include "genres.h"
#include "mp3file.h"

/**
 * Constructor.
 *
 * @param dn directory name
 * @param fn filename
 */

Mp3File::Mp3File(const QString& dn, const QString& fn) :
	QListBoxText(fn), dirname(dn), filename(fn), new_filename(fn)
{
	setInSelection(FALSE);
	tagV1 = 0;
	tagV2 = 0;
	changedV1 = FALSE;
	changedV2 = FALSE;
}

/**
 * Destructor.
 */

Mp3File::~Mp3File(void)
{
	if (tagV1) {
		delete tagV1;
	}
	if (tagV2) {
		delete tagV2;
	}
}

/**
 * Append asterisk to text if file was changed.
 */

void Mp3File::refreshText(void)
{
	QString txt(filename);
	if (isChanged()) {
		txt.append(" *");
	}
	if (text() != txt) {
		setText(txt);
	}
}

/**
 * Get absolute filename.
 *
 * @return absolute file path.
 */

QString Mp3File::getAbsFilename(void) const
{
	QDir dir(dirname);
	return QDir::cleanDirPath(dir.absFilePath(new_filename));
}

/**
 * Read tags from file.
 *
 * @param force TRUE to force reading even if tags were already read.
 */

void Mp3File::readTags(bool force)
{
	QString absfn = dirname + QDir::separator() + filename;
	const char *fn = absfn.latin1();

	if (force && tagV1) {
		tagV1->Clear();
		tagV1->Link(fn, ID3TT_ID3V1);
		changedV1 = FALSE;
	}
	if (!tagV1) {
		tagV1 = new ID3_Tag;
		if (tagV1) {
			tagV1->Link(fn, ID3TT_ID3V1);
			changedV1 = FALSE;
		}
	}

	if (force && tagV2) {
		tagV2->Clear();
		tagV2->Link(fn, ID3TT_ID3V2);
		changedV2 = FALSE;
	}
	if (!tagV2) {
		tagV2 = new ID3_Tag;
		if (tagV2) {
			tagV2->Link(fn, ID3TT_ID3V2);
			changedV2 = FALSE;
		}
	}

	if (force) {
		new_filename = filename;
	}
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force TRUE to force writing even if file was not changed.
 *
 * @return TRUE if the file was renamed, i.e. the file name is no longer valid.
 */

bool Mp3File::writeTags(bool force)
{
	if (tagV1 && (force || changedV1)) {
		tagV1->Update(ID3TT_ID3V1);
		// There seems to be a bug in id3lib: The V1 genre is not
		// removed. So we check here and strip the whole header
		// if there are no frames.
		if (tagV1->NumFrames() == 0) {
			tagV1->Strip(ID3TT_ID3V1);
		}
		changedV1 = FALSE;
	}
	if (tagV2 && (force || changedV2)) {
		tagV2->Update(ID3TT_ID3V2);
		changedV2 = FALSE;
	}
	if ((new_filename != filename) &&
		!QFile::exists(dirname + QDir::separator() + new_filename)) {
		if (!QDir(dirname).rename(filename, new_filename)) {
			qDebug("rename(%s, %s) failed", filename.latin1(),
			       new_filename.latin1());
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Remove all ID3v1 tags.
 */

void Mp3File::removeTagsV1(void)
{
	if (tagV1) {
		ID3_Tag::Iterator* iter = tagV1->CreateIterator();
		ID3_Frame* frame;
		while ((frame = iter->GetNext()) != NULL) {
			tagV1->RemoveFrame(frame);
		}
		delete iter;
		changedV1 = TRUE;
	}
}

/**
 * Remove all ID3v2 tags.
 */

void Mp3File::removeTagsV2(void)
{
	if (tagV2) {
		ID3_Tag::Iterator* iter = tagV2->CreateIterator();
		ID3_Frame* frame;
		while ((frame = iter->GetNext()) != NULL) {
			tagV2->RemoveFrame(frame);
		}
		delete iter;
		changedV2 = TRUE;
	}
}

/**
 * Generate code to get text field.
 * QString::null is returned if the field does not exist.
 * Last line before return is "ID3_FreeString(str); \" in
 * later versions of id3lib. 
 *
 * @param name field name (Title, Artist, Album, Comment)
 * @param version ID3 version (V1, V2)
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

#define GET_TEXT_FIELD(name, version) \
	if (!tag##version) { \
		return QString::null; \
	} \
	char *str = ID3_Get##name(tag##version); \
	QString result = str ? str : ""; \
	if (str != NULL) delete [] str; \
	return result

/**
 * Get ID3v1 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getTitleV1(void)
{
	GET_TEXT_FIELD(Title, V1);
}

/**
 * Get ID3v1 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getArtistV1(void)
{
	GET_TEXT_FIELD(Artist, V1);
}

/**
 * Get ID3v1 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getAlbumV1(void)
{
	GET_TEXT_FIELD(Album, V1);
}

/**
 * Get ID3v1 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getCommentV1(void)
{
	GET_TEXT_FIELD(Comment, V1);
}

/**
 * Get ID3v1 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getYearV1(void)
{
	if (!tagV1) {
		return -1;
	}
	char *str = ID3_GetYear(tagV1);
	if (str) {
		QString result(str);
		delete [] str; // ID3_FreeString(str);
		return result.toInt();
	}
	return 0;
}

/**
 * Get ID3v1 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getTrackNumV1(void)
{
	if (!tagV1) {
		return -1;
	}
	char *str = ID3_GetTrack(tagV1);
	if (str) {
		QString result(str);
		delete [] str; // ID3_FreeString(str);
		return result.toInt();
	}
	return 0;
}

/**
 * Get ID3v1 genre.
 *
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getGenreNumV1(void)
{
	if (!tagV1) {
		return -1;
	}
	int n = (int)ID3_GetGenreNum(tagV1);
	return (n < 0xff) ? n : 0xff;
}

/**
 * Get ID3v2 title.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getTitleV2(void)
{
	GET_TEXT_FIELD(Title, V2);
}

/**
 * Get ID3v2 artist.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getArtistV2(void)
{
	GET_TEXT_FIELD(Artist, V2);
}

/**
 * Get ID3v2 album.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getAlbumV2(void)
{
	GET_TEXT_FIELD(Album, V2);
}

/**
 * Get ID3v2 comment.
 *
 * @return string,
 *         "" if the field does not exist,
 *         QString::null if the tags do not exist.
 */

QString Mp3File::getCommentV2(void)
{
	GET_TEXT_FIELD(Comment, V2);
}

/**
 * Get ID3v2 year.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getYearV2(void)
{
	if (!tagV2) {
		return -1;
	}
	char *str = ID3_GetYear(tagV2);
	if (str) {
		QString result(str);
		delete [] str; // ID3_FreeString(str);
		return result.toInt();
	}
	return 0;
}

/**
 * Get ID3v2 track.
 *
 * @return number,
 *         0 if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getTrackNumV2(void)
{
	if (!tagV2) {
		return -1;
	}
	char *str = ID3_GetTrack(tagV2);
	if (str) {
		QString result(str);
		delete [] str; // ID3_FreeString(str);
		return result.toInt();
	}
	return 0;
}

/**
 * Get ID3v2 genre.
 *
 * @return number,
 *         0xff if the field does not exist,
 *         -1 if the tags do not exist.
 */

int Mp3File::getGenreNumV2(void)
{
	if (!tagV2) {
		return -1;
	}
	int n = (int)ID3_GetGenreNum(tagV2);
	return (n < 0xff) ? n : 0xff;
}

/**
 * Get ID3v1 tags from file.
 *
 * @param st tags to put result
 */

void Mp3File::getStandardTagsV1(StandardTags *st)
{
	st->title = getTitleV1();
	st->artist = getArtistV1();
	st->album = getAlbumV1();
	st->comment = getCommentV1();
	st->year = getYearV1();
	st->track = getTrackNumV1();
	st->genre = getGenreNumV1();
}

/**
 * Get ID3v2 tags from file.
 *
 * @param st tags to put result
 */

void Mp3File::getStandardTagsV2(StandardTags *st)
{
	st->title = getTitleV2();
	st->artist = getArtistV2();
	st->album = getAlbumV2();
	st->comment = getCommentV2();
	st->year = getYearV2();
	st->track = getTrackNumV2();
	st->genre = getGenreNumV2();
}

/**
 * Generate code to set text field.
 * QString::null is returned if the field does not exist.
 *
 * @param name field name (Title, Artist, Album, Comment)
 * @param version ID3 version (V1, V2)
 * @param str (QString variable) string to set, "" to remove field.
 */

#define SET_TEXT_FIELD(name, version) \
	if (tag##version && !str.isNull()) { \
		if (str.isEmpty()) \
			ID3_Remove##name##s(tag##version); \
		else \
			ID3_Add##name(tag##version, str.latin1(), TRUE); \
		changed##version = TRUE; \
	}

/**
 * Set ID3v1 title.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setTitleV1(const QString& str)
{
	SET_TEXT_FIELD(Title, V1);
}

/**
 * Set ID3v1 artist.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setArtistV1(const QString& str)
{
	SET_TEXT_FIELD(Artist, V1);
}

/**
 * Set ID3v1 album.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setAlbumV1(const QString& str)
{
	SET_TEXT_FIELD(Album, V1);
}

/**
 * Set ID3v1 comment.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setCommentV1(const QString& str)
{
	SET_TEXT_FIELD(Comment, V1);
}

/**
 * Set ID3v1 year.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setYearV1(int num)
{
	if (tagV1 && num >= 0) {
		if (num == 0) {
			ID3_RemoveYears(tagV1);
		}
		else {
			QString str;
			str.setNum(num);
			ID3_AddYear(tagV1, str.latin1(), TRUE);
		}
		changedV1 = TRUE;
	}
}

/**
 * Set ID3v1 track.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setTrackNumV1(int num)
{
	if (tagV1 && num >= 0) {
		if (num == 0) {
			ID3_RemoveTracks(tagV1);
		}
		else {
			ID3_AddTrack(tagV1, (uchar)num, 0, TRUE);
		}
		changedV1 = TRUE;
	}
}

/**
 * Set ID3v1 genre.
 *
 * @param num number to set, 0xff to remove field.
 */

void Mp3File::setGenreNumV1(int num)
{
	if (tagV1 && num >= 0) {
		if (num == 0xff) {
			ID3_RemoveGenres(tagV1);
		}
		else {
			ID3_AddGenre(tagV1, (size_t)num, TRUE);
		}
		changedV1 = TRUE;
	}
}

/**
 * Set ID3v2 title.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setTitleV2(const QString& str)
{
	SET_TEXT_FIELD(Title, V2);
}

/**
 * Set ID3v2 artist.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setArtistV2(const QString& str)
{
	SET_TEXT_FIELD(Artist, V2);
}

/**
 * Set ID3v2 album.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setAlbumV2(const QString& str)
{
	SET_TEXT_FIELD(Album, V2);
}

/**
 * Set ID3v2 comment.
 *
 * @param str string to set, "" to remove field.
 */

void Mp3File::setCommentV2(const QString& str)
{
	SET_TEXT_FIELD(Comment, V2);
}

/**
 * Set ID3v2 year.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setYearV2(int num)
{
	if (tagV2 && num >= 0) {
		if (num == 0) {
			ID3_RemoveYears(tagV2);
		}
		else {
			QString str;
			str.setNum(num);
			ID3_AddYear(tagV2, str.latin1(), TRUE);
		}
		changedV2 = TRUE;
	}
}

/**
 * Set ID3v2 track.
 *
 * @param num number to set, 0 to remove field.
 */

void Mp3File::setTrackNumV2(int num)
{
	if (tagV2 && num >= 0) {
		if (num == 0) {
			ID3_RemoveTracks(tagV2);
		}
		else {
			ID3_AddTrack(tagV2, (uchar)num, 0, TRUE);
		}
		changedV2 = TRUE;
	}
}

/**
 * Set ID3v2 genre.
 *
 * @param num number to set, 0xff to remove field.
 */

void Mp3File::setGenreNumV2(int num)
{
	if (tagV2 && num >= 0) {
		if (num == 0xff) {
			ID3_RemoveGenres(tagV2);
		}
		else {
			ID3_AddGenre(tagV2, (size_t)num, TRUE);
		}
		changedV2 = TRUE;
	}
}

/**
 * Set ID3v1 tags.
 *
 * @param st tags to set
 */

void  Mp3File::setStandardTagsV1(const StandardTags *st)
{
	StandardTags oldst;
	getStandardTagsV1(&oldst);
	if (st->title != oldst.title) {
		setTitleV1(st->title);
	}
	if (st->artist != oldst.artist) {
		setArtistV1(st->artist);
	}
	if (st->album != oldst.album) {
		setAlbumV1(st->album);
	}
	if (st->comment != oldst.comment) {
		setCommentV1(st->comment);
	}
	if (st->year != oldst.year) {
		setYearV1(st->year);
	}
	if (st->track != oldst.track) {
		setTrackNumV1(st->track);
	}
	if (st->genre != oldst.genre) {
		setGenreNumV1(st->genre);
	}
}

/**
 * Set ID3v2 tags.
 *
 * @param st tags to set
 */

void  Mp3File::setStandardTagsV2(const StandardTags *st)
{
	StandardTags oldst;
	getStandardTagsV2(&oldst);
	if (st->title != oldst.title) {
		setTitleV2(st->title);
	}
	if (st->artist != oldst.artist) {
		setArtistV2(st->artist);
	}
	if (st->album != oldst.album) {
		setAlbumV2(st->album);
	}
	if (st->comment != oldst.comment) {
		setCommentV2(st->comment);
	}
	if (st->year != oldst.year) {
		setYearV2(st->year);
	}
	if (st->track != oldst.track) {
		setTrackNumV2(st->track);
	}
	if (st->genre != oldst.genre) {
		setGenreNumV2(st->genre);
	}
}

#if QT_VERSION >= 300
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
#endif

/**
 * Get tags from filename.
 * Supported formats:
 * with QT3:
 * album/track - artist - song.mp3
 * artist - album/track song.mp3
 * /artist - album - track - song.mp3
 * album/artist - track - song.mp3
 * album/artist - song.mp3
 *
 * with QT2 (only weak regexp support):
 * artist - album/track song.mp3
 *
 * @param st tags to put result
 */

void Mp3File::getTagsFromFilename(StandardTags *st)
{
#if QT_VERSION < 300
	int start, end;
	const QString fn(getAbsFilename());
	QRegExp re("[^/]+ - [^/]+/\\d\\d?\\d?[- ]+[^/]+\\.mp3");
	start = re.match(fn);
	if (start != -1) {
		end = fn.find(" - ", start);
		st->artist = fn.mid(start, end - start);
		start = end + 3;
		end = fn.find("/", start);
		st->album = fn.mid(start, end - start);
		start = end + 1;
		for (end = start + 1;; end++) {
			if (fn[end] < '0' || fn[end] > '9') break;
		}
		st->track = fn.mid(start, end - start).toInt();
		for (;; end++) {
			if (fn[end] != ' ' && fn[end] != '-') break;
		}
		start = end;
		end = fn.find(".mp3", start);
		st->title = fn.mid(start, end - start);
	}
#else
	QRegExp re;
	QString fn(getAbsFilename());
	// album/track - artist - song.mp3
	re.setPattern("([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+) - ([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->track = re.cap(2).toInt();
		st->artist = re.cap(3);
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// artist - album/track song.mp3
	re.setPattern("([^/]+) - ([^/]+)/(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// /artist - album - track - song.mp3
	re.setPattern("/([^/]+[^-_/ ]) - ([^-_/ ][^/]+[^-_/ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->artist = re.cap(1);
		st->album = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		return;
	}
	// album/artist - track - song.mp3
	re.setPattern("([^/]+)/([^/]+[^-_\\./ ])[-_\\. ]+(\\d{1,3})[-_\\. ]+([^-_\\./ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->track = re.cap(3).toInt();
		st->title = re.cap(4);
		remove_artist(st->album);
		return;
	}
	// album/artist - song.mp3
	re.setPattern("([^/]+)/([^/]+[^-_/ ]) - ([^-_/ ][^/]+)\\.mp3");
	if (re.search(fn) != -1) {
		st->album = re.cap(1);
		st->artist = re.cap(2);
		st->title = re.cap(3);
		remove_artist(st->album);
		return;
	}
#endif
}

static const char *fnFmt[] = {
	"%t %s.mp3",
	"%t. %s.mp3",
	"%a - %s.mp3",
	"%a-%s.mp3",
	"%a_%s.mp3",
	"(%a) %s.mp3",
	"%t. %a - %s.mp3",
	"%a - %t - %s.mp3",
	"%a - %l - %t - %s.mp3",
	0
};

const char **Mp3File::fnFmtList = &fnFmt[0];

/**
 * Get filename from tags.
 * Supported formats:
 * artist - album/track song.mp3
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

void Mp3File::getFilenameFromTags(const StandardTags *st, QString fmt)
{
	const int num_tag_codes = 7;
	const char tag_code[num_tag_codes] = {
	    's', 'l', 'a', 'c', 'y', 't', 'g'};
	const char *tag_str[num_tag_codes];
	const char *insert_str[num_tag_codes];
	QString year, track;
	year.sprintf("%d", st->year);
	track.sprintf("%02d", st->track);
	tag_str[0] = st->title.latin1();
	tag_str[1] = st->album.latin1();
	tag_str[2] = st->artist.latin1();
	tag_str[3] = st->comment.latin1();
	tag_str[4] = year.latin1();
	tag_str[5] = track.latin1();
	tag_str[6] = Genres::getName(st->genre);
	int pos = 0;
	for (int i = 0;; ++i) {
		pos = fmt.find('%', pos);
		if (pos == -1) break;
		if (i >= num_tag_codes) {
			// maximum of insert strings reached,
			// remove rest of string
			fmt.truncate(pos);
			break;
		}
		++pos;
		for (int k = 0;; ++k) {
			if (k >= num_tag_codes) {
				// invalid code at pos, remove it
				fmt.remove(--pos, 2);
				break;
			}
			if (fmt[pos] == tag_code[k]) {
				// code found, prepare format and string for sprintf
				fmt[pos] = 's';
				insert_str[i] = tag_str[k];
				++pos;
				break;
			}
		}
	}
	new_filename.sprintf(
		fmt, insert_str[0], insert_str[1], insert_str[2],
		insert_str[3], insert_str[4], insert_str[5], insert_str[6]);
}

/**
 * Update frame list box.
 *
 * @param lb list box
 */

void Mp3File::updateTagListV2(QListBox *lb)
{
	if (tagV2) {
		ID3_Tag::Iterator* iter = tagV2->CreateIterator();
		ID3_Frame* frame;
		lb->clear();
		while ((frame = iter->GetNext()) != NULL) {
			lb->insertItem(frame->GetTextID());
		}
		delete iter;
	}
}
