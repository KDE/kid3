/**
 * \file musicbrainzreleasedialog.cpp
 * MusicBrainz release database import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
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
	&Kid3App::s_musicBrainzCfg
};


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataVector track data to be filled with imported values
 */
MusicBrainzReleaseDialog::MusicBrainzReleaseDialog(
	QWidget *parent,
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
		StandardTags stHdr;
		stHdr.setInactive();
		stHdr.album = release.namedItem("title").toElement().text();
		stHdr.artist = release.namedItem("artist").toElement().namedItem("name").toElement().text();

		ImportTrackDataVector::iterator it = m_trackDataVector.begin();
		bool atTrackDataListEnd = (it == m_trackDataVector.end());
		int trackNr = 1;
		StandardTags st(stHdr);
		QDomElement trackList = release.namedItem("track-list").toElement();
		for (QDomNode trackNode = trackList.namedItem("track");
				 !trackNode.isNull();
				 trackNode = trackNode.nextSibling()) {
			QDomElement track = trackNode.toElement();
			st.track = trackNr;
			st.title = track.namedItem("title").toElement().text();
			int duration = track.namedItem("duration").toElement().text().toInt() / 1000;
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
