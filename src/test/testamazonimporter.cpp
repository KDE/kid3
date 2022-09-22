/**
 * \file testamazonimporter.cpp
 * Test import from Amazon server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11 Feb 2021
 *
 * Copyright (C) 2021  Urs Fleisch
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

#include "testamazonimporter.h"
#include <QTest>
#include "serverimporter.h"
#include "trackdatamodel.h"

void TestAmazonImporter::initTestCase()
{
  setServerImporter(QLatin1String("AmazonImport"));
}

void TestAmazonImporter::testQueryAlbums()
{
  queryAlbums(QLatin1String("Wizard"), QLatin1String("Odin"));

  AlbumListModel* albumModel = m_importer->getAlbumListModel();
  QVERIFY(albumModel->rowCount() > 0);
  QString text, category, id;
  for (int row = 0; row < albumModel->rowCount(); ++row) {
    albumModel->getItem(row, text, category, id);
    if (text.contains(QLatin1String("Wizard")) &&
        text.contains(QLatin1String("Odin"))) {
      break;
    }
  }
  QCOMPARE(text, QString(QLatin1String("Wizard - Odin")));
  QCOMPARE(category, QString(QLatin1String("dp")));
  QVERIFY(!id.isEmpty());
}

void TestAmazonImporter::testQueryTracks()
{
  queryTracks(QLatin1String("dp"), QLatin1String("B00U1GQ4D0"));

  QStringList titles;
  titles << QLatin1String("The Prophecy")
         << QLatin1String("Betrayer")
         << QLatin1String("Dead Hope")
         << QLatin1String("Dark God")
         << QLatin1String("Loki S Punishment")
         << QLatin1String("Beginning of the End")
         << QLatin1String("Thor S Hammer")
         << QLatin1String("Hall of Odin")
         << QLatin1String("The Powergod")
         << QLatin1String("March of the Einheriers")
         << QLatin1String("End of All")
         << QLatin1String("Ultimate War (Bonus Track)")
         << QLatin1String("Golden Dawn (Bonus Track)")
         << QLatin1String("Betrayer (video bonus track)");
  QCOMPARE(m_trackDataModel->rowCount(), 14);
  for (int row = 0; row < 13; ++row) {
    QCOMPARE(m_trackDataModel->index(row, 0).data().toString(), "");
    QCOMPARE(m_trackDataModel->index(row, 3).data().toInt(), row + 1);
    QCOMPARE(m_trackDataModel->index(row, 4).data().toString(), titles.at(row));
    QCOMPARE(m_trackDataModel->index(row, 5).data().toString(),
             QString(QLatin1String("Wizard")));
    QCOMPARE(m_trackDataModel->index(row, 6).data().toString(),
             QString(QLatin1String("Odin")));
    QCOMPARE(m_trackDataModel->index(row, 7).data().toInt(), 2015);
    QCOMPARE(m_trackDataModel->index(row, 8).data().toString(),
             QString(QLatin1String("")));
    QCOMPARE(m_trackDataModel->index(row, 9).data().toString(),
             QString(QLatin1String("")));
  }
}
