/**
 * \file flacfile.cpp
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 *
 * Copyright (C) 2005-2013  Urs Fleisch
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
#include <QImage>
#include <sys/stat.h>
#ifdef Q_OS_WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <cstdio>
#include <cmath>
#include <QByteArray>

/**
 * Constructor.
 *
 * @param dn directory name
 * @param fn filename
 * @param idx model index
 */
FlacFile::FlacFile(const QString& dn, const QString& fn,
                   const QPersistentModelIndex& idx) :
  OggFile(dn, fn, idx), m_chain(0)
{
}

/**
 * Destructor.
 */
FlacFile::~FlacFile()
{
  if (m_chain) {
    delete m_chain;
  }
}

#ifdef HAVE_FLAC_PICTURE
/**
 * Get the picture block as a picture frame.
 *
 * @param frame frame to set
 * @param pic   picture block to get
 */
static void getPicture(Frame& frame, const FLAC::Metadata::Picture* pic)
{
  QByteArray ba(reinterpret_cast<const char*>(pic->get_data()),
    pic->get_data_length());
  PictureFrame::setFields(
    frame,
    Frame::Field::TE_ISO8859_1, QLatin1String(""),
    QString::fromLatin1(pic->get_mime_type()),
    static_cast<PictureFrame::PictureType>(pic->get_type()),
    QString::fromUtf8(
      reinterpret_cast<const char*>(pic->get_description())),
    ba);
  frame.setExtendedType(Frame::ExtendedType(Frame::FT_Picture, QLatin1String("Picture")));
}

/**
 * Set the picture block with the picture frame.
 *
 * @param frame frame to get
 * @param pic picture block to set
 */
