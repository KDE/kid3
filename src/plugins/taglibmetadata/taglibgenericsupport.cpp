/**
 * \file taglibgenericsupport.cpp
 * Support for generic files and tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Nov 2025
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

#include "taglibgenericsupport.h"

#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

TagLib::File* TagLibGenericSupport::createFromExtension(
  TagLib::IOStream*, const TagLib::String&) const
{
  return nullptr;
}

bool TagLibGenericSupport::readFile(TagLibFile&, TagLib::File*) const
{
  return false;
}

bool TagLibGenericSupport::writeFile(TagLibFile& f, TagLib::File*, bool force,
  int, bool& fileChanged) const
{
  if (anyTagMustBeSaved(f, force)) {
    if (saveFileRef(f)) {
      fileChanged = true;
    }
  }
  return true;
}

bool TagLibGenericSupport::readAudioProperties(
  TagLibFile&, TagLib::AudioProperties*) const
{
  return false;
}

QStringList TagLibGenericSupport::getFrameIds(
  const TagLibFile& f, Frame::TagNumber tagNr) const
{
  QStringList lst;
  static const char* const fieldNames[] = {
    "CONTACT",
    "DISCTOTAL",
    "EAN/UPN",
    "ENCODING",
    "ENGINEER",
    "ENSEMBLE",
    "GUESTARTIST",
    "LABEL",
    "LABELNO",
    "LICENSE",
    "LOCATION",
    "OPUS",
    "ORGANIZATION",
    "PARTNUMBER",
    "PRODUCER",
    "PRODUCTNUMBER",
    "RECORDINGDATE",
    "TRACKTOTAL",
    "VERSION",
    "VOLUME"
  };
  const bool picturesSupported = f.m_extraFrames.isRead() ||
      f.m_tagType[tagNr] == TaggedFile::TT_Vorbis || f.m_tagType[tagNr] == TaggedFile::TT_Ape;
  for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
    if (k != Frame::FT_Picture || picturesSupported) {
      if (auto name = Frame::ExtendedType(static_cast<Frame::Type>(k),
                                          QLatin1String("")).getName();
          !name.isEmpty()) {
        lst.append(name);
      }
    }
  }
  for (auto fieldName : fieldNames) {
    lst.append(QString::fromLatin1(fieldName)); // clazy:exclude=reserve-candidates
  }
  return lst;
}
