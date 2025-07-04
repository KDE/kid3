/**
 * \file musicbrainzclient.cpp
 * MusicBrainz client.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Sep 2005
 *
 * Copyright (C) 2005-2024  Urs Fleisch
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
#include <QByteArray>
#include <QDomDocument>
#include <QRegularExpression>
#include "httpclient.h"
#include "trackdatamodel.h"
#include "fingerprintcalculator.h"

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
  if (bytes.indexOf(R"("status": "ok")") >= 0) {
    if (int startPos = bytes.indexOf("\"recordings\": ["); startPos >= 0) {
      startPos += 15;
      if (int endPos = bytes.indexOf(']', startPos); endPos > startPos) {
        QRegularExpression idRe(QLatin1String("\"id\":\\s*\"([^\"]+)\""));
        QString recordings(QString::fromLatin1(bytes.mid(startPos,
                                                         endPos - startPos)));
        auto it = idRe.globalMatch(recordings);
        while (it.hasNext()) {
          auto match = it.next();
          ids.append(match.captured(1));
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
  if (QDomDocument doc;
#if QT_VERSION >= 0x060500
      doc.setContent(xmlStr)
#else
      doc.setContent(xmlStr, false)
#endif
     ) {
    if (QDomElement recording =
          doc.namedItem(QLatin1String("metadata"))
             .namedItem(QLatin1String("recording")).toElement();
        !recording.isNull()) {
      bool ok;
      ImportTrackData frames;
      frames.setTitle(recording.namedItem(QLatin1String("title")).toElement()
                      .text());
      int length = recording.namedItem(QLatin1String("length")).toElement()
          .text().toInt(&ok);
      if (ok) {
        frames.setImportDuration(length / 1000);
      }
      if (QDomNode artistNode = recording.namedItem(QLatin1String("artist-credit"));
          !artistNode.isNull()) {
        QString artist(artistNode.namedItem(QLatin1String("name-credit"))
                       .namedItem(QLatin1String("artist"))
                       .namedItem(QLatin1String("name")).toElement().text());
        frames.setArtist(artist);
      }
      if (QDomNode releaseNode = recording.namedItem(QLatin1String("release-list"))
                                          .namedItem(QLatin1String("release"));
          !releaseNode.isNull()) {
        frames.setAlbum(releaseNode.namedItem(QLatin1String("title"))
                        .toElement().text());
        if (QString date(releaseNode.namedItem(QLatin1String("date")).toElement()
                                    .text());
            !date.isEmpty()) {
          QRegularExpression dateRe(QLatin1String(R"(^(\d{4})(?:-\d{2})?(?:-\d{2})?$)"));
          auto match = dateRe.match(date);
          int year = 0;
          if (match.hasMatch()) {
            year = match.captured(1).toInt();
          } else {
            year = date.toInt();
          }
          if (year != 0) {
            frames.setYear(year);
          }
        }
        if (QDomNode trackNode = releaseNode
                                 .namedItem(QLatin1String("medium-list"))
                                 .namedItem(QLatin1String("medium"))
                                 .namedItem(QLatin1String("track-list"))
                                 .namedItem(QLatin1String("track"));
            !trackNode.isNull()) {
          int trackNr = trackNode.namedItem(QLatin1String("position"))
              .toElement().text().toInt(&ok);
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
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values,
 *                      is passed with filenames set
 */
MusicBrainzClient::MusicBrainzClient(QNetworkAccessManager* netMgr,
                                     TrackDataModel *trackDataModel)
  : ServerTrackImporter(netMgr, trackDataModel),
    m_fingerprintCalculator(new FingerprintCalculator(this)),
    m_state(Idle), m_currentIndex(-1)
{
  m_headers["User-Agent"] = "curl/7.52.1";
  connect(httpClient(), &HttpClient::bytesReceived,
          this, &MusicBrainzClient::receiveBytes);
  connect(m_fingerprintCalculator, &FingerprintCalculator::finished,
          this, &MusicBrainzClient::receiveFingerprint);
}

/**
 * Name of import source.
 * @return name.
 */
const char* MusicBrainzClient::name() const {
  return QT_TRANSLATE_NOOP("@default", "MusicBrainz Fingerprint");
}

/** NULL-terminated array of server strings, 0 if not used */
const char** MusicBrainzClient::serverList() const
{
  return nullptr;
}

