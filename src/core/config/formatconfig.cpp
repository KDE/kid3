/**
 * \file formatconfig.cpp
 * Format configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#include "formatconfig.h"
#include "config.h"
#include <QString>
#include <QRegularExpression>
#include <QLocale>
#include <QCoreApplication>
#include "generalconfig.h"
#include "isettings.h"
#include "frame.h"

/**
 * Constructor.
 */
FormatConfig::FormatConfig(const QString& grp)
  : GeneralConfig(grp),
    m_caseConversion(AllFirstLettersUppercase),
    m_maximumLength(255),
    m_useForOtherFileNames(true),
    m_enableMaximumLength(false),
    m_filenameFormatter(false),
    m_formatWhileEditing(false),
    m_strRepEnabled(false),
    m_enableValidation(true)
{
  m_strRepMap.clear();
}

/**
 * Destructor.
 */
FormatConfig::~FormatConfig()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Set specific properties for a filename format.
 * This will set default string conversions and not touch the file
 * extension when formatting.
 */
void FormatConfig::setAsFilenameFormatter()
{
  m_filenameFormatter = true;
  m_caseConversion = NoChanges;
  m_localeName = QString();
  m_locale.reset();
  m_strRepEnabled = true;
  m_strRepMap.append({
    {QLatin1String("/"), QLatin1String("-")},
    {QLatin1String(":"), QLatin1String("-")},
    {QLatin1String("."), QLatin1String("")},
    {QLatin1String("?"), QLatin1String("")},
    {QLatin1String("*"), QLatin1String("")},
    {QLatin1String("\""), QLatin1String("''")},
    {QLatin1String("<"), QLatin1String("-")},
    {QLatin1String(">"), QLatin1String("-")},
    {QLatin1String("|"), QLatin1String("-")},
    {QChar(0xe4), QLatin1String("ae")},
    {QChar(0xf6), QLatin1String("oe")},
    {QChar(0xfc), QLatin1String("ue")},
    {QChar(0xc4), QLatin1String("Ae")},
    {QChar(0xd6), QLatin1String("Oe")},
    {QChar(0xdc), QLatin1String("Ue")},
    {QChar(0xdf), QLatin1String("ss")},
    {QChar(0xc0), QLatin1String("A")},
    {QChar(0xc1), QLatin1String("A")},
    {QChar(0xc2), QLatin1String("A")},
    {QChar(0xc3), QLatin1String("A")},
    {QChar(0xc5), QLatin1String("A")},
    {QChar(0xc6), QLatin1String("AE")},
    {QChar(0xc7), QLatin1String("C")},
    {QChar(0xc8), QLatin1String("E")},
    {QChar(0xc9), QLatin1String("E")},
    {QChar(0xca), QLatin1String("E")},
    {QChar(0xcb), QLatin1String("E")},
    {QChar(0xcc), QLatin1String("I")},
    {QChar(0xcd), QLatin1String("I")},
    {QChar(0xce), QLatin1String("I")},
    {QChar(0xcf), QLatin1String("I")},
    {QChar(0xd0), QLatin1String("D")},
    {QChar(0xd1), QLatin1String("N")},
    {QChar(0xd2), QLatin1String("O")},
    {QChar(0xd3), QLatin1String("O")},
    {QChar(0xd4), QLatin1String("O")},
    {QChar(0xd5), QLatin1String("O")},
    {QChar(0xd7), QLatin1String("x")},
    {QChar(0xd8), QLatin1String("O")},
    {QChar(0xd9), QLatin1String("U")},
    {QChar(0xda), QLatin1String("U")},
    {QChar(0xdb), QLatin1String("U")},
    {QChar(0xdd), QLatin1String("Y")},
    {QChar(0xe0), QLatin1String("a")},
    {QChar(0xe1), QLatin1String("a")},
    {QChar(0xe2), QLatin1String("a")},
    {QChar(0xe3), QLatin1String("a")},
    {QChar(0xe5), QLatin1String("a")},
    {QChar(0xe6), QLatin1String("ae")},
    {QChar(0xe7), QLatin1String("c")},
    {QChar(0xe8), QLatin1String("e")},
    {QChar(0xe9), QLatin1String("e")},
    {QChar(0xea), QLatin1String("e")},
    {QChar(0xeb), QLatin1String("e")},
    {QChar(0xec), QLatin1String("i")},
    {QChar(0xed), QLatin1String("i")},
    {QChar(0xee), QLatin1String("i")},
    {QChar(0xef), QLatin1String("i")},
    {QChar(0xf0), QLatin1String("d")},
    {QChar(0xf1), QLatin1String("n")},
    {QChar(0xf2), QLatin1String("o")},
    {QChar(0xf3), QLatin1String("o")},
    {QChar(0xf4), QLatin1String("o")},
    {QChar(0xf5), QLatin1String("o")},
    {QChar(0xf8), QLatin1String("o")},
    {QChar(0xf9), QLatin1String("u")},
    {QChar(0xfa), QLatin1String("u")},
    {QChar(0xfb), QLatin1String("u")},
    {QChar(0xfd), QLatin1String("y")},
    {QChar(0xff), QLatin1String("y")}
  });
}

