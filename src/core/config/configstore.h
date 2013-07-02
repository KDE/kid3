/**
 * \file configstore.h
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

#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include "generalconfig.h" // Kid3Settings
#include "formatconfig.h"
#include "importconfig.h"
#include "exportconfig.h"
#include "batchimportconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"
#include "rendirconfig.h"
#include "numbertracksconfig.h"
#include "useractionsconfig.h"
#include "guiconfig.h"
#include "networkconfig.h"
#include "freedbconfig.h"
#include "discogsconfig.h"
#include "amazonconfig.h"
#include "musicbrainzconfig.h"
#include "filterconfig.h"
#include "playlistconfig.h"

/**
 * Configuration storage.
 */
class KID3_CORE_EXPORT ConfigStore {
public:
  /**
   * Constructor.
   * @param config application settings
   */
  explicit ConfigStore(ISettings* config);

  /**
   * Destructor.
   */
  ~ConfigStore();

  /**
   * Persist configuration.
   */
  void writeToConfig();

  /**
   * Read persisted configuration.
   */
  void readFromConfig();


  /** Filename format configuration */
  static FilenameFormatConfig s_fnFormatCfg;
  /** Tag format configuration */
  static TagFormatConfig s_id3FormatCfg;
  /** Import configuration */
  static ImportConfig s_importCfg;
  /** Export configuration */
  static ExportConfig s_exportCfg;
  /** Batch import configuration */
  static BatchImportConfig s_batchImportCfg;
  /** Tag configuration */
  static TagConfig s_tagCfg;
  /** File configuration */
  static FileConfig s_fileCfg;
  /** Rename directory configuration */
  static RenDirConfig s_renDirCfg;
  /** Number tracks configuration */
  static NumberTracksConfig s_numberTracksCfg;
  /** User actions configuration */
  static UserActionsConfig s_userActionsCfg;
  /** GUI configuration */
  static GuiConfig s_guiCfg;
  /** Network configuration */
  static NetworkConfig s_networkCfg;
  /** Freedb configuration */
  static FreedbConfig s_freedbCfg;
  /** TrackType configuration */
  static TrackTypeConfig s_trackTypeCfg;
  /** Discogs configuration */
  static DiscogsConfig s_discogsCfg;
  /** Amazon configuration */
  static AmazonConfig s_amazonCfg;
  /** MusicBrainz configuration */
  static MusicBrainzConfig s_musicBrainzCfg;
  /** Filter configuration */
  static FilterConfig s_filterCfg;
  /** Playlist configuration */
  static PlaylistConfig s_playlistCfg;

private:
  ISettings* m_config;
};

#endif // CONFIGSTORE_H
