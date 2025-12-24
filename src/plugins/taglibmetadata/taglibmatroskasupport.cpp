/**
 * \file taglibmatroskasupport.cpp
 * Support for Matroska files and tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 24 Dec 2025
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

#include "taglibmatroskasupport.h"

#include <QJsonDocument>
#include <matroskafile.h>
#include <matroskaattachments.h>
#include <matroskaattachedfile.h>
#include <matroskachapter.h>
#include <matroskachapters.h>
#include <matroskachapteredition.h>
#include <matroskasimpletag.h>
#include <matroskatag.h>

#include "pictureframe.h"
#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

constexpr struct {
  const char* name;
  TagLib::Matroska::SimpleTag::TargetTypeValue targetType;
  bool strict;
} matroskaNamesForTypes[] = {
  {"TITLE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},               // FT_Title,
  {"ARTIST", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},              // FT_Artist,
  {"TITLE", TagLib::Matroska::SimpleTag::TargetTypeValue::Album, true},                // FT_Album,
  {"COMMENT", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},             // FT_Comment,
  {"DATE_RECORDED", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},       // FT_Date,
  {"PART_NUMBER", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},         // FT_Track,
  {"GENRE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},               // FT_Genre,
                                                                                // FT_LastV1Frame = FT_Track,
  {"ARTIST", TagLib::Matroska::SimpleTag::TargetTypeValue::Album, true},               // FT_AlbumArtist,
  {"ARRANGER", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},            // FT_Arranger,
  {"WRITTEN_BY", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},          // FT_Author,
  {"BPM", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},                 // FT_Bpm,
  {"CATALOG_NUMBER", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},      // FT_CatalogNumber,
  {"COMPILATION", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},         // FT_Compilation,
  {"COMPOSER", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},            // FT_Composer,
  {"CONDUCTOR", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},           // FT_Conductor,
  {"COPYRIGHT", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},           // FT_Copyright,
  {"PART_NUMBER", TagLib::Matroska::SimpleTag::TargetTypeValue::Album, true},          // FT_Disc,
  {"ENCODER", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},             // FT_EncodedBy,
  {"ENCODER_SETTINGS", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},    // FT_EncoderSettings,
  {"DATE_ENCODED", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},        // FT_EncodingTime,
  {"GROUPING", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},            // FT_Grouping,
  {"INITIAL_KEY", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},         // FT_InitialKey,
  {"ISRC", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},                // FT_Isrc,
  {"LANGUAGE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},            // FT_Language,
  {"LYRICIST", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},            // FT_Lyricist,
  {"LYRICS", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},              // FT_Lyrics,
  {"ORIGINAL_MEDIA_TYPE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false}, // FT_Media,
  {"MOOD", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},                // FT_Mood,
  {"ORIGINALALBUM", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},       // FT_OriginalAlbum,
  {"ORIGINALARTIST", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},      // FT_OriginalArtist,
  {"ORIGINALDATE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},        // FT_OriginalDate,
  {"DESCRIPTION", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},         // FT_Description,
  {"PERFORMER", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},      // FT_Performer,
  {"PICTURE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},             // FT_Picture,
  {"LABEL_CODE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},          // FT_Publisher,
  {"RELEASECOUNTRY", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},      // FT_ReleaseCountry,
  {"REMIXED_BY", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},          // FT_Remixer,
  {"TITLESORT", TagLib::Matroska::SimpleTag::TargetTypeValue::Album, true},            // FT_SortAlbum,
  {"ARTISTSORT", TagLib::Matroska::SimpleTag::TargetTypeValue::Album, true},           // FT_SortAlbumArtist,
  {"ARTISTSORT", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},          // FT_SortArtist,
  {"COMPOSERSORT", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},        // FT_SortComposer,
  {"TITLESORT", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},           // FT_SortName,
  {"SUBTITLE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},            // FT_Subtitle,
  {"WEBSITE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},             // FT_Website,
  {"WWWAUDIOFILE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},        // FT_WWWAudioFile,
  {"WWWAUDIOSOURCE", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},      // FT_WWWAudioSource,
  {"DATE_RELEASED", TagLib::Matroska::SimpleTag::TargetTypeValue::Album, false},       // FT_ReleaseDate,
  {"RATING", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},              // FT_Rating,
  {"WORK", TagLib::Matroska::SimpleTag::TargetTypeValue::Track, false},                // FT_Work,
                                                                                // FT_Custom1
};

/**
 * Get name of frame from type.
 *
 * @param type type
 * @param targetType the target type is returned here
 *
 * @return name.
 */
