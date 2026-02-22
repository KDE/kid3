/**
 * \file taglibmp4support.cpp
 * Support for MP4 files and tags.
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

#include "taglibmp4support.h"

#include <mp4file.h>

#include "pictureframe.h"
#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

/** Type of data in MP4 frame. */
enum Mp4ValueType {
  MVT_ByteArray,
  MVT_CoverArt,
  MVT_String,
  MVT_Bool,
  MVT_Int,
  MVT_IntPair,
  MVT_Byte,
  MVT_UInt,
  MVT_LongLong
#if TAGLIB_VERSION >= 0x020200
  , MVT_Stem
#endif
};

/** MP4 name, frame type and value type. */
struct Mp4NameTypeValue {
  const char* name;
  Frame::Type type;
  Mp4ValueType value;
};

/** Mapping between frame types and field names. */
const Mp4NameTypeValue mp4NameTypeValues[] = {
  { "\251nam", Frame::FT_Title, MVT_String },
  { "\251ART", Frame::FT_Artist, MVT_String },
  { "\251wrt", Frame::FT_Composer, MVT_String },
  { "\251alb", Frame::FT_Album, MVT_String },
  { "\251day", Frame::FT_Date, MVT_String },
  { "\251enc", Frame::FT_EncodedBy, MVT_String },
  { "\251cmt", Frame::FT_Comment, MVT_String },
  { "gnre", Frame::FT_Genre, MVT_String },
  // (c)gen is after gnre so that it is used in the maps because TagLib uses it
  { "\251gen", Frame::FT_Genre, MVT_String },
  { "trkn", Frame::FT_Track, MVT_IntPair },
  { "disk", Frame::FT_Disc, MVT_IntPair },
  { "cpil", Frame::FT_Compilation, MVT_Bool },
  { "tmpo", Frame::FT_Bpm, MVT_Int },
  { "\251grp", Frame::FT_Grouping, MVT_String },
  { "aART", Frame::FT_AlbumArtist, MVT_String },
  { "pgap", Frame::FT_Other, MVT_Bool },
  { "cprt", Frame::FT_Copyright, MVT_String },
  { "\251lyr", Frame::FT_Lyrics, MVT_String },
  { "tvsh", Frame::FT_Other, MVT_String },
  { "tvnn", Frame::FT_Other, MVT_String },
  { "tven", Frame::FT_Other, MVT_String },
  { "tvsn", Frame::FT_Other, MVT_UInt },
  { "tves", Frame::FT_Other, MVT_UInt },
  { "desc", Frame::FT_Description, MVT_String },
  { "ldes", Frame::FT_Other, MVT_String },
  { "sonm", Frame::FT_SortName, MVT_String },
  { "soar", Frame::FT_SortArtist, MVT_String },
  { "soaa", Frame::FT_SortAlbumArtist, MVT_String },
  { "soal", Frame::FT_SortAlbum, MVT_String },
  { "soco", Frame::FT_SortComposer, MVT_String },
  { "sosn", Frame::FT_Other, MVT_String },
  { "\251too", Frame::FT_EncoderSettings, MVT_String },
  { "purd", Frame::FT_Other, MVT_String },
  { "pcst", Frame::FT_Other, MVT_Bool },
  { "keyw", Frame::FT_Other, MVT_String },
  { "catg", Frame::FT_Other, MVT_String },
#if TAGLIB_VERSION >= 0x020000
  { "hdvd", Frame::FT_Other, MVT_UInt },
#else
  { "hdvd", Frame::FT_Other, MVT_Bool },
#endif
  { "stik", Frame::FT_Other, MVT_Byte },
  { "rtng", Frame::FT_Other, MVT_Byte },
  { "apID", Frame::FT_Other, MVT_String },
  { "akID", Frame::FT_Other, MVT_Byte },
  { "sfID", Frame::FT_Other, MVT_UInt },
  { "cnID", Frame::FT_Other, MVT_UInt },
  { "atID", Frame::FT_Other, MVT_UInt },
  { "plID", Frame::FT_Other, MVT_LongLong },
  { "geID", Frame::FT_Other, MVT_UInt },
  { "ownr", Frame::FT_Other, MVT_String },
#if TAGLIB_VERSION >= 0x010c00
  { "purl", Frame::FT_Other, MVT_String },
  { "egid", Frame::FT_Other, MVT_String },
  { "cmID", Frame::FT_Other, MVT_UInt },
#endif
  { "xid ", Frame::FT_Other, MVT_String },
  { "covr", Frame::FT_Picture, MVT_CoverArt },
#if TAGLIB_VERSION >= 0x020200
  { "stem", Frame::FT_Other, MVT_Stem },
#endif
#if TAGLIB_VERSION >= 0x010c00
  { "\251wrk", Frame::FT_Work, MVT_String },
  { "\251mvn", Frame::FT_Other, MVT_String },
  { "\251mvi", Frame::FT_Other, MVT_Int },
  { "\251mvc", Frame::FT_Other, MVT_Int },
  { "shwm", Frame::FT_Other, MVT_Bool },
#endif
  { "ARRANGER", Frame::FT_Arranger, MVT_String },
  { "AUTHOR", Frame::FT_Author, MVT_String },
  { "CATALOGNUMBER", Frame::FT_CatalogNumber, MVT_String },
  { "CONDUCTOR", Frame::FT_Conductor, MVT_String },
  { "ENCODINGTIME", Frame::FT_EncodingTime, MVT_String },
  { "INITIALKEY", Frame::FT_InitialKey, MVT_String },
  { "ISRC", Frame::FT_Isrc, MVT_String },
  { "LANGUAGE", Frame::FT_Language, MVT_String },
  { "LYRICIST", Frame::FT_Lyricist, MVT_String },
  { "MOOD", Frame::FT_Mood, MVT_String },
  { "SOURCEMEDIA", Frame::FT_Media, MVT_String },
  { "ORIGINALALBUM", Frame::FT_OriginalAlbum, MVT_String },
  { "ORIGINALARTIST", Frame::FT_OriginalArtist, MVT_String },
  { "ORIGINALDATE", Frame::FT_OriginalDate, MVT_String },
  { "PERFORMER", Frame::FT_Performer, MVT_String },
  { "PUBLISHER", Frame::FT_Publisher, MVT_String },
  { "RELEASECOUNTRY", Frame::FT_ReleaseCountry, MVT_String },
  { "REMIXER", Frame::FT_Remixer, MVT_String },
  { "SUBTITLE", Frame::FT_Subtitle, MVT_String },
  { "WEBSITE", Frame::FT_Website, MVT_String },
  { "WWWAUDIOFILE", Frame::FT_WWWAudioFile, MVT_String },
  { "WWWAUDIOSOURCE", Frame::FT_WWWAudioSource, MVT_String },
  { "RELEASEDATE", Frame::FT_ReleaseDate, MVT_String },
  { "rate", Frame::FT_Rating, MVT_String }
};

