/**
 * \file formatconfig.h
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

#ifndef FORMATCONFIG_H
#define FORMATCONFIG_H

#include "config.h"
#include "generalconfig.h"
#include <QMap>
#include "kid3api.h"

class QString;
class QLocale;
class FrameCollection;

/**
 * Format configuration.
 */
class KID3_CORE_EXPORT FormatConfig : public GeneralConfig
{
public:
  /** Case conversion options. */
  enum CaseConversion {
      NoChanges,
      AllLowercase,
      AllUppercase,
      FirstLetterUppercase,
      AllFirstLettersUppercase,
      NumCaseConversions
  };

  /**
   * Constructor.
   *
   * @param grp configuration group
   */
  FormatConfig(const QString& grp);

  /**
   * Destructor.
   */
  virtual ~FormatConfig();

  /**
   * Set specific properties for a filename format.
   * This will set default string conversions and not touch the file
   * extension when formatting.
   */
  void setAsFilenameFormatter();

  /**
   * Format a string using this configuration.
   *
   * @param str string to format
   */
  void formatString(QString& str) const;

  /**
   * Format frames using this configuration.
   *
   * @param frames frames
   */
  void formatFrames(FrameCollection& frames) const;

  /**
   * Format frames if format while editing is switched on.
   *
   * @param frames frames
   */
  void formatFramesIfEnabled(FrameCollection& frames) const;

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  virtual void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(ISettings* config);

  /**
   * Get name of locale to use for string conversions.
   * @return locale name
   */
  QString getLocaleName() const { return m_localeName; }

  /**
   * Set name of locale to use for string conversions.
   * @param localeName locale name
   */
  void setLocaleName(const QString& localeName);

  /** true to enable formating in line edits */
  bool m_formatWhileEditing;
  /** Case conversion option */
  CaseConversion m_caseConversion;
  /** true if string replacement enabled */
  bool m_strRepEnabled;
  /** Mapping for string replacement */
  QMap<QString, QString> m_strRepMap;

private:
  /** Locale to use for case conversion */
  QString m_localeName;

  /** Returns a lowercase copy of @a str. */
  QString toLower(const QString& str) const;

  /** Returns an uppercase copy of @a str. */
  QString toUpper(const QString& str) const;

  /** Locale to use for string conversions */
  const QLocale* m_locale;

  /** true if it is a file formatter */
  bool m_filenameFormatter;
};

#endif
