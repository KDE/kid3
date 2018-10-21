/**
 * \file oggfile.cpp
 * Handling of Ogg files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
 *
 * Copyright (C) 2005-2018  Urs Fleisch
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

#include "oggfile.hpp"

#include <QFile>
#include <QDir>
#include <QByteArray>
#include <cstdio>
#include <cmath>
#ifdef HAVE_VORBIS
#include <vorbis/vorbisfile.h>
#include "vcedit.h"
#endif
#include "pictureframe.h"
#include "tagconfig.h"

namespace {

/*
 * The following functions are used to access an Ogg/Vorbis file
 * using a QIODevice. They are used by vcedit_open_callbacks() and
 * ov_open_callbacks().
 */

/**
 * Read from a QIODevice using an fread() like interface.
 * @param ptr location to store data read
 * @param size size of one element in bytes
 * @param nmemb number of elements to read
 * @param stream QIODevice* to read from
 * @return number of elements read.
 */
size_t oggread(void* ptr, size_t size, size_t nmemb, void* stream)
{
  if (!stream || !size)
    return 0;

  auto iodev = reinterpret_cast<QIODevice*>(stream);
  qint64 len = iodev->read(reinterpret_cast<char*>(ptr), size * nmemb);
  return len / size;
}

/**
 * Write to a QIODevice using an fwrite() like interface.
 * @param ptr location of data to write
 * @param size size of one element in bytes
 * @param nmemb number of elements to write
 * @param stream QIODevice* to write to
 * @return number of elements written.
 */
size_t oggwrite(const void* ptr, size_t size, size_t nmemb, void* stream)
{
  if (!stream || !size)
    return 0;

  auto iodev = reinterpret_cast<QIODevice*>(stream);
  qint64 len = iodev->write(reinterpret_cast<const char*>(ptr), size * nmemb);
  return len / size;
}

/**
 * Seek in a QIODevice using an fseek() like interface.
 * @param stream QIODevice* to seek
 * @param offset byte position
 * @param whence SEEK_SET, SEEK_CUR, or SEEK_END
 * @return 0 if ok, -1 on error.
 */
int oggseek(void* stream, ogg_int64_t offset, int whence)
{
  auto iodev = reinterpret_cast<QIODevice*>(stream);
  if (!iodev || iodev->isSequential())
    return -1;

  qint64 pos = offset;
  if (whence == SEEK_END) {
    pos += iodev->size();
  } else if (whence == SEEK_CUR) {
    pos += iodev->pos();
  }

  if (iodev->seek(pos))
    return 0;
  return -1;
}

/**
 * Close QIODevice using an fclose() like interface.
 * @param stream QIODevice* to close
 * @return 0 if ok.
 */
int oggclose(void* stream)
{
  if (auto iodev = reinterpret_cast<QIODevice*>(stream)) {
    iodev->close();
    return 0;
  }
  return -1;
}

/**
 * Get position in QIODevice using an ftell() like interface.
 * @param stream QIODevice*
 * @return current position, -1 on error.
 */
long oggtell(void* stream)
{
  if (auto iodev = reinterpret_cast<QIODevice*>(stream)) {
    return iodev->pos();
  }
  return -1;
}

}

/**
 * Constructor.
 *
 * @param idx index in file proxy model
 */
OggFile::OggFile(const QPersistentModelIndex& idx)
  : TaggedFile(idx), m_fileRead(false)
{
}

/**
 * Get key of tagged file format.
 * @return "OggMetadata".
 */
QString OggFile::taggedFileKey() const
{
  return QLatin1String("OggMetadata");
}

#ifdef HAVE_VORBIS
/**
 * Get features supported.
 * @return bit mask with Feature flags set.
 */