const char* getMatroskaNameFromType(Frame::Type type,
  TagLib::Matroska::SimpleTag::TargetTypeValue& targetType)
{
  Q_STATIC_ASSERT(std::size(matroskaNamesForTypes) == Frame::FT_Custom1);
  if (Frame::isCustomFrameType(type)) {
    targetType = TagLib::Matroska::SimpleTag::TargetTypeValue::Track;
    return Frame::getNameForCustomFrame(type);
  }
  if (type < Frame::FT_Custom1) {
    auto [name, targetTypeValue, strict] = matroskaNamesForTypes[type];
    targetType = targetTypeValue;
    return name;
  }
  targetType = TagLib::Matroska::SimpleTag::TargetTypeValue::None;
  return "UNKNOWN";
}

/**
 * Get the frame type for a Matroska name.
 *
 * @param name Matroska simple tag name
 * @param targetType Matroska target type value
 *
 * @return frame type.
 */
Frame::Type getTypeFromMatroskaName(const QString& name, TagLib::Matroska::SimpleTag::TargetTypeValue targetType)
{
  int i = 0;
  for (const auto& nameTarget : matroskaNamesForTypes) {
    if (name == QString::fromUtf8(nameTarget.name) &&
      (targetType == nameTarget.targetType ||
        (targetType == TagLib::Matroska::SimpleTag::TargetTypeValue::None &&
          !nameTarget.strict))) {
      return static_cast<Frame::Type>(i);
    }
    ++i;
  }
  return Frame::getTypeFromCustomFrameName(name.toLatin1());
}

QString getMatroskaName(const Frame& frame,
  TagLib::Matroska::SimpleTag::TargetTypeValue& targetType)
{
  if (Frame::Type type = frame.getType(); type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getMatroskaNameFromType(type, targetType));
  }
  targetType = TagLib::Matroska::SimpleTag::TargetTypeValue::Track;
  return TaggedFile::fixUpTagKey(frame.getName(), TaggedFile::TT_Vorbis).toUpper();
}

QString toSimpleTextOrJson(const QVariantMap& metadata)
{
  if (metadata.isEmpty()) {
    return {};
  }
  if (metadata.size() == 1) {
    if (const QVariant& firstValue = metadata.first();
#if QT_VERSION >= 0x060000
        firstValue.typeId() == QMetaType::QString
#else
        firstValue.type() == QVariant::String
#endif
       ) {
      return firstValue.toString();
    }
  }
  return QString::fromUtf8(QJsonDocument::fromVariant(metadata)
    .toJson(QJsonDocument::Compact));
}

QVariantMap fromSimpleTextOrJson(const QString& str)
{
  if (str.startsWith(QLatin1Char('{')) && str.endsWith(QLatin1Char('}'))) {
    return QJsonDocument::fromJson(str.toUtf8()).toVariant().toMap();
  }
  return {{QLatin1String("text"), str}};
}

void matroskaPictureToFrame(
  const TagLib::Matroska::AttachedFile& attachedFile, Frame& frame)
{
  const TagLib::ByteVector& bv = attachedFile.data();
  const QByteArray data(bv.data(), static_cast<int>(bv.size()));
  const QString& mediaType = toQString(attachedFile.mediaType());
  const QString& description = toQString(attachedFile.description());
  const QString& fileName = toQString(attachedFile.fileName());
  const QString& uid = QString::number(attachedFile.uid());
  PictureFrame::setFields(
    frame, Frame::TE_ISO8859_1, QLatin1String("JPG"), mediaType,
    PictureFrame::PT_CoverFront, description, data);
  frame.fieldList().append({Frame::ID_Filename, fileName});
  frame.fieldList().append({Frame::ID_Id, uid});
}

TagLib::Matroska::AttachedFile frameToMatroskaPicture(const Frame& frame)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType;
  QByteArray data;
  QString imgFormat, mimeType, description;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType, pictureType,
                          description, data);
  const QString fileName = Frame::getField(frame, Frame::ID_Filename).toString();
  const qulonglong uid = Frame::getField(frame, Frame::ID_Id).toULongLong();
  return TagLib::Matroska::AttachedFile(
    TagLib::ByteVector(data.constData(), static_cast<unsigned int>(data.size())),
    toTString(fileName), toTString(mimeType), uid, toTString(description));
}

