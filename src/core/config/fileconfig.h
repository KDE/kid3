/**
 * \file fileconfig.h
 * File related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#include <QStringList>
#include "generalconfig.h"
#include "kid3api.h"

/**
 * File related configuration.
 */
class KID3_CORE_EXPORT FileConfig : public StoredConfig<FileConfig> {
  Q_OBJECT
  /** filter of file names to be opened */
  Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter
             NOTIFY nameFilterChanged)
  /** patterns for folders to include in file list */
  Q_PROPERTY(QStringList includeFolders READ includeFolders
             WRITE setIncludeFolders NOTIFY includeFoldersChanged)
  /** patterns for folders to exclude in file list */
  Q_PROPERTY(QStringList excludeFolders READ excludeFolders
             WRITE setExcludeFolders NOTIFY excludeFoldersChanged)
  /** true to show hidden files */
  Q_PROPERTY(bool showHiddenFiles READ showHiddenFiles WRITE setShowHiddenFiles
             NOTIFY showHiddenFilesChanged)
  /** true if punctuation characters and symbols are ignored when sorting */
  Q_PROPERTY(bool sortIgnoringPunctuation READ sortIgnoringPunctuation
             WRITE setSortIgnoringPunctuation
             NOTIFY sortIgnoringPunctuationChanged)
  /** filename format */
  Q_PROPERTY(QString toFilenameFormat READ toFilenameFormat
             WRITE setToFilenameFormat NOTIFY toFilenameFormatChanged)
  /** filename formats */
  Q_PROPERTY(QStringList toFilenameFormats READ toFilenameFormats
             WRITE setToFilenameFormats NOTIFY toFilenameFormatsChanged)
  /** from filename format */
  Q_PROPERTY(QString fromFilenameFormat READ fromFilenameFormat
             WRITE setFromFilenameFormat NOTIFY fromFilenameFormatChanged)
  /** from filename formats */
  Q_PROPERTY(QStringList fromFilenameFormats READ fromFilenameFormats
             WRITE setFromFilenameFormats NOTIFY fromFilenameFormatsChanged)
  /** default file name to save cover art */
  Q_PROPERTY(QString defaultCoverFileName READ defaultCoverFileName
             WRITE setDefaultCoverFileName NOTIFY defaultCoverFileNameChanged)
  /** path to last opened file */
  Q_PROPERTY(QString lastOpenedFile READ lastOpenedFile WRITE setLastOpenedFile
             NOTIFY lastOpenedFileChanged)
  /** text encoding used for exports and playlists */
  Q_PROPERTY(QString textEncoding READ textEncoding WRITE setTextEncoding
             NOTIFY textEncodingChanged)
  /** text encoding used for exports and playlists */
  Q_PROPERTY(int textEncodingIndex READ textEncodingIndex
             WRITE setTextEncodingIndex NOTIFY textEncodingChanged)
  /** true to preserve file time stamps */
  Q_PROPERTY(bool preserveTime READ preserveTime WRITE setPreserveTime
             NOTIFY preserveTimeChanged)
  /** true to mark changed fields */
  Q_PROPERTY(bool markChanges READ markChanges WRITE setMarkChanges
             NOTIFY markChangesChanged)
  /** true to open last opened file on startup */
  Q_PROPERTY(bool loadLastOpenedFile READ loadLastOpenedFile
             WRITE setLoadLastOpenedFile NOTIFY loadLastOpenedFileChanged)

