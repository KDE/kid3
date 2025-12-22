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

#include "pictureframe.h"
#include "taglibutils.h"
#include "taglibfile.h"
#include "tpropertymap.h"

using namespace TagLibUtils;

namespace {

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
const char* getPropertyNameFromType(Frame::Type type)
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
    "ENCODEDBY",       // FT_EncodedBy,
    "ENCODING",        // FT_EncoderSettings,
    "ENCODINGTIME",    // FT_EncodingTime,
    "GROUPING",        // FT_Grouping,
    "INITIALKEY",      // FT_InitialKey,
    "ISRC",            // FT_Isrc,
    "LANGUAGE",        // FT_Language,
    "LYRICIST",        // FT_Lyricist,
    "LYRICS",          // FT_Lyrics,
    "MEDIA",           // FT_Media,
    "MOOD",            // FT_Mood,
    "ORIGINALALBUM",   // FT_OriginalAlbum,
    "ORIGINALARTIST",  // FT_OriginalArtist,
    "ORIGINALDATE",    // FT_OriginalDate,
    "DESCRIPTION",     // FT_Description,
    "PERFORMER",       // FT_Performer,
    "PICTURE",         // FT_Picture,
    "LABEL",           // FT_Publisher,
    "RELEASECOUNTRY",  // FT_ReleaseCountry,
    "REMIXER",         // FT_Remixer,
    "ALBUMSORT",       // FT_SortAlbum,
    "ALBUMARTISTSORT", // FT_SortAlbumArtist,
    "ARTISTSORT",      // FT_SortArtist,
    "COMPOSERSORT",    // FT_SortComposer,
    "TITLESORT",       // FT_SortName,
    "SUBTITLE",        // FT_Subtitle,
    "ARTISTWEBPAGE",   // FT_Website,
    "FILEWEBPAGE",     // FT_WWWAudioFile,
    "AUDIOSOURCEWEBPAGE", // FT_WWWAudioSource,
    "RELEASEDATE",     // FT_ReleaseDate,
    "RATING",          // FT_Rating,
    "WORK"             // FT_Work,
                       // FT_Custom1
  };
  Q_STATIC_ASSERT(std::size(names) == Frame::FT_Custom1);
  if (Frame::isCustomFrameType(type)) {
    return Frame::getNameForCustomFrame(type);
  }
  return type < Frame::FT_Custom1 ? names[type] : "UNKNOWN";
}

/**
 * Get the frame type for a property name.
 *
 * @param name property key
 *
 * @return frame type.
 */
Frame::Type getTypeFromPropertyName(const QString& name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i < Frame::FT_Custom1; ++i) {
      auto type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getPropertyNameFromType(type)), type);
    }
  }
  if (auto it = strNumMap.constFind(name.toUpper());
      it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::getTypeFromCustomFrameName(name.toLatin1());
}

QString getPropertyName(const Frame& frame)
{
  if (Frame::Type type = frame.getType(); type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getPropertyNameFromType(type));
  }
  return TaggedFile::fixUpTagKey(frame.getName(), TaggedFile::TT_Vorbis).toUpper();
}

#if TAGLIB_VERSION >= 0x020000
void propertyPictureToFrame(const TagLib::VariantMap& property, Frame& frame)
{
  auto mimeType = property.value("mimeType").value<TagLib::String>();
  auto pictureType = property.value("pictureType").value<TagLib::String>();
  auto description = property.value("description").value<TagLib::String>();
  auto data = property.value("data").value<TagLib::ByteVector>();
  QByteArray ba(data.data(), data.size());
  bool hasImgProps = property.contains("width");
  auto imgProps = hasImgProps
    ? PictureFrame::ImageProperties(
      property.value("width").value<int>(),
      property.value("height").value<int>(),
      property.value("depth").value<int>(),
      property.value("numColors").value<int>(),
      ba)
    : PictureFrame::ImageProperties();
  PictureFrame::setFields(
    frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), toQString(mimeType),
    PictureFrame::getPictureTypeFromString(pictureType.toCString()),
    toQString(description), ba, hasImgProps ? &imgProps : nullptr);
}

