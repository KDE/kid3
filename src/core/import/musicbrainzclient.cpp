/**
 * \file musicbrainzclient.cpp
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 *
 * Copyright (C) 2005-2012  Urs Fleisch
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

#include "musicbrainzclient.h"
#ifdef HAVE_CHROMAPRINT

#include <QByteArray>
#include <QDomDocument>
#include "httpclient.h"
#include "trackdatamodel.h"
#include "fingerprintcalculator.h"
#include "qtcompatmac.h"

namespace {

/**
 * Parse response from acoustid.org.
 * @param bytes response in JSON format
 * @return list of MusicBrainz IDs
 */
QStringList parseAcoustidIds(const QByteArray& bytes)
{
  /*
   * The response from acoustid.org is in JSON format and looks like this:
   * {
   *   "status": "ok",
   *   "results": [{
   *     "recordings": [{"id": "14fef9a4-9b50-4e9f-9e22-490fd86d1861"}],
   *     "score": 0.938621, "id": "29bf7ce3-0182-40da-b840-5420203369c4"
   *   }]
   * }
   */
  QStringList ids;
  if (bytes.indexOf("\"status\": \"ok\"") >= 0) {
    int startPos = bytes.indexOf("\"recordings\": [");
    if (startPos >= 0) {
      startPos += 15;
      int endPos = bytes.indexOf(']', startPos);
      if (endPos > startPos) {
        QRegExp idRe("\"id\":\\s*\"([^\"]+)\"");
        QString recordings(bytes.mid(startPos, endPos - startPos));
        int pos = 0;
        while ((pos = idRe.indexIn(recordings, pos)) != -1) {
          ids.append(idRe.cap(1));
          pos += idRe.matchedLength();
        }
      }
    }
  }
  return ids;
}

/**
 * Parse response from MusicBrainz server.
 *
 * @param bytes XML response from MusicBrainz
 * @param trackDataVector the resulting track data will be appended to this
 *                        vector
 */
void parseMusicBrainzMetadata(const QByteArray& bytes,
                              ImportTrackDataVector& trackDataVector)
{
  /*
   * The XML response from MusicBrainz looks like this (simplified):
   * <?xml version="1.0" encoding="UTF-8"?>
   * <metadata xmlns="http://musicbrainz.org/ns/mmd-2.0#">
   *   <recording id="14fef9a4-9b50-4e9f-9e22-490fd86d1861">
   *     <title>Trip the Darkness</title>
   *     <length>192000</length>
   *     <artist-credit>
   *       <name-credit>
   *         <artist id="6fea1339-260c-40fe-bb7a-ace5c8438955">
   *           <name>Lacuna Coil</name>
   *         </artist>
   *       </name-credit>
   *     </artist-credit>
   *     <release-list count="2">
   *       <release id="aa7b7302-6ab0-409b-ab0f-b1e14732e11a">
   *         <title>Dark Adrenaline</title>
   *         <date>2012-01-24</date>
   *         <medium-list count="1">
   *           <medium>
   *             <track-list count="12" offset="0">
   *               <track>
   *                 <position>1</position>
   *               </track>
   *             </track-list>
   *           </medium>
   *         </medium-list>
   *       </release>
   *     </release-list>
   *   </recording>
   * </metadata>
   */
  int start = bytes.indexOf("<?xml");
  int end = bytes.indexOf("</metadata>");
  QByteArray xmlStr = start >= 0 && end > start ?
    bytes.mid(start, end + 11 - start) : bytes;
  QDomDocument doc;
  if (doc.setContent(xmlStr, false)) {
    QDomElement recording =
      doc.namedItem("metadata").namedItem("recording").toElement();
    if (!recording.isNull()) {
      bool ok;
      ImportTrackData frames;
      frames.setTitle(recording.namedItem("title").toElement().text());
      int length = recording.namedItem("length").toElement().text().toInt(&ok);
      if (ok) {
        frames.setImportDuration(length / 1000);
      }
      QDomNode artistNode = recording.namedItem("artist-credit");
      if (!artistNode.isNull()) {
        QString artist(artistNode.namedItem("name-credit").namedItem("artist").
            namedItem("name").toElement().text());
        frames.setArtist(artist);
      }
      QDomNode releaseNode = recording.namedItem("release-list").
          namedItem("release");
      if (!releaseNode.isNull()) {
        frames.setAlbum(releaseNode.namedItem("title").toElement().text());
        QString date(releaseNode.namedItem("date").toElement().text());
        if (!date.isEmpty()) {
          QRegExp dateRe("(\\d{4})(?:-\\d{2})?(?:-\\d{2})?");
          int year = 0;
          if (dateRe.exactMatch(date)) {
            year = dateRe.cap(1).toInt();
          } else {
            year = date.toInt();
          }
          if (year != 0) {
            frames.setYear(year);
          }
        }
        QDomNode trackNode = releaseNode.namedItem("medium-list").
            namedItem("medium").namedItem("track-list").namedItem("track");
        if (!trackNode.isNull()) {
          int trackNr = trackNode.namedItem("position").toElement().text().
              toInt(&ok);
          if (ok) {
            frames.setTrack(trackNr);
          }
        }
      }
      trackDataVector.append(frames);
    }
  }
}

}


/**
 * Constructor.
 *
 * @param trackDataModel track data to be filled with imported values,
 *                      is passed with filenames set
 */
MusicBrainzClient::MusicBrainzClient(TrackDataModel* trackDataModel) :
  m_httpClient(new HttpClient(this)),
  m_fingerprintCalculator(new FingerprintCalculator),
  m_trackDataModel(trackDataModel),
  m_state(Idle), m_currentIndex(-1)
{
  connect(m_httpClient, SIGNAL(bytesReceived(QByteArray)),
          this, SLOT(receiveBytes(QByteArray)));
}