/**
 * Get MP4 name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
void getMp4NameForType(Frame::Type type, TagLib::String& name,
                       Mp4ValueType& value)
{
  static QMap<Frame::Type, unsigned> typeNameMap;
  if (typeNameMap.empty()) {
    // first time initialization
    for (unsigned i = 0; i < std::size(mp4NameTypeValues); ++i) {
      if (mp4NameTypeValues[i].type != Frame::FT_Other) {
        typeNameMap.insert(mp4NameTypeValues[i].type, i);
      }
    }
  }
  name = "";
  value = MVT_String;
  if (type != Frame::FT_Other) {
    if (auto it = typeNameMap.constFind(type);
        it != typeNameMap.constEnd()) {
      name = mp4NameTypeValues[*it].name;
      value = mp4NameTypeValues[*it].value;
    } else {
      if (auto customFrameName = Frame::getNameForCustomFrame(type);
          !customFrameName.isEmpty()) {
        name = TagLib::String(customFrameName.constData());
      }
    }
  }
}

/**
 * Get MP4 value type and frame type for an MP4 name.
 *
 * @param name  MP4 name
 * @param type  the frame type is returned here
 * @param value the MP4 value type is returned here
 *
 * @return true if free-form frame.
 */
bool getMp4TypeForName(const TagLib::String& name, Frame::Type& type,
                       Mp4ValueType& value)
{
  static QMap<TagLib::String, unsigned> nameTypeMap;
  if (nameTypeMap.empty()) {
    // first time initialization
    for (unsigned i = 0; i < std::size(mp4NameTypeValues); ++i) {
      nameTypeMap.insert(mp4NameTypeValues[i].name, i);
    }
  }
  if (auto it = nameTypeMap.constFind(name);
      it != nameTypeMap.constEnd()) {
    type = mp4NameTypeValues[*it].type;
    value = mp4NameTypeValues[*it].value;
    if (type == Frame::FT_Other) {
      type = Frame::getTypeFromCustomFrameName(name.toCString());
    }
    return name[0] >= 'A' && name[0] <= 'Z';
  }
  type = Frame::getTypeFromCustomFrameName(name.toCString());
  value = MVT_String;
  return true;
}

