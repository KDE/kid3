/**
 * \file tracktypeimporter.h
 * TrackType.org importer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Apr 2007
 *
 * Copyright (C) 2007-2011  Urs Fleisch
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

#ifndef TRACKTYPEIMPORTER_H
#define TRACKTYPEIMPORTER_H

#include "freedbimporter.h"

/**
 * TrackType.org importer.
 */
class TrackTypeImporter : public FreedbImporter
{
public:
  /**
   * Constructor.
   *
   * @param parent          parent object
   * @param trackDataModel track data to be filled with imported values
   */
  TrackTypeImporter(QObject* parent,
                    TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~TrackTypeImporter();

  /**
   * Name of import source.
   * @return name.
   */
  virtual const char* name() const;

  /** NULL-terminated array of server strings, 0 if not used */
  virtual const char** serverList() const;

  /** default server, 0 to disable */
  virtual const char* defaultServer() const;

  /** configuration, 0 if not used */
  virtual ServerImporterConfig* config() const;

  /**
   * Process finished findCddbAlbum request.
   *
   * @param searchStr search data received
   */
  virtual void parseFindResults(const QByteArray& searchStr);

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
};

#endif
