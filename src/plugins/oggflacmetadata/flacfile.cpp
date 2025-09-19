/**
 * \file flacfile.cpp
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 *
 * Copyright (C) 2005-2024  Urs Fleisch
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

#include "flacfile.hpp"

#include "genres.h"
#include "pictureframe.h"
#include <FLAC++/metadata.h>
#include <QFile>
#include <QDir>
#include <cstdio>
#include <cmath>
#include <QByteArray>

namespace {

#ifdef HAVE_FLAC_PICTURE
/**
 * Get the picture block as a picture frame.
 *
 * @param frame frame to set
 * @param pic   picture block to get
 */
void getPicture(Frame& frame, const FLAC::Metadata::Picture* pic)
{
  QByteArray ba(reinterpret_cast<const char*>(pic->get_data()),
    pic->get_data_length());
  PictureFrame::ImageProperties imgProps(
        pic->get_width(), pic->get_height(), pic->get_depth(),
        pic->get_colors(), ba);
  PictureFrame::setFields(
    frame,
    Frame::TE_ISO8859_1, QLatin1String(""),
    QString::fromLatin1(pic->get_mime_type()),
    static_cast<PictureFrame::PictureType>(pic->get_type()),
    QString::fromUtf8(
      reinterpret_cast<const char*>(pic->get_description())),
    ba, &imgProps);
  frame.setExtendedType(Frame::ExtendedType(Frame::FT_Picture, QLatin1String("Picture")));
}

/**
 * Set the picture block with the picture frame.
 *
 * @param frame frame to get
 * @param pic picture block to set
 *
 * @return true if ok.
 */
bool setPicture(const Frame& frame, FLAC::Metadata::Picture* pic)
{
  Frame::TextEncoding enc;
  PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
  QString imgFormat, mimeType, description;
  QByteArray ba;
  PictureFrame::ImageProperties imgProps;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType,
                          pictureType, description, ba, &imgProps);
  if (!imgProps.isValidForImage(ba)) {
    imgProps = PictureFrame::ImageProperties(ba);
  }
  pic->set_width(imgProps.width());
  pic->set_height(imgProps.height());
  pic->set_depth(imgProps.depth());
  pic->set_colors(imgProps.numColors());
  pic->set_mime_type(mimeType.toLatin1());
  pic->set_type(
    static_cast<FLAC__StreamMetadata_Picture_Type>(pictureType));
  pic->set_description(
    reinterpret_cast<const FLAC__byte*>(
      static_cast<const char*>(description.toUtf8())));
  const auto data = reinterpret_cast<const FLAC__byte*>(ba.data());
  int dataLength = ba.size();
  if (!data || dataLength <= 0) {
    // Avoid assertion crash in FLAC__metadata_object_picture_set_data().
    qWarning("FLAC picture data empty");
    return false;
  }
  pic->set_data(data, dataLength);
  if (pic->get_length() >= (1u << FLAC__STREAM_METADATA_LENGTH_LEN)) {
    // Avoid assertion crash in FLAC write_metadata_block_header_cb_().
    qWarning("FLAC picture is too large");
    return false;
  }
  return true;
}
#endif // HAVE_FLAC_PICTURE

}

/**
 * Constructor.
 *
 * @param idx index in file proxy model
 */
FlacFile::FlacFile(const QPersistentModelIndex& idx) : OggFile(idx)
{
}

/**
 * Destructor.
 */
FlacFile::~FlacFile()
{
  // Must not be inline because of forward declared QScopedPointer.
}

/**
 * Get key of tagged file format.
 * @return "FlacMetadata".
 */
QString FlacFile::taggedFileKey() const
{
  return QLatin1String("FlacMetadata");
}

/**
 * Read tags from file.
 *
 * @param force true to force reading even if tags were already read.
 */
