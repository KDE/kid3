/**
 * \file discogsdialog.cpp
 * Discogs import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
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

#include <qregexp.h>
#include "kid3.h"
#include "discogsclient.h"
#include "discogsdialog.h"
#include "genres.h"

static const ImportSourceDialog::Properties props = {
	0,
	0,
	0,
	"import-discogs",
	&Kid3App::s_discogsCfg
};


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataVector track data to be filled with imported values
 */
DiscogsDialog::DiscogsDialog(QWidget* parent,
														 ImportTrackDataVector& trackDataVector)
	: ImportSourceDialog(parent, "Discogs", trackDataVector,
											 new DiscogsClient, props)
{
}

/**
 * Destructor.
 */
DiscogsDialog::~DiscogsDialog()
{
}

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void DiscogsDialog::parseFindResults(const QByteArray& searchStr)
{
	// releases have the format:
	// <li><a href="/release/761529"><span style="font-size: 11pt;"><em>Amon</em> <em>Amarth</em> - The <em>Avenger</em></span></a><br>
	QString str = QString::fromUtf8(searchStr);
	QRegExp idTitleRe("<a href=\"/release/([0-9]+)\">(.+)</a>");
	QStringList lines = QCM_split(QRegExp("[\\r\\n]+"), str);
	m_albumListBox->clear();
	for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		if (idTitleRe.QCM_indexIn(*it) != -1) {
			QString title(idTitleRe.cap(2));
			title.replace(QRegExp("<[^>]+>"), "");
			new AlbumListItem(
				m_albumListBox,
				title,
				"release",
				idTitleRe.cap(1));
		}
	}
	m_albumListBox->setFocus();
}

/**
 * Parse result of album request and populate m_trackDataVector with results.
 *
 * @param albumStr album data received
 */
void DiscogsDialog::parseAlbumResults(const QByteArray& albumStr)
{
	QRegExp nlSpaceRe("[\r\n]+\\s*");
	QRegExp htmlTagRe("<[^>]+>");
	QString str = QString::fromUtf8(albumStr);
	StandardTags stHdr;
	stHdr.setInactive();
	/*
	 * artist and album can be found in the title:
<title>Amon Amarth - The Avenger</title>
	 */
	int end = 0;
	int start = str.QCM_indexOf("<title>");
	if (start >= 0) {
		start += 7; // skip <title>
		end = str.QCM_indexOf("</title>", start);
		if (end > start) {
			QString titleStr = str.mid(start, end - start);
			titleStr.replace(nlSpaceRe, " "); // reduce new lines and space after them
			start = 0;
			end = titleStr.QCM_indexOf(" - ", start);
			if (end > start) {
				stHdr.artist = titleStr.mid(start, end - start);
				start = end + 3; // skip " - "
			}
			stHdr.album = titleStr.mid(start);
		}
	}
	/*
	 * the year can be found in "Released:"
<tr><td align=right>Released:</td><td>1999</td></tr>
	 */
	start = str.QCM_indexOf("Released:");
	if (start >= 0) {
		start += 9; // skip "Released:"
		end = str.QCM_indexOf("</tr>", start);
		if (end > start) {
			QString yearStr = str.mid(start, end - start);
			yearStr.replace(nlSpaceRe, ""); // strip new lines and space after them
			yearStr.replace(htmlTagRe, ""); // strip HTML tags
			QRegExp yearRe("(\\d{4})"); // this should skip day and month numbers
			if (yearRe.QCM_indexIn(yearStr) >= 0) {
				stHdr.year = yearRe.cap(1).toInt();
			}
		}
	}
	/*
	 * the genre can be found in "Genre:" or "Style:" (lines with only whitespace in between):
<tr><td align=right>Genre:</td><td>
      Rock
</td></tr>
<tr><td align=right>Style:</td><td>
    Death Metal, 
    Heavy Metal
</td></tr>
	 */
	// All genres found are checked for an ID3v1 number, starting with those
	// in the Style field.
	QStringList genreList;
	static const char* const fields[] = { "Style:", "Genre:" };
	for (unsigned i = 0; i < sizeof(fields) / sizeof(fields[0]); ++i) {
		start = str.QCM_indexOf(fields[i]);
		if (start >= 0) {
			start += qstrlen(fields[i]); // skip field
			end = str.QCM_indexOf("</tr>", start);
			if (end > start) {
				QString genreStr = str.mid(start, end - start);
				genreStr.replace(nlSpaceRe, ""); // strip new lines and space after them
				genreStr.replace(htmlTagRe, ""); // strip HTML tags
				if (genreStr.QCM_indexOf(',') >= 0) {
					genreList += QCM_split(QRegExp(",\\s*"), genreStr);
				} else {
					if (!genreStr.isEmpty()) {
						genreList += genreStr;
					}
				}
			}
		}
	}
	int genreNum = 255;
	for (QStringList::const_iterator it = genreList.begin();
			 it != genreList.end();
			 ++it) {
		genreNum = Genres::getNumber(*it);
		if (genreNum != 255) {
			break;
		}
	}
	if (genreNum != 255) {
		stHdr.genre = Genres::getName(genreNum);
	} else if (!genreList.empty()) {
		stHdr.genre = genreList.front();
	}

	/*
	 * album tracks have the format (lines with only whitespace in between):
<b>Tracklisting:</b><br>
<table border=0 cellpadding=2 cellspacing=2>
  <tr>
    <td nowrap align="left">1</td>
      <td>&nbsp;</td>
    <td>Bleed For Ancient Gods (4:31)</td>
  </tr>
(..)
</table>
	 *
	 * Variations: strange track numbers, no durations, links instead of tracks
	 */
	start = str.QCM_indexOf("Tracklisting:");
	if (start >= 0) {
		end = str.QCM_indexOf("</table>", start);
		if (end > start) {
			str = str.mid(start, end - start);
			// strip whitespace
			str.replace(nlSpaceRe, "");

			StandardTags st(stHdr);
			QRegExp titleTimeRe("(.+)\\s+\\((\\d+):(\\d+)\\)");
			ImportTrackDataVector::iterator it = m_trackDataVector.begin();
			bool atTrackDataListEnd = (it == m_trackDataVector.end());
			int trackNr = 1;
			start = 0;
			while ((end = str.QCM_indexOf("</td></tr>", start)) > start) {
				int titleStart = str.QCM_lastIndexOf("<td>", end);
				if (titleStart > start) {
					titleStart += 4; // skip <td>
				} else {
					break;
				}
				QString title(str.mid(titleStart, end - titleStart));
				start = end + 10; // skip </td></tr>
				if (title.QCM_indexOf("</") != -1 || title.QCM_indexOf("<a href") != -1) {
					// strange entry instead of track => skip
					continue;
				}
				int duration = 0;
				if (titleTimeRe.exactMatch(title)) {
					duration = titleTimeRe.cap(2).toInt() * 60 +
						titleTimeRe.cap(3).toInt();
					title = titleTimeRe.cap(1);
				}
				st.track = trackNr;
				st.title = title;
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
				++trackNr;
				st = stHdr;
			}

			// handle redundant tracks
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
	}
}