/**
 * Strip free form prefix from MP4 frame name.
 *
 * @param name MP4 frame name to be stripped
 */
void stripMp4FreeFormName(TagLib::String& name)
{
  if (name.startsWith("----")) {
    int nameStart = name.rfind(":");
    if (nameStart == -1) {
      nameStart = 5;
    } else {
      ++nameStart;
    }
    name = name.substr(nameStart);

    Frame::Type type;
    Mp4ValueType valueType;
    if (!getMp4TypeForName(name, type, valueType)) {
      // not detected as free form => mark with ':' as first character
      name = ':' + name;
    }
  }
}

/**
 * Prepend free form prefix to MP4 frame name.
 * Only names starting with a capital letter or ':' are prefixed.
 *
 * @param name MP4 frame name to be prefixed
 * @param mp4Tag tag to check for existing item
 */
void prefixMp4FreeFormName(TagLib::String& name, const TagLib::MP4::Tag* mp4Tag)
{
  if (
#if TAGLIB_VERSION >= 0x010a00
      !mp4Tag->contains(name)
#else
      !const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap().contains(name)
#endif
      && ((!name.startsWith("----") &&
           !(name.length() == 4 &&
             (static_cast<char>(name[0]) == '\251' ||
              (name[0] >= 'a' && name[0] <= 'z')))) ||
#if TAGLIB_VERSION >= 0x010a00
          mp4Tag->contains("----:com.apple.iTunes:" + name)
#else
          const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap().contains(
            "----:com.apple.iTunes:" + name)
#endif
          )
      ) {
    Frame::Type type;
    Mp4ValueType valueType;
    if (getMp4TypeForName(name, type, valueType)) {
      // free form
      if (name[0] == ':') name = name.substr(1);
      TagLib::String freeFormName = "----:com.apple.iTunes:" + name;
      if (unsigned int nameLen;
#if TAGLIB_VERSION >= 0x010a00
          !mp4Tag->contains(freeFormName)
#else
          !const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap().contains(
            freeFormName)
#endif
          && (nameLen = name.length()) > 0) {
        // Not an iTunes free form name, maybe using another prefix
        // (such as "----:com.nullsoft.winamp:").
        // Search for a frame which ends with this name.
#if TAGLIB_VERSION >= 0x010a00
        const TagLib::MP4::ItemMap& items = mp4Tag->itemMap();
#else
        const TagLib::MP4::ItemListMap& items =
            const_cast<TagLib::MP4::Tag*>(mp4Tag)->itemListMap();
#endif
        for (const auto& [key, item] : items) {
          if (key.length() >= nameLen &&
              key.substr(key.length() - nameLen, nameLen) == name) {
            freeFormName = key;
            break;
          }
        }
      }
      name = freeFormName;
    }
  }
}