public:
  /**
   * Constructor.
   */
  FileConfig();

  /**
   * Destructor.
   */
  ~FileConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  void readFromConfig(ISettings* config) override;

  /** Get filter of file names to be opened. */
  QString nameFilter() const { return m_nameFilter; }

  /** Set filter of file names to be opened. */
  void setNameFilter(const QString& nameFilter);

  /** Get patterns for folders to include in file list. */
  QStringList includeFolders() const { return m_includeFolders; }

  /** Set patterns for folders to include in file list. */
  void setIncludeFolders(const QStringList& includeFolders);

  /** Get patterns for folders to exclude in file list. */
  QStringList excludeFolders() const { return m_excludeFolders; }

  /** Set patterns for folders to exclude in file list. */
  void setExcludeFolders(const QStringList& excludeFolders);

  /** Check if hidden files are shown. */
  bool showHiddenFiles() const { return m_showHiddenFiles; }

  /** Set if hidden files are shown. */
  void setShowHiddenFiles(bool showHiddenFiles);

  /** Check if punctuation characters and symbols are ignored when sorting. */
  bool sortIgnoringPunctuation() const { return m_sortIgnoringPunctuation; }

  /** Set if punctuation characters and symbols are ignored when sorting. */
  void setSortIgnoringPunctuation(bool sortIgnoringPunctuation);

  /** Get filename format. */
  QString toFilenameFormat() const { return m_formatText; }

  /** Set filename format. */
  void setToFilenameFormat(const QString& formatText);

  /** Get filename formats. */
  QStringList toFilenameFormats() const { return m_formatItems; }

  /** Set filename formats. */
  void setToFilenameFormats(const QStringList& formatItems);

  /** Get from filename format. */
  QString fromFilenameFormat() const { return m_formatFromFilenameText; }

  /** Set from filename format. */
  void setFromFilenameFormat(const QString& formatFromFilenameText);

  /** Get from filename formats. */
  QStringList fromFilenameFormats() const {
    return m_formatFromFilenameItems;
  }

  /** Set from filename formats. */
  void setFromFilenameFormats(const QStringList& fromFilenameFormats);

  /** Get default file name to save cover art. */
  QString defaultCoverFileName() const { return m_defaultCoverFileName; }

  /** Set default file name to save cover art. */
  void setDefaultCoverFileName(const QString& defaultCoverFileName);

  /** Get path to last opened file. */
  QString lastOpenedFile() const { return m_lastOpenedFile; }

  /** Set path to last opened file. */
  void setLastOpenedFile(const QString& lastOpenedFile);

  /** Get text encoding used for exports and playlists. */
  QString textEncoding() const { return m_textEncoding; }

  /** Set text encoding used for exports and playlists. */
  void setTextEncoding(const QString& textEncoding);

  /** Get index of text encoding in getTextCodecNames() */
  int textEncodingIndex() const;

  /** Set text encoding from index in getTextCodecNames(). */
  void setTextEncodingIndex(int index);

  /** Check if file time stamps are preserved. */
  bool preserveTime() const { return m_preserveTime; }

  /** Set if file time stamps are preserved. */
  void setPreserveTime(bool preserveTime);

  /** Check if changed fields are marked. */
  bool markChanges() const { return m_markChanges; }

  /** Set if changed fields are marked. */
  void setMarkChanges(bool markChanges);

  /** Check if the last opened file is loaded on startup. */
  bool loadLastOpenedFile() const { return m_loadLastOpenedFile; }

  /** Set if the last opened file is loaded on startup. */
  void setLoadLastOpenedFile(bool loadLastOpenedFile);

signals:
  /** Emitted when @a nameFilter changed. */
  void nameFilterChanged(const QString& nameFilter);

  /** Emitted when @a includeFolders changed. */
  void includeFoldersChanged(const QStringList& includeFolders);

  /** Emitted when @a excludeFolders changed. */
  void excludeFoldersChanged(const QStringList& excludeFolders);

  /** Emitted when @a showHiddenFiles changed. */
  void showHiddenFilesChanged(bool showHiddenFiles);

  /** Emitted when @a sortIgnoringPunctuation changed. */
  void sortIgnoringPunctuationChanged(bool sortIgnoringPunctuation);

  /** Emitted when @a formatText changed. */
  void toFilenameFormatChanged(const QString& toFilenameFormat);

  /** Emitted when @a formatItems changed. */
  void toFilenameFormatsChanged(const QStringList& toFilenameFormats);

  /** Emitted when @a formatFromFilenameText changed. */
  void fromFilenameFormatChanged(const QString& fromFilenameFormat);

  /** Emitted when @a formatFromFilenameItems changed. */
  void fromFilenameFormatsChanged(const QStringList& fromFilenameFormats);

  /** Emitted when @a defaultCoverFileName changed. */
  void defaultCoverFileNameChanged(const QString& defaultCoverFileName);

  /** Emitted when @a lastOpenedFile changed. */
  void lastOpenedFileChanged(const QString& lastOpenedFile);

  /** Emitted when @a textEncoding changed. */
  void textEncodingChanged(const QString& textEncoding);

  /** Emitted when @a preserveTime changed. */
  void preserveTimeChanged(bool preserveTime);

  /** Emitted when @a markChanges changed. */
  void markChangesChanged(bool markChanges);

  /** Emitted when @a loadLastOpenedFile changed. */
  void loadLastOpenedFileChanged(bool loadLastOpenedFile);

private:
  friend FileConfig& StoredConfig<FileConfig>::instance();

  void initFormatListsIfEmpty();

  QString m_nameFilter;
  QStringList m_includeFolders;
  QStringList m_excludeFolders;
  QString m_formatText;
  QStringList m_formatItems;
  QString m_formatFromFilenameText;
  QStringList m_formatFromFilenameItems;
  QString m_defaultCoverFileName;
  QString m_lastOpenedFile;
  QString m_textEncoding;
  bool m_preserveTime;
  bool m_markChanges;
  bool m_loadLastOpenedFile;
  bool m_showHiddenFiles;
  bool m_sortIgnoringPunctuation;

  /** Index in configuration storage */
  static int s_index;
};
