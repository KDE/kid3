/**
 * \file taglibapesupport.cpp
 * Support for APE, MPC, WavPack files and tags.
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

#include "taglibapesupport.h"

#include <apefile.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <wavpackfile.h>

#include "pictureframe.h"
#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

/**
 * Get a picture frame from the bytes in an APE cover art frame.
 * The cover art frame has the following data:
 * zero terminated description string (UTF-8), picture data.
 *
 * @param name key of APE item
 * @param data bytes in APE cover art frame
 * @param frame the picture frame is returned here
 */
void parseApePicture(const QString& name,
                     const TagLib::ByteVector& data, Frame& frame)
{
  QByteArray picture;
  TagLib::String description;
  // Do not search for a description if the first byte could start JPG or PNG
  // data.
  if (int picPos = data.isEmpty() || data.at(0) == '\xff' || data.at(0) == '\x89'
                     ? -1 : data.find('\0');
      picPos >= 0) {
    description = TagLib::String(data.mid(0, picPos), TagLib::String::UTF8);
    picture = QByteArray(data.data() + picPos + 1, data.size() - picPos - 1);
  } else {
    picture = QByteArray(data.data(), data.size());
  }
  Frame::PictureType pictureType = Frame::PT_CoverFront;
  if (name.startsWith(QLatin1String("COVER ART (")) &&
      name.endsWith(QLatin1Char(')'))) {
    QString typeStr = name.mid(11);
    typeStr.chop(1);
    pictureType = PictureFrame::getPictureTypeFromString(typeStr.toLatin1());
  }
  PictureFrame::setFields(
        frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
        QLatin1String("image/jpeg"), pictureType,
        toQString(description), picture);
}

/**
 * Render the bytes of an APE cover art frame from a picture frame.
 *
 * @param frame picture frame
 * @param data  the bytes for the APE cover art are returned here
 */
void renderApePicture(const Frame& frame, TagLib::ByteVector& data)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType;
  QByteArray picture;
  QString imgFormat, mimeType, description;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, picture);
  if (frame.isValueChanged()) {
    description = frame.getValue();
  }
  data.append(toTString(description).data(TagLib::String::UTF8));
  data.append('\0');
  data.append(TagLib::ByteVector(picture.constData(), picture.size()));
}

/**
 * Get internal name of an APE picture frame.
 *
 * @param pictureType picture type
 *
 * @return APE key.
 */
TagLib::String getApePictureName(PictureFrame::PictureType pictureType)
{
  TagLib::String name("COVER ART (");
  name += TagLib::String(PictureFrame::getPictureTypeString(pictureType))
      .upper();
  name += ')';
  return name;
}

/**
 * Get internal name of an APE frame.
 *
 * @param frame frame
 *
 * @return APE key.
 */
QString getApeName(const Frame& frame)
{
  if (Frame::Type type = frame.getType(); type == Frame::FT_Date) {
    return QLatin1String("YEAR");
  } else {
    if (type == Frame::FT_Track) {
      return QLatin1String("TRACK");
    }
    if (type == Frame::FT_Picture) {
      PictureFrame::PictureType pictureType;
      if (!PictureFrame::getPictureType(frame, pictureType)) {
        pictureType = Frame::PT_CoverFront;
      }
      return toQString(getApePictureName(pictureType));
    }
    if (type <= Frame::FT_LastFrame) {
      return QString::fromLatin1(getVorbisNameFromType(type));
    }
    return TaggedFile::fixUpTagKey(frame.getName(),
                                   TaggedFile::TT_Ape).toUpper();
  }
}

/**
 * Get the frame type for an APE name.
 *
 * @param name APE tag name
 *
 * @return frame type.
 */
Frame::Type getTypeFromApeName(const QString& name)
{
  Frame::Type type = getTypeFromVorbisName(name);
  if (type == Frame::FT_Other) {
    if (name == QLatin1String("YEAR")) {
      type = Frame::FT_Date;
    } else if (name == QLatin1String("TRACK")) {
      type = Frame::FT_Track;
    } else if (name == QLatin1String("ENCODED BY")) {
      type = Frame::FT_EncodedBy;
    } else if (name.startsWith(QLatin1String("COVER ART"))) {
      type = Frame::FT_Picture;
    }
  }
  return type;
}

}


TagLib::File* TagLibApeSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "APE")
    return new TagLib::APE::File(stream);
  if (ext == "MPC")
    return new TagLib::MPC::File(stream);
  if (ext == "WV")
    return new TagLib::WavPack::File(stream);
  return nullptr;
}

