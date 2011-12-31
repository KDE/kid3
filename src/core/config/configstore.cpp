/**
 * \file configstore.cpp
 * Configuration storage.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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
#ifndef CONFIG_USE_KDE
#include "shortcutsmodel.h"
#endif

MiscConfig ConfigStore::s_miscCfg("General Options");
ImportConfig ConfigStore::s_genCfg("General Options");
FormatConfig ConfigStore::s_fnFormatCfg("FilenameFormat");
FormatConfig ConfigStore::s_id3FormatCfg("Id3Format");
FreedbConfig ConfigStore::s_freedbCfg("Freedb");
FreedbConfig ConfigStore::s_trackTypeCfg("TrackType");
DiscogsConfig ConfigStore::s_discogsCfg("Discogs");
AmazonConfig ConfigStore::s_amazonCfg("Amazon");
MusicBrainzConfig ConfigStore::s_musicBrainzCfg("MusicBrainz");
FilterConfig ConfigStore::s_filterCfg("Filter");
PlaylistConfig ConfigStore::s_playlistCfg("Playlist");

/**
 * Constructor.
 */
ConfigStore::ConfigStore()
{
#ifdef CONFIG_USE_KDE
  m_config = new KConfig;
#else
  m_config = new Kid3Settings(QSettings::UserScope, "kid3.sourceforge.net", "Kid3");
  m_config->beginGroup("/kid3");
  m_shortcutsModel = new ShortcutsModel;
#endif
}

/**
 * Destructor.
 */
ConfigStore::~ConfigStore()
{
#ifndef CONFIG_USE_KDE
  delete m_shortcutsModel;
#endif
  // m_config is not deleted because this could lead to a crash on Mac OS.
}

/**
 * Persist configuration.
 */
void ConfigStore::writeToConfig()
{
  s_miscCfg.writeToConfig(m_config);
  s_fnFormatCfg.writeToConfig(m_config);
  s_id3FormatCfg.writeToConfig(m_config);
  s_genCfg.writeToConfig(m_config);
  s_freedbCfg.writeToConfig(m_config);
  s_trackTypeCfg.writeToConfig(m_config);
  s_discogsCfg.writeToConfig(m_config);
  s_amazonCfg.writeToConfig(m_config);
  s_filterCfg.writeToConfig(m_config);
  s_playlistCfg.writeToConfig(m_config);
#ifdef HAVE_TUNEPIMP
  s_musicBrainzCfg.writeToConfig(m_config);
#endif
}

/**
 * Read persisted configuration.
 */
void ConfigStore::readFromConfig()
{
  s_miscCfg.readFromConfig(m_config);
  s_fnFormatCfg.readFromConfig(m_config);
  s_id3FormatCfg.readFromConfig(m_config);
  s_genCfg.readFromConfig(m_config);
  s_freedbCfg.readFromConfig(m_config);
  s_trackTypeCfg.readFromConfig(m_config);
  s_discogsCfg.readFromConfig(m_config);
  s_amazonCfg.readFromConfig(m_config);
  s_filterCfg.readFromConfig(m_config);
  s_playlistCfg.readFromConfig(m_config);
#ifdef HAVE_TUNEPIMP
  s_musicBrainzCfg.readFromConfig(m_config);
#endif
}
