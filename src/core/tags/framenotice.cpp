/**
 * \file framenotice.cpp
 * Warning about tag frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 May 2017
 *
 * Copyright (C) 2017  Urs Fleisch
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

#include "framenotice.h"
#include <QCoreApplication>
#include <QSet>
#include <QRegExp>
#include "frame.h"

namespace {

using CheckFunction = bool (*)(const QString&);

bool beginsWithYearAndSpace(const QString& str)
{
  if (str.length() < 5 || str.at(4) != QLatin1Char(' '))
    return false;

  for (int i = 0; i < 4; ++i) {
    if (!str.at(i).isDigit()) {
      return false;
    }
  }
  return true;
}

bool isDayMonth(const QString& str)
{
  if (str.length() != 4)
    return false;

  int day = str.leftRef(2).toInt();
  int month = str.midRef(2).toInt();
  return !(day < 1 || day > 31 || month < 1 || month > 12);
}

bool isHourMinute(const QString& str)
{
  if (str.length() != 4)
    return false;

  int hour = str.leftRef(2).toInt();
  int minute = str.midRef(2).toInt();
  return !(hour < 0 || hour > 23 || minute < 0 || minute > 59);
}

bool isNumeric(const QString& str)
{
  bool ok;
  str.toInt(&ok);
  return ok;
}

bool isYear(const QString& str)
{
  return str.length() == 4 && isNumeric(str);
}

bool isNumberTotal(const QString& str)
{
  const int slashPos = str.indexOf(QLatin1Char('/'));
  if (slashPos != -1) {
    return isNumeric(str.left(slashPos)) && isNumeric(str.mid(slashPos + 1));
  } else {
    return isNumeric(str);
  }
}

bool isIsrc(const QString& str)
{
  if (str.length() != 12)
    return false;

  for (int i = 0; i < 5; ++i) {
    if (!str.at(i).isLetterOrNumber()) {
      return false;
    }
  }
  for (int i = 5; i < 12; ++i) {
    if (!str.at(i).isDigit()) {
      return false;
    }
  }
  return true;
}

bool isIsoDateTime(const QString& str)
{
  return FrameNotice::isoDateTimeRexExp().exactMatch(str);
}

bool isMusicalKey(const QString& str)
{
  const int len = str.length();
  if (len < 1 || len > 3)
    return false;

  // Although not in the ID3v2 standard, allow commonly used Camelot wheel
  // values 1A-12A, 1B-12B http://www.mixedinkey.com/harmonic-mixing-guide/
  const QChar lastChar = str.at(len - 1);
  if (lastChar == QLatin1Char('A') || lastChar == QLatin1Char('B')) {
    bool ok;
    int nr = str.leftRef(len - 1).toInt(&ok);
    if (ok && nr >= 1 && nr <= 12) {
      return true;
    }
  }

  const QString allowedChars(QLatin1String("ABCDEFGb#mo"));
  for (int i = 0; i < len; ++i) {
    if (!allowedChars.contains(str.at(i))) {
      return false;
    }
  }
  return true;
}

bool isLanguageCode(const QString& str)
{
  if (str.length() != 3)
    return false;

  if (str == QLatin1String("XXX"))
    return true;

  for (int i = 0; i < 3; ++i) {
    const QChar ch = str.at(i);
    if (!ch.isLetter() || !ch.isLower()) {
      return false;
    }
  }
  return true;
}

bool isStringList(const QString& str)
{
  return str.contains(QLatin1Char('|'));
}

}

/**
 * Get translated description of notice.
 * @return description, empty if none.
 */
QString FrameNotice::getDescription() const
{
  static const char* const descriptions[] = {
    "",
    QT_TRANSLATE_NOOP("@default", "Truncated"),
    QT_TRANSLATE_NOOP("@default", "Size is too large"),
    QT_TRANSLATE_NOOP("@default", "Must be unique"),
    QT_TRANSLATE_NOOP("@default", "New line is forbidden"),
    QT_TRANSLATE_NOOP("@default", "Carriage return is forbidden"),
    QT_TRANSLATE_NOOP("@default", "Owner must be non-empty"),
    QT_TRANSLATE_NOOP("@default", "Must be numeric"),
    QT_TRANSLATE_NOOP("@default", "Must be numeric or number/total"),
    QT_TRANSLATE_NOOP("@default", "Format is DDMM"),
    QT_TRANSLATE_NOOP("@default", "Format is HHMM"),
    QT_TRANSLATE_NOOP("@default", "Format is YYYY"),
    QT_TRANSLATE_NOOP("@default", "Must begin with a year and a space character"),
    QT_TRANSLATE_NOOP("@default", "Must be ISO 8601 date/time"),
    QT_TRANSLATE_NOOP("@default", "Must be musical key, 3 characters, A-G, b, #, m, o\n"
                                  "or Camelot wheel value 1A-12A, 1B-12B"),
    QT_TRANSLATE_NOOP("@default", "Must have ISO 639-2 language code, 3 lowercase characters"),
    QT_TRANSLATE_NOOP("@default", "Must be ISRC code, 12 characters"),
    QT_TRANSLATE_NOOP("@default", "Must be list of strings separated by '|'"),
    QT_TRANSLATE_NOOP("@default", "Has excess white space"),
  };
  struct not_used { int array_size_check[
      sizeof(descriptions) / sizeof(descriptions[0]) == NumWarnings
      ? 1 : -1 ]; };
  return m_warning < NumWarnings
      ? QCoreApplication::translate("@default", descriptions[m_warning])
      : QString();
}

