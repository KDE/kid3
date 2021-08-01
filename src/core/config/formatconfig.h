/**
 * \file formatconfig.h
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

#pragma once

#include <QScopedPointer>
#include <QVariantMap>
#include <QStringList>
#include "generalconfig.h"
#include "kid3api.h"

class QString;
class QLocale;
class FrameCollection;

/**
 * Format configuration.
 */
class KID3_CORE_EXPORT FormatConfig : public GeneralConfig {
  Q_OBJECT
  /** Mapping for string replacement */
  Q_PROPERTY(QStringList strRepMap READ strRepStringList
             WRITE setStrRepStringList NOTIFY strRepMapChanged)
  /** Case conversion option */
  Q_PROPERTY(int caseConversion READ caseConversion WRITE setCaseConversionInt
             NOTIFY caseConversionChanged)
  /** name of locale to use for string conversions */
  Q_PROPERTY(QString localeName READ localeName WRITE setLocaleName
             NOTIFY localeNameChanged)
  /** maximum length */
  Q_PROPERTY(int maximumLength READ maximumLength WRITE setMaximumLength
             NOTIFY maximumLengthChanged)
  /** true to enable length restriction */
  Q_PROPERTY(bool enableMaximumLength READ enableMaximumLength
             WRITE setEnableMaximumLength NOTIFY enableMaximumLengthChanged)
  /** true to enable formating in line edits */
  Q_PROPERTY(bool formatWhileEditing READ formatWhileEditing
             WRITE setFormatWhileEditing NOTIFY formatWhileEditingChanged)
  /** true if string replacement enabled */
  Q_PROPERTY(bool strRepEnabled READ strRepEnabled WRITE setStrRepEnabled
             NOTIFY strRepEnabledChanged)
  /** true to enable data validation */
  Q_PROPERTY(bool enableValidation READ enableValidation
             WRITE setEnableValidation NOTIFY enableValidationChanged)
  /** true to use format for playlist and folder names */
  Q_PROPERTY(bool useForOtherFileNames READ useForOtherFileNames
             WRITE setUseForOtherFileNames NOTIFY useForOtherFileNamesChanged)
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
  virtual ~FormatConfig() override;

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
   * Join base name and extension respecting maximum length.
   *
   * Truncation to maximumLength() is only done if enableMaximumLength() and
   * setAsFilenameFormatter() are set.
   *
   * @param baseName file name without extension
   * @param extension file name extension starting with dot
   * @return file name with extension, eventually truncated to maximum length.
   */
  QString joinFileName(const QString& baseName, const QString& extension) const;

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
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(ISettings* config) override;

  /** Get mapping for string replacement. */
  QList<QPair<QString, QString>> strRepMap() const { return m_strRepMap; }

  /** Set mapping for string replacement. */
  void setStrRepMap(const QList<QPair<QString, QString>>& strRepMap);

  /** Get mapping for string replacement as a list with alternating key, values. */
  QStringList strRepStringList() const;

  /** Set mapping for string replacement from a list with alternating key, values. */
  void setStrRepStringList(const QStringList& lst);

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

  /** Check if format shall be used for playlist and folder names. */
  bool useForOtherFileNames() const { return m_useForOtherFileNames; }

  /** Set if format shall be used for playlist and folder names. */
  void setUseForOtherFileNames(bool useForOtherFileNames);

  /** Check if length restriction is enabled. */
  bool enableMaximumLength() const { return m_enableMaximumLength; }

  /** Set if length restriction is enabled. */
  void setEnableMaximumLength(bool enableMaximumLength);

  /** Get maximum length. */
  int maximumLength() const { return m_maximumLength; }

  /** Set maximum length. */
  void setMaximumLength(int maximumLength);

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
  void strRepMapChanged(const QList<QPair<QString, QString>>& strRepMap);

  /** Emitted when @a caseConversion changed. */
  void caseConversionChanged(FormatConfig::CaseConversion caseConversion);

  /** Emitted when @a localeName changed. */
  void localeNameChanged(const QString& localeName);

  /** Emitted when @a formatWhileEditing changed. */
  void formatWhileEditingChanged(bool formatWhileEditing);

  /** Emitted when @a strRepEnabled changed. */
  void strRepEnabledChanged(bool strRepEnabled);

  /** Emitted when @a enableValidation changed. */
  void enableValidationChanged(bool enableValidation);

  /** Emitted when @a useForOtherFileNames changed. */
  void useForOtherFileNamesChanged(bool useForOtherFileNames);

  /** Emitted when @a enableMaximumLength changed. */
  void enableMaximumLengthChanged(bool enableMaximumLength);

  /** Emitted when @a maximumLength changed. */
  void maximumLengthChanged(int maximumLength);

private:
  /** Returns a lowercase copy of @a str. */
  QString toLower(const QString& str) const;

  /** Returns an uppercase copy of @a str. */
  QString toUpper(const QString& str) const;

  void setCaseConversionInt(int caseConversion) {
    setCaseConversion(static_cast<CaseConversion>(caseConversion));
  }

  QList<QPair<QString, QString>> m_strRepMap;
  CaseConversion m_caseConversion;
  QString m_localeName;
  /** Locale to use for string conversions */
  QScopedPointer<const QLocale> m_locale;
  int m_maximumLength;
  bool m_useForOtherFileNames;
  bool m_enableMaximumLength;
  /** true if it is a file formatter */
  bool m_filenameFormatter;
  bool m_formatWhileEditing;
  bool m_strRepEnabled;
  bool m_enableValidation;
};


/**
 * FormatConfig subclass for stored filename format configuration instance.
 */
class KID3_CORE_EXPORT FilenameFormatConfig
    : public StoredConfig<FilenameFormatConfig, FormatConfig> {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  FilenameFormatConfig();

  /**
   * Destructor.
   */
  virtual ~FilenameFormatConfig() override = default;

private:
  friend FilenameFormatConfig&
  StoredConfig<FilenameFormatConfig, FormatConfig>::instance();

  /** Index in configuration storage */
  static int s_index;
};


/**
 * FormatConfig subclass for stored tag format configuration instance.
 */
class KID3_CORE_EXPORT TagFormatConfig
    : public StoredConfig<TagFormatConfig, FormatConfig> {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  TagFormatConfig();

  /**
   * Destructor.
   */
  virtual ~TagFormatConfig() override = default;

private:
  friend TagFormatConfig&
  StoredConfig<TagFormatConfig, FormatConfig>::instance();

  /** Index in configuration storage */
  static int s_index;
};
