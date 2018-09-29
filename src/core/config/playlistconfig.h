/**
 * \file playlistconfig.h
 * Configuration for playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Sep 2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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

#ifndef PLAYLISTCONFIG_H
#define PLAYLISTCONFIG_H

#include <QString>
#include "config.h"
#include "generalconfig.h"
#include "kid3api.h"

/**
 * Playlist configuration.
 */
class KID3_CORE_EXPORT PlaylistConfig : public StoredConfig<PlaylistConfig> {
  Q_OBJECT
  /** Playlist location */
  Q_PROPERTY(int location READ location WRITE setLocationInt NOTIFY locationChanged)
  /** Playlist format */
  Q_PROPERTY(int format READ format WRITE setFormatInt NOTIFY formatChanged)
  /** Playlist file name format */
  Q_PROPERTY(QString fileNameFormat READ fileNameFormat WRITE setFileNameFormat NOTIFY fileNameFormatChanged)
  /** Tag field used for sorting */
  Q_PROPERTY(QString sortTagField READ sortTagField WRITE setSortTagField NOTIFY sortTagFieldChanged)
  /** Format for additional information */
  Q_PROPERTY(QString infoFormat READ infoFormat WRITE setInfoFormat NOTIFY infoFormatChanged)
  /** Use file name format if true, else directory name */
  Q_PROPERTY(bool useFileNameFormat READ useFileNameFormat WRITE setUseFileNameFormat NOTIFY useFileNameFormatChanged)
  /** Include only selected files if true, else all files */
  Q_PROPERTY(bool onlySelectedFiles READ onlySelectedFiles WRITE setOnlySelectedFiles NOTIFY onlySelectedFilesChanged)
  /** Sort by tag field if true, else file name */
  Q_PROPERTY(bool useSortTagField READ useSortTagField WRITE setUseSortTagField NOTIFY useSortTagFieldChanged)
  /** Use full path for files in playlist if true, else relative path */
  Q_PROPERTY(bool useFullPath READ useFullPath WRITE setUseFullPath NOTIFY useFullPathChanged)
  /** Write info format, else only list of files */
  Q_PROPERTY(bool writeInfo READ writeInfo WRITE setWriteInfo NOTIFY writeInfoChanged)
  Q_ENUMS(PlaylistFormat)
  Q_ENUMS(PlaylistLocation)
public:
  /**
   * Playlist format.
   */
  enum PlaylistFormat {
    PF_M3U, /**< M3U */
    PF_PLS, /**< PLS */
    PF_XSPF /**< XSPF */
  };

  /**
   * Location to create playlist.
   */
  enum PlaylistLocation {
    PL_CurrentDirectory, /**< create in current directory */
    PL_EveryDirectory,   /**< create in every directory */
    PL_TopLevelDirectory /**< create in top-level directory */
  };

  /**
   * Constructor.
   */
  explicit PlaylistConfig();

  /**
   * Copy constructor.
   * @param other instance to be copied
   */
  PlaylistConfig(const PlaylistConfig& other);

  /**
   * Destructor.
   */
  virtual ~PlaylistConfig() override;

  /**
   * Assignment operator.
   * @param other instance to be copied
   * @return reference to this instance.
   */
  PlaylistConfig& operator=(const PlaylistConfig& other);

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

  /** Get playlist location. */
  PlaylistLocation location() const { return m_location; }

  /** Set playlist location. */
  void setLocation(PlaylistLocation location);

  /** Get playlist format. */
  PlaylistFormat format() const { return m_format;}

  /** Set playlist format. */
  void setFormat(PlaylistFormat format);

  /** Get playlist file name format. */
  QString fileNameFormat() const { return m_fileNameFormat; }

  /** Set playlist file name format. */
  void setFileNameFormat(const QString& fileNameFormat);

  /** Get tag field used for sorting. */
  QString sortTagField() const { return m_sortTagField; }

  /** Set tag field used for sorting. */
  void setSortTagField(const QString& sortTagField);

  /** Get format for additional information. */
  QString infoFormat() const { return m_infoFormat; }

  /** Set format for additional information. */
  void setInfoFormat(const QString& infoFormat);

  /** Check if file name format is used. */
  bool useFileNameFormat() const { return m_useFileNameFormat; }

  /** Set if file name format is used. */
  void setUseFileNameFormat(bool useFileNameFormat);

  /** Check if only selected files are included. */
  bool onlySelectedFiles() const { return m_onlySelectedFiles; }

  /** Set if only selected files are included. */
  void setOnlySelectedFiles(bool onlySelectedFiles);

  /** Check if sorted by tag field. */
  bool useSortTagField() const { return m_useSortTagField; }

  /** Set if sorted by tag field. */
  void setUseSortTagField(bool useSortTagField);

  /** Check if full path for files is used in playlist. */
  bool useFullPath() const { return m_useFullPath; }

  /** Set if full path for files is used in playlist. */
  void setUseFullPath(bool useFullPath);

  /** Check if info format is written. */
  bool writeInfo() const { return m_writeInfo; }

  /** Set if info format is written. */
  void setWriteInfo(bool writeInfo);

  /**
   * Get file extension for playlist format.
   * @return ".m3u", ".pls" or ".xspf".
   */
  QString fileExtensionForFormat() const;

  /**
   * Get playlist format from file extension.
   * @param path file path or name ending with extension
   * @param ok if set true is returned here if @a path has a playlist extension
   * @return playlist format.
   */
  static PlaylistFormat formatFromFileExtension(const QString& path,
                                                bool* ok = nullptr);

signals:
  /** Emitted when @a location changed. */
  void locationChanged(PlaylistLocation location);

  /** Emitted when @a format changed. */
  void formatChanged(PlaylistFormat format);

  /** Emitted when @a fileNameFormat changed. */
  void fileNameFormatChanged(const QString& fileNameFormat);

  /** Emitted when @a sortTagField changed. */
  void sortTagFieldChanged(const QString& sortTagField);

  /** Emitted when @a infoFormat changed. */
  void infoFormatChanged(const QString& infoFormat);

  /** Emitted when @a useFileNameFormat changed. */
  void useFileNameFormatChanged(bool useFileNameFormat);

  /** Emitted when @a onlySelectedFiles changed. */
  void onlySelectedFilesChanged(bool onlySelectedFiles);

  /** Emitted when @a useSortTagField changed. */
  void useSortTagFieldChanged(bool useSortTagField);

  /** Emitted when @a useFullPath changed. */
  void useFullPathChanged(bool useFullPath);

  /** Emitted when @a writeInfo changed. */
  void writeInfoChanged(bool writeInfo);

private:
  friend PlaylistConfig& StoredConfig<PlaylistConfig>::instance();

  void setLocationInt(int location) {
    setLocation(static_cast<PlaylistLocation>(location));
  }

  void setFormatInt(int format) {
    setFormat(static_cast<PlaylistFormat>(format));
  }

  PlaylistLocation m_location;
  PlaylistFormat m_format;
  QString m_fileNameFormat;
  QString m_sortTagField;
  QString m_infoFormat;
  bool m_useFileNameFormat;
  bool m_onlySelectedFiles;
  bool m_useSortTagField;
  bool m_useFullPath;
  bool m_writeInfo;

  /** Index in configuration storage */
  static int s_index;
};

#endif // PLAYLISTCONFIG_H