void frameToPropertyPicture(const Frame& frame, TagLib::VariantMap* property)
{
  Frame::TextEncoding enc;
  QString imgFormat;
  QString mimeType;
  PictureFrame::PictureType pictureType;
  QString description;
  QByteArray data;
  PictureFrame::ImageProperties imgProps;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, data, &imgProps);
  property->insert("mimeType", toTString(mimeType));
  property->insert("pictureType",
    TagLib::String(PictureFrame::getPictureTypeString(pictureType)));
  property->insert("description", toTString(description));
  property->insert("data", TagLib::ByteVector(data.constData(), data.size()));
  if (!imgProps.isNull()) {
    property->insert("width", imgProps.width());
    property->insert("height", imgProps.height());
    property->insert("depth", imgProps.depth());
    property->insert("numColors", imgProps.numColors());
  }
}
#endif

}

TagLib::File* TagLibGenericSupport::createFromExtension(
  TagLib::IOStream*, const TagLib::String&) const
{
  return nullptr;
}

bool TagLibGenericSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (file) {
    const QString& filename = f.getFilename();
    int dotPos = filename.lastIndexOf(QLatin1Char('.'));
    if (dotPos != -1) {
      f.m_fileExtension = filename.mid(dotPos);
      putFileRefTagInTag2(f);
#if TAGLIB_VERSION >= 0x020000
      if (!f.m_extraFrames.isRead()) {
        const auto pics = file->complexProperties("PICTURE");
        int i = 0;
        for (const auto& pic : pics) {
          PictureFrame frame;
          propertyPictureToFrame(pic, frame);
          frame.setIndex(Frame::toNegativeIndex(i++));
          f.m_extraFrames.append(frame);
        }
        f.m_extraFrames.setRead(true);
        return true;
      }
#endif
    }
  }
  return false;
}

bool TagLibGenericSupport::writeFile(TagLibFile& f, TagLib::File*, bool force,
  int, bool& fileChanged) const
{
  if (anyTagMustBeSaved(f, force)) {
#if TAGLIB_VERSION >= 0x020000
    if (f.m_extraFrames.isRead()) {
      TagLib::List<TagLib::VariantMap> props;
      const auto frames = f.m_extraFrames;
      for (const Frame& frame : frames) {
        if (frame.getType() == Frame::FT_Picture) {
          TagLib::VariantMap prop;
          frameToPropertyPicture(frame, &prop);
          props.append(prop);
        }
      }
      f.m_fileRef.setComplexProperties("PICTURE", props);
    }
#endif
    if (saveFileRef(f)) {
      fileChanged = true;
    }
  }
  return true;
}

bool TagLibGenericSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties*) const
{
  f.m_detailInfo.format = f.getFileExtension().mid(1).toUpper();
  return true;
}

QString TagLibGenericSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType&) const
{
  if (tag) {
    return QLatin1String("TagLib");
  }
  return {};
}

