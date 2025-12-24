/**
 * \file taglibfile.cpp
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 *
 * Copyright (C) 2006-2024  Urs Fleisch
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

#include "taglibfile.h"
#include <QDir>
#include <QString>

#include "textcodecstringhandler.h"
#include <QByteArray>
#include <QScopedPointer>
#if TAGLIB_VERSION < 0x020000
#include "taglibext/aac/aacfiletyperesolver.h"
#include "taglibext/mp2/mp2filetyperesolver.h"
#endif
#include "genres.h"
#include "pictureframe.h"

#include "taglibutils.h"
#include "taglibfileiostream.h"
#include "taglibmpegsupport.h"
#include "taglibmp4support.h"
#include "taglibvorbissupport.h"
#include "taglibriffsupport.h"
#include "taglibapesupport.h"
#include "taglibtrueaudiosupport.h"
#include "taglibasfsupport.h"
#include "taglibmodsupport.h"
#include "taglibdsfsupport.h"
#if TAGLIB_VERSION >= 0x020200
#include "taglibmatroskasupport.h"
#endif
#include "taglibgenericsupport.h"

using namespace TagLibUtils;

/** Default text encoding */
TagLib::String::Type TagLibFile::s_defaultTextEncoding = TagLib::String::Latin1;

/** Format support */
QList<TagLibFormatSupport*> TagLibFile::s_formats;

/**
 * Constructor.
 *
 * @param idx index in tagged file system model
 */
TagLibFile::TagLibFile(const QPersistentModelIndex& idx)
  : TaggedFile(idx),
    m_tagInformationRead(false), m_fileRead(false),
    m_stream(nullptr),
    m_id3v2Version(0),
    m_activatedFeatures(0), m_duration(0)
{
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = false;
    m_isTagSupported[tagNr] = tagNr == Frame::Tag_2;
    m_tag[tagNr] = nullptr;
    m_tagType[tagNr] = TT_Unknown;
  }
}

/**
 * Destructor.
 */
TagLibFile::~TagLibFile()
{
  closeFile(true);
}

/**
 * Get key of tagged file format.
 * @return "TaglibMetadata".
 */
QString TagLibFile::taggedFileKey() const
{
  return QLatin1String("TaglibMetadata");
}

/**
 * Get features supported.
 * @return bit mask with Feature flags set.
 */
int TagLibFile::taggedFileFeatures() const
{
  return TF_ID3v11 | TF_ID3v22 |
      TF_OggFlac |
      TF_OggPictures |
      TF_ID3v23 |
      TF_ID3v24;
}

/**
 * Get currently active tagged file features.
 * @return active tagged file features (TF_ID3v23, TF_ID3v24, or 0).
 * @see setActiveTaggedFileFeatures()
 */
int TagLibFile::activeTaggedFileFeatures() const
{
  return m_activatedFeatures;
}

/**
 * Activate some features provided by the tagged file.
 * TagLibFile provides the TF_ID3v23 and TF_ID3v24 features, which determine
 * the ID3v2 version used in writeTags() (the overload without id3v2Version).
 * If 0 is set, the default behavior applies, i.e. for new files,
 * TagConfig::id3v2Version() is used, else the existing version.
 *
 * @param features TF_ID3v23, TF_ID3v24, or 0
 */
void TagLibFile::setActiveTaggedFileFeatures(int features)
{
  m_activatedFeatures = features;
}

/**
 * Free resources allocated when calling readTags().
 *
 * @param force true to force clearing even if the tags are modified
 */