void matroskaAttachedFileToFrame(
  const TagLib::Matroska::AttachedFile& attachedFile, Frame& frame)
{
  const TagLib::ByteVector& bv = attachedFile.data();
  const QByteArray data(bv.data(), static_cast<int>(bv.size()));
  const QString& mediaType = toQString(attachedFile.mediaType());
  const QString fileName = toQString(attachedFile.fileName());
  const QString& description = toQString(attachedFile.description());
  const QString& uid = QString::number(attachedFile.uid());
  frame.setExtendedType(
    Frame::ExtendedType(Frame::FT_Other, QLatin1String("General Object")));
  frame.setValue(description);
  // The fields for non-picture attachments are the same as for the
  // ID3 GEOB frame plus the UID as an ID.
  frame.fieldList() = {
    {Frame::ID_TextEnc, Frame::TE_ISO8859_1},
    {Frame::ID_MimeType, mediaType},
    {Frame::ID_Filename, fileName},
    {Frame::ID_Description, description},
    {Frame::ID_Data, data},
    {Frame::ID_Id, uid}
  };
}

TagLib::Matroska::AttachedFile frameToMatroskaAttachedFile(const Frame& frame)
{
  QByteArray data;
  QString mimeType, description;
  PictureFrame::getData(frame, data);
  PictureFrame::getMimeType(frame, mimeType);
  PictureFrame::getDescription(frame, description);
  const QString fileName = Frame::getField(frame, Frame::ID_Filename).toString();
  const qulonglong uid = Frame::getField(frame, Frame::ID_Id).toULongLong();
  return TagLib::Matroska::AttachedFile(
    TagLib::ByteVector(data.constData(), static_cast<unsigned int>(data.size())),
    toTString(fileName), toTString(mimeType), uid, toTString(description));
}

void matroskaChapterEditionToFrame(
  const TagLib::Matroska::ChapterEdition& chapterEdition, Frame& frame)
{
  const QString& uid = QString::number(chapterEdition.uid());
  QVariantMap editionMap;
  if (!chapterEdition.isDefault()) {
    editionMap.insert(QLatin1String("default"), chapterEdition.isDefault());
  }
  if (chapterEdition.isOrdered()) {
    editionMap.insert(QLatin1String("ordered"), chapterEdition.isOrdered());
  }
  const QString description = toSimpleTextOrJson(editionMap);
  frame.setExtendedType(
    Frame::ExtendedType(Frame::FT_Other, QLatin1String("Chapters")));
  frame.setValue(description);

  TagLib::String language;
  QVariantList synchedData;

  unsigned long long lastTimeEnd = 0ULL;

  unsigned long long chapterNr = 1ULL;
  for (const auto& chapter : chapterEdition.chapterList()) {
    if (lastTimeEnd && lastTimeEnd != chapter.timeStart()) {
      synchedData.append(static_cast<double>(lastTimeEnd) / 1E6);
      synchedData.append(QString());
    }
    synchedData.append(static_cast<double>(chapter.timeStart()) / 1E6);
    QVariantMap chapMap;
    for (const auto& display : chapter.displayList()) {
      if (language.isEmpty()) {
        language = display.language();
      }
      chapMap.insert(toQString(display.language()), toQString(display.string()));
    }
    if (chapter.uid() != chapterNr) {
      chapMap.insert(QLatin1String("uid"), chapter.uid());
    }
    if (chapter.isHidden()) {
      chapMap.insert(QLatin1String("hidden"), chapter.isHidden());
    }
    synchedData.append(toSimpleTextOrJson(chapMap));
    lastTimeEnd = chapter.timeEnd();
    ++chapterNr;
  }
  // synchedData.append(static_cast<quint32>(lastTimeEnd / 1000000ULL));
  synchedData.append(static_cast<double>(lastTimeEnd) / 1E6);
  synchedData.append(QString());

  // The fields for chapters are the same as for the ID3 SYLT frame.
  frame.fieldList() = {
    {Frame::ID_TextEnc, Frame::TE_UTF8},
    {Frame::ID_Language, toQString(language)},
    {Frame::ID_TimestampFormat, 2}, // milliseconds as unit
    {Frame::ID_ContentType, 0}, // other
    {Frame::ID_Description, description},
    {Frame::ID_Id, uid},
    {Frame::ID_Data, synchedData}
  };
}

