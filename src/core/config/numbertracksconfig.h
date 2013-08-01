/**
 * \file numbertracksconfig.h
 * Configuration for track numbering.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
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

#ifndef NUMBERTRACKSCONFIG_H
#define NUMBERTRACKSCONFIG_H

#include "generalconfig.h"
#include "trackdata.h"
#include "kid3api.h"

/**
 * Configuration for track numbering.
 */
class KID3_CORE_EXPORT NumberTracksConfig :
  public StoredConfig<NumberTracksConfig>
{
public:
  /**
   * Constructor.
   */
  NumberTracksConfig();

  /**
   * Destructor.
   */
  virtual ~NumberTracksConfig();

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config);

  /** number tracks in tags 1, tags 2, or both */
  TrackData::TagVersion m_numberTracksDst;
  /** number tracks start number */
  int m_numberTracksStart;

  /** Index in configuration storage */
  static int s_index;
};

#endif
