/**
 * \file filterconfig.h
 * Configuration for filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
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

#ifndef FILTERCONFIG_H
#define FILTERCONFIG_H

#include <QStringList>
#include "config.h"
#include "generalconfig.h"
#include "kid3api.h"

/**
 * Filter configuration.
 */
class KID3_CORE_EXPORT FilterConfig : public StoredConfig<FilterConfig> {
public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp configuration group
   */
  explicit FilterConfig(const QString& grp);

  /**
   * Destructor.
   */
  virtual ~FilterConfig();

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  virtual void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(ISettings* config);

  /**
   * Set the filename format in the "Filename Tag Mismatch" filter.
   *
   * @param format filename format
   */
  void setFilenameFormat(const QString& format);

  /** Names of filter expressions */
  QStringList m_filterNames;
  /** Filter expressions */
  QStringList m_filterExpressions;
  /** Selected filter */
  int m_filterIdx;
  /** Window geometry */
  QByteArray m_windowGeometry;
};

#endif
