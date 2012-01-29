/**
 * \file musicbrainzclient.h
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

#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include "config.h"

#include <QObject>

#ifdef HAVE_CHROMAPRINT

#include "trackdata.h"

class QByteArray;
class ImportTrackData;
class ImportTrackDataVector;
class TrackDataModel;
class HttpClient;
class FingerprintCalculator;

/**
 * MusicBrainz client.
 */
class MusicBrainzClient : public QObject
{
Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param trackDataModel track data to be filled with imported values,
   *                       is passed with filenames set
   */
  explicit MusicBrainzClient(TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~MusicBrainzClient();

  /**
   * Set configuration.
   *
   * @param server server
   */
  void setConfig(const QString& server);

  /**
   * Add the files in the file list.
   */
  void addFiles();

signals:
  /**
   * Emitted when status of a file changed.
   * Parameter: index of file, status text
   */
  void statusChanged(int, QString);

  /**
   * Emitted when meta data for a recognized file are received.
   * Parameter index of file, track data
   */
  void metaDataReceived(int, ImportTrackData&);

  /**
   * Emitted when results for an ambiguous file are received.
   * Parameter index of file, track data list
   */
  void resultsReceived(int, ImportTrackDataVector&);

private slots:
  void receiveBytes(const QByteArray& bytes);

private:
  enum State {
    Idle,
    CalculatingFingerprint,
    GettingIds,
    GettingMetadata
  };

  bool verifyIdIndex();
  bool verifyTrackIndex();
  void resetState();
  void processNextStep();
  void processNextTrack();

  HttpClient* m_httpClient;
  FingerprintCalculator* m_fingerprintCalculator;
  TrackDataModel* m_trackDataModel;
  State m_state;
  QVector<QString> m_filenameOfTrack;
  QVector<QStringList> m_idsOfTrack;
  int m_currentIndex;
  ImportTrackDataVector m_currentTrackData;
  QString m_musicBrainzServer;
};

#else // HAVE_CHROMAPRINT

// Just to suppress moc "No relevant classes found" warning.
class MusicBrainzClient : public QObject {
Q_OBJECT
};

#endif // HAVE_CHROMAPRINT

#endif // MUSICBRAINZCLIENT_H
