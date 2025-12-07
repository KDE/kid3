/**
 * \file taglibvorbissupport.cpp
 * Support for Ogg, Opus, FLAC and Speex files and Vorbis tags.
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

#include "taglibvorbissupport.h"

#include <flacfile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <oggflacfile.h>
#include <opusfile.h>
#include <speexfile.h>
#include <vorbisfile.h>

#include "pictureframe.h"
#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

/**
 * Set a picture frame from a FLAC picture.
 *
 * @param pic FLAC picture
 * @param frame the picture frame is returned here
 */
void flacPictureToFrame(const TagLib::FLAC::Picture* pic, Frame& frame)
{
  TagLib::ByteVector picData(pic->data());
  QByteArray ba(picData.data(), picData.size());
  PictureFrame::ImageProperties imgProps(
        pic->width(), pic->height(), pic->colorDepth(),
        pic->numColors(), ba);
  PictureFrame::setFields(
    frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), toQString(pic->mimeType()),
    static_cast<PictureFrame::PictureType>(pic->type()),
    toQString(pic->description()),
    ba, &imgProps);
}

/**
 * Set a FLAC picture from a frame.
 *
 * @param frame picture frame
 * @param pic the FLAC picture to set
 */
void frameToFlacPicture(const Frame& frame, TagLib::FLAC::Picture* pic)
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
  pic->setType(static_cast<TagLib::FLAC::Picture::Type>(pictureType));
  pic->setMimeType(toTString(mimeType));
  pic->setDescription(toTString(description));
  pic->setData(TagLib::ByteVector(data.data(), data.size()));
  if (!imgProps.isValidForImage(data)) {
    imgProps = PictureFrame::ImageProperties(data);
  }
  pic->setWidth(imgProps.width());
  pic->setHeight(imgProps.height());
  pic->setColorDepth(imgProps.depth());
  pic->setNumColors(imgProps.numColors());
}

}


TagLib::File* TagLibVorbisSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "OGG") {
    TagLib::File* file = new TagLib::Vorbis::File(stream);
    if (!file->isValid()) {
      delete file;
      file = new TagLib::Ogg::FLAC::File(stream);
    }
    return file;
  }
  if (ext == "OGA") {
    TagLib::File* file = new TagLib::Ogg::FLAC::File(stream);
    if (!file->isValid()) {
      delete file;
      file = new TagLib::Vorbis::File(stream);
    }
    return file;
  }
  if (ext == "FLAC")
#if TAGLIB_VERSION >= 0x020000
    return new TagLib::FLAC::File(stream);
#else
    return new TagLib::FLAC::File(stream,
      TagLib::ID3v2::FrameFactory::instance());
#endif
  if (ext == "SPX")
    return new TagLib::Ogg::Speex::File(stream);
  if (ext == "OPUS")
    return new TagLib::Ogg::Opus::File(stream);
  return nullptr;
}

bool TagLibVorbisSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) {
    f.m_fileExtension = QLatin1String(".flac");
    f.m_isTagSupported[Frame::Tag_1] = true;
    f.m_isTagSupported[Frame::Tag_3] = true;
    if (!f.m_tag[Frame::Tag_1]) {
      f.m_tag[Frame::Tag_1] = flacFile->ID3v1Tag();
      f.markTagUnchanged(Frame::Tag_1);
    }
    if (!f.m_tag[Frame::Tag_2]) {
      f.m_tag[Frame::Tag_2] = flacFile->xiphComment();
      f.markTagUnchanged(Frame::Tag_2);
    }
    if (!f.m_tag[Frame::Tag_3]) {
      f.m_tag[Frame::Tag_3] = flacFile->ID3v2Tag();
      f.markTagUnchanged(Frame::Tag_3);
    }
    if (!f.m_extraFrames.isRead()) {
      const TagLib::List pics(flacFile->pictureList());
      int i = 0;
      for (auto it = pics.begin(); it != pics.end(); ++it) {
        PictureFrame frame;
        flacPictureToFrame(*it, frame);
        frame.setIndex(Frame::toNegativeIndex(i++));
        f.m_extraFrames.append(frame);
      }
      f.m_extraFrames.setRead(true);
    }
    return true;
  }
  if (dynamic_cast<TagLib::Vorbis::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".ogg");
    putFileRefTagInTag2(f);
    putPicturesInExtraFrames(f);
    return true;
  }
  if (dynamic_cast<TagLib::Ogg::Speex::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".spx");
    putFileRefTagInTag2(f);
    putPicturesInExtraFrames(f);
    return true;
  }
  if (dynamic_cast<TagLib::Ogg::Opus::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".opus");
    putFileRefTagInTag2(f);
    putPicturesInExtraFrames(f);
    return true;
  }
  return false;
}