int OggFile::taggedFileFeatures() const
{
  return TF_OggPictures;
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void OggFile::readTags(bool force)
{
  bool priorIsTagInformationRead = isTagInformationRead();
  if (force || !m_fileRead) {
    m_comments.clear();
    markTagUnchanged(Frame::Tag_2);
    m_fileRead = true;
    QString fnIn = currentFilePath();

    if (readFileInfo(m_fileInfo, fnIn)) {
      QFile fpIn(fnIn);
      if (fpIn.open(QIODevice::ReadOnly)) {
        vcedit_state* state = ::vcedit_new_state();
        if (state) {
          if (::vcedit_open_callbacks(state, &fpIn, oggread, oggwrite) >= 0) {
            vorbis_comment* vc = ::vcedit_comments(state);
            if (vc) {
              for (int i = 0; i < vc->comments; ++i) {
                QString userComment =
                  QString::fromUtf8(vc->user_comments[i],
                                    vc->comment_lengths[i]);
                int equalPos = userComment.indexOf(QLatin1Char('='));
                if (equalPos != -1) {
                  QString name(
                    userComment.left(equalPos).trimmed().toUpper());
                  QString value(
                    userComment.mid(equalPos + 1).trimmed());
                  if (!value.isEmpty()) {
                    m_comments.push_back(CommentField(name, value));
                  }
                }
              }
            }
          }
          ::vcedit_clear(state);
        }
        fpIn.close();
      }
    }
  }

  if (force) {
    setFilename(currentFilename());
  }

  notifyModelDataChanged(priorIsTagInformationRead);
}

/**
 * Write tags to file and rename it if necessary.
 *
 * @param force   true to force writing even if file was not changed.
 * @param renamed will be set to true if the file was renamed,
 *                i.e. the file name is no longer valid, else *renamed
 *                is left unchanged
 * @param preserve true to preserve file time stamps
 *
 * @return true if ok, false if the file could not be written or renamed.
 */
bool OggFile::writeTags(bool force, bool* renamed, bool preserve)
{
  QString dirname = getDirname();
  if (isChanged() &&
    !QFileInfo(currentFilePath()).isWritable()) {
    revertChangedFilename();
    return false;
  }

  if (m_fileRead && (force || isTagChanged(Frame::Tag_2))) {
    bool writeOk = false;
    // we have to rename the original file and delete it afterwards
    QString filename = currentFilename();
    QString tempFilename(filename + QLatin1String("_KID3"));
    if (!renameFile(filename, tempFilename)) {
      return false;
    }
    QString fnIn = dirname + QDir::separator() + tempFilename;
    QString fnOut = dirname + QDir::separator() + getFilename();
    QFile fpIn(fnIn);
    if (fpIn.open(QIODevice::ReadOnly)) {

      // store time stamp if it has to be preserved
      quint64 actime = 0, modtime = 0;
      if (preserve) {
        getFileTimeStamps(fnIn, actime, modtime);
      }

      QFile fpOut(fnOut);
      if (fpOut.open(QIODevice::WriteOnly)) {
        vcedit_state* state = ::vcedit_new_state();
        if (state) {
          if (::vcedit_open_callbacks(state, &fpIn, oggread, oggwrite) >= 0) {
            vorbis_comment* vc = ::vcedit_comments(state);
            if (vc) {
              ::vorbis_comment_clear(vc);
              ::vorbis_comment_init(vc);
              auto it = m_comments.begin(); // clazy:exclude=detaching-member
              while (it != m_comments.end()) {
                QString name((*it).getName());
                QString value((*it).getValue());
                if (!value.isEmpty()) {
                  ::vorbis_comment_add_tag(
                    vc,
                    name.toLatin1().data(),
                    value.toUtf8().data());
                  ++it;
                } else {
                  it = m_comments.erase(it);
                }
              }
              if (::vcedit_write(state, &fpOut) >= 0) {
                writeOk = true;
              }
            }
          }
          ::vcedit_clear(state);
        }
        fpOut.close();
      }
      fpIn.close();

      // restore time stamp
      if (actime || modtime) {
        setFileTimeStamps(fnOut, actime, modtime);
      }
    }
    if (!writeOk) {
      // restore old file
      QDir(dirname).remove(getFilename());
      renameFile(tempFilename, currentFilename());
      return false;
    }
    markTagUnchanged(Frame::Tag_2);
    QDir(dirname).remove(tempFilename);
    if (isFilenameChanged()) {
      markFilenameUnchanged();
      *renamed = true;
    }
  } else if (isFilenameChanged()) {
    // tags not changed, but file name
    if (!renameFile(currentFilename(), getFilename())) {
      return false;
    }
    markFilenameUnchanged();
    *renamed = true;
  }
  return true;
}

/**
 * Free resources allocated when calling readTags().
 *
 * @param force true to force clearing even if the tags are modified
 */
void OggFile::clearTags(bool force)
{
  if (!m_fileRead || (isChanged() && !force))
    return;

  bool priorIsTagInformationRead = isTagInformationRead();
  m_comments.clear();
  markTagUnchanged(Frame::Tag_2);
  m_fileRead = false;
  notifyModelDataChanged(priorIsTagInformationRead);
}
#else // HAVE_VORBIS
void OggFile::readTags(bool) {}
bool OggFile::writeTags(bool, bool*, bool) { return false; }
void OggFile::clearTags(bool) {}
#endif // HAVE_VORBIS

namespace {

/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
const char* getVorbisNameFromType(Frame::Type type)
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
    "ENCODED-BY",      // FT_EncodedBy,
    "ENCODERSETTINGS", // FT_EncoderSettings,
    "ENCODINGTIME",    // FT_EncodingTime,
    "GROUPING",        // FT_Grouping,
    "INITIALKEY",      // FT_InitialKey,
    "ISRC",            // FT_Isrc,
    "LANGUAGE",        // FT_Language,
    "LYRICIST",        // FT_Lyricist,
    "LYRICS",          // FT_Lyrics,
    "SOURCEMEDIA",     // FT_Media,
    "MOOD",            // FT_Mood,
    "ORIGINALALBUM",   // FT_OriginalAlbum,
    "ORIGINALARTIST",  // FT_OriginalArtist,
    "ORIGINALDATE",    // FT_OriginalDate,
    "PART",            // FT_Part,
    "PERFORMER",       // FT_Performer,
    "METADATA_BLOCK_PICTURE", // FT_Picture,
    "PUBLISHER",       // FT_Publisher,
    "RELEASECOUNTRY",  // FT_ReleaseCountry,
    "REMIXER",         // FT_Remixer,
    "ALBUMSORT",       // FT_SortAlbum,
    "ALBUMARTISTSORT", // FT_SortAlbumArtist,
    "ARTISTSORT",      // FT_SortArtist,
    "COMPOSERSORT",    // FT_SortComposer,
    "TITLESORT",       // FT_SortName,
    "SUBTITLE",        // FT_Subtitle,
    "WEBSITE",         // FT_Website,
    "WWWAUDIOFILE",    // FT_WWWAudioFile,
    "WWWAUDIOSOURCE",  // FT_WWWAudioSource,
    "RELEASEDATE",     // FT_ReleaseDate,
    "RATING"           // FT_Rating,
                       // FT_LastFrame = FT_Rating
  };
  struct not_used { int array_size_check[
      sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1
      ? 1 : -1 ]; };
  if (type == Frame::FT_Picture &&
      TagConfig::instance().pictureNameIndex() == TagConfig::VP_COVERART) {
    return "COVERART";
  }
  return type <= Frame::FT_LastFrame ? names[type] : "UNKNOWN";
}

