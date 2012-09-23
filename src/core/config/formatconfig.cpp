/**
 * \file formatconfig.cpp
 * Format configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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
#ifdef CONFIG_USE_KDE
#include <kconfiggroup.h>
#else
#include <QString>
#include <QStringList>
#endif
#include "generalconfig.h"
#include "frame.h"

/**
 * Constructor.
 */
FormatConfig::FormatConfig(const QString& grp) :
  GeneralConfig(grp),
  m_formatWhileEditing(false),
  m_caseConversion(AllFirstLettersUppercase),
  m_useSystemLocale(false),
  m_strRepEnabled(false),
  m_filenameFormatter(false)
{
  m_strRepMap.clear();
}

/**
 * Destructor.
 */
FormatConfig::~FormatConfig() {}

/**
 * Set specific properties for a filename format.
 * This will set default string conversions and not touch the file
 * extension when formatting.
 */
void FormatConfig::setAsFilenameFormatter()
{
  m_filenameFormatter = true;
  m_caseConversion = NoChanges;
  m_useSystemLocale = false;
  m_strRepEnabled = true;
  m_strRepMap["/"] = "-";
  m_strRepMap[":"] = "-";
  m_strRepMap["."] = "";
  m_strRepMap["?"] = "";
  m_strRepMap["*"] = "";
  m_strRepMap["\""] = "''";
  m_strRepMap["<"] = "-";
  m_strRepMap[">"] = "-";
  m_strRepMap["|"] = "-";
  m_strRepMap[QChar(0xe4)] = "ae";
  m_strRepMap[QChar(0xf6)] = "oe";
  m_strRepMap[QChar(0xfc)] = "ue";
  m_strRepMap[QChar(0xc4)] = "Ae";
  m_strRepMap[QChar(0xd6)] = "Oe";
  m_strRepMap[QChar(0xdc)] = "Ue";
  m_strRepMap[QChar(0xdf)] = "ss";
  m_strRepMap[QChar(0xc0)] = "A";
  m_strRepMap[QChar(0xc1)] = "A";
  m_strRepMap[QChar(0xc2)] = "A";
  m_strRepMap[QChar(0xc3)] = "A";
  m_strRepMap[QChar(0xc5)] = "A";
  m_strRepMap[QChar(0xc6)] = "AE";
  m_strRepMap[QChar(0xc7)] = "C";
  m_strRepMap[QChar(0xc8)] = "E";
  m_strRepMap[QChar(0xc9)] = "E";
  m_strRepMap[QChar(0xca)] = "E";
  m_strRepMap[QChar(0xcb)] = "E";
  m_strRepMap[QChar(0xcc)] = "I";
  m_strRepMap[QChar(0xcd)] = "I";
  m_strRepMap[QChar(0xce)] = "I";
  m_strRepMap[QChar(0xcf)] = "I";
  m_strRepMap[QChar(0xd0)] = "D";
  m_strRepMap[QChar(0xd1)] = "N";
  m_strRepMap[QChar(0xd2)] = "O";
  m_strRepMap[QChar(0xd3)] = "O";
  m_strRepMap[QChar(0xd4)] = "O";
  m_strRepMap[QChar(0xd5)] = "O";
  m_strRepMap[QChar(0xd7)] = "x";
  m_strRepMap[QChar(0xd8)] = "O";
  m_strRepMap[QChar(0xd9)] = "U";
  m_strRepMap[QChar(0xda)] = "U";
  m_strRepMap[QChar(0xdb)] = "U";
  m_strRepMap[QChar(0xdd)] = "Y";
  m_strRepMap[QChar(0xe0)] = "a";
  m_strRepMap[QChar(0xe1)] = "a";
  m_strRepMap[QChar(0xe2)] = "a";
  m_strRepMap[QChar(0xe3)] = "a";
  m_strRepMap[QChar(0xe5)] = "a";
  m_strRepMap[QChar(0xe6)] = "ae";
  m_strRepMap[QChar(0xe7)] = "c";
  m_strRepMap[QChar(0xe8)] = "e";
  m_strRepMap[QChar(0xe9)] = "e";
  m_strRepMap[QChar(0xea)] = "e";
  m_strRepMap[QChar(0xeb)] = "e";
  m_strRepMap[QChar(0xec)] = "i";
  m_strRepMap[QChar(0xed)] = "i";
  m_strRepMap[QChar(0xee)] = "i";
  m_strRepMap[QChar(0xef)] = "i";
  m_strRepMap[QChar(0xf0)] = "d";
  m_strRepMap[QChar(0xf1)] = "n";
  m_strRepMap[QChar(0xf2)] = "o";
  m_strRepMap[QChar(0xf3)] = "o";
  m_strRepMap[QChar(0xf4)] = "o";
  m_strRepMap[QChar(0xf5)] = "o";
  m_strRepMap[QChar(0xf8)] = "o";
  m_strRepMap[QChar(0xf9)] = "u";
  m_strRepMap[QChar(0xfa)] = "u";
  m_strRepMap[QChar(0xfb)] = "u";
  m_strRepMap[QChar(0xfd)] = "y";
  m_strRepMap[QChar(0xff)] = "y";
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
    dotPos = str.lastIndexOf('.');
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
        QString newstr;
        bool wordstart = true;
        for (unsigned i = 0; i < static_cast<unsigned>(str.length()); ++i) {
          QChar ch = str.at(i);
          if (!ch.isLetterOrNumber() &&
            ch != '\'' && ch != '`') {
            wordstart = true;
            newstr.append(ch);
          } else if (wordstart) {
            wordstart = false;
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
    QMap<QString, QString>::ConstIterator it;
    for (it = m_strRepMap.begin(); it != m_strRepMap.end(); ++it) {
      str.replace(it.key(), *it);
    }
  }
  /* append extension if it was removed */
  if (dotPos != -1) {
    str.append(ext);
  }
}

/** Returns a lowercase copy of @a str. */
QString FormatConfig::toLower(const QString& str) const
{
#if QT_VERSION >= 0x040800
  if (m_useSystemLocale)
    return m_locale.toLower(str);
#endif
  return str.toLower();
}

/** Returns an uppercase copy of @a str. */
QString FormatConfig::toUpper(const QString& str) const
{
#if QT_VERSION >= 0x040800
  if (m_useSystemLocale)
    return m_locale.toUpper(str);
#endif
  return str.toUpper();
}

/**
 * Format frames using this configuration.
 *
 * @param frames frames
 */
void FormatConfig::formatFrames(FrameCollection& frames) const
{
  for (FrameCollection::iterator it = frames.begin();
       it != frames.end();
       ++it) {
    Frame& frame = const_cast<Frame&>(*it);
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
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void FormatConfig::writeToConfig(Kid3Settings* config) const
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  cfg.writeEntry("FormatWhileEditing", m_formatWhileEditing);
  cfg.writeEntry("CaseConversion", static_cast<int>(m_caseConversion));
  cfg.writeEntry("UseSystemLocale", m_useSystemLocale);
  cfg.writeEntry("StrRepEnabled", m_strRepEnabled);
  cfg.writeEntry("StrRepMapKeys", m_strRepMap.keys());
  cfg.writeEntry("StrRepMapValues", m_strRepMap.values());
#else
  config->beginGroup("/" + m_group);
  config->setValue("/FormatWhileEditing", QVariant(m_formatWhileEditing));
  config->setValue("/CaseConversion", QVariant(m_caseConversion));
  config->setValue("/UseSystemLocale", QVariant(m_useSystemLocale));
  config->setValue("/StrRepEnabled", QVariant(m_strRepEnabled));
  config->setValue("/StrRepMapKeys", QVariant(m_strRepMap.keys()));
  config->setValue("/StrRepMapValues", QVariant(m_strRepMap.values()));
  config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void FormatConfig::readFromConfig(Kid3Settings* config)
{
#ifdef CONFIG_USE_KDE
  KConfigGroup cfg = config->group(m_group);
  m_formatWhileEditing = cfg.readEntry("FormatWhileEditing", m_formatWhileEditing);
  m_caseConversion = (CaseConversion)cfg.readEntry("CaseConversion",
                              (int)m_caseConversion);
  m_useSystemLocale = cfg.readEntry("UseSystemLocale", m_useSystemLocale);
  m_strRepEnabled = cfg.readEntry("StrRepEnabled", m_strRepEnabled);
  QStringList keys = cfg.readEntry("StrRepMapKeys", QStringList());
  QStringList values = cfg.readEntry("StrRepMapValues", QStringList());
  if (!keys.empty() && !values.empty()) {
    QStringList::Iterator itk, itv;
    m_strRepMap.clear();
    for (itk = keys.begin(), itv = values.begin();
       itk != keys.end() && itv != values.end();
       ++itk, ++itv) {
      m_strRepMap[*itk] = *itv;
    }
  }
#else
  config->beginGroup("/" + m_group);
  m_formatWhileEditing = config->value("/FormatWhileEditing", m_formatWhileEditing).toBool();
  m_caseConversion = (CaseConversion)config->value("/CaseConversion",
                                                   (int)m_caseConversion).toInt();
  m_useSystemLocale = config->value("/UseSystemLocale", m_useSystemLocale).toBool();
  m_strRepEnabled = config->value("/StrRepEnabled", m_strRepEnabled).toBool();
  QStringList keys = config->value("/StrRepMapKeys").toStringList();
  QStringList values = config->value("/StrRepMapValues").toStringList();
  if (!keys.empty() && !values.empty()) {
    QStringList::Iterator itk, itv;
    m_strRepMap.clear();
    for (itk = keys.begin(), itv = values.begin();
       itk != keys.end() && itv != values.end();
       ++itk, ++itv) {
      m_strRepMap[*itk] = *itv;
    }
  }
  config->endGroup();
#endif
}