void FlacFile::readTags(bool force)
{
  bool priorIsTagInformationRead = isTagInformationRead();
  if (force || !m_fileRead) {
    m_comments.clear();
    markTagUnchanged(Frame::Tag_2);
    m_fileRead = true;
    QByteArray fnIn = QFile::encodeName(currentFilePath());
    readFileInfo(m_fileInfo, nullptr); // just to start invalid
    if (!m_chain) {
      m_chain.reset(new FLAC::Metadata::Chain);
    }
    if (m_chain && m_chain->is_valid()) {
      if (m_chain->read(fnIn)) {
#ifdef HAVE_FLAC_PICTURE
        m_pictures.clear();
        int pictureNr = 0;
#endif
        FLAC::Metadata::Iterator mdit;
        mdit.init(*m_chain);
        while (mdit.is_valid()) {
          if (::FLAC__MetadataType mdt = mdit.get_block_type();
              mdt == FLAC__METADATA_TYPE_STREAMINFO) {
            if (FLAC::Metadata::Prototype* proto = mdit.get_block()) {
              auto si =
                dynamic_cast<FLAC::Metadata::StreamInfo*>(proto);
              readFileInfo(m_fileInfo, si);
              delete proto;
            }
          } else if (mdt == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
            if (FLAC::Metadata::Prototype* proto = mdit.get_block()) {
              if (auto vc =
                    dynamic_cast<FLAC::Metadata::VorbisComment*>(proto);
                  vc && vc->is_valid()) {
                unsigned numComments = vc->get_num_comments();
                for (unsigned i = 0; i < numComments; ++i) {
                  if (FLAC::Metadata::VorbisComment::Entry entry =
                        vc->get_comment(i);
                      entry.is_valid()) {
                    QString name =
                      QString::fromUtf8(entry.get_field_name(),
                                        entry.get_field_name_length())
                        .trimmed().toUpper();
                    if (QString value =
                          QString::fromUtf8(entry.get_field_value(),
                            entry.get_field_value_length()).trimmed();
                        !value.isEmpty()) {
                      m_comments.push_back(
                        CommentField(name, value));
                    }
                  }
                }
              }
              delete proto;
            }
          }
#ifdef HAVE_FLAC_PICTURE
          else if (mdt == FLAC__METADATA_TYPE_PICTURE) {
            if (FLAC::Metadata::Prototype* proto = mdit.get_block()) {
              if (auto pic =
                  dynamic_cast<FLAC::Metadata::Picture*>(proto)) {
                Frame frame(Frame::FT_Picture, QLatin1String(""),
                            QLatin1String(""), Frame::toNegativeIndex(pictureNr++));
                getPicture(frame, pic);
                m_pictures.push_back(frame);
              }
              delete proto;
            }
          }
#endif
          if (!mdit.next()) {
            break;
          }
        }
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
bool FlacFile::writeTags(bool force, bool* renamed, bool preserve)
{
  if (isChanged() &&
    !QFileInfo(currentFilePath()).isWritable()) {
    revertChangedFilename();
    return false;
  }

  if (m_fileRead && (force || isTagChanged(Frame::Tag_2)) && m_chain && m_chain->is_valid()) {
    bool commentsSet = false;
#ifdef HAVE_FLAC_PICTURE
    bool pictureSet = false;
    bool pictureRemoved = false;
    auto pictureIt = m_pictures.begin(); // clazy:exclude=detaching-member
#endif

    if (FLAC::Metadata::Chain::Status status = m_chain->status();
        status == FLAC__METADATA_CHAIN_STATUS_NOT_A_FLAC_FILE ||
        status == FLAC__METADATA_CHAIN_STATUS_ERROR_OPENING_FILE) {
      // This check is done because of a crash in mdit.get_block_type() with an
      // empty file with flac extension. m_chain->status() will set the status
      // to FLAC__METADATA_CHAIN_STATUS_OK (!?), so we have to delete the
      // chain to avoid a crash with the next call to writeTags().
      m_chain.reset();
      return false;
    }

    m_chain->sort_padding();
    FLAC::Metadata::Iterator mdit;
    mdit.init(*m_chain);
    while (mdit.is_valid()) {
      if (::FLAC__MetadataType mdt = mdit.get_block_type();
          mdt == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        if (commentsSet) {
          mdit.delete_block(true);
        } else {
          if (FLAC::Metadata::Prototype* proto = mdit.get_block()) {
            if (auto vc =
                  dynamic_cast<FLAC::Metadata::VorbisComment*>(proto);
                vc && vc->is_valid()) {
              setVorbisComment(vc);
              commentsSet = true;
            }
            delete proto;
          }
        }
      }
#ifdef HAVE_FLAC_PICTURE
      else if (mdt == FLAC__METADATA_TYPE_PICTURE) {
        if (pictureIt != m_pictures.end()) {
          if (FLAC::Metadata::Prototype* proto = mdit.get_block()) {
            if (auto pic = dynamic_cast<FLAC::Metadata::Picture*>(proto)) {
              if (setPicture(*pictureIt++, pic)) {
                pictureSet = true;
              } else {
                mdit.delete_block(false);
                pictureRemoved = true;
              }
            }
            delete proto;
          }
        } else {
          mdit.delete_block(false);
          pictureRemoved = true;
        }
      } else if (mdt == FLAC__METADATA_TYPE_PADDING) {
        if (pictureIt != m_pictures.end()) {
          if (auto pic = new FLAC::Metadata::Picture;
              setPicture(*pictureIt, pic) && mdit.set_block(pic)) {
            ++pictureIt;
            pictureSet = true;
          } else {
            delete pic;
          }
        } else if (pictureRemoved) {
          mdit.delete_block(false);
        }
      }
#endif
      if (!mdit.next()) {
        if (!commentsSet) {
          auto vc = new FLAC::Metadata::VorbisComment;
          if (vc->is_valid()) {
            setVorbisComment(vc);
            if (mdit.insert_block_after(vc)) {
              commentsSet = true;
            }
          }
          if (!commentsSet) {
            delete vc;
          }
        }
#ifdef HAVE_FLAC_PICTURE
        while (pictureIt != m_pictures.end()) {
          if (auto pic = new FLAC::Metadata::Picture;
              setPicture(*pictureIt, pic) && mdit.insert_block_after(pic)) {
            pictureSet = true;
          } else {
            delete pic;
          }
          ++pictureIt;
        }
#endif
        break;
      }
    }
#ifdef HAVE_FLAC_PICTURE
    if ((commentsSet || pictureSet) &&
        m_chain->write(!pictureRemoved, preserve)) {
      markTagUnchanged(Frame::Tag_2);
    }
#else
    if (commentsSet &&
        m_chain->write(true, preserve)) {
      markTagUnchanged(Frame::Tag_2);
    }
#endif
    else {
      return false;
    }
  }
  if (isFilenameChanged()) {
    if (!renameFile()) {
      return false;
    }
    markFilenameUnchanged();
    // link tags to new file name
    readTags(true);
    *renamed = true;
  }
  return true;
}

/**
 * Free resources allocated when calling readTags().
 *
 * @param force true to force clearing even if the tags are modified
 */
void FlacFile::clearTags(bool force)
{
  if (!m_fileRead || (isChanged() && !force))
    return;

  bool priorIsTagInformationRead = isTagInformationRead();
  if (m_chain) {
    m_chain.reset();
  }
#ifdef HAVE_FLAC_PICTURE
  m_pictures.clear();
#endif
  m_comments.clear();
  markTagUnchanged(Frame::Tag_2);
  m_fileRead = false;
  notifyModelDataChanged(priorIsTagInformationRead);
}

#ifdef HAVE_FLAC_PICTURE
/**
 * Check if file has a tag.
 *
 * @param tagNr tag number
 * @return true if a tag is available.
 * @see isTagInformationRead()
 */
bool FlacFile::hasTag(Frame::TagNumber tagNr) const
{
  return tagNr == Frame::Tag_2 && (OggFile::hasTag(Frame::Tag_2) || !m_pictures.empty());
}

/**
 * Set a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool FlacFile::setFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    if (Frame::ExtendedType extendedType = frame.getExtendedType();
        extendedType.getType() == Frame::FT_Picture) {
      if (int index = Frame::fromNegativeIndex(frame.getIndex());
          index >= 0 && index < m_pictures.size()) {
        if (auto it = m_pictures.begin() + index; it != m_pictures.end()) {
          Frame newFrame(frame);
          PictureFrame::setDescription(newFrame, frame.getValue());
          if (PictureFrame::areFieldsEqual(*it, newFrame)) {
            it->setValueChanged(false);
          } else {
            *it = newFrame;
            markTagChanged(Frame::Tag_2, extendedType);
          }
          return true;
        }
      }
    }
  }
  return OggFile::setFrame(tagNr, frame);
}

/**
 * Add a frame in the tags.
 *
 * @param tagNr tag number
 * @param frame frame to add
 *
 * @return true if ok.
 */
bool FlacFile::addFrame(Frame::TagNumber tagNr, Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    if (Frame::ExtendedType extendedType = frame.getExtendedType();
        extendedType.getType() == Frame::FT_Picture) {
      if (frame.getFieldList().empty()) {
        PictureFrame::setFields(
              frame, Frame::TE_ISO8859_1, QLatin1String("JPG"),
              QLatin1String("image/jpeg"), PictureFrame::PT_CoverFront,
              QLatin1String(""), QByteArray());
      }
      PictureFrame::setDescription(frame, frame.getValue());
      frame.setIndex(Frame::toNegativeIndex(m_pictures.size()));
      m_pictures.push_back(frame);
      markTagChanged(Frame::Tag_2, extendedType);
      return true;
    }
  }
  return OggFile::addFrame(tagNr, frame);
}

/**
 * Delete a frame from the tags.
 *
 * @param tagNr tag number
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool FlacFile::deleteFrame(Frame::TagNumber tagNr, const Frame& frame)
{
  if (tagNr == Frame::Tag_2) {
    if (Frame::ExtendedType extendedType = frame.getExtendedType();
        extendedType.getType() == Frame::FT_Picture) {
      if (int index = Frame::fromNegativeIndex(frame.getIndex());
          index >= 0 && index < m_pictures.size()) {
        m_pictures.removeAt(index);
        markTagChanged(Frame::Tag_2, extendedType);
        return true;
      }
    }
  }
  return OggFile::deleteFrame(tagNr, frame);
}

/**
 * Remove frames.
 *
 * @param tagNr tag number
 * @param flt filter specifying which frames to remove
 */
void FlacFile::deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt)
{
  if (tagNr != Frame::Tag_2)
    return;

  if (flt.areAllEnabled() || flt.isEnabled(Frame::FT_Picture)) {
    m_pictures.clear();
    markTagChanged(Frame::Tag_2, Frame::ExtendedType(Frame::FT_Picture));
  }
  OggFile::deleteFrames(tagNr, flt);
}

/**
 * Get all frames in tag.
 *
 * @param tagNr tag number
 * @param frames frame collection to set.
 */
void FlacFile::getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames)
{
  OggFile::getAllFrames(tagNr, frames);
  if (tagNr == Frame::Tag_2) {
    int i = 0;
    for (auto it = m_pictures.begin(); it != m_pictures.end(); ++it) { // clazy:exclude=detaching-member
      it->setIndex(Frame::toNegativeIndex(i++));
      frames.insert(*it);
    }
    updateMarkedState(tagNr, frames);
  }
}
#endif // HAVE_FLAC_PICTURE

/**
 * Set the vorbis comment block with the comments.
 *
 * @param vc vorbis comment block to set
 */
void FlacFile::setVorbisComment(FLAC::Metadata::VorbisComment* vc)
{
  // first all existing comments are deleted
#ifndef HAVE_NO_FLAC_STREAMMETADATA_OPERATOR
  // the C++ API is not complete
  const ::FLAC__StreamMetadata* fsmd = *vc;
  FLAC__metadata_object_vorbiscomment_resize_comments(
    const_cast<FLAC__StreamMetadata*>(fsmd), 0);
#else
  const unsigned numComments = vc->get_num_comments();
  for (unsigned i = 0; i < numComments; ++i) {
    vc->delete_comment(0);
  }
#endif
  // then our comments are appended
  auto it = m_comments.begin(); // clazy:exclude=detaching-member
  while (it != m_comments.end()) {
    QString name = fixUpTagKey(it->getName(), TT_Vorbis);
    if (QString value(it->getValue()); !value.isEmpty()) {
      // The number of bytes - not characters - has to be passed to the
      // Entry constructor, thus qstrlen is used.
      QByteArray valueCStr = value.toUtf8();
      vc->insert_comment(vc->get_num_comments(),
        FLAC::Metadata::VorbisComment::Entry(
          name.toLatin1().data(), valueCStr, qstrlen(valueCStr)));
      ++it;
    } else {
      it = m_comments.erase(it);
    }
  }
}

/**
 * Get technical detail information.
 *
 * @param info the detail information is returned here
 */
void FlacFile::getDetailInfo(DetailInfo& info) const
{
  if (m_fileRead && m_fileInfo.valid) {
    info.valid = true;
    info.format = QLatin1String("FLAC");
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
unsigned FlacFile::getDuration() const
{
  if (m_fileRead && m_fileInfo.valid) {
    return m_fileInfo.duration;
  }
  return 0;
}

/**
 * Get file extension including the dot.
 *
 * @return file extension ".flac".
 */
QString FlacFile::getFileExtension() const
{
  return QLatin1String(".flac");
}


/**
 * Read information about a FLAC file.
 * @param info file info to fill
 * @param si stream info
 * @return true if ok.
 */
bool FlacFile::readFileInfo(FileInfo& info,
                            const FLAC::Metadata::StreamInfo* si) const
{
  if (si && si->is_valid()) {
    info.valid = true;
    info.channels = si->get_channels();
    info.sampleRate = si->get_sample_rate();
    info.duration = info.sampleRate != 0
        ? si->get_total_samples() / info.sampleRate : 0;
    info.bitrate = si->get_bits_per_sample() * info.sampleRate;
  } else {
    info.valid = false;
  }
  return info.valid;
}