void TagLibFile::clearTags(bool force)
{
  if (isChanged() && !force)
    return;

  bool priorIsTagInformationRead = isTagInformationRead();
  closeFile(true);
  m_extraFrames.clear();
  m_extraFrames.setRead(false);
  m_tagInformationRead = false;
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = false;
    m_tagFormat[tagNr].clear();
    m_tagType[tagNr] = TT_Unknown;
  }
  FOR_TAGLIB_TAGS(tagNr) {
    markTagUnchanged(tagNr);
  }
  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void TagLibFile::readTags(bool force)
{
  bool priorIsTagInformationRead = isTagInformationRead();
  QString fileName = currentFilePath();

  if (force || m_fileRef.isNull()) {
    delete m_stream;
    m_stream = new FileIOStream(fileName);
    m_fileRef = TagLib::FileRef(FileIOStream::create(m_stream));
    if (m_fileRef.isNull()) {
#ifdef Q_OS_WIN32
      m_fileRef = TagLib::FileRef(fileName.toStdWString().c_str());
#else
      m_fileRef = TagLib::FileRef(QFile::encodeName(fileName).constData());
#endif
    }
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = nullptr;
    }
    FOR_TAGLIB_TAGS(tagNr) {
      markTagUnchanged(tagNr);
    }
    m_fileRead = true;

    m_extraFrames.clear();
    m_extraFrames.setRead(false);
  }

  if (TagLib::File* file;
      !m_fileRef.isNull() && (file = m_fileRef.file()) != nullptr) {
    m_fileExtension = QLatin1String(".mp3");
    m_isTagSupported[Frame::Tag_1] = false;
    for (auto format : s_formats) {
      if (format->readFile(*this, file)) {
        break;
      }
    }
  }

  // Cache information, so that it is available after file is closed.
  m_tagInformationRead = true;
  FOR_TAGLIB_TAGS(tagNr) {
    m_hasTag[tagNr] = m_tag[tagNr] && !m_tag[tagNr]->isEmpty();
    m_tagFormat[tagNr] = getTagFormat(m_tag[tagNr], m_tagType[tagNr]);
  }
  readAudioProperties();

  if (force) {
    setFilename(currentFilename());
  }

  closeFile(false);

  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Close file handle.
 * TagLib keeps the file handle open until the FileRef is destroyed.
 * This causes problems when the operating system has a limited number of
 * open file handles. This method closes the file by assigning a new file
 * reference. Note that this will also invalidate the tag pointers.
 * The file is only closed if there are no unsaved tag changes or if the
 * @a force parameter is set.
 *
 * @param force true to close the file even if tags are changed
 */
void TagLibFile::closeFile(bool force)
{
  if (force) {
    m_fileRef = TagLib::FileRef();
    delete m_stream;
    m_stream = nullptr;
    FOR_TAGLIB_TAGS(tagNr) {
      m_tag[tagNr] = nullptr;
    }
    m_fileRead = false;
  } else if (m_stream) {
    m_stream->closeFileHandle();
  }
}

/**
 * Make sure that file is open.
 * This method should be called before accessing m_fileRef, m_tag.
 *
 * @param force true to force reopening of file even if it is already open
 */
