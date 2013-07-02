/**
 * \file importconfig.h
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2009  Urs Fleisch
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

#ifndef IMPORTCONFIG_H
#define IMPORTCONFIG_H

#include <QStringList>
#include <QMap>
#include "config.h"
#include "generalconfig.h"
#include "trackdata.h"

/**
 * Import configuration.
 */
class KID3_CORE_EXPORT ImportConfig : public StoredConfig<ImportConfig> {
public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp configuration group
   */
  explicit ImportConfig(const QString& grp);

  /**
   * Destructor.
   */
  virtual ~ImportConfig();

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

  /** import server */
  int m_importServer;
  /** tag version to import */
  TrackData::TagVersion m_importDest;
  /** Names of import formats */
  QStringList m_importFormatNames;
  /** regexp describing header import format */
  QStringList m_importFormatHeaders;
  /** regexp describing track import format */
  QStringList m_importFormatTracks;
  /** selected import format */
  int m_importFormatIdx;
  /** check maximum allowable time difference */
  bool m_enableTimeDifferenceCheck;
  /** maximum allowable time difference */
  int m_maxTimeDifference;
  /** visible optional columns in import table */
  quint64 m_importVisibleColumns;
  /** import window geometry */
  QByteArray m_importWindowGeometry;

  /** Names of import tags formats */
  QStringList m_importTagsNames;
  /** Expressions for tag import sources */
  QStringList m_importTagsSources;
  /** regexp describing extraction from import tag sources */
  QStringList m_importTagsExtractions;
  /** selected import tags format */
  int m_importTagsIdx;

  /** names of picture sources */
  QStringList m_pictureSourceNames;
  /** picture source URLs */
  QStringList m_pictureSourceUrls;
  /** selected picture source */
  int m_pictureSourceIdx;
  /** Browse cover art window geometry */
  QByteArray m_browseCoverArtWindowGeometry;
  /** Mapping for picture URL matching */
  QMap<QString, QString> m_matchPictureUrlMap;

  /** Last directory used for import or export. */
  QString m_importDir;
};

#endif