/**
 * Format a string using this configuration.
 *
 * @param str string to format
 */
void FormatConfig::formatString(QString& str) const
{
  QString ext;
  int dotPos = -1;
  if (m_filenameFormatter) {
    /* Do not format the extension if it is a filename */
    dotPos = str.lastIndexOf(QLatin1Char('.'));
    if (dotPos != -1) {
      ext = str.right(str.length() - dotPos);
      str = str.left(dotPos);
    }
  }
  if (m_caseConversion != NoChanges) {
    switch (m_caseConversion) {
      case AllLowercase:
        str = toLower(str);
        break;
      case AllUppercase:
        str = toUpper(str);
        break;
      case FirstLetterUppercase: {
        const int strLen = str.length();
        int firstLetterPos = 0;
        while (firstLetterPos < strLen && !str.at(firstLetterPos).isLetter()) {
          ++firstLetterPos;
        }
        if (firstLetterPos < strLen) {
          str = toUpper(str.left(firstLetterPos + 1)) +
                toLower(str.right(strLen - firstLetterPos - 1));
        }
        break;
      }
      case AllFirstLettersUppercase: {
        static const QString romanLetters(QLatin1String("IVXLCDM"));
        QString newstr;
        bool wordstart = true;
        const int strLen = str.length();
        for (int i = 0; i < strLen; ++i) {
          QChar ch = str.at(i);
          if (!ch.isLetterOrNumber() &&
            ch != QLatin1Char('\'') && ch != QLatin1Char('`')) {
            wordstart = true;
            newstr.append(ch);
          } else if (wordstart) {
            wordstart = false;

            // Skip word if it is a roman number
            if (romanLetters.contains(ch)) {
              int j = i + 1;
              while (j < strLen) {
                QChar c = str.at(j);
                if (!c.isLetterOrNumber()) {
                  break;
                } else if (!romanLetters.contains(c)) {
                  j = i;
                  break;
                }
                ++j;
              }
              if (j > i) {
#if QT_VERSION >= 0x060000
                newstr.append(str.mid(i, j - i));
#else
                newstr.append(str.midRef(i, j - i));
#endif
                i = j - 1;
                continue;
              }
            }

            newstr.append(toUpper(ch));
          } else {
            newstr.append(toLower(ch));
          }
        }
        str = newstr;
        break;
      }
      default:
        ;
    }
  }
  if (m_strRepEnabled) {
    for (auto it = m_strRepMap.constBegin(); it != m_strRepMap.constEnd(); ++it) {
      QString before = it->first;
      QString after = it->second;
      if (before.length() > 1 &&
          before.startsWith(QLatin1Char('/')) &&
          before.endsWith(QLatin1Char('/'))) {
        QRegularExpression re(before.mid(1, before.length() - 2));
        str.replace(re, after);
      } else {
        str.replace(before, after);
      }
    }
  }
  str = joinFileName(str, ext);
}

/**
 * Join base name and extension respecting maximum length.
 *
 * Truncation to maximumLength() is only done if enableMaximumLength() and
 * setAsFilenameFormatter() are set.
 *
 * @param baseName file name without extension
 * @param extension file name extension starting with dot
 * @return file name with extension, eventually truncated to maximum length.
 */
