/**
 * \file testmusicbrainzreleaseimportparser.cpp
 * Test parsing of import data from MusicBrainz server.
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

#include "testmusicbrainzreleaseimportparser.h"
#include <QtTest>
#include "serverimporter.h"
#include "trackdatamodel.h"

void TestMusicBrainzReleaseImportParser::initTestCase()
{
  setServerImporter(QLatin1String("MusicBrainzImport"));
}

void TestMusicBrainzReleaseImportParser::testParseAlbums()
{
  static const char searchStr[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><metadata xmlns=\"http://musicbrainz.org/ns/mmd-2.0#\" xmlns:ext=\"http://musicbrainz.org/ns/ext#-2.0\"><release-list offset=\"0\" count=\"3\"><release ext:score=\"100\" id=\"8c433fd2-9259-4c20-bfe5-58757df15b29\"><title>Odin</title><status>Official</status><text-representation><language>eng</language><script>Latn</script></text-representation><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit><release-group type=\"Album\" id=\"a7f36fa7-33f8-315e-be1f-c26cd96d9548\"><primary-type>Album</primary-type></release-group><date>2003</date><country>DE</country><barcode>693723003023</barcode><asin>B00009VGKI</asin><label-info-list><label-info><catalog-number>LMP 0303-054</catalog-number><label id=\"76beb709-a8f8-4ad5-828c-6ec8660a6935\"><name>Limb Music Products</name></label></label-info></label-info-list><medium-list count=\"1\"><track-count>13</track-count><medium><format>CD</format><disc-list count=\"0\"/><track-list count=\"13\"/></medium></medium-list></release><release ext:score=\"100\" id=\"978c7ed1-a854-4ef2-bd4e-e7c1317be854\"><title>Odin</title><status>Official</status><text-representation><language>eng</language><script>Latn</script></text-representation><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit><release-group type=\"Album\" id=\"a7f36fa7-33f8-315e-be1f-c26cd96d9548\"><primary-type>Album</primary-type></release-group><date>2003-08-19</date><country>DE</country><barcode>693723654720</barcode><asin>B00008OUEN</asin><label-info-list><label-info><catalog-number>LMP 0303-054 CD</catalog-number><label id=\"76beb709-a8f8-4ad5-828c-6ec8660a6935\"><name>Limb Music Products</name></label></label-info></label-info-list><medium-list count=\"1\"><track-count>11</track-count><medium><format>CD</format><disc-list count=\"1\"/><track-list count=\"11\"/></medium></medium-list></release><release ext:score=\"100\" id=\"7d57cc0b-70cd-4887-9399-e19e496fc8c4\"><title>Odin</title><status>Official</status><text-representation><script>Latn</script></text-representation><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit><release-group type=\"Album\" id=\"a7f36fa7-33f8-315e-be1f-c26cd96d9548\"><primary-type>Album</primary-type></release-group><medium-list count=\"1\"><track-count>12</track-count><medium><disc-list count=\"0\"/><track-list count=\"12\"/></medium></medium-list></release></release-list></metadata>";
  onFindFinished(searchStr);
  QStandardItemModel* albumModel = m_importer->getAlbumListModel();
  QCOMPARE(albumModel->rowCount(), 3);
  AlbumListItem* item = dynamic_cast<AlbumListItem*>(albumModel->item(0, 0));
  QVERIFY(item);
  QCOMPARE(item->text(), QString(QLatin1String("Wizard - Odin")));
  QCOMPARE(item->getCategory(), QString(QLatin1String("release")));
  QVERIFY(!item->getId().isEmpty());
}

void TestMusicBrainzReleaseImportParser::testParseTracks()
{
  static const char albumStr[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?><metadata xmlns=\"http://musicbrainz.org/ns/mmd-2.0#\"><release id=\"978c7ed1-a854-4ef2-bd4e-e7c1317be854\"><title>Odin</title><status>Official</status><quality>normal</quality><text-representation><language>eng</language><script>Latn</script></text-representation><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit><date>2003-08-19</date><country>DE</country><barcode>693723654720</barcode><asin>B00008OUEN</asin><label-info-list count=\"1\"><label-info><catalog-number>LMP 0303-054 CD</catalog-number><label id=\"76beb709-a8f8-4ad5-828c-6ec8660a6935\"><name>Limb Music Products</name><sort-name>Limb Music Products</sort-name><label-code>924</label-code></label></label-info></label-info-list><medium-list count=\"1\"><medium><position>1</position><track-list count=\"11\" offset=\"0\"><track><position>1</position><number>1</number><length>319173</length><recording id=\"dac7c002-432f-4dcb-ad57-5ebde8e258b0\"><title>The Prophecy</title><length>319173</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>2</position><number>2</number><length>293186</length><recording id=\"3e326f9e-7132-49d8-acff-e9eafc09a073\"><title>Betrayer</title><length>293186</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>3</position><number>3</number><length>362026</length><recording id=\"cbafa8e8-1639-4bdb-88d8-8d0db1c29fcc\"><title>Dead Hope</title><length>362026</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>4</position><number>4</number><length>342946</length><recording id=\"a3312b96-340a-45b8-ad1f-fef15343fd33\"><title>Dark God</title><length>342946</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>5</position><number>5</number><length>308746</length><recording id=\"40792d11-6087-484a-b573-b5dc4b54ebde\"><title>Loki's Punishment</title><length>308746</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>6</position><number>6</number><length>241600</length><recording id=\"3b23dfbd-4f6c-445a-836a-9882b9e10ad7\"><title>Beginning of the End</title><length>241600</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>7</position><number>7</number><length>301573</length><recording id=\"98f11cca-1a69-4f41-ac3b-726d5174b404\"><title>Thor's Hammer</title><length>301573</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>8</position><number>8</number><length>306680</length><recording id=\"e82be71a-df65-480a-9958-ee98f6bab005\"><title>Hall of Odin</title><length>306680</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>9</position><number>9</number><length>321506</length><recording id=\"149eebfa-7188-4c96-b535-7e1abe45b86b\"><title>The Powergod</title><length>321506</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>10</position><number>10</number><length>340400</length><recording id=\"4ebcddbb-ffae-41d1-b9c9-d5aea6bca9e5\"><title>March of the Einheriers</title><length>340400</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track><track><position>11</position><number>11</number><length>233720</length><recording id=\"80168326-bd79-4287-a8d6-313066257dfd\"><title>End of All</title><length>233720</length><artist-credit><name-credit><artist id=\"d1075cad-33e3-496b-91b0-d4670aabf4f8\"><name>Wizard</name><sort-name>Wizard</sort-name><disambiguation>German power metal</disambiguation></artist></name-credit></artist-credit></recording></track></track-list></medium></medium-list><relation-list target-type=\"url\"><relation type=\"amazon asin\"><target>http://www.amazon.de/gp/product/B00008OUEN</target></relation></relation-list></release></metadata>";
  onAlbumFinished(albumStr);

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