/**
 * Get an MP4 type for a frame.
 *
 * @param frame frame
 * @param name  the MP4 name is returned here
 * @param value the MP4 value type is returned here
 */
void getMp4TypeForFrame(const Frame& frame, TagLib::String& name,
                        Mp4ValueType& value)
{
  if (frame.getType() != Frame::FT_Other) {
    getMp4NameForType(frame.getType(), name, value);
    if (name.isEmpty()) {
      name = toTString(frame.getInternalName());
    }
  } else {
    Frame::Type type;
    name = toTString(TaggedFile::fixUpTagKey(frame.getInternalName(),
                                             TaggedFile::TT_Mp4));
    getMp4TypeForName(name, type, value);
  }
}

/**
 * Get an MP4 item for a frame.
 *
 * @param frame frame
 * @param name  the name for the item is returned here
 *
 * @return MP4 item, an invalid item is returned if not supported.
 */
TagLib::MP4::Item getMp4ItemForFrame(const Frame& frame, TagLib::String& name)
{
  Mp4ValueType valueType;
  getMp4TypeForFrame(frame, name, valueType);
  switch (valueType) {
  case MVT_String:
    return splitToTStringList(frame.getValue());
  case MVT_Bool:
    return {frame.getValue().toInt() != 0};
  case MVT_Int:
    return {frame.getValue().toInt()};
  case MVT_IntPair:
  {
    QString str1 = frame.getValue(), str2 = QLatin1String("0");
    if (int slashPos = static_cast<int>(str1.indexOf(QLatin1Char('/')));
        slashPos != -1) {
      str2 = str1.mid(slashPos + 1);
      str1.truncate(slashPos);
    }
    return {str1.toInt(), str2.toInt()};
  }
  case MVT_CoverArt:
  {
    QByteArray ba;
    TagLib::MP4::CoverArt::Format format = TagLib::MP4::CoverArt::JPEG;
    if (PictureFrame::getData(frame, ba)) {
      if (QString mimeType;
          PictureFrame::getMimeType(frame, mimeType) &&
          mimeType == QLatin1String("image/png")) {
        format = TagLib::MP4::CoverArt::PNG;
      }
    }
    TagLib::MP4::CoverArt coverArt(format,
                                   TagLib::ByteVector(ba.data(), ba.size()));
    TagLib::MP4::CoverArtList coverArtList;
    coverArtList.append(coverArt);
    return coverArtList;
  }
#if TAGLIB_VERSION >= 0x020200
  case MVT_Stem:
  {
    QByteArray ba;
    PictureFrame::getData(frame, ba);
    TagLib::MP4::Stem stem(TagLib::ByteVector(ba.data(), ba.size()));
    return stem;
  }
#endif
  case MVT_Byte:
    return {static_cast<uchar>(frame.getValue().toInt())};
  case MVT_UInt:
    return {frame.getValue().toUInt()};
  case MVT_LongLong:
    return {frame.getValue().toLongLong()};
  case MVT_ByteArray:
  default:
    // binary data and album art are not handled by TagLib
    return {};
  }
}

}


TagLib::File* TagLibMp4Support::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "M4A" || ext == "M4R" || ext == "M4B" || ext == "M4P" ||
      ext == "M4R" || ext == "MP4" || ext == "3G2" || ext == "M4V" ||
      ext == "MP4V")
    return new TagLib::MP4::File(stream);
  return nullptr;
}

bool TagLibMp4Support::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (dynamic_cast<TagLib::MP4::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".m4a");
    putFileRefTagInTag2(f);
    putPicturesInExtraFrames(f);
    return true;
  }
  return false;
}

