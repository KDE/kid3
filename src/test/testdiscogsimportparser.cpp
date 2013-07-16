/**
 * \file testdiscogsimportparser.cpp
 * Test parsing of import data from Discogs server.
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

#include "testdiscogsimportparser.h"
#include <QtTest>
#include "serverimporter.h"
#include "trackdatamodel.h"

void TestDiscogsImportParser::initTestCase()
{
  setServerImporter(QLatin1String("DiscogsImport"));
}

void TestDiscogsImportParser::testParseAlbums()
{
  static const char searchStr[] =
    "{\"pagination\": {\"per_page\": 50, \"pages\": 1, \"page\": 1, \"urls\": {}, \"items\": 10}, \"results\": [{\"style\": [\"Heavy Metal\"], \"thumb\": \"http://api.discogs.com/image/R-90-2487778-1293847958.jpeg\", \"format\": [\"CD\", \"Album\", \"Limited Edition\", \"Enhanced\"], \"country\": \"Germany\", \"title\": \"Wizard (23) - Odin\", \"uri\": \"/Wizard-Odin/release/2487778\", \"label\": [\"LMP\"], \"catno\": \"LMP 0303-054 Ltd. CD\", \"year\": \"2003\", \"genre\": [\"Rock\"], \"resource_url\": \"http://api.discogs.com/releases/2487778\", \"type\": \"release\", \"id\": 2487778}, {\"style\": [\"Heavy Metal\"], \"thumb\": \"http://api.discogs.com/image/R-90-2487932-1293847561.jpeg\", \"format\": [\"CD\", \"Album\", \"Enhanced\"], \"country\": \"Japan\", \"title\": \"Wizard (23) - Odin\", \"uri\": \"/Wizard-Odin/release/2487932\", \"label\": [\"Soundholic\"], \"catno\": \"TKCS-85065\", \"year\": \"2003\", \"genre\": [\"Rock\"], \"resource_url\": \"http://api.discogs.com/releases/2487932\", \"type\": \"release\", \"id\": 2487932}, {\"style\": [\"Heavy Metal\"], \"thumb\": \"http://api.discogs.com/image/R-90-3312195-1339346316-2529.jpeg\", \"format\": [\"CD\", \"Album\", \"Promo\"], \"country\": \"Germany\", \"barcode\": [\"6 93723 65472 0\", \"GEMA\", \"LC 00924\", \"-LMP0303-054CD - Invictus - Black Heart 001225384 Limb Music Products\"], \"uri\": \"/Wizard-Odin/release/3312195\", \"label\": [\"LMP\", \"SPV GmbH\"], \"catno\": \"LMP 0303-054 CD\", \"year\": \"2003\", \"genre\": [\"Rock\"], \"title\": \"Wizard (23) - Odin\", \"resource_url\": \"http://api.discogs.com/releases/3312195\", \"type\": \"release\", \"id\": 3312195}, {\"style\": [\"Speed Metal\"], \"thumb\": \"http://s.discogss.com/images/record90.png\", \"format\": [\"Vinyl\", \"LP\", \"Picture Disc\", \"Limited Edition\"], \"country\": \"Germany\", \"barcode\": [\"6 93723 65471 3\", \"GEMA\"], \"uri\": \"/Wizard-Odin/release/3675993\", \"label\": [\"LMP\", \"SPV GmbH\"], \"catno\": \"LMP 0303-054 Ltd. PIC LP\", \"year\": \"2003\", \"genre\": [\"Rock\"], \"title\": \"Wizard (23) - Odin\", \"resource_url\": \"http://api.discogs.com/releases/3675993\", \"type\": \"release\", \"id\": 3675993}, {\"style\": [\"Heavy Metal\"], \"thumb\": \"http://api.discogs.com/image/R-90-2487767-1293847243.jpeg\", \"format\": [\"CD\", \"Album\", \"Enhanced\"], \"country\": \"Germany\", \"barcode\": [\"4 028466 116178\", \"GEMA\", \"LC 06398\"], \"uri\": \"/Wizard-Thor/release/2487767\", \"label\": [\"Massacre Records\", \"Massacre Records\", \"Massacre Records\", \"Soulfood\", \"The Red Room\", \"Magic Hall Studios\"], \"catno\": \"MAS DP0617\", \"year\": \"2009\", \"genre\": [\"Rock\"], \"title\": \"Wizard (23) - Thor\", \"resource_url\": \"http://api.discogs.com/releases/2487767\", \"type\": \"release\", \"id\": 2487767}, {\"style\": [\"Black Metal\"], \"thumb\": \"http://api.discogs.com/image/R-90-3573177-1335800447.jpeg\", \"format\": [\"Cassette\", \"Single Sided\"], \"country\": \"Germany\", \"title\": \"Desaster - Lost In The Ages\", \"uri\": \"/Desaster-Lost-In-The-Ages/release/3573177\", \"label\": [\"Not On Label (Desaster Self-released)\"], \"catno\": \"none\", \"year\": \"1994\", \"genre\": [\"Rock\"], \"resource_url\": \"http://api.discogs.com/releases/3573177\", \"type\": \"release\", \"id\": 3573177}, {\"style\": [\"New Wave\", \"Goth Rock\"], \"thumb\": \"http://api.discogs.com/image/R-90-1764235-1244486470.jpeg\", \"format\": [\"CD\", \"Album\"], \"country\": \"France\", \"barcode\": [\"3770001009247\"], \"uri\": \"/Babel-17-The-Ice-Wall/release/1764235\", \"label\": [\"Infrastition\"], \"catno\": \"Ino 001\", \"year\": \"2009\", \"genre\": [\"Electronic\", \"Rock\"], \"title\": \"Babel 17 - The Ice Wall\", \"resource_url\": \"http://api.discogs.com/releases/1764235\", \"type\": \"release\", \"id\": 1764235}, {\"style\": [\"Thrash\", \"Black Metal\"], \"thumb\": \"http://api.discogs.com/image/R-90-2894728-1306089742.jpeg\", \"format\": [\"Vinyl\", \"12\\\"\", \"Picture Disc\", \"Compilation\", \"Limited Edition\", \"Vinyl\", \"12\\\"\", \"Single Sided\", \"Etched\", \"Limited Edition\"], \"country\": \"Germany\", \"title\": \"Desaster - 20 Years Of Total Desaster\", \"uri\": \"/Desaster-20-Years-Of-Total-Desaster/release/2894728\", \"label\": [\"Kneel Before The Master's Throne Records\"], \"catno\": \"KNEEL 026\", \"year\": \"2009\", \"genre\": [\"Rock\"], \"resource_url\": \"http://api.discogs.com/releases/2894728\", \"type\": \"release\", \"id\": 2894728}, {\"style\": [\"Abstract\", \"Ambient\"], \"thumb\": \"http://api.discogs.com/image/R-90-1399903-1216441299.jpeg\", \"format\": [\"CD\", \"Album\"], \"country\": \"US\", \"title\": \"Mike Kelley - Day Is Done / Original Motion Picture Soundtrack\", \"uri\": \"/Mike-Kelley-Day-Is-Done-Original-Motion-Picture-Soundtrack/release/1399903\", \"label\": [\"Compound Annex Records\"], \"catno\": \"Compound#14\", \"year\": \"2005\", \"genre\": [\"Non-Music\", \"Stage & Screen\"], \"resource_url\": \"http://api.discogs.com/releases/1399903\", \"type\": \"release\", \"id\": 1399903}, {\"style\": [\"Black Metal\", \"Viking Metal\", \"Psychedelic Rock\", \"Prog Rock\"], \"thumb\": \"http://api.discogs.com/image/R-90-3045032-1313173110.jpeg\", \"format\": [\"CDr\", \"CD-ROM\", \"Compilation\", \"Unofficial Release\"], \"country\": \"Russia\", \"barcode\": [\"(03318103\", \"MB-UG-80GPM1\"], \"uri\": \"/Enslaved-Einherjer-Mp3-Collection/release/3045032\", \"label\": [\"MP3SERVICE\"], \"catno\": \"none\", \"year\": \"2004\", \"genre\": [\"Rock\"], \"title\": \"Enslaved & Einherjer - Mp3 Collection\", \"resource_url\": \"http://api.discogs.com/releases/3045032\", \"type\": \"release\", \"id\": 3045032}]}";
  onFindFinished(searchStr);

  QStandardItemModel* albumModel = m_importer->getAlbumListModel();
  QVERIFY(albumModel->rowCount() > 0);
  AlbumListItem* item = static_cast<AlbumListItem*>(albumModel->item(0, 0));
  QVERIFY(item);
  QVERIFY(item->type() == AlbumListItem::Type);
  QCOMPARE(item->text(), QString(QLatin1String("Wizard - Odin")));
  QCOMPARE(item->getCategory(), QString(QLatin1String("releases")));
  QVERIFY(!item->getId().isEmpty());
}

void TestDiscogsImportParser::testParseTracks()
{
  static const char albumStr[] =
    "{\"styles\": [\"Heavy Metal\"], \"series\": [], \"labels\": [{\"id\": 34707, \"resource_url\": \"http://api.discogs.com/labels/34707\", \"catno\": \"LMP 0303-054 Ltd. CD\", \"name\": \"LMP\", \"entity_type\": \"\"}], \"year\": 2003, \"artists\": [{\"join\": \"\", \"name\": \"Wizard (23)\", \"anv\": \"\", \"tracks\": \"\", \"role\": \"\", \"resource_url\": \"http://api.discogs.com/artists/1746637\", \"id\": 1746637}], \"images\": [{\"uri\": \"http://api.discogs.com/image/R-2487778-1293847958.jpeg\", \"height\": 522, \"width\": 600, \"resource_url\": \"http://api.discogs.com/image/R-2487778-1293847958.jpeg\", \"type\": \"primary\", \"uri150\": \"http://api.discogs.com/image/R-150-2487778-1293847958.jpeg\"}, {\"uri\": \"http://api.discogs.com/image/R-2487778-1293847967.jpeg\", \"height\": 526, \"width\": 600, \"resource_url\": \"http://api.discogs.com/image/R-2487778-1293847967.jpeg\", \"type\": \"secondary\", \"uri150\": \"http://api.discogs.com/image/R-150-2487778-1293847967.jpeg\"}], \"id\": 2487778, \"genres\": [\"Rock\"], \"thumb\": \"http://api.discogs.com/image/R-150-2487778-1293847958.jpeg\", \"extraartists\": [], \"title\": \"Odin\", \"master_id\": 280805, \"tracklist\": [{\"duration\": \"5:19\", \"position\": \"1\", \"title\": \"The Prophecy\"}, {\"duration\": \"4:53\", \"position\": \"2\", \"title\": \"Betrayer\"}, {\"duration\": \"6:02\", \"position\": \"3\", \"title\": \"Dead Hope\"}, {\"duration\": \"5:43\", \"position\": \"4\", \"title\": \"Dark God\"}, {\"duration\": \"5:08\", \"position\": \"5\", \"title\": \"Lokis Punishment\"}, {\"duration\": \"4:01\", \"position\": \"6\", \"title\": \"Beginning Of The End\"}, {\"duration\": \"5:01\", \"position\": \"7\", \"title\": \"Thor's Hammer\"}, {\"duration\": \"5:06\", \"position\": \"8\", \"title\": \"Hall Of Odin\"}, {\"duration\": \"5:21\", \"position\": \"9\", \"title\": \"The Powergod\"}, {\"duration\": \"5:40\", \"position\": \"10\", \"title\": \"March Of The Einherjers\"}, {\"duration\": \"3:53\", \"position\": \"11\", \"title\": \"End Of All\"}, {\"duration\": \"4:52\", \"position\": \"12\", \"title\": \"Ultimate War (Bonus Track)\"}, {\"duration\": \"5:05\", \"position\": \"13\", \"title\": \"Golden Dawn (Bonus Track)\"}, {\"duration\": \"\", \"position\": \"Video\", \"title\": \"Betrayer\"}], \"status\": \"Accepted\", \"released_formatted\": \"2003\", \"master_url\": \"http://api.discogs.com/masters/280805\", \"released\": \"2003\", \"country\": \"Germany\", \"notes\": \"Only 4000 copies worldwide!\", \"companies\": [], \"uri\": \"http://www.discogs.com/Wizard-Odin/release/2487778\", \"formats\": [{\"descriptions\": [\"Album\", \"Limited Edition\", \"Enhanced\"], \"text\": \"Digipak\", \"name\": \"CD\", \"qty\": \"1\"}], \"resource_url\": \"http://api.discogs.com/releases/2487778\", \"data_quality\": \"Correct\"}";
  onAlbumFinished(albumStr);

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
    QCOMPARE(m_trackDataModel->index(row, 11).data().toString(), QString(QLatin1String("CD")));
    QCOMPARE(m_trackDataModel->index(row, 12).data().toString(), QString(QLatin1String("LMP")));
    QCOMPARE(m_trackDataModel->index(row, 13).data().toString(), QString(QLatin1String("Germany")));
  }
}