TagLib::Matroska::ChapterEdition frameToMatroskaChapterEdition(const Frame& frame)
{
  const TagLib::String language = toTString(Frame::getField(frame, Frame::ID_Language).toString());
  const QVariantList synchedData = Frame::getField(frame, Frame::ID_Data).toList();

  struct ChapterData {
    TagLib::List<TagLib::Matroska::Chapter::Display> displays;
    unsigned long long timeStart;
    unsigned long long timeEnd;
    unsigned long long uid;
    bool hidden;
  };
  QList<ChapterData> chapterData;
  unsigned long long chapterNr = 1ULL;
  int chapterDataIndex = -1;

  QListIterator it(synchedData);
  while (it.hasNext()) {
    auto time = static_cast<unsigned long long>(it.next().toDouble() * 1E6);
    auto text = it.next().toString();
    if (chapterDataIndex >= 0 && !chapterData[chapterDataIndex].timeEnd) {
      chapterData[chapterDataIndex].timeEnd = time;
      if (text.isEmpty()) {
        continue;
      }
    }
    auto map = fromSimpleTextOrJson(text);
    ChapterData cd;
    cd.timeStart = time;
    cd.timeEnd = 0;
    cd.uid = map.take(QLatin1String("uid")).toULongLong();
    if (!cd.uid) {
      cd.uid = chapterNr;
    }
    cd.hidden = map.take(QLatin1String("hidden")).toBool();
    if (!map.isEmpty()) {
      for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        cd.displays.append(TagLib::Matroska::Chapter::Display(
          toTString(it.value().toString()),
          it.key() != QLatin1String("text") ? toTString(it.key()) : language));
      }
    } else {
      cd.displays.append(TagLib::Matroska::Chapter::Display("", language));
    }
    chapterData.append(cd);
    ++chapterNr;
    ++chapterDataIndex;
  }

  TagLib::List<TagLib::Matroska::Chapter> chapters;
  for (const auto& cd : chapterData) {
    chapters.append(TagLib::Matroska::Chapter(
      cd.timeStart, cd.timeEnd, cd.displays, cd.uid, cd.hidden));
  }
  const qulonglong uid = Frame::getField(frame, Frame::ID_Id).toULongLong();
  const QString description = Frame::getField(frame, Frame::ID_Description).toString();
  auto map = fromSimpleTextOrJson(description);
  return TagLib::Matroska::ChapterEdition(
    chapters,
    map.value(QLatin1String("default"), true).toBool(),
    map.value(QLatin1String("ordered"), false).toBool(),
    uid);
}

TagLib::Matroska::SimpleTag frameToMatroskaSimpleTag(const Frame& frame)
{
  const QVariant dataVar = Frame::getField(frame, Frame::ID_Data);
  const bool isBinary = dataVar.isValid();
  const QByteArray data = isBinary ? dataVar.toByteArray() : QByteArray();
  const TagLib::String name = toTString(frame.getInternalName());
  const TagLib::String value = toTString(frame.getValue());
  auto targetType =
    static_cast<TagLib::Matroska::SimpleTag::TargetTypeValue>(
      Frame::getField(frame, Frame::ID_TargetType).toInt() * 10);
  const TagLib::String language = toTString(
    Frame::getField(frame, Frame::ID_Language).toString());
  const bool defaultLanguage =
    Frame::getField(frame, Frame::ID_Default).toBool();
  const unsigned long long trackUid =
    Frame::getField(frame, Frame::ID_Id).toULongLong();
  return !isBinary
    ? TagLib::Matroska::SimpleTag(
        name, value, targetType, language, defaultLanguage, trackUid)
    : TagLib::Matroska::SimpleTag(
        name, TagLib::ByteVector(data.constData(), data.size()),
        targetType, language, defaultLanguage, trackUid);
}

bool isExtraFrame(Frame::Type type, const QString& name)
{
  return type == Frame::FT_Picture ||
        (type == Frame::FT_Other && (
          name == QLatin1String("General Object") ||
          name == QLatin1String("Chapters")));
}

