/**
 * \file jsonparser.cpp
 * JSON serializer and deserializer.
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

#include "jsonparser.h"
#include <QMap>
#include <QString>
#include <QVariant>
#include <QDateTime>
#include <limits.h>

/**
 * JSON deserializer.
 */
class JsonDeserializer {
public:
  /**
   * Constructor.
   */
  JsonDeserializer();

  /**
   * Deserialize a JSON string to a string-variant map.
   * @param str string to deserialize
   * @param ok if not null, true is returned here on success
   * @return deserialized string-variant map
   */
  QVariant deserialize(const QString& str, bool* ok = nullptr);

private:
  void skipWhiteSpace();
  bool requireDelimiter(const QString& delimiters);
  QString parseSymbol();

  QString m_str;
  int m_len;
  int m_pos;
};


namespace {

QString stripQuotes(const QString& str)
{
  return (str.startsWith(QLatin1Char('"')) && str.endsWith(QLatin1Char('"')))
      ? str.mid(1, str.length() - 2)
      : str;
}

QVariant valueStringToVariant(const QString& value)
{
  bool ok;
  if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))) {
    // The value is probably a string, but could also be a date/time.
    QString str(value.mid(1, value.length() - 2));
    QDateTime dt(QDateTime::fromString(str, Qt::ISODate));
    if (dt.isValid())
      return dt;
    else
      return str;
  }

  // Return nested objects or arrays.
  if (value.startsWith(QLatin1Char('{')) || value.startsWith(QLatin1Char('[')))
    return JsonDeserializer().deserialize(value);

  if (value == QLatin1String("true"))
    return true;
  else if (value == QLatin1String("false"))
    return false;
  else if (value == QLatin1String("null"))
    return QVariant();

  qlonglong num = value.toLongLong(&ok);
  if (ok) {
    if (num >= INT_MIN && num <= INT_MAX)
      return static_cast<int>(num);
    return num;
  }

  double dbl = value.toDouble(&ok);
  if (ok)
    return dbl;

  return value;
}

QString variantToValueString(const QVariant& var)
{
  QString value;
  QVariant::Type type = var.type();
  if (!var.isValid()) {
    value = QLatin1String("null");
  } else if (type == QVariant::List) {
    QVariantList lst(var.toList());
    // Serialize into an array container "[...]".
    for (int i = 0; i < lst.size(); i++) {
      value += QString(QLatin1String("%1%2")).arg(value.isEmpty() ? QLatin1String("") : QLatin1String(", ")).
          arg(variantToValueString(lst.at(i)));
    }
    value = QString(QLatin1String("[%1]")).arg(value);
  } else if (type == QVariant::Map) {
    // Serialize into an object container "{...}".
    QVariantMap map(var.toMap());
    for (QMap<QString, QVariant>::const_iterator it = map.constBegin();
         it != map.constEnd();
         ++it) {
      value += QString(QLatin1String("%1\"%2\": %3")).arg(value.isEmpty() ? QLatin1String("") : QLatin1String(", ")).
          arg(it.key()).arg(variantToValueString(it.value()));
    }
    value = QString(QLatin1String("{%1}")).arg(value);
  } else {
    value = var.toString();
    if (value.startsWith(QLatin1Char('{')) || value.startsWith(QLatin1Char('['))) {
      ; // keep value
    } else if (type == QVariant::String || type == QVariant::DateTime ||
               type == QVariant::Date || type == QVariant::Time) {
      value = QLatin1Char('"') +
          value.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char('"'), QLatin1String("\\\"")) + QLatin1Char('"');
    }
  }
  return value;
}

}


JsonDeserializer::JsonDeserializer(): m_len(0), m_pos(0)
{
}

