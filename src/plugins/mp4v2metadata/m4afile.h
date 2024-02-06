/**
 * \file m4afile.h
 * Handling of MPEG-4 audio files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Oct 2007
 *
 * Copyright (C) 2007-2024  Urs Fleisch
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

#pragma once

#include "taggedfile.h"
#include <QMap>

/** MPEG-4 audio file */
class M4aFile : public TaggedFile {
public:
  /**
   * Constructor.
   *
   * @param idx index in tagged file system model
   */
  explicit M4aFile(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  ~M4aFile() override = default;

  /**
   * Get key of tagged file format.
   * @return "Mp4v2Metadata".
   */
  QString taggedFileKey() const override;

  /**
   * Read tags from file.
   *
   * @param force true to force reading even if tags were already read.
   */
  void readTags(bool force) override;

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
  bool writeTags(bool force, bool* renamed, bool preserve) override;

  /**
   * Free resources allocated when calling readTags().
   *
   * @param force true to force clearing even if the tags are modified
   */
  void clearTags(bool force) override;

  /**
   * Remove frames.
   *
   * @param tagNr tag number
   * @param flt filter specifying which frames to remove
   */
  void deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt) override;

  /**
   * Check if tag information has already been read.
   *
   * @return true if information is available,
   *         false if the tags have not been read yet, in which case
   *         hasTag() does not return meaningful information.
   */
  bool isTagInformationRead() const override;

  /**
   * Check if file has a tag.
   *
   * @param tagNr tag number
   * @return true if a V2 tag is available.
   * @see isTagInformationRead()
   */
  bool hasTag(Frame::TagNumber tagNr) const override;

  /**
   * Get technical detail information.
   *
   * @param info the detail information is returned here
   */
  void getDetailInfo(DetailInfo& info) const override;

  /**
   * Get duration of file.
   *
   * @return duration in seconds,
   *         0 if unknown.
   */
  unsigned getDuration() const override;

  /**
   * Get file extension including the dot.
   *
   * @return file extension ".m4a".
   */
  QString getFileExtension() const override;

  /**
   * Get the format of tag.
   *
   * @param tagNr tag number
   * @return "Vorbis".
   */
  QString getTagFormat(Frame::TagNumber tagNr) const override;

  /**
   * Get a specific frame from the tags.
   *
   * @param tagNr tag number
   * @param type  frame type
   * @param frame the frame is returned here
   *
   * @return true if ok.
   */
  bool getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const override;

  /**
   * Set a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to set
   *
   * @return true if ok.
   */
  bool setFrame(Frame::TagNumber tagNr, const Frame& frame) override;

  /**
   * Add a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to add
   *
   * @return true if ok.
   */
  bool addFrame(Frame::TagNumber tagNr, Frame& frame) override;

  /**
   * Delete a frame from the tags.
   *
   * @param tagNr tag number
   * @param frame frame to delete.
   *
   * @return true if ok.
   */
  bool deleteFrame(Frame::TagNumber tagNr, const Frame& frame) override;

  /**
   * Get all frames in tag.
   *
   * @param tagNr tag number
   * @param frames frame collection to set.
   */
  void getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames) override;

  /**
   * Get a list of frame IDs which can be added.
   * @param tagNr tag number
   * @return list with frame IDs.
   */
  QStringList getFrameIds(Frame::TagNumber tagNr) const override;

private:
  M4aFile(const M4aFile&);
  M4aFile& operator=(const M4aFile&);

  /**
   * Get metadata field as string.
   *
   * @param name field name
   *
   * @return value as string, "" if not found,
   *         QString::null if the tags have not been read yet.
   */
  QString getTextField(const QString& name) const;

  /**
   * Set text field.
   * If value is null if the tags have not been read yet, nothing is changed.
   * If value is different from the current value, tag 2 is marked as changed.
   *
   * @param name name
   * @param value value, "" to remove, QString::null to do nothing
   * @param type frame type
   */
  void setTextField(const QString& name, const QString& value,
                    const Frame::ExtendedType& type);

  /** true if file has been read. */
  bool m_fileRead;

  /** Information about MPEG-4 file. */
  struct FileInfo {
    /**
     * Constructor.
     */
    FileInfo() : valid(false), channels(0), sampleRate(0), bitrate(0),
                 duration(0) {}

    /**
     * Read information about an MPEG-4 file.
     * @param handle MP4 handle
     * @return true if ok.
     */
    bool read(void* handle);

    bool valid;      /**< true if read() was successful */
    int channels;    /**< number of channels */
    long sampleRate; /**< sample rate in Hz */
    long bitrate;    /**< bitrate in bits/s */
    long duration;   /**< duration in seconds */
  };

  /** Info about file. */
  FileInfo m_fileInfo;

  /** Map with metadata. */
  typedef QMap<QString, QByteArray> MetadataMap;

  /** Metadata. */
  MetadataMap m_metadata;
  QList<Frame> m_extraFrames;
};
