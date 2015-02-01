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
#include <QStringList>
#include "kid3api.h"

class QString;
class QLocale;
class FrameCollection;

/**
 * Format configuration.
 */
class KID3_CORE_EXPORT FormatConfig : public GeneralConfig
{
  Q_OBJECT
  /** Mapping for string replacement */
  Q_PROPERTY(QVariantMap strRepMap READ strRepVariantMap WRITE setStrRepVariantMap NOTIFY strRepMapChanged)
  /** Case conversion option */
  Q_PROPERTY(int caseConversion READ caseConversion WRITE setCaseConversionInt NOTIFY caseConversionChanged)
  /** name of locale to use for string conversions */
  Q_PROPERTY(QString localeName READ localeName WRITE setLocaleName NOTIFY localeNameChanged)
  /** true to enable formating in line edits */
  Q_PROPERTY(bool formatWhileEditing READ formatWhileEditing WRITE setFormatWhileEditing NOTIFY formatWhileEditingChanged)
  /** true if string replacement enabled */
  Q_PROPERTY(bool strRepEnabled READ strRepEnabled WRITE setStrRepEnabled NOTIFY strRepEnabledChanged)
  /** true to enable data validation */
  Q_PROPERTY(bool enableValidation READ enableValidation WRITE setEnableValidation NOTIFY enableValidationChanged)
  Q_ENUMS(CaseConversion)
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
  explicit FormatConfig(const QString& grp);

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

  /** Get mapping for string replacement. */
  QMap<QString, QString> strRepMap() const { return m_strRepMap; }

  /** Set mapping for string replacement. */
  void setStrRepMap(const QMap<QString, QString>& strRepMap);

  /** Get mapping for string replacement as a variant map. */
  QVariantMap strRepVariantMap() const;

  /** Set mapping for string replacement from a variant map. */
  void setStrRepVariantMap(const QVariantMap& map);

  /** Get case conversion option. */
  CaseConversion caseConversion() const { return m_caseConversion; }

  /** Set case conversion option. */
  void setCaseConversion(CaseConversion caseConversion);

  /**
   * Get name of locale to use for string conversions.
   * @return locale name
   */
  QString localeName() const { return m_localeName; }

  /**
   * Set name of locale to use for string conversions.
   * @param localeName locale name
   */
  void setLocaleName(const QString& localeName);

  /** Check if formatting in line edits is enabled. */
  bool formatWhileEditing() const { return m_formatWhileEditing; }

  /** Set if formatting in line edits is enabled. */
  void setFormatWhileEditing(bool formatWhileEditing);

  /** Check if string replacement is enabled. */
  bool strRepEnabled() const { return m_strRepEnabled; }

  /** Set if string replacement is enabled. */
  void setStrRepEnabled(bool strRepEnabled);

  /** Check if data validation is enabled. */
  bool enableValidation() const { return m_enableValidation; }

  /** Set if data validation is enabled. */
  void setEnableValidation(bool enableValidation);

  /**
   * String list of case conversion names.
   */
  Q_INVOKABLE static QStringList getCaseConversionNames();

  /**
   * String list of locale names.
   */
  Q_INVOKABLE static QStringList getLocaleNames();

signals:
  /** Emitted when @a strRepMap changed. */
  void strRepMapChanged(const QMap<QString, QString>& strRepMap);

  /** Emitted when @a caseConversion changed. */
  void caseConversionChanged(CaseConversion caseConversion);

  /** Emitted when @a localeName changed. */
  void localeNameChanged(const QString& localeName);

  /** Emitted when @a formatWhileEditing changed. */
  void formatWhileEditingChanged(bool formatWhileEditing);

  /** Emitted when @a strRepEnabled changed. */
  void strRepEnabledChanged(bool strRepEnabled);

  /** Emitted when @a enableValidation changed. */
  void enableValidationChanged(bool enableValidation);

private:
  /** Returns a lowercase copy of @a str. */
  QString toLower(const QString& str) const;

  /** Returns an uppercase copy of @a str. */
  QString toUpper(const QString& str) const;

  void setCaseConversionInt(int caseConversion) {
    setCaseConversion(static_cast<CaseConversion>(caseConversion));
  }

  QMap<QString, QString> m_strRepMap;
  CaseConversion m_caseConversion;
  QString m_localeName;
  /** Locale to use for string conversions */
  const QLocale* m_locale;
  /** true if it is a file formatter */
  bool m_filenameFormatter;
  bool m_formatWhileEditing;
  bool m_strRepEnabled;
  bool m_enableValidation;
};


/**
 * FormatConfig subclass for stored filename format configuration instance.
 */
class KID3_CORE_EXPORT FilenameFormatConfig :
    public StoredConfig<FilenameFormatConfig, FormatConfig> {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  FilenameFormatConfig();

  /**
   * Destructor.
   */
  virtual ~FilenameFormatConfig();

private:
  friend FilenameFormatConfig&
  StoredConfig<FilenameFormatConfig, FormatConfig>::instance();

  /** Index in configuration storage */
  static int s_index;
};


/**
 * FormatConfig subclass for stored tag format configuration instance.
 */
class KID3_CORE_EXPORT TagFormatConfig :
    public StoredConfig<TagFormatConfig, FormatConfig> {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  TagFormatConfig();

  /**
   * Destructor.
   */
  virtual ~TagFormatConfig();

private:
  friend TagFormatConfig&
  StoredConfig<TagFormatConfig, FormatConfig>::instance();

  /** Index in configuration storage */
  static int s_index;
};

#endif
