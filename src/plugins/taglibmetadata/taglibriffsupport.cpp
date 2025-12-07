/**
 * \file taglibriffsupport.cpp
 * Support for WAV and AIFF files, INFO tags.
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

#include "taglibriffsupport.h"

#include <aifffile.h>
#include <aiffproperties.h>

#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

#if TAGLIB_VERSION >= 0x010a00
/**
 * Get name of INFO tag from type.
 *
 * @param type type
 *
 * @return name, NULL if not supported.
 */
TagLib::ByteVector getInfoNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    "INAM",  // FT_Title,
    "IART",  // FT_Artist,
    "IPRD",  // FT_Album,
    "ICMT",  // FT_Comment,
    "ICRD",  // FT_Date,
    "IPRT",  // FT_Track
    "IGNR",  // FT_Genre,
             // FT_LastV1Frame = FT_Track,
    nullptr, // FT_AlbumArtist,
    "IENG",  // FT_Arranger,
    nullptr, // FT_Author,
    "IBPM",  // FT_Bpm,
    nullptr, // FT_CatalogNumber,
    nullptr, // FT_Compilation,
    "IMUS",  // FT_Composer,
    nullptr, // FT_Conductor,
    "ICOP",  // FT_Copyright,
    nullptr, // FT_Disc,
    "ITCH",  // FT_EncodedBy,
    "ISFT",  // FT_EncoderSettings,
    "IDIT",  // FT_EncodingTime,
    nullptr, // FT_Grouping,
    nullptr, // FT_InitialKey,
    "ISRC",  // FT_Isrc,
    "ILNG",  // FT_Language,
    "IWRI",  // FT_Lyricist,
    nullptr, // FT_Lyrics,
    "IMED",  // FT_Media,
    nullptr, // FT_Mood,
    nullptr, // FT_OriginalAlbum,
    nullptr, // FT_OriginalArtist,
    nullptr, // FT_OriginalDate,
    nullptr, // FT_Description,
    "ISTR",  // FT_Performer,
    nullptr, // FT_Picture,
    "IPUB",  // FT_Publisher,
    "ICNT",  // FT_ReleaseCountry,
    "IEDT",  // FT_Remixer,
    nullptr, // FT_SortAlbum,
    nullptr, // FT_SortAlbumArtist,
    nullptr, // FT_SortArtist,
    nullptr, // FT_SortComposer,
    nullptr, // FT_SortName,
    "PRT1",  // FT_Subtitle,
    "IBSU",  // FT_Website,
    nullptr, // FT_WWWAudioFile,
    nullptr, // FT_WWWAudioSource,
    nullptr, // FT_ReleaseDate,
    "IRTD",  // FT_Rating,
    nullptr, // FT_Work,
             // FT_Custom1
  };
  Q_STATIC_ASSERT(std::size(names) == Frame::FT_Custom1);
  if (type == Frame::FT_Track) {
    QByteArray ba = TagConfig::instance().riffTrackName().toLatin1();
    return TagLib::ByteVector(ba.constData(), ba.size());
  }
  if (Frame::isCustomFrameType(type)) {
    return TagLib::ByteVector(Frame::getNameForCustomFrame(type).constData());
  }
  const char* name = type <= Frame::FT_LastFrame ? names[type] : nullptr;
  return name ? TagLib::ByteVector(name, 4) : TagLib::ByteVector();
}

/**
 * Get name of INFO tag from type.
 *
 * @param extendedType type
 *
 * @return name, NULL if not supported.
 */
TagLib::ByteVector getInfoNameFromType(const Frame::ExtendedType& extendedType)
{
  Frame::Type type = extendedType.getType();
  if (type == Frame::FT_Track) {
    // Do not change the track type to the configured track number field name
    // if it is already a valid track INFO type.
    const QString internalName = extendedType.getInternalName();
    if (const QStringList riffTrackNames = TagConfig::getRiffTrackNames();
        riffTrackNames.contains(internalName)) {
      auto infoName = internalName.toLatin1();
      return TagLib::ByteVector(infoName.constData(), infoName.size());
    }
  }
  return getInfoNameFromType(type);
}

/**
 * Get the frame type for an INFO name.
 *
 * @param id INFO tag name
 *
 * @return frame type.
 */
