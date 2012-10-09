/**
 * \file testserverimporterbase.cpp
 * Base class for server importer tests.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Oct 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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
#include <QtTest>
#include <QNetworkAccessManager>
#include "serverimporter.h"
#include "trackdatamodel.h"

TestServerImporterBase::TestServerImporterBase(QObject* parent) :
  QObject(parent),
  m_netMgr(new QNetworkAccessManager(this)),
  m_trackDataModel(new TrackDataModel(this)),
  m_importer(0)
{
}

TestServerImporterBase::~TestServerImporterBase()
{
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
      connect(m_importer, SIGNAL(findFinished(QByteArray)),
              this, SLOT(onFindFinished(QByteArray)));
      connect(m_importer, SIGNAL(albumFinished(QByteArray)),
              this, SLOT(onAlbumFinished(QByteArray)));
    }
  }
}

void TestServerImporterBase::queryAlbums(const QString& artist, const QString& album)
{
  QEventLoop eventLoop;
  QTimer timer;
  timer.setSingleShot(true);
  connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
  connect(this, SIGNAL(albumsUpdated()),
          &eventLoop, SLOT(quit()));
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
  connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
  connect(this, SIGNAL(trackDataUpdated()),
          &eventLoop, SLOT(quit()));
  m_importer->getTrackList(m_importer->config(), cat, id);
  timer.start(5000);
  eventLoop.exec();
  QVERIFY(timer.isActive());
}