/**
 * Get the frame type for a Vorbis name.
 *
 * @param name Vorbis tag name
 *
 * @return frame type.
 */
Frame::Type getTypeFromVorbisName(QString name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
      auto type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getVorbisNameFromType(type)), type);
    }
    strNumMap.insert(QLatin1String("DESCRIPTION"), Frame::FT_Comment);
    strNumMap.insert(QLatin1String("COVERART"), Frame::FT_Picture);
    strNumMap.insert(QLatin1String("METADATA_BLOCK_PICTURE"), Frame::FT_Picture);
  }
  auto it = strNumMap.constFind(name.remove(QLatin1Char('=')).toUpper());
  if (it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::FT_Other;
}

/**
 * Get internal name of a Vorbis frame.
 *
 * @param frame frame
 *
 * @return Vorbis key.
 */
QString getVorbisName(const Frame& frame)
{
  Frame::Type type = frame.getType();
  if (type <= Frame::FT_LastFrame) {
    return QString::fromLatin1(getVorbisNameFromType(type));
  } else {
    return frame.getName().remove(QLatin1Char('=')).toUpper();
  }
}

}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void OggFile::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  if (tagNr != Frame::Tag_2)
    return;

  if (flt.areAllEnabled()) {
    m_comments.clear();
    markTagChanged(Frame::Tag_2, Frame::FT_UnknownFrame);
  } else {
    bool changed = false;
    for (auto it = m_comments.begin(); it != m_comments.end();) { // clazy:exclude=detaching-member
      QString name((*it).getName());
      if (flt.isEnabled(getTypeFromVorbisName(name), name)) {
        it = m_comments.erase(it);
        changed = true;
      } else {
        ++it;
      }
    }
    if (changed) {
      markTagChanged(Frame::Tag_2, Frame::FT_UnknownFrame);
    }
  }
}

