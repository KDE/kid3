/**
 * \file musicbrainzclient.h
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

#pragma once

#include <QObject>
#include "servertrackimporter.h"
#include "trackdata.h"

class QByteArray;
class FingerprintCalculator;

/**
 * MusicBrainz client.
 */
class MusicBrainzClient : public ServerTrackImporter {
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
  ~MusicBrainzClient() override = default;

  /**
   * Name of import source.
   * @return name.
   */
  const char* name() const override;

  /** NULL-terminated array of server strings, 0 if not used */
  const char** serverList() const override;

  /** default server, 0 to disable */
  const char* defaultServer() const override;

  /** anchor to online help, 0 to disable */
  const char* helpAnchor() const override;

  /** configuration, 0 if not used */
  ServerImporterConfig* config() const override;

  /**
   * Set configuration.
   *
   * @param cfg import server configuration, 0 if not used
   */
  void setConfig(const ServerImporterConfig* cfg) override;

  /**
   * Add the files in the file list.
   */
  void start() override;

  /**
   * Reset the client state.
   */
  void stop() override;

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