Frame::Type getTypeFromInfoName(const TagLib::ByteVector& id)
{
  static QMap<TagLib::ByteVector, int> strNumMap;
  if (strNumMap.isEmpty()) {
    // first time initialization
    for (int i = 0; i < Frame::FT_Custom1; ++i) {
      auto type = static_cast<Frame::Type>(i);
      if (TagLib::ByteVector str = getInfoNameFromType(type); !str.isEmpty()) {
        strNumMap.insert(str, type);
      }
    }
    QStringList riffTrackNames = TagConfig::getRiffTrackNames();
    riffTrackNames.append(TagConfig::instance().riffTrackName());
    const auto constRiffTrackNames = riffTrackNames;
    for (const QString& str : constRiffTrackNames) {
      QByteArray ba = str.toLatin1();
      strNumMap.insert(TagLib::ByteVector(ba.constData(), ba.size()),
                       Frame::FT_Track);
    }
  }
  if (auto it = strNumMap.constFind(id); it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::getTypeFromCustomFrameName(
        QByteArray(id.data(), id.size()));
}

/**
 * Get internal name of an INFO frame.
 *
 * @param frame frame
 *
 * @return INFO id, "IKEY" if not found.
 */
TagLib::ByteVector getInfoName(const Frame& frame)
{
  if (TagLib::ByteVector str = getInfoNameFromType(frame.getExtendedType());
      !str.isEmpty()) {
    return str;
  }

  if (QString name = frame.getInternalName(); name.length() >= 4) {
    QByteArray ba = name.left(4).toUpper().toLatin1();
    return TagLib::ByteVector(ba.constData(), 4);
  }

  return "IKEY";
}
#endif

}


/**
 * Destructor.
 */
WavFile::~WavFile()
{
  // not inline or default to silence weak-vtables warning
}

WavFile::WavFile(TagLib::IOStream *stream) : TagLib::RIFF::WAV::File(stream)
{
}

void WavFile::changeToLowercaseId3Chunk()
{
  if (readOnly() || !isValid())
    return;

  int i;
  for (i = chunkCount() - 1; i >= 0; --i) {
    if (chunkName(i) == "ID3 ") {
      break;
    }
  }
  if (i >= 0) {
    TagLib::ByteVector data = chunkData(i);
    removeChunk(i);
    setChunkData("id3 ", data);
  }
}


TagLib::File* TagLibRiffSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if(ext == "WAV")
    return new WavFile(stream);
  if (ext == "AIF" || ext == "AIFF")
    return new TagLib::RIFF::AIFF::File(stream);
  return nullptr;
}

bool TagLibRiffSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (auto wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
    f.m_fileExtension = QLatin1String(".wav");
    f.m_tag[Frame::Tag_1] = nullptr;
    f.markTagUnchanged(Frame::Tag_1);
#if TAGLIB_VERSION >= 0x010a00
    f.m_isTagSupported[Frame::Tag_3] = true;
    if (!f.m_tag[Frame::Tag_2]) {
      TagLib::ID3v2::Tag* id3v2Tag = wavFile->ID3v2Tag();
      f.setId3v2VersionFromTag(id3v2Tag);
      f.m_tag[Frame::Tag_2] = id3v2Tag;
      f.markTagUnchanged(Frame::Tag_2);
    }
    if (!f.m_tag[Frame::Tag_3]) {
      f.m_tag[Frame::Tag_3] = wavFile->InfoTag();
      f.markTagUnchanged(Frame::Tag_3);
    }
#else
    if (!f.m_tag[Frame::Tag_2]) {
      f.m_tag[Frame::Tag_2] = wavFile->tag();
      f.markTagUnchanged(Frame::Tag_2);
    }
#endif
    return true;
  }
  if (dynamic_cast<TagLib::RIFF::AIFF::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".aiff");
    putFileRefTagInTag2(f);
    return true;
  }
  return false;
}

