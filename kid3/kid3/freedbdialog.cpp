/**
 * \file freedbdialog.cpp
 * freedb.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2004
 *
 * Copyright (C) 2004-2007  Urs Fleisch
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

#include <qregexp.h>
#include "kid3.h"
#include "freedbclient.h"
#include "freedbdialog.h"
#include "genres.h"
#include "importparser.h"
#if QT_VERSION < 0x040000
#include <cstring>
#endif

static const char* serverList[] = {
	"www.gnudb.org:80",
	"gnudb.gnudb.org:80",
	"freedb.org:80",
	"freedb.freedb.org:80",
	"at.freedb.org:80",
	"au.freedb.org:80",
	"ca.freedb.org:80",
	"es.freedb.org:80",
	"fi.freedb.org:80",
	"lu.freedb.org:80",
	"ru.freedb.org:80",
	"uk.freedb.org:80",
	"us.freedb.org:80",
	0                  // end of StrList
};

static const ImportSourceDialog::Properties props = {
	serverList,
	"www.gnudb.org:80",
	"/~cddb/cddb.cgi",
	"import-freedb",
	&Kid3App::s_freedbCfg,
	false
};


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataVector track data to be filled with imported values
 */
FreedbDialog::FreedbDialog(QWidget* parent,
													 ImportTrackDataVector& trackDataVector)
	: ImportSourceDialog(parent, "gnudb.org", trackDataVector,
											 new FreedbClient, props)
{
}

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 * @param trackDataVector track data to be filled with imported values
 * @param client  client to use, this object takes ownership of it
 * @param props   constant dialog properties, must exist while dialog exists
 */
FreedbDialog::FreedbDialog(QWidget* parent, QString caption,
													 ImportTrackDataVector& trackDataVector,
													 ImportSourceClient* client,
													 const Properties& props)
	: ImportSourceDialog(parent, caption, trackDataVector, client, props)
{
}

/**
 * Destructor.
 */
FreedbDialog::~FreedbDialog()
{
}

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void FreedbDialog::parseFindResults(const QByteArray& searchStr)
{
/*
<h2>Search Results, 1 albums found:</h2>
<br><br>
<a href="http://www.gnudb.org/cd/ro920b810c"><b>Catharsis / Imago</b></a><br>
Tracks: 12, total time: 49:07, year: 2002, genre: Metal<br>
<a href="http://www.gnudb.org/gnudb/rock/920b810c" target=_blank>Discid: rock / 920b810c</a><br>
*/
	bool isUtf8 = false;
#if QT_VERSION >= 0x040000
	int charSetPos = searchStr.indexOf("charset=");
	if (charSetPos != -1) {
		charSetPos += 8;
		QByteArray charset(searchStr.mid(charSetPos, 5));
		isUtf8 = charset == "utf-8" || charset == "UTF-8";
	}
#else
	const char* searchStrData = searchStr.data();
	const char* charSetPtr = std::strstr(searchStrData, "charset=");
	if (charSetPtr) {
		charSetPtr += 8;
		isUtf8 = std::strncmp(charSetPtr, "utf-8", 5) == 0 ||
		         std::strncmp(charSetPtr, "UTF-8", 5) == 0;
	}
#endif
	QString str = isUtf8 ? QString::fromUtf8(searchStr) :
	                       QString::fromLatin1(searchStr);
	QRegExp titleRe("<a href=\"[^\"]+/cd/[^\"]+\"><b>([^<]+)</b></a>");
	QRegExp catIdRe("Discid: ([a-z]+)[\\s/]+([0-9a-f]+)");
	QStringList lines = QCM_split(QRegExp("[\\r\\n]+"), str);
	QString title;
	bool inEntries = false;
	m_albumListBox->clear();
	for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		if (inEntries) {
			if (titleRe.QCM_indexIn(*it) != -1) {
				title = titleRe.cap(1);
			}
			if (catIdRe.QCM_indexIn(*it) != -1) {
				new AlbumListItem(
					m_albumListBox,
					title,
					catIdRe.cap(1),
					catIdRe.cap(2));
			}
		} else if ((*it).QCM_indexOf(" albums found:") != -1) {
			inEntries = true;
		}
	}
	m_albumListBox->setFocus();
}

/**
 * Parse the track durations from freedb.org.
 *
 * @param text          text buffer containing data from freedb.org
 * @param trackDuration list for results
 */
