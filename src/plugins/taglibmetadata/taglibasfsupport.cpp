/**
 * \file taglibasfsupport.cpp
 * Support for WMA files and ASF tags.
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

#include "taglibasfsupport.h"

#include <asffile.h>

#include "attributedata.h"
#include "pictureframe.h"
#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

/** Indices of fixed ASF frames. */
enum AsfFrameIndex {
  AFI_Title,
  AFI_Artist,
  AFI_Comment,
  AFI_Copyright,
  AFI_Rating,
  AFI_Attributes
};

/** ASF name, frame type and value type. */
struct AsfNameTypeValue {
  const char* name;
  Frame::Type type;
  TagLib::ASF::Attribute::AttributeTypes value;
};

/** Mapping between frame types and field names. */
const AsfNameTypeValue asfNameTypeValues[] = {
  { "Title", Frame::FT_Title, TagLib::ASF::Attribute::UnicodeType },
  { "Author", Frame::FT_Artist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AlbumTitle", Frame::FT_Album, TagLib::ASF::Attribute::UnicodeType },
  { "Description", Frame::FT_Comment, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Year", Frame::FT_Date, TagLib::ASF::Attribute::UnicodeType },
  { "Copyright", Frame::FT_Copyright, TagLib::ASF::Attribute::UnicodeType },
  { "Rating Information", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/TrackNumber", Frame::FT_Track, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Track", Frame::FT_Track, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Genre", Frame::FT_Genre, TagLib::ASF::Attribute::UnicodeType },
  { "WM/GenreID", Frame::FT_Genre, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AlbumArtist", Frame::FT_AlbumArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AlbumSortOrder", Frame::FT_SortAlbum, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ArtistSortOrder", Frame::FT_SortArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/TitleSortOrder", Frame::FT_SortName, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Producer", Frame::FT_Arranger, TagLib::ASF::Attribute::UnicodeType },
  { "WM/BeatsPerMinute", Frame::FT_Bpm, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Composer", Frame::FT_Composer, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Conductor", Frame::FT_Conductor, TagLib::ASF::Attribute::UnicodeType },
  { "WM/PartOfSet", Frame::FT_Disc, TagLib::ASF::Attribute::UnicodeType },
  { "WM/EncodedBy", Frame::FT_EncodedBy, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ContentGroupDescription", Frame::FT_Work, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ISRC", Frame::FT_Isrc, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Language", Frame::FT_Language, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Writer", Frame::FT_Lyricist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Lyrics", Frame::FT_Lyrics, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AudioSourceURL", Frame::FT_WWWAudioSource, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalAlbumTitle", Frame::FT_OriginalAlbum, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalArtist", Frame::FT_OriginalArtist, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalReleaseYear", Frame::FT_OriginalDate, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SubTitleDescription", Frame::FT_Description, TagLib::ASF::Attribute::UnicodeType },
  { "WM/Picture", Frame::FT_Picture, TagLib::ASF::Attribute::BytesType },
  { "WM/Publisher", Frame::FT_Publisher, TagLib::ASF::Attribute::UnicodeType },
  { "WM/ModifiedBy", Frame::FT_Remixer, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SubTitle", Frame::FT_Subtitle, TagLib::ASF::Attribute::UnicodeType },
  { "WM/AuthorURL", Frame::FT_Website, TagLib::ASF::Attribute::UnicodeType },
  { "AverageLevel", Frame::FT_Other, TagLib::ASF::Attribute::DWordType },
  { "PeakValue", Frame::FT_Other, TagLib::ASF::Attribute::DWordType },
  { "WM/AudioFileURL", Frame::FT_WWWAudioFile, TagLib::ASF::Attribute::UnicodeType },
  { "WM/EncodingSettings", Frame::FT_EncoderSettings, TagLib::ASF::Attribute::UnicodeType },
  { "WM/EncodingTime", Frame::FT_EncodingTime, TagLib::ASF::Attribute::BytesType },
  { "WM/InitialKey", Frame::FT_InitialKey, TagLib::ASF::Attribute::UnicodeType },
  // incorrect WM/Lyrics_Synchronised data make file inaccessible in Windows
  // { "WM/Lyrics_Synchronised", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
  { "WM/MCDI", Frame::FT_Other, TagLib::ASF::Attribute::BytesType },
  { "WM/MediaClassPrimaryID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/MediaClassSecondaryID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/Mood", Frame::FT_Mood, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalFilename", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/OriginalLyricist", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/PromotionURL", Frame::FT_Other, TagLib::ASF::Attribute::UnicodeType },
  { "WM/SharedUserRating", Frame::FT_Rating, TagLib::ASF::Attribute::UnicodeType },
  { "WM/WMCollectionGroupID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/WMCollectionID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType },
  { "WM/WMContentID", Frame::FT_Other, TagLib::ASF::Attribute::GuidType }
};

/**
 * Get ASF name and value type for a frame type.
 *
 * @param type  frame type
 * @param name  the ASF name is returned here
 * @param value the ASF value type is returned here
 */
void getAsfNameForType(Frame::Type type, TagLib::String& name,
                       TagLib::ASF::Attribute::AttributeTypes& value)
{
  static QMap<Frame::Type, unsigned> typeNameMap;
  if (typeNameMap.empty()) {
    // first time initialization
    for (unsigned i = 0; i < std::size(asfNameTypeValues); ++i) {
      if (asfNameTypeValues[i].type != Frame::FT_Other &&
          !typeNameMap.contains(asfNameTypeValues[i].type)) {
        typeNameMap.insert(asfNameTypeValues[i].type, i);
      }
    }
  }
  name = "";
  value = TagLib::ASF::Attribute::UnicodeType;
  if (type != Frame::FT_Other) {
    if (auto it = typeNameMap.constFind(type);
        it != typeNameMap.constEnd()) {
      name = asfNameTypeValues[*it].name;
      value = asfNameTypeValues[*it].value;
    } else {
      if (auto customFrameName = Frame::getNameForCustomFrame(type);
          !customFrameName.isEmpty()) {
        name = TagLib::String(customFrameName.constData());
      }
    }
  }
}

/**
 * Get ASF value type and frame type for an ASF name.
 *
 * @param name  ASF name
 * @param type  the frame type is returned here
 * @param value the ASF value type is returned here
 */
void getAsfTypeForName(const TagLib::String& name, Frame::Type& type,
                       TagLib::ASF::Attribute::AttributeTypes& value)
{
  static QMap<TagLib::String, unsigned> nameTypeMap;
  if (nameTypeMap.empty()) {
    // first time initialization
    for (unsigned i = 0; i < std::size(asfNameTypeValues); ++i) {
      nameTypeMap.insert(asfNameTypeValues[i].name, i);
    }
  }
  if (auto it = nameTypeMap.constFind(name);
      it != nameTypeMap.constEnd()) {
    type = asfNameTypeValues[*it].type;
    value = asfNameTypeValues[*it].value;
  } else {
    type = Frame::getTypeFromCustomFrameName(name.toCString());
    value = TagLib::ASF::Attribute::UnicodeType;
  }
}

/**
 * Get an ASF type for a frame.
 *
 * @param frame frame
 * @param name  the name for the attribute is returned here
 * @param value the ASF value type is returned here
 */
void getAsfTypeForFrame(const Frame& frame, TagLib::String& name,
                        TagLib::ASF::Attribute::AttributeTypes& value)
{
  if (frame.getType() != Frame::FT_Other) {
    getAsfNameForType(frame.getType(), name, value);
    if (name.isEmpty()) {
      name = toTString(frame.getInternalName());
    }
  } else {
    Frame::Type type;
    name = toTString(TaggedFile::fixUpTagKey(frame.getInternalName(),
                                             TaggedFile::TT_Asf));
    getAsfTypeForName(name, type, value);
  }
}

/**
 * Get a picture frame from a WM/Picture.
 *
 * @param picture ASF picture
 * @param frame   the picture frame is returned here
 *
 * @return true if ok.
 */
bool parseAsfPicture(const TagLib::ASF::Picture& picture, Frame& frame)
{
  if (!picture.isValid())
    return false;

  TagLib::ByteVector data = picture.picture();
  QString description(toQString(picture.description()));
  PictureFrame::setFields(frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
                          toQString(picture.mimeType()),
                          static_cast<PictureFrame::PictureType>(picture.type()),
                          description,
                          QByteArray(data.data(), data.size()));
  frame.setType(Frame::FT_Picture);
  return true;
}

/**
 * Render the bytes of a WM/Picture from a picture frame.
 *
 * @param frame   picture frame
 * @param picture the ASF picture is returned here
 */
void renderAsfPicture(const Frame& frame, TagLib::ASF::Picture& picture)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType;
  QByteArray data;
  QString imgFormat, mimeType, description;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, data);

  if (frame.isValueChanged()) {
    description = frame.getValue();
  }
  picture.setMimeType(toTString(mimeType));
  picture.setType(static_cast<TagLib::ASF::Picture::Type>(pictureType));
  picture.setDescription(toTString(description));
  picture.setPicture(TagLib::ByteVector(data.data(), data.size()));
}

/**
 * Get an ASF attribute for a frame.
 *
 * @param frame     frame
 * @param valueType ASF value type
 *
 * @return ASF attribute, an empty attribute is returned if not supported.
 */
TagLib::ASF::Attribute getAsfAttributeForFrame(
  const Frame& frame,
  TagLib::ASF::Attribute::AttributeTypes valueType)
{
  switch (valueType) {
  case TagLib::ASF::Attribute::UnicodeType:
    return toTString(frame.getValue());
  case TagLib::ASF::Attribute::BoolType:
    return frame.getValue() == QLatin1String("1");
  case TagLib::ASF::Attribute::WordType:
    return frame.getValue().toUShort();
  case TagLib::ASF::Attribute::DWordType:
    return frame.getValue().toUInt();
  case TagLib::ASF::Attribute::QWordType:
    return frame.getValue().toULongLong();
  case TagLib::ASF::Attribute::BytesType:
  case TagLib::ASF::Attribute::GuidType:
  default:
    if (frame.getType() != Frame::FT_Picture) {
      QByteArray ba;
      if (AttributeData(frame.getInternalName()).toByteArray(frame.getValue(), ba)) {
        return TagLib::ByteVector(ba.data(), ba.size());
      }
      if (QVariant fieldValue = frame.getFieldValue(Frame::ID_Data);
          fieldValue.isValid()) {
        ba = fieldValue.toByteArray();
        return TagLib::ByteVector(ba.data(), ba.size());
      }
    }
    else {
      TagLib::ASF::Picture picture;
      renderAsfPicture(frame, picture);
      return picture;
    }
  }
  return {};
}

}


TagLib::File* TagLibAsfSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "WMA" || ext == "ASF" || ext == "WMV")
    return new TagLib::ASF::File(stream);
  return nullptr;
}

bool TagLibAsfSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (dynamic_cast<TagLib::ASF::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".wma");
    putFileRefTagInTag2(f);
    return true;
  }
  return false;
}