bool isExtraFrame(const Frame::ExtendedType& type)
{
  return isExtraFrame(type.getType(), type.getInternalName());
}

}


TagLib::File* TagLibMatroskaSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "MKA" || ext == "MKV" || ext == "WEBM")
    return new TagLib::Matroska::File(stream);
  return nullptr;
}

bool TagLibMatroskaSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (auto mkaFile = dynamic_cast<TagLib::Matroska::File*>(file)) {
    f.m_fileExtension = QLatin1String(".mka");
    putFileRefTagInTag2(f);

    if (!f.m_extraFrames.isRead()) {
      int i = 0;
      if (auto attachments = mkaFile->attachments()) {
        for (const auto& attachedFile : attachments->attachedFileList()) {
          if (attachedFile.mediaType().startsWith("image/")) {
            PictureFrame frame;
            matroskaPictureToFrame(attachedFile, frame);
            frame.setIndex(Frame::toNegativeIndex(i++));
            f.m_extraFrames.append(frame);
          } else {
            Frame frame;
            matroskaAttachedFileToFrame(attachedFile, frame);
            frame.setIndex(Frame::toNegativeIndex(i++));
            f.m_extraFrames.append(frame);
          }
        }
      }
      if (auto chapters = mkaFile->chapters()) {
        for (const auto& chapterEdition : chapters->chapterEditionList()) {
          Frame frame;
          matroskaChapterEditionToFrame(chapterEdition, frame);
          frame.setIndex(Frame::toNegativeIndex(i++));
          f.m_extraFrames.append(frame);
        }
      }
      f.m_extraFrames.setRead(true);
    }
    return true;
  }
  return false;
}


