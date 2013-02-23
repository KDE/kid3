/**
 * \file serverimporterconfig.h
 * Configuration for server import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#ifndef SERVERIMPORTERCONFIG_H
#define SERVERIMPORTERCONFIG_H

#include "generalconfig.h"
#include <QString>
#include "kid3api.h"

/**
 * Freedb configuration.
 */
class KID3_CORE_EXPORT ServerImporterConfig : public GeneralConfig {
public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp         configuration group
   * @param cgiPathUsed true to use CgiPath configuration
   * @param additionalTagsUsed true to use AdditionalTags configuration
   */
  ServerImporterConfig(const QString& grp, bool cgiPathUsed = true,
                       bool additionalTagsUsed = false);

  /**
   * Constructor.
   * Used to create temporary configuration.
   */
  ServerImporterConfig();

  /**
   * Destructor.
   */
  virtual ~ServerImporterConfig();

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

  /** server */
  QString m_server;

  /** CGI path used for access */
  QString m_cgiPath;

  /** window geometry */
  QByteArray m_windowGeometry;

  /** true if CgiPath configuration is used */
  bool m_cgiPathUsed;

  /** true if additional tags configuration is used */
  bool m_additionalTagsUsed;

  /** standard tags imported */
  bool m_standardTags;

  /** additional tags imported */
  bool m_additionalTags;

  /** cover art imported */
  bool m_coverArt;
};

#endif
