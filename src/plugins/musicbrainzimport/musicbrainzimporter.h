/**
 * \file musicbrainzimporter.h
 * MusicBrainz release database importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 *
 * Copyright (C) 2006-2024  Urs Fleisch
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

#include "serverimporter.h"

/**
 * MusicBrainz release database importer.
 */
class MusicBrainzImporter : public ServerImporter
{
public:
  /**
   * Constructor.
   *
   * @param netMgr network access manager
   * @param trackDataModel track data to be filled with imported values
   */
  MusicBrainzImporter(QNetworkAccessManager* netMgr,
                      TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  ~MusicBrainzImporter() override = default;

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

  /** additional tags option, false if not used */
  bool additionalTags() const override;

  /**
   * Process finished findCddbAlbum request.
   *
   * @param searchStr search data received
   */
  void parseFindResults(const QByteArray& searchStr) override;

  /**
   * Parse result of album request and populate m_trackDataModel with results.
   *
   * @param albumStr album data received
   */
  void parseAlbumResults(const QByteArray& albumStr) override;

  /**
   * Send a query command to search on the server.
   *
   * @param cfg      import source configuration
   * @param artist   artist to search
   * @param album    album to search
   */
  void sendFindQuery(
    const ServerImporterConfig* cfg,
    const QString& artist, const QString& album) override;

  /**
   * Send a query command to fetch the track list
   * from the server.
   *
   * @param cfg      import source configuration
   * @param cat      category
   * @param id       ID
   */
  void sendTrackListQuery(
    const ServerImporterConfig* cfg, const QString& cat, const QString& id) override;

private:
  QMap<QByteArray, QByteArray> m_headers;
};