void TagLibVorbisSupport::putPicturesInExtraFrames(TagLibFile& f)
{
  if (!f.m_extraFrames.isRead()) {
#if TAGLIB_VERSION >= 0x010b00
    if (auto xiphComment =
      dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[Frame::Tag_2])) {
      const TagLib::List pics(xiphComment->pictureList());
      int i = 0;
      for (auto it = pics.begin(); it != pics.end(); ++it) {
        PictureFrame frame;
        flacPictureToFrame(*it, frame);
        frame.setIndex(Frame::toNegativeIndex(i++));
        f.m_extraFrames.append(frame);
      }
      f.m_extraFrames.setRead(true);
    }
#endif
  }
}

bool TagLibVorbisSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
  int, bool& fileChanged) const
{
  if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
#if TAGLIB_VERSION >= 0x010b00
      static constexpr int tagTypes[TagLibFile::NUM_TAGS] = {
        TagLib::FLAC::File::ID3v1, TagLib::FLAC::File::XiphComment,
        TagLib::FLAC::File::ID3v2
      };
      FOR_TAGLIB_TAGS(tagNr) {
        if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr)) && f.m_tag[tagNr]->isEmpty()) {
          flacFile->strip(tagTypes[tagNr]);
          fileChanged = true;
          f.m_tag[tagNr] = nullptr;
          f.markTagUnchanged(tagNr);
        }
      }
#endif
      flacFile->removePictures();
      const auto frames = f.m_extraFrames;
      for (const Frame& frame : frames) {
        // ReSharper disable once CppDFAMemoryLeak
        auto pic = new TagLib::FLAC::Picture;
        frameToFlacPicture(frame, pic);
        flacFile->addPicture(pic);
      }
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
#if TAGLIB_VERSION >= 0x010b00
  if (auto xiphComment =
      dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[Frame::Tag_2])) {
    if (anyTagMustBeSaved(f, force)) {
      xiphComment->removeAllPictures();
      const auto frames = f.m_extraFrames;
      for (const Frame& frame : frames) {
        // ReSharper disable once CppDFAMemoryLeak
        auto pic = new TagLib::FLAC::Picture;
        frameToFlacPicture(frame, pic);
        xiphComment->addPicture(pic);
      }
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
#endif
  return false;
}

bool TagLibVorbisSupport::makeTagSettable(TagLibFile& f, TagLib::File* file,
  Frame::TagNumber tagNr) const
{
  if (tagNr == Frame::Tag_1) {
    if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) {
      f.m_tag[tagNr] = flacFile->ID3v1Tag(true);
    }
  } else if (tagNr == Frame::Tag_2) {
    if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) {
      f.m_tag[tagNr] = flacFile->xiphComment(true);
    }
  } else if (tagNr == Frame::Tag_3) {
    if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file)) {
      f.m_tag[tagNr] = flacFile->ID3v2Tag(true);
    }
  }
  return false;
}

bool TagLibVorbisSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (dynamic_cast<TagLib::Vorbis::Properties*>(audioProperties) != nullptr) {
    f.m_detailInfo.format = QLatin1String("Ogg Vorbis");
    return true;
  }
  if (auto flacProperties =
      dynamic_cast<TagLib::FLAC::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QLatin1String("FLAC");
#if TAGLIB_VERSION >= 0x010a00
    if (int bits = flacProperties->bitsPerSample(); bits > 0) {
      f.m_detailInfo.format += QLatin1Char(' ');
      f.m_detailInfo.format += QString::number(bits);
      f.m_detailInfo.format += QLatin1String(" bit");
    }
#endif
    return true;
  }
  if (auto opusProperties =
      dynamic_cast<TagLib::Ogg::Opus::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("Opus %1"))
        .arg(opusProperties->opusVersion());
    return true;
  }
  if (auto speexProperties =
      dynamic_cast<TagLib::Ogg::Speex::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("Speex %1")).arg(speexProperties->speexVersion());
    return true;
  }
  return false;
}

