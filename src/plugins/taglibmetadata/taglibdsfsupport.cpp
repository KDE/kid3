/**
 * \file taglibdsfsupport.cpp
 * Support for DSF and DFF files.
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

#include "taglibdsfsupport.h"
#include "taglibfile.h"

#include <taglib.h>
#include <id3v2framefactory.h>
#if TAGLIB_VERSION >= 0x020000
#include <dsffile.h>
#include <dsdifffile.h>
#else
#include <id3v2tag.h>
#include "taglibext/dsf/dsffiletyperesolver.h"
#include "taglibext/dsf/dsffile.h"
#include "taglibext/dsdiff/dsdifffiletyperesolver.h"
#include "taglibext/dsdiff/dsdifffile.h"
#endif

#include "taglibutils.h"

using namespace TagLibUtils;

TagLib::File* TagLibDsfSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "DSF")
#if TAGLIB_VERSION >= 0x020000
    return new TagLib::DSF::File(stream);
#else
    return new DSFFile(stream, TagLib::ID3v2::FrameFactory::instance());
#endif
  if (ext == "DFF")
#if TAGLIB_VERSION >= 0x020000
    return new TagLib::DSDIFF::File(stream);
#else
    return new DSDIFFFile(stream, TagLib::ID3v2::FrameFactory::instance());
#endif
  return nullptr;
}

bool TagLibDsfSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
#if TAGLIB_VERSION >= 0x020000
  if (auto dsfFile = dynamic_cast<TagLib::DSF::File*>(file)) {
#else
  if (auto dsfFile = dynamic_cast<DSFFile*>(file)) {
#endif
    f.m_fileExtension = QLatin1String(".dsf");
    f.m_tag[Frame::Tag_1] = nullptr;
    f.markTagUnchanged(Frame::Tag_1);
    if (!f.m_tag[Frame::Tag_2]) {
#if TAGLIB_VERSION >= 0x020000
      TagLib::ID3v2::Tag* id3v2Tag = dsfFile->tag();
#else
      TagLib::ID3v2::Tag* id3v2Tag = dsfFile->ID3v2Tag();
#endif
      f.setId3v2VersionFromTag(id3v2Tag);
      f.m_tag[Frame::Tag_2] = id3v2Tag;
      f.markTagUnchanged(Frame::Tag_2);
    }
    return true;
  }
#if TAGLIB_VERSION >= 0x020000
  if (auto dffFile = dynamic_cast<TagLib::DSDIFF::File*>(file)) {
#else
  if (auto dffFile = dynamic_cast<DSDIFFFile*>(file)) {
#endif
    f.m_fileExtension = QLatin1String(".dff");
    f.m_tag[Frame::Tag_1] = nullptr;
    f.markTagUnchanged(Frame::Tag_1);
    if (!f.m_tag[Frame::Tag_2]) {
      TagLib::ID3v2::Tag* id3v2Tag = dffFile->ID3v2Tag();
      f.setId3v2VersionFromTag(id3v2Tag);
      f.m_tag[Frame::Tag_2] = id3v2Tag;
      f.markTagUnchanged(Frame::Tag_2);
    }
    return true;
  }
  return false;
}

bool TagLibDsfSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
                                 int id3v2Version, bool& fileChanged) const
{
#if TAGLIB_VERSION >= 0x020000
  if (auto dsfFile = dynamic_cast<TagLib::DSF::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      f.setId3v2VersionOrDefault(id3v2Version);
      if (dsfFile->save(f.m_id3v2Version == 4 ? TagLib::ID3v2::v4
                                              : TagLib::ID3v2::v3)) {
#else
  if (auto dsfFile = dynamic_cast<DSFFile*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      f.setId3v2VersionOrDefault(id3v2Version);
      if (dsfFile->save(f.m_id3v2Version)) {
#endif
        fileChanged = true;
        FOR_TAGLIB_TAGS(tagNr) {
          f.markTagUnchanged(tagNr);
        }
      }
    }
    return true;
  }
#if TAGLIB_VERSION >= 0x020000
  if (auto dffFile = dynamic_cast<TagLib::DSDIFF::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      int saveMask = 0;
      if (f.m_tag[Frame::Tag_2] && (force || f.isTagChanged(Frame::Tag_2))) {
        if (f.m_tag[Frame::Tag_2]->isEmpty()) {
          dffFile->strip(TagLib::DSDIFF::File::ID3v2);
          fileChanged = true;
          f.m_tag[Frame::Tag_2] = nullptr;
          f.markTagUnchanged(Frame::Tag_2);
        } else {
          saveMask = TagLib::DSDIFF::File::ID3v2;
        }
      }
      f.setId3v2VersionOrDefault(id3v2Version);
      if (saveMask != 0 && dffFile->save(saveMask,
                        TagLib::File::StripNone,
                        f.m_id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3)) {
#else
  if (auto dffFile = dynamic_cast<DSDIFFFile*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      f.setId3v2VersionOrDefault(id3v2Version);
      if (dffFile->save(f.m_id3v2Version)) {
#endif
        fileChanged = true;
        FOR_TAGLIB_TAGS(tagNr) {
          f.markTagUnchanged(tagNr);
        }
      }
    }
    return true;
  }
  return false;
}

bool TagLibDsfSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
#if TAGLIB_VERSION >= 0x020000
  if (auto dsfProperties =
      dynamic_cast<TagLib::DSF::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("DSF %1"))
                              .arg(dsfProperties->formatVersion());
    return true;
  }
  if (dynamic_cast<TagLib::DSDIFF::Properties*>(audioProperties) != nullptr) {
    f.m_detailInfo.format = QString(QLatin1String("DFF"));
    return true;
  }
#else
  if (auto dsfProperties =
        dynamic_cast<DSFProperties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("DSF %1"))
      .arg(dsfProperties->version());
    return true;
  }
  if (dynamic_cast<DSDIFFProperties*>(audioProperties) != nullptr) {
    f.m_detailInfo.format = QString(QLatin1String("DFF"));
    return true;
  }
#endif
  return false;
}