/** default server, 0 to disable */
const char* MusicBrainzClient::defaultServer() const {
  return nullptr;
}

/** anchor to online help, 0 to disable */
const char* MusicBrainzClient::helpAnchor() const {
  return "import-musicbrainz";
}

/** configuration, 0 if not used */
ServerImporterConfig* MusicBrainzClient::config() const {
  return nullptr;
}

/**
 * Verify if m_currentIndex is in range of m_idsOfTrack.
 * @return true if index OK, false if index was invalid and state is reset.
 */
bool MusicBrainzClient::verifyIdIndex()
{
  if (m_currentIndex < 0 || m_currentIndex >= m_idsOfTrack.size()) {
    qWarning("Invalid index %d for IDs (size %d)",
             m_currentIndex, static_cast<int>(m_idsOfTrack.size()));
    stop();
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
             m_currentIndex, static_cast<int>(m_filenameOfTrack.size()));
    stop();
    return false;
  }
  return true;
}

/**
 * Reset the state to Idle and no track.
 */
void MusicBrainzClient::stop()
{
  m_fingerprintCalculator->stop();
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
      emit statusChanged(m_currentIndex, tr("Unrecognized"));
    }
    m_state = GettingMetadata;
    processNextStep();
    break;
  case GettingMetadata:
    parseMusicBrainzMetadata(bytes, m_currentTrackData);
    if (!verifyIdIndex())
      return;
    if (m_idsOfTrack.at(m_currentIndex).isEmpty()) {
      emit statusChanged(m_currentIndex, m_currentTrackData.size() == 1
                         ? tr("Recognized") : tr("User Selection"));
      emit resultsReceived(m_currentIndex, m_currentTrackData);
    }
    processNextStep();
    break;
  default:
    ;
  }
}

/**
 * Receive fingerprint from decoder.
 *
 * @param fingerprint Chromaprint fingerprint
 * @param duration duration in seconds
 * @param error error code
 */
void MusicBrainzClient::receiveFingerprint(const QString& fingerprint,
                                           int duration, int error)
{
  if (error == FingerprintCalculator::Ok) {
    m_state = GettingIds;
    emit statusChanged(m_currentIndex, tr("ID Lookup"));
    QString path(
      QLatin1String("/v2/lookup?client=LxDbFAXo&meta=recordingids&duration=") +
      QString::number(duration) +
      QLatin1String("&fingerprint=") + fingerprint);
    httpClient()->sendRequest(QLatin1String("api.acoustid.org"), path,
                              QLatin1String("https"));
  } else {
    emit statusChanged(m_currentIndex, tr("Error"));
    if (m_state != Idle) {
      processNextTrack();
    }
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
    emit statusChanged(m_currentIndex, tr("Fingerprint"));
    m_fingerprintCalculator->start(m_filenameOfTrack.at(m_currentIndex));
    break;
  }
  case GettingMetadata:
  {
    if (!verifyIdIndex())
      return;
    if (QStringList& ids = m_idsOfTrack[m_currentIndex]; !ids.isEmpty()) {
      emit statusChanged(m_currentIndex, tr("Metadata Lookup"));
      QString path(QLatin1String("/ws/2/recording/") + ids.takeFirst() +
                   QLatin1String("?inc=artists+releases+media"));
      httpClient()->sendRequest(QLatin1String("musicbrainz.org"), path,
                                QLatin1String("https"), m_headers);
    } else {
      processNextTrack();
    }
    break;
  }
  case GettingIds:
    qWarning("processNextStep() called in state GettingIds");
    stop();
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
    stop();
  }
  m_currentTrackData.clear();
  processNextStep();
}

/**
 * Set configuration.
 *
 * @param cfg import server configuration, 0 if not used
 */
void MusicBrainzClient::setConfig(const ServerImporterConfig* cfg)
{
  Q_UNUSED(cfg)
}

/**
 * Add the files in the file list.
 */
void MusicBrainzClient::start()
{
  m_filenameOfTrack.clear();
  m_idsOfTrack.clear();
  const ImportTrackDataVector& trackDataVector(trackDataModel()->trackData());
  for (auto it = trackDataVector.constBegin();
       it != trackDataVector.constEnd();
       ++it) {
    if (it->isEnabled()) {
      m_filenameOfTrack.append(it->getAbsFilename());
      m_idsOfTrack.append(QStringList());
    }
  }
  stop();
  processNextTrack();
}
