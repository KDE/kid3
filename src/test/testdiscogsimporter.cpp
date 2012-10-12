/**
 * \file testdiscogsimporter.cpp
 * Test import from Discogs server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2012
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

#include "testdiscogsimporter.h"
#include <QtTest>
#include "discogsimporter.h"
#include "trackdatamodel.h"

void TestDiscogsImporter::initTestCase()
{
  setServerImporter(new DiscogsImporter(m_netMgr, m_trackDataModel));
}

void TestDiscogsImporter::testQueryAlbums()
{
  queryAlbums("Wizard", "Odin");

  QStandardItemModel* albumModel = m_importer->getAlbumListModel();
  QVERIFY(albumModel->rowCount() > 0);
  AlbumListItem* item = dynamic_cast<AlbumListItem*>(albumModel->item(0, 0));
  QVERIFY(item);
  QCOMPARE(item->text(), QString("Wizard - Odin"));
  QCOMPARE(item->getCategory(), QString("releases"));
  QVERIFY(!item->getId().isEmpty());
}

void TestDiscogsImporter::testQueryTracks()
{
  queryTracks("releases", "2487778");

  QStringList titles;
  titles << "The Prophecy" << "Betrayer" << "Dead Hope" << "Dark God"
         << "Lokis Punishment" << "Beginning Of The End" << "Thor's Hammer"
         << "Hall Of Odin" << "The Powergod" << "March Of The Einherjers"
         << "End Of All" << "Ultimate War (Bonus Track)"
         << "Golden Dawn (Bonus Track)" << "Betrayer";
  QStringList lengths;
  lengths << "5:19" << "4:53" << "6:02" << "5:43" << "5:08" << "4:01" << "5:01"
          << "5:06" << "5:21" << "5:40" << "3:53" << "4:52" << "5:05" << "";
  QCOMPARE(m_trackDataModel->rowCount(), 14);
  for (int row = 0; row < 11; ++row) {
    QCOMPARE(m_trackDataModel->index(row, 0).data().toString(), lengths.at(row));
    QCOMPARE(m_trackDataModel->index(row, 3).data().toInt(), row + 1);
    QCOMPARE(m_trackDataModel->index(row, 4).data().toString(), titles.at(row));
    QCOMPARE(m_trackDataModel->index(row, 5).data().toString(), QString("Wizard"));
    QCOMPARE(m_trackDataModel->index(row, 6).data().toString(), QString("Odin"));
    QCOMPARE(m_trackDataModel->index(row, 7).data().toInt(), 2003);
    QCOMPARE(m_trackDataModel->index(row, 8).data().toString(), QString("Heavy Metal"));
    QCOMPARE(m_trackDataModel->index(row, 10).data().toString(), QString("CD"));
    QCOMPARE(m_trackDataModel->index(row, 11).data().toString(), QString("LMP"));
  }
}