bool TagLibApeSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
#if TAGLIB_VERSION >= 0x010b00
  if (auto mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) {
    f.m_fileExtension = QLatin1String(".mpc");
    f.m_isTagSupported[Frame::Tag_1] = true;
    if (!f.m_tag[Frame::Tag_1]) {
      f.m_tag[Frame::Tag_1] = mpcFile->ID3v1Tag();
      f.markTagUnchanged(Frame::Tag_1);
    }
    if (!f.m_tag[Frame::Tag_2]) {
      f.m_tag[Frame::Tag_2] = mpcFile->APETag();
      f.markTagUnchanged(Frame::Tag_2);
    }
    return true;
  }
  if (auto wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) {
    f.m_fileExtension = QLatin1String(".wv");
    f.m_isTagSupported[Frame::Tag_1] = true;
    if (!f.m_tag[Frame::Tag_1]) {
      f.m_tag[Frame::Tag_1] = wvFile->ID3v1Tag();
      f.markTagUnchanged(Frame::Tag_1);
    }
    if (!f.m_tag[Frame::Tag_2]) {
      f.m_tag[Frame::Tag_2] = wvFile->APETag();
      f.markTagUnchanged(Frame::Tag_2);
    }
    return true;
  }
#endif
  if (auto apeFile = dynamic_cast<TagLib::APE::File*>(file)) {
    f.m_fileExtension = QLatin1String(".ape");
    f.m_isTagSupported[Frame::Tag_1] = true;
    if (!f.m_tag[Frame::Tag_1]) {
      f.m_tag[Frame::Tag_1] = apeFile->ID3v1Tag();
      f.markTagUnchanged(Frame::Tag_1);
    }
    if (!f.m_tag[Frame::Tag_2]) {
      f.m_tag[Frame::Tag_2] = apeFile->APETag();
      f.markTagUnchanged(Frame::Tag_2);
    }
    return true;
  }
  return false;
}

bool TagLibApeSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
                                   int, bool& fileChanged) const
{
  if (auto mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
#if TAGLIB_VERSION >= 0x010b00
      static constexpr int tagTypes[TagLibFile::NUM_TAGS] = {
        TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2,
        TagLib::MPC::File::APE, TagLib::MPC::File::NoTags
      };
      FOR_TAGLIB_TAGS(tagNr) {
        if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr)) &&
            f.m_tag[tagNr]->isEmpty()) {
          mpcFile->strip(tagTypes[tagNr]);
          fileChanged = true;
          f.m_tag[tagNr] = nullptr;
          f.markTagUnchanged(tagNr);
        }
      }
#else
      // it does not work if there is also an ID3 tag (bug in TagLib)
      mpcFile->remove(TagLib::MPC::File::ID3v1 | TagLib::MPC::File::ID3v2);
      fileChanged = true;
#endif
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
  if (auto wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
#if TAGLIB_VERSION >= 0x010b00
      static constexpr int tagTypes[TagLibFile::NUM_TAGS] = {
        TagLib::WavPack::File::ID3v1, TagLib::WavPack::File::APE,
        TagLib::WavPack::File::NoTags
      };
      FOR_TAGLIB_TAGS(tagNr) {
        if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr)) &&
          f.m_tag[tagNr]->isEmpty()) {
          wvFile->strip(tagTypes[tagNr]);
          fileChanged = true;
          f.m_tag[tagNr] = nullptr;
          f.markTagUnchanged(tagNr);
        }
      }
#else
      // it does not work if there is also an ID3 tag (bug in TagLib)
      wvFile->strip(TagLib::WavPack::File::ID3v1);
      fileChanged = true;
#endif
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
  if (auto apeFile = dynamic_cast<TagLib::APE::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      static constexpr int tagTypes[TagLibFile::NUM_TAGS] = {
        TagLib::MPEG::File::ID3v1, TagLib::APE::File::APE,
        TagLib::APE::File::NoTags
      };
      FOR_TAGLIB_TAGS(tagNr) {
        if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr)) && f.m_tag[tagNr]->isEmpty()) {
          apeFile->strip(tagTypes[tagNr]);
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

bool TagLibApeSupport::makeTagSettable(TagLibFile& f, TagLib::File* file,
  Frame::TagNumber tagNr) const
{
  if (tagNr == Frame::Tag_1) {
#if TAGLIB_VERSION >= 0x010b00
    if (auto mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) {
      f.m_tag[tagNr] = mpcFile->ID3v1Tag(true);
      return true;
    }
    if (auto wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) {
      f.m_tag[tagNr] = wvFile->ID3v1Tag(true);
      return true;
    }
#endif
    if (auto apeFile = dynamic_cast<TagLib::APE::File*>(file)) {
      f.m_tag[tagNr] = apeFile->ID3v1Tag(true);
      return true;
    }
  } else if (tagNr == Frame::Tag_2) {
    if (auto mpcFile = dynamic_cast<TagLib::MPC::File*>(file)) {
      f.m_tag[tagNr] = mpcFile->APETag(true);
      return true;
    }
    if (auto wvFile = dynamic_cast<TagLib::WavPack::File*>(file)) {
      f.m_tag[tagNr] = wvFile->APETag(true);
      return true;
    }
    if (auto apeFile = dynamic_cast<TagLib::APE::File*>(file)) {
      f.m_tag[tagNr] = apeFile->APETag(true);
      return true;
    }
  }
  return false;
}

bool TagLibApeSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto apeProperties =
      dynamic_cast<TagLib::APE::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("APE %1.%2 %3 bit"))
      .arg(apeProperties->version() / 1000)
      .arg(apeProperties->version() % 1000)
      .arg(apeProperties->bitsPerSample());
    return true;
  }
  if (dynamic_cast<TagLib::MPC::Properties*>(audioProperties) != nullptr) {
    f.m_detailInfo.format = QLatin1String("MPC");
    return true;
  }
  if (auto wvProperties =
      dynamic_cast<TagLib::WavPack::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QLatin1String("WavPack ");
    f.m_detailInfo.format += QString::number(wvProperties->version(), 16);
    f.m_detailInfo.format += QLatin1Char(' ');
    f.m_detailInfo.format += QString::number(wvProperties->bitsPerSample());
    f.m_detailInfo.format += QLatin1String(" bit");
    return true;
  }
  return false;
}

