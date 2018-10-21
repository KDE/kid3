/**
 * \file testserverimporterbase.h
 * Base class for server importer tests.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Oct 2012
 *
 * Copyright (C) 2012-2018  Urs Fleisch
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

#include <QObject>

class QNetworkAccessManager;
class TrackDataModel;
class ServerImporter;
class ISettings;
class ConfigStore;

/**
 * Base class for server importer tests.
 */
class TestServerImporterBase : public QObject {
  Q_OBJECT
public:
  explicit TestServerImporterBase(QObject* parent = nullptr);
  virtual ~TestServerImporterBase() override;

public slots:
  void onFindFinished(const QByteArray& searchStr);
  void onAlbumFinished(const QByteArray& albumStr);

signals:
  void albumsUpdated();
  void trackDataUpdated();

protected:
  void setServerImporter(ServerImporter* importer);
  void setServerImporter(const QString& key);
  void queryAlbums(const QString& artist, const QString& album);
  void queryTracks(const QString& cat, const QString& id);

  QNetworkAccessManager* m_netMgr;
  TrackDataModel* m_trackDataModel;
  ServerImporter* m_importer;
  ISettings* m_settings;
  ConfigStore* m_configStore;
};
