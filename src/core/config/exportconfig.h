/**
 * \file exportconfig.h
 * Configuration for export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Jun 2013
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

#ifndef EXPORTCONFIG_H
#define EXPORTCONFIG_H

#include <QStringList>
#include "generalconfig.h"
#include "trackdata.h"

/**
 * Export configuration.
 */
class KID3_CORE_EXPORT ExportConfig : public StoredConfig<ExportConfig> {
public:
  /**
   * Constructor.
   */
  ExportConfig();

  /**
   * Destructor.
   */
  virtual ~ExportConfig();

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

  /** Tag1 to export ID3v1 tags, Tag2 for ID3v2 tags */
  TrackData::TagVersion m_exportSrcV1;
  /** Names of export formats */
  QStringList m_exportFormatNames;
  /** regexp describing header export format */
  QStringList m_exportFormatHeaders;
  /** regexp describing track export format */
  QStringList m_exportFormatTracks;
  /** regexp describing trailer export format */
  QStringList m_exportFormatTrailers;
  /** selected export format */
  int m_exportFormatIdx;
  /** export window geometry */
  QByteArray m_exportWindowGeometry;
};

#endif
