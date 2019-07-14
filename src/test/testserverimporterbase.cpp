/**
 * \file testserverimporterbase.cpp
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

#include "testserverimporterbase.h"
#include <QTest>
#include <QNetworkAccessManager>
#include <QTimer>
#include "dummysettings.h"
#include "kid3application.h"
#include "iserverimporterfactory.h"
#include "serverimporter.h"
#include "trackdatamodel.h"
#include "configstore.h"

TestServerImporterBase::TestServerImporterBase(QObject* parent)
  : QObject(parent),
    m_netMgr(new QNetworkAccessManager(this)),
    m_trackDataModel(new TrackDataModel(nullptr, this)),
    m_importer(nullptr), m_settings(nullptr), m_configStore(nullptr)
{
  if (!ConfigStore::instance()) {
    m_settings = new DummySettings;
    m_configStore = new ConfigStore(m_settings);
  }
}

TestServerImporterBase::~TestServerImporterBase()
{
  delete m_configStore;
  delete m_settings;
}

void TestServerImporterBase::onFindFinished(const QByteArray& searchStr)
{
  if (m_importer)
    m_importer->parseFindResults(searchStr);
  emit albumsUpdated();
}

void TestServerImporterBase::onAlbumFinished(const QByteArray& albumStr)
{
  if (m_importer) {
    m_importer->setStandardTags(true);
    m_importer->setAdditionalTags(true);
    m_importer->parseAlbumResults(albumStr);
  }
  emit trackDataUpdated();
}

void TestServerImporterBase::setServerImporter(ServerImporter* importer)
{
  if (importer != m_importer) {
    m_importer = importer;
    if (m_importer) {
      connect(m_importer, &ImportClient::findFinished,
              this, &TestServerImporterBase::onFindFinished);
      connect(m_importer, &ImportClient::albumFinished,
              this, &TestServerImporterBase::onAlbumFinished);
    }
  }
}

void TestServerImporterBase::setServerImporter(const QString& key)
{
  ServerImporter* serverImporter = nullptr;
  QObjectList plugins = Kid3Application::loadPlugins();
  foreach (QObject* plugin, plugins) {
    if (IServerImporterFactory* importerFactory =
        qobject_cast<IServerImporterFactory*>(plugin)) {
      if (importerFactory->serverImporterKeys().contains(key)) {
        serverImporter = importerFactory->createServerImporter(
              key, m_netMgr, m_trackDataModel);
        break;
      }
    }
  }
  QVERIFY(serverImporter != nullptr);
  setServerImporter(serverImporter);
}

void TestServerImporterBase::queryAlbums(const QString& artist, const QString& album)
{
  QEventLoop eventLoop;
  QTimer timer;
  timer.setSingleShot(true);
  connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
  connect(this, &TestServerImporterBase::albumsUpdated,
          &eventLoop, &QEventLoop::quit);
  m_importer->find(m_importer->config(), artist, album);
  timer.start(5000);
  eventLoop.exec();
  QVERIFY(timer.isActive());
}

void TestServerImporterBase::queryTracks(const QString& cat, const QString& id)
{
  QEventLoop eventLoop;
  QTimer timer;
  timer.setSingleShot(true);
  connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
  connect(this, &TestServerImporterBase::trackDataUpdated,
          &eventLoop, &QEventLoop::quit);
  m_importer->getTrackList(m_importer->config(), cat, id);
  timer.start(5000);
  eventLoop.exec();
  QVERIFY(timer.isActive());
}
