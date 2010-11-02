/**
 * \file discogsdialog.cpp
 * Discogs import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 *
 * Copyright (C) 2006-2009  Urs Fleisch
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
	&Kid3App::s_discogsCfg,
	true
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
	// <div><a href="/Amon-Amarth-The-Avenger/release/398878"><em>Amon</em> <em>Amarth</em> - <em>The</em> <em>Avenger</em></a></div>
	QString str = QString::fromUtf8(searchStr);
	QRegExp idTitleRe("<a href=\"/([^/]*/?release)/([0-9]+)\">(.+)</a>");
	QStringList lines = QCM_split("\n", str.remove('\r'));
	m_albumListBox->clear();
	for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		if (idTitleRe.QCM_indexIn(*it) != -1) {
			QString title(idTitleRe.cap(3));
			title.replace(QRegExp("<[^>]+>"), "");
			if (!title.isEmpty()) {
				new AlbumListItem(
					m_albumListBox,
					title,
					idTitleRe.cap(1),
					idTitleRe.cap(2));
			}
		}
	}
	m_albumListBox->setFocus();
}

/**
 * Remove trailing stars and numbers like (2) from a string.
 *
 * @param str string
 *
 * @return fixed up string.
 */
static QString fixUpArtist(QString str)
{
	str.replace(QRegExp(",(\\S)"), ", \\1");
	str.replace("* / ", " / ");
	str.replace("*,", ",");
	str.remove(QRegExp("\\*$"));
	str.remove(QRegExp("[*\\s]*\\(\\d+\\)\\(tracks:[^)]+\\)"));
	str.replace(QRegExp("[*\\s]*\\((?:\\d+|tracks:[^)]+)\\) / "), " / ");
	str.replace(QRegExp("[*\\s]*\\((?:\\d+|tracks:[^)]+)\\),"), ",");
	return str.remove(QRegExp("[*\\s]*\\((?:\\d+|tracks:[^)]+)\\)$"));
}


/**
 * Add involved people to a frame.
 * The format used is (should be converted according to tag specifications):
 * involvee 1 (involvement 1)\n
 * involvee 2 (involvement 2)\n
 * ...
 * involvee n (involvement n)
 *
 * @param frames      frame collection
 * @param type        type of frame
 * @param involvement involvement (e.g. instrument)
 * @param involvee    name of involvee (e.g. musician)
 */
static void addInvolvedPeople(
	FrameCollection& frames, Frame::Type type,
	const QString& involvement, const QString& involvee)
{
	QString value = frames.getValue(type);
	if (!value.isEmpty()) value += Frame::stringListSeparator();
	value += involvement;
	value += Frame::stringListSeparator();
	value += involvee;
	frames.setValue(type, value);
}

/**
 * Set tags from a string with credits lines.
 * The string must have lines like "Composed By - Iommi", separated by \\n.
 *
 * @param str    credits string
 * @param frames tags will be added to these frames
 *
 * @return true if credits found.
 */
