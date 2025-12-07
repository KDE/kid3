/**
 * \file taglibutils.h
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

#pragma once

#include "taglibfile.h"

/** for loop through all supported tag number values. */
#define FOR_TAGLIB_TAGS(variable) \
  for (Frame::TagNumber variable = Frame::Tag_1; \
       variable < TagLibFile::NUM_TAGS; \
       variable = static_cast<Frame::TagNumber>(variable + 1))

/** for loop through all supported tag number values in reverse order. */
#define FOR_TAGLIB_TAGS_REVERSE(variable) \
  for (Frame::TagNumber variable = static_cast<Frame::TagNumber>(TagLibFile::NUM_TAGS - 1); \
       variable >= Frame::Tag_1; \
       variable = static_cast<Frame::TagNumber>(variable - 1))

#if defined TAGLIB_WITH_OFFSET_TYPE || TAGLIB_VERSION >= 0x020000
typedef TagLib::offset_t taglib_offset_t;
typedef TagLib::offset_t taglib_uoffset_t;
#else
typedef long taglib_offset_t;
typedef ulong taglib_uoffset_t;
#endif

namespace TagLibUtils {

  /** Convert QString @a s to a TagLib::String. */
  TagLib::String toTString(const QString& s);

  /** Convert TagLib::String @a s to a QString. */
  QString toQString(const TagLib::String& s);

  /**
   * Convert TagLib::StringList @a tstrs to QString joining with
   * Frame::stringListSeparator().
   */
  QString joinToQString(const TagLib::StringList &tstrs);

  /**
   * Convert QString @a s to a TagLib::StringList splitting with
   * Frame::stringListSeparator().
   */
  TagLib::StringList splitToTStringList(const QString &s);

  /**
   * Get name of frame from type.
   *
   * @param type type
   *
   * @return name.
   */
  const char* getVorbisNameFromType(Frame::Type type);

  /**
   * Get the frame type for a Vorbis name.
   *
   * @param name Vorbis tag name
   *
   * @return frame type.
   */
  Frame::Type getTypeFromVorbisName(QString name);

}