/**
 * Get text field.
 *
 * @param name name
 * @return value, "" if not found,
 *         QString::null if the tags have not been read yet.
 */
QString OggFile::getTextField(const QString& name) const
{
  if (m_fileRead) {
    return m_comments.getValue(name);
  }
  return QString();
}

/**
 * Set text field.
 * If value is null if the tags have not been read yet, nothing is changed.
 * If value is different from the current value, tag 2 is marked as changed.
 *
 * @param name name
 * @param value value, "" to remove, QString::null to do nothing
 * @param type frame type
 */
void OggFile::setTextField(const QString& name, const QString& value,
                           Frame::Type type)
{
  if (m_fileRead && !value.isNull() &&
      m_comments.setValue(name, value)) {
    markTagChanged(Frame::Tag_2, type);
  }
}

/**
 * Check if tag information has already been read.
 *
 * @return true if information is available,
 *         false if the tags have not been read yet, in which case
 *         hasTag() does not return meaningful information.
 */
bool OggFile::isTagInformationRead() const
{
  return m_fileRead;
}

/**
 * Check if file has a tag.
 *
 * @param tagNr tag number
 * @return true if a tag is available.
 * @see isTagInformationRead()
 */
bool OggFile::hasTag(Frame::TagNumber tagNr) const
{
  return tagNr == Frame::Tag_2 && !m_comments.empty();
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".ogg".
 */
QString OggFile::getFileExtension() const
{
  return QLatin1String(".ogg");
}

#ifdef HAVE_VORBIS
/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void OggFile::getDetailInfo(DetailInfo& info) const
{
  if (m_fileRead && m_fileInfo.valid) {
    info.valid = true;
    info.format = QLatin1String("Ogg Vorbis");
    info.bitrate = m_fileInfo.bitrate / 1000;
    info.sampleRate = m_fileInfo.sampleRate;
    info.channels = m_fileInfo.channels;
    info.duration = m_fileInfo.duration;
  } else {
    info.valid = false;
  }
}

/**
 * Get duration of file.
 *
 * @return duration in seconds,
 *         0 if unknown.
 */
unsigned OggFile::getDuration() const
{
  if (m_fileRead && m_fileInfo.valid) {
    return m_fileInfo.duration;
  }
  return 0;
}
#else // HAVE_VORBIS
void OggFile::getDetailInfo(DetailInfo& info) const { info.valid = false; }
unsigned OggFile::getDuration() const { return 0; }
#endif // HAVE_VORBIS

/**
 * Get the format of tag.
 *
 * @param tagNr tag number
 * @return "Vorbis".
 */
QString OggFile::getTagFormat(Frame::TagNumber tagNr) const
{
  return hasTag(tagNr) ? QLatin1String("Vorbis") : QString();
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
bool OggFile::getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const
{
  if (type < Frame::FT_FirstFrame || type > Frame::FT_LastV1Frame ||
      tagNr > 1)
    return false;

  if (tagNr == Frame::Tag_1) {
    frame.setValue(QString());
  } else {
    frame.setValue(getTextField(
                     QString::fromLatin1(getVorbisNameFromType(type))));
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
bool OggFile::setFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    if (frame.getType() == Frame::FT_Track) {
      int numTracks = getTotalNumberOfTracksIfEnabled();
      if (numTracks > 0) {
        QString numTracksStr = QString::number(numTracks);
        formatTrackNumberIfEnabled(numTracksStr, false);
        if (getTextField(QLatin1String("TRACKTOTAL")) != numTracksStr) {
          setTextField(QLatin1String("TRACKTOTAL"), numTracksStr, Frame::FT_Other);
          markTagChanged(Frame::Tag_2, Frame::FT_Other);
        }
      }
    }

    // If the frame has an index, change that specific frame
    int index = frame.getIndex();
    if (index != -1 && index < static_cast<int>(m_comments.size())) {
      QString value = frame.getValue();
      if (frame.getType() == Frame::FT_Picture) {
        Frame newFrame(frame);
        PictureFrame::setDescription(newFrame, value);
        PictureFrame::getFieldsToBase64(newFrame, value);
        if (!value.isEmpty() && frame.getInternalName() == QLatin1String("COVERART")) {
          QString mimeType;
          PictureFrame::getMimeType(frame, mimeType);
          setTextField(QLatin1String("COVERARTMIME"), mimeType, Frame::FT_Other);
        }
      } else if (frame.getType() == Frame::FT_Track) {
        formatTrackNumberIfEnabled(value, false);
      }
      if (m_comments[index].getValue() != value) {
        m_comments[index].setValue(value);
        markTagChanged(Frame::Tag_2, frame.getType());
      }
      return true;
    }
  }

  // Try the basic method
  Frame::Type type = frame.getType();
  if (type < Frame::FT_FirstFrame || type > Frame::FT_LastV1Frame ||
      tagNr > 1)
    return false;

  if (tagNr == Frame::Tag_2) {
    if (type == Frame::FT_Track) {
      int numTracks;
      int num = splitNumberAndTotal(frame.getValue(), &numTracks);
      if (num >= 0) {
        QString str;
        if (num != 0) {
          str.setNum(num);
          formatTrackNumberIfEnabled(str, false);
        } else {
          str = QLatin1String("");
        }
        setTextField(QLatin1String("TRACKNUMBER"), str, Frame::FT_Track);
        if (numTracks > 0) {
          str.setNum(numTracks);
          formatTrackNumberIfEnabled(str, false);
          setTextField(QLatin1String("TRACKTOTAL"), str, Frame::FT_Other);
        }
      }
    } else {
      setTextField(type == Frame::FT_Comment
                   ? getCommentFieldName()
                   : QString::fromLatin1(getVorbisNameFromType(type)),
                   frame.getValue(), type);
    }
  }
  return true;
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add
 *
 * @return true if ok.
 */
bool OggFile::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    // Add a new frame.
    QString name(getVorbisName(frame));
    QString value(frame.getValue());
    if (frame.getType() == Frame::FT_Picture) {
      if (frame.getFieldList().empty()) {
        PictureFrame::setFields(
          frame, Frame::TE_ISO8859_1, QLatin1String(""), QLatin1String("image/jpeg"),
          PictureFrame::PT_CoverFront, QLatin1String(""), QByteArray());
      }
      frame.setExtendedType(Frame::ExtendedType(Frame::FT_Picture, name));
      PictureFrame::getFieldsToBase64(frame, value);
    }
    m_comments.push_back(OggFile::CommentField(name, value));
    frame.setExtendedType(Frame::ExtendedType(frame.getType(), name));
    frame.setIndex(m_comments.size() - 1);
    markTagChanged(Frame::Tag_2, frame.getType());
    return true;
  }
  return false;
}

/**
 * Delete a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool OggFile::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    // If the frame has an index, delete that specific frame
    int index = frame.getIndex();
    if (index != -1 && index < static_cast<int>(m_comments.size())) {
      m_comments.removeAt(index);
      markTagChanged(Frame::Tag_2, frame.getType());
      return true;
    }
  }

  // Try the superclass method
  return TaggedFile::deleteFrame(tagNr, frame);
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void OggFile::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  if (tagNr == Frame::Tag_2) {
    frames.clear();
    QString name;
    int i = 0;
    for (auto it = m_comments.constBegin(); it != m_comments.constEnd(); ++it) {
      name = (*it).getName();
      Frame::Type type = getTypeFromVorbisName(name);
      if (type == Frame::FT_Picture) {
        Frame frame(type, QLatin1String(""), name, i++);
        PictureFrame::setFieldsFromBase64(frame, (*it).getValue());
        if (name == QLatin1String("COVERART")) {
          PictureFrame::setMimeType(frame, getTextField(QLatin1String("COVERARTMIME")));
        }
        frames.insert(frame);
      } else {
        frames.insert(Frame(type, (*it).getValue(), name, i++));
      }
    }
    updateMarkedState(tagNr, frames);
    frames.addMissingStandardFrames();
    return;
  }

  TaggedFile::getAllFrames(tagNr, frames);
}

/**
 * Get a list of frame IDs which can be added.
 * @param tagNr tag number
 * @return list with frame IDs.
 */
QStringList OggFile::getFrameIds(Frame::TagNumber tagNr) const
{
  if (tagNr != Frame::Tag_2)
    return QStringList();

  static const char* const fieldNames[] = {
    "CONTACT",
    "DESCRIPTION",
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

  QStringList lst;
  lst.reserve(Frame::FT_LastFrame - Frame::FT_FirstFrame + 1 +
              sizeof(fieldNames) / sizeof(fieldNames[0]));
  for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
    lst.append(Frame::ExtendedType(static_cast<Frame::Type>(k),
                                   QLatin1String("")).getName());
  }
  for (auto fieldName : fieldNames) {
    lst.append(QString::fromLatin1(fieldName));
  }
  return lst;
}

#ifdef HAVE_VORBIS
/**
 * Read information about an Ogg/Vorbis file.
 * @param info file info to fill
 * @param fn file name
 * @return true if ok.
 */
bool OggFile::readFileInfo(FileInfo& info, const QString& fn) const
{
  static ::ov_callbacks ovcb = {
    oggread, oggseek, oggclose, oggtell
  };
  info.valid = false;
  QFile fp(fn);
  if (fp.open(QIODevice::ReadOnly)) {
    OggVorbis_File vf;
    if (::ov_open_callbacks(&fp, &vf, nullptr, 0, ovcb) == 0) {
      vorbis_info* vi = ::ov_info(&vf, -1);
      if (vi) {
        info.valid = true;
        info.version = vi->version;
        info.channels = vi->channels;
        info.sampleRate = vi->rate;
        info.bitrate = vi->bitrate_nominal;
        if (info.bitrate <= 0) {
          info.bitrate = vi->bitrate_upper;
        }
        if (info.bitrate <= 0) {
          info.bitrate = vi->bitrate_lower;
        }
      }
      info.duration = static_cast<long>(::ov_time_total(&vf, -1));
      ::ov_clear(&vf); // closes file, do not use ::fclose()
    } else {
      fp.close();
    }
  }
  return info.valid;
}
#endif // HAVE_VORBIS

/**
 * Get value.
 * @param name name
 * @return value, "" if not found.
 */
QString OggFile::CommentList::getValue(const QString& name) const
{
  for (const_iterator it = begin(); it != end(); ++it) {
    if ((*it).getName() == name) {
      return (*it).getValue();
    }
  }
  return QLatin1String("");
}

/**
 * Set value.
 * @param name name
 * @param value value
 * @return true if value was changed.
 */
bool OggFile::CommentList::setValue(const QString& name, const QString& value)
{
  for (iterator it = begin(); it != end(); ++it) {
    if ((*it).getName() == name) {
      QString oldValue = (*it).getValue();
      if (value != oldValue) {
        (*it).setValue(value);
        return true;
      } else {
        return false;
      }
    }
  }
  if (!value.isEmpty()) {
    CommentField cf(name, value);
    push_back(cf);
    return true;
  } else {
    return false;
  }
}