/**
 * Get regular expression to validate an ISO 8601 date/time.
 * @return regular expression matching ISO date/time and periods.
 */
const QRegExp& FrameNotice::isoDateTimeRexExp()
{
  static const QRegExp isoDateRe(QLatin1String(
    // This is a simplified regular expression from
    // http://www.pelagodesign.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/
    // relaxed to allow appending any string after a slash, so that ISO 8601
    // periods of time can by entered while still providing suffient validation.
    "(\\d{4})(-((0[1-9]|1[0-2])(-([12]\\d|0[1-9]|3[01]))?)(T((([01]\\d|2[0-3])"
    "(:[0-5]\\d)?|24:00))?(:[0-5]\\d([\\.,]\\d+)?)?"
    "([zZ]|([\\+-])([01]\\d|2[0-3]):?([0-5]\\d)?)?)?(/.*)?)?"
  ));
  return isoDateRe;
}

/**
 * Check if a picture frame exceeds a given size.
 * TooLarge notice is set in @a frame, if its size is larger than @a maxSize.
 * @param frame picture frame to check
 * @param maxSize maximum size of picture data in bytes
 * @return true if size too large.
 */
bool FrameNotice::addPictureTooLargeNotice(Frame& frame, int maxSize)
{
  QVariant data = Frame::getField(frame, Frame::ID_Data);
  if (!data.isNull()) {
    if (data.toByteArray().size() > maxSize) {
      frame.setMarked(FrameNotice::TooLarge);
      return true;
    }
  }
  return false;
}

/**
 * Check if frames violate the ID3v2 standard.
 * Violating frames are marked with the corresponding notice.
 * @param frames frames to check
 * @return true if a violation is detected.
 */