bool TagLibRiffSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
  int id3v2Version, bool& fileChanged) const
{
  if (auto wavFile = dynamic_cast<WavFile*>(file)) {
    if (anyTagMustBeSaved(f, force)) {
      static constexpr TagLib::RIFF::WAV::File::TagTypes tagTypes[TagLibFile::NUM_TAGS] = {
        TagLib::RIFF::WAV::File::NoTags, TagLib::RIFF::WAV::File::ID3v2,
  #if TAGLIB_VERSION >= 0x010a00
        TagLib::RIFF::WAV::File::Info
  #else
        TagLib::RIFF::WAV::File::NoTags
  #endif
      };
      int saveTags = 0;
      FOR_TAGLIB_TAGS(tagNr) {
        if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr)) &&
            f.m_tag[tagNr]->isEmpty()) {
          f.m_tag[tagNr] = nullptr;
        } else {
          saveTags |= tagTypes[tagNr];
        }
      }
      f.setId3v2VersionOrDefault(id3v2Version);
      if (
  #if TAGLIB_VERSION >= 0x010c00
          wavFile->save(
            static_cast<TagLib::RIFF::WAV::File::TagTypes>(saveTags),
            TagLib::File::StripOthers,
            f.m_id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3)
  #else
          wavFile->save(static_cast<TagLib::RIFF::WAV::File::TagTypes>(
                          saveTags), true, f.m_id3v2Version)
  #endif
          ) {
        if (TagConfig::instance().lowercaseId3RiffChunk()) {
          wavFile->changeToLowercaseId3Chunk();
        }
        fileChanged = true;
        FOR_TAGLIB_TAGS(tagNr) {
          f.markTagUnchanged(tagNr);
        }
        return true;
      }
      if (saveFileRef(f)) {
        fileChanged = true;
      }
    }
    return true;
  }
  return false;
}

bool TagLibRiffSupport::makeTagSettable(TagLibFile& f, TagLib::File* file,
  Frame::TagNumber tagNr) const
{
  if (tagNr == Frame::Tag_2) {
    if (auto wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
      f.m_tag[tagNr] = wavFile->ID3v2Tag();
      return true;
    }
  }
#if TAGLIB_VERSION >= 0x010a00
  else if (tagNr == Frame::Tag_3) {
    if (auto wavFile = dynamic_cast<TagLib::RIFF::WAV::File*>(file)) {
      f.m_tag[tagNr] = wavFile->InfoTag();
      return true;
    }
  }
#endif
  return false;
}

bool TagLibRiffSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto wavProperties =
      dynamic_cast<TagLib::RIFF::WAV::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QLatin1String("WAV");
