/**
 * \file taglibformatsupport.cpp
 * Base class for audio formats supported by TagLib.
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

#include "taglibformatsupport.h"
#include "taglibfile.h"
#include "taglibutils.h"

using namespace TagLibUtils;

TagLibFormatSupport::TagLibFormatSupport() = default;
TagLibFormatSupport::~TagLibFormatSupport() = default;

bool TagLibFormatSupport::writeFile(TagLibFile&, TagLib::File*, bool,
  int, bool&) const
{
  return false;
}

bool TagLibFormatSupport::makeTagSettable(TagLibFile&, TagLib::File*,
  Frame::TagNumber) const
{
  return false;
}

QString TagLibFormatSupport::getTagFormat(const TagLib::Tag*,
  TaggedFile::TagType&) const
{
  return {};
}

bool TagLibFormatSupport::setFrame(TagLibFile&, Frame::TagNumber,
  const Frame&) const
{
  return false;
}

bool TagLibFormatSupport::addFrame(TagLibFile&, Frame::TagNumber, Frame&) const
{
  return false;
}

bool TagLibFormatSupport::deleteFrame(TagLibFile&, Frame::TagNumber,
  const Frame&) const
{
  return false;
}

bool TagLibFormatSupport::deleteFrames(TagLibFile&, Frame::TagNumber,
  const FrameFilter&) const
{
  return false;
}

bool TagLibFormatSupport::getAllFrames(TagLibFile&, Frame::TagNumber,
  FrameCollection&) const
{
  return false;
}

QStringList TagLibFormatSupport::getFrameIds(const TagLibFile&,
  Frame::TagNumber) const
{
  return {};
}


void TagLibFormatSupport::putFileRefTagInTag2(TagLibFile& f)
{
  f.m_tag[Frame::Tag_1] = nullptr;
  f.markTagUnchanged(Frame::Tag_1);
  if (!f.m_tag[Frame::Tag_2]) {
    f.m_tag[Frame::Tag_2] = f.m_fileRef.tag();
    f.markTagUnchanged(Frame::Tag_2);
  }
}

bool TagLibFormatSupport::anyTagMustBeSaved(const TagLibFile& f, bool force)
{
  FOR_TAGLIB_TAGS(tagNr) {
    if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr))) {
      return true;
    }
  }
  return false;
}

bool TagLibFormatSupport::saveFileRef(TagLibFile& f)
{
  if (f.m_fileRef.save()) {
    FOR_TAGLIB_TAGS(tagNr) {
      f.markTagUnchanged(tagNr);
    }
    return true;
  }
  return false;
}

bool TagLibFormatSupport::setFrameWithoutIndex(TagLibFile& f, Frame::TagNumber tagNr, const Frame& frame) const
{
  // Try the basic method
  if (QString str = frame.getValue();
      f.makeTagSettable(tagNr) && !str.isNull()) {
    TagLib::Tag* tag = f.m_tag[tagNr];
    if (!tag)
      return false;
    Frame::Type type = frame.getType();
#if TAGLIB_VERSION >= 0x010b01
    TagLib::String tstr = toTString(str);
#else
    TagLib::String tstr = str.isEmpty() ? TagLib::String::null : toTString(str);
#endif
    if (type == Frame::FT_Date) {
      uint oldNum = tag->year();
      int num = frame.getValueAsNumber();
      if (tagNr == Frame::Tag_Id3v1) {
        if (num >= 0 && num != static_cast<int>(oldNum)) {
          tag->setYear(num);
          f.markTagChanged(tagNr, Frame::ExtendedType(type));
        }
      } else {
        if (num > 0 && num != static_cast<int>(oldNum) &&
            f.getDefaultTextEncoding() == TagLib::String::Latin1) {
          tag->setYear(num);
          f.markTagChanged(tagNr, Frame::ExtendedType(type));
        } else if (num == 0 || num != static_cast<int>(oldNum)){
          QString yearStr;
          if (num != 0) {
            yearStr.setNum(num);
          } else {
            yearStr = frame.getValue();
          }
#if TAGLIB_VERSION >= 0x010b01
          TagLib::String yearTStr = toTString(yearStr);
#else
          TagLib::String yearTStr =
              yearStr.isEmpty() ? TagLib::String::null : toTString(yearStr);
#endif
          setTagValue(f, tagNr, type, yearTStr);
          f.markTagChanged(tagNr, Frame::ExtendedType(type));
        }
      }
    } else if (type == Frame::FT_Track) {
      uint oldNum = tag->track();
      if (int num = frame.getValueAsNumber();
          num >= 0 && num != static_cast<int>(oldNum)) {
        if (tagNr == Frame::Tag_Id3v1) {
          if (int n = f.checkTruncation(tagNr, num, 1ULL << type); n != -1) {
            num = n;
          }
          tag->setTrack(num);
        } else {
          int numTracks;
          num = TagLibFile::splitNumberAndTotal(str, &numTracks);
          QString trackStr = f.trackNumberString(num, numTracks);
          if (num != static_cast<int>(oldNum)) {
            setTagValue(f, tagNr, type, toTString(trackStr));
          }
        }
        f.markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    } else if (type == Frame::FT_Album) {
      if (TagLib::String oldTstr = tag->album(); tstr != oldTstr) {
        setTagValue(f, tagNr, type, tstr);
        f.markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    } else if (type == Frame::FT_Comment) {
      if (TagLib::String oldTstr = tag->comment(); tstr != oldTstr) {
        setTagValue(f, tagNr, type, tstr);
        f.markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    } else if (type == Frame::FT_Artist) {
      if (TagLib::String oldTstr = tag->artist(); tstr != oldTstr) {
        setTagValue(f, tagNr, type, tstr);
        f.markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    } else if (type == Frame::FT_Title) {
      if (TagLib::String oldTstr = tag->title(); tstr != oldTstr) {
        setTagValue(f, tagNr, type, tstr);
        f.markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    } else if (type == Frame::FT_Genre) {
      if (TagLib::String oldTstr = tag->genre(); tstr != oldTstr) {
        setTagValue(f, tagNr, type, tstr);
        f.markTagChanged(tagNr, Frame::ExtendedType(type));
      }
    } else {
      return false;
    }
  }
  return true;
}

void TagLibFormatSupport::setTagValue(TagLibFile& f, Frame::TagNumber tagNr, Frame::Type type,
  const TagLib::String& str) const
{
  TagLib::Tag* tag = f.m_tag[tagNr];
  switch (type) {
  case Frame::FT_Date:
    tag->setYear(str.toInt());
    break;
  case Frame::FT_Track:
    tag->setTrack(str.toInt());
    break;
  case Frame::FT_Album:
    tag->setAlbum(str);
    break;
  case Frame::FT_Comment:
    tag->setComment(str);
    break;
  case Frame::FT_Artist:
    tag->setArtist(str);
    break;
  case Frame::FT_Title:
    tag->setTitle(str);
    break;
  case Frame::FT_Genre:
    tag->setGenre(str);
    break;
  default: ;
  }
}