QVariant JsonDeserializer::deserialize(const QString& str,
                                       bool* ok)
{
  QVariant result;
  bool isOk = false;
  m_str = str;
  m_len = str.length();
  m_pos = 0;
  if (requireDelimiter(QLatin1String("{"))) {
    // Deserialize from object container "{...}".
    QVariantMap map;
    isOk = true;
    while (m_pos < m_len) {
      QString key = parseSymbol();
      if (key.isEmpty() || !requireDelimiter(QLatin1String(":"))) {
        isOk = false;
        break;
      }
      QString value = parseSymbol();
      if (value.isEmpty() || !requireDelimiter(QLatin1String(",}"))) {
        isOk = false;
        break;
      }
      map.insert(stripQuotes(key), valueStringToVariant(value));
    }
    result = map;
  } else if (requireDelimiter(QLatin1String("["))) {
    // Deserialize from array container "[...]".
    QVariantList lst;
    isOk = true;
    for (int i = 0; m_pos < m_len; i++) {
      QString value = parseSymbol();
      if (value.isEmpty() || !requireDelimiter(QLatin1String(",]"))) {
        isOk = false;
        break;
      }
      lst.append(valueStringToVariant(value));
    }
    result = lst;
  }
  if (ok) {
    *ok = isOk;
  }
  return result;
}

void JsonDeserializer::skipWhiteSpace()
{
  QChar ch;
  while (m_pos < m_len && ((ch = m_str.at(m_pos)) == QLatin1Char(' ') ||
                         ch == QLatin1Char('\t') || ch == QLatin1Char('\r') || ch == QLatin1Char('\n'))) {
    ++m_pos;
  }
}

bool JsonDeserializer::requireDelimiter(const QString& delimiters)
{
  skipWhiteSpace();
  if (m_pos < m_len && delimiters.indexOf(m_str.at(m_pos)) != -1) {
    ++m_pos;
    skipWhiteSpace();
    return true;
  }
  return false;
}

QString JsonDeserializer::parseSymbol()
{
  QString result;
  skipWhiteSpace();
  if (m_pos < m_len) {
    const QChar beginCh = m_str.at(m_pos);
    if (beginCh == QLatin1Char('"')) {
      // String, get symbol between double quotes respecting escaped quotes.
      int endPos;
      int searchPos = m_pos + 1;
      forever {
        endPos = m_str.indexOf(QLatin1Char('"'), searchPos);
        if (endPos < 1 || m_str.at(endPos - 1) != QLatin1Char('\\'))
          break;
        searchPos = endPos + 1;
      }
      if (endPos > m_pos) {
        result = m_str.mid(m_pos, ++endPos - m_pos).
            replace(QLatin1String("\\\""), QLatin1String("\"")).replace(QLatin1String("\\\\"), QLatin1String("\\"));
        m_pos = endPos;
      }
    } else if (beginCh == QLatin1Char('{') || beginCh == QLatin1Char('[')) {
      // Object or array, find end. Nesting is supported.
      const QChar endCh = beginCh == QLatin1Char('{') ? QLatin1Char('}') : QLatin1Char(']');
      int nestingLevel = 0;
      bool insideString = false;
      QChar lastCh;
      int endPos = m_pos + 1;
      while (endPos < m_len) {
        const QChar ch = m_str.at(endPos);
        if (insideString) {
          if (ch == QLatin1Char('"') && lastCh != QLatin1Char('\\')) {
            insideString = false;
          }
        } else {
          if (ch == QLatin1Char('"')) {
            insideString = true;
          } else if (ch == beginCh) {
            ++nestingLevel;
          } else if (ch == endCh) {
            if (nestingLevel == 0) {
              break;
            } else {
              --nestingLevel;
            }
          }
        }
        lastCh = ch;
        ++endPos;
      }
      if (endPos < m_len) {
        result = m_str.mid(m_pos, ++endPos - m_pos);
        m_pos = endPos;
      }
    } else {
      // Probably number or symbol without whitespace.
      int startPos = m_pos;
      const QString endChars = QLatin1String(" \t\r\n:,}]");
      while (m_pos < m_len && endChars.indexOf(m_str.at(m_pos)) == -1) {
        ++m_pos;
      }
      result = m_str.mid(startPos, m_pos - startPos);
    }
  }
  skipWhiteSpace();
  return result;
}


/**
 * Serialize a variant as a JSON string.
 * @param var variant
 * @return JSON representation of @a var.
 */
QString JsonParser::serialize(const QVariant& var)
{
  return variantToValueString(var);
}

/**
 * Deserialize a JSON string to a string-variant map.
 * @param str string to deserialize
 * @param ok if not null, true is returned here on success
 * @return deserialized string-variant map
 */
QVariant JsonParser::deserialize(const QString& str, bool* ok)
{
  return JsonDeserializer().deserialize(str, ok);
}