#if TAGLIB_VERSION >= 0x010a00
    if (int format = wavProperties->format(); format > 0) {
      // https://tools.ietf.org/html/rfc2361#appendix-A
      static const struct {
        int code;
        const char* name;
      } codeToName[] = {
        {0x0001, "PCM"}, {0x0002, "ADPCM"}, {0x003, "IEEE Float"},
        {0x0004, "VSELP"}, {0x0005, "IBM CVSD"}, {0x0006, "ALAW"},
        {0x0007, "MULAW"}, {0x0010, "OKI ADPCM"}, {0x0011, "DVI ADPCM"},
        {0x0012, "MediaSpace ADPCM"}, {0x0013, "Sierra ADPCM"},
        {0x0014, "G.723 ADPCM"}, {0x0015, "DIGISTD"}, {0x0016, "DIGIFIX"},
        {0x0017, "OKI ADPCM"}, {0x0018, "MediaVision ADPCM"}, {0x0019, "CU"},
        {0x0020, "Yamaha ADPCM"}, {0x0021, "Sonarc"}, {0x0022, "True Speech"},
        {0x0023, "EchoSC1"}, {0x0024, "AF36"}, {0x0025, "APTX"},
        {0x0026, "AF10"}, {0x0027, "Prosody 1612"}, {0x0028, "LRC"},
        {0x0030, "Dolby AC2"}, {0x0031, "GSM610"}, {0x0032, "MSNAudio"},
        {0x0033, "Antex ADPCME"}, {0x0034, "Control Res VQLPC"}, {0x0035, "Digireal"},
        {0x0036, "DigiADPCM"}, {0x0037, "Control Res CR10"}, {0x0038, "NMS VBXADPCM"},
        {0x0039, "Roland RDAC"}, {0x003a, "EchoSC3"}, {0x003b, "Rockwell ADPCM"},
        {0x003c, "Rockwell DIGITALK"}, {0x003d, "Xebec"}, {0x0040, "G.721 ADPCM"},
        {0x0041, "G.728 CELP"}, {0x0042, "MSG723"}, {0x0050, "MPEG"},
        {0x0052, "RT24"}, {0x0053, "PAC"}, {0x0055, "MPEG Layer 3"},
        {0x0059, "Lucent G.723"}, {0x0060, "Cirrus"}, {0x0061, "ESPCM"},
        {0x0062, "Voxware"}, {0x0063, "Canopus Atrac"}, {0x0064, "G.726 ADPCM"},
        {0x0065, "G.722 ADPCM"}, {0x0066, "DSAT"}, {0x0067, "DSAT Display"},
        {0x0069, "Voxware Byte Aligned"}, {0x0070, "Voxware AC8"}, {0x0071, "Voxware AC10"},
        {0x0072, "Voxware AC16"}, {0x0073, "Voxware AC20"}, {0x0074, "Voxware MetaVoice"},
        {0x0075, "Voxware MetaSound"}, {0x0076, "Voxware RT29HW"}, {0x0077, "Voxware VR12"},
        {0x0078, "Voxware VR18"}, {0x0079, "Voxware TQ40"}, {0x0080, "Softsound"},
        {0x0081, "Voxware TQ60"}, {0x0082, "MSRT24"}, {0x0083, "G.729A"},
        {0x0084, "MVI MV12"}, {0x0085, "DF G.726"}, {0x0086, "DF GSM610"},
        {0x0088, "ISIAudio"}, {0x0089, "Onlive"}, {0x0091, "SBC24"},
        {0x0092, "Dolby AC3 SPDIF"}, {0x0097, "ZyXEL ADPCM"}, {0x0098, "Philips LPCBB"},
        {0x0099, "Packed"}, {0x0100, "Rhetorex ADPCM"}, {0x0101, "IRAT"},
        {0x0111, "Vivo G.723"}, {0x0112, "Vivo Siren"}, {0x0123, "Digital G.723"},
        {0x0200, "Creative ADPCM"}, {0x0202, "Creative FastSpeech8"}, {0x0203, "Creative FastSpeech10"},
        {0x0220, "Quarterdeck"}, {0x0300, "FM Towns Snd"}, {0x0400, "BTV Digital"},
        {0x0680, "VME VMPCM"}, {0x1000, "OLIGSM"}, {0x1001, "OLIADPCM"},
        {0x1002, "OLICELP"}, {0x1003, "OLISBC"}, {0x1004, "OLIOPR"},
        {0x1100, "LH Codec"}, {0x1400, "Norris"}, {0x1401, "ISIAudio"},
        {0x1500, "Soundspace Music Compression"}, {0x2000, "DVM"}
      };
      for (const auto& [code, name] : codeToName) {
        if (format == code) {
          f.m_detailInfo.format += QLatin1Char(' ');
          f.m_detailInfo.format += QString::fromLatin1(name);
          break;
        }
      }
    }
    if (int bits = wavProperties->bitsPerSample(); bits > 0) {
      f.m_detailInfo.format += QLatin1Char(' ');
      f.m_detailInfo.format += QString::number(bits);
      f.m_detailInfo.format += QLatin1String(" bit");
    }
#endif
    return true;
  }
  if (auto aiffProperties =
      dynamic_cast<TagLib::RIFF::AIFF::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QLatin1String("AIFF");
#if TAGLIB_VERSION >= 0x010a00
    if (int bits = aiffProperties->bitsPerSample(); bits > 0) {
      f.m_detailInfo.format += QLatin1Char(' ');
      f.m_detailInfo.format += QString::number(bits);
      f.m_detailInfo.format += QLatin1String(" bit");
    }
#endif
    return true;
  }
  return false;
}

QString TagLibRiffSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType& type) const
{
#if TAGLIB_VERSION >= 0x010a00
  if (dynamic_cast<const TagLib::RIFF::Info::Tag*>(tag) != nullptr) {
    type = TaggedFile::TT_Info;
    return QLatin1String("RIFF INFO");
  }
#endif
  return QString();
}

bool TagLibRiffSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
#if TAGLIB_VERSION >= 0x010a00
  if (auto infoTag = dynamic_cast<TagLib::RIFF::Info::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      infoTag->setFieldText(getInfoName(frame), toTString(frame.getValue()));
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
#endif
  return false;
}

