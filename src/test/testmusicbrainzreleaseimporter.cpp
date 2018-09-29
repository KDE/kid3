/**
 * \file testmusicbrainzreleaseimporter.cpp
 * Test import from MusicBrainz server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 07 Oct 2012
 *
 * Copyright (C) 2012-2013  Urs Fleisch
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
#include <QTest>
#include "serverimporter.h"
#include "trackdatamodel.h"

void TestMusicBrainzReleaseImporter::initTestCase()
{
  setServerImporter(QLatin1String("MusicBrainzImport"));
}

void TestMusicBrainzReleaseImporter::testQueryAlbums()
{
  queryAlbums(QLatin1String("Wizard"), QLatin1String("Odin"));
  QStandardItemModel* albumModel = m_importer->getAlbumListModel();
  QCOMPARE(albumModel->rowCount(), 3);
  auto  item = static_cast<AlbumListItem*>(albumModel->item(0, 0));
  QVERIFY(item);
  QVERIFY(item->type() == AlbumListItem::Type);
  QCOMPARE(item->text(), QString(QLatin1String("Wizard - Odin")));
  QCOMPARE(item->getCategory(), QString(QLatin1String("release")));
  QVERIFY(!item->getId().isEmpty());
}

void TestMusicBrainzReleaseImporter::testQueryTracks()
{
  queryTracks(QLatin1String("release"), QLatin1String("978c7ed1-a854-4ef2-bd4e-e7c1317be854"));

  QStringList titles;
  titles << QLatin1String("The Prophecy") << QLatin1String("Betrayer") << QLatin1String("Dead Hope") << QLatin1String("Dark God")
         << QLatin1String("Loki's Punishment") << QLatin1String("Beginning of the End") << QLatin1String("Thor's Hammer")
         << QLatin1String("Hall of Odin") << QLatin1String("The Powergod") << QLatin1String("March of the Einheriers")
         << QLatin1String("End of All");
  QStringList lengths;
  lengths << QLatin1String("5:19") << QLatin1String("4:53") << QLatin1String("6:02") << QLatin1String("5:42") << QLatin1String("5:08") << QLatin1String("4:01") << QLatin1String("5:01")
          << QLatin1String("5:06") << QLatin1String("5:21") << QLatin1String("5:40") << QLatin1String("3:53");
  QCOMPARE(m_trackDataModel->rowCount(), 11);
  for (int row = 0; row < 11; ++row) {
    QCOMPARE(m_trackDataModel->index(row, 0).data().toString(), lengths.at(row));
    QCOMPARE(m_trackDataModel->index(row, 3).data().toInt(), row + 1);
    QCOMPARE(m_trackDataModel->index(row, 4).data().toString(), titles.at(row));
    QCOMPARE(m_trackDataModel->index(row, 5).data().toString(), QString(QLatin1String("Wizard")));
    QCOMPARE(m_trackDataModel->index(row, 6).data().toString(), QString(QLatin1String("Odin")));
    QCOMPARE(m_trackDataModel->index(row, 7).data().toInt(), 2003);
    QCOMPARE(m_trackDataModel->index(row, 10).data().toString(), QString(QLatin1String("Wizard")));
    QCOMPARE(m_trackDataModel->index(row, 11).data().toString(), QString(QLatin1String("LMP 0303-054 CD")));
    QCOMPARE(m_trackDataModel->index(row, 12).data().toString(), QString(QLatin1String("Limb Music Products")));
    QCOMPARE(m_trackDataModel->index(row, 13).data().toString(), QString(QLatin1String("DE")));
  }
}