static void setPicture(const Frame& frame, FLAC::Metadata::Picture* pic)
{
  Frame::Field::TextEncoding enc;
  PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
  QString imgFormat, mimeType, description;
  QByteArray ba;
  PictureFrame::getFields(frame, enc, imgFormat, mimeType,
                          pictureType, description, ba);
  QImage image;
  if (image.loadFromData(ba)) {
    pic->set_width(image.width());
    pic->set_height(image.height());
    pic->set_depth(image.depth());
    pic->set_colors(image.colorCount());
  }
  pic->set_mime_type(mimeType.toLatin1());
  pic->set_type(
    static_cast<FLAC__StreamMetadata_Picture_Type>(pictureType));
  pic->set_description(
    reinterpret_cast<const FLAC__byte*>(
      static_cast<const char*>(description.toUtf8())));
  pic->set_data(reinterpret_cast<const FLAC__byte*>(ba.data()), ba.size());
}
#endif // HAVE_FLAC_PICTURE

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
  if (force || !m_fileRead) {
    m_comments.clear();
    markTag2Unchanged();
    m_fileRead = true;
    QByteArray fnIn = QFile::encodeName(getDirname() + QDir::separator() + currentFilename());
    m_fileInfo.read(0); // just to start invalid
    if (!m_chain) {
      m_chain = new FLAC::Metadata::Chain;
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
          ::FLAC__MetadataType mdt = mdit.get_block_type();
          if (mdt == FLAC__METADATA_TYPE_STREAMINFO) {
            FLAC::Metadata::Prototype* proto = mdit.get_block();
            if (proto) {
              FLAC::Metadata::StreamInfo* si =
                dynamic_cast<FLAC::Metadata::StreamInfo*>(proto);
              m_fileInfo.read(si);
              delete proto;
            }
          } else if (mdt == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
            FLAC::Metadata::Prototype* proto = mdit.get_block();
            if (proto) {
              FLAC::Metadata::VorbisComment* vc =
                dynamic_cast<FLAC::Metadata::VorbisComment*>(proto);
              if (vc && vc->is_valid()) {
                unsigned numComments = vc->get_num_comments();
                for (unsigned i = 0; i < numComments; ++i) {
                  FLAC::Metadata::VorbisComment::Entry entry =
                    vc->get_comment(i);
                  if (entry.is_valid()) {
                    QString name =
                      QString::fromUtf8(entry.get_field_name(),
                                        entry.get_field_name_length()).
                      trimmed().toUpper();
                    QString value =
                      QString::fromUtf8(entry.get_field_value(),
                                        entry.get_field_value_length()).
                      trimmed();
                    if (!value.isEmpty()) {
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
            FLAC::Metadata::Prototype* proto = mdit.get_block();
            if (proto) {
              FLAC::Metadata::Picture* pic =
                dynamic_cast<FLAC::Metadata::Picture*>(proto);
              if (pic) {
                Frame frame(Frame::FT_Picture, QLatin1String(""), QLatin1String(""), pictureNr++);
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
    !QFileInfo(getDirname() + QDir::separator() + currentFilename()).isWritable()) {
    return false;
  }

  if (m_fileRead && (force || isTag2Changed()) && m_chain && m_chain->is_valid()) {
    bool commentsSet = false;
#ifdef HAVE_FLAC_PICTURE
    bool pictureSet = false;
    bool pictureRemoved = false;
    PictureList::iterator pictureIt = m_pictures.begin();
#endif

    if (m_chain->status() == FLAC__METADATA_CHAIN_STATUS_NOT_A_FLAC_FILE) {
      // This check is done because of a crash in mdit.get_block_type() with an
      // empty file with flac extension. m_chain->status() will set the status
      // to FLAC__METADATA_CHAIN_STATUS_OK (!?), so we have to delete the
      // chain to avoid a crash with the next call to writeTags().
      delete m_chain;
      m_chain = 0;
      return false;
    }

    m_chain->sort_padding();
    FLAC::Metadata::Iterator mdit;
    mdit.init(*m_chain);
    while (mdit.is_valid()) {
      ::FLAC__MetadataType mdt = mdit.get_block_type();
      if (mdt == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        if (commentsSet) {
          mdit.delete_block(true);
        } else {
          FLAC::Metadata::Prototype* proto = mdit.get_block();
          if (proto) {
            FLAC::Metadata::VorbisComment* vc =
              dynamic_cast<FLAC::Metadata::VorbisComment*>(proto);
            if (vc && vc->is_valid()) {
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
          FLAC::Metadata::Prototype* proto = mdit.get_block();
          if (proto) {
            FLAC::Metadata::Picture* pic =
              dynamic_cast<FLAC::Metadata::Picture*>(proto);
            if (pic) {
              setPicture(*pictureIt++, pic);
              pictureSet = true;
            }
            delete proto;
          }
        } else {
          mdit.delete_block(false);
          pictureRemoved = true;
        }
      } else if (mdt == FLAC__METADATA_TYPE_PADDING) {
        if (pictureIt != m_pictures.end()) {
          FLAC::Metadata::Picture* pic = new FLAC::Metadata::Picture;
          setPicture(*pictureIt, pic);
          if (mdit.set_block(pic)) {
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
          FLAC::Metadata::VorbisComment* vc = new FLAC::Metadata::VorbisComment;
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
          FLAC::Metadata::Picture* pic = new FLAC::Metadata::Picture;
          setPicture(*pictureIt, pic);
          if (mdit.insert_block_after(pic)) {
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
      markTag2Unchanged();
    }
#else
    if (commentsSet &&
        m_chain->write(true, preserve)) {
      markTag2Unchanged();
    }
#endif
    else {
      return false;
    }
  }
  if (getFilename() != currentFilename()) {
    if (!renameFile(currentFilename(), getFilename())) {
      return false;
    }
    updateCurrentFilename();
    // link tags to new file name
    readTags(true);
    *renamed = true;
  }
  return true;
}

#ifdef HAVE_FLAC_PICTURE
/**
 * Check if file has an ID3v2 tag.
 *
 * @return true if a V2 tag is available.
 * @see isTagInformationRead()
 */
bool FlacFile::hasTagV2() const
{
  return OggFile::hasTagV2() || !m_pictures.empty();
}

/**
 * Set a frame in the tags 2.
 *
 * @param frame frame to set
 *
 * @return true if ok.
 */
bool FlacFile::setFrameV2(const Frame& frame)
{
  if (frame.getType() == Frame::FT_Picture) {
    int index = frame.getIndex();
    if (index != -1 && index < static_cast<int>(m_pictures.size())) {
      PictureList::iterator it = m_pictures.begin() + index;
      if (it != m_pictures.end()) {
        Frame newFrame(frame);
        PictureFrame::setDescription(newFrame, frame.getValue());
        if (PictureFrame::areFieldsEqual(*it, newFrame)) {
          it->setValueChanged(false);
        } else {
          *it = newFrame;
          markTag2Changed(Frame::FT_Picture);
        }
        return true;
      }
    }
  }
  return OggFile::setFrameV2(frame);
}

/**
 * Add a frame in the tags 2.
 *
 * @param frame frame to add
 *
 * @return true if ok.
 */
bool FlacFile::addFrameV2(Frame& frame)
{
  if (frame.getType() == Frame::FT_Picture) {
    if (frame.getFieldList().empty()) {
      PictureFrame::setFields(
        frame, Frame::Field::TE_ISO8859_1, QLatin1String("JPG"), QLatin1String("image/jpeg"),
        PictureFrame::PT_CoverFront, QLatin1String(""), QByteArray());
    }
    PictureFrame::setDescription(frame, frame.getValue());
    frame.setIndex(m_pictures.size());
    m_pictures.push_back(frame);
    markTag2Changed(Frame::FT_Picture);
    return true;
  }
  return OggFile::addFrameV2(frame);
}

/**
 * Delete a frame in the tags 2.
 *
 * @param frame frame to delete.
 *
 * @return true if ok.
 */
bool FlacFile::deleteFrameV2(const Frame& frame)
{
  if (frame.getType() == Frame::FT_Picture) {
    int index = frame.getIndex();
    if (index != -1 && index < static_cast<int>(m_pictures.size())) {
      m_pictures.removeAt(index);
      markTag2Changed(Frame::FT_Picture);
      return true;
    }
  }
  return OggFile::deleteFrameV2(frame);
}

/**
 * Remove ID3v2 frames.
 *
 * @param flt filter specifying which frames to remove
 */
void FlacFile::deleteFramesV2(const FrameFilter& flt)
{
  if (flt.areAllEnabled() || flt.isEnabled(Frame::FT_Picture)) {
    m_pictures.clear();
    markTag2Changed(Frame::FT_Picture);
  }
  OggFile::deleteFramesV2(flt);
}

/**
 * Get all frames in tag 2.
 *
 * @param frames frame collection to set.
 */
void FlacFile::getAllFramesV2(FrameCollection& frames)
{
  OggFile::getAllFramesV2(frames);
  int i = 0;
  for (PictureList::iterator it = m_pictures.begin();
       it != m_pictures.end();
       ++it) {
    (*it).setIndex(i++);
    frames.insert(*it);
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
  CommentList::iterator it = m_comments.begin();
  while (it != m_comments.end()) {
    QString name((*it).getName());
    QString value((*it).getValue());
    if (!value.isEmpty()) {
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
 * @param fn file name
 * @return true if ok.
 */
bool FlacFile::FileInfo::read(FLAC::Metadata::StreamInfo* si)
{
  if (si && si->is_valid()) {
    valid = true;
    channels = si->get_channels();
    sampleRate = si->get_sample_rate();
    duration = si->get_total_samples() / sampleRate;
    bitrate = si->get_bits_per_sample() * sampleRate;
  } else {
    valid = false;
  }
  return valid;
}