QString FormatConfig::joinFileName(const QString& baseName,
                                   const QString& extension) const
{
  QString str(baseName);
  QString ext(extension);
  if (m_filenameFormatter && m_enableMaximumLength) {
    if (m_maximumLength > 0 && ext.length() > m_maximumLength) {
      ext.truncate(m_maximumLength);
    }
    int maxLength = m_maximumLength - ext.length();
    if (maxLength > 0 && str.length() > maxLength) {
      str.truncate(maxLength);
      str = str.trimmed();
    }
  }
  if (!ext.isEmpty()) {
    str.append(ext);
  }
  return str;
}

/** Returns a lowercase copy of @a str. */
QString FormatConfig::toLower(const QString& str) const
{
  if (m_locale)
    return m_locale->toLower(str);
  return str.toLower();
}

/** Returns an uppercase copy of @a str. */
QString FormatConfig::toUpper(const QString& str) const
{
  if (m_locale)
    return m_locale->toUpper(str);
  return str.toUpper();
}

/**
 * Format frames using this configuration.
 *
 * @param frames frames
 */
void FormatConfig::formatFrames(FrameCollection& frames) const
{
  for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
    auto& frame = const_cast<Frame&>(*it);
    if (frame.getType() != Frame::FT_Genre) {
      QString value(frame.getValue());
      if (!value.isEmpty()) {
        formatString(value);
        frame.setValueIfChanged(value);
      }
    }
  }
}

/**
 * Format frames if format while editing is switched on.
 *
 * @param frames frames
 */
void FormatConfig::formatFramesIfEnabled(FrameCollection& frames) const
{
  if (m_formatWhileEditing) {
    formatFrames(frames);
  }
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void FormatConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("FormatWhileEditing"), QVariant(m_formatWhileEditing));
  config->setValue(QLatin1String("CaseConversion"), QVariant(m_caseConversion));
  config->setValue(QLatin1String("LocaleName"), QVariant(m_localeName));
  config->setValue(QLatin1String("StrRepEnabled"), QVariant(m_strRepEnabled));
  config->setValue(QLatin1String("EnableValidation"), QVariant(m_enableValidation));
  config->setValue(QLatin1String("UseForOtherFileNames"), QVariant(m_useForOtherFileNames));
  config->setValue(QLatin1String("EnableMaximumLength"), QVariant(m_enableMaximumLength));
  config->setValue(QLatin1String("MaximumLength"), QVariant(m_maximumLength));
  QStringList keys, values;
  for (auto it = m_strRepMap.constBegin(); it != m_strRepMap.constEnd(); ++it) {
    keys.append(it->first);
    values.append(it->second);
  }
  config->setValue(QLatin1String("StrRepMapKeys"), QVariant(keys));
  config->setValue(QLatin1String("StrRepMapValues"), QVariant(values));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void FormatConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_formatWhileEditing = config->value(QLatin1String("FormatWhileEditing"),
                                       m_formatWhileEditing).toBool();
  m_caseConversion = static_cast<CaseConversion>(
        config->value(QLatin1String("CaseConversion"),
                      static_cast<int>(m_caseConversion)).toInt());
  m_localeName = config->value(QLatin1String("LocaleName"),
                               m_localeName).toString();
  m_strRepEnabled = config->value(QLatin1String("StrRepEnabled"),
                                  m_strRepEnabled).toBool();
  m_enableValidation = config->value(QLatin1String("EnableValidation"),
                                     m_enableValidation).toBool();
  m_useForOtherFileNames = config->value(QLatin1String("UseForOtherFileNames"),
                                        m_useForOtherFileNames).toBool();
  m_enableMaximumLength = config->value(QLatin1String("EnableMaximumLength"),
                                        m_enableMaximumLength).toBool();
  m_maximumLength = config->value(QLatin1String("MaximumLength"),
                                  m_maximumLength).toInt();
  QStringList keys = config->value(QLatin1String("StrRepMapKeys"),
                                   QStringList()).toStringList();
  QStringList values = config->value(QLatin1String("StrRepMapValues"),
                                     QStringList()).toStringList();
  if (!keys.empty() && !values.empty()) {
    m_strRepMap.clear();
    for (auto itk = keys.constBegin(), itv = values.constBegin();
         itk != keys.constEnd() && itv != values.constEnd();
         ++itk, ++itv) {
      m_strRepMap.append({*itk, *itv});
    }
  }
  config->endGroup();
}

/**
 * Set name of locale to use for string conversions.
 * @param localeName locale name
 */
