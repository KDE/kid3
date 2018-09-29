/**
 * \file servertrackimporter.h
 * Abstract base class for track imports from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef SERVERTRACKIMPORTER_H
#define SERVERTRACKIMPORTER_H

#include <QObject>
#include "kid3api.h"

class QNetworkAccessManager;
class ImportTrackDataVector;
class TrackDataModel;
class ServerImporterConfig;
class HttpClient;

/**
 * Abstract base class for track imports from a server.
 */
class KID3_CORE_EXPORT ServerTrackImporter : public QObject
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
  ServerTrackImporter(QNetworkAccessManager* netMgr,
                      TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~ServerTrackImporter() override;

  /**
   * Name of import source.
   * @return name.
   */
  virtual const char* name() const = 0;

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
  virtual void start() = 0;

  /**
   * Reset the client state.
   */
  virtual void stop() = 0;

signals:
  /**
   * Emitted when status of a file changed.
   * Parameter: index of file, status text
   */
  void statusChanged(int, const QString&);

  /**
   * Emitted when results for a file are received.
   * Parameter index of file, track data list
   */
  void resultsReceived(int, ImportTrackDataVector&);

protected:
  /**
   * Access to HTTP client.
   * @return HTTP client.
   */
  HttpClient* httpClient() { return m_httpClient; }

  /**
   * @brief Access to track data model.
   * @return track data model.
   */
  TrackDataModel* trackDataModel() { return m_trackDataModel; }

private:
  HttpClient* m_httpClient;
  TrackDataModel* m_trackDataModel;
};

#endif // SERVERTRACKIMPORTER_H