bool TagLibAsfSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (dynamic_cast<TagLib::ASF::Properties*>(audioProperties) != nullptr) {
    f.m_detailInfo.format = QLatin1String("ASF");
    return true;
  }
  return false;
}

QString TagLibAsfSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType& type) const
{
  if (dynamic_cast<const TagLib::ASF::Tag*>(tag) != nullptr) {
    type = TaggedFile::TT_Asf;
    return QLatin1String("ASF");
  }
  return {};
}

bool TagLibAsfSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto asfTag = dynamic_cast<TagLib::ASF::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      switch (index) {
      case AFI_Title:
        asfTag->setTitle(toTString(frame.getValue()));
        break;
      case AFI_Artist:
        asfTag->setArtist(toTString(frame.getValue()));
        break;
      case AFI_Comment:
        asfTag->setComment(toTString(frame.getValue()));
        break;
      case AFI_Copyright:
        asfTag->setCopyright(toTString(frame.getValue()));
        break;
      case AFI_Rating:
        asfTag->setRating(toTString(frame.getValue()));
        break;
      case AFI_Attributes:
      default:
      {
        TagLib::String name;
        TagLib::ASF::Attribute::AttributeTypes valueType;
        getAsfTypeForFrame(frame, name, valueType);
        TagLib::ASF::Attribute attribute =
          getAsfAttributeForFrame(frame, valueType);
        if (TagLib::ASF::AttributeListMap& attrListMap =
              asfTag->attributeListMap();
            attrListMap.contains(name) && attrListMap[name].size() > 1) {
          int i = AFI_Attributes;
          bool found = false;
          for (auto& [attrName, attrList] : attrListMap) {
            for (TagLib::ASF::Attribute& attr : attrList) {
              if (i++ == index) {
                found = true;
                attr = attribute;
                break;
              }
            }
            if (found) {
              break;
            }
          }
        } else {
          asfTag->setAttribute(name, attribute);
        }
      }
      }
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibAsfSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
  if (auto asfTag = dynamic_cast<TagLib::ASF::Tag*>(f.m_tag[tagNr])) {
    if (frame.getType() == Frame::FT_Picture &&
        frame.getFieldList().empty()) {
      PictureFrame::setFields(frame);
    }
    TagLib::String name;
    TagLib::ASF::Attribute::AttributeTypes valueType;
    getAsfTypeForFrame(frame, name, valueType);
    if (valueType == TagLib::ASF::Attribute::BytesType &&
        frame.getType() != Frame::FT_Picture) {
      Frame::Field field;
      field.m_id = Frame::ID_Data;
      field.m_value = QByteArray();
      frame.fieldList().push_back(field);
    }
    TagLib::ASF::Attribute attribute = getAsfAttributeForFrame(frame, valueType);
    asfTag->addAttribute(name, attribute);
    frame.setExtendedType(Frame::ExtendedType(frame.getType(),
                                              toQString(name)));

    const TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
    int index = AFI_Attributes;
    bool found = false;
    for (const auto& [attrName, attrList] : attrListMap) {
      if (attrName == name) {
        index += static_cast<int>(attrList.size()) - 1;
        found = true;
        break;
      }
      index += static_cast<int>(attrList.size());
    }
    frame.setIndex(found ? index : -1);
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibAsfSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto asfTag = dynamic_cast<TagLib::ASF::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      switch (index) {
      case AFI_Title:
        asfTag->setTitle("");
        break;
      case AFI_Artist:
        asfTag->setArtist("");
        break;
      case AFI_Comment:
        asfTag->setComment("");
        break;
      case AFI_Copyright:
        asfTag->setCopyright("");
        break;
      case AFI_Rating:
        asfTag->setRating("");
        break;
      case AFI_Attributes:
      default:
      {
        TagLib::String name = toTString(frame.getInternalName());
        if (TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
            attrListMap.contains(name) && attrListMap[name].size() > 1) {
          int i = AFI_Attributes;
          bool found = false;
          for (auto& [attrName, attrList] : attrListMap) {
            for (auto ait = attrList.begin();
                 ait != attrList.end();
                 ++ait) {
              if (i++ == index) {
                found = true;
                attrList.erase(ait);
                break;
              }
            }
            if (found) {
              break;
            }
          }
        } else {
          asfTag->removeItem(name);
        }
      }
      }
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
  }
  return false;
}