void TagLibMp4Support::putPicturesInExtraFrames(TagLibFile& f)
{
  if (!f.m_extraFrames.isRead()) {
    if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[Frame::Tag_2])) {
#if TAGLIB_VERSION >= 0x010a00
      const auto& itemMap = mp4Tag->itemMap();
      auto it = itemMap.find("covr");
      const TagLib::MP4::CoverArtList pics = it != itemMap.end()
        ? it->second.toCoverArtList() : TagLib::MP4::CoverArtList();
#else
      const TagLib::MP4::CoverArtList pics(mp4Tag->itemListMap()["covr"].toCoverArtList());
#endif
      int i = 0;
      for (const auto& coverArt : pics) {
        TagLib::ByteVector bv = coverArt.data();
        QString mimeType, imgFormat;
        switch (coverArt.format()) {
        case TagLib::MP4::CoverArt::PNG:
          mimeType = QLatin1String("image/png");
          imgFormat = QLatin1String("PNG");
          break;
        case TagLib::MP4::CoverArt::BMP:
          mimeType = QLatin1String("image/bmp");
          imgFormat = QLatin1String("BMP");
          break;
        case TagLib::MP4::CoverArt::GIF:
          mimeType = QLatin1String("image/gif");
          imgFormat = QLatin1String("GIF");
          break;
        case TagLib::MP4::CoverArt::JPEG:
        case TagLib::MP4::CoverArt::Unknown:
        default:
          mimeType = QLatin1String("image/jpeg");
          imgFormat = QLatin1String("JPG");
        }
        PictureFrame frame(
          QByteArray(bv.data(), static_cast<int>(bv.size())),
          QLatin1String(""), PictureFrame::PT_CoverFront, mimeType,
          Frame::TE_ISO8859_1, imgFormat);
        frame.setIndex(Frame::toNegativeIndex(i++));
        frame.setExtendedType(Frame::ExtendedType(Frame::FT_Picture,
                                                  QLatin1String("covr")));
        f.m_extraFrames.append(frame);
      }
      f.m_extraFrames.setRead(true);
    }
  }
}

bool TagLibMp4Support::writeFile(TagLibFile& f, TagLib::File* file, bool force,
  int, bool& fileChanged) const
{
  if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[Frame::Tag_2])) {
    if (anyTagMustBeSaved(f, force)) {
      if (!f.m_extraFrames.isEmpty()) {
        TagLib::MP4::CoverArtList coverArtList;
        const auto frames = f.m_extraFrames;
        for (const Frame& frame : frames) {
          QByteArray ba;
          TagLib::MP4::CoverArt::Format format = TagLib::MP4::CoverArt::JPEG;
          if (PictureFrame::getData(frame, ba)) {
            if (QString mimeType;
                PictureFrame::getMimeType(frame, mimeType)) {
              if (mimeType == QLatin1String("image/png")) {
                format = TagLib::MP4::CoverArt::PNG;
              } else if (mimeType == QLatin1String("image/bmp")) {
                format = TagLib::MP4::CoverArt::BMP;
              } else if (mimeType == QLatin1String("image/gif")) {
                format = TagLib::MP4::CoverArt::GIF;
              }
            }
          }
          coverArtList.append(TagLib::MP4::CoverArt(
                      format,
                      TagLib::ByteVector(
                        ba.data(), static_cast<unsigned int>(ba.size()))));
        }
#if TAGLIB_VERSION >= 0x010a00
        mp4Tag->setItem("covr", coverArtList);
#else
        mp4Tag->itemListMap()["covr"] = coverArtList;
#endif
      } else {
#if TAGLIB_VERSION >= 0x010a00
        mp4Tag->removeItem("covr");
#else
        mp4Tag->itemListMap().erase("covr");
#endif
      }
#if TAGLIB_VERSION >= 0x010d00
      if (TagLib::MP4::File* mp4File;
          (force || f.isTagChanged(Frame::Tag_2)) && mp4Tag->isEmpty() &&
          (mp4File = dynamic_cast<TagLib::MP4::File*>(file)) != nullptr) {
        mp4File->strip();
        fileChanged = true;
        f.m_tag[Frame::Tag_2] = nullptr;
        f.markTagUnchanged(Frame::Tag_2);
        return true;
      }
#endif
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
  return false;
}

bool TagLibMp4Support::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto mp4Properties =
      dynamic_cast<TagLib::MP4::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QLatin1String("MP4");
#if TAGLIB_VERSION >= 0x010a00
    switch (mp4Properties->codec()) {
    case TagLib::MP4::Properties::AAC:
      f.m_detailInfo.format += QLatin1String(" AAC");
      break;
    case TagLib::MP4::Properties::ALAC:
      f.m_detailInfo.format += QLatin1String(" ALAC");
      break;
    case TagLib::MP4::Properties::Unknown:
      ;
    }
    if (int bits = mp4Properties->bitsPerSample(); bits > 0) {
      f.m_detailInfo.format += QLatin1Char(' ');
      f.m_detailInfo.format += QString::number(bits);
      f.m_detailInfo.format += QLatin1String(" bit");
    }
#endif
    return true;
  }
  return false;
}

