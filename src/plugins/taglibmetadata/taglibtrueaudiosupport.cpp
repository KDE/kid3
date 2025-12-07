/**
 * \file taglibtrueaudiosupport.cpp
 * Support for TrueAudio files and tags.
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

#include "taglibtrueaudiosupport.h"

#include <id3v1tag.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <trueaudiofile.h>

#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

TagLib::File* TagLibTrueAudioSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "TTA")
    return new TagLib::TrueAudio::File(stream);
  return nullptr;
}

bool TagLibTrueAudioSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (auto ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) {
    f.m_fileExtension = QLatin1String(".tta");
    f.m_isTagSupported[Frame::Tag_1] = true;
    if (!f.m_tag[Frame::Tag_1]) {
      f.m_tag[Frame::Tag_1] = ttaFile->ID3v1Tag();
      f.markTagUnchanged(Frame::Tag_1);
    }
    if (!f.m_tag[Frame::Tag_2]) {
      f.m_tag[Frame::Tag_2] = ttaFile->ID3v2Tag();
      f.markTagUnchanged(Frame::Tag_2);
    }
    return true;
  }
  return false;
}

bool TagLibTrueAudioSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
  int, bool& fileChanged) const
{
  if (auto ttaFile =
      dynamic_cast<TagLib::TrueAudio::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      static constexpr int tagTypes[TagLibFile::NUM_TAGS] = {
        TagLib::MPEG::File::ID3v1, TagLib::MPEG::File::ID3v2,
        TagLib::MPEG::File::NoTags
      };
      FOR_TAGLIB_TAGS(tagNr) {
        if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr)) && f.m_tag[tagNr]->isEmpty()) {
          ttaFile->strip(tagTypes[tagNr]);
          fileChanged = true;
          f.m_tag[tagNr] = nullptr;
          f.markTagUnchanged(tagNr);
        }
      }
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
  return false;
}

bool TagLibTrueAudioSupport::makeTagSettable(TagLibFile& f, TagLib::File* file,
  Frame::TagNumber tagNr) const
{
  if (tagNr == Frame::Tag_1) {
    if (auto ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) {
      f.m_tag[tagNr] = ttaFile->ID3v1Tag(true);
      return true;
    }
  } else if (tagNr == Frame::Tag_2) {
    if (auto ttaFile = dynamic_cast<TagLib::TrueAudio::File*>(file)) {
      f.m_tag[tagNr] = ttaFile->ID3v2Tag(true);
      return true;
    }
  }
  return false;
}

bool TagLibTrueAudioSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto ttaProperties =
      dynamic_cast<TagLib::TrueAudio::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QLatin1String("True Audio ");
    f.m_detailInfo.format += QString::number(ttaProperties->ttaVersion());
    f.m_detailInfo.format += QLatin1Char(' ');
    f.m_detailInfo.format += QString::number(ttaProperties->bitsPerSample());
    f.m_detailInfo.format += QLatin1String(" bit");
    return true;
  }
  return false;
}