bool TagLibAsfSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (auto asfTag = dynamic_cast<TagLib::ASF::Tag*>(f.m_tag[tagNr])) {
    if (flt.areAllEnabled()) {
      asfTag->setTitle("");
      asfTag->setArtist("");
      asfTag->setComment("");
      asfTag->setCopyright("");
      asfTag->setRating("");
      asfTag->attributeListMap().clear();
    } else {
      if (flt.isEnabled(Frame::FT_Title))
        asfTag->setTitle("");
      if (flt.isEnabled(Frame::FT_Artist))
        asfTag->setArtist("");
      if (flt.isEnabled(Frame::FT_Comment))
        asfTag->setComment("");
      if (flt.isEnabled(Frame::FT_Copyright))
        asfTag->setCopyright("");
      if (flt.isEnabled(Frame::FT_Other, QLatin1String("Rating Information")))
        asfTag->setRating("");

      TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
      Frame::Type type;
      TagLib::ASF::Attribute::AttributeTypes valueType;
      for (auto it = attrListMap.begin();
           it != attrListMap.end();) {
        getAsfTypeForName(it->first, type, valueType);
        if (QString name(toQString(it->first));
            flt.isEnabled(type, name)) {
          attrListMap.erase(it++);
        } else {
          ++it;
        }
      }
    }
    f.markTagChanged(tagNr, Frame::ExtendedType());
    return true;
  }
  return false;
}

