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
	// <li><a href="/release/761529"><span style="font-size: 11pt;"><em>Amon</em> <em>Amarth</em> - The <em>Avenger</em></span></a><br>
	QString str = QString::fromUtf8(searchStr);
	QRegExp idTitleRe("<a href=\"/release/([0-9]+)\">(.+)</a>");
	QStringList lines = QCM_split("<p/>", str.remove('\n').remove('\r'));
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
	QString str = QString::fromUtf8(albumStr);
	FrameCollection framesHdr;
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
				framesHdr.setArtist(fixUpArtist(titleStr.mid(start, end - start)));
				start = end + 3; // skip " - "
			}
			framesHdr.setAlbum(titleStr.mid(start));
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
				framesHdr.setYear(yearRe.cap(1).toInt());
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
			end = str.QCM_indexOf("</tr>", start);
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
			end = str.QCM_indexOf("</tr>", start);
			if (end > start) {
				QString mediaStr = str.mid(start, end - start);
				mediaStr.replace(nlSpaceRe, ""); // strip new lines and space after them
				mediaStr.replace(htmlTagRe, ""); // strip HTML tags
				framesHdr.setValue(Frame::FT_Media, mediaStr);
			}
		}

		/*
		 * credits can be found in "Credits:"
		 */
		start = str.QCM_indexOf("Credits:");
		if (start >= 0) {
			start += 8; // skip "Credits:"
			end = str.QCM_indexOf("</tr>", start);
			if (end > start) {
				QString creditsStr = str.mid(start, end - start);
				creditsStr.replace(nlSpaceRe, ""); // strip new lines and space after them
				creditsStr.replace("<br>", "\n");
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

			FrameCollection frames(framesHdr);
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
				if (additionalTags) {
					int artistStart = str.QCM_indexOf("<a href=\"/artist/", start);
					if (artistStart > start && artistStart < titleStart) {
						artistStart = str.QCM_indexOf('>', artistStart);
						if (artistStart > start && artistStart < titleStart) {
							++artistStart; // skip '>'
							int artistEnd = str.QCM_indexOf("</a>", artistStart);
							if (artistEnd > artistStart && artistEnd < titleStart) {
								// use the artist in the header as the album artist
								// and the artist in the track as the artist
								frames.setArtist(
									fixUpArtist(str.mid(artistStart, artistEnd - artistStart)));
								frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
							}
						}
					}
				}
				start = end + 10; // skip </td></tr>
				if (title.QCM_indexOf("</") != -1 || title.QCM_indexOf("<a href") != -1) {
					// strange entry instead of track => skip
					if (additionalTags) {
						if (title.startsWith("<b>") && title.endsWith("</b>")) {
							// a bold title is a set subtitle
							QString subtitle(title.mid(3, title.length() - 7));
							subtitle.remove(htmlTagRe); // strip HTML tags
							// remove duration
							subtitle.remove(QRegExp("\\s*\\(\\d+:\\d+\\)$"));
							framesHdr.setValue(Frame::FT_Part, subtitle);
							frames.setValue(Frame::FT_Part, subtitle);
						}
					}
					continue;
				}

				if (additionalTags) {
					int nextEnd, nextTitleStart = -1;
					if ((nextEnd = str.QCM_indexOf("</td></tr>", start)) > start &&
							(nextTitleStart = str.QCM_lastIndexOf("<td>", nextEnd)) > start) {
						QString nextTitle(str.mid(nextTitleStart, nextEnd - nextTitleStart));
						if (nextTitle.QCM_indexOf("</") != -1 ||
								nextTitle.QCM_indexOf("<a href") != -1) {
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

				int duration = 0;
				if (titleTimeRe.exactMatch(title)) {
					duration = titleTimeRe.cap(2).toInt() * 60 +
						titleTimeRe.cap(3).toInt();
					title = titleTimeRe.cap(1);
				}
				frames.setTrack(trackNr);
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
