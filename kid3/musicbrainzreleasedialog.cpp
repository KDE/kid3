/**
 * \file musicbrainzreleasedialog.cpp
 * MusicBrainz release database import dialog.
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
#include <qdom.h>
#include "kid3.h"
#include "musicbrainzreleaseclient.h"
#include "musicbrainzreleasedialog.h"

static const char* serverList[] = {
	"musicbrainz.org:80",
	"de.musicbrainz.org:80",
	"nl.musicbrainz.org:80",
	0                  // end of StrList
};

static const ImportSourceDialog::Properties props = {
	serverList,
	"musicbrainz.org:80",
	0,
	"import-musicbrainzrelease",
	&Kid3App::s_musicBrainzCfg,
	true
};


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataVector track data to be filled with imported values
 */
MusicBrainzReleaseDialog::MusicBrainzReleaseDialog(
	QWidget* parent,
	ImportTrackDataVector& trackDataVector)
	: ImportSourceDialog(parent, "MusicBrainz", trackDataVector,
											 new MusicBrainzReleaseClient, props)
{
}

/**
 * Destructor.
 */
MusicBrainzReleaseDialog::~MusicBrainzReleaseDialog()
{
}

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void MusicBrainzReleaseDialog::parseFindResults(const QByteArray& searchStr)
{
	/*
<metadata>
	<release-list>
		<release id="978c7ed1-a854-4ef2-bd4e-e7c1317be854" type="Album Official" ext:score="100">
			<title>Odin</title>
			<asin>B00008OUEN</asin>
			<artist id="d1075cad-33e3-496b-91b0-d4670aabf4f8">
				<name>Wizard</name>
			</artist>
			<track-list count="11"/>
		</release>
	*/
#if QT_VERSION >= 0x040000
	int start = searchStr.indexOf("<?xml");
	int end = searchStr.indexOf("</metadata>");
	QByteArray xmlStr = searchStr;
#else
	QCString xmlStr(searchStr.data(), searchStr.size());
	int start = xmlStr.find("<?xml");
	int end = xmlStr.find("</metadata>");
#endif
	if (start >= 0 && end > start) {
		xmlStr = xmlStr.mid(start, end + 11 - start);
	}
	QDomDocument doc;
	if (doc.setContent(xmlStr, false)) {
		m_albumListBox->clear();
		QDomElement releaseList =
			doc.namedItem("metadata").toElement().namedItem("release-list").toElement();
		for (QDomNode releaseNode = releaseList.namedItem("release");
				 !releaseNode.isNull();
				 releaseNode = releaseNode.nextSibling()) {
			QDomElement release = releaseNode.toElement();
			QString id = release.attribute("id");
			QString title = release.namedItem("title").toElement().text();
			QDomElement artist = release.namedItem("artist").toElement();
			QString name = artist.namedItem("name").toElement().text();
			new AlbumListItem(
				m_albumListBox,
				name + " - " + title,
				"release",
				id);
		}
		m_albumListBox->setFocus();
	}
}

/**
 * Fix up attribute strings by separating them by commas and inserting spaces
 * between camel case words.
 *
 * @param str camel case string, e.g. "ElectricGuitar AcousticGuitar"
 *
 * @return fixed up string, e.g. "Electric Guitar, Acoustic Guitar".
 */
