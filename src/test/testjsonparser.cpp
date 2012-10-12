/**
 * \file testjsonparser.cpp
 * Test JSON serializer and deserializer.
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

#include "testjsonparser.h"
#include "jsonparser.h"

namespace {

QString numberedName(int nr)
{
  return QString("val%1").arg(nr, 2, 10, QChar('0'));
}

}

void TestJsonParser::serializeAndDeserializeMessage()
{
  QMap<QString, QVariant> map;

  map["uri"] = "http://www.youtube.com/watch?v=QVdDhOnoR8k";
  map["duration"] = 334;
  map["embed"] = true;
  map["format"] = "12\"";

  QString str = JsonParser::serialize(map);
  QCOMPARE(str, QString("{\"duration\": 334, \"embed\": true, \"format\": \"12\\\"\", \"uri\": \"http://www.youtube.com/watch?v=QVdDhOnoR8k\"}"));

  bool ok;
  QVariant json = JsonParser::deserialize(str, &ok);
  QMap<QString, QVariant> deser = json.toMap();
  QVERIFY(ok);
  QVariant val = deser.value("uri");
  QCOMPARE(val.type(), QVariant::String);
  QCOMPARE(val.toString(), QString("http://www.youtube.com/watch?v=QVdDhOnoR8k"));
  val = deser.value("duration");
  QCOMPARE(val.type(), QVariant::Int);
  QCOMPARE(val.toInt(), 334);
  val = deser.value("embed");
  QCOMPARE(val.type(), QVariant::Bool);
  QCOMPARE(val.toBool(), true);
  val = deser.value("format");
  QCOMPARE(val.type(), QVariant::String);
  QCOMPARE(val.toString(), QString("12\""));

  QCOMPARE(JsonParser::serialize(deser), str);
}

void TestJsonParser::serializeAndDeserializeTypes()
{
  QMap<QString, QVariant> map;

  QDateTime valDateTime(QDate(2011, 8, 18), QTime(14, 2, 7));
  int valInt(-7654321);
  qlonglong valLongLong(1234567890123456789LL);
  bool valBool1(false);
  bool valBool2(true);
  QVariant valNull;
  QString valString("String with \"'\\{}[]");
  double valDouble(3.141592653);
  QVariantMap valObject;
  valObject["nested"] = true;
  valObject["obj"] = 1e23;
  QVariantList valArray;
  QVariantMap valArrayElement;
  valArrayElement["val"] = 2;
  valArray << 1 << valArrayElement << 3;
  bool ok;

  QVariantList values;
  values << valDateTime << valInt << valLongLong << valBool1 << valBool2
         << valNull << valString << valDouble
         << valObject << QVariant(valArray);
  for (int i = 0; i < values.length(); ++i) {
    map.insert(numberedName(i), values.at(i));
  }

  QString str = JsonParser::serialize(map);
  QCOMPARE(str, QString(
             "{\"val00\": \"2011-08-18T14:02:07\", \"val01\": -7654321, "
             "\"val02\": 1234567890123456789, "
             "\"val03\": false, \"val04\": true, \"val05\": null, \"val06\": "
             "\"String with \\\"'\\\\{}[]\", \"val07\": 3.141592653, "
             "\"val08\": {\"nested\": true, "
             "\"obj\": 1e+23}, \"val09\": [1, {\"val\": 2}, 3]}"
             ));

  QMap<QString, QVariant> deser = JsonParser::deserialize(str, &ok).toMap();
  QVERIFY(ok);
  for (int i = 0; i < values.length(); ++i) {
    QVariant val = deser.value(numberedName(i));
    QCOMPARE(val.type(), values.at(i).type());
    QCOMPARE(val, values.at(i));
  }

  QCOMPARE(JsonParser::serialize(deser), str);
}

void TestJsonParser::deserializeMusicData()
{
  QString musicStr(
    "{\n"
    "  \"id\": 1,\n"
    "  \"title\": \"Stockholm\",\n"
    "  \"styles\": [ \"Deep House\" ],\n"
    "  \"labels\": [\n"
    "    { \"entity_type\": \"1\", \"name\": \"Svek\" },\n"
    "    { \"name\": \"LMP\" }\n"
    "  ],\n"
    "  \"results\": [\n"
    "    {\"style\": [\"Heavy Metal\"]},\n"
    "    {\"style\": [\"Hard Rock}]\"]}\n"
    "  ]\n"
    "}\n");

  bool ok = true;
  QVariantMap musicMap = JsonParser::deserialize(musicStr, &ok).toMap();
  QVERIFY(ok);
  QCOMPARE(musicMap.value("id"), QVariant(1));
  QCOMPARE(musicMap.value("title"), QVariant("Stockholm"));

  QVariantList styles;
  styles << "Deep House";
  QCOMPARE(musicMap.value("styles"), QVariant(styles));

  QVariantMap label1, label2;
  label1["entity_type"] = "1";
  label1["name"] = "Svek";
  label2["name"] = "LMP";
  QVariantList labels;
  labels << label1 << label2;
  QCOMPARE(musicMap.value("labels"), QVariant(labels));

  QVariantMap result1, result2;
  QVariantList resultStyle1, resultStyle2;
  resultStyle1 << "Heavy Metal";
  resultStyle2 << "Hard Rock}]";
  result1["style"] = resultStyle1;
  result2["style"] = resultStyle2;
  QVariantList results;
  results << result1 << result2;
  QCOMPARE(musicMap.value("results"), QVariant(results));
}
