/**
 * \file batchimportconfig.h
 * Configuration for batch import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2013
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

#ifndef BATCHIMPORTCONFIG_H
#define BATCHIMPORTCONFIG_H

#include <QStringList>
#include "config.h"
#include "generalconfig.h"
#include "trackdata.h"
#include "kid3api.h"

class BatchImportProfile;

/**
 * Filter configuration.
 */
class KID3_CORE_EXPORT BatchImportConfig : public GeneralConfig {
public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp configuration group
   */
  explicit BatchImportConfig(const QString& grp);

  /**
   * Destructor.
   */
  virtual ~BatchImportConfig();

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  virtual void writeToConfig(Kid3Settings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(Kid3Settings* config);

  /**
   * Get a batch import profile.
   *
   * @param name name of profile
   * @param profile the profile will be returned here
   * @return true if profile with @a name found.
   */
  bool getProfileByName(const QString& name, BatchImportProfile& profile) const;

  /** tag version to import */
  TrackData::TagVersion m_importDest;
  /** Names of profiles */
  QStringList m_profileNames;
  /** Profile import sources */
  QStringList m_profileSources;
  /** Selected profile */
  int m_profileIdx;
  /** Window geometry */
  QByteArray m_windowGeometry;
};

#endif