static void parseFreedbTrackDurations(
	const QString& text,
	TrackDurationList& trackDuration)
{
/* Example freedb format:
   # Track frame offsets:
   # 150
   # 2390
   # 23387
   # 44650
   # 61322
   # 94605
   # 121710
   # 144637
   # 176820
   # 187832
   # 218930
   #
   # Disc length: 3114 seconds
*/
	trackDuration.clear();
	QRegExp discLenRe("Disc length:\\s*\\d+");
	int discLenPos, len;
	discLenPos = discLenRe.QCM_indexIn(text, 0);
	if (discLenPos != -1) {
		len = discLenRe.matchedLength();
		discLenPos += 12;
		int discLen = text.mid(discLenPos, len - 12).toInt();
		int trackOffsetPos = text.QCM_indexOf("Track frame offsets", 0);
		if (trackOffsetPos != -1) {
			QRegExp re("#\\s*\\d+");
			int lastOffset = -1;
			while ((trackOffsetPos = re.QCM_indexIn(text, trackOffsetPos)) != -1 &&
						 trackOffsetPos < discLenPos) {
				len = re.matchedLength();
				trackOffsetPos += 1;
				int trackOffset = text.mid(trackOffsetPos, len - 1).toInt();
				if (lastOffset != -1) {
					int duration = (trackOffset - lastOffset) / 75;
					trackDuration.append(duration);
				}
				lastOffset = trackOffset;
			}
			if (lastOffset != -1) {
				int duration = (discLen * 75 - lastOffset) / 75;
				trackDuration.append(duration);
			}
		}
	}
}

/**
 * Parse the album specific data (artist, album, year, genre) from freedb.org.
 *
 * @param text text buffer containing data from freedb.org
 * @param frames tags to put result
 */
static void parseFreedbAlbumData(const QString& text,
																 FrameCollection& frames)
{
	QRegExp fdre("DTITLE=\\s*(\\S[^\\r\\n]*\\S)\\s*/\\s*(\\S[^\\r\\n]*\\S)[\\r\\n]");
	if (fdre.QCM_indexIn(text) != -1) {
		frames.setArtist(fdre.cap(1));
		frames.setAlbum(fdre.cap(2));
	}
	fdre.setPattern("EXTD=[^\\r\\n]*YEAR:\\s*(\\d+)\\D");
	if (fdre.QCM_indexIn(text) != -1) {
		frames.setYear(fdre.cap(1).toInt());
	}
	fdre.setPattern("EXTD=[^\\r\\n]*ID3G:\\s*(\\d+)\\D");
	if (fdre.QCM_indexIn(text) != -1) {
		frames.setGenre(Genres::getName(fdre.cap(1).toInt()));
	}
}

/**
 * Parse result of album request and populate m_trackDataVector with results.
 *
 * @param albumStr album data received
 */
void FreedbDialog::parseAlbumResults(const QByteArray& albumStr)
{
	QString text = QString::fromUtf8(albumStr);
	FrameCollection framesHdr;
	TrackDurationList trackDuration;
	parseFreedbTrackDurations(text, trackDuration);
	parseFreedbAlbumData(text, framesHdr);

	FrameCollection frames(framesHdr);
	ImportTrackDataVector::iterator it = m_trackDataVector.begin();
	TrackDurationList::const_iterator tdit = trackDuration.begin();
	bool atTrackDataListEnd = (it == m_trackDataVector.end());
	int pos = 0;
	int idx, oldpos = pos;
	int tracknr = 0;
	for (;;) {
		QRegExp fdre(QString("TTITLE%1=([^\\r\\n]+)[\\r\\n]").arg(tracknr));
		QString title;
		while ((idx = fdre.QCM_indexIn(text, pos)) != -1) {
			title += fdre.cap(1);
			pos = idx + fdre.matchedLength();
		}
		if (pos > oldpos) {
			frames.setTrack(tracknr + 1);
			frames.setTitle(title);
		} else {
			break;
		}
		int duration = (tdit != trackDuration.end()) ?
			*tdit++ : 0;
		if (atTrackDataListEnd) {
			ImportTrackData trackData;
			trackData.setFrameCollection(frames);
			trackData.setImportDuration(duration);
			m_trackDataVector.push_back(trackData);
		} else {
			(*it).setFrameCollection(frames);
			(*it).setImportDuration(duration);
			++it;
			atTrackDataListEnd = (it == m_trackDataVector.end());
		}
		frames = framesHdr;
		oldpos = pos;
		++tracknr;
	}
	frames.clear();
	while (!atTrackDataListEnd) {
		if ((*it).getFileDuration() == 0) {
			it = m_trackDataVector.erase(it);
		} else {
			(*it).setFrameCollection(frames);
			(*it).setImportDuration(0);
			++it;
		}
		atTrackDataListEnd = (it == m_trackDataVector.end());
	}
}