bool TagLibRiffSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
#if TAGLIB_VERSION >= 0x010a00
  if (auto infoTag = dynamic_cast<TagLib::RIFF::Info::Tag*>(f.m_tag[tagNr])) {
    TagLib::ByteVector id = getInfoName(frame);
    TagLib::String tvalue = toTString(frame.getValue());
    if (tvalue.isEmpty()) {
      tvalue = " "; // empty values are not added by TagLib
    }
    infoTag->setFieldText(id, tvalue);
    QString name = QString::fromLatin1(id.data(), id.size());
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
    const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
    int index = 0;
    bool found = false;
    for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
      if (it->first == id) {
        found = true;
        break;
      }
      ++index;
    }
    frame.setIndex(found ? index : -1);
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
#endif
  return false;
}

bool TagLibRiffSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
#if TAGLIB_VERSION >= 0x010a00
  if (auto infoTag = dynamic_cast<TagLib::RIFF::Info::Tag*>(f.m_tag[tagNr])) {
    QByteArray ba = frame.getInternalName().toLatin1();
    TagLib::ByteVector id(ba.constData(), ba.size());
    infoTag->removeField(id);
    f.markTagChanged(tagNr, frame.getExtendedType());
    return true;
  }
#endif
  return false;
}

bool TagLibRiffSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
#if TAGLIB_VERSION >= 0x010a00
  if (auto infoTag = dynamic_cast<TagLib::RIFF::Info::Tag*>(f.m_tag[tagNr])) {
    const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
    if (flt.areAllEnabled()) {
      for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
        infoTag->removeField(it->first);
      }
    } else {
      for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
        TagLib::ByteVector id = it->first;
        if (QString name = QString::fromLatin1(id.data(), id.size());
            flt.isEnabled(getTypeFromInfoName(id), name)) {
          infoTag->removeField(id);
        }
      }
    }
    f.markTagChanged(tagNr, Frame::ExtendedType());
    return true;
  }
#endif
  return false;
}

bool TagLibRiffSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
#if TAGLIB_VERSION >= 0x010a00
  if (auto infoTag = dynamic_cast<TagLib::RIFF::Info::Tag*>(f.m_tag[tagNr])) {
    const TagLib::RIFF::Info::FieldListMap itemListMap = infoTag->fieldListMap();
    int i = 0;
    for (auto it = itemListMap.begin(); it != itemListMap.end(); ++it) {
      TagLib::ByteVector id = it->first;
      TagLib::String s = it->second;
      QString name = QString::fromLatin1(id.data(), id.size());
      QString value = toQString(s);
      Frame::Type type = getTypeFromInfoName(id);
      Frame frame(type, value, name, i++);
      frames.insert(frame);
    }
    return true;
  }
#endif
  return false;
}

QStringList TagLibRiffSupport::getFrameIds(
  const TagLibFile& f, Frame::TagNumber tagNr) const
{
  QStringList lst;
#if TAGLIB_VERSION >= 0x010a00
  if (f.m_tagType[tagNr] == TaggedFile::TT_Info) {
    static const char* const fieldNames[] = {
      "IARL", // Archival Location
      "ICMS", // Commissioned
      "ICRP", // Cropped
      "IDIM", // Dimensions
      "IDPI", // Dots Per Inch
      "IKEY", // Keywords
      "ILGT", // Lightness
      "IPLT", // Palette Setting
      "ISBJ", // Subject
      "ISHP", // Sharpness
      "ISRF", // Source Form
    };
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      if (auto type = static_cast<Frame::Type>(k);
          !getInfoNameFromType(type).isEmpty()) {
        lst.append(Frame::ExtendedType(type, QLatin1String("")).getName()); // clazy:exclude=reserve-candidates
      }
    }
    for (auto fieldName : fieldNames) {
      lst.append(QString::fromLatin1(fieldName)); // clazy:exclude=reserve-candidates
    }
  }
#endif
  return lst;
}

void TagLibRiffSupport::setTagValue(TagLibFile& f, Frame::TagNumber tagNr, Frame::Type type, const TagLib::String& str) const
{
  TagLib::Tag* tag = f.m_tag[tagNr];
  if (type == Frame::FT_Track) {
    if (auto infoTag = dynamic_cast<TagLib::RIFF::Info::Tag*>(tag)) {
      infoTag->setFieldText(getInfoNameFromType(Frame::FT_Track), str);
      return;
    }
  }
  TagLibFormatSupport::setTagValue(f, tagNr, type, str);
}