bool TagLibGenericSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (TagLib::Tag* tag;
      tagNr == Frame::Tag_2 && (tag = f.m_tag[tagNr]) != nullptr) {
    if (Frame::ExtendedType extendedType = frame.getExtendedType();
        extendedType.getType() == Frame::FT_Picture) {
#if TAGLIB_VERSION >= 0x020000
      if (f.m_extraFrames.isRead()) {
        if (int idx = Frame::fromNegativeIndex(frame.getIndex());
            idx >= 0 && idx < f.m_extraFrames.size()) {
          Frame newFrame(frame);
          PictureFrame::setDescription(newFrame, frame.getValue());
          if (PictureFrame::areFieldsEqual(f.m_extraFrames[idx], newFrame)) {
            f.m_extraFrames[idx].setValueChanged(false);
          } else {
            f.m_extraFrames[idx] = newFrame;
            f.markTagChanged(tagNr, extendedType);
          }
          return true;
        }
      }
#endif
      return false;
    }

    if (frame.getIndex() != -1) {
      QString name = getPropertyName(frame);
      TagLib::String key = toTString(name);
      TagLib::StringList value = splitToTStringList(frame.getValue());
#if TAGLIB_VERSION >= 0x020000
      TagLib::PropertyMap propertyMap = f.m_fileRef.properties();
      propertyMap[key] = value;
      f.m_fileRef.setProperties(propertyMap);
#else
      TagLib::PropertyMap propertyMap = tag->properties();
      propertyMap[key] = value;
      tag->setProperties(propertyMap);
#endif
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibGenericSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
  if (TagLib::Tag* tag;
      tagNr == Frame::Tag_2 && (tag = f.m_tag[tagNr]) != nullptr) {
    if (frame.getType() == Frame::FT_Picture) {
#if TAGLIB_VERSION >= 0x020000
      if (f.m_extraFrames.isRead()) {
        if (frame.getFieldList().isEmpty()) {
          PictureFrame::setFields(
            frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
            QLatin1String("image/jpeg"), PictureFrame::PT_CoverFront, QLatin1String(""),
            QByteArray());
        }
        PictureFrame::setDescription(frame, frame.getValue());
        frame.setIndex(Frame::toNegativeIndex(f.m_extraFrames.size()));
        f.m_extraFrames.append(frame);
        f.markTagChanged(tagNr, frame.getExtendedType());
        return true;
      }
#endif
      return false;
    }
    QString name = getPropertyName(frame);
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
#if TAGLIB_VERSION >= 0x020000
    TagLib::PropertyMap propertyMap = f.m_fileRef.properties();
#else
    TagLib::PropertyMap propertyMap = tag->properties();
#endif
    TagLib::String key = toTString(name);
    TagLib::StringList value = splitToTStringList(frame.getValue());
    propertyMap[key] = value;
    int index = 0;
    for (auto& [propertyName, stringList] : propertyMap) {
      if (propertyName == key) {
        frame.setIndex(index);
        break;
      }
      ++index;
    }
#if TAGLIB_VERSION >= 0x020000
    f.m_fileRef.setProperties(propertyMap);
#else
    tag->setProperties(propertyMap);
#endif
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibGenericSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (TagLib::Tag* tag;
      tagNr == Frame::Tag_2 && (tag = f.m_tag[tagNr]) != nullptr) {
    if (frame.getType() == Frame::FT_Picture) {
#if TAGLIB_VERSION >= 0x020000
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
#endif
      return false;
    }

    TagLib::String key = toTString(frame.getInternalName());
#if TAGLIB_VERSION >= 0x020000
    TagLib::PropertyMap propertyMap = f.m_fileRef.properties();
    propertyMap.erase(key);
    f.m_fileRef.setProperties(propertyMap);
#else
    TagLib::PropertyMap propertyMap = tag->properties();
    propertyMap.erase(key);
    tag->setProperties(propertyMap);
#endif
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibGenericSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (TagLib::Tag* tag;
      tagNr == Frame::Tag_2 && (tag = f.m_tag[tagNr]) != nullptr) {
#if TAGLIB_VERSION >= 0x020000
    TagLib::PropertyMap propertyMap = f.m_fileRef.properties();
#else
    TagLib::PropertyMap propertyMap = tag->properties();
#endif
    if (flt.areAllEnabled()) {
      propertyMap.clear();
#if TAGLIB_VERSION >= 0x020000
      f.m_fileRef.removeUnsupportedProperties(propertyMap.unsupportedData());
#else
      tag->removeUnsupportedProperties(propertyMap.unsupportedData());
#endif
      f.m_extraFrames.clear();
    } else {
      TagLib::StringList keys;
      for (auto& [propertyName, stringList] : propertyMap) {
        keys.append(propertyName);
      }
      for (const auto& key : keys) {
        if (QString name = toQString(key);
            flt.isEnabled(getTypeFromPropertyName(name), name)) {
          propertyMap.erase(key);
        }
      }
#if TAGLIB_VERSION >= 0x020000
      if (flt.isEnabled(Frame::FT_Picture)) {
        f.m_extraFrames.clear();
      }
#endif
    }
#if TAGLIB_VERSION >= 0x020000
    f.m_fileRef.setProperties(propertyMap);
#else
    tag->setProperties(propertyMap);
#endif
    f.markTagChanged(tagNr, Frame::ExtendedType());
    return true;
  }
  return false;
}

bool TagLibGenericSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (TagLib::Tag* tag;
      tagNr == Frame::Tag_2 && (tag = f.m_tag[tagNr]) != nullptr) {
#if TAGLIB_VERSION >= 0x020000
    const TagLib::PropertyMap propertyMap = f.m_fileRef.properties();
#else
    const TagLib::PropertyMap propertyMap = tag->properties();
#endif
    int i = 0;
    for (const auto& [propteryName, stringList] : propertyMap) {
      QString name = toQString(propteryName);
      Frame::Type type = getTypeFromPropertyName(name);
      frames.insert(Frame(type, joinToQString(stringList), name, i++));
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
