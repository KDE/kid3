/**
 * \file taglibutils.cpp
 * Utility functions for tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Nov 2025
 *
 * Copyright (C) 2025  Urs Fleisch
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

#include "taglibutils.h"

#include <tstring.h>

namespace TagLibUtils {

/** Convert QString @a s to a TagLib::String. */
TagLib::String toTString(const QString& s)
{
  int len = s.length();
  QVarLengthArray<wchar_t> a(len + 1);
  wchar_t* const ws = a.data();
  // Do not use `len = s.toWCharArray(ws); ws[len] = 0;`, this would construct
  // an array with UCS-4 encoded wide characters (if not on Windows), which
  // is not compatible with TagLib, which expects only 16 bit characters.
  // This works for Basic Multilingual Plane only, but not for surrogate pairs.
  wchar_t* wsPtr = ws;
  for (auto it = s.constBegin(); it != s.constEnd(); ++it) {
    *wsPtr++ = it->unicode();
  }
  *wsPtr = 0;
  return TagLib::String(ws);
}

/** Convert TagLib::String @a s to a QString. */
QString toQString(const TagLib::String& s)
{
  return QString::fromWCharArray(s.toCWString(), s.size());
}

/**
 * Convert TagLib::StringList @a tstrs to QString joining with
 * Frame::stringListSeparator().
 */
QString joinToQString(const TagLib::StringList &tstrs)
{
  QStringList strs;
  strs.reserve(tstrs.size());
  for (const TagLib::String &tstr : tstrs) {
    strs.append(toQString(tstr));
  }
  return Frame::joinStringList(strs);
}

/**
 * Convert QString @a s to a TagLib::StringList splitting with
 * Frame::stringListSeparator().
 */
TagLib::StringList splitToTStringList(const QString &s)
{
  const QStringList qstrs = Frame::splitStringList(s);
  TagLib::StringList tstrs;
  for (const QString &qstr : qstrs) {
    tstrs.append(toTString(qstr));
  }
  return tstrs;
}

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
const char* getVorbisNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    "TITLE",           // FT_Title,
    "ARTIST",          // FT_Artist,
    "ALBUM",           // FT_Album,
    "COMMENT",         // FT_Comment,
    "DATE",            // FT_Date,
    "TRACKNUMBER",     // FT_Track,
    "GENRE",           // FT_Genre,
                       // FT_LastV1Frame = FT_Track,
    "ALBUMARTIST",     // FT_AlbumArtist,
    "ARRANGER",        // FT_Arranger,
    "AUTHOR",          // FT_Author,
    "BPM",             // FT_Bpm,
    "CATALOGNUMBER",   // FT_CatalogNumber,
    "COMPILATION",     // FT_Compilation,
    "COMPOSER",        // FT_Composer,
    "CONDUCTOR",       // FT_Conductor,
    "COPYRIGHT",       // FT_Copyright,
    "DISCNUMBER",      // FT_Disc,
    "ENCODED-BY",      // FT_EncodedBy,
    "ENCODERSETTINGS", // FT_EncoderSettings,
    "ENCODINGTIME",    // FT_EncodingTime,
    "GROUPING",        // FT_Grouping,
    "INITIALKEY",      // FT_InitialKey,
    "ISRC",            // FT_Isrc,
    "LANGUAGE",        // FT_Language,
    "LYRICIST",        // FT_Lyricist,
    "LYRICS",          // FT_Lyrics,
    "SOURCEMEDIA",     // FT_Media,
    "MOOD",            // FT_Mood,
    "ORIGINALALBUM",   // FT_OriginalAlbum,
    "ORIGINALARTIST",  // FT_OriginalArtist,
    "ORIGINALDATE",    // FT_OriginalDate,
    "DESCRIPTION",     // FT_Description,
    "PERFORMER",       // FT_Performer,
    "METADATA_BLOCK_PICTURE", // FT_Picture,
    "PUBLISHER",       // FT_Publisher,
    "RELEASECOUNTRY",  // FT_ReleaseCountry,
    "REMIXER",         // FT_Remixer,
    "ALBUMSORT",       // FT_SortAlbum,
    "ALBUMARTISTSORT", // FT_SortAlbumArtist,
    "ARTISTSORT",      // FT_SortArtist,
    "COMPOSERSORT",    // FT_SortComposer,
    "TITLESORT",       // FT_SortName,
    "SUBTITLE",        // FT_Subtitle,
    "WEBSITE",         // FT_Website,
    "WWWAUDIOFILE",    // FT_WWWAudioFile,
    "WWWAUDIOSOURCE",  // FT_WWWAudioSource,
    "RELEASEDATE",     // FT_ReleaseDate,
    "RATING",          // FT_Rating,
    "WORK"             // FT_Work,
                       // FT_Custom1
  };
  Q_STATIC_ASSERT(std::size(names) == Frame::FT_Custom1);
  if (type == Frame::FT_Picture &&
      TagConfig::instance().pictureNameIndex() == TagConfig::VP_COVERART) {
    return "COVERART";
  }
  if (Frame::isCustomFrameType(type)) {
    return Frame::getNameForCustomFrame(type);
  }
  return type < Frame::FT_Custom1 ? names[type] : "UNKNOWN";
}

/**
 * Get the frame type for a Vorbis name.
 *
 * @param name Vorbis tag name
 *
 * @return frame type.
 */
Frame::Type getTypeFromVorbisName(QString name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i < Frame::FT_Custom1; ++i) {
      auto type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getVorbisNameFromType(type)), type);
    }
    strNumMap.insert(QLatin1String("COVERART"), Frame::FT_Picture);
    strNumMap.insert(QLatin1String("METADATA_BLOCK_PICTURE"), Frame::FT_Picture);
  }
  if (auto it = strNumMap.constFind(name.remove(QLatin1Char('=')).toUpper());
      it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::getTypeFromCustomFrameName(name.toLatin1());
}

}
