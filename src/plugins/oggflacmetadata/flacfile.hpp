/**
 * \file flacfile.hpp
 * Handling of FLAC files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Oct 2005
 *
 * Copyright (C) 2005-2011  Urs Fleisch
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

#ifndef FLACFILE_H
#define FLACFILE_H

#include "oggflacconfig.h"
#include "oggfile.hpp"

namespace FLAC {
  namespace Metadata {
    class Chain;
    class VorbisComment;
    class StreamInfo;
    class Picture;
  }
}

 /** List box item containing FLAC file */
class FlacFile : public OggFile {
public:
  /**
   * Constructor.
   *
   * @param idx index in file proxy model
   */
  explicit FlacFile(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~FlacFile();

  /**
   * Get key of tagged file format.
   * @return "FlacMetadata".
   */
  virtual QString taggedFileKey() const;

  /**
   * Read tags from file.
   *
   * @param force true to force reading even if tags were already read.
   */
  virtual void readTags(bool force);

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
  virtual bool writeTags(bool force, bool* renamed, bool preserve);

  /**
   * Get technical detail information.
   *
   * @param info the detail information is returned here
   */
  virtual void getDetailInfo(DetailInfo& info) const;

  /**
   * Get duration of file.
   *
   * @return duration in seconds,
   *         0 if unknown.
   */
  virtual unsigned getDuration() const;

  /**
   * Get file extension including the dot.
   *
   * @return file extension ".flac".
   */
  virtual QString getFileExtension() const;

#ifdef HAVE_FLAC_PICTURE
  /**
   * Check if file has an ID3v2 tag.
   *
   * @return true if a V2 tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTagV2() const;

  /**
   * Set a frame in the tags 2.
   *
   * @param frame frame to set
   *
   * @return true if ok.
   */
  virtual bool setFrameV2(const Frame& frame);

  /**
   * Add a frame in the tags 2.
   *
   * @param frame frame to add
   *
   * @return true if ok.
   */
  virtual bool addFrameV2(Frame& frame);

  /**
   * Delete a frame in the tags 2.
   *
   * @param frame frame to delete.
   *
   * @return true if ok.
   */
  virtual bool deleteFrameV2(const Frame& frame);

  /**
   * Remove ID3v2 frames.
   *
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFramesV2(const FrameFilter& flt);

  /**
   * Get all frames in tag 2.
   *
   * @param frames frame collection to set.
   */
  virtual void getAllFramesV2(FrameCollection& frames);
#endif // HAVE_FLAC_PICTURE

private:
  FlacFile(const FlacFile&);
  FlacFile& operator=(const FlacFile&);

  /**
   * Read information about a FLAC file.
   * @param info file info to fill
   * @param si stream info
   * @return true if ok.
   */
  bool readFileInfo(FileInfo& info, FLAC::Metadata::StreamInfo* si) const;

  /**
   * Set the vorbis comment block with the comments.
   *
   * @param vc vorbis comment block to set
   */
  void setVorbisComment(FLAC::Metadata::VorbisComment* vc);

#ifdef HAVE_FLAC_PICTURE
  /** Pictures */
  typedef QList<Frame> PictureList;
  PictureList m_pictures;
#endif // HAVE_FLAC_PICTURE

  /** FLAC metadata chain. */
  FLAC::Metadata::Chain* m_chain;
};

#endif // FLACFILE_H
