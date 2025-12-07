/**
 * \file taglibmpegsupport.cpp
 * Support for MP3 files and ID3 tags.
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

#include "taglibmpegsupport.h"
#include "taglibfile.h"

#include <apetag.h>
#include <attachedpictureframe.h>
#include <commentsframe.h>
#include <generalencapsulatedobjectframe.h>
#include <id3v1genres.h>
#include <id3v1tag.h>
#include <id3v2framefactory.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <ownershipframe.h>
#include <popularimeterframe.h>
#include <privateframe.h>
#include <relativevolumeframe.h>
#include <textidentificationframe.h>
#include <uniquefileidentifierframe.h>
#include <unsynchronizedlyricsframe.h>
#include <urllinkframe.h>

#if TAGLIB_VERSION >= 0x010a00
#include <synchronizedlyricsframe.h>
#include <eventtimingcodesframe.h>
#include <chapterframe.h>
#include <tableofcontentsframe.h>
#else
#include "taglibext/synchronizedlyricsframe.h"
#include "taglibext/eventtimingcodesframe.h"
#endif
#if TAGLIB_VERSION >= 0x010b00
#include <podcastframe.h>
#endif

#include "attributedata.h"
#include "genres.h"
#include "taglibutils.h"

using namespace TagLibUtils;

namespace TagLibMpegSupportInternal {

/**
 * Fix up the format of the value if needed for an ID3v2 frame.
 *
 * @param self this TagLibFile instance
 * @param frameType type of frame
 * @param value the value to be set for frame, will be modified if needed
 */
void fixUpTagLibFrameValue(const TagLibFile* self,
                           Frame::Type frameType, QString& value)
{
  if (frameType == Frame::FT_Genre) {
    if (const bool useId3v23 = self->m_id3v2Version == 3;
        !TagConfig::instance().genreNotNumeric() ||
        (useId3v23 && value.contains(Frame::stringListSeparator()))) {
      value = Genres::getNumberString(value, useId3v23);
    }
  } else if (frameType == Frame::FT_Track) {
    self->formatTrackNumberIfEnabled(value, true);
  } else if ((frameType == Frame::FT_Arranger ||
              frameType == Frame::FT_Performer) &&
             !value.isEmpty() &&
             !value.contains(Frame::stringListSeparator())) {
    // When using TIPL or TMCL and writing an ID3v2.3.0 tag, TagLib
    // needs in ID3v2::Tag::downgradeFrames() a string list with at
    // least two elements, otherwise it will not take the value over
    // to an IPLS frame. If there is a single value in such a case,
    // add a second element.
    value = Frame::joinStringList({value, QLatin1String("")});
  }
}

}

namespace {

#if TAGLIB_VERSION >= 0x010a00
void setChaptersFrameFields(Frame& frame,
                            const QString& description = QString(),
                            const QVariantList& data = QVariantList())
{
  frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                                            QLatin1String("Chapters")));
  frame.setValue(QString(QLatin1String("")));

  Frame::Field field;
  Frame::FieldList& fields = frame.fieldList();
  fields.clear();

  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = 2; // milliseconds
  fields.append(field);

  field.m_id = Frame::ID_ContentType;
  field.m_value = 0; // other
  fields.append(field);

  field.m_id = Frame::ID_Description;
  field.m_value = description;
  fields.append(field);

  field.m_id = Frame::ID_Data;
  field.m_value = data;
  fields.append(field);
}

bool ctocChapToChaptersFrame(const TagLib::ID3v2::Frame* ctocFrame,
                             const TagLib::ID3v2::FrameList& chapFrames,
                             Frame& frame)
{
  if (auto ctoc = dynamic_cast<const TagLib::ID3v2::TableOfContentsFrame*>(
        ctocFrame); ctoc && ctoc->isTopLevel()) {
    QMap<TagLib::ByteVector,
         std::tuple<unsigned int, unsigned int, TagLib::String>> chapters;
    for (const auto& id3v2Frame : chapFrames) {
      if (auto chap = dynamic_cast<TagLib::ID3v2::ChapterFrame*>(id3v2Frame)) {
        TagLib::ByteVector elementId = chap->elementID();
        unsigned int startTime = chap->startTime();
        unsigned int endTime = chap->endTime();
        TagLib::String title;
        if (const auto& titleFrames = chap->embeddedFrameList("TIT2");
            !titleFrames.isEmpty()) {
          title = titleFrames.front()->toString();
        }
        chapters.insert(elementId, {startTime, endTime, title});
      }
    }

    QVariantList data;
    quint32 time = 0;
    const auto tocElements = ctoc->childElements();
    for (const auto& tocElement : tocElements) {
      if (auto it = chapters.constFind(tocElement);
          it != chapters.constEnd()) {
        const auto& [ startTime, endTime, title ] = *it;
        data.append(startTime);
        data.append(toQString(title));
        time = endTime;
      }
    }
    data.append(time);
    data.append(QString());

    TagLib::String tocTitle;
    if (const auto& tocTitleFrames = ctoc->embeddedFrameList("TIT2");
        !tocTitleFrames.isEmpty()) {
      tocTitle = tocTitleFrames.front()->toString();
    }

    setChaptersFrameFields(frame, toQString(tocTitle), data);
    return true;
  }
  return false;
}

void chaptersFrameToCtocChap(const Frame& frame, TagLib::ID3v2::Tag* id3v2Tag)
{
  QVariantList data = Frame::getField(frame, Frame::ID_Data).toList();
  int dataLen = data.size();
  if (dataLen >= 2) {
    quint32 lastTime = data.at(dataLen - 2).toUInt();
    if (QString lastTitle = data.at(dataLen - 1).toString();
        !lastTitle.trimmed().isEmpty()) {
      data.append(lastTime);
      data.append(QString());
      dataLen += 2;
    }
  }
  if (dataLen > 2 && (dataLen & 1) == 0) {
    int chapterCount = (dataLen - 2) / 2;

    TagLib::ID3v2::TableOfContentsFrame* ctocFrame = nullptr;
    const auto ctocFrames = id3v2Tag->frameList("CTOC");
    for (auto id3v2Frame : ctocFrames) {
      if (auto ctoc = dynamic_cast<TagLib::ID3v2::TableOfContentsFrame*>(
            id3v2Frame); ctoc && ctoc->isTopLevel()) {
        ctocFrame = ctoc;
        break;
      }
    }
    if (!ctocFrame) {
      ctocFrame = new TagLib::ID3v2::TableOfContentsFrame("toc01");
      ctocFrame->setIsTopLevel(true);
      id3v2Tag->addFrame(ctocFrame);
    }
    TagLib::ByteVectorList elementIds;
    for (int i = 1; i <= chapterCount; ++i) {
      auto chapId = QString(QLatin1String("chp%1"))
                    .arg(i, 2, 10, QLatin1Char('0'))
                    .toLatin1();
      elementIds.append(TagLib::ByteVector(chapId.data(), chapId.size()));
    }
    ctocFrame->setChildElements(elementIds);

    auto description = toTString(Frame::getField(frame, Frame::ID_Description)
                                 .toString());
    if (auto ctocTitleFrames = ctocFrame->embeddedFrameList("TIT2");
        !ctocTitleFrames.isEmpty()) {
      ctocTitleFrames.front()->setText(description);
    } else if (!description.isEmpty()) {
      auto tit2Frame = new TagLib::ID3v2::TextIdentificationFrame(
                         "TIT2", TagLib::String::UTF16);
      tit2Frame->setText(description);
      ctocFrame->addEmbeddedFrame(tit2Frame);
    }

    const auto ctocs = id3v2Tag->frameList("CTOC");
    for (auto it = ctocs.begin(); it != ctocs.end(); ++it) {
      if (*it != ctocFrame) {
        id3v2Tag->removeFrame(*it, true);
      }
    }

    int numChapFramesToAdd = chapterCount - id3v2Tag->frameList("CHAP").size();
    while (numChapFramesToAdd-- > 0) {
      id3v2Tag->addFrame(new TagLib::ID3v2::ChapterFrame(
                           " ", 0, 0, 0xFFFFFFFF, 0xFFFFFFFF));
    }
    const auto chaps = id3v2Tag->frameList("CHAP");
    for (auto it = std::next(chaps.begin(), chapterCount);
         it != chaps.end();
         ++it) {
      id3v2Tag->removeFrame(*it, true);
    }

    QList<TagLib::ID3v2::ChapterFrame*> chapFrames;
    chapFrames.reserve(chapterCount);
    QList<TagLib::String> chapTitles;
    chapTitles.reserve(chapterCount);
    QList<TagLib::ByteVector> chapIds;
    chapIds.reserve(chapterCount);
    for (auto id3v2Frame : id3v2Tag->frameList("CHAP")) {
      if (auto chap = dynamic_cast<TagLib::ID3v2::ChapterFrame*>(id3v2Frame)) {
        TagLib::String chapTitle;
        if (auto chapTitleFrames = chap->embeddedFrameList("TIT2");
            !chapTitleFrames.isEmpty()) {
          chapTitle = chapTitleFrames.front()->toString();
        } else {
          chap->addEmbeddedFrame(new TagLib::ID3v2::TextIdentificationFrame(
                                   "TIT2", TagLib::String::UTF16));
        }
        chapFrames.append(chap);
        chapTitles.append(chapTitle);
        chapIds.append(chap->elementID());
      }
    }

    TagLib::ID3v2::ChapterFrame* lastChapFrame = nullptr;
    int i = 0;
    QListIterator<QVariant> it(data);
    while (it.hasNext()) {
      unsigned int time = it.next().toUInt();
      if (lastChapFrame) {
        lastChapFrame->setEndTime(time);
        lastChapFrame->setEndOffset(0xFFFFFFFF);
      }
      if (!it.hasNext() || chapFrames.isEmpty()) {
        break;
      }

      if (i < chapterCount) {
        TagLib::String chapterTitle = toTString(it.next().toString().trimmed());
        int idx = chapTitles.indexOf(chapterTitle);
        if (idx == -1) {
          idx = chapIds.indexOf(elementIds[i]);
          if (idx == -1) {
            idx = 0;
          }
        }
        chapTitles.removeAt(idx);
        chapIds.removeAt(idx);
        TagLib::ID3v2::ChapterFrame* chapFrame = chapFrames.takeAt(idx);
        chapFrame->setElementID(elementIds[i]);
        chapFrame->setStartTime(time);
        chapFrame->setStartOffset(0xFFFFFFFF);
        chapFrame->embeddedFrameList("TIT2").front()->setText(chapterTitle);
        lastChapFrame = chapFrame;
      } else {
        lastChapFrame = nullptr;
      }
      ++i;
    }
  }
}
#endif

/**
 * Check if string needs Unicode encoding.
 *
 * @return true if Unicode needed,
 *         false if Latin-1 sufficient.
 */
bool needsUnicode(const QString& qstr)
{
  bool result = false;
  uint unicodeSize = qstr.length();
  const QChar* qcarray = qstr.unicode();
  for (uint i = 0; i < unicodeSize; ++i) {
    if (char ch = qcarray[i].toLatin1();
        ch == 0 || (ch & 0x80) != 0) {
      result = true;
      break;
    }
  }
  return result;
}

/**
 * Check if string needs Unicode encoding.
 *
 * @return true if Unicode needed,
 *         false if Latin-1 sufficient.
 */
bool needsUnicode(const TagLib::String& str)
{
  for (auto ch : str.toWString()) {
    if (ch >= 0x80) {
      return true;
    }
  }
  return false;
}

/**
 * Get the configured text encoding.
 *
 * @param unicode true if unicode is required
 *
 * @return text encoding.
 */
TagLib::String::Type getTextEncodingConfig(bool unicode)
{
  TagLib::String::Type enc = TagLibFile::getDefaultTextEncoding();
  if (unicode && enc == TagLib::String::Latin1) {
    enc = TagLib::String::UTF8;
  }
  return enc;
}

