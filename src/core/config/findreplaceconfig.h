/**
 * \file findreplaceconfig.h
 * Configuration for find/replace dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#ifndef FINDREPLACECONFIG_H
#define FINDREPLACECONFIG_H

#include <QStringList>
#include "config.h"
#include "generalconfig.h"
#include "tagsearcher.h"
#include "kid3api.h"

class BatchImportProfile;

/**
 * Filter configuration.
 */
class KID3_CORE_EXPORT FindReplaceConfig : public StoredConfig<FindReplaceConfig> {
public:
  /**
   * Constructor.
   */
  FindReplaceConfig();

  /**
   * Destructor.
   */
  virtual ~FindReplaceConfig();

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

  /**
   * Get search parameters.
   * @return search parameters.
   */
  const TagSearcher::Parameters& getParameters() const { return m_params; }

  /**
   * Set search parameters.
   * @param params search parameters
   */
  void setParameters(const TagSearcher::Parameters& params) {
    m_params = params;
  }

  /**
   * Get window geometry.
   * @return window geometry.
   */
  QByteArray getWindowGeometry() const { return m_windowGeometry; }

  /**
   * Set window geometry.
   * @param windowGeometry geometry
   */
  void setWindowGeometry(const QByteArray& windowGeometry) {
    m_windowGeometry = windowGeometry;
  }

  /** Index in configuration storage */
  static int s_index;

private:
  TagSearcher::Parameters m_params;
  QByteArray m_windowGeometry;
};

#endif