QString TagLibMp4Support::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType& type) const
{
  if (dynamic_cast<const TagLib::MP4::Tag*>(tag) != nullptr) {
    type = TaggedFile::TT_Mp4;
    return QLatin1String("MP4");
  }
  return {};
}

bool TagLibMp4Support::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      if (Frame::ExtendedType extendedType = frame.getExtendedType();
          extendedType.getType() == Frame::FT_Picture) {
        if (f.m_extraFrames.isRead()) {
          if (int idx = Frame::fromNegativeIndex(frame.getIndex());
              idx >= 0 && idx < f.m_extraFrames.size()) {
            if (Frame newFrame(frame);
                PictureFrame::areFieldsEqual(f.m_extraFrames[idx], newFrame)) {
              f.m_extraFrames[idx].setValueChanged(false);
            } else {
              f.m_extraFrames[idx] = newFrame;
              f.markTagChanged(tagNr, extendedType);
            }
            return true;
          }
          return false;
        }
      }
      setMp4Frame(f, frame, mp4Tag);
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibMp4Support::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
  if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[tagNr])) {
    if (frame.getType() == Frame::FT_Picture) {
      if (frame.getFieldList().empty()) {
        PictureFrame::setFields(frame);
      }
      if (f.m_extraFrames.isRead()) {
        frame.setIndex(Frame::toNegativeIndex(static_cast<int>(f.m_extraFrames.size())));
        f.m_extraFrames.append(frame);
        f.markTagChanged(tagNr, frame.getExtendedType());
        return true;
      }
    }
    TagLib::String name;
    TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
    if (!item.isValid()) {
      return false;
    }
    frame.setExtendedType(Frame::ExtendedType(frame.getType(),
                                              toQString(name)));
    prefixMp4FreeFormName(name, mp4Tag);
#if TAGLIB_VERSION >= 0x020200
    if (frame.getInternalName() == QLatin1String("stem")) {
      frame.fieldList() = {{Frame::ID_Data, QByteArray()}};
    }
#endif
#if TAGLIB_VERSION >= 0x010b01
    mp4Tag->setItem(name, item);
    const TagLib::MP4::ItemMap& itemListMap = mp4Tag->itemMap();
#else
    mp4Tag->itemListMap()[name] = item;
    const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
#endif
    int index = 0;
    bool found = false;
    for (const auto& [itemName, itm] : itemListMap) {
      if (itemName == name) {
        found = true;
        break;
      }
      ++index;
    }
    frame.setIndex(found ? index : -1);
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibMp4Support::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[tagNr])) {
    if (frame.getType() == Frame::FT_Picture) {
      if (f.m_extraFrames.isRead()) {
        if (int idx = Frame::fromNegativeIndex(frame.getIndex());
            idx >= 0 && idx < f.m_extraFrames.size()) {
          f.m_extraFrames.removeAt(idx);
          while (idx < f.m_extraFrames.size()) {
            f.m_extraFrames[idx].setIndex(Frame::toNegativeIndex(idx));
            ++idx;
          }
          f.markTagChanged(tagNr, frame.getExtendedType());
          return true;
        }
      }
    }
    TagLib::String name = toTString(frame.getInternalName());
    prefixMp4FreeFormName(name, mp4Tag);
#if TAGLIB_VERSION >= 0x010b01
    mp4Tag->removeItem(name);
#else
    mp4Tag->itemListMap().erase(name);
#endif
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibMp4Support::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[tagNr])) {
    const auto& itemMap = mp4Tag->itemMap();
    if (flt.areAllEnabled()) {
#if TAGLIB_VERSION >= 0x010b01
      for (auto it = itemMap.begin(); it != itemMap.end();) {
        mp4Tag->removeItem((it++)->first);
      }
#else
      mp4Tag->itemListMap().clear();
#endif
      f.m_extraFrames.clear();
    } else {
#if TAGLIB_VERSION >= 0x010b01
      for (auto it = itemMap.begin(); it != itemMap.end();) {
        TagLib::String name = it->first;
        stripMp4FreeFormName(name);
        Frame::Type type;
        Mp4ValueType valueType;
        getMp4TypeForName(name, type, valueType);
        if (flt.isEnabled(type, toQString(name))) {
          mp4Tag->removeItem((it++)->first);
        } else {
          ++it;
        }
      }
#else
      TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
      Frame::Type type;
      Mp4ValueType valueType;
      for (auto it = itemListMap.begin();
           it != itemListMap.end();) {
        TagLib::String name = it->first;
        stripMp4FreeFormName(name);
        getMp4TypeForName(name, type, valueType);
        if (flt.isEnabled(type, toQString(name))) {
          itemListMap.erase(it++);
        } else {
          ++it;
        }
      }
#endif
      if (flt.isEnabled(Frame::FT_Picture)) {
        f.m_extraFrames.clear();
      }
    }
    f.markTagChanged(tagNr, Frame::ExtendedType());
    return true;
  }
  return false;
}