bool TagLibMatroskaSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
  int, bool& fileChanged) const
{
  if (auto mkaFile = dynamic_cast<TagLib::Matroska::File*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      if (auto attachments = mkaFile->attachments(false)) {
        attachments->clear();
      }
      if (auto chapters = mkaFile->chapters(false)) {
        chapters->clear();
      }
      const auto frames = f.m_extraFrames;
      for (const Frame& frame : frames) {
        if (frame.getExtendedType() == Frame::ExtendedType(
              Frame::FT_Other, QLatin1String("Chapters"))) {
          mkaFile->chapters(true)->addChapterEdition(
            frameToMatroskaChapterEdition(frame));
        } else if (frame.getType() == Frame::FT_Picture) {
          mkaFile->attachments(true)->addAttachedFile(
            frameToMatroskaPicture(frame));
        } else {
          mkaFile->attachments(true)->addAttachedFile(
            frameToMatroskaAttachedFile(frame));
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

bool TagLibMatroskaSupport::makeTagSettable(TagLibFile& f, TagLib::File* file,
  Frame::TagNumber tagNr) const
{
  if (TagLib::Matroska::File* mkaFile;
      tagNr == Frame::Tag_2 &&
      (mkaFile = dynamic_cast<TagLib::Matroska::File*>(file)) != nullptr) {
    f.m_tag[tagNr] = mkaFile->tag(true);
    return true;
  }
  return false;
}

bool TagLibMatroskaSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto mkaProperties = dynamic_cast<TagLib::Matroska::Properties*>(audioProperties)) {
    f.m_detailInfo.format = toQString(
      mkaProperties->docType().substr(0, 1).upper() +
      mkaProperties->docType().substr(1));
    f.m_detailInfo.format += QLatin1String(" Version ") +
      QString::number(mkaProperties->docTypeVersion());
    if (!mkaProperties->codecName().isEmpty()) {
      f.m_detailInfo.format += QLatin1String(" Codec ") +
        toQString(mkaProperties->codecName());
    }
    return true;
  }
  return false;
}

QString TagLibMatroskaSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType&) const
{
  if (dynamic_cast<const TagLib::Matroska::Tag*>(tag) != nullptr) {
    return QLatin1String("Matroska");
  }
  return {};
}

bool TagLibMatroskaSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto mkaTag = dynamic_cast<TagLib::Matroska::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      if (Frame::ExtendedType extendedType = frame.getExtendedType();
          isExtraFrame(extendedType)) {
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
      if (index < static_cast<int>(mkaTag->simpleTagsList().size())) {
        mkaTag->removeSimpleTag(index);
        mkaTag->insertSimpleTag(index, frameToMatroskaSimpleTag(frame));
        f.markTagChanged(tagNr, frame.getExtendedType());
      }
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibMatroskaSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
  if (auto mkaTag = dynamic_cast<TagLib::Matroska::Tag*>(f.m_tag[tagNr])) {
    if (Frame::ExtendedType extendedType = frame.getExtendedType();
        isExtraFrame(extendedType)) {
      if (frame.getFieldList().isEmpty()) {
        if (extendedType.getType() == Frame::FT_Picture) {
          PictureFrame::setFields(frame);
          frame.fieldList().append({
            {Frame::ID_Filename, QString()},
            {Frame::ID_Id, QString()}
          });
        } else if (extendedType.getName() == QLatin1String("General Object")) {
          frame.fieldList() = {
            {Frame::ID_TextEnc, Frame::TE_ISO8859_1},
            {Frame::ID_MimeType, QString()},
            {Frame::ID_Filename, QString()},
            {Frame::ID_Description, QString()},
            {Frame::ID_Data, QByteArray()},
            {Frame::ID_Id, QString()}
          };
        } else {
          frame.fieldList() = {
            {Frame::ID_TextEnc, Frame::TE_UTF8},
            {Frame::ID_Language, QString()},
            {Frame::ID_TimestampFormat, 2}, // milliseconds as unit
            {Frame::ID_ContentType, 0}, // other
            {Frame::ID_Description, QString()},
            {Frame::ID_Id, QString()},
            {Frame::ID_Data, QVariantList()}
          };
        }
      }
      if (f.m_extraFrames.isRead()) {
        frame.setIndex(Frame::toNegativeIndex(static_cast<int>(f.m_extraFrames.size())));
        f.m_extraFrames.append(frame);
        f.markTagChanged(tagNr, extendedType);
        return true;
      }
    }

    // Add a Matroska simple tag for the given frame.
    // To create simple tags with binary contents, " - binary" can be appended
    // to the name, it will be stripped away.
    bool isBinary = false;
    if (QString internalName = frame.getInternalName();
        internalName.endsWith(QLatin1String(" - binary"))) {
      isBinary = true;
      internalName.truncate(internalName.length() - 9);
      frame.setExtendedType(Frame::ExtendedType(frame.getType(), internalName));
    }
    TagLib::Matroska::SimpleTag::TargetTypeValue targetType;
    QString name = getMatroskaName(frame, targetType);
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
    if (!isBinary) {
      frame.fieldList() = {{Frame::ID_Text, frame.getValue()}};
    } else {
      frame.fieldList() = {{Frame::ID_Data, QByteArray()}};
    }
    frame.fieldList().append({
      {Frame::ID_TargetType, static_cast<int>(targetType) / 10},
      {Frame::ID_Language, QLatin1String("en")},
      {Frame::ID_Default, true},
      {Frame::ID_Id,  QLatin1String("0")}
    });
    frame.setIndex(mkaTag->simpleTagsList().size());
    mkaTag->addSimpleTag(frameToMatroskaSimpleTag(frame));
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
  return false;
}

bool TagLibMatroskaSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto mkaTag = dynamic_cast<TagLib::Matroska::Tag*>(f.m_tag[tagNr])) {
    if (Frame::ExtendedType extendedType = frame.getExtendedType();
        isExtraFrame(extendedType)) {
      if (f.m_extraFrames.isRead()) {
        if (int idx = Frame::fromNegativeIndex(frame.getIndex());
            idx >= 0 && idx < f.m_extraFrames.size()) {
          f.m_extraFrames.removeAt(idx);
          while (idx < f.m_extraFrames.size()) {
            f.m_extraFrames[idx].setIndex(Frame::toNegativeIndex(idx));
            ++idx;
          }
          f.markTagChanged(tagNr, extendedType);
          return true;
        }
      }
    }
    if (int index = frame.getIndex();
        index >= 0 && index < static_cast<int>(mkaTag->simpleTagsList().size())) {
      mkaTag->removeSimpleTag(index);
      f.markTagChanged(tagNr, frame.getExtendedType());
    }
  }
  return false;
}

