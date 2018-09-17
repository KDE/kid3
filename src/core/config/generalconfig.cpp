/**
 * \file generalconfig.cpp
 * General configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "generalconfig.h"
#include <QStringList>

namespace {

/** Index of latin-1 entry in getTextCodecNames(). */
enum { TextEncodingLatin1Index = 13 };

}

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
GeneralConfig::GeneralConfig(const QString& grp) : m_group(grp) {}

/**
 * Destructor.
 */
GeneralConfig::~GeneralConfig() {}

/**
 * Convert list of integers to list of strings.
 * @param intList list of integers
 * @return list of strings.
 */
QStringList GeneralConfig::intListToStringList(const QList<int>& intList)
{
  QStringList result;
  for (int value : intList) {
    result.append(QString::number(value));
  }
  return result;
}

/**
 * Convert list of strings to list of integers.
 * @param strList list of strings
 * @return list of integers.
 */
QList<int> GeneralConfig::stringListToIntList(const QStringList& strList)
{
  QList<int> result;
  for (const QString& value : strList) {
    result.append(value.toInt());
  }
  return result;
}

/**
 * String list of available text codecs.
 *
 * @return list of codec names.
 */
QStringList GeneralConfig::getTextCodecNames()
{
  static QStringList textEncodingList;
  if (textEncodingList.isEmpty()) {
    static const char* const codecs[] = {
      "Apple Roman (macintosh)",
      "Big5",
      "big5-0",
      "Big5-HKSCS",
      "big5hkscs-0",
      "EUC-JP",
      "EUC-KR",
      "GB18030",
      "GBK (windows-936)",
      "hp-roman8",
      "IBM850",
      "IBM866",
      "ISO-2022-JP (JIS7)",
      "ISO-8859-1 (latin1)",
      "ISO-8859-2 (latin2)",
      "ISO-8859-3 (latin3)",
      "ISO-8859-4 (latin4)",
      "ISO-8859-5 (cyrillic)",
      "ISO-8859-6 (arabic)",
      "ISO-8859-7 (greek)",
      "ISO-8859-8 (hebrew)",
      "ISO-8859-9 (latin5)",
      "ISO-8859-10 (latin6)",
      "ISO-8859-13 (baltic)",
      "ISO-8859-14 (latin8, iso-celtic)",
      "ISO-8859-15 (latin9)",
      "ISO-8859-16 (latin10)",
      "ISO-10646-UCS-2 (UTF-16)",
      "Iscii-Bng",
      "Iscii-Dev",
      "Iscii-Gjr",
      "Iscii-Knd",
      "Iscii-Mlm",
      "Iscii-Ori",
      "Iscii-Pnj",
      "Iscii-Tlg",
      "Iscii-Tml",
      "jisx0201*-0",
      "KOI8-R",
      "KOI8-U",
      "ksc5601.1987-0",
      "mulelao-1",
      "Shift_JIS (SJIS, MS_Kanji)",
      "System",
      "TIS-620 (ISO 8859-11)",
      "TSCII",
      "UTF-8",
      "windows-1250",
      "windows-1251",
      "windows-1252",
      "windows-1253",
      "windows-1254",
      "windows-1255",
      "windows-1256",
      "windows-1257",
      "windows-1258",
      "WINSAMI2 (WS2)",
      0
    };
    Q_ASSERT(qstrcmp(codecs[TextEncodingLatin1Index], "ISO-8859-1 (latin1)") == 0);
    const char* const* str = codecs;
    while (*str) {
      textEncodingList += QString::fromLatin1(*str++);
    }
  }
  return textEncodingList;
}

/**
 * Remove aliases in braces from text encoding name.
 *
 * @param comboEntry text encoding name
 *
 * @return codec name.
 */
QString GeneralConfig::getTextCodecName(const QString& comboEntry)
{
  int braceIdx = comboEntry.indexOf(QLatin1String(" ("));
  return braceIdx == -1 ? comboEntry : comboEntry.left(braceIdx);
}

/**
 * Get index of text encoding in getTextCodecNames().
 * @param textEncoding text encoding name
 * @return index of encoding.
 */
int GeneralConfig::indexFromTextCodecName(const QString& textEncoding)
{
  int index = 0;
  QStringList textEncodingList = getTextCodecNames();
  for (QStringList::const_iterator it = textEncodingList.constBegin();
       it != textEncodingList.constEnd();
       ++it) {
    if (getTextCodecName(*it) == textEncoding) {
      return index;
    }
    ++index;
  }
  return TextEncodingLatin1Index;
}

/**
 * Get text encoding name from index in getTextCodecNames().
 * @param index index of encoding
 * @return text encoding name, null if index invalid.
 */
QString GeneralConfig::indexToTextCodecName(int index)
{
  QStringList textEncodingList = getTextCodecNames();
  return (index >= 0 && index < textEncodingList.size())
      ? getTextCodecName(textEncodingList.at(index))
      : QString();
}