bool TagLibMp4Support::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(f.m_tag[tagNr])) {
#if TAGLIB_VERSION >= 0x010b01
    const TagLib::MP4::ItemMap& itemListMap = mp4Tag->itemMap();
#else
    const TagLib::MP4::ItemListMap& itemListMap = mp4Tag->itemListMap();
#endif
    int i = 0;
    for (const auto& [itemName, item] : itemListMap) {
      TagLib::String name = itemName;
      stripMp4FreeFormName(name);
      Frame::Type type;
      Mp4ValueType valueType;
      getMp4TypeForName(name, type, valueType);
      QString value;
      switch (valueType) {
      case MVT_String:
      {
        TagLib::StringList strings = item.toStringList();
        value = strings.size() > 0
            ? joinToQString(strings)
            : QLatin1String("");
        break;
      }
      case MVT_Bool:
        value = item.toBool() ? QLatin1String("1") : QLatin1String("0");
        break;
      case MVT_Int:
        value.setNum(item.toInt());
        break;
      case MVT_IntPair:
      {
        auto [first, second] = item.toIntPair();
        value.setNum(first);
        if (second != 0) {
          value += QLatin1Char('/');
          value += QString::number(second);
        }
        break;
      }
      case MVT_CoverArt:
        // handled by m_extraFrames
        break;
#if TAGLIB_VERSION >= 0x020200
      case MVT_Stem:
        // handled below
        break;
#endif
      case MVT_Byte:
        value.setNum(item.toByte());
        break;
      case MVT_UInt:
        value.setNum(item.toUInt());
        break;
      case MVT_LongLong:
        value.setNum(item.toLongLong());
        break;
      case MVT_ByteArray:
      default:
        // binary data and album art are not handled by TagLib
        value = QLatin1String("");
      }
#if TAGLIB_VERSION >= 0x020200
      if (valueType == MVT_Stem) {
        Frame stemFrame(type, value, toQString(name), i++);
        TagLib::ByteVector bv = item.toStem().data();
        stemFrame.fieldList() = {{
              Frame::ID_Data,
              QByteArray(bv.data(), static_cast<int>(bv.size()))
        }};
        frames.insert(stemFrame);
      } else
#endif
      if (type != Frame::FT_Picture) {
        frames.insert(
          Frame(type, value, toQString(name), i++));
      }
    }
    if (f.m_extraFrames.isRead()) {
      for (auto it = f.m_extraFrames.constBegin(); it != f.m_extraFrames.constEnd(); ++it) {
        frames.insert(*it);
      }
    }
    return true;
  }
  return false;
}