QString TagLibApeSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType& type) const
{
  if (dynamic_cast<const TagLib::APE::Tag*>(tag) != nullptr) {
    type = TaggedFile::TT_Ape;
    return QLatin1String("APE");
  }
  return {};
}

bool TagLibApeSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto apeTag = dynamic_cast<TagLib::APE::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      if (frame.getType() == Frame::FT_Picture) {
        TagLib::ByteVector data;
        renderApePicture(frame, data);
        QString oldName = frame.getInternalName();
        QString newName = getApeName(frame);
        if (newName != oldName) {
          // If the picture type changes, the frame with the old name has to
          // be replaced with a frame with the new name.
          apeTag->removeItem(toTString(oldName));
        }
        apeTag->setData(toTString(newName), data);
      } else {
        const auto key = toTString(getApeName(frame));
        const auto values = splitToTStringList(frame.getValue());
        apeTag->removeItem(key);
        apeTag->setItem(key, TagLib::APE::Item(key, values));
      }
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibApeSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr,
  Frame& frame) const
{
  if (auto apeTag = dynamic_cast<TagLib::APE::Tag*>(f.m_tag[tagNr])) {
    if (frame.getType() == Frame::FT_Picture &&
        frame.getFieldList().isEmpty()) {
      // Do not replace an already existing picture.
      Frame::PictureType pictureType = Frame::PT_CoverFront;
      const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
      for (int i = Frame::PT_CoverFront; i <= Frame::PT_PublisherLogo; ++i) {
        if (auto pt = static_cast<Frame::PictureType>(i);
            itemListMap.find(getApePictureName(pt)) == itemListMap.end()) {
          pictureType = pt;
          break;
        }
      }
      PictureFrame::setFields(
            frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
            QLatin1String("image/jpeg"), pictureType);
    }
    QString name(getApeName(frame));
    TagLib::String tname = toTString(name);
    if (frame.getType() == Frame::FT_Picture) {
      TagLib::ByteVector data;
      renderApePicture(frame, data);
      apeTag->setData(tname, data);
    } else {
      TagLib::String tvalue = toTString(frame.getValue());
      if (tvalue.isEmpty()) {
        tvalue = " "; // empty values are not added by TagLib
      }
      apeTag->addValue(tname, tvalue, true);
    }
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));

    const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
    int index = 0;
    bool found = false;
    for (const auto& [itemName, item] : itemListMap) {
      if (itemName == tname) {
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

bool TagLibApeSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto apeTag = dynamic_cast<TagLib::APE::Tag*>(f.m_tag[tagNr])) {
    TagLib::String key = toTString(frame.getInternalName());
    apeTag->removeItem(key);
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibApeSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (auto apeTag = dynamic_cast<TagLib::APE::Tag*>(f.m_tag[tagNr])) {
    const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
    if (flt.areAllEnabled()) {
      for (auto it = itemListMap.begin();
           it != itemListMap.end();) {
        apeTag->removeItem((it++)->first);
      }
    } else {
      for (auto it = itemListMap.begin();
           it != itemListMap.end();) {
        if (QString name(toQString(it->first));
            flt.isEnabled(getTypeFromApeName(name), name)) {
          apeTag->removeItem((it++)->first);
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

bool TagLibApeSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (auto apeTag = dynamic_cast<TagLib::APE::Tag*>(f.m_tag[tagNr])) {
    const TagLib::APE::ItemListMap& itemListMap = apeTag->itemListMap();
    int i = 0;
    for (const auto& [itemName, item] : itemListMap) {
      QString name = toQString(itemName);
      Frame::Type type = getTypeFromApeName(name);
      TagLib::StringList values;
      if (type != Frame::FT_Picture) {
        values = item.values();
      }
      Frame frame(type, values.size() > 0
                  ? joinToQString(values) : QLatin1String(""),
                  name, i++);
      if (type == Frame::FT_Picture) {
        TagLib::ByteVector data = item.binaryData();
        parseApePicture(name, data, frame);
      }
      frames.insert(frame);
    }
    return true;
  }
  return false;
}