void TagLibFile::makeFileOpen(bool force) const
{
  if (!m_fileRead || force) {
    const_cast<TagLibFile*>(this)->readTags(force);
  }
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force    true to force writing even if file was not changed.
 * @param renamed  will be set to true if the file was renamed,
 *                 i.e. the file name is no longer valid, else *renamed
 *                 is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool TagLibFile::writeTags(bool force, bool* renamed, bool preserve)
{
  int id3v2Version;
  if (m_activatedFeatures & TF_ID3v24)
    id3v2Version = 4;
  else if (m_activatedFeatures & TF_ID3v23)
    id3v2Version = 3;
  else
    id3v2Version = 0;
  return writeTags(force, renamed, preserve, id3v2Version);
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force    true to force writing even if file was not changed.
 * @param renamed  will be set to true if the file was renamed,
 *                 i.e. the file name is no longer valid, else *renamed
 *                 is left unchanged
 * @param preserve true to preserve file time stamps
 * @param id3v2Version ID3v2 version to use, 0 to use existing or preferred,
 *                     3 to force ID3v2.3.0, 4 to force ID3v2.4.0. Is ignored
 *                     if TagLib version is less than 1.8.0.
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool TagLibFile::writeTags(bool force, bool* renamed, bool preserve,
                           int id3v2Version)
{
  QString fnStr(currentFilePath());
  if (isChanged() && !QFileInfo(fnStr).isWritable()) {
    closeFile(false);
    revertChangedFilename();
    return false;
  }

  // store time stamp if it has to be preserved
  quint64 actime = 0, modtime = 0;
  if (preserve) {
    getFileTimeStamps(fnStr, actime, modtime);
  }

  bool fileChanged = false;
  if (TagLib::File* file;
      !m_fileRef.isNull() && (file = m_fileRef.file()) != nullptr) {
    if (m_stream) {
#ifndef Q_OS_WIN32
      QString fileName = QFile::decodeName(m_stream->name());
#else
      QString fileName = toQString(m_stream->name().toString());
#endif
      if (fnStr != fileName) {
        qDebug("TagLibFile: Fix file name mismatch, should be '%s', not '%s'",
               qPrintable(fnStr), qPrintable(fileName));
        m_stream->setName(fnStr);
      }
    }
    for (auto format : s_formats) {
      if (format->writeFile(*this, file, force, id3v2Version, fileChanged)) {
        break;
      }
    }
  }

  // If the file was changed, make sure it is written to disk.
  // This is done when the file is closed. Later the file is opened again.
  // If the file is not properly closed, doubled tags can be
  // written if the file is finally closed!
  // This can be reproduced with an untagged MP3 file, then add
  // an ID3v2 title, save, add an ID3v2 artist, save, reload
  // => double ID3v2 tags.
  // On Windows it is necessary to close the file before renaming it,
  // so it is done even if the file is not changed.
#ifndef Q_OS_WIN32
  closeFile(fileChanged);
#else
  closeFile(true);
#endif

  // restore time stamp
  if (actime || modtime) {
    setFileTimeStamps(fnStr, actime, modtime);
  }

  if (isFilenameChanged()) {
    if (!renameFile()) {
      return false;
    }
    markFilenameUnchanged();
    *renamed = true;
  }

#ifndef Q_OS_WIN32
  if (fileChanged)
#endif
    makeFileOpen(true);
  return true;
}

namespace {

/**
 * Get a genre string from a string which can contain the genre itself,
 * or only the genre number or the genre number in parentheses.
 *
 * @param str genre string
 *
 * @return genre.
 */
QString getGenreString(const TagLib::String& str)
{
#if TAGLIB_VERSION < 0x010b01
  if (str.isNull()) {
    return QLatin1String("");
  }
#endif
  QString qs = toQString(str);
  int n = 0xff;
  bool ok = false;
  if (int cpPos = 0;
      !qs.isEmpty() && qs[0] == QLatin1Char('(') &&
      (cpPos = qs.indexOf(QLatin1Char(')'), 2)) > 1) {
#if QT_VERSION >= 0x060000
    n = qs.mid(1, cpPos - 1).toInt(&ok);
#else
    n = qs.midRef(1, cpPos - 1).toInt(&ok);
#endif
    if (!ok || n > 0xff) {
      n = 0xff;
    }
    return QString::fromLatin1(Genres::getName(n));
  }
  if ((n = qs.toInt(&ok)) >= 0 && n <= 0xff && ok) {
    return QString::fromLatin1(Genres::getName(n));
  }
  return qs;
}

}

/**
 * Create tag if it does not already exist so that it can be set.
 *
 * @return true if tag can be set.
 */
bool TagLibFile::makeTagSettable(Frame::TagNumber tagNr)
{
  if (tagNr >= NUM_TAGS)
    return false;

  makeFileOpen();
  if (!m_tag[tagNr]) {
    if (TagLib::File* file;
        !m_fileRef.isNull() && (file = m_fileRef.file()) != nullptr) {
      for (auto format : s_formats) {
        if (format->makeTagSettable(*this, file, tagNr)) {
          break;
        }
      }
    }
  }
  return m_tag[tagNr] != nullptr;
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTag() does not return meaningful information.
 */
bool TagLibFile::isTagInformationRead() const
{
  return m_tagInformationRead;
}

/**
 * Check if tags are supported by the format of this file.
 *
 * @param tagNr tag number
 * @return true.
 */
bool TagLibFile::isTagSupported(Frame::TagNumber tagNr) const
{
  return tagNr < NUM_TAGS ? m_isTagSupported[tagNr] : false;
}

/**
 * Check if file has a tag.
 *
 * @param tagNr tag number
 * @return true if tag is available.
 * @see isTagInformationRead()
 */
bool TagLibFile::hasTag(Frame::TagNumber tagNr) const
{
  return tagNr < NUM_TAGS ? m_hasTag[tagNr] : false;
}

/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void TagLibFile::getDetailInfo(DetailInfo& info) const
{
  info = m_detailInfo;
}

/**
 * Cache technical detail information.
 */
void TagLibFile::readAudioProperties()
{
  if (TagLib::AudioProperties* audioProperties;
      !m_fileRef.isNull() &&
      (audioProperties = m_fileRef.audioProperties()) != nullptr) {
    m_detailInfo.valid = true;
    for (auto format : s_formats) {
      if (format->readAudioProperties(*this, audioProperties)) {
        break;
      }
    }
    m_detailInfo.bitrate = audioProperties->bitrate();
    m_detailInfo.sampleRate = audioProperties->sampleRate();
    if (audioProperties->channels() > 0) {
      m_detailInfo.channels = audioProperties->channels();
    }
#if TAGLIB_VERSION >= 0x020000
    m_detailInfo.duration = audioProperties->lengthInSeconds();
#else
    // lengthInSeconds() does not work for DSF with TagLib 1.x, because
    // it is not virtual.
    m_detailInfo.duration = audioProperties->length();
#endif
  } else {
    m_detailInfo.valid = false;
  }
}

/**
 * Set m_id3v2Version to 3 or 4 from tag if it exists, else to 0.
 * @param id3v2Tag ID3v2 tag
 */
void TagLibFile::setId3v2VersionFromTag(const TagLib::ID3v2::Tag* id3v2Tag)
{
  m_id3v2Version = 0;
  if (TagLib::ID3v2::Header* header;
      id3v2Tag && (header = id3v2Tag->header()) != nullptr) {
    if (!id3v2Tag->isEmpty()) {
      m_id3v2Version = header->majorVersion();
    } else {
      header->setMajorVersion(TagConfig::instance().id3v2Version() ==
                              TagConfig::ID3v2_3_0 ? 3 : 4);
    }
  }
}

/**
 * Set m_id3v2Version from given value (3 or 4) or use default from
 * configuration if not already set to 3 or 4.
 * @param id3v2Version 3 or 4 to force version, 0 to use existing version
 * or default
 */
void TagLibFile::setId3v2VersionOrDefault(int id3v2Version)
{
  if (id3v2Version == 3 || id3v2Version == 4) {
    m_id3v2Version = id3v2Version;
  }
  if (m_id3v2Version != 3 && m_id3v2Version != 4) {
    m_id3v2Version = TagConfig::instance().id3v2Version() ==
        TagConfig::ID3v2_3_0 ? 3 : 4;
  }
}

/**
 * Get duration of file.
 *
 * @return duration in seconds,
 *         0 if unknown.
 */
unsigned TagLibFile::getDuration() const
{
  return m_detailInfo.valid ? m_detailInfo.duration : 0;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".mp3".
 */
QString TagLibFile::getFileExtension() const
{
  return m_fileExtension;
}

/**
 * Get the format of a tag.
 *
 * @param tag tag, 0 if no tag available
 * @param type the tag type is returned here
 *
 * @return string describing format of tag,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TagLibFile::getTagFormat(const TagLib::Tag* tag, TagType& type)
{
  if (tag && !tag->isEmpty()) {
    for (auto format : s_formats) {
      if (QString tagFormat = format->getTagFormat(tag, type);
          !tagFormat.isNull()) {
        return tagFormat;
      }
    }
  }
  type = TT_Unknown;
  return QString();
}

/**
 * Get the format of tag.
 *
 * @param tagNr tag number
 * @return string describing format of tag,
 *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
 *         QString::null if unknown.
 */
QString TagLibFile::getTagFormat(Frame::TagNumber tagNr) const
{
  return tagNr < NUM_TAGS ? m_tagFormat[tagNr] : QString();
}

/**
 * Get internal name of a Vorbis frame.
 *
 * @param frame frame
 *
 * @return Vorbis key.
 */
QString TagLibFile::getVorbisName(const Frame& frame) const
{
  if (Frame::Type type = frame.getType(); type == Frame::FT_Comment) {
    return getCommentFieldName();
  } else if (type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getVorbisNameFromType(type));
  }
  return fixUpTagKey(frame.getName(), TT_Vorbis).toUpper();
}

/**
 * Get a specific frame from the tags.
 *
 * @param tagNr tag number
 * @param type  frame type
 * @param frame the frame is returned here
 *
 * @return true if ok.
 */
bool TagLibFile::getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const
{
  if (tagNr >= NUM_TAGS)
    return false;

  makeFileOpen();
  if (TagLib::Tag* tag = m_tag[tagNr]) {
    TagLib::String tstr;
    switch (type) {
    case Frame::FT_Album:
      tstr = tag->album();
      break;
    case Frame::FT_Artist:
      tstr = tag->artist();
      break;
    case Frame::FT_Comment:
      tstr = tag->comment();
      if (tagNr == Frame::Tag_Id3v1
#if TAGLIB_VERSION < 0x010b01
          && !tstr.isNull()
#endif
          ) {
        tstr = tstr.substr(0, 28);
      }
      break;
    case Frame::FT_Date:
    {
      uint nr = tag->year();
      tstr = nr != 0 ? TagLib::String::number(nr) : "";
      break;
    }
    case Frame::FT_Genre:
      tstr = tag->genre();
      break;
    case Frame::FT_Title:
      tstr = tag->title();
      break;
    case Frame::FT_Track:
    {
      uint nr = tag->track();
      tstr = nr != 0 ? TagLib::String::number(nr) : "";
      break;
    }
    default:
      // maybe handled in a subclass
      return false;
    }
#if TAGLIB_VERSION >= 0x010b01
    QString str = tagNr != Frame::Tag_Id3v1 && type == Frame::FT_Genre
        ? getGenreString(tstr) : toQString(tstr);
#else
    QString str = tagNr != Frame::Tag_Id3v1 && type == Frame::FT_Genre
        ? getGenreString(tstr)
        : tstr.isNull() ? QLatin1String("") : toQString(tstr);
#endif
    frame.setValue(str);
  } else {
    frame.setValue(QString());
  }
  frame.setType(type);
  return true;
}

/**
 * Set a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool TagLibFile::setFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr >= NUM_TAGS)
    return false;

  makeFileOpen();
  for (auto format : s_formats) {
    if (format->setFrame(*this, tagNr, frame)) {
      return true;
    }
  }
  return false;
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add, a field list may be added by this method
 *
 * @return true if ok.
 */
bool TagLibFile::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr >= NUM_TAGS)
    return false;

  if (tagNr != Frame::Tag_Id3v1) {
    // Add a new frame.
    if (makeTagSettable(tagNr)) {
      for (auto format : s_formats) {
        if (format->addFrame(*this, tagNr, frame)) {
          return true;
        }
      }
    }
  }

  // Try the superclass method
  return TaggedFile::addFrame(tagNr, frame);
}

/**
 * Delete a frame from the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool TagLibFile::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr >= NUM_TAGS)
    return false;

  if (tagNr != Frame::Tag_Id3v1) {
    makeFileOpen();
    // If the frame has an index, delete that specific frame
    if (int index = frame.getIndex(); index != -1 && m_tag[tagNr]) {
      for (auto format : s_formats) {
        if (format->deleteFrame(*this, tagNr, frame)) {
          return true;
        }
      }
    }
  }

  // Try the superclass method
  return TaggedFile::deleteFrame(tagNr, frame);
}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void TagLibFile::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  if (tagNr >= NUM_TAGS)
    return;

  makeFileOpen();
  if (tagNr == Frame::Tag_Id3v1) {
    if (m_tag[tagNr]) {
      TaggedFile::deleteFrames(tagNr, flt);
    }
  } else {
    if (m_tag[tagNr]) {
      for (auto format : s_formats) {
        if (format->deleteFrames(*this, tagNr, flt)) {
          return;
        }
      }
      TaggedFile::deleteFrames(tagNr, flt);
    }
  }
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void TagLibFile::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  if (tagNr >= NUM_TAGS)
    return;

  if (tagNr != Frame::Tag_Id3v1) {
    makeFileOpen();
    frames.clear();
    if (m_tag[tagNr]) {
      bool tagHandled = false;
      for (auto format : s_formats) {
        tagHandled = format->getAllFrames(*this, tagNr, frames);
        if (tagHandled) {
          break;
        }
      }

      if (!tagHandled) {
        TaggedFile::getAllFrames(tagNr, frames);
      }
    }
    updateMarkedState(tagNr, frames);
    if (tagNr <= Frame::Tag_2) {
      frames.addMissingStandardFrames();
    }
    return;
  }

  TaggedFile::getAllFrames(tagNr, frames);
}

/**
 * Close file handle which is held open by the TagLib object.
 */
void TagLibFile::closeFileHandle()
{
  closeFile(false);
}

/**
 * Add a suitable field list for the frame if missing.
 * If a frame is created, its field list is empty. This method will create
 * a field list appropriate for the frame type and tagged file type if no
 * field list exists.
 * @param tagNr tag number
 * @param frame frame where field list is added
 */
void TagLibFile::addFieldList(Frame::TagNumber tagNr, Frame& frame) const
{
  TagLibMpegSupport::addFieldList(*this, tagNr, frame);
}

/**
 * Get a list of frame IDs which can be added.
 * @param tagNr tag number
 * @return list with frame IDs.
 */
QStringList TagLibFile::getFrameIds(Frame::TagNumber tagNr) const
{
  for (auto format : s_formats) {
    if (QStringList lst = format->getFrameIds(*this, tagNr); !lst.isEmpty()) {
      return lst;
    }
  }
  return {};
}

/**
 * Set the encoding to be used for tag 1.
 *
 * @param name of encoding, default is ISO 8859-1
 */
void TagLibFile::setTextEncodingV1(const QString& name)
{
#if QT_VERSION >= 0x060000
  TextCodecStringHandler::setStringDecoder(name);
#else
  TextCodecStringHandler::setTextCodec(name != QLatin1String("ISO-8859-1")
      ? QTextCodec::codecForName(name.toLatin1().data()) : nullptr);
#endif
}

/**
 * Set the default text encoding.
 *
 * @param textEnc default text encoding
 */
void TagLibFile::setDefaultTextEncoding(TagConfig::TextEncoding textEnc)
{
  // Do not use TagLib::ID3v2::FrameFactory::setDefaultTextEncoding(),
  // it will change the encoding of existing frames read in, not only
  // of newly created frames, which is really not what we want!
  switch (textEnc) {
  case TagConfig::TE_ISO8859_1:
    s_defaultTextEncoding = TagLib::String::Latin1;
    break;
  case TagConfig::TE_UTF16:
    s_defaultTextEncoding = TagLib::String::UTF16;
    break;
  case TagConfig::TE_UTF8:
  default:
    s_defaultTextEncoding = TagLib::String::UTF8;
  }
}

/**
 * Notify about configuration change.
 * This method shall be called when the configuration changes.
 */
void TagLibFile::notifyConfigurationChange()
{
  setDefaultTextEncoding(
    static_cast<TagConfig::TextEncoding>(TagConfig::instance().textEncoding()));
  setTextEncodingV1(TagConfig::instance().textEncodingV1());
}

namespace {

/**
 * Used to register file types at static initialization time.
 */
class TagLibInitializer {
public:
  /** Constructor. */
  TagLibInitializer();

  /** Destructor. */
  ~TagLibInitializer();

  /**
   * Initialization.
   * Is deferred because it will crash on Mac OS X if done in the constructor.
   */
  void init();

private:
  Q_DISABLE_COPY(TagLibInitializer)

#if TAGLIB_VERSION < 0x020000
  QScopedPointer<AACFileTypeResolver> m_aacFileTypeResolver;
  QScopedPointer<MP2FileTypeResolver> m_mp2FileTypeResolver;
#endif
  QScopedPointer<TextCodecStringHandler> m_textCodecStringHandler;
};


TagLibInitializer::TagLibInitializer() :
#if TAGLIB_VERSION < 0x020000
    m_aacFileTypeResolver(new AACFileTypeResolver),
    m_mp2FileTypeResolver(new MP2FileTypeResolver),
#endif
    m_textCodecStringHandler(new TextCodecStringHandler)
{
}

void TagLibInitializer::init()
{
#if TAGLIB_VERSION < 0x020000
  TagLib::FileRef::addFileTypeResolver(m_aacFileTypeResolver.data());
  TagLib::FileRef::addFileTypeResolver(m_mp2FileTypeResolver.data());
#endif
  TagLib::ID3v1::Tag::setStringHandler(m_textCodecStringHandler.data());
}

TagLibInitializer::~TagLibInitializer() {
  // Must not be inline because of forward declared QScopedPointer.
}

TagLibInitializer tagLibInitializer;

}

/**
 * Static initialization.
 * Registers file types.
 */
void TagLibFile::staticInit()
{
  s_formats = {
    new TagLibMpegSupport,
    new TagLibMp4Support,
    new TagLibVorbisSupport,
    new TagLibRiffSupport,
    new TagLibApeSupport,
    new TagLibTrueAudioSupport,
    new TagLibAsfSupport,
    new TagLibModSupport,
    new TagLibDsfSupport,
#if TAGLIB_VERSION >= 0x020200
    new TagLibMatroskaSupport,
#endif
    // It is essential that TagLibGenericSupport is last to provide defaults for
    // writeFile() and getFrameIds().
    new TagLibGenericSupport
  };
  FileIOStream::registerFormatSupport(s_formats);
  tagLibInitializer.init();
}