/**
 * Remove the first COMM frame with an empty description.
 *
 * @param id3v2Tag ID3v2 tag
 */
void removeCommentFrame(TagLib::ID3v2::Tag* id3v2Tag)
{
  const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList("COMM");
  for (auto it = frameList.begin();
       it != frameList.end();
       ++it) {
    if (auto id3Frame =
          dynamic_cast<TagLib::ID3v2::CommentsFrame*>(*it);
        id3Frame && id3Frame->description().isEmpty()) {
      id3v2Tag->removeFrame(id3Frame, true);
      break;
    }
  }
}

void addTagLibFrame(TagLib::ID3v2::Tag* id3v2Tag, TagLib::ID3v2::Frame* frame)
{
#ifdef Q_OS_WIN32
  // freed in Windows DLL => must be allocated in the same DLL
#if TAGLIB_VERSION >= 0x020000
  TagLib::ID3v2::Header tagHeader;
  tagHeader.setMajorVersion(4);
  TagLib::ID3v2::Frame* dllAllocatedFrame =
      TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render(),
                                                           &tagHeader);
#else
  TagLib::ID3v2::Frame* dllAllocatedFrame =
      TagLib::ID3v2::FrameFactory::instance()->createFrame(frame->render());
#endif
  if (dllAllocatedFrame) {
    id3v2Tag->addFrame(dllAllocatedFrame);
  }
  delete frame;
#else
  id3v2Tag->addFrame(frame);
#endif
}

/**
 * Write a Unicode field if the tag is ID3v2 and Latin-1 is not sufficient.
 *
 * @param id3v2Tag tag
 * @param tstr     text as TagLib::String
 * @param frameId  ID3v2 frame ID
 *
 * @return true if an ID3v2 Unicode field was written.
 */
bool setId3v2Unicode(TagLib::ID3v2::Tag* id3v2Tag,
                     const TagLib::String& tstr, const char* frameId)
{
  // first check if this string needs to be stored as unicode
  TagLib::String::Type enc = getTextEncodingConfig(needsUnicode(tstr));
  if (TagLib::ByteVector id(frameId);
      enc != TagLib::String::Latin1 || id == "COMM" || id == "TDRC") {
    if (id == "COMM") {
      removeCommentFrame(id3v2Tag);
    } else {
      id3v2Tag->removeFrames(id);
    }
    if (!tstr.isEmpty()) {
      TagLib::ID3v2::Frame* frame;
      if (frameId[0] != 'C') {
        frame = new TagLib::ID3v2::TextIdentificationFrame(id, enc);
      } else {
        auto commFrame =
            new TagLib::ID3v2::CommentsFrame(enc);
        frame = commFrame;
        commFrame->setLanguage("eng"); // for compatibility with iTunes
      }
      frame->setText(tstr);
      addTagLibFrame(id3v2Tag, frame);
    }
    return true;
  }
  return false;
}


