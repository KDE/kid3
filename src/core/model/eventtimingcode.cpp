/**
 * \file eventtimingcode.cpp
 * Event timing code to string conversion.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "eventtimingcode.h"

#include <QCoreApplication>

namespace {

const struct {
  const char* text;
  int code;
} codes[] = {
  { QT_TRANSLATE_NOOP("@default", "padding (has no meaning)"), 0x00 },
  { QT_TRANSLATE_NOOP("@default", "end of initial silence"), 0x01 },
  { QT_TRANSLATE_NOOP("@default", "intro start"), 0x02 },
  { QT_TRANSLATE_NOOP("@default", "main part start"), 0x03 },
  { QT_TRANSLATE_NOOP("@default", "outro start"), 0x04 },
  { QT_TRANSLATE_NOOP("@default", "outro end"), 0x05 },
  { QT_TRANSLATE_NOOP("@default", "verse start"), 0x06 },
  { QT_TRANSLATE_NOOP("@default", "refrain start"), 0x07 },
  { QT_TRANSLATE_NOOP("@default", "interlude start"), 0x08 },
  { QT_TRANSLATE_NOOP("@default", "theme start"), 0x09 },
  { QT_TRANSLATE_NOOP("@default", "variation start"), 0x0a },
  { QT_TRANSLATE_NOOP("@default", "key change"), 0x0b },
  { QT_TRANSLATE_NOOP("@default", "time change"), 0x0c },
  { QT_TRANSLATE_NOOP("@default", "momentary unwanted noise (Snap, Crackle & Pop)"), 0x0d },
  { QT_TRANSLATE_NOOP("@default", "sustained noise"), 0x0e },
  { QT_TRANSLATE_NOOP("@default", "sustained noise end"), 0x0f },
  { QT_TRANSLATE_NOOP("@default", "intro end"), 0x10 },
  { QT_TRANSLATE_NOOP("@default", "main part end"), 0x11 },
  { QT_TRANSLATE_NOOP("@default", "verse end"), 0x12 },
  { QT_TRANSLATE_NOOP("@default", "refrain end"), 0x13 },
  { QT_TRANSLATE_NOOP("@default", "theme end"), 0x14 },
  { QT_TRANSLATE_NOOP("@default", "profanity"), 0x15 },
  { QT_TRANSLATE_NOOP("@default", "profanity end"), 0x16 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 0"), 0xe0 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 1"), 0xe1 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 2"), 0xe2 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 3"), 0xe3 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 4"), 0xe4 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 5"), 0xe5 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 6"), 0xe6 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 7"), 0xe7 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 8"), 0xe8 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch 9"), 0xe9 },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch A"), 0xea },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch B"), 0xeb },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch C"), 0xec },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch D"), 0xed },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch E"), 0xee },
  { QT_TRANSLATE_NOOP("@default", "not predefined synch F"), 0xef },
  { QT_TRANSLATE_NOOP("@default", "audio end (start of silence)"), 0xfd },
  { QT_TRANSLATE_NOOP("@default", "audio file ends"), 0xfe }
};

const int numCodes = static_cast<int>(sizeof codes / sizeof codes[0]);

}

/**
 * Get string representation.
 * @return code description.
 */
QString EventTimeCode::toString() const
{
  for (int i = 0; i < numCodes; ++i) {
    if (codes[i].code == m_code) {
      return QString::fromLatin1(codes[i].text);
    }
  }
  return QString(QLatin1String("reserved for future use %1")).
      arg(m_code, 2, 16, QLatin1Char('0'));
}

/**
 * Get translated string representation.
 * @return code description.
 */
QString EventTimeCode::toTranslatedString() const
{
  for (int i = 0; i < numCodes; ++i) {
    if (codes[i].code == m_code) {
      return QCoreApplication::translate("@default", codes[i].text);
    }
  }
  const char* const reservedForFutureUseStr =
      QT_TRANSLATE_NOOP("@default", "reserved for future use %1");
  return QCoreApplication::translate("@default", reservedForFutureUseStr).
      arg(m_code, 2, 16, QLatin1Char('0'));
}

/**
 * Get index of code in list of strings.
 * @return index.
 */
int EventTimeCode::toIndex() const
{
  for (int i = 0; i < numCodes; ++i) {
    if (codes[i].code == m_code) {
      return i;
    }
  }
  return -1;
}

/**
 * Create from string.
 * @param str untranslated string
 * @return event time code.
 */
EventTimeCode EventTimeCode::fromString(const char* str)
{
  for (int i = 0; i < numCodes; ++i) {
    if (qstrcmp(codes[i].text, str) == 0) {
      return EventTimeCode(codes[i].code);
    }
  }
  return EventTimeCode(-1);
}

/**
 * Create from index.
 * @param index index
 * @return event time code.
 */
EventTimeCode EventTimeCode::fromIndex(int index)
{
  if (index >= 0 && index < numCodes) {
    return EventTimeCode(codes[index].code);
  }
  return EventTimeCode(-1);
}

/**
 * Get list of translated strings.
 * @return code descriptions.
 */
QStringList EventTimeCode::getTranslatedStrings()
{
  QStringList strings;
  strings.reserve(numCodes);
  for (int i = 0; i < numCodes; ++i) {
    strings << QCoreApplication::translate("@default", codes[i].text);
  }
  return strings;
}