static QString fixUpCamelCase(QString str)
{
	str.replace(QRegExp("\\s+"), ", ");
	return str.replace(QRegExp("([a-z])([A-Z])"), "\\1 \\2");
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
 * Set tags from an XML node with a relation list.
 *
 * @param relationList relation-list with target-type Artist
 * @param frames       tags will be added to these frames
 *
 * @return true if credits found.
 */
static bool parseCredits(const QDomElement& relationList, FrameCollection& frames)
{
	bool result = false;
	QDomNode relation(relationList.firstChild());
	while (!relation.isNull()) {
		QString artist(relation.toElement().namedItem("artist").toElement().
									 namedItem("name").toElement().text());
		if (!artist.isEmpty()) {
			QString type(relation.toElement().attribute("type"));
			if (type == "Instrument") {
				QString attributes(relation.toElement().attribute("attributes"));
				if (!attributes.isEmpty()) {
					addInvolvedPeople(frames, Frame::FT_Performer,
														fixUpCamelCase(attributes), artist);
				}
			} else if (type == "Vocal") {
				addInvolvedPeople(frames, Frame::FT_Performer, type, artist);
			} else {
				static const struct {
					const char* credit;
					Frame::Type type;
				} creditToType[] = {
					{ "Composer", Frame::FT_Composer },
					{ "Conductor", Frame::FT_Conductor },
					{ "PerformingOrchestra", Frame::FT_AlbumArtist },
					{ "Lyricist", Frame::FT_Lyricist },
					{ "Publisher", Frame::FT_Publisher },
					{ "Remixer", Frame::FT_Remixer }
				};
				bool found = false;
				for (unsigned i = 0;
						 i < sizeof(creditToType) / sizeof(creditToType[0]);
						 ++i) {
					if (type == creditToType[i].credit) {
						frames.setValue(creditToType[i].type, artist);
						found = true;
						break;
					}
				}
				if (!found && type != "Tribute") {
					addInvolvedPeople(frames, Frame::FT_Arranger,
														fixUpCamelCase(type), artist);
				}
			}
		}
		result = true;
		relation = relation.nextSibling();
	}
	return result;
}

/**
 * Parse result of album request and populate m_trackDataVector with results.
 *
 * @param albumStr album data received
 */
void MusicBrainzReleaseDialog::parseAlbumResults(const QByteArray& albumStr)
{
	/*
<metadata>
	<release id="978c7ed1-a854-4ef2-bd4e-e7c1317be854" type="Album Official">
		<title>Odin</title>
		<asin>B00008OUEN</asin>
		<artist id="d1075cad-33e3-496b-91b0-d4670aabf4f8">
			<name>Wizard</name>
			<sort-name>Wizard</sort-name>
		</artist>
		<track-list>
			<track id="dac7c002-432f-4dcb-ad57-5ebde8e258b0">
				<title>The Prophecy</title>
				<duration>319173</duration>
			</track>
	*/
#if QT_VERSION >= 0x040000
	int start = albumStr.indexOf("<?xml");
	int end = albumStr.indexOf("</metadata>");
	QByteArray xmlStr = start >= 0 && end > start ?
		albumStr.mid(start, end + 11 - start) : albumStr;
#else
	QCString xmlStr(albumStr.data(), albumStr.size());
	int start = xmlStr.find("<?xml");
	int end = xmlStr.find("</metadata>");
	if (start >= 0 && end > start) {
		xmlStr = xmlStr.mid(start, end + 11 - start);
	}
#endif
	QDomDocument doc;
	if (doc.setContent(xmlStr, false)) {
		QDomElement release =
			doc.namedItem("metadata").toElement().namedItem("release").toElement();
		FrameCollection framesHdr;
		framesHdr.setAlbum(release.namedItem("title").toElement().text());
		framesHdr.setArtist(release.namedItem("artist").toElement().namedItem("name").toElement().text());

		m_trackDataVector.setCoverArtUrl(QString::null);
		const bool coverArt = getCoverArt();
		if (coverArt) {
			QString asin(release.namedItem("asin").toElement().text());
			if (!asin.isEmpty()) {
				m_trackDataVector.setCoverArtUrl(
					QString("http://www.amazon.com/dp/") + asin);
			}
		}

		const bool additionalTags = getAdditionalTags();
		if (additionalTags) {
			// release year and label can be found in the release-event-list
			QDomElement releaseEventList(release.namedItem("release-event-list").toElement());
			if (!releaseEventList.isNull()) {
				QDomElement event((releaseEventList.namedItem("event").toElement()));
				if (!event.isNull()) {
					QString date(event.attribute("date"));
					QRegExp dateRe("(\\d{4})(?:-\\d{2})?(?:-\\d{2})?");
					int year = 0;
					if (dateRe.exactMatch(date)) {
						year = dateRe.cap(1).toInt();
					} else {
						year = date.toInt();
					}
					if (year != 0) {
						framesHdr.setYear(year);
					}
					QString label(event.namedItem("label").namedItem("name").toElement().text());
					if (!label.isEmpty()) {
						framesHdr.setValue(Frame::FT_Publisher, label);
					}
				}
			}
		}

		if (additionalTags || coverArt) {
			QDomNode relationListNode(release.firstChild());
			while (!relationListNode.isNull()) {
				if (relationListNode.nodeName() == "relation-list") {
					QDomElement relationList(relationListNode.toElement());
					if (!relationList.isNull()) {
						QString targetType(relationList.attribute("target-type"));
						if (targetType == "Artist") {
							if (additionalTags) {
								parseCredits(relationList, framesHdr);
							}
						} else if (targetType == "Url") {
							if (coverArt) {
								QDomNode relationNode(relationList.firstChild());
								while (!relationNode.isNull()) {
									if (relationNode.nodeName() == "relation") {
										QDomElement relation(relationNode.toElement());
										if (!relation.isNull()) {
											QString type(relation.attribute("type"));
											if (type == "CoverArtLink" || type == "AmazonAsin") {
												m_trackDataVector.setCoverArtUrl(
													relation.attribute("target"));
											}
										}
									}
									relationNode = relationNode.nextSibling();
								}
							}
						}
					}
				}
				relationListNode = relationListNode.nextSibling();
			}
		}

		ImportTrackDataVector::iterator it = m_trackDataVector.begin();
		bool atTrackDataListEnd = (it == m_trackDataVector.end());
		int trackNr = 1;
		FrameCollection frames(framesHdr);
		QDomElement trackList = release.namedItem("track-list").toElement();
		for (QDomNode trackNode = trackList.namedItem("track");
				 !trackNode.isNull();
				 trackNode = trackNode.nextSibling()) {
			QDomElement track = trackNode.toElement();
			frames.setTrack(trackNr);
			frames.setTitle(track.namedItem("title").toElement().text());
			int duration = track.namedItem("duration").toElement().text().toInt() / 1000;
			if (additionalTags) {
				QString artist(track.namedItem("artist").toElement().
											 namedItem("name").toElement().text());
				if (!artist.isEmpty()) {
					// use the artist in the header as the album artist
					// and the artist in the track as the artist
					frames.setArtist(artist);
					frames.setValue(Frame::FT_AlbumArtist, framesHdr.getArtist());
				}
				QDomNode relationListNode(trackNode.firstChild());
				while (!relationListNode.isNull()) {
					if (relationListNode.nodeName() == "relation-list") {
						QDomElement relationList(relationListNode.toElement());
						if (!relationList.isNull()) {
							if (relationList.attribute("target-type") == "Artist") {
								parseCredits(relationList, frames);
							}
						}
					}
					relationListNode = relationListNode.nextSibling();
				}
			}
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
