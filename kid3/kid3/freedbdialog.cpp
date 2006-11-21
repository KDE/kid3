/**
 * \file freedbdialog.cpp
 * freedb.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2004
 */

#include <qregexp.h>
#include "kid3.h"
#include "freedbclient.h"
#include "freedbdialog.h"
#if QT_VERSION >= 0x040000
#include <Q3ValueList>
#endif

static const char* serverList[] = {
	"freedb2.org:80",
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
	"freedb.freedb.org:80",
	"/~cddb/cddb.cgi",
	"import-freedb",
	&Kid3App::s_freedbCfg
};


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataVector track data to be filled with imported values
 */
FreedbDialog::FreedbDialog(QWidget *parent,
													 ImportTrackDataVector& trackDataVector)
	: ImportSourceDialog(parent, "freedb.org", trackDataVector,
											 new FreedbClient, props)
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
210 exact matches found
categ discid dtitle
(more matches...)
.
or
211 close matches found
rock 920b810c Catharsis / Imago
.
theoretically, but never seen
200	categ discid dtitle
*/
	QString str = QString::fromUtf8(searchStr);
	QRegExp catIdTitleRe("([a-z]+)\\s+([0-9a-f]+)\\s+([^/]+ / .+)");
	QStringList lines = QStringList::split(QRegExp("[\\r\\n]+"), str);
	bool inEntries = false;
	m_albumListBox->clear();
	for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		if (*it == ".") {
			break;
		}
		if (inEntries) {
			if (catIdTitleRe.exactMatch(*it)) {
				new AlbumListItem(
					m_albumListBox,
					catIdTitleRe.cap(3),
					catIdTitleRe.cap(1),
					catIdTitleRe.cap(2));
			}
		} else {
			if ((*it).startsWith("21") && (*it).find(" match") != -1) {
				inEntries = true;
			} else if ((*it).startsWith("200 ")) {
				if (catIdTitleRe.exactMatch((*it).mid(4))) {
					new AlbumListItem(
						m_albumListBox,
						catIdTitleRe.cap(3),
						catIdTitleRe.cap(1),
						catIdTitleRe.cap(2));
				}
			}
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
	Q3ValueList<int>& trackDuration)
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
		int trackOffsetPos = text.find("Track frame offsets", 0);
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
 * @param st   standard tag to put result
 */
static void parseFreedbAlbumData(const QString &text,
																 StandardTags& st)
{
	QRegExp fdre("DTITLE=\\s*(\\S[^\\r\\n]*\\S)\\s*/\\s*(\\S[^\\r\\n]*\\S)[\\r\\n]");
	if (fdre.search(text) != -1) {
		st.artist = fdre.cap(1);
		st.album = fdre.cap(2);
	}
	fdre.setPattern("EXTD=[^\\r\\n]*YEAR:\\s*(\\d+)\\D");
	if (fdre.search(text) != -1) {
		st.year = fdre.cap(1).toInt();
	}
	fdre.setPattern("EXTD=[^\\r\\n]*ID3G:\\s*(\\d+)\\D");
	if (fdre.search(text) != -1) {
		st.genre = fdre.cap(1).toInt();
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
	StandardTags st_hdr;
	st_hdr.setInactive();
	Q3ValueList<int> trackDuration;
	parseFreedbTrackDurations(text, trackDuration);
	parseFreedbAlbumData(text, st_hdr);

	StandardTags st(st_hdr);
	ImportTrackDataVector::iterator it = m_trackDataVector.begin();
	Q3ValueList<int>::const_iterator tdit = trackDuration.begin();
	bool atTrackDataListEnd = (it == m_trackDataVector.end());
	int pos = 0;
	int idx, oldpos = pos;
	int tracknr = 0;
	for (;;) {
		QRegExp fdre(QString("TTITLE%1=([^\\r\\n]+)[\\r\\n]").arg(tracknr));
		QString title;
		while ((idx = fdre.search(text, pos)) != -1) {
			title += fdre.cap(1);
			pos = idx + fdre.matchedLength();
		}
		if (pos > oldpos) {
			st.track = tracknr + 1;
			st.title = title;
		} else {
			break;
		}
		int duration = (tdit != trackDuration.end()) ?
			*tdit++ : 0;
		if (atTrackDataListEnd) {
			ImportTrackData trackData;
			trackData.setStandardTags(st);
			trackData.setImportDuration(duration);
			m_trackDataVector.push_back(trackData);
		} else {
			(*it).setStandardTags(st);
			(*it).setImportDuration(duration);
			++it;
			atTrackDataListEnd = (it == m_trackDataVector.end());
		}
		st = st_hdr;
		oldpos = pos;
		++tracknr;
	}
	st.setInactive();
	while (!atTrackDataListEnd) {
		if ((*it).getFileDuration() == 0) {
			it = m_trackDataVector.erase(it);
		} else {
			(*it).setStandardTags(st);
			(*it).setImportDuration(0);
			++it;
		}
		atTrackDataListEnd = (it == m_trackDataVector.end());
	}
}