QString TagLibVorbisSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType& type) const
{
  if (dynamic_cast<const TagLib::Ogg::XiphComment*>(tag) != nullptr) {
    type = TaggedFile::TT_Vorbis;
    return QLatin1String("Vorbis");
  }
  return QString();
}

bool TagLibVorbisSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      QString frameValue(frame.getValue());
      if (Frame::ExtendedType extendedType = frame.getExtendedType();
          extendedType.getType() == Frame::FT_Picture) {
        if (f.m_extraFrames.isRead()) {
          if (int idx = Frame::fromNegativeIndex(frame.getIndex());
              idx >= 0 && idx < f.m_extraFrames.size()) {
            Frame newFrame(frame);
            PictureFrame::setDescription(newFrame, frameValue);
            if (PictureFrame::areFieldsEqual(f.m_extraFrames[idx], newFrame)) {
              f.m_extraFrames[idx].setValueChanged(false);
            } else {
              f.m_extraFrames[idx] = newFrame;
              f.markTagChanged(tagNr, extendedType);
            }
            return true;
          }
          return false;
        }
        Frame newFrame(frame);
        PictureFrame::setDescription(newFrame, frameValue);
        PictureFrame::getFieldsToBase64(newFrame, frameValue);
        if (!frameValue.isEmpty() &&
          frame.getInternalName() == QLatin1String("COVERART")) {
          QString mimeType;
          PictureFrame::getMimeType(frame, mimeType);
          oggTag->addField("COVERARTMIME", toTString(mimeType), true);
        }
      }
      TagLib::String key = toTString(getVorbisName(f, frame));
      TagLib::String value = toTString(frameValue);
      if (const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
          fieldListMap.contains(key) && fieldListMap[key].size() > 1) {
        int i = 0;
        bool found = false;
        for (auto it = fieldListMap.begin();
             it != fieldListMap.end();
             ++it) {
          TagLib::StringList stringList = it->second;
          for (auto slit = stringList.begin(); slit != stringList.end(); ++slit) {
            if (i++ == index) {
              *slit = value;
              found = true;
              break;
            }
          }
          if (found) {
            // Replace all fields with this key to preserve the order.
#if TAGLIB_VERSION >= 0x010b01
            oggTag->removeFields(key);
#else
            oggTag->removeField(key);
#endif
            for (auto slit = stringList.begin(); slit != stringList.end(); ++slit) {
              oggTag->addField(key, *slit, false);
            }
            break;
          }
        }
      } else {
        oggTag->addField(key, value, true);
      }
      if (frame.getType() == Frame::FT_Track) {
        if (int numTracks = f.getTotalNumberOfTracksIfEnabled();
            numTracks > 0) {
          oggTag->addField("TRACKTOTAL", TagLib::String::number(numTracks), true);
        }
      }
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibVorbisSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
  if (auto oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[tagNr])) {
    QString name(f.getVorbisName(frame));
    QString value(frame.getValue());
    if (frame.getType() == Frame::FT_Picture) {
      if (frame.getFieldList().empty()) {
        PictureFrame::setFields(
          frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), QLatin1String("image/jpeg"),
          PictureFrame::PT_CoverFront, QLatin1String(""), QByteArray());
      }
      if (f.m_extraFrames.isRead()) {
        PictureFrame::setDescription(frame, value);
        frame.setIndex(Frame::toNegativeIndex(f.m_extraFrames.size()));
        f.m_extraFrames.append(frame);
        f.markTagChanged(tagNr, frame.getExtendedType());
        return true;
      }
      PictureFrame::getFieldsToBase64(frame, value);
    }
    TagLib::String tname = toTString(name);
    TagLib::String tvalue = toTString(value);
    if (tvalue.isEmpty()) {
      tvalue = " "; // empty values are not added by TagLib
    }
    oggTag->addField(tname, tvalue, false);
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));

    const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
    int index = 0;
    bool found = false;
    for (auto it = fieldListMap.begin();
         it != fieldListMap.end();
         ++it) {
      if (it->first == tname) {
        index += it->second.size() - 1;
        found = true;
        break;
      }
      index += it->second.size();
    }
    frame.setIndex(found ? index : -1);
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibVorbisSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[tagNr])) {
    QString frameValue(frame.getValue());
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
      } else {
        PictureFrame::getFieldsToBase64(frame, frameValue);
      }
    }
    TagLib::String key =
      toTString(frame.getInternalName());
