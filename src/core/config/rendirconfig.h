/**
 * \file rendirconfig.h
 * Configuration for directory renaming.
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

#ifndef RENDIRCONFIG_H
#define RENDIRCONFIG_H

#include "generalconfig.h"
#include "trackdata.h"
#include "kid3api.h"

/**
 * Configuration for directory renaming.
 */
class KID3_CORE_EXPORT RenDirConfig : public StoredConfig<RenDirConfig>
{
public:
  /**
   * Constructor.
   *
   * @param group configuration group
   */
  explicit RenDirConfig(const QString& group);

  /**
   * Destructor.
   */
  virtual ~RenDirConfig();

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

  /** directory name format */
  QString m_dirFormatText;
  /** index of directory name format selected */
  int m_dirFormatItem;
  /** rename directory from tags 1, tags 2, or both */
  TrackData::TagVersion m_renDirSrc;

  /** Default directory format list */
  static const char** s_defaultDirFmtList;
};

#endif