bool TagLibMatroskaSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (auto mkaTag = dynamic_cast<TagLib::Matroska::Tag*>(f.m_tag[tagNr])) {
    if (flt.areAllEnabled()) {
      mkaTag->clearSimpleTags();
      f.m_extraFrames.clear();
      f.markTagChanged(tagNr, Frame::ExtendedType());
    } else {
      TagLib::Matroska::SimpleTagsList simpleTags = mkaTag->simpleTagsList();
      bool simpleTagRemoved = false;
      for (auto it = simpleTags.begin();
           it != simpleTags.end();) {
        QString name = toQString(it->name());
        Frame::Type type = getTypeFromMatroskaName(name, it->targetTypeValue());
        if (flt.isEnabled(type, name)) {
          simpleTagRemoved = true;
          it = simpleTags.erase(it);
        } else {
          ++it;
        }
      }
      if (simpleTagRemoved) {
        mkaTag->clearSimpleTags();
        mkaTag->addSimpleTags(simpleTags);
      }

      bool extraFrameRemoved = false;
      if (f.m_extraFrames.isRead()) {
        for (auto it = f.m_extraFrames.begin();
             it != f.m_extraFrames.end();) {
          if (flt.isEnabled(it->getType(), it->getInternalName())) {
            extraFrameRemoved = true;
            it = f.m_extraFrames.erase(it);
          } else {
            ++it;
          }
        }
        if (extraFrameRemoved) {
          int i = 0;
          for (Frame& frame : f.m_extraFrames) {
            frame.setIndex(Frame::toNegativeIndex(i++));
          }
        }
      }

      if (simpleTagRemoved || extraFrameRemoved) {
        f.markTagChanged(tagNr, Frame::ExtendedType());
      }
    }
    return true;
  }
  return false;
}

bool TagLibMatroskaSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (auto mkaTag = dynamic_cast<const TagLib::Matroska::Tag*>(f.m_tag[tagNr])) {
    const auto& simpleTags = mkaTag->simpleTagsList();
    int i = 0;
    for (const auto& simpleTag : simpleTags) {
      const QString name = toQString(simpleTag.name());
      Frame::Type type = getTypeFromMatroskaName(name, simpleTag.targetTypeValue());
      QString value;
      if (simpleTag.type() == TagLib::Matroska::SimpleTag::StringType) {
        value = toQString(simpleTag.toString());
      }
      Frame frame(type, value, name, i++);
      if (simpleTag.type() == TagLib::Matroska::SimpleTag::StringType) {
        frame.fieldList().append({Frame::ID_Text, value});
      } else if (simpleTag.type() == TagLib::Matroska::SimpleTag::BinaryType) {
        const TagLib::ByteVector bv = simpleTag.toByteVector();
        frame.fieldList().append(
          {Frame::ID_Data, QByteArray(bv.data(), bv.size())});
      }
      frame.fieldList().append({
        {Frame::ID_TargetType, static_cast<int>(simpleTag.targetTypeValue()) / 10},
        {Frame::ID_Language, toQString(simpleTag.language())},
        {Frame::ID_Default, simpleTag.defaultLanguageFlag()},
        {Frame::ID_Id,  QString::number(simpleTag.trackUid())}
      });
      frames.insert(frame);
    }
    if (f.m_extraFrames.isRead()) {
      for (auto it = f.m_extraFrames.constBegin();
           it != f.m_extraFrames.constEnd();
           ++it) {
        frames.insert(*it);
      }
    }
    return true;
  }
  return false;
}

QStringList TagLibMatroskaSupport::getFrameIds(
  const TagLibFile& f, Frame::TagNumber tagNr) const
{
  QStringList lst;
  if (dynamic_cast<TagLib::Matroska::Tag*>(f.m_tag[tagNr])) {
    static const char* const fieldNames[] = {
      "DIRECTOR",
      "DURATION",
      "SUMMARY",
      "SYNOPSIS",
      "TOTAL_PARTS",
      "Chapters",
      "General Object"
    };
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
        if (auto name = Frame::ExtendedType(static_cast<Frame::Type>(k),
                                            QLatin1String("")).getName();
            !name.isEmpty()) {
          lst.append(name);
        }
    }
    for (auto fieldName : fieldNames) {
      lst.append(QString::fromLatin1(fieldName)); // clazy:exclude=reserve-candidates
    }
  }
  return lst;
}
