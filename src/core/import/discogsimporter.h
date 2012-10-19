/**
 * \file discogsimporter.h
 * Discogs importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Oct 2006
 *
 * Copyright (C) 2006-2012  Urs Fleisch
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

#ifndef DISCOGSIMPORTER_H
#define DISCOGSIMPORTER_H

#include "serverimporter.h"
#include "kid3api.h"

/**
 * Discogs importer.
 */
class KID3_CORE_EXPORT DiscogsImporter : public ServerImporter
{
public:
  /**
   * Constructor.
   *
   * @param netMgr network access manager
   * @param trackDataModel track data to be filled with imported values
   */
  DiscogsImporter(QNetworkAccessManager* netMgr,
                  TrackDataModel *trackDataModel);

  /**
   * Destructor.
   */
  virtual ~DiscogsImporter();

  /**
   * Name of import source.
   * @return name.
   */
  virtual const char* name() const;

  /** anchor to online help, 0 to disable */
  virtual const char* helpAnchor() const;

  /** configuration, 0 if not used */
  virtual ServerImporterConfig* config() const;

  /** additional tags option, false if not used */
  virtual bool additionalTags() const;

  /**
   * Process finished findCddbAlbum request.
   *
   * @param searchStr search data received
   */
  virtual void parseFindResults(const QByteArray& searchStr);

  /**
   * Parse result of album request and populate m_trackDataModel with results.
   *
   * @param albumStr album data received
   */
  virtual void parseAlbumResults(const QByteArray& albumStr);

  /**
   * Send a query command to search on the server.
   *
   * @param cfg      import source configuration
   * @param artist   artist to search
   * @param album    album to search
   */
  virtual void sendFindQuery(
    const ServerImporterConfig* cfg,
    const QString& artist, const QString& album);

  /**
   * Send a query command to fetch the track list
   * from the server.
   *
   * @param cfg      import source configuration
   * @param cat      category
   * @param id       ID
   */
  virtual void sendTrackListQuery(
    const ServerImporterConfig* cfg, const QString& cat, const QString& id);

private:
  QMap<QByteArray, QByteArray> m_discogsHeaders;
};

#endif