/**
 * Destructor.
 */
MusicBrainzClient::~MusicBrainzClient()
{
  delete m_fingerprintCalculator;
}

/**
 * Verify if m_currentIndex is in range of m_idsOfTrack.
 * @return true if index OK, false if index was invalid and state is reset.
 */
bool MusicBrainzClient::verifyIdIndex()
{
  if (m_currentIndex < 0 || m_currentIndex >= m_idsOfTrack.size()) {
    qWarning("Invalid index %d for IDs (size %d)",
             m_currentIndex, m_idsOfTrack.size());
    resetState();
    return false;
  }
  return true;
}

/**
 * Verify if m_currentIndex is in range of m_filenameOfTrack.
 * @return true if index OK, false if index was invalid and state is reset.
 */
bool MusicBrainzClient::verifyTrackIndex()
{
  if (m_currentIndex < 0 || m_currentIndex >= m_filenameOfTrack.size()) {
    qWarning("Invalid index %d for track (size %d)",
             m_currentIndex, m_filenameOfTrack.size());
    resetState();
    return false;
  }
  return true;
}

/**
 * Reset the state to Idle and no track.
 */
void MusicBrainzClient::resetState()
{
  m_currentIndex = -1;
  m_state = Idle;
}

/**
 * Receive response from web service.
 * @param bytes bytes received
 */
void MusicBrainzClient::receiveBytes(const QByteArray& bytes)
{
  switch (m_state) {
  case GettingIds:
    if (!verifyIdIndex())
      return;
    m_idsOfTrack[m_currentIndex] = parseAcoustidIds(bytes);
    if (m_idsOfTrack.at(m_currentIndex).isEmpty()) {
      emit statusChanged(m_currentIndex, i18n("Unrecognized"));
    }
    m_state = GettingMetadata;
    processNextStep();
    break;
  case GettingMetadata:
    parseMusicBrainzMetadata(bytes, m_currentTrackData);
    if (!verifyIdIndex())
      return;
    if (m_idsOfTrack.at(m_currentIndex).isEmpty()) {
      int numResults = m_currentTrackData.size();
      if (numResults == 1) {
        emit statusChanged(m_currentIndex, i18n("Recognized"));
        emit metaDataReceived(m_currentIndex, m_currentTrackData.first());
      } else if (numResults > 1) {
        emit statusChanged(m_currentIndex, i18n("User Selection"));
        emit resultsReceived(m_currentIndex, m_currentTrackData);
      }
    }
    processNextStep();
    break;
  default:
    ;
  }
}

/**
 * Process next step in importing from fingerprints.
 */
void MusicBrainzClient::processNextStep()
{
  switch (m_state) {
  case Idle:
    break;
  case CalculatingFingerprint:
  {
    if (!verifyTrackIndex())
      return;
    emit statusChanged(m_currentIndex, i18n("Fingerprint"));
    FingerprintCalculator::Result fp =
        m_fingerprintCalculator->calculateFingerprint(
          m_filenameOfTrack.at(m_currentIndex));
    if (fp.getError() != FingerprintCalculator::Result::Ok) {
      emit statusChanged(m_currentIndex, i18n("Error"));
      processNextTrack();
    }
    m_state = GettingIds;
    emit statusChanged(m_currentIndex, i18n("ID Lookup"));
    QString path("/v2/lookup?client=LxDbFAXo&meta=recordingids&duration=" +
                 QString::number(fp.getDuration()) +
                 "&fingerprint=" + fp.getFingerprint());
    m_httpClient->sendRequest("api.acoustid.org", path);
    break;
  }
  case GettingMetadata:
  {
    if (!verifyIdIndex())
      return;
    QStringList& ids = m_idsOfTrack[m_currentIndex];
    if (!ids.isEmpty()) {
      emit statusChanged(m_currentIndex, i18n("Metadata Lookup"));
      QString path("/ws/2/recording/" + ids.takeFirst() +
                   "?inc=artists+releases+media");
      m_httpClient->sendRequest(m_musicBrainzServer, path);
    } else {
      processNextTrack();
    }
    break;
  }
  case GettingIds:
    qWarning("processNextStep() called in state GettingIds");
    resetState();
  }
}

/**
 * Process next track.
 * If all tracks have been processed, the state is reset to Idle.
 */
void MusicBrainzClient::processNextTrack()
{
  if (m_currentIndex < m_filenameOfTrack.size() - 1) {
    ++m_currentIndex;
    m_state = CalculatingFingerprint;
  } else {
    resetState();
  }
  m_currentTrackData.clear();
  processNextStep();
}

/**
 * Set configuration.
 *
 * @param server   server
 */
void MusicBrainzClient::setConfig(const QString& server)
{
  m_musicBrainzServer = server;
}

/**
 * Add the files in the file list.
 */
void MusicBrainzClient::addFiles()
{
  m_filenameOfTrack.clear();
  m_idsOfTrack.clear();
  const ImportTrackDataVector& trackDataVector(m_trackDataModel->trackData());
  for (ImportTrackDataVector::const_iterator it = trackDataVector.constBegin();
       it != trackDataVector.constEnd();
       ++it) {
    if (it->isEnabled()) {
      m_filenameOfTrack.append(it->getAbsFilename());
      m_idsOfTrack.append(QStringList());
    }
  }
  resetState();
  processNextTrack();
}

#endif // HAVE_CHROMAPRINT
