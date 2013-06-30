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
class KID3_CORE_EXPORT TagConfig : public GeneralConfig
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
   * Constructor.
   */
  explicit TagConfig(const QString& group);

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

  /** true to mark truncated ID3v1.1 fields */
  bool m_markTruncations;
  /** true to write total number of tracks into track fields */
  bool m_enableTotalNumberOfTracks;
  /** true to write genres as text instead of numeric string */
  bool m_genreNotNumeric;
  /** field name used for Vorbis comment entries */
  QString m_commentName;
  /** index of field name used for Vorbis picture entries */
  int m_pictureNameItem;
  /** custom genres for ID3v2.3 */
  QStringList m_customGenres;
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
  /** true to show only custom genres in combo boxes */
  bool m_onlyCustomGenres;
};

#endif
