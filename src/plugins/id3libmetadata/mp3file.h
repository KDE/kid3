/**
 * \file mp3file.h
 * Handling of tagged MP3 files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
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

#ifndef MP3FILE_H
#define MP3FILE_H

#include "taggedfile.h"
#include "tagconfig.h"

class ID3_Tag;
class ID3_Field;
class ID3_Frame;
class QTextCodec;

/** List box item containing MP3 file */
class Mp3File : public TaggedFile {
public:
  /**
   * Constructor.
   *
   * @param idx index in file proxy model
   */
  explicit Mp3File(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~Mp3File();

  /**
   * Get key of tagged file format.
   * @return "Id3libMetadata".
   */
  virtual QString taggedFileKey() const;

  /**
   * Get features supported.
   * @return bit mask with Feature flags set.
   */
  virtual int taggedFileFeatures() const;

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
   * Remove frames.
   *
   * @param tagNr tag number
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt);

  /**
   * Check if tag information has already been read.
   *
   * @return true if information is available,
   *         false if the tags have not been read yet, in which case
   *         hasTag() does not return meaningful information.
   */
  virtual bool isTagInformationRead() const;

  /**
   * Check if file has a tag.
   *
   * @param tagNr tag number
   * @return true if a tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTag(Frame::TagNumber tagNr) const;

  /**
   * Check if tags are supported by the format of this file.
   *
   * @param tagNr tag number
   * @return true.
   */
  virtual bool isTagSupported(Frame::TagNumber tagNr) const;

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
   * @return file extension ".mp3".
   */
  virtual QString getFileExtension() const;

  /**
   * Get the format of tag.
   *
   * @param tagNr tag number
   * @return string describing format of tag,
   *         e.g. "ID3v1.1", "ID3v2.3", "ID3v2.4".
   */
  virtual QString getTagFormat(Frame::TagNumber tagNr) const;

  /**
   * Get a specific frame from the tags.
   *
   * @param tagNr tag number
   * @param type  frame type
   * @param frame the frame is returned here
   *
   * @return true if ok.
   */
  virtual bool getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const;

  /**
   * Set a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to set
   *
   * @return true if ok.
   */
  virtual bool setFrame(Frame::TagNumber tagNr, const Frame& frame);

  /**
   * Add a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to add, a field list may be added by this method
   *
   * @return true if ok.
   */
  virtual bool addFrame(Frame::TagNumber tagNr, Frame& frame);

  /**
   * Delete a frame from the tags.
   *
   * @param tagNr tag number
   * @param frame frame to delete.
   *
   * @return true if ok.
   */
  virtual bool deleteFrame(Frame::TagNumber tagNr, const Frame& frame);

  /**
   * Get all frames in tag.
   *
   * @param tagNr tag number
   * @param frames frame collection to set.
   */
  virtual void getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames);

  /**
   * Get a list of frame IDs which can be added.
   * @param tagNr tag number
   * @return list with frame IDs.
   */
  virtual QStringList getFrameIds(Frame::TagNumber tagNr) const;

  /**
   * Add a suitable field list for the frame if missing.
   * If a frame is created, its field list is empty. This method will create
   * a field list appropriate for the frame type and tagged file type if no
   * field list exists.
   * @param tagNr tag number
   * @param frame frame where field list is added
   */
  virtual void addFieldList(Frame::TagNumber tagNr, Frame& frame) const;

  /**
   * Notify about configuration change.
   * This method shall be called when the configuration changes.
   */
  static void notifyConfigurationChange();

private:
  Mp3File(const Mp3File&);
  Mp3File& operator=(const Mp3File&);

  /**
   * Set track.
   *
   * @param tag ID3 tag
   * @param num number to set, 0 to remove field.
   * @param numTracks total number of tracks, <=0 to ignore
   *
   * @return true if the field was changed.
   */
  bool setTrackNum(ID3_Tag* tag, int num, int numTracks = 0) const;

  /**
   * Set the fields in an id3lib frame from the field in the frame.
   *
   * @param id3Frame id3lib frame
   * @param frame    frame with fields
   */
  void setId3v2Frame(ID3_Frame* id3Frame, const Frame& frame) const;

  /**
   * Create an id3lib frame from a frame.
   * @param frame frame
   * @return id3lib frame, 0 if invalid.
   */
  ID3_Frame* createId3FrameFromFrame(Frame& frame) const;

  /**
   * Set the text codec to be used for tag 1.
   *
   * @param codec text codec, 0 to use default (ISO 8859-1)
   */
  static void setTextCodecV1(const QTextCodec* codec);

  /**
   * Set the default text encoding.
   *
   * @param textEnc default text encoding
   */
  static void setDefaultTextEncoding(TagConfig::TextEncoding textEnc);

  /** ID3v1 tags */
  ID3_Tag* m_tagV1;

  /** ID3v2 tags */
  ID3_Tag* m_tagV2;
};

#endif // MP3FILE_H
