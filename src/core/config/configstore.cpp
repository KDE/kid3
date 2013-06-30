/**
 * \file configstore.cpp
 * Configuration storage.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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

#include "configstore.h"
#include "config.h"

TagConfig ConfigStore::s_tagCfg(QLatin1String("General Options"));
FileConfig ConfigStore::s_fileCfg(QLatin1String("General Options"));
RenDirConfig ConfigStore::s_renDirCfg(QLatin1String("General Options"));
NumberTracksConfig ConfigStore::s_numberTracksCfg(QLatin1String("General Options"));
UserActionsConfig ConfigStore::s_userActionsCfg(QLatin1String("MenuCommands"));
GuiConfig ConfigStore::s_guiCfg(QLatin1String("General Options"));
NetworkConfig ConfigStore::s_networkCfg(QLatin1String("General Options"));
ImportConfig ConfigStore::s_importCfg(QLatin1String("General Options"));
ExportConfig ConfigStore::s_exportCfg(QLatin1String("General Options"));
BatchImportConfig ConfigStore::s_batchImportCfg(QLatin1String("BatchImport"));
FormatConfig ConfigStore::s_fnFormatCfg(QLatin1String("FilenameFormat"));
FormatConfig ConfigStore::s_id3FormatCfg(QLatin1String("Id3Format"));
FreedbConfig ConfigStore::s_freedbCfg(QLatin1String("Freedb"));
FreedbConfig ConfigStore::s_trackTypeCfg(QLatin1String("TrackType"));
DiscogsConfig ConfigStore::s_discogsCfg(QLatin1String("Discogs"));
AmazonConfig ConfigStore::s_amazonCfg(QLatin1String("Amazon"));
MusicBrainzConfig ConfigStore::s_musicBrainzCfg(QLatin1String("MusicBrainz"));
FilterConfig ConfigStore::s_filterCfg(QLatin1String("Filter"));
PlaylistConfig ConfigStore::s_playlistCfg(QLatin1String("Playlist"));

/**
 * Constructor.
 * @param config application settings
 */
ConfigStore::ConfigStore(ISettings* config) : m_config(config)
{
}

/**
 * Destructor.
 */
ConfigStore::~ConfigStore()
{
}

/**
 * Persist configuration.
 */
void ConfigStore::writeToConfig()
{
  s_tagCfg.writeToConfig(m_config);
  s_fileCfg.writeToConfig(m_config);
  s_renDirCfg.writeToConfig(m_config);
  s_numberTracksCfg.writeToConfig(m_config);
  s_userActionsCfg.writeToConfig(m_config);
  s_guiCfg.writeToConfig(m_config);
  s_networkCfg.writeToConfig(m_config);
  s_fnFormatCfg.writeToConfig(m_config);
  s_id3FormatCfg.writeToConfig(m_config);
  s_importCfg.writeToConfig(m_config);
  s_exportCfg.writeToConfig(m_config);
  s_batchImportCfg.writeToConfig(m_config);
  s_freedbCfg.writeToConfig(m_config);
  s_trackTypeCfg.writeToConfig(m_config);
  s_discogsCfg.writeToConfig(m_config);
  s_amazonCfg.writeToConfig(m_config);
  s_filterCfg.writeToConfig(m_config);
  s_playlistCfg.writeToConfig(m_config);
  s_musicBrainzCfg.writeToConfig(m_config);
}

/**
 * Read persisted configuration.
 */
void ConfigStore::readFromConfig()
{
  s_tagCfg.readFromConfig(m_config);
  s_fileCfg.readFromConfig(m_config);
  s_renDirCfg.readFromConfig(m_config);
  s_numberTracksCfg.readFromConfig(m_config);
  s_userActionsCfg.readFromConfig(m_config);
  s_guiCfg.readFromConfig(m_config);
  s_networkCfg.readFromConfig(m_config);
  s_fnFormatCfg.readFromConfig(m_config);
  s_id3FormatCfg.readFromConfig(m_config);
  s_importCfg.readFromConfig(m_config);
  s_exportCfg.readFromConfig(m_config);
  s_batchImportCfg.readFromConfig(m_config);
  s_freedbCfg.readFromConfig(m_config);
  s_trackTypeCfg.readFromConfig(m_config);
  s_discogsCfg.readFromConfig(m_config);
  s_amazonCfg.readFromConfig(m_config);
  s_filterCfg.readFromConfig(m_config);
  s_playlistCfg.readFromConfig(m_config);
  s_musicBrainzCfg.readFromConfig(m_config);
}
