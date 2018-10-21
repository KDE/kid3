/**
 * \file servertrackimporter.cpp
 * Abstract base class for track imports from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Jun 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "servertrackimporter.h"
#include "httpclient.h"

/**
 * Constructor.
 *
 * @param netMgr network access manager
 * @param trackDataModel track data to be filled with imported values,
 *                       is passed with filenames set
 */
ServerTrackImporter::ServerTrackImporter(QNetworkAccessManager* netMgr,
                                         TrackDataModel* trackDataModel)
  : QObject(netMgr),
    m_httpClient(new HttpClient(netMgr)),
    m_trackDataModel(trackDataModel) {
}

/** NULL-terminated array of server strings, 0 if not used */
const char** ServerTrackImporter::serverList() const { return nullptr; }

/** default server, 0 to disable */
const char* ServerTrackImporter::defaultServer() const { return nullptr; }

/** anchor to online help, 0 to disable */
const char* ServerTrackImporter::helpAnchor() const { return nullptr; }

/** configuration, 0 if not used */
ServerImporterConfig* ServerTrackImporter::config() const { return nullptr; }

/**
 * Set configuration.
 *
 * @param cfg import server configuration, 0 if not used
 */
void ServerTrackImporter::setConfig(const ServerImporterConfig* cfg)
{
  Q_UNUSED(cfg)
}
