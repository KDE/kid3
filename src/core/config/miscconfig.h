/**
 * \file miscconfig.h
 * Miscellaneous Configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Jan 2004
 *
 * Copyright (C) 2004-2012  Urs Fleisch
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

#ifndef MISCCONFIG_H
#define MISCCONFIG_H

#include <QStringList>
#include <QList>
#include "config.h"
#include "generalconfig.h"
#include "trackdata.h"
#include "kid3api.h"

class QString;

/**
 * Miscellaneous Configuration.
 */
class KID3_CORE_EXPORT MiscConfig : public GeneralConfig
{
public:
  /** The ID3v2 version used for new tags. */
  enum Id3v2Version {
    ID3v2_3_0 = 0,
    ID3v2_4_0 = 1,
    ID3v2_3_0_TAGLIB = 2
  };

  /** Encoding used for ID3v2 frames. */
  enum TextEncoding {
    TE_ISO8859_1,
    TE_UTF16,
    TE_UTF8
  };

  /** Name for Vorbis picture. */
  enum VorbisPictureName {
    VP_METADATA_BLOCK_PICTURE,
    VP_COVERART
  };

  /**
   * External command in context menu.
   */
  class MenuCommand {
  public:
    /**
     * Constructor.
     *
     * @param name display name
     * @param cmd  command string with argument codes
     * @param confirm true if confirmation required
     * @param showOutput true if output of command shall be shown
     */
    explicit MenuCommand(const QString& name = QString::null,
                         const QString& cmd = QString::null,
                         bool confirm = false, bool showOutput = false);

    /**
     * Constructor.
     *
     * @param strList string list with encoded command
     */
    explicit MenuCommand(const QStringList& strList);

    /**
     * Encode into string list.
     *
     * @return string list with encoded command.
     */
    QStringList toStringList() const;

    /**
     * Get the display name.
     * @return name.
     */
    const QString& getName() const { return m_name; }

    /**
     * Set the display name.
     * @param name display name
     */
    void setName(const QString& name) { m_name = name; }

    /**
     * Get the command string.
     * @return command string.
     */
    const QString& getCommand() const { return m_cmd; }

    /**
     * Set the command string.
     * @param cmd command string.
     */
    void setCommand(const QString& cmd) { m_cmd = cmd; }

    /**
     * Check if command must be confirmed.
     * @return true if command has to be confirmed.
     */
    bool mustBeConfirmed() const { return m_confirm; }

    /**
     * Set if command must be confirmed.
     * @param confirm true if command has to be confirmed
     */
    void setMustBeConfirmed(bool confirm) { m_confirm = confirm; }

    /**
     * Check if command output has to be shown.
     * @return true if command output has to be shown.
     */
    bool outputShown() const { return m_showOutput; }

    /**
     * Set if command output has to be shown.
     * @param showOutput true if command output has to be shown
     */
    void setOutputShown(bool showOutput) { m_showOutput = showOutput; }

  private:
    QString m_name;
    QString m_cmd;
    bool m_confirm;
    bool m_showOutput;
  };

  /**
   * Constructor.
   */
  MiscConfig(const QString& group);

  /**
   * Destructor.
   */
  virtual ~MiscConfig();

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  void writeToConfig(Kid3Settings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  void readFromConfig(Kid3Settings* config);

  /**
   * Get file pattern part of m_nameFilter.
   * @return file patterns, e.g. "*.mp3".
   */
  QString getNameFilterPatterns() const;

  /** true to mark truncated ID3v1.1 fields */
  bool m_markTruncations;
  /** true to write total number of tracks into track fields */
  bool m_enableTotalNumberOfTracks;
  /** true to write genres as text instead of numeric string */
  bool m_genreNotNumeric;
  /** true to preserve file time stamps */
  bool m_preserveTime;
  /** true to mark changed fields */
  bool m_markChanges;
  /** field name used for Vorbis comment entries */
  QString m_commentName;
  /** index of field name used for Vorbis picture entries */
  int m_pictureNameItem;
  /** filter of file names to be opened */
  QString m_nameFilter;
  /** filename format */
  QString m_formatText;
  /** index of filename format selected */
  int m_formatItem;
  /** filename formats */
  QStringList m_formatItems;
  /** from filename format */
  QString m_formatFromFilenameText;
  /** index of from filename format selected */
  int m_formatFromFilenameItem;
  /** from filename formats */
  QStringList m_formatFromFilenameItems;
  /** directory name format */
  QString m_dirFormatText;
  /** index of directory name format selected */
  int m_dirFormatItem;
  /** rename directory from tags 1, tags 2, or both */
  TrackData::TagVersion m_renDirSrc;
  /** number tracks in tags 1, tags 2, or both */
  TrackData::TagVersion m_numberTracksDst;
  /** number tracks start number */
  int m_numberTracksStart;
  /** size of splitter in main window */
  QList<int> m_splitterSizes;
  /** size of file/dirlist splitter */
  QList<int> m_vSplitterSizes;
  /** commands available in context menu */
  QList<MenuCommand> m_contextMenuCommands;
  /** custom genres for ID3v2.3 */
  QStringList m_customGenres;
#ifndef CONFIG_USE_KDE
  /** true to hide toolbar */
  bool m_hideToolBar;
  /** true to hide statusbar */
  bool m_hideStatusBar;
#endif
  /** true to automatically hide unused tags */
  bool m_autoHideTags;
  /** true to hide file controls */
  bool m_hideFile;
  /** true to hide ID3v1.1 controls */
  bool m_hideV1;
  /** true to hide ID3v2.3 controls */
  bool m_hideV2;
  /** true to hide picture preview */
  bool m_hidePicture;
  /** version used for new ID3v2 tags */
  int m_id3v2Version;
  /** text encoding used for new ID3v1 tags */
  QString m_textEncodingV1;
  /** text encoding used for new ID3v2 tags */
  int m_textEncoding;
  /** frames which are displayed for Tag 2 even if not present */
  quint64 m_quickAccessFrames;
  /** number of digits in track number */
  int m_trackNumberDigits;
  /** true to play file on double click */
  bool m_playOnDoubleClick;
  /** true if proxy is used */
  bool m_useProxy;
  /** proxy used for access */
  QString m_proxy;
  /** true to use proxy authentication */
  bool m_useProxyAuthentication;
  /** proxy user name */
  QString m_proxyUserName;
  /** proxy password */
  QString m_proxyPassword;
  /** web browser substituted for %b */
  QString m_browser;
  /** true to show only custom genres in combo boxes */
  bool m_onlyCustomGenres;
  /** true to open last opened file on startup */
  bool m_loadLastOpenedFile;
  /** path to last opened file */
  QString m_lastOpenedFile;
  /** default file name to save cover art */
  QString m_defaultCoverFileName;
#ifndef CONFIG_USE_KDE
  /** mainwindow geometry */
  QByteArray m_geometry;
  /** mainwindow state */
  QByteArray m_windowState;
  /** true if custom application font is used */
  bool m_useFont;
  /** custom application font family */
  QString m_fontFamily;
  /** custom application font size */
  int m_fontSize;
  /** custom application style, empty if not used */
  QString m_style;
  /** Don't use the native file dialog if true */
  bool m_dontUseNativeDialogs;
#endif

  /** Default directory format list */
  static const char** s_defaultDirFmtList;
};

#endif
