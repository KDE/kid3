/**
 * \file testmusicbrainzreleaseimporter.cpp
 * Test import from MusicBrainz server.
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

#include "testmusicbrainzreleaseimporter.h"
#include <QtTest>
#include "musicbrainzreleaseimporter.h"
#include "trackdatamodel.h"

void TestMusicBrainzReleaseImporter::initTestCase()
{
  setServerImporter(new MusicBrainzReleaseImporter(m_netMgr, m_trackDataModel));
}

void TestMusicBrainzReleaseImporter::testQueryAlbums()
{
  queryAlbums("Wizard", "Odin");
  QStandardItemModel* albumModel = m_importer->getAlbumListModel();
  QCOMPARE(albumModel->rowCount(), 3);
  AlbumListItem* item = dynamic_cast<AlbumListItem*>(albumModel->item(0, 0));
  QVERIFY(item);
  QCOMPARE(item->text(), QString("Wizard - Odin"));
  QCOMPARE(item->getCategory(), QString("release"));
  QVERIFY(!item->getId().isEmpty());
}

void TestMusicBrainzReleaseImporter::testQueryTracks()
{
  queryTracks("release", "978c7ed1-a854-4ef2-bd4e-e7c1317be854");

  QStringList titles;
  titles << "The Prophecy" << "Betrayer" << "Dead Hope" << "Dark God"
         << "Loki's Punishment" << "Beginning of the End" << "Thor's Hammer"
         << "Hall of Odin" << "The Powergod" << "March of the Einheriers"
         << "End of All";
  QStringList lengths;
  lengths << "5:19" << "4:53" << "6:02" << "5:42" << "5:08" << "4:01" << "5:01"
          << "5:06" << "5:21" << "5:40" << "3:53";
  QCOMPARE(m_trackDataModel->rowCount(), 11);
  for (int row = 0; row < 11; ++row) {
    QCOMPARE(m_trackDataModel->index(row, 0).data().toString(), lengths.at(row));
    QCOMPARE(m_trackDataModel->index(row, 3).data().toInt(), row + 1);
    QCOMPARE(m_trackDataModel->index(row, 4).data().toString(), titles.at(row));
    QCOMPARE(m_trackDataModel->index(row, 5).data().toString(), QString("Wizard"));
    QCOMPARE(m_trackDataModel->index(row, 6).data().toString(), QString("Odin"));
    QCOMPARE(m_trackDataModel->index(row, 7).data().toInt(), 2003);
    QCOMPARE(m_trackDataModel->index(row, 10).data().toString(), QString("Wizard"));
    QCOMPARE(m_trackDataModel->index(row, 11).data().toString(), QString("LMP 0303-054 CD"));
    QCOMPARE(m_trackDataModel->index(row, 12).data().toString(), QString("Limb Music Products"));
    QCOMPARE(m_trackDataModel->index(row, 13).data().toString(), QString("DE"));
  }
}