/** Types and descriptions for id3lib frame IDs */
const struct TypeStrOfId {
  const char* str;
  Frame::Type type;
  bool supported;
} typeStrOfId[] = {
  { QT_TRANSLATE_NOOP("@default", "AENC - Audio encryption"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "APIC - Attached picture"), Frame::FT_Picture, true },
  { QT_TRANSLATE_NOOP("@default", "ASPI - Audio seek point index"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010a00
  { QT_TRANSLATE_NOOP("@default", "CHAP - Chapter"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "COMM - Comments"), Frame::FT_Comment, true },
  { QT_TRANSLATE_NOOP("@default", "COMR - Commercial"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010a00
  { QT_TRANSLATE_NOOP("@default", "CTOC - Table of contents"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "ENCR - Encryption method registration"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "EQU2 - Equalisation (2)"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "ETCO - Event timing codes"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "GEOB - General encapsulated object"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "GRID - Group identification registration"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010c00
  { QT_TRANSLATE_NOOP("@default", "GRP1 - Grouping"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "LINK - Linked information"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "MCDI - Music CD identifier"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "MLLT - MPEG location lookup table"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010c00
  { QT_TRANSLATE_NOOP("@default", "MVIN - Movement Number"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "MVNM - Movement Name"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "OWNE - Ownership frame"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "PRIV - Private frame"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "PCNT - Play counter"), Frame::FT_Other, false },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "PCST - Podcast"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "POPM - Popularimeter"), Frame::FT_Rating, true },
  { QT_TRANSLATE_NOOP("@default", "POSS - Position synchronisation frame"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "RBUF - Recommended buffer size"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "RVA2 - Relative volume adjustment (2)"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "RVRB - Reverb"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "SEEK - Seek frame"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "SIGN - Signature frame"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "SYLT - Synchronized lyric/text"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "SYTC - Synchronized tempo codes"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "TALB - Album/Movie/Show title"), Frame::FT_Album, true },
  { QT_TRANSLATE_NOOP("@default", "TBPM - BPM (beats per minute)"),  Frame::FT_Bpm, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TCAT - Podcast category"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TCMP - iTunes compilation flag"), Frame::FT_Compilation, true },
  { QT_TRANSLATE_NOOP("@default", "TCOM - Composer"), Frame::FT_Composer, true },
  { QT_TRANSLATE_NOOP("@default", "TCON - Content type"), Frame::FT_Genre, true },
  { QT_TRANSLATE_NOOP("@default", "TCOP - Copyright message"), Frame::FT_Copyright, true },
  { QT_TRANSLATE_NOOP("@default", "TDEN - Encoding time"), Frame::FT_EncodingTime, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TDES - Podcast description"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TDLY - Playlist delay"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TDOR - Original release time"), Frame::FT_OriginalDate, true },
  { QT_TRANSLATE_NOOP("@default", "TDRC - Recording time"), Frame::FT_Date, true },
  { QT_TRANSLATE_NOOP("@default", "TDRL - Release time"), Frame::FT_ReleaseDate, true },
  { QT_TRANSLATE_NOOP("@default", "TDTG - Tagging time"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TENC - Encoded by"), Frame::FT_EncodedBy, true },
  { QT_TRANSLATE_NOOP("@default", "TEXT - Lyricist/Text writer"), Frame::FT_Lyricist, true },
  { QT_TRANSLATE_NOOP("@default", "TFLT - File type"), Frame::FT_Other, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TGID - Podcast identifier"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TIPL - Involved people list"), Frame::FT_Arranger, true },
  { QT_TRANSLATE_NOOP("@default", "TIT1 - Content group description"), Frame::FT_Work, true },
  { QT_TRANSLATE_NOOP("@default", "TIT2 - Title/songname/content description"), Frame::FT_Title, true },
  { QT_TRANSLATE_NOOP("@default", "TIT3 - Subtitle/Description refinement"), Frame::FT_Description, true },
  { QT_TRANSLATE_NOOP("@default", "TKEY - Initial key"), Frame::FT_InitialKey, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "TKWD - Podcast keywords"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "TLAN - Language(s)"), Frame::FT_Language, true },
  { QT_TRANSLATE_NOOP("@default", "TLEN - Length"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TMCL - Musician credits list"), Frame::FT_Performer, true },
  { QT_TRANSLATE_NOOP("@default", "TMED - Media type"), Frame::FT_Media, true },
  { QT_TRANSLATE_NOOP("@default", "TMOO - Mood"), Frame::FT_Mood, true },
  { QT_TRANSLATE_NOOP("@default", "TOAL - Original album/movie/show title"), Frame::FT_OriginalAlbum, true },
  { QT_TRANSLATE_NOOP("@default", "TOFN - Original filename"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TOLY - Original lyricist(s)/text writer(s)"), Frame::FT_Author, true },
  { QT_TRANSLATE_NOOP("@default", "TOPE - Original artist(s)/performer(s)"), Frame::FT_OriginalArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TOWN - File owner/licensee"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TPE1 - Lead performer(s)/Soloist(s)"), Frame::FT_Artist, true },
  { QT_TRANSLATE_NOOP("@default", "TPE2 - Band/orchestra/accompaniment"), Frame::FT_AlbumArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TPE3 - Conductor/performer refinement"), Frame::FT_Conductor, true },
  { QT_TRANSLATE_NOOP("@default", "TPE4 - Interpreted, remixed, or otherwise modified by"), Frame::FT_Remixer, true },
  { QT_TRANSLATE_NOOP("@default", "TPOS - Part of a set"), Frame::FT_Disc, true },
  { QT_TRANSLATE_NOOP("@default", "TPRO - Produced notice"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TPUB - Publisher"), Frame::FT_Publisher, true },
  { QT_TRANSLATE_NOOP("@default", "TRCK - Track number/Position in set"), Frame::FT_Track, true },
  { QT_TRANSLATE_NOOP("@default", "TRSN - Internet radio station name"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TRSO - Internet radio station owner"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "TSO2 - Album artist sort order"), Frame::FT_SortAlbumArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TSOA - Album sort order"), Frame::FT_SortAlbum, true },
  { QT_TRANSLATE_NOOP("@default", "TSOC - Composer sort order"), Frame::FT_SortComposer, true },
  { QT_TRANSLATE_NOOP("@default", "TSOP - Performer sort order"), Frame::FT_SortArtist, true },
  { QT_TRANSLATE_NOOP("@default", "TSOT - Title sort order"), Frame::FT_SortName, true },
  { QT_TRANSLATE_NOOP("@default", "TSRC - ISRC (international standard recording code)"), Frame::FT_Isrc, true },
  { QT_TRANSLATE_NOOP("@default", "TSSE - Software/Hardware and settings used for encoding"), Frame::FT_EncoderSettings, true },
  { QT_TRANSLATE_NOOP("@default", "TSST - Set subtitle"), Frame::FT_Subtitle, true },
  { QT_TRANSLATE_NOOP("@default", "TXXX - User defined text information"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "UFID - Unique file identifier"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "USER - Terms of use"), Frame::FT_Other, false },
  { QT_TRANSLATE_NOOP("@default", "USLT - Unsynchronized lyric/text transcription"), Frame::FT_Lyrics, true },
  { QT_TRANSLATE_NOOP("@default", "WCOM - Commercial information"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WCOP - Copyright/Legal information"), Frame::FT_Other, true },
#if TAGLIB_VERSION >= 0x010b00
  { QT_TRANSLATE_NOOP("@default", "WFED - Podcast feed"), Frame::FT_Other, true },
#endif
  { QT_TRANSLATE_NOOP("@default", "WOAF - Official audio file webpage"), Frame::FT_WWWAudioFile, true },
  { QT_TRANSLATE_NOOP("@default", "WOAR - Official artist/performer webpage"), Frame::FT_Website, true },
  { QT_TRANSLATE_NOOP("@default", "WOAS - Official audio source webpage"), Frame::FT_WWWAudioSource, true },
  { QT_TRANSLATE_NOOP("@default", "WORS - Official internet radio station homepage"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WPAY - Payment"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WPUB - Official publisher webpage"), Frame::FT_Other, true },
  { QT_TRANSLATE_NOOP("@default", "WXXX - User defined URL link"), Frame::FT_Other, true }
};

/**
 * Get type and description of frame.
 *
 * @param id   ID of frame
 * @param type the type is returned here
 * @param str  the description is returned here
 */
void getTypeStringForFrameId(const TagLib::ByteVector& id, Frame::Type& type,
                             const char*& str)
{
  static TagLib::Map<TagLib::ByteVector, unsigned> idIndexMap;
  if (idIndexMap.isEmpty()) {
    for (unsigned i = 0; i < std::size(typeStrOfId); ++i) {
      idIndexMap.insert(TagLib::ByteVector(typeStrOfId[i].str, 4), i);
    }
  }
  if (idIndexMap.contains(id)) {
    const auto& [s, t, supported] = typeStrOfId[idIndexMap[id]];
    type = t;
    str = s;
    if (type == Frame::FT_Other) {
      type = Frame::getTypeFromCustomFrameName(
            QByteArray(id.data(), id.size()));
    }
  } else {
    type = Frame::FT_UnknownFrame;
    str = "????";
  }
}

/**
 * Get string description starting with 4 bytes ID.
 *
 * @param type type of frame
 *
 * @return string.
 */
const char* getStringForType(Frame::Type type)
{
  if (type != Frame::FT_Other) {
    for (const auto& [s, t, supported] : typeStrOfId) {
      if (t == type) {
        return s;
      }
    }
  }
  return "????";
}

/**
 * Get the fields from a text identification frame.
 *
 * @param tFrame text identification frame
 * @param fields the fields are appended to this list
 * @param type   frame type
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromTextFrame(
  const TagLib::ID3v2::TextIdentificationFrame* tFrame,
  Frame::FieldList& fields, Frame::Type type)
{
  QString text;
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = tFrame->textEncoding();
  fields.push_back(field);

  if (auto txxxFrame =
      dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(tFrame)) {
    field.m_id = Frame::ID_Description;
    field.m_value = toQString(txxxFrame->description());
    fields.push_back(field);

    TagLib::StringList slText = tFrame->fieldList();
    text = slText.size() > 1 ? toQString(slText[1]) : QLatin1String("");
  } else {
    // if there are multiple items, put them into one string
    // separated by a special separator.
    text = joinToQString(tFrame->fieldList());
  }
  field.m_id = Frame::ID_Text;
  if (type == Frame::FT_Genre) {
    text = Genres::getNameString(text);
  }
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an attached picture frame.
 *
 * @param apicFrame attached picture frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromApicFrame(
  const TagLib::ID3v2::AttachedPictureFrame* apicFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = apicFrame->textEncoding();
  fields.push_back(field);

  // for compatibility with ID3v2.3 id3lib
  field.m_id = Frame::ID_ImageFormat;
  field.m_value = QString(QLatin1String(""));
  fields.push_back(field);

  field.m_id = Frame::ID_MimeType;
  field.m_value = toQString(apicFrame->mimeType());
  fields.push_back(field);

  field.m_id = Frame::ID_PictureType;
  field.m_value = apicFrame->type();
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  QString text = toQString(apicFrame->description());
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  TagLib::ByteVector pic = apicFrame->picture();
  QByteArray ba = QByteArray(pic.data(), pic.size());
  field.m_value = ba;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a comments frame.
 *
 * @param commFrame comments frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromCommFrame(
  const TagLib::ID3v2::CommentsFrame* commFrame, Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = commFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Language;
  TagLib::ByteVector bvLang = commFrame->language();
  field.m_value = QString::fromLatin1(QByteArray(bvLang.data(), bvLang.size()));
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  field.m_value = toQString(commFrame->description());
  fields.push_back(field);

  field.m_id = Frame::ID_Text;
  QString text = toQString(commFrame->toString());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a unique file identifier frame.
 *
 * @param ufidFrame unique file identifier frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromUfidFrame(
  const TagLib::ID3v2::UniqueFileIdentifierFrame* ufidFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Owner;
  field.m_value = toQString(ufidFrame->owner());
  fields.push_back(field);

  field.m_id = Frame::ID_Id;
  TagLib::ByteVector id = ufidFrame->identifier();
  auto ba = QByteArray(id.data(), id.size());
  field.m_value = ba;
  fields.push_back(field);

  if (!ba.isEmpty()) {
    if (QString text(QString::fromLatin1(ba));
        ba.size() - text.length() <= 1 &&
        AttributeData::isHexString(text, 'Z', QLatin1String("-"))) {
      return text;
    }
  }
  return QString();
}

/**
 * Get the fields from a general encapsulated object frame.
 *
 * @param geobFrame general encapsulated object frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromGeobFrame(
  const TagLib::ID3v2::GeneralEncapsulatedObjectFrame* geobFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = geobFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_MimeType;
  field.m_value = toQString(geobFrame->mimeType());
  fields.push_back(field);

  field.m_id = Frame::ID_Filename;
  field.m_value = toQString(geobFrame->fileName());
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  QString text = toQString(geobFrame->description());
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  TagLib::ByteVector obj = geobFrame->object();
  QByteArray ba = QByteArray(obj.data(), obj.size());
  field.m_value = ba;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a URL link frame.
 *
 * @param wFrame URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromUrlFrame(
  const TagLib::ID3v2::UrlLinkFrame* wFrame, Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Url;
  QString text = toQString(wFrame->url());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a user URL link frame.
 *
 * @param wxxxFrame user URL link frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromUserUrlFrame(
  const TagLib::ID3v2::UserUrlLinkFrame* wxxxFrame, Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = wxxxFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  field.m_value = toQString(wxxxFrame->description());
  fields.push_back(field);

  field.m_id = Frame::ID_Url;
  QString text = toQString(wxxxFrame->url());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an unsynchronized lyrics frame.
 * This is copy-pasted from editCommFrame().
 *
 * @param usltFrame unsynchronized frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromUsltFrame(
  const TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = usltFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Language;
  TagLib::ByteVector bvLang = usltFrame->language();
  field.m_value = QString::fromLatin1(QByteArray(bvLang.data(), bvLang.size()));
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  field.m_value = toQString(usltFrame->description());
  fields.push_back(field);

  field.m_id = Frame::ID_Text;
  QString text = toQString(usltFrame->toString());
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from a synchronized lyrics frame.
 *
 * @param syltFrame synchronized lyrics frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromSyltFrame(
  const TagLib::ID3v2::SynchronizedLyricsFrame* syltFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = syltFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Language;
  TagLib::ByteVector bvLang = syltFrame->language();
  field.m_value = QString::fromLatin1(QByteArray(bvLang.data(), bvLang.size()));
  fields.push_back(field);

  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = syltFrame->timestampFormat();
  fields.push_back(field);

  field.m_id = Frame::ID_ContentType;
  field.m_value = syltFrame->type();
  fields.push_back(field);

  field.m_id = Frame::ID_Description;
  QString text = toQString(syltFrame->description());
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList synchedData;
  const TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList stl =
      syltFrame->synchedText();
  for (auto it = stl.begin(); it != stl.end(); ++it) {
    synchedData.append(static_cast<quint32>(it->time));
    synchedData.append(toQString(it->text));
  }
  field.m_value = synchedData;
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an event timing codes frame.
 *
 * @param etcoFrame event timing codes frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromEtcoFrame(
  const TagLib::ID3v2::EventTimingCodesFrame* etcoFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TimestampFormat;
  field.m_value = etcoFrame->timestampFormat();
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList synchedData;
  const TagLib::ID3v2::EventTimingCodesFrame::SynchedEventList sel =
      etcoFrame->synchedEvents();
  for (auto it = sel.begin(); it != sel.end(); ++it) {
    synchedData.append(static_cast<quint32>(it->time));
    synchedData.append(static_cast<int>(it->type));
  }
  field.m_value = synchedData;
  fields.push_back(field);

  return QString();
}

/**
 * Get the fields from a private frame.
 *
 * @param privFrame private frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromPrivFrame(
  const TagLib::ID3v2::PrivateFrame* privFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Owner;
  QString owner = toQString(privFrame->owner());
  field.m_value = owner;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  TagLib::ByteVector data = privFrame->data();
  auto ba = QByteArray(data.data(), data.size());
  field.m_value = ba;
  fields.push_back(field);

  if (!owner.isEmpty() && !ba.isEmpty()) {
    if (QString str; AttributeData(owner).toString(ba, str)) {
      return str;
    }
  }
  return QString();
}

/**
 * Get the fields from a popularimeter frame.
 *
 * @param popmFrame popularimeter frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromPopmFrame(
  const TagLib::ID3v2::PopularimeterFrame* popmFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Email;
  field.m_value = toQString(popmFrame->email());
  fields.push_back(field);

  field.m_id = Frame::ID_Rating;
  field.m_value = popmFrame->rating();
  QString text(field.m_value.toString());
  fields.push_back(field);

  field.m_id = Frame::ID_Counter;
  field.m_value = popmFrame->counter();
  fields.push_back(field);

  return text;
}

/**
 * Get the fields from an ownership frame.
 *
 * @param owneFrame ownership frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromOwneFrame(
  const TagLib::ID3v2::OwnershipFrame* owneFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_TextEnc;
  field.m_value = owneFrame->textEncoding();
  fields.push_back(field);

  field.m_id = Frame::ID_Date;
  field.m_value = toQString(owneFrame->datePurchased());
  fields.push_back(field);

  field.m_id = Frame::ID_Price;
  field.m_value = toQString(owneFrame->pricePaid());
  fields.push_back(field);

  field.m_id = Frame::ID_Seller;
  QString text(toQString(owneFrame->seller()));
  field.m_value = text;
  fields.push_back(field);

  return text;
}

/**
 * Get a string representation of the data in an RVA2 frame.
 * @param rva2Frame RVA2 frame
 * @return string containing lines with space separated values for
 * type of channel, volume adjustment, bits representing peak,
 * peak volume. The peak volume is a hex byte array, the other values
 * are integers, the volume adjustment is signed. Bits representing peak
 * and peak volume are omitted if they have zero bits.
 */
QString rva2FrameToString(
    const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame)
{
  QString text;
  const TagLib::List<TagLib::ID3v2::RelativeVolumeFrame::ChannelType> channels =
      rva2Frame->channels();
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    TagLib::ID3v2::RelativeVolumeFrame::ChannelType type = *it;
    if (!text.isEmpty()) {
      text += QLatin1Char('\n');
    }
    short adj = rva2Frame->volumeAdjustmentIndex(type);
    auto [bitsRepresentingPeak, peakVolume] = rva2Frame->peakVolume(type);
    text += QString::number(type);
    text += QLatin1Char(' ');
    text += QString::number(adj);
    if (bitsRepresentingPeak > 0) {
      text += QLatin1Char(' ');
      text += QString::number(bitsRepresentingPeak);
      text += QLatin1Char(' ');
      text += QString::fromLatin1(
            QByteArray(peakVolume.data(), peakVolume.size()).toHex());
    }
  }
  return text;
}

/**
 * Set the data in an RVA2 frame from a string representation.
 * @param rva2Frame RVA2 frame to set
 * @param text string representation
 * @see rva2FrameToString()
 */
void rva2FrameFromString(TagLib::ID3v2::RelativeVolumeFrame* rva2Frame,
                         const TagLib::String& text)
{
  // Unfortunately, it is not possible to remove data for a specific channel.
  // Only the whole frame could be deleted and a new one created.
  const auto lines = toQString(text).split(QLatin1Char('\n'));
  for (const QString& line : lines) {
    if (QStringList strs = line.split(QLatin1Char(' ')); strs.size() > 1) {
      bool ok;
      if (int typeInt = strs.at(0).toInt(&ok);
          ok && typeInt >= 0 && typeInt <= 8) {
        short adj = strs.at(1).toShort(&ok);
        if (ok) {
          auto type =
              static_cast<TagLib::ID3v2::RelativeVolumeFrame::ChannelType>(
                typeInt);
          rva2Frame->setVolumeAdjustmentIndex(adj, type);
          if (strs.size() > 3) {
            int bitsInt = strs.at(2).toInt(&ok);
            if (QByteArray ba = QByteArray::fromHex(strs.at(3).toLatin1());
                ok && bitsInt > 0 && bitsInt <= 255 &&
                bitsInt <= ba.size() * 8) {
              TagLib::ID3v2::RelativeVolumeFrame::PeakVolume peak;
              peak.bitsRepresentingPeak = bitsInt;
              peak.peakVolume.setData(ba.constData(), ba.size());
              rva2Frame->setPeakVolume(peak, type);
            }
          }
        }
      }
    }
  }
}

/**
 * Get the fields from a relative volume frame.
 *
 * @param rva2Frame relative volume frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromRva2Frame(
  const TagLib::ID3v2::RelativeVolumeFrame* rva2Frame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Id;
  field.m_value = toQString(rva2Frame->identification());
  fields.push_back(field);

  QString text = rva2FrameToString(rva2Frame);
  field.m_id = Frame::ID_Text;
  field.m_value = text;
  fields.push_back(field);
  return text;
}

#if TAGLIB_VERSION >= 0x010a00
Frame createFrameFromId3Frame(const TagLib::ID3v2::Frame* id3Frame, int index);

/**
 * Get the fields from a chapter frame.
 *
 * @param chapFrame chapter frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromChapFrame(
  const TagLib::ID3v2::ChapterFrame* chapFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Id;
  QString text = toQString(
        TagLib::String(chapFrame->elementID(), TagLib::String::Latin1));
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList data;
  data.append(chapFrame->startTime());
  data.append(chapFrame->endTime());
  data.append(chapFrame->startOffset());
  data.append(chapFrame->endOffset());
  field.m_value = data;
  fields.push_back(field);

  field.m_id = Frame::ID_Subframe;
  const TagLib::ID3v2::FrameList& frameList = chapFrame->embeddedFrameList();
  for (auto it = frameList.begin();
       it != frameList.end();
       ++it) {
    Frame frame(createFrameFromId3Frame(*it, -1));
    field.m_value = frame.getExtendedType().getName();
    fields.push_back(field);
    fields.append(frame.getFieldList());
  }

  return text;
}

/**
 * Get the fields from a table of contents frame.
 *
 * @param ctocFrame table of contents frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromCtocFrame(
  const TagLib::ID3v2::TableOfContentsFrame* ctocFrame,
  Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Id;
  QString text = toQString(
        TagLib::String(ctocFrame->elementID(), TagLib::String::Latin1));
  field.m_value = text;
  fields.push_back(field);

  field.m_id = Frame::ID_Data;
  QVariantList data;
  data.append(ctocFrame->isTopLevel());
  data.append(ctocFrame->isOrdered());
  QStringList elements;
  const TagLib::ByteVectorList childElements = ctocFrame->childElements();
  for (auto it = childElements.begin(); it != childElements.end(); ++it) {
    elements.append(toQString(TagLib::String(*it, TagLib::String::Latin1)));
  }
  data.append(elements);
  field.m_value = data;
  fields.push_back(field);

  field.m_id = Frame::ID_Subframe;
  const TagLib::ID3v2::FrameList& frameList = ctocFrame->embeddedFrameList();
  for (auto it = frameList.begin();
       it != frameList.end();
       ++it) {
    Frame frame(createFrameFromId3Frame(*it, -1));
    field.m_value = frame.getExtendedType().getName();
    fields.push_back(field);
    fields.append(frame.getFieldList());
  }

  return text;
}
#endif

/**
 * Get the fields from an unknown frame.
 *
 * @param unknownFrame unknown frame
 * @param fields the fields are appended to this list
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromUnknownFrame(
  const TagLib::ID3v2::Frame* unknownFrame, Frame::FieldList& fields)
{
  Frame::Field field;
  field.m_id = Frame::ID_Data;
  TagLib::ByteVector dat = unknownFrame->render();
  auto ba = QByteArray(dat.data(), dat.size());
  field.m_value = ba;
  fields.push_back(field);
  return QString();
}

/**
 * Get the fields from an ID3v2 tag.
 *
 * @param frame  frame
 * @param fields the fields are appended to this list
 * @param type   frame type
 *
 * @return text representation of fields (Text or URL).
 */
QString getFieldsFromId3Frame(const TagLib::ID3v2::Frame* frame,
                              Frame::FieldList& fields, Frame::Type type)
{
  if (frame) {
    if (auto tFrame =
          dynamic_cast<const TagLib::ID3v2::TextIdentificationFrame*>(frame)) {
      return getFieldsFromTextFrame(tFrame, fields, type);
    }
    if (auto apicFrame =
          dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(frame)) {
      return getFieldsFromApicFrame(apicFrame, fields);
    }
    if (auto commFrame =
         dynamic_cast<const TagLib::ID3v2::CommentsFrame*>(frame)) {
      return getFieldsFromCommFrame(commFrame, fields);
    }
    if (auto ufidFrame =
         dynamic_cast<const TagLib::ID3v2::UniqueFileIdentifierFrame*>(frame)) {
      return getFieldsFromUfidFrame(ufidFrame, fields);
    }
    if (auto geobFrame =
        dynamic_cast<const TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(frame)) {
      return getFieldsFromGeobFrame(geobFrame, fields);
    }
    if (auto wxxxFrame =
         dynamic_cast<const TagLib::ID3v2::UserUrlLinkFrame*>(frame)) {
      return getFieldsFromUserUrlFrame(wxxxFrame, fields);
    }
    if (auto wFrame = dynamic_cast<const TagLib::ID3v2::UrlLinkFrame*>(frame)) {
      return getFieldsFromUrlFrame(wFrame, fields);
    }
    if (auto usltFrame =
         dynamic_cast<const TagLib::ID3v2::UnsynchronizedLyricsFrame*>(frame)) {
      return getFieldsFromUsltFrame(usltFrame, fields);
    }
    if (auto syltFrame =
         dynamic_cast<const TagLib::ID3v2::SynchronizedLyricsFrame*>(frame)) {
      return getFieldsFromSyltFrame(syltFrame, fields);
    }
    if (auto etcoFrame =
         dynamic_cast<const TagLib::ID3v2::EventTimingCodesFrame*>(frame)) {
      return getFieldsFromEtcoFrame(etcoFrame, fields);
    }
    if (auto privFrame =
         dynamic_cast<const TagLib::ID3v2::PrivateFrame*>(frame)) {
      return getFieldsFromPrivFrame(privFrame, fields);
    }
    if (auto popmFrame =
         dynamic_cast<const TagLib::ID3v2::PopularimeterFrame*>(frame)) {
      return getFieldsFromPopmFrame(popmFrame, fields);
    }
    if (auto owneFrame =
         dynamic_cast<const TagLib::ID3v2::OwnershipFrame*>(frame)) {
      return getFieldsFromOwneFrame(owneFrame, fields);
    }
    if (auto rva2Frame =
         dynamic_cast<const TagLib::ID3v2::RelativeVolumeFrame*>(frame)) {
      return getFieldsFromRva2Frame(rva2Frame, fields);
    }
#if TAGLIB_VERSION >= 0x010a00
    if (auto chapFrame =
         dynamic_cast<const TagLib::ID3v2::ChapterFrame*>(frame)) {
      return getFieldsFromChapFrame(chapFrame, fields);
    }
    if (auto ctocFrame =
         dynamic_cast<const TagLib::ID3v2::TableOfContentsFrame*>(frame)) {
      return getFieldsFromCtocFrame(ctocFrame, fields);
    }
#endif
    TagLib::ByteVector id = frame->frameID();
#if TAGLIB_VERSION < 0x010a00
    if (id.startsWith("SYLT")) {
      TagLib::ID3v2::SynchronizedLyricsFrame syltFrm(frame->render());
      return getFieldsFromSyltFrame(&syltFrm, fields);
    }
    if (id.startsWith("ETCO")) {
      TagLib::ID3v2::EventTimingCodesFrame etcoFrm(frame->render());
      return getFieldsFromEtcoFrame(&etcoFrm, fields);
    }
#endif
    return getFieldsFromUnknownFrame(frame, fields);
  }
  return QString();
}

/**
 * Convert a string to a language code byte vector.
 *
 * @param str string containing language code.
 *
 * @return 3 byte vector with language code.
 */
TagLib::ByteVector languageCodeByteVector(QString str)
{
  if (uint len = str.length(); len > 3) {
    str.truncate(3);
  } else if (len < 3) {
    for (uint i = len; i < 3; ++i) {
      str += QLatin1Char(' ');
    }
  }
  return TagLib::ByteVector(str.toLatin1().data(), str.length());
}

/**
 * The following template functions are used to uniformly set fields
 * in the different ID3v2 frames.
 */
//! @cond
template <class T>
void setTextEncoding(T*, TagLib::String::Type) {}

template <>
void setTextEncoding(TagLib::ID3v2::TextIdentificationFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UserTextIdentificationFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::AttachedPictureFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::CommentsFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UserUrlLinkFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setTextEncoding(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}


template <class T>
void setDescription(T*, const Frame::Field&) {}

template <>
void setDescription(TagLib::ID3v2::UserTextIdentificationFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::AttachedPictureFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <>
void setDescription(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                    const Frame::Field& fld)
{
  f->setDescription(toTString(fld.m_value.toString()));
}

template <class T>
void setMimeType(T*, const Frame::Field&) {}

template <>
void setMimeType(TagLib::ID3v2::AttachedPictureFrame* f,
                 const Frame::Field& fld)
{
  f->setMimeType(toTString(fld.m_value.toString()));
}

template <>
void setMimeType(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                 const Frame::Field& fld)
{
  f->setMimeType(toTString(fld.m_value.toString()));
}

template <class T>
void setPictureType(T*, const Frame::Field&) {}

template <>
void setPictureType(TagLib::ID3v2::AttachedPictureFrame* f,
                    const Frame::Field& fld)
{
  f->setType(
    static_cast<TagLib::ID3v2::AttachedPictureFrame::Type>(
      fld.m_value.toInt()));
}

template <class T>
void setData(T*, const Frame::Field&) {}

template <>
void setData(TagLib::ID3v2::Frame* f, const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setData(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::AttachedPictureFrame* f, const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setPicture(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
             const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setObject(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
             const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setIdentifier(TagLib::ByteVector(ba.data(), ba.size()));
}

template <>
void setData(TagLib::ID3v2::SynchronizedLyricsFrame* f,
             const Frame::Field& fld)
{
  TagLib::ID3v2::SynchronizedLyricsFrame::SynchedTextList stl;
  QVariantList synchedData(fld.m_value.toList());
  QListIterator it(synchedData);
  while (it.hasNext()) {
    quint32 time = it.next().toUInt();
    if (!it.hasNext())
      break;

    TagLib::String text = toTString(it.next().toString());
    stl.append(TagLib::ID3v2::SynchronizedLyricsFrame::SynchedText(time, text));
  }
  f->setSynchedText(stl);
}

template <>
void setData(TagLib::ID3v2::EventTimingCodesFrame* f,
             const Frame::Field& fld)
{
  TagLib::ID3v2::EventTimingCodesFrame::SynchedEventList sel;
  QVariantList synchedData(fld.m_value.toList());
  QListIterator it(synchedData);
  while (it.hasNext()) {
    quint32 time = it.next().toUInt();
    if (!it.hasNext())
      break;

    auto type =
        static_cast<TagLib::ID3v2::EventTimingCodesFrame::EventType>(
          it.next().toInt());
    sel.append(TagLib::ID3v2::EventTimingCodesFrame::SynchedEvent(time, type));
  }
  f->setSynchedEvents(sel);
}

template <class T>
void setLanguage(T*, const Frame::Field&) {}

template <>
void setLanguage(TagLib::ID3v2::CommentsFrame* f, const Frame::Field& fld)
{
  f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <>
void setLanguage(TagLib::ID3v2::UnsynchronizedLyricsFrame* f,
                 const Frame::Field& fld)
{
  f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <>
void setLanguage(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                 const Frame::Field& fld)
{
  f->setLanguage(languageCodeByteVector(fld.m_value.toString()));
}

template <class T>
void setOwner(T*, const Frame::Field&) {}

template <>
void setOwner(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
              const Frame::Field& fld)
{
  f->setOwner(toTString(fld.m_value.toString()));
}

template <>
void setOwner(TagLib::ID3v2::PrivateFrame* f,
              const Frame::Field& fld)
{
  f->setOwner(toTString(fld.m_value.toString()));
}

template <>
void setData(TagLib::ID3v2::PrivateFrame* f,
             const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setData(TagLib::ByteVector(ba.data(), ba.size()));
}

template <class T>
void setIdentifier(T*, const Frame::Field&) {}

template <>
void setIdentifier(TagLib::ID3v2::UniqueFileIdentifierFrame* f,
                   const Frame::Field& fld)
{
  QByteArray ba(fld.m_value.toByteArray());
  f->setIdentifier(TagLib::ByteVector(ba.data(), ba.size()));
}

template <class T>
void setFilename(T*, const Frame::Field&) {}

template <>
void setFilename(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
                 const Frame::Field& fld)
{
  f->setFileName(toTString(fld.m_value.toString()));
}

template <class T>
void setUrl(T*, const Frame::Field&) {}

template <>
void setUrl(TagLib::ID3v2::UrlLinkFrame* f, const Frame::Field& fld)
{
  f->setUrl(toTString(fld.m_value.toString()));
}

template <>
void setUrl(TagLib::ID3v2::UserUrlLinkFrame* f, const Frame::Field& fld)
{
  f->setUrl(toTString(fld.m_value.toString()));
}

template <class T>
void setValue(T* f, const TagLib::String& text)
{
  f->setText(text);
}

template <>
void setValue(TagLib::ID3v2::AttachedPictureFrame* f, const TagLib::String& text)
{
  f->setDescription(text);
}

template <>
void setValue(TagLib::ID3v2::GeneralEncapsulatedObjectFrame* f,
              const TagLib::String& text)
{
  f->setDescription(text);
}

void setStringOrList(TagLib::ID3v2::TextIdentificationFrame* f,
                     const TagLib::String& text)
{
  if (text.find(Frame::stringListSeparator().toLatin1()) == -1) {
    f->setText(text);
  } else {
    f->setText(splitToTStringList(toQString(text)));
  }
}

template <>
void setValue(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
  setStringOrList(f, text);
}

template <>
void setValue(TagLib::ID3v2::UniqueFileIdentifierFrame* f, const TagLib::String& text)
{
  if (AttributeData::isHexString(toQString(text), 'Z', QLatin1String("-"))) {
    TagLib::ByteVector data(text.data(TagLib::String::Latin1));
    data.append('\0');
    f->setIdentifier(data);
  }
}

template <>
void setValue(TagLib::ID3v2::SynchronizedLyricsFrame* f, const TagLib::String& text)
{
  f->setDescription(text);
}

template <>
void setValue(TagLib::ID3v2::PrivateFrame* f, const TagLib::String& text)
{
  QByteArray newData;
  if (TagLib::String owner = f->owner(); !owner.isEmpty() &&
      AttributeData(toQString(owner))
      .toByteArray(toQString(text), newData)) {
    f->setData(TagLib::ByteVector(newData.data(), newData.size()));
  }
}

template <>
void setValue(TagLib::ID3v2::PopularimeterFrame* f, const TagLib::String& text)
{
  f->setRating(text.toInt());
}

template <class T>
void setText(T* f, const TagLib::String& text)
{
  f->setText(text);
}

template <>
void setText(TagLib::ID3v2::TextIdentificationFrame* f, const TagLib::String& text)
{
  setStringOrList(f, text);
}

template <class T>
void setEmail(T*, const Frame::Field&) {}

template <>
void setEmail(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
  f->setEmail(toTString(fld.m_value.toString()));
}

template <class T>
void setRating(T*, const Frame::Field&) {}

template <>
void setRating(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
  f->setRating(fld.m_value.toInt());
}

template <class T>
void setCounter(T*, const Frame::Field&) {}

template <>
void setCounter(TagLib::ID3v2::PopularimeterFrame* f, const Frame::Field& fld)
{
  f->setCounter(fld.m_value.toUInt());
}

template <class T>
void setDate(T*, const Frame::Field&) {}

template <>
void setDate(TagLib::ID3v2::OwnershipFrame* f, const Frame::Field& fld)
{
  // The date string must have exactly 8 characters (should be YYYYMMDD)
  QString date(fld.m_value.toString().leftJustified(8, QLatin1Char(' '), true));
  f->setDatePurchased(toTString(date));
}

template <class T>
void setPrice(T*, const Frame::Field&) {}

template <>
void setPrice(TagLib::ID3v2::OwnershipFrame* f, const Frame::Field& fld)
{
  f->setPricePaid(toTString(fld.m_value.toString()));
}

template <class T>
void setSeller(T*, const Frame::Field&) {}

template <>
void setSeller(TagLib::ID3v2::OwnershipFrame* f, const Frame::Field& fld)
{
  f->setSeller(toTString(fld.m_value.toString()));
}

template <>
void setTextEncoding(TagLib::ID3v2::OwnershipFrame* f,
                     TagLib::String::Type enc)
{
  f->setTextEncoding(enc);
}

template <>
void setValue(TagLib::ID3v2::OwnershipFrame* f, const TagLib::String& text)
{
  f->setSeller(text);
}

template <class T>
void setTimestampFormat(T*, const Frame::Field&) {}

template <>
void setTimestampFormat(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                        const Frame::Field& fld)
{
  f->setTimestampFormat(
        static_cast<TagLib::ID3v2::SynchronizedLyricsFrame::TimestampFormat>(
          fld.m_value.toInt()));
}

template <>
void setTimestampFormat(TagLib::ID3v2::EventTimingCodesFrame* f,
                        const Frame::Field& fld)
{
  f->setTimestampFormat(
        static_cast<TagLib::ID3v2::EventTimingCodesFrame::TimestampFormat>(
          fld.m_value.toInt()));
}

template <class T>
void setContentType(T*, const Frame::Field&) {}

template <>
void setContentType(TagLib::ID3v2::SynchronizedLyricsFrame* f,
                    const Frame::Field& fld)
{
  f->setType(static_cast<TagLib::ID3v2::SynchronizedLyricsFrame::Type>(
               fld.m_value.toInt()));
}

template <>
void setIdentifier(TagLib::ID3v2::RelativeVolumeFrame* f,
                   const Frame::Field& fld)
{
  f->setIdentification(toTString(fld.m_value.toString()));
}

template <>
void setText(TagLib::ID3v2::RelativeVolumeFrame* f, const TagLib::String& text)
{
  rva2FrameFromString(f, text);
}

template <>
void setValue(TagLib::ID3v2::RelativeVolumeFrame* f, const TagLib::String& text)
{
  rva2FrameFromString(f, text);
}

#if TAGLIB_VERSION >= 0x010a00
TagLib::ID3v2::Frame* createId3FrameFromFrame(const TagLibFile* self,
                                              Frame& frame);

template <>
void setIdentifier(TagLib::ID3v2::ChapterFrame* f,
                   const Frame::Field& fld)
{
  QByteArray id = fld.m_value.toString().toLatin1();
  f->setElementID(TagLib::ByteVector(id.constData(), id.size()));
}

template <>
void setIdentifier(TagLib::ID3v2::TableOfContentsFrame* f,
                   const Frame::Field& fld)
{
  QByteArray id = fld.m_value.toString().toLatin1();
  f->setElementID(TagLib::ByteVector(id.constData(), id.size()));
}

template <>
void setValue(TagLib::ID3v2::ChapterFrame* f, const TagLib::String& text)
{
  f->setElementID(text.data(TagLib::String::Latin1));
}

template <>
void setValue(TagLib::ID3v2::TableOfContentsFrame* f, const TagLib::String& text)
{
  f->setElementID(text.data(TagLib::String::Latin1));
}

template <>
void setData(TagLib::ID3v2::ChapterFrame* f,
             const Frame::Field& fld)
{
  if (QVariantList data(fld.m_value.toList()); data.size() == 4) {
    f->setStartTime(data.at(0).toUInt());
    f->setEndTime(data.at(1).toUInt());
    f->setStartOffset(data.at(2).toUInt());
    f->setEndOffset(data.at(3).toUInt());
  }
  // The embedded frames are deleted here because frames without subframes
  // do not have an ID_Subframe field and setSubframes() is not called.
  while (!f->embeddedFrameList().isEmpty()) {
    f->removeEmbeddedFrame(f->embeddedFrameList()[0]);
  }
  // f->removeEmbeddedFrame() calls erase() thereby invalidating an iterator
  // on f->embeddedFrameList(). The uncommented code below will therefore crash.
  // const TagLib::ID3v2::FrameList l = f->embeddedFrameList();
  // for (auto it = l.begin(); it != l.end(); ++it) {
  //   f->removeEmbeddedFrame(*it, true);
  // }
}

template <>
void setData(TagLib::ID3v2::TableOfContentsFrame* f,
             const Frame::Field& fld)
{
  if (QVariantList data(fld.m_value.toList()); data.size() >= 3) {
    f->setIsTopLevel(data.at(0).toBool());
    f->setIsOrdered(data.at(1).toBool());
    QStringList elementStrings = data.at(2).toStringList();
    TagLib::ByteVectorList elements;
    for (auto it = elementStrings.constBegin();
         it != elementStrings.constEnd();
         ++it) {
      QByteArray id = it->toLatin1();
      elements.append(TagLib::ByteVector(id.constData(), id.size()));
    }
    f->setChildElements(elements);
  }
  // The embedded frames are deleted here because frames without subframes
  // do not have an ID_Subframe field and setSubframes() is not called.
  while (!f->embeddedFrameList().isEmpty()) {
    f->removeEmbeddedFrame(f->embeddedFrameList()[0]);
  }
  // f->removeEmbeddedFrame() calls erase() thereby invalidating an iterator
  // on f->embeddedFrameList(). The uncommented code below will therefore crash.
  // const TagLib::ID3v2::FrameList l = f->embeddedFrameList();
  // for (auto it = l.begin(); it != l.end(); ++it) {
  //   f->removeEmbeddedFrame(*it, true);
  // }
}

template <class T>
void setSubframes(const TagLibFile*, T*, Frame::FieldList::const_iterator, // clazy:exclude=function-args-by-ref
                  Frame::FieldList::const_iterator) {}

template <>
void setSubframes(const TagLibFile* self, TagLib::ID3v2::ChapterFrame* f,
                  Frame::FieldList::const_iterator begin, // clazy:exclude=function-args-by-ref
                  Frame::FieldList::const_iterator end) // clazy:exclude=function-args-by-ref
{
  FrameCollection frames = FrameCollection::fromSubframes(begin, end);
  for (auto it = frames.begin(); it != frames.end(); ++it) {
    f->addEmbeddedFrame(createId3FrameFromFrame(self, const_cast<Frame&>(*it)));
  }
}

template <>
void setSubframes(const TagLibFile* self, TagLib::ID3v2::TableOfContentsFrame* f,
                  Frame::FieldList::const_iterator begin, // clazy:exclude=function-args-by-ref
                  Frame::FieldList::const_iterator end) // clazy:exclude=function-args-by-ref
{
  FrameCollection frames = FrameCollection::fromSubframes(begin, end);
  for (auto it = frames.begin(); it != frames.end(); ++it) {
    f->addEmbeddedFrame(createId3FrameFromFrame(self, const_cast<Frame&>(*it)));
  }
}
#endif

//! @endcond

/**
 * Set the fields in a TagLib ID3v2 frame.
 *
 * @param self   this TagLibFile instance
 * @param tFrame TagLib frame to set
 * @param frame  frame with field values
 */
template <class T>
void setTagLibFrame(const TagLibFile* self, T* tFrame, const Frame& frame)
{
  // If value is changed or field list is empty,
  // set from value, else from FieldList.
  if (const Frame::FieldList& fieldList = frame.getFieldList();
      frame.isValueChanged() || fieldList.empty()) {
    QString text(frame.getValue());
    TagLibMpegSupportInternal::fixUpTagLibFrameValue(
      self, frame.getType(), text);
    setValue(tFrame, toTString(text));
    setTextEncoding(tFrame, getTextEncodingConfig(needsUnicode(text)));
  } else {
    for (auto fldIt = fieldList.constBegin(); fldIt != fieldList.constEnd(); ++fldIt) {
      switch (const Frame::Field& fld = *fldIt; fld.m_id) {
      case Frame::ID_Text:
      {
        QString value(fld.m_value.toString());
        TagLibMpegSupportInternal::fixUpTagLibFrameValue(
          self, frame.getType(), value);
        setText(tFrame, toTString(value));
        break;
      }
      case Frame::ID_TextEnc:
        setTextEncoding(tFrame, static_cast<TagLib::String::Type>(
                          fld.m_value.toInt()));
        break;
      case Frame::ID_Description:
        setDescription(tFrame, fld);
        break;
      case Frame::ID_MimeType:
        setMimeType(tFrame, fld);
        break;
      case Frame::ID_PictureType:
        setPictureType(tFrame, fld);
        break;
      case Frame::ID_Data:
        setData(tFrame, fld);
        break;
      case Frame::ID_Language:
        setLanguage(tFrame, fld);
        break;
      case Frame::ID_Owner:
        setOwner(tFrame, fld);
        break;
      case Frame::ID_Id:
        setIdentifier(tFrame, fld);
        break;
      case Frame::ID_Filename:
        setFilename(tFrame, fld);
        break;
      case Frame::ID_Url:
        setUrl(tFrame, fld);
        break;
      case Frame::ID_Email:
        setEmail(tFrame, fld);
        break;
      case Frame::ID_Rating:
        setRating(tFrame, fld);
        break;
      case Frame::ID_Counter:
        setCounter(tFrame, fld);
        break;
      case Frame::ID_Price:
        setPrice(tFrame, fld);
        break;
      case Frame::ID_Date:
        setDate(tFrame, fld);
        break;
      case Frame::ID_Seller:
        setSeller(tFrame, fld);
        break;
      case Frame::ID_TimestampFormat:
        setTimestampFormat(tFrame, fld);
        break;
      case Frame::ID_ContentType:
        setContentType(tFrame, fld);
        break;
#if TAGLIB_VERSION >= 0x010a00
      case Frame::ID_Subframe:
        setSubframes(self, tFrame, fldIt, fieldList.end());
        return;
#endif
      default: ;
      }
    }
  }
}

/**
 * Modify an ID3v2 frame.
 *
 * @param self     this TagLibFile instance
 * @param id3Frame original ID3v2 frame
 * @param frame    frame with fields to set in new frame
 */
void setId3v2Frame(const TagLibFile* self,
  TagLib::ID3v2::Frame* id3Frame, const Frame& frame)
{
  if (id3Frame) {
    if (auto tFrame =
         dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(id3Frame)) {
      if (auto txxxFrame =
          dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>(id3Frame)) {
        setTagLibFrame(self, txxxFrame, frame);
      } else {
        setTagLibFrame(self, tFrame, frame);
      }
    } else if (auto apicFrame =
                dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)) {
      setTagLibFrame(self, apicFrame, frame);
    } else if (auto commFrame =
                dynamic_cast<TagLib::ID3v2::CommentsFrame*>(id3Frame)) {
      setTagLibFrame(self, commFrame, frame);
    } else if (auto ufidFrame =
            dynamic_cast<TagLib::ID3v2::UniqueFileIdentifierFrame*>(id3Frame)) {
      setTagLibFrame(self, ufidFrame, frame);
    } else if (auto geobFrame =
       dynamic_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(id3Frame)) {
      setTagLibFrame(self, geobFrame, frame);
    } else if (auto wxxxFrame =
                dynamic_cast<TagLib::ID3v2::UserUrlLinkFrame*>(id3Frame)) {
      setTagLibFrame(self, wxxxFrame, frame);
    } else if (auto wFrame =
                dynamic_cast<TagLib::ID3v2::UrlLinkFrame*>(id3Frame)) {
      setTagLibFrame(self, wFrame, frame);
    } else if (auto usltFrame =
            dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(id3Frame)) {
      setTagLibFrame(self, usltFrame, frame);
    } else if (auto syltFrame =
              dynamic_cast<TagLib::ID3v2::SynchronizedLyricsFrame*>(id3Frame)) {
      setTagLibFrame(self, syltFrame, frame);
    } else if (auto etcoFrame =
                dynamic_cast<TagLib::ID3v2::EventTimingCodesFrame*>(id3Frame)) {
      setTagLibFrame(self, etcoFrame, frame);
    } else if (auto privFrame =
                dynamic_cast<TagLib::ID3v2::PrivateFrame*>(id3Frame)) {
      setTagLibFrame(self, privFrame, frame);
    } else if (auto popmFrame =
                dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(id3Frame)) {
      setTagLibFrame(self, popmFrame, frame);
    } else if (auto owneFrame =
                dynamic_cast<TagLib::ID3v2::OwnershipFrame*>(id3Frame)) {
      setTagLibFrame(self, owneFrame, frame);
    } else if (auto rva2Frame =
                dynamic_cast<TagLib::ID3v2::RelativeVolumeFrame*>(id3Frame)) {
      setTagLibFrame(self, rva2Frame, frame);
#if TAGLIB_VERSION >= 0x010a00
    } else if (auto chapFrame =
                dynamic_cast<TagLib::ID3v2::ChapterFrame*>(id3Frame)) {
      setTagLibFrame(self, chapFrame, frame);
    } else if (auto ctocFrame =
                dynamic_cast<TagLib::ID3v2::TableOfContentsFrame*>(id3Frame)) {
      setTagLibFrame(self, ctocFrame, frame);
#endif
    } else {
      TagLib::ByteVector id(id3Frame->frameID());
      // create temporary objects for frames not known by TagLib,
      // an UnknownFrame copy will be created by the edit method.
#if TAGLIB_VERSION < 0x010a00
      if (id.startsWith("SYLT")) {
        TagLib::ID3v2::SynchronizedLyricsFrame syltFrm(id3Frame->render());
        setTagLibFrame(self, &syltFrm, frame);
        id3Frame->setData(syltFrm.render());
      } else if (id.startsWith("ETCO")) {
        TagLib::ID3v2::EventTimingCodesFrame etcoFrm(id3Frame->render());
        setTagLibFrame(self, &etcoFrm, frame);
        id3Frame->setData(etcoFrm.render());
      } else
#endif
      {
        setTagLibFrame(self, id3Frame, frame);
      }
    }
  }
}

/**
 * Check if an ID3v2.4.0 frame ID is valid.
 *
 * @param frameId frame ID (4 characters)
 *
 * @return true if frame ID is valid.
 */
bool isFrameIdValid(const QString& frameId)
{
  Frame::Type type;
  const char* str;
  getTypeStringForFrameId(TagLib::ByteVector(frameId.toLatin1().data(), 4), type, str);
  return type != Frame::FT_UnknownFrame;
}

/**
 * Create a TagLib ID3 frame from a frame.
 * @param self this TagLibFile instance
 * @param frame frame
 * @return TagLib ID3 frame, 0 if invalid.
 */
TagLib::ID3v2::Frame* createId3FrameFromFrame(const TagLibFile* self,
                                              Frame& frame)
{
  TagLib::String::Type enc = TagLibFile::getDefaultTextEncoding();
  QString name = !Frame::isCustomFrameTypeOrOther(frame.getType())
      ? QString::fromLatin1(getStringForType(frame.getType()))
      : frame.getName();
  QString frameId = name;
  frameId.truncate(4);
  TagLib::ID3v2::Frame* id3Frame = nullptr;

  if (name == QLatin1String("AverageLevel") ||
      name == QLatin1String("PeakValue") ||
      name.startsWith(QLatin1String("WM/"))) {
    frameId = QLatin1String("PRIV");
  } else if (name.startsWith(QLatin1String("iTun"))) {
    frameId = QLatin1String("COMM");
  }

  if (frameId.startsWith(QLatin1String("T"))
#if TAGLIB_VERSION >= 0x010b00
      || frameId == QLatin1String("WFED")
#endif
#if TAGLIB_VERSION >= 0x010c00
      || frameId == QLatin1String("MVIN") || frameId == QLatin1String("MVNM")
      || frameId == QLatin1String("GRP1")
#endif
    ) {
    if (frameId == QLatin1String("TXXX")) {
      id3Frame = new TagLib::ID3v2::UserTextIdentificationFrame(enc);
    } else if (isFrameIdValid(frameId)) {
      id3Frame = new TagLib::ID3v2::TextIdentificationFrame(
        TagLib::ByteVector(frameId.toLatin1().data(), frameId.length()), enc);
      id3Frame->setText(""); // is necessary for createFrame() to work
    }
  } else if (frameId == QLatin1String("COMM")) {
    auto commFrame =
        new TagLib::ID3v2::CommentsFrame(enc);
    id3Frame = commFrame;
    commFrame->setLanguage("eng"); // for compatibility with iTunes
    if (frame.getType() == Frame::FT_Other) {
      commFrame->setDescription(toTString(frame.getName()));
    }
  } else if (frameId == QLatin1String("APIC")) {
    id3Frame = new TagLib::ID3v2::AttachedPictureFrame;
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)->setTextEncoding(enc);
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)->setMimeType(
      "image/jpeg");
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(id3Frame)->setType(
      TagLib::ID3v2::AttachedPictureFrame::FrontCover);
  } else if (frameId == QLatin1String("UFID")) {
    // the bytevector must not be empty
    auto ufidFrame =
        new TagLib::ID3v2::UniqueFileIdentifierFrame(
                  TagLib::String("http://www.id3.org/dummy/ufid.html"),
                  TagLib::ByteVector(" "));
    id3Frame = ufidFrame;
    if (AttributeData::isHexString(frame.getValue(), 'Z', QLatin1String("-"))) {
      QByteArray data = (frame.getValue() + QLatin1Char('\0')).toLatin1();
      ufidFrame->setIdentifier(TagLib::ByteVector(data.constData(),
                                                  data.size()));
    }
  } else if (frameId == QLatin1String("GEOB")) {
    id3Frame = new TagLib::ID3v2::GeneralEncapsulatedObjectFrame;
    static_cast<TagLib::ID3v2::GeneralEncapsulatedObjectFrame*>(id3Frame)->setTextEncoding(enc);
  } else if (frameId.startsWith(QLatin1String("W"))) {
    if (frameId == QLatin1String("WXXX")) {
      id3Frame = new TagLib::ID3v2::UserUrlLinkFrame(enc);
    } else if (isFrameIdValid(frameId)) {
      id3Frame = new TagLib::ID3v2::UrlLinkFrame(
        TagLib::ByteVector(frameId.toLatin1().data(), frameId.length()));
      id3Frame->setText("http://"); // is necessary for createFrame() to work
    }
  } else if (frameId == QLatin1String("USLT")) {
    id3Frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(enc);
    static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(id3Frame)->setLanguage("eng");
  } else if (frameId == QLatin1String("SYLT")) {
    id3Frame = new TagLib::ID3v2::SynchronizedLyricsFrame(enc);
    static_cast<TagLib::ID3v2::SynchronizedLyricsFrame*>(id3Frame)->setLanguage("eng");
  } else if (frameId == QLatin1String("ETCO")) {
    id3Frame = new TagLib::ID3v2::EventTimingCodesFrame;
  } else if (frameId == QLatin1String("POPM")) {
    auto popmFrame = new TagLib::ID3v2::PopularimeterFrame;
    id3Frame = popmFrame;
    popmFrame->setEmail(toTString(TagConfig::instance().defaultPopmEmail()));
  } else if (frameId == QLatin1String("PRIV")) {
    auto privFrame = new TagLib::ID3v2::PrivateFrame;
    id3Frame = privFrame;
    if (!frame.getName().startsWith(QLatin1String("PRIV"))) {
      privFrame->setOwner(toTString(frame.getName()));
      if (QByteArray data;
          AttributeData(frame.getName()).toByteArray(frame.getValue(), data)) {
        privFrame->setData(TagLib::ByteVector(data.constData(), data.size()));
      }
    }
  } else if (frameId == QLatin1String("OWNE")) {
    id3Frame = new TagLib::ID3v2::OwnershipFrame(enc);
  } else if (frameId == QLatin1String("RVA2")) {
    id3Frame = new TagLib::ID3v2::RelativeVolumeFrame;
#if TAGLIB_VERSION >= 0x010b00
  } else if (frameId == QLatin1String("PCST")) {
    id3Frame = new TagLib::ID3v2::PodcastFrame;
#endif
#if TAGLIB_VERSION >= 0x010a00
  } else if (frameId == QLatin1String("CHAP")) {
    // crashes with an empty elementID
    id3Frame = new TagLib::ID3v2::ChapterFrame("chp", 0, 0,
                                               0xffffffff, 0xffffffff);
  } else if (frameId == QLatin1String("CTOC")) {
    // crashes with an empty elementID
    id3Frame = new TagLib::ID3v2::TableOfContentsFrame("toc");
#endif
  }
  if (!id3Frame) {
    auto txxxFrame = new TagLib::ID3v2::UserTextIdentificationFrame(enc);
    TagLib::String description;
    if (frame.getType() == Frame::FT_CatalogNumber) {
      description = "CATALOGNUMBER";
    } else if (frame.getType() == Frame::FT_ReleaseCountry) {
      description = "RELEASECOUNTRY";
    } else if (frame.getType() == Frame::FT_Grouping) {
      description = "GROUPING";
    } else if (frame.getType() == Frame::FT_Subtitle) {
      description = "SUBTITLE";
    } else {
      description = toTString(frame.getName());
      frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                    QLatin1String("TXXX - User defined text information")));
    }
    txxxFrame->setDescription(description);
    id3Frame = txxxFrame;
  } else {
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
  }
  if (id3Frame) {
    if (!frame.fieldList().empty()) {
      frame.setValueFromFieldList();
      setId3v2Frame(self, id3Frame, frame);
    }
  }
  return id3Frame;
}

/**
 * Create a frame from a TagLib ID3 frame.
 * @param id3Frame TagLib ID3 frame
 * @param index -1 if not used
 * @return frame.
 */
Frame createFrameFromId3Frame(const TagLib::ID3v2::Frame* id3Frame, int index)
{
  Frame::Type type;
  const char* name;
  getTypeStringForFrameId(id3Frame->frameID(), type, name);
  Frame frame(type, toQString(id3Frame->toString()), QString::fromLatin1(name), index);
  frame.setValue(getFieldsFromId3Frame(id3Frame, frame.fieldList(), type));
  if (id3Frame->frameID().mid(1, 3) == "XXX" ||
      type == Frame::FT_Comment) {
    if (QVariant fieldValue = frame.getFieldValue(Frame::ID_Description);
        fieldValue.isValid()) {
      if (QString description = fieldValue.toString(); !description.isEmpty()) {
        if (description == QLatin1String("CATALOGNUMBER")) {
          frame.setType(Frame::FT_CatalogNumber);
        } else if (description == QLatin1String("RELEASECOUNTRY")) {
          frame.setType(Frame::FT_ReleaseCountry);
        } else if (description == QLatin1String("GROUPING")) {
          frame.setType(Frame::FT_Grouping);
        } else if (description == QLatin1String("SUBTITLE")) {
          frame.setType(Frame::FT_Subtitle);
        } else {
          if (description.startsWith(QLatin1String("QuodLibet::"))) {
            // remove ExFalso/QuodLibet "namespace"
            description = description.mid(11);
          }
          frame.setExtendedType(Frame::ExtendedType(
            Frame::getTypeFromCustomFrameName(description.toLatin1()),
            frame.getInternalName() + QLatin1Char('\n') + description));
        }
      }
    }
  } else if (id3Frame->frameID().startsWith("PRIV")) {
    if (QVariant fieldValue = frame.getFieldValue(Frame::ID_Owner);
        fieldValue.isValid()) {
      if (QString owner = fieldValue.toString(); !owner.isEmpty()) {
        frame.setExtendedType(Frame::ExtendedType(Frame::FT_Other,
                  frame.getInternalName() + QLatin1Char('\n') + owner));
      }
    }
  }
  return frame;
}

}


TagLib::File* TagLibMpegSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "MP3" || ext == "MP2" || ext == "AAC")
#if TAGLIB_VERSION >= 0x020000
    return new TagLib::MPEG::File(stream);
#else
  return new TagLib::MPEG::File(stream,
    TagLib::ID3v2::FrameFactory::instance());
#endif
  return nullptr;
}

bool TagLibMpegSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (auto mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) {
    QString fileName = f.currentFilePath();
    QString ext(fileName.right(4).toLower());
    f.m_fileExtension =
        ext == QLatin1String(".aac") || ext == QLatin1String(".mp2")
        ? ext : QLatin1String(".mp3");
    f.m_isTagSupported[Frame::Tag_1] = true;
    f.m_isTagSupported[Frame::Tag_3] = true;
    if (!f.m_tag[Frame::Tag_1]) {
      f.m_tag[Frame::Tag_1] = mpegFile->ID3v1Tag();
      f.markTagUnchanged(Frame::Tag_1);
    }
    if (!f.m_tag[Frame::Tag_2]) {
      TagLib::ID3v2::Tag* id3v2Tag = mpegFile->ID3v2Tag();
      f.setId3v2VersionFromTag(id3v2Tag);
      f.m_tag[Frame::Tag_2] = id3v2Tag;
      f.markTagUnchanged(Frame::Tag_2);
#if TAGLIB_VERSION >= 0x010a00
      if (!f.m_extraFrames.isRead()) {
        const auto ctocFrames = id3v2Tag->frameList("CTOC");
        int i = 0;
        for (auto ctocFrame : ctocFrames) {
          if (Frame frame;
              ctocChapToChaptersFrame(
                ctocFrame, id3v2Tag->frameList("CHAP"), frame)) {
            frame.setIndex(Frame::toNegativeIndex(i++));
            f.m_extraFrames.append(frame);
            break;
          }
        }
        f.m_extraFrames.setRead(true);
      }
#endif
    }
    if (!f.m_tag[Frame::Tag_3]) {
      f.m_tag[Frame::Tag_3] = mpegFile->APETag();
      f.markTagUnchanged(Frame::Tag_3);
    }
    return true;
  }
  return false;
}

bool TagLibMpegSupport::writeFile(TagLibFile& f, TagLib::File* file, bool force,
  int id3v2Version, bool& fileChanged) const
{
  if (auto mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) {
    static constexpr int tagTypes[TagLibFile::NUM_TAGS] = {
      TagLib::MPEG::File::ID3v1, TagLib::MPEG::File::ID3v2,
      TagLib::MPEG::File::APE
    };
    int saveMask = 0;
    // We iterate through the tags in reverse order to work around
    // a TagLib bug: When stripping the APE tag after the ID3v1 tag,
    // the ID3v1 tag is not removed.
    FOR_TAGLIB_TAGS_REVERSE(tagNr) {
      if (f.m_tag[tagNr] && (force || f.isTagChanged(tagNr))) {
        if (f.m_tag[tagNr]->isEmpty()) {
          mpegFile->strip(tagTypes[tagNr]);
          fileChanged = true;
          f.m_tag[tagNr] = nullptr;
          f.markTagUnchanged(tagNr);
        } else {
          saveMask |= tagTypes[tagNr];
        }
      }
    }
    if (saveMask != 0) {
      f.setId3v2VersionOrDefault(id3v2Version);
      if (
#if TAGLIB_VERSION >= 0x010c00
          mpegFile->save(
            saveMask, TagLib::File::StripNone,
            f.m_id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3,
            TagLib::File::DoNotDuplicate)
#else
          mpegFile->save(saveMask, false, f.m_id3v2Version, false)
#endif
          ) {
        fileChanged = true;
        FOR_TAGLIB_TAGS(tagNr) {
          if (saveMask & tagTypes[tagNr]) {
            f.markTagUnchanged(tagNr);
          }
        }
      }
    }
    return true;
  }
  return false;
}

bool TagLibMpegSupport::makeTagSettable(TagLibFile& f, TagLib::File* file,
  Frame::TagNumber tagNr) const
{
  if (tagNr == Frame::Tag_1) {
    if (auto mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) {
      f.m_tag[tagNr] = mpegFile->ID3v1Tag(true);
      return true;
    }
  } else if (tagNr == Frame::Tag_2) {
    if (auto mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) {
      f.m_tag[tagNr] = mpegFile->ID3v2Tag(true);
      return true;
    }
  } else if (tagNr == Frame::Tag_3) {
    if (auto mpegFile = dynamic_cast<TagLib::MPEG::File*>(file)) {
      f.m_tag[tagNr] = mpegFile->APETag(true);
      return true;
    }
  }
  return false;
}

bool TagLibMpegSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto mpegProperties =
      dynamic_cast<TagLib::MPEG::Properties*>(audioProperties)) {
#if TAGLIB_VERSION < 0x020000
    if (f.getFilename().right(4).toLower() == QLatin1String(".aac")) {
      f.m_detailInfo.format = QLatin1String("AAC");
      return true;
    }
#endif
    switch (mpegProperties->version()) {
    case TagLib::MPEG::Header::Version1:
      f.m_detailInfo.format = QLatin1String("MPEG 1 ");
      break;
    case TagLib::MPEG::Header::Version2:
      f.m_detailInfo.format = QLatin1String("MPEG 2 ");
      break;
    case TagLib::MPEG::Header::Version2_5:
      f.m_detailInfo.format = QLatin1String("MPEG 2.5 ");
      break;
#if TAGLIB_VERSION >= 0x020000
    case TagLib::MPEG::Header::Version4:
      f.m_detailInfo.format = QLatin1String("MPEG 4 ");
      break;
#endif
    }
    if (int layer = mpegProperties->layer(); layer >= 1 && layer <= 3) {
      f.m_detailInfo.format += QLatin1String("Layer ");
      f.m_detailInfo.format += QString::number(layer);
    }
    switch (mpegProperties->channelMode()) {
    case TagLib::MPEG::Header::Stereo:
      f.m_detailInfo.channelMode = TagLibFile::DetailInfo::CM_Stereo;
      f.m_detailInfo.channels = 2;
      break;
    case TagLib::MPEG::Header::JointStereo:
      f.m_detailInfo.channelMode = TagLibFile::DetailInfo::CM_JointStereo;
      f.m_detailInfo.channels = 2;
      break;
    case TagLib::MPEG::Header::DualChannel:
      f.m_detailInfo.channels = 2;
      break;
    case TagLib::MPEG::Header::SingleChannel:
      f.m_detailInfo.channels = 1;
      break;
    }
#if TAGLIB_VERSION >= 0x020000
    if (mpegProperties->isADTS()) {
      f.m_detailInfo.format += QLatin1String("ADTS");
      f.m_detailInfo.channels = mpegProperties->channels();
    }
#endif
    return true;
  }
  return false;
}

QString TagLibMpegSupport::getTagFormat(
  const TagLib::Tag* tag, TaggedFile::TagType& type) const
{
  if (dynamic_cast<const TagLib::ID3v1::Tag*>(tag) != nullptr) {
    type = TaggedFile::TT_Id3v1;
    return QLatin1String("ID3v1.1");
  }
  if (auto id3v2Tag = dynamic_cast<const TagLib::ID3v2::Tag*>(tag)) {
    type = TaggedFile::TT_Id3v2;
    if (TagLib::ID3v2::Header* header = id3v2Tag->header()) {
      uint majorVersion = header->majorVersion();
      uint revisionNumber = header->revisionNumber();
      return QString(QLatin1String("ID3v2.%1.%2"))
        .arg(majorVersion).arg(revisionNumber);
    }
    return QLatin1String("ID3v2");
  }
  return QString();
}

bool TagLibMpegSupport::setFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
#if TAGLIB_VERSION >= 0x010a00
      if (frame.getType() == Frame::FT_Other &&
          frame.getName() == QLatin1String("Chapters") &&
          !f.m_extraFrames.isEmpty() &&
          f.m_extraFrames.front().getName() == QLatin1String("Chapters")) {
        chaptersFrameToCtocChap(frame, id3v2Tag);
        f.m_extraFrames.front() = frame;
        f.markTagChanged(tagNr, frame.getExtendedType());
        return true;
      }
#endif
      if (const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
          index >= 0 && index < static_cast<int>(frameList.size())) {
        // This is a hack. The frameList should not be modified directly.
        // However when removing the old frame and adding a new frame,
        // the indices of all frames get invalid.
        setId3v2Frame(&f, frameList[index], frame);
        f.markTagChanged(tagNr, frame.getExtendedType());
#if TAGLIB_VERSION >= 0x010a00
        // Update the pseudo Chapters frame if CHAP or CTOC frame is modified.
        if (frame.getType() == Frame::FT_Other &&
          (frame.getName().startsWith(QLatin1String("CHAP")) ||
            frame.getName().startsWith(QLatin1String("CTOC"))) &&
          !f.m_extraFrames.isEmpty() &&
          f.m_extraFrames.front().getName() == QLatin1String("Chapters")) {
          const auto ctocFrames = id3v2Tag->frameList("CTOC");
          for (auto ctocFrame : ctocFrames) {
            if (ctocChapToChaptersFrame(
              ctocFrame, id3v2Tag->frameList("CHAP"), f.m_extraFrames.front())) {
              break;
            }
          }
        }
#endif
        return true;
      }
    }
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  if (tagNr == Frame::Tag_Id3v1) {
    return setFrameWithoutIndex(f, tagNr, frame);
  }
  return false;
}

bool TagLibMpegSupport::addFrame(TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const
{
  if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr])) {
#if TAGLIB_VERSION >= 0x010a00
    if (frame.getType() == Frame::FT_Other &&
        frame.getName() == QLatin1String("Chapters") &&
        f.m_extraFrames.isRead()) {
      if (frame.getFieldList().empty()) {
        setChaptersFrameFields(frame);
      }
      frame.setIndex(Frame::toNegativeIndex(f.m_extraFrames.size()));
      f.m_extraFrames.append(frame);
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
#endif
    if (TagLib::ID3v2::Frame* id3Frame = createId3FrameFromFrame(&f, frame)) {
      if (frame.fieldList().empty()) {
        // add field list to frame
        getFieldsFromId3Frame(id3Frame, frame.fieldList(), frame.getType());
        frame.setFieldListFromValue();
      }
      if (frame.getType() == Frame::FT_Other) {
        // Set the correct frame type if the frame was added using the ID.
        Frame::Type type;
        const char* str;
        getTypeStringForFrameId(id3Frame->frameID(), type, str);
        if (type != Frame::FT_UnknownFrame) {
          frame.setExtendedType(
            Frame::ExtendedType(type, QString::fromLatin1(str)));
        }
      }
      frame.setIndex(id3v2Tag->frameList().size());
      addTagLibFrame(id3v2Tag, id3Frame);
      f.markTagChanged(tagNr, frame.getExtendedType());
      return true;
    }
  }
  return false;
}

bool TagLibMpegSupport::deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
  const Frame& frame) const
{
  if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr])) {
    if (int index = frame.getIndex(); index != -1) {
      if (const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
          index >= 0 && index < static_cast<int>(frameList.size())) {
        id3v2Tag->removeFrame(frameList[index]);
        f.markTagChanged(tagNr, frame.getExtendedType());
        return true;
      }
    }
  }
  return false;
}

bool TagLibMpegSupport::deleteFrames(
  TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const
{
  if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr])) {
    const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
    if (flt.areAllEnabled()) {
      for (auto it = frameList.begin();
           it != frameList.end();) {
        id3v2Tag->removeFrame(*it++, true);
      }
#if TAGLIB_VERSION >= 0x010a00
      f.m_extraFrames.clear();
#endif
    } else {
      for (auto it = frameList.begin();
         it != frameList.end();) {
        if (Frame frame(createFrameFromId3Frame(*it, -1));
            flt.isEnabled(frame.getType(), frame.getName())) {
          id3v2Tag->removeFrame(*it++, true);
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

bool TagLibMpegSupport::getAllFrames(
  TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const
{
  if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr])) {
    const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameList();
    int i = 0;
    for (auto it = frameList.begin();
         it != frameList.end();
         ++it) {
      Frame frame(createFrameFromId3Frame(*it, i++));
      if (frame.getType() == Frame::FT_UnknownFrame) {
        if (TagLib::ByteVector frameID = (*it)->frameID().mid(0, 4);
            frameID == "TDAT" || frameID == "TIME" || frameID == "TRDA" ||
            frameID == "TYER") {
          // These frames are converted to a TDRC frame by TagLib.
          continue;
        }
      }
      frames.insert(frame);
    }
#if TAGLIB_VERSION >= 0x010a00
    if (f.m_extraFrames.isRead()) {
      for (auto it = f.m_extraFrames.constBegin(); it != f.m_extraFrames.constEnd(); ++it) {
        frames.insert(*it);
      }
    }
#endif
    return true;
  }
  return false;
}

QStringList TagLibMpegSupport::getFrameIds(
  const TagLibFile& f, Frame::TagNumber tagNr) const
{
  QStringList lst;
  if (f.m_tagType[tagNr] == TaggedFile::TT_Id3v2 ||
      (f.m_tagType[tagNr] == TaggedFile::TT_Unknown &&
       dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr]))) {
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      if (auto name = Frame::ExtendedType(
            static_cast<Frame::Type>(k), QLatin1String("")).getName(); // clazy:exclude=reserve-candidates
          !name.isEmpty()) {
        lst.append(name);
      }
    }
    for (const auto& [str, type, supported] : typeStrOfId) {
      if (type == Frame::FT_Other && supported && str) {
        lst.append(QString::fromLatin1(str));
      }
    }
#if TAGLIB_VERSION >= 0x010a00
    lst.append(QLatin1String("Chapters"));
#endif
  }
  return lst;
}

