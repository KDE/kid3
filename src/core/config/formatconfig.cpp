/**
 * \file formatconfig.cpp
 * Format configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2017  Urs Fleisch
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
#include <QLocale>
#include <QCoreApplication>
#include "generalconfig.h"
#include "frame.h"

/**
 * Constructor.
 */
FormatConfig::FormatConfig(const QString& grp) :
  GeneralConfig(grp),
  m_caseConversion(AllFirstLettersUppercase),
  m_maximumLength(255),
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
  m_strRepMap[QLatin1String("/")] = QLatin1Char('-');
  m_strRepMap[QLatin1String(":")] = QLatin1Char('-');
  m_strRepMap[QLatin1String(".")] = QLatin1String("");
  m_strRepMap[QLatin1String("?")] = QLatin1String("");
  m_strRepMap[QLatin1String("*")] = QLatin1String("");
  m_strRepMap[QLatin1String("\"")] = QLatin1String("''");
  m_strRepMap[QLatin1String("<")] = QLatin1Char('-');
  m_strRepMap[QLatin1String(">")] = QLatin1Char('-');
  m_strRepMap[QLatin1String("|")] = QLatin1Char('-');
  m_strRepMap[QChar(0xe4)] = QLatin1String("ae");
  m_strRepMap[QChar(0xf6)] = QLatin1String("oe");
  m_strRepMap[QChar(0xfc)] = QLatin1String("ue");
  m_strRepMap[QChar(0xc4)] = QLatin1String("Ae");
  m_strRepMap[QChar(0xd6)] = QLatin1String("Oe");
  m_strRepMap[QChar(0xdc)] = QLatin1String("Ue");
  m_strRepMap[QChar(0xdf)] = QLatin1String("ss");
  m_strRepMap[QChar(0xc0)] = QLatin1Char('A');
  m_strRepMap[QChar(0xc1)] = QLatin1Char('A');
  m_strRepMap[QChar(0xc2)] = QLatin1Char('A');
  m_strRepMap[QChar(0xc3)] = QLatin1Char('A');
  m_strRepMap[QChar(0xc5)] = QLatin1Char('A');
  m_strRepMap[QChar(0xc6)] = QLatin1String("AE");
  m_strRepMap[QChar(0xc7)] = QLatin1Char('C');
  m_strRepMap[QChar(0xc8)] = QLatin1Char('E');
  m_strRepMap[QChar(0xc9)] = QLatin1Char('E');
  m_strRepMap[QChar(0xca)] = QLatin1Char('E');
  m_strRepMap[QChar(0xcb)] = QLatin1Char('E');
  m_strRepMap[QChar(0xcc)] = QLatin1Char('I');
  m_strRepMap[QChar(0xcd)] = QLatin1Char('I');
  m_strRepMap[QChar(0xce)] = QLatin1Char('I');
  m_strRepMap[QChar(0xcf)] = QLatin1Char('I');
  m_strRepMap[QChar(0xd0)] = QLatin1Char('D');
  m_strRepMap[QChar(0xd1)] = QLatin1Char('N');
  m_strRepMap[QChar(0xd2)] = QLatin1Char('O');
  m_strRepMap[QChar(0xd3)] = QLatin1Char('O');
  m_strRepMap[QChar(0xd4)] = QLatin1Char('O');
  m_strRepMap[QChar(0xd5)] = QLatin1Char('O');
  m_strRepMap[QChar(0xd7)] = QLatin1Char('x');
  m_strRepMap[QChar(0xd8)] = QLatin1Char('O');
  m_strRepMap[QChar(0xd9)] = QLatin1Char('U');
  m_strRepMap[QChar(0xda)] = QLatin1Char('U');
  m_strRepMap[QChar(0xdb)] = QLatin1Char('U');
  m_strRepMap[QChar(0xdd)] = QLatin1Char('Y');
  m_strRepMap[QChar(0xe0)] = QLatin1Char('a');
  m_strRepMap[QChar(0xe1)] = QLatin1Char('a');
  m_strRepMap[QChar(0xe2)] = QLatin1Char('a');
  m_strRepMap[QChar(0xe3)] = QLatin1Char('a');
  m_strRepMap[QChar(0xe5)] = QLatin1Char('a');
  m_strRepMap[QChar(0xe6)] = QLatin1String("ae");
  m_strRepMap[QChar(0xe7)] = QLatin1Char('c');
  m_strRepMap[QChar(0xe8)] = QLatin1Char('e');
  m_strRepMap[QChar(0xe9)] = QLatin1Char('e');
  m_strRepMap[QChar(0xea)] = QLatin1Char('e');
  m_strRepMap[QChar(0xeb)] = QLatin1Char('e');
  m_strRepMap[QChar(0xec)] = QLatin1Char('i');
  m_strRepMap[QChar(0xed)] = QLatin1Char('i');
  m_strRepMap[QChar(0xee)] = QLatin1Char('i');
  m_strRepMap[QChar(0xef)] = QLatin1Char('i');
  m_strRepMap[QChar(0xf0)] = QLatin1Char('d');
  m_strRepMap[QChar(0xf1)] = QLatin1Char('n');
  m_strRepMap[QChar(0xf2)] = QLatin1Char('o');
  m_strRepMap[QChar(0xf3)] = QLatin1Char('o');
  m_strRepMap[QChar(0xf4)] = QLatin1Char('o');
  m_strRepMap[QChar(0xf5)] = QLatin1Char('o');
  m_strRepMap[QChar(0xf8)] = QLatin1Char('o');
  m_strRepMap[QChar(0xf9)] = QLatin1Char('u');
  m_strRepMap[QChar(0xfa)] = QLatin1Char('u');
  m_strRepMap[QChar(0xfb)] = QLatin1Char('u');
  m_strRepMap[QChar(0xfd)] = QLatin1Char('y');
  m_strRepMap[QChar(0xff)] = QLatin1Char('y');
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
      case FirstLetterUppercase:
        str = toUpper(str.at(0)) + toLower(str.right(str.length() - 1));
        break;
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
                newstr.append(str.midRef(i, j - i));
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
      str.replace(it.key(), *it);
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
  for (auto it = frames.begin(); it != frames.end(); ++it) {
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
  config->setValue(QLatin1String("EnableMaximumLength"), QVariant(m_enableMaximumLength));
  config->setValue(QLatin1String("MaximumLength"), QVariant(m_maximumLength));
  config->setValue(QLatin1String("StrRepMapKeys"), QVariant(m_strRepMap.keys()));
  config->setValue(QLatin1String("StrRepMapValues"), QVariant(m_strRepMap.values()));
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
  m_formatWhileEditing = config->value(QLatin1String("FormatWhileEditing"), m_formatWhileEditing).toBool();
  m_caseConversion = (CaseConversion)config->value(QLatin1String("CaseConversion"),
                                                   (int)m_caseConversion).toInt();
  m_localeName = config->value(QLatin1String("LocaleName"), m_localeName).toString();
  m_strRepEnabled = config->value(QLatin1String("StrRepEnabled"), m_strRepEnabled).toBool();
  m_enableValidation = config->value(QLatin1String("EnableValidation"), m_enableValidation).toBool();
  m_enableMaximumLength = config->value(QLatin1String("EnableMaximumLength"), m_enableMaximumLength).toBool();
  m_maximumLength = config->value(QLatin1String("MaximumLength"), m_maximumLength).toInt();
  QStringList keys = config->value(QLatin1String("StrRepMapKeys"),
                                   QStringList()).toStringList();
  QStringList values = config->value(QLatin1String("StrRepMapValues"),
                                     QStringList()).toStringList();
  if (!keys.empty() && !values.empty()) {
    m_strRepMap.clear();
    for (auto itk = keys.constBegin(), itv = values.constBegin();
         itk != keys.constEnd() && itv != values.constEnd();
         ++itk, ++itv) {
      m_strRepMap[*itk] = *itv;
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

void FormatConfig::setStrRepMap(const QMap<QString, QString>& strRepMap)
{
  if (m_strRepMap != strRepMap) {
    m_strRepMap = strRepMap;
    emit strRepMapChanged(m_strRepMap);
  }
}

QVariantMap FormatConfig::strRepVariantMap() const
{
  QVariantMap map;
  for (auto it = m_strRepMap.constBegin();
       it != m_strRepMap.constEnd();
       ++it) {
    map.insert(it.key(), it.value());
  }
  return map;
}

void FormatConfig::setStrRepVariantMap(const QVariantMap& map)
{
  QMap<QString, QString> strRepMap;
  for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
    strRepMap.insert(it.key(), it.value().toString());
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
FilenameFormatConfig::FilenameFormatConfig() :
  StoredConfig<FilenameFormatConfig, FormatConfig>(
    QLatin1String("FilenameFormat"))
{
  setAsFilenameFormatter();
}


int TagFormatConfig::s_index = -1;

/**
 * Constructor.
 */
TagFormatConfig::TagFormatConfig() :
  StoredConfig<TagFormatConfig, FormatConfig>(QLatin1String("TagFormat"))
{
}
