/**
 * \file tagconfig.h
 * Tag related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef TAGCONFIG_H
#define TAGCONFIG_H

#include <QStringList>
#include "generalconfig.h"
#include "kid3api.h"

/**
 * Tag related configuration.
 */
class KID3_CORE_EXPORT TagConfig : public StoredConfig<TagConfig>
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

  /** Available tag formats which can be queried at run time. */
  enum TagFormatFlag {
    TF_ID3v2_3_0_ID3LIB = 1 << 0,
    TF_ID3v2_3_0_TAGLIB = 1 << 1,
    TF_ID3v2_4_0_TAGLIB = 1 << 2,
    TF_VORBIS_LIBOGG    = 1 << 3
  };

  /**
   * Constructor.
   */
  TagConfig();

  /**
   * Destructor.
   */
  virtual ~TagConfig();

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config);

  /**
   * Check if a certain tag format is available.
   * @param fmt tag format
   * @return true if tag format is available.
   * @remark This information is not stored in the configuration, it is
   * registered at initialization time using setTagFormat().
   */
  bool hasTagFormat(TagFormatFlag fmt) const {
    return (m_tagFormats & fmt) != 0;
  }

  /**
   * Register that a certain tag format is available.
   * @param fmt tag format to register
   * @remark This information is not stored in the configuration, it is
   * queried at run time using hasTagFormat().
   */
  void setTagFormat(TagFormatFlag fmt) {
    m_tagFormats |= fmt;
  }

  /** true to mark truncated ID3v1.1 fields */
  bool markTruncations() const { return m_markTruncations; }

  /** Set true to mark truncated ID3v1.1 fields. */
  void setMarkTruncations(bool markTruncations) {
    m_markTruncations = markTruncations;
  }

  /** true to write total number of tracks into track fields */
  bool enableTotalNumberOfTracks() const { return m_enableTotalNumberOfTracks; }

  /** Set true to write total number of tracks into track fields. */
  void setEnableTotalNumberOfTracks(bool enableTotalNumberOfTracks) {
    m_enableTotalNumberOfTracks = enableTotalNumberOfTracks;
  }

  /** true to write genres as text instead of numeric string */
  bool genreNotNumeric() const { return m_genreNotNumeric; }

  /** Set true to write genres as text instead of numeric string. */
  void setGenreNotNumeric(bool genreNotNumeric) {
    m_genreNotNumeric = genreNotNumeric;
  }

  /** field name used for Vorbis comment entries */
  QString commentName() const { return m_commentName; }

  /** Set field name used for Vorbis comment entries. */
  void setCommentName(const QString& commentName) {
    m_commentName = commentName;
  }

  /** index of field name used for Vorbis picture entries */
  int pictureNameItem() const { return m_pictureNameItem; }

  /** Set index of field name used for Vorbis picture entries. */
  void setPictureNameItem(int pictureNameItem) {
    m_pictureNameItem = pictureNameItem;
  }

  /** custom genres for ID3v2.3 */
  QStringList customGenres() const { return m_customGenres; }

  /** Set custom genres for ID3v2.3. */
  void setCustomGenres(const QStringList& customGenres) {
    m_customGenres = customGenres;
  }

  /** version used for new ID3v2 tags */
  int id3v2Version() const;

  /** Set version used for new ID3v2 tags. */
  void setId3v2Version(int id3v2Version) {
    m_id3v2Version = id3v2Version;
  }

  /** text encoding used for new ID3v1 tags */
  QString textEncodingV1() const { return m_textEncodingV1; }

  /** Set text encoding used for new ID3v1 tags. */
  void setTextEncodingV1(const QString& textEncodingV1) {
    m_textEncodingV1 = textEncodingV1;
  }

  /** text encoding used for new ID3v2 tags */
  int textEncoding() const { return m_textEncoding; }

  /** Set text encoding used for new ID3v2 tags. */
  void setTextEncoding(int textEncoding) {
    m_textEncoding = textEncoding;
  }

  /** frames which are displayed for Tag 2 even if not present */
  quint64 quickAccessFrames() const {
    return m_quickAccessFrames;
  }

  /** Set frames which are displayed for Tag 2 even if not present. */
  void setQuickAccessFrames(quint64 quickAccessFrames) {
    m_quickAccessFrames = quickAccessFrames;
  }

  /** number of digits in track number */
  int trackNumberDigits() const { return m_trackNumberDigits; }

  /** Set number of digits in track number. */
  void setTrackNumberDigits(int trackNumberDigits) {
    m_trackNumberDigits = trackNumberDigits;
  }

  /** true to show only custom genres in combo boxes */
  bool onlyCustomGenres() const { return m_onlyCustomGenres; }

  /** Set true to show only custom genres in combo boxes. */
  void setOnlyCustomGenres(bool onlyCustomGenres) {
    m_onlyCustomGenres = onlyCustomGenres;
  }

  /** The order in which meta data plugins are tried when opening a file */
  QStringList pluginOrder() const { return m_pluginOrder; }

  /** Set the order in which meta data plugins are tried when opening a file. */
  void setPluginOrder(const QStringList& pluginOrder) {
    m_pluginOrder = pluginOrder;
  }

  /** Disabled plugins */
  QStringList disabledPlugins() const { return m_disabledPlugins; }

  /** Set list of disabled plugins. */
  void setDisabledPlugins(const QStringList& disabledPlugins) {
    m_disabledPlugins = disabledPlugins;
  }

  /**
   * Access to list of available plugins.
   * @return available plugins.
   * @remark This information is not stored in the configuration, it is
   * determined when the plugins are loaded.
   */
  QStringList& availablePlugins() { return m_availablePlugins; }

  /**
   * Get list of available plugins.
   * @return available plugins.
   */
  QStringList getAvailablePlugins() const { return m_availablePlugins; }

  /** Index in configuration storage */
  static int s_index;

private:
  bool m_markTruncations;
  bool m_enableTotalNumberOfTracks;
  bool m_genreNotNumeric;
  QString m_commentName;
  int m_pictureNameItem;
  QStringList m_customGenres;
  int m_id3v2Version;
  QString m_textEncodingV1;
  int m_textEncoding;
  quint64 m_quickAccessFrames;
  int m_trackNumberDigits;
  bool m_onlyCustomGenres;
  QStringList m_pluginOrder;
  QStringList m_disabledPlugins;

  QStringList m_availablePlugins;
  int m_tagFormats;
};

#endif