#if TAGLIB_VERSION >= 0x010b01
    oggTag->removeFields(key, toTString(frameValue));
#else
    oggTag->removeField(key, toTString(frameValue));
#endif
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibVorbisSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (auto oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[tagNr])) {
    const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
    if (flt.areAllEnabled()) {
      for (auto it = fieldListMap.begin();
           it != fieldListMap.end();) {
#if TAGLIB_VERSION >= 0x010b01
        oggTag->removeFields((it++)->first);
#else
        oggTag->removeField((it++)->first);
#endif
      }
    } else {
      for (auto it = fieldListMap.begin();
           it != fieldListMap.end();) {
        if (QString name(toQString(it->first));
            flt.isEnabled(getTypeFromVorbisName(name), name)) {
#if TAGLIB_VERSION >= 0x010b01
          oggTag->removeFields((it++)->first);
#else
          oggTag->removeField((it++)->first);
#endif
        } else {
          ++it;
        }
      }
      if (flt.isEnabled(Frame::FT_Picture)) {
        f.m_extraFrames.clear();
      }
    }
    f.markTagChanged(tagNr, Frame::ExtendedType());
    return true;
  }
  return false;
}

bool TagLibVorbisSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (auto oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(f.m_tag[tagNr])) {
    const TagLib::Ogg::FieldListMap& fieldListMap = oggTag->fieldListMap();
    int i = 0;
    for (auto it = fieldListMap.begin();
         it != fieldListMap.end();
         ++it) {
      QString name = toQString(it->first);
      Frame::Type type = getTypeFromVorbisName(name);
      const TagLib::StringList stringList = it->second;
      for (auto slit = stringList.begin(); slit != stringList.end(); ++slit) {
        if (type == Frame::FT_Picture) {
          Frame frame(type, QLatin1String(""), name, i++);
          PictureFrame::setFieldsFromBase64(
                frame, toQString(TagLib::String(*slit)));
          if (name == QLatin1String("COVERART")) {
            if (TagLib::StringList mt = oggTag->fieldListMap()["COVERARTMIME"];
                !mt.isEmpty()) {
              PictureFrame::setMimeType(frame, toQString(mt.front()));
            }
          }
          frames.insert(frame);
        } else {
          frames.insert(Frame(type, toQString(TagLib::String(*slit)),
                              name, i++));
        }
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

void TagLibVorbisSupport::setTagValue(TagLibFile& f, Frame::TagNumber tagNr, Frame::Type type, const TagLib::String& str) const
{
  TagLib::Tag* tag = f.m_tag[tagNr];
  if (type == Frame::FT_Date) {
    if (auto oggTag = dynamic_cast<TagLib::Ogg::XiphComment*>(tag)) {
      oggTag->addField(getVorbisNameFromType(type), str, true);
      return;
    }
  }
  TagLibFormatSupport::setTagValue(f, tagNr, type, str);
}

/**
 * Get internal name of a Vorbis frame.
 *
 * @param frame frame
 *
 * @return Vorbis key.
 */
QString TagLibVorbisSupport::getVorbisName(const TagLibFile& f, const Frame& frame)
{
  if (Frame::Type type = frame.getType(); type == Frame::FT_Comment) {
    return f.getCommentFieldName();
  } else if (type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getVorbisNameFromType(type));
  }
  return f.fixUpTagKey(frame.getName(), TaggedFile::TT_Vorbis).toUpper();
}
