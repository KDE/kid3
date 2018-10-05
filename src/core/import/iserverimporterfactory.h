/**
 * \file iserverimporterfactory.h
 * Interface for server importer factory.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jul 2013
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

#pragma once

#include <QtPlugin>
#include "kid3api.h"

class QString;
class QStringList;
class QNetworkAccessManager;
class TrackDataModel;
class ServerImporter;

/**
 * Interface for server importer factory.
 */
class KID3_CORE_EXPORT IServerImporterFactory {
public:
  /**
   * Destructor.
   */
  virtual ~IServerImporterFactory();

  /**
   * Get keys of available server importers.
   * @return list of keys.
   */
  virtual QStringList serverImporterKeys() const = 0;

  /**
   * Create server importer.
   * @param key server importer key
   * @param netMgr network access manager
   * @param trackDataModel track data to be filled with imported values
   * @return server importer instance, 0 if key unknown.
   * @remarks The caller takes ownership of the returned instance.
   */
  virtual ServerImporter* createServerImporter(
      const QString& key,
      QNetworkAccessManager* netMgr, TrackDataModel* trackDataModel) = 0;
};

Q_DECLARE_INTERFACE(IServerImporterFactory,
                    "net.sourceforge.kid3.IServerImporterFactory")