bool TagLibAsfSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (auto asfTag = dynamic_cast<TagLib::ASF::Tag*>(f.m_tag[tagNr])) {
    TagLib::String name;
    TagLib::ASF::Attribute::AttributeTypes valueType;
    Frame::Type type = Frame::FT_Title;
    getAsfNameForType(type, name, valueType);
    QString value = toQString(asfTag->title());
    frames.insert(Frame(type, value, toQString(name), AFI_Title));

    type = Frame::FT_Artist;
    getAsfNameForType(type, name, valueType);
    value = toQString(asfTag->artist());
    frames.insert(Frame(type, value, toQString(name), AFI_Artist));

    type = Frame::FT_Comment;
    getAsfNameForType(type, name, valueType);
    value = toQString(asfTag->comment());
    frames.insert(Frame(type, value, toQString(name), AFI_Comment));

    type = Frame::FT_Copyright;
    getAsfNameForType(type, name, valueType);
    value = toQString(asfTag->copyright());
    frames.insert(Frame(type, value, toQString(name), AFI_Copyright));

    name = QT_TRANSLATE_NOOP("@default", "Rating Information");
    getAsfTypeForName(name, type, valueType);
    value = toQString(asfTag->rating());
    frames.insert(Frame(type, value, toQString(name), AFI_Rating));

    int i = AFI_Attributes;
    QByteArray ba;
    const TagLib::ASF::AttributeListMap& attrListMap = asfTag->attributeListMap();
    for (const auto& [attrName, attrList] : attrListMap) {
      name = attrName;
      getAsfTypeForName(name, type, valueType);
      for (const auto& attr : attrList) {
        switch (attr.type()) {
        case TagLib::ASF::Attribute::UnicodeType:
          value = toQString(attr.toString());
          break;
        case TagLib::ASF::Attribute::BoolType:
          value = attr.toBool() ? QLatin1String("1") : QLatin1String("0");
          break;
        case TagLib::ASF::Attribute::DWordType:
          value.setNum(attr.toUInt());
          break;
        case TagLib::ASF::Attribute::QWordType:
          value.setNum(attr.toULongLong());
          break;
        case TagLib::ASF::Attribute::WordType:
          value.setNum(attr.toUShort());
          break;
        case TagLib::ASF::Attribute::BytesType:
        case TagLib::ASF::Attribute::GuidType:
        default:
        {
          TagLib::ByteVector bv = attr.toByteVector();
          ba = QByteArray(bv.data(), bv.size());
          value = QLatin1String("");
          AttributeData(toQString(name)).toString(ba, value);
        }
        }
        Frame frame(type, value, toQString(name), i);
        if (attr.type() == TagLib::ASF::Attribute::BytesType &&
            valueType == TagLib::ASF::Attribute::BytesType) {
          Frame::Field field;
          field.m_id = Frame::ID_Data;
          field.m_value = ba;
          frame.fieldList().push_back(field);
        }
        ++i;
        if (type == Frame::FT_Picture) {
          parseAsfPicture(attr.toPicture(), frame);
        }
        frames.insert(frame);
      }
    }
    return true;
  }
  return false;
}

QStringList TagLibAsfSupport::getFrameIds(
  const TagLibFile& f, Frame::TagNumber tagNr) const
{
  QStringList lst;
  if (f.m_tagType[tagNr] == TaggedFile::TT_Asf) {
    TagLib::ASF::Attribute::AttributeTypes valueType;
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      TagLib::String name = "";
      auto type = static_cast<Frame::Type>(k);
      getAsfNameForType(type, name, valueType);
      if (!name.isEmpty()) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName()); // clazy:exclude=reserve-candidates
      }
    }
    for (const auto& [name, type, value] : asfNameTypeValues) {
      if (type == Frame::FT_Other) {
        lst.append(QString::fromLatin1(name));
      }
    }
  }
  return lst;
}
