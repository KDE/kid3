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

#include <QObject>
#include "servertrackimporter.h"
#include "trackdata.h"

class QByteArray;
class FingerprintCalculator;

/**
 * MusicBrainz client.
 */
class MusicBrainzClient : public ServerTrackImporter
{
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param netMgr network access manager
   * @param trackDataModel track data to be filled with imported values,
   *                       is passed with filenames set
   */
  MusicBrainzClient(QNetworkAccessManager* netMgr,
                    TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~MusicBrainzClient();

  /**
   * Name of import source.
   * @return name.
   */
  virtual const char* name() const;

  /** NULL-terminated array of server strings, 0 if not used */
  virtual const char** serverList() const;

  /** default server, 0 to disable */
  virtual const char* defaultServer() const;

  /** anchor to online help, 0 to disable */
  virtual const char* helpAnchor() const;

  /** configuration, 0 if not used */
  virtual ServerImporterConfig* config() const;

  /**
   * Set configuration.
   *
   * @param cfg import server configuration, 0 if not used
   */
  virtual void setConfig(const ServerImporterConfig* cfg);

  /**
   * Add the files in the file list.
   */
  virtual void start();

  /**
   * Reset the client state.
   */
  virtual void stop();

signals:
  /**
   * Emitted when status of a file changed.
   * Parameter: index of file, status text
   */
  void statusChanged(int, QString);

  /**
   * Emitted when results for a file are received.
   * Parameter index of file, track data list
   */
  void resultsReceived(int, ImportTrackDataVector&);

private slots:
  void receiveBytes(const QByteArray& bytes);

  void receiveFingerprint(const QString& fingerprint, int duration, int error);

private:
  enum State {
    Idle,
    CalculatingFingerprint,
    GettingIds,
    GettingMetadata
  };

  bool verifyIdIndex();
  bool verifyTrackIndex();
  void processNextStep();
  void processNextTrack();

  FingerprintCalculator* m_fingerprintCalculator;
  State m_state;
  QVector<QString> m_filenameOfTrack;
  QVector<QStringList> m_idsOfTrack;
  int m_currentIndex;
  ImportTrackDataVector m_currentTrackData;
  QMap<QByteArray, QByteArray> m_headers;
};

#endif // MUSICBRAINZCLIENT_H
