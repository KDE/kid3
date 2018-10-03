/**
 * \file acoustidimportplugin.h
 * AcoustID import plugin.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Jul 2013
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

#ifndef ACOUSTIDIMPORTPLUGIN_H
#define ACOUSTIDIMPORTPLUGIN_H

#include <QObject>
#include "iservertrackimporterfactory.h"

/**
 * AcoustID import plugin.
 */
class KID3_PLUGIN_EXPORT AcoustidImportPlugin :
    public QObject, public IServerTrackImporterFactory {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "net.sourceforge.kid3.IServerTrackImporterFactory")
  Q_INTERFACES(IServerTrackImporterFactory)
public:
  /*!
   * Constructor.
   * @param parent parent object
   */
  explicit AcoustidImportPlugin(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~AcoustidImportPlugin() override = default;

  /**
   * Get keys of available server importers.
   * @return list of keys.
   */
  virtual QStringList serverTrackImporterKeys() const override;

  /**
   * Create server importer.
   * @param key server importer key
   * @param netMgr network access manager
   * @param trackDataModel track data to be filled with imported values
   * @return server importer instance, 0 if key unknown.
   * @remarks The caller takes ownership of the returned instance.
   */
  virtual ServerTrackImporter* createServerTrackImporter(
      const QString& key,
      QNetworkAccessManager* netMgr, TrackDataModel* trackDataModel) override;
};

#endif // ACOUSTIDIMPORTPLUGIN_H