/**
 * Truncate a string if needed.
 * @param f file
 * @param tagNr tag number
 * @param str  string to be checked
 * @param flag flag to be set if string has to be truncated
 * @param len  maximum length of string
 *
 * @return str truncated to len characters if necessary, else unchanged string.
 */
TagLib::String TagLibMpegSupport::truncateIfNeeded(TagLibFile& f,
  Frame::TagNumber tagNr, const TagLib::String& str, quint64 flag, int len)
{
  TagLib::String result = str;
  if (tagNr != Frame::Tag_Id3v1)
    return result;

  bool priorTruncation = f.hasTruncationFlag();
  if (static_cast<int>(str.length()) > len) {
    result = str.substr(0, len);
    f.setTruncationFlag(flag);
  } else {
    f.clearTruncationFlag(flag);
  }
  f.notifyTruncationChanged(priorTruncation);
  return result;
}

void TagLibMpegSupport::setTagValue(TagLibFile& f, Frame::TagNumber tagNr, Frame::Type type, const TagLib::String& str) const
{
  TagLib::Tag* tag = f.m_tag[tagNr];
  if (type == Frame::FT_Date) {
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      if (setId3v2Unicode(id3v2Tag, str, "TDRC")) {
        return;
      }
    }
  } else if (type == Frame::FT_Track) {
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      if (const char* frameId = "TRCK";
          !setId3v2Unicode(id3v2Tag, str, frameId)) {
        auto trackFrame = new TagLib::ID3v2::TextIdentificationFrame(
              frameId, f.getDefaultTextEncoding());
        trackFrame->setText(str);
        id3v2Tag->removeFrames(frameId);
        addTagLibFrame(id3v2Tag, trackFrame);
      }
      return;
    }
  } else if (type == Frame::FT_Album) {
    if (tagNr == Frame::Tag_Id3v1) {
      tag->setAlbum(truncateIfNeeded(f, tagNr, str, 1ULL << Frame::FT_Album, 30));
      return;
    }
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      if (setId3v2Unicode(id3v2Tag, str, "TALB")) {
        return;
      }
    }
  } else if (type == Frame::FT_Comment) {
    if (tagNr == Frame::Tag_Id3v1) {
      tag->setComment(truncateIfNeeded(f, tagNr, str, 1ULL << Frame::FT_Comment, 28));
      return;
    }
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      if (setId3v2Unicode(id3v2Tag, str, "COMM")) {
        return;
      }
    }
  } else if (type == Frame::FT_Artist) {
    if (tagNr == Frame::Tag_Id3v1) {
      tag->setArtist(truncateIfNeeded(f, tagNr, str, 1ULL << Frame::FT_Artist, 30));
      return;
    }
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      if (setId3v2Unicode(id3v2Tag, str, "TPE1")) {
        return;
      }
    }
  } else if (type == Frame::FT_Title) {
    if (tagNr == Frame::Tag_Id3v1) {
      tag->setTitle(truncateIfNeeded(f, tagNr, str, 1ULL << Frame::FT_Title, 30));
      return;
    }
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      if (setId3v2Unicode(id3v2Tag, str, "TIT2")) {
        return;
      }
    }
  } else if (type == Frame::FT_Genre) {
    if (tagNr == Frame::Tag_Id3v1) {
      TagLib::String tstr = str;
      const auto genres = splitToTStringList(toQString(tstr));
      for (const auto& genre : genres) {
        if (TagLib::ID3v1::genreIndex(genre) != 0xff) {
          tstr = genre;
          break;
        }
        static const struct {
          const char* newName;
          const char* oldName;
        } alternativeGenreNames[] = {
            { "Avant-Garde", "Avantgarde" },
            { "Beat Music", "Beat" },
            { "Bebop", "Bebob" },
            { "Britpop", "BritPop" },
            { "Dancehall", "Dance Hall" },
            { "Dark Wave", "Darkwave" },
            { "Euro House", "Euro-House" },
            { "Eurotechno", "Euro-Techno" },
            { "Fast Fusion", "Fusion" },
            { "Folk Rock", "Folk/Rock" },
            { "Hip Hop", "Hip-Hop" },
            { "Jazz-Funk", "Jazz+Funk" },
            { "Pop-Funk", "Pop/Funk" },
            { "Synth-Pop", "Synthpop" },
            { "Worldbeat", "Negerpunk" }
          };
        static TagLib::Map<TagLib::String, TagLib::String> genreNameMap;
        if (genreNameMap.isEmpty()) {
          // first time initialization
          for (const auto& [newName, oldName] : alternativeGenreNames) {
            genreNameMap.insert(newName, oldName);
          }
        }
        if (auto it = genreNameMap.find(tstr);
          it != genreNameMap.end()) {
          tstr = it->second;
          break;
        }
      }
      tag->setGenre(tstr);
      // if the string cannot be converted to a number, set the truncation flag
      truncateIfNeeded(f, tagNr, !tstr.isEmpty() &&
                      TagLib::ID3v1::genreIndex(tstr) == 0xff
                      ? "1" : "", 1ULL << type, 0);
      return;
    }
    if (auto id3v2Tag = dynamic_cast<TagLib::ID3v2::Tag*>(tag)) {
      const char* frameId = "TCON";
      if (TagLib::ID3v2::TextIdentificationFrame* genreFrame;
          TagConfig::instance().genreNotNumeric() &&
          (genreFrame = new TagLib::ID3v2::TextIdentificationFrame(
            frameId, f.getDefaultTextEncoding())) != nullptr) {
        genreFrame->setText(str);
        id3v2Tag->removeFrames(frameId);
        addTagLibFrame(id3v2Tag, genreFrame);
        return;
      }
    }
  }
  TagLibFormatSupport::setTagValue(f, tagNr, type, str);
}

void TagLibMpegSupport::addFieldList(const TagLibFile& f, Frame::TagNumber tagNr, Frame& frame)
{
  if (dynamic_cast<TagLib::ID3v2::Tag*>(f.m_tag[tagNr]) != nullptr &&
      frame.fieldList().isEmpty()) {
    TagLib::ID3v2::Frame* id3Frame = createId3FrameFromFrame(&f, frame);
    getFieldsFromId3Frame(id3Frame, frame.fieldList(), frame.getType());
    frame.setFieldListFromValue();
    delete id3Frame;
  }
}