bool FrameNotice::addId3StandardViolationNotice(FrameCollection& frames)
{
  static const struct {
    const char* id;
    Warning warning;
  } idWarning[] = {
    {"TBPM", Numeric},
    {"TDLY", Numeric},
    {"TLEN", Numeric},
    {"TSIZ", Numeric},
    {"TCOP", YearSpace},
    {"TPRO", YearSpace},
    {"TDAT", DayMonth},
    {"TIME", HourMinute},
    {"TORY", Year},
    {"TYER", Year},
    {"TPOS", NrTotal},
    {"TRCK", NrTotal},
    {"TSRC", IsrcCode},
    {"TDEN", IsoDate},
    {"TDOR", IsoDate},
    {"TDRC", IsoDate},
    {"TDRL", IsoDate},
    {"TDTG", IsoDate},
    {"TKEY", MusicalKey},
    {"TLAN", LanguageCode},
    {"IPLS", StringList},
    {"TMCL", StringList},
    {"TIPL", StringList},
  };
  static const struct {
    Warning warning;
    CheckFunction func;
  } warningFunc[] = {
    {Numeric, isNumeric},
    {YearSpace, beginsWithYearAndSpace},
    {DayMonth, isDayMonth},
    {HourMinute, isHourMinute},
    {Year, isYear},
    {NrTotal, isNumberTotal},
    {IsrcCode, isIsrc},
    {IsoDate, isIsoDateTime},
    {MusicalKey, isMusicalKey},
    {LanguageCode, isLanguageCode},
    {StringList, isStringList},
  };
  static const struct {
    const char* id;
    Frame::FieldId field;
  } uniqIdField[] = {
    {"UFID", Frame::ID_Owner},
    {"TXXX", Frame::ID_Description},
    {"WXXX", Frame::ID_Description},
    {"IPLS", Frame::ID_NoField},
    {"USLT", Frame::ID_Language}, // and Frame::ID_Description
    {"SYLT", Frame::ID_Language}, // and Frame::ID_Description
    {"COMM", Frame::ID_Language}, // and Frame::ID_Description
    {"USER", Frame::ID_Language},
    {"APIC", Frame::ID_PictureType}, // and Frame::ID_Description
    {"GEOB", Frame::ID_Description},
    {"PCNT", Frame::ID_NoField},
    {"POPM", Frame::ID_Email},
    {"RBUF", Frame::ID_NoField},
    {"AENC", Frame::ID_Owner},
    {"LINK", Frame::ID_Id}, // and Frame::ID_Url, Frame::ID_Text
    {"POSS", Frame::ID_NoField},
    {"OWNE", Frame::ID_NoField},
    {"COMR", Frame::ID_Data},
    {"ENCR", Frame::ID_Owner},
    {"GRID", Frame::ID_Owner},
    {"PRIV", Frame::ID_Owner},
  };
  static QMap<QString, Warning> warnings;
  static QMap<Warning, CheckFunction> checks;
  static QMap<QString, Frame::FieldId> uniques;
  if (warnings.isEmpty()) {
    for (unsigned int i = 0;
         i < sizeof(idWarning) / sizeof (idWarning[0]);
         ++i) {
      warnings[QString::fromLatin1(idWarning[i].id)] = idWarning[i].warning;
    }
    for (unsigned int i = 0;
         i < sizeof(warningFunc) / sizeof (warningFunc[0]);
         ++i) {
      checks[warningFunc[i].warning] = warningFunc[i].func;
    }
    for (unsigned int i = 0;
         i < sizeof(uniqIdField) / sizeof (uniqIdField[0]);
         ++i) {
      uniques[QString::fromLatin1(uniqIdField[i].id)] = uniqIdField[i].field;
    }
  }

  QSet<QString> uniqueIds;
  bool marked = false;
  for (auto it = frames.begin(); it != frames.end(); ++it) {
    auto& frame = const_cast<Frame&>(*it);
    QString name = frame.getInternalName();
    QString id = name.left(4);
    QString uniqueId;

    // Check for uniqueness.
    static const Frame::FieldId NOT_UNIQUE = Frame::ID_Subframe;
    Frame::FieldId fieldId = uniques.value(id, NOT_UNIQUE);
    if (fieldId != NOT_UNIQUE) {
      uniqueId = id;
      if (fieldId != Frame::ID_NoField) {
        uniqueId += frame.getFieldValue(fieldId).toString();
        if (fieldId == Frame::ID_Language || fieldId == Frame::ID_PictureType) {
          uniqueId += frame.getFieldValue(Frame::ID_Description).toString();
        } else if (fieldId == Frame::ID_Id) {
          uniqueId += frame.getFieldValue(Frame::ID_Url).toString();
          uniqueId += frame.getFieldValue(Frame::ID_Text).toString();
        }
      }
    } else if (id.startsWith(QLatin1Char('T')) ||
               (id.startsWith(QLatin1Char('W')) &&
                id != QLatin1String("WCOM") && id != QLatin1String("WOAR"))) {
      uniqueId = id;
    }
    if (!uniqueId.isEmpty()) {
      if (uniqueIds.contains(uniqueId)) {
        frame.setMarked(Unique);
        marked = true;
        continue;
      } else {
        uniqueIds.insert(uniqueId);
      }
    }

    // Check value formats.
    QString value = frame.getValue();

    Warning warning = warnings.value(id, None);
    if (warning != None) {
      CheckFunction check = checks.value(warning);
      if (check && !check(value)) {
        frame.setMarked(warning);
        marked = true;
        continue;
      }
    }

    // If nothing else is said newline character is forbidden.
    // Allowed in full text strings: USLT, SYLT, USER, COMM.
    if (value.contains(QLatin1Char('\n'))) {
      if (id != QLatin1String("COMM") && id != QLatin1String("USLT") &&
          id != QLatin1String("SYLT") && id != QLatin1String("USER")) {
        frame.setMarked(NlForbidden);
        marked = true;
        continue;
      }
      // A newline is represented, when allowed, with $0A only.
      if (value.contains(QLatin1String("\r\n")) &&
          frame.getFieldValue(Frame::ID_TextEnc).toInt() == Frame::TE_ISO8859_1)
      {
        frame.setMarked(CrForbidden);
        marked = true;
        continue;
      }
    }
    if (value.startsWith(QLatin1Char(' ')) ||
        value.endsWith(QLatin1Char(' '))) {
      frame.setMarked(ExcessSpace);
      marked = true;
      continue;
    }
    // 'Owner identifier' must be non-empty.
    if (id == QLatin1String("UFID") &&
        frame.getFieldValue(Frame::ID_Owner).toString().isEmpty()) {
      frame.setMarked(OwnerEmpty);
      marked = true;
    // USLT, SYLT, COMM, USER: The language should be represented in lower case.
    // If the language is not known the string "XXX" should be used.
    // USER is omitted because it is not supported by TagLib and would give
    // false positives.
    } else if ((id == QLatin1String("COMM") || id == QLatin1String("USLT") ||
                id == QLatin1String("SYLT")) &&
               !isLanguageCode(frame.getFieldValue(Frame::ID_Language)
                               .toString())) {
      frame.setMarked(LanguageCode);
      marked = true;
    }
  }
  return marked;
}