static bool parseCredits(const QString& str, FrameCollection& frames)
{
	bool result = false;
	QStringList lines = QCM_split("\n", str);
	for (QStringList::const_iterator it = lines.begin();
			 it != lines.end();
			 ++it) {
		int nameStart = (*it).QCM_indexOf(" - ");
		if (nameStart != -1) {
			QString name(fixUpArtist((*it).mid(nameStart + 3)));
			QStringList credits = QCM_split(", ", (*it).left(nameStart));
			for (QStringList::const_iterator cit = credits.begin();
					 cit != credits.end();
					 ++cit) {
				static const struct {
					const char* credit;
					Frame::Type type;
				} creditToType[] = {
					{ "Composed By", Frame::FT_Composer },
					{ "Conductor", Frame::FT_Conductor },
					{ "Orchestra", Frame::FT_AlbumArtist },
					{ "Lyrics By", Frame::FT_Lyricist },
					{ "Written-By", Frame::FT_Author },
					{ "Written By", Frame::FT_Author },
					{ "Remix", Frame::FT_Remixer },
					{ "Music By", Frame::FT_Composer },
					{ "Songwriter", Frame::FT_Composer }
				};
				bool found = false;
				for (unsigned i = 0;
						 i < sizeof(creditToType) / sizeof(creditToType[0]);
						 ++i) {
					if (*cit == creditToType[i].credit) {
						frames.setValue(creditToType[i].type, name);
						found = true;
						break;
					}
				}
				if (found) {
					result = true;
				} else {
					static const struct {
						const char* credit;
						const char* arrangement;
					} creditToArrangement[] = {
						{ "Arranged By", "Arranger" },
						{ "Mixed By", "Mixer" },
						{ "DJ Mix", "DJMixer" },
						{ "Dj Mix", "DJMixer" },
						{ "Engineer", "Engineer" },
						{ "Mastered By", "Engineer" },
						{ "Producer", "Producer" },
						{ "Co-producer", "Producer" },
						{ "Executive Producer", "Producer" }
					};
					for (unsigned i = 0;
							 i < sizeof(creditToArrangement) / sizeof(creditToArrangement[0]);
							 ++i) {
						if ((*cit).startsWith(creditToArrangement[i].credit)) {
							addInvolvedPeople(frames, Frame::FT_Arranger,
																creditToArrangement[i].arrangement, name);
							found = true;
							break;
						}
					}
				}
				if (found) {
					result = true;
				} else {
					static const char* const instruments[] = {
						"Performer", "Vocals", "Voice", "Featuring", "Choir", "Chorus",
						"Baritone", "Tenor", "Rap", "Scratches", "Drums", "Percussion",
						"Keyboards", "Cello", "Piano", "Organ", "Synthesizer", "Keys",
						"Wurlitzer", "Rhodes", "Harmonica", "Xylophone", "Guitar", "Bass",
						"Strings", "Violin", "Viola", "Banjo", "Harp", "Mandolin",
						"Clarinet", "Horn", "Cornet", "Flute", "Oboe", "Saxophone",
						"Trumpet", "Tuba", "Trombone"
					};
					for (unsigned i = 0;
							 i < sizeof(instruments) / sizeof(instruments[0]);
							 ++i) {
						if ((*cit).contains(instruments[i])) {
							addInvolvedPeople(frames, Frame::FT_Performer, *cit, name);
							found = true;
							break;
						}
					}
				}
				if (found) {
					result = true;
				}
			}
		}
	}
	return result;
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
	QRegExp atDiscogsRe("\\s*\\([^)]+\\) at Discogs$");
	QString str = QString::fromUtf8(albumStr);
	FrameCollection framesHdr;
	/*
	 * artist and album can be found in the title:
<title>Amon Amarth - The Avenger (CD, Album, Dig) at Discogs</title>
	 */
	int end = 0;
	int start = str.QCM_indexOf("<title>");
	if (start >= 0) {
		start += 7; // skip <title>
		end = str.QCM_indexOf("</title>", start);
		if (end > start) {
			QString titleStr = str.mid(start, end - start);
			titleStr.replace(atDiscogsRe, "");
			titleStr.replace(nlSpaceRe, " "); // reduce new lines and space after them
			start = 0;
			end = titleStr.QCM_indexOf(" - ", start);
			if (end > start) {
				framesHdr.setArtist(fixUpArtist(titleStr.mid(start, end - start)));
				start = end + 3; // skip " - "
			}
			framesHdr.setAlbum(titleStr.mid(start));
		}
	}
	/*
	 * the year can be found in "Released:"
<div class="head">Released:</div><div class="content">02 Nov 1999</div>
	 */
	start = str.QCM_indexOf("Released:");
	if (start >= 0) {
		start += 9; // skip "Released:"
		end = str.QCM_indexOf("</div>", start + 1);
		if (end > start) {
			QString yearStr = str.mid(start, end - start);
			yearStr.replace(nlSpaceRe, ""); // strip new lines and space after them
			yearStr.replace(htmlTagRe, ""); // strip HTML tags
			QRegExp yearRe("(\\d{4})"); // this should skip day and month numbers
			if (yearRe.QCM_indexIn(yearStr) >= 0) {
				framesHdr.setYear(yearRe.cap(1).toInt());
			}
		}
	}
	/*
	 * the genre can be found in "Genre:" or "Style:" (lines with only whitespace in between):
<div class="head">Genre:</div><div class="content">
      Rock
</div>
<div class="head">Style:</div><div class="content">
    Viking Metal, 
    Death Metal
</div>
	 */
	// All genres found are checked for an ID3v1 number, starting with those
	// in the Style field.
	QStringList genreList;
	static const char* const fields[] = { "Style:", "Genre:" };
	for (unsigned i = 0; i < sizeof(fields) / sizeof(fields[0]); ++i) {
		start = str.QCM_indexOf(fields[i]);
		if (start >= 0) {
			start += qstrlen(fields[i]); // skip field
			end = str.QCM_indexOf("</div>", start + 1);
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
		framesHdr.setGenre(Genres::getName(genreNum));
	} else if (!genreList.empty()) {
		framesHdr.setGenre(genreList.front());
	}

	const bool additionalTags = getAdditionalTags();
	if (additionalTags) {
		/*
		 * publisher can be found in "Label:"
		 */
		start = str.QCM_indexOf("Label:");
		if (start >= 0) {
			start += 6; // skip "Label:"
			end = str.QCM_indexOf("</div>", start + 1);
			if (end > start) {
				QString labelStr = str.mid(start, end - start);
				labelStr.replace(nlSpaceRe, ""); // strip new lines and space after them
				labelStr.replace(htmlTagRe, ""); // strip HTML tags
				labelStr = fixUpArtist(labelStr);
				if (labelStr != "Not On Label") {
					framesHdr.setValue(Frame::FT_Publisher, labelStr);
				}
			}
		}

		/*
		 * media can be found in "Format:"
		 */
		start = str.QCM_indexOf("Format:");
		if (start >= 0) {
			start += 7; // skip "Format:"
			end = str.QCM_indexOf("</div>", start + 1);
			if (end > start) {
				QString mediaStr = str.mid(start, end - start);
				mediaStr.replace(nlSpaceRe, ""); // strip new lines and space after them
				mediaStr.replace(htmlTagRe, ""); // strip HTML tags
				framesHdr.setValue(Frame::FT_Media, mediaStr);
			}
		}

		/*
		 * credits can be found in "Credits"
		 */
		start = str.QCM_indexOf("<h1>Credits</h1>");
		if (start >= 0) {
			start += 16; // skip "Credits"
			end = str.QCM_indexOf("</div>", start + 1);
			if (end > start) {
				QString creditsStr = str.mid(start, end - start);
				creditsStr.replace(nlSpaceRe, ""); // strip new lines and space after them
				creditsStr.replace("<br />", "\n");
				creditsStr.replace(htmlTagRe, ""); // strip HTML tags
				parseCredits(creditsStr, framesHdr);
			}
		}
	}

	m_trackDataVector.setCoverArtUrl(QString::null);
	if (getCoverArt()) {
		/*
		 * cover art can be found in image source
		 */
		start = str.QCM_indexOf("<img src=\"http://www.discogs.com/image/");
		if (start >= 0) {
			start += 10; // skip <img src="
			end = str.QCM_indexOf("\"", start);
			if (end > start) {
				m_trackDataVector.setCoverArtUrl(str.mid(start, end - start));
			}
		}
	}

	/*
	 * album tracks have the format (lines with only whitespace in between):
<h1>Tracklist</h1>
<div class="section_content">
<table>
  <tr class="first">
    <td class="track_pos">1</td>
      <td>&nbsp;</td>
    <td class="track_title">Bleed For Ancient Gods</td>
    <td class="track_duration">4:31</td>
    <td class="track_itunes"></td>
  </tr>
  <tr>
    <td class="track_pos">2</td>
(..)
</table>
	 *
	 * Variations: strange track numbers, no durations, links instead of tracks,
	 * only "track" instead of "track_title", align attribute in "track_duration"
	 */
	start = str.QCM_indexOf(">Tracklist</");
	if (start >= 0) {
		end = str.QCM_indexOf("</table>", start);
		if (end > start) {
			str = str.mid(start, end - start);
			// strip whitespace
			str.replace(nlSpaceRe, "");

			FrameCollection frames(framesHdr);
			QRegExp posRe("<td class=\"track_pos\">(\\d+)</td>");
			QRegExp artistsRe("<td class=\"track_artists\"><a href=\"/artist/[^>]+>([^<]+)</a>");
			QRegExp titleRe("<td class=\"track(?:_title)?\">([^<]+)</td>");
			QRegExp durationRe("<td class=\"track_duration\"[^>]*>(\\d+):(\\d+)</td>");
			QRegExp indexRe("<td class=\"track_index\">([^<]+)$");
			QRegExp rowEndRe("</td>[\\s\\r\\n]*</tr>");
			ImportTrackDataVector::iterator it = m_trackDataVector.begin();
			bool atTrackDataListEnd = (it == m_trackDataVector.end());
			int trackNr = 1;
			start = 0;
			while ((end = rowEndRe.QCM_indexIn(str, start)) > start) {
				QString trackDataStr = str.mid(start, end - start);
				QString title;
				int duration = 0;
				int pos = trackNr;
				if (titleRe.QCM_indexIn(trackDataStr) >= 0) {
					title = titleRe.cap(1);
				}
				if (durationRe.QCM_indexIn(trackDataStr) >= 0) {
					duration = durationRe.cap(1).toInt() * 60 +
						durationRe.cap(2).toInt();
				}
				if (posRe.QCM_indexIn(trackDataStr) >= 0) {
					pos = posRe.cap(1).toInt();
				}
				if (additionalTags) {
					if (artistsRe.QCM_indexIn(trackDataStr) >= 0) {
						// use the artist in the header as the album artist
						// and the artist in the track as the artist
						frames.setArtist(
							fixUpArtist(artistsRe.cap(1)));
						frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
					}
				}
				start = end + 10; // skip </td></tr>
				if (indexRe.QCM_indexIn(trackDataStr) >= 0) {
					if (additionalTags) {
						QString subtitle(indexRe.cap(1));
						framesHdr.setValue(Frame::FT_Part, subtitle);
						frames.setValue(Frame::FT_Part, subtitle);
					}
					continue;
				}
				if (additionalTags) {
					int nextEnd = rowEndRe.QCM_indexIn(str, start);
					if (nextEnd > start) {
						QString nextTitle(str.mid(start, nextEnd - start));
						if (nextTitle.QCM_indexOf("<tr class=\"track_extra_artists\">") != -1) {
							// additional track info like "Music By, Lyrics By - "
							nextTitle.replace("<br>", "\n");
							nextTitle.replace(htmlTagRe, ""); // strip HTML tags
							nextTitle.remove("&nbsp;");
							if (parseCredits(nextTitle, frames)) {
								start = nextEnd + 10; // skip </td></tr>
							}
						}
					}
				}

				if (!title.isEmpty() || duration != 0) {
					frames.setTrack(pos);
					frames.setTitle(title);
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
					++trackNr;
				}
				frames = framesHdr;
			}

			// handle redundant tracks
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
	}
}