QStringList TagLibMp4Support::getFrameIds(
  const TagLibFile& f, Frame::TagNumber tagNr) const
{
  QStringList lst;
  if (f.m_tagType[tagNr] == TaggedFile::TT_Mp4) {
    Mp4ValueType valueType;
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      TagLib::String name = "";
      auto type = static_cast<Frame::Type>(k);
      getMp4NameForType(type, name, valueType);
      if (!name.isEmpty() && valueType != MVT_ByteArray &&
          !(name[0] >= 'A' && name[0] <= 'Z')) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName()); // clazy:exclude=reserve-candidates
      }
    }
    for (const auto& [name, type, value] : mp4NameTypeValues) {
      if (type == Frame::FT_Other &&
          value != MVT_ByteArray &&
          !(name[0] >= 'A' &&
            name[0] <= 'Z')) {
        lst.append(QString::fromLatin1(name));
      }
    }
  }
  return lst;
}

void TagLibMp4Support::setTagValue(TagLibFile& f, Frame::TagNumber tagNr, Frame::Type type, const TagLib::String& str) const
{
  TagLib::Tag* tag = f.m_tag[tagNr];
  if (type == Frame::FT_Date) {
    if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(tag)) {
      TagLib::String name;
      Mp4ValueType valueType;
      getMp4NameForType(type, name, valueType);
      auto item = TagLib::MP4::Item(str);
      if (valueType == MVT_String && item.isValid()) {
#if TAGLIB_VERSION >= 0x010b01
        mp4Tag->setItem(name, item);
#else
        mp4Tag->itemListMap()[name] = item;
#endif
        return;
      }
    }
  } else if (type == Frame::FT_Track) {
    if (auto mp4Tag = dynamic_cast<TagLib::MP4::Tag*>(tag)) {
      // Set a frame in order to store the total number too.
      Frame trackFrame(Frame::FT_Track,  toQString(str),
        QLatin1String(""), -1);
      setMp4Frame(f, trackFrame, mp4Tag);
      return;
    }
  }
  TagLibFormatSupport::setTagValue(f, tagNr, type, str);
}

/**
 * Set a frame in an MP4 tag.
 * @param f     file
 * @param frame frame to set
 * @param mp4Tag MP4 tag
 */
void TagLibMp4Support::setMp4Frame(
  TagLibFile& f, const Frame& frame, TagLib::MP4::Tag* mp4Tag)
{
  TagLib::String name;
  if (TagLib::MP4::Item item = getMp4ItemForFrame(frame, name);
      item.isValid()) {
    if (int numTracks;
        name == "trkn" &&
        (numTracks = f.getTotalNumberOfTracksIfEnabled()) > 0) {
      if (auto [first, second] = item.toIntPair(); second == 0) {
        item = TagLib::MP4::Item(first, numTracks);
      }
    }
    prefixMp4FreeFormName(name, mp4Tag);
#if TAGLIB_VERSION >= 0x010b01
    mp4Tag->setItem(name, item);
#else
    mp4Tag->itemListMap()[name] = item;
#endif
    f.markTagChanged(Frame::Tag_2, frame.getExtendedType());
  }
}
