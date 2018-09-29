/**
 * \file testdiscogsimporter.cpp
 * Test import from Discogs server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2012
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

#include "testdiscogsimporter.h"
#include <QTest>
#include "serverimporter.h"
#include "trackdatamodel.h"

void TestDiscogsImporter::initTestCase()
{
  setServerImporter(QLatin1String("DiscogsImport"));
}

void TestDiscogsImporter::testQueryAlbums()
{
  queryAlbums(QLatin1String("Wizard"), QLatin1String("Odin"));

  QStandardItemModel* albumModel = m_importer->getAlbumListModel();
  QVERIFY(albumModel->rowCount() > 0);
  auto  item = static_cast<AlbumListItem*>(albumModel->item(0, 0));
  QVERIFY(item);
  QVERIFY(item->type() == AlbumListItem::Type);
  QCOMPARE(item->text(), QString(QLatin1String("Wizard - Odin")));
  QCOMPARE(item->getCategory(), QString(QLatin1String("Wizard-Odin/release")));
  QVERIFY(!item->getId().isEmpty());
}

void TestDiscogsImporter::testQueryTracks()
{
  queryTracks(QLatin1String("Wizard-Odin/release"), QLatin1String("2487778"));

  QStringList titles;
  titles << QLatin1String("The Prophecy") << QLatin1String("Betrayer") << QLatin1String("Dead Hope") << QLatin1String("Dark God")
         << QLatin1String("Lokis Punishment") << QLatin1String("Beginning Of The End") << QLatin1String("Thor's Hammer")
         << QLatin1String("Hall Of Odin") << QLatin1String("The Powergod") << QLatin1String("March Of The Einherjers")
         << QLatin1String("End Of All") << QLatin1String("Ultimate War (Bonus Track)")
         << QLatin1String("Golden Dawn (Bonus Track)") << QLatin1String("Betrayer");
  QStringList lengths;
  lengths << QLatin1String("5:19") << QLatin1String("4:53") << QLatin1String("6:02") << QLatin1String("5:43") << QLatin1String("5:08") << QLatin1String("4:01") << QLatin1String("5:01")
          << QLatin1String("5:06") << QLatin1String("5:21") << QLatin1String("5:40") << QLatin1String("3:53") << QLatin1String("4:52") << QLatin1String("5:05") << QLatin1String("");
  QCOMPARE(m_trackDataModel->rowCount(), 14);
  for (int row = 0; row < 11; ++row) {
    QCOMPARE(m_trackDataModel->index(row, 0).data().toString(), lengths.at(row));
    QCOMPARE(m_trackDataModel->index(row, 3).data().toInt(), row + 1);
    QCOMPARE(m_trackDataModel->index(row, 4).data().toString(), titles.at(row));
    QCOMPARE(m_trackDataModel->index(row, 5).data().toString(), QString(QLatin1String("Wizard")));
    QCOMPARE(m_trackDataModel->index(row, 6).data().toString(), QString(QLatin1String("Odin")));
    QCOMPARE(m_trackDataModel->index(row, 7).data().toInt(), 2003);
    QCOMPARE(m_trackDataModel->index(row, 8).data().toString(), QString(QLatin1String("Heavy Metal")));
    QCOMPARE(m_trackDataModel->index(row, 10).data().toString(), QString(QLatin1String("LMP 0303-054 Ltd. CD")));
    QCOMPARE(m_trackDataModel->index(row, 11).data().toString(), QString(QLatin1String("CD, Album, Limited Edition, Enhanced, Digipak")));
    QCOMPARE(m_trackDataModel->index(row, 12).data().toString(), QString(QLatin1String("LMP")));
    QCOMPARE(m_trackDataModel->index(row, 13).data().toString(), QString(QLatin1String("Germany")));
  }
}