void FormatConfig::setLocaleName(const QString& localeName)
{
  if (localeName != m_localeName) {
    m_localeName = localeName;
    m_locale.reset(new QLocale(m_localeName));
    emit localeNameChanged(m_localeName);
  }
}

void FormatConfig::setEnableValidation(bool enableValidation)
{
  if (m_enableValidation != enableValidation) {
    m_enableValidation = enableValidation;
    emit enableValidationChanged(m_enableValidation);
  }
}

void FormatConfig::setUseForOtherFileNames(bool useForOtherFileNames)
{
  if (m_useForOtherFileNames != useForOtherFileNames) {
    m_useForOtherFileNames = useForOtherFileNames;
    emit useForOtherFileNamesChanged(m_useForOtherFileNames);
  }
}

void FormatConfig::setEnableMaximumLength(bool enableMaximumLength)
{
  if (m_enableMaximumLength != enableMaximumLength) {
    m_enableMaximumLength = enableMaximumLength;
    emit enableMaximumLengthChanged(m_enableMaximumLength);
  }
}

void FormatConfig::setMaximumLength(int maximumLength)
{
  if (m_maximumLength != maximumLength) {
    m_maximumLength = maximumLength;
    emit maximumLengthChanged(m_maximumLength);
  }
}

void FormatConfig::setStrRepMap(const QList<QPair<QString, QString>>& strRepMap)
{
  if (m_strRepMap != strRepMap) {
    m_strRepMap = strRepMap;
    emit strRepMapChanged(m_strRepMap);
  }
}

QStringList FormatConfig::strRepStringList() const
{
  QStringList lst;
  for (auto it = m_strRepMap.constBegin();
       it != m_strRepMap.constEnd();
       ++it) {
    lst.append(it->first);
    lst.append(it->second);
  }
  return lst;
}

void FormatConfig::setStrRepStringList(const QStringList& lst)
{
  QList<QPair<QString, QString>> strRepMap;
  auto it = lst.constBegin();
  while (it != lst.constEnd()) {
    QString key = *it++;
    if (it != lst.constEnd()) {
      strRepMap.append({key, *it++});
    }
  }
  setStrRepMap(strRepMap);
}

void FormatConfig::setCaseConversion(CaseConversion caseConversion)
{
  if (m_caseConversion != caseConversion) {
    m_caseConversion = caseConversion;
    emit caseConversionChanged(m_caseConversion);
  }
}

void FormatConfig::setStrRepEnabled(bool strRepEnabled)
{
  if (m_strRepEnabled != strRepEnabled) {
    m_strRepEnabled = strRepEnabled;
    emit strRepEnabledChanged(m_strRepEnabled);
  }
}

void FormatConfig::setFormatWhileEditing(bool formatWhileEditing)
{
  if (m_formatWhileEditing != formatWhileEditing) {
    m_formatWhileEditing = formatWhileEditing;
    emit formatWhileEditingChanged(m_formatWhileEditing);
  }
}

/**
 * String list of case conversion names.
 */
QStringList FormatConfig::getCaseConversionNames()
{
  static const char* const names[NumCaseConversions] = {
    QT_TRANSLATE_NOOP("@default", "No changes"),
    QT_TRANSLATE_NOOP("@default", "All lowercase"),
    QT_TRANSLATE_NOOP("@default", "All uppercase"),
    QT_TRANSLATE_NOOP("@default", "First letter uppercase"),
    QT_TRANSLATE_NOOP("@default", "All first letters uppercase")
  };
  QStringList strs;
  strs.reserve(NumCaseConversions);
  for (int i = 0; i < NumCaseConversions; ++i) {
    strs.append(QCoreApplication::translate("@default", names[i]));
  }
  return strs;
}

/**
 * String list of locale names.
 */
QStringList FormatConfig::getLocaleNames()
{
  return QStringList() << tr("None") << QLocale().uiLanguages();
}


int FilenameFormatConfig::s_index = -1;

/**
 * Constructor.
 */
FilenameFormatConfig::FilenameFormatConfig()
  : StoredConfig<FilenameFormatConfig, FormatConfig>(
      QLatin1String("FilenameFormat"))
{
  setAsFilenameFormatter();
}


int TagFormatConfig::s_index = -1;

/**
 * Constructor.
 */
TagFormatConfig::TagFormatConfig()
  : StoredConfig<TagFormatConfig, FormatConfig>(QLatin1String("TagFormat"))
{
}
