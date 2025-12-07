/**
 * \file taglibformatsupport.h
 * Base class for audio formats supported by TagLib.
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

#pragma once

#include "frame.h"
#include "taggedfile.h"

class TagLibFile;

namespace TagLib {
  class String;
  class File;
  class IOStream;
  class AudioProperties;
  class Tag;
}

/**
 * Base class for audio formats supported by TagLib.
 *
 * The functions for specific audio formats supported by TagLib are separated
 * into modules derived from this class. As friends of TagLibFile, they have
 * full access to its internals. The methods provided are called sequentially
 * until a format supporter for the specific file/tag is found, most methods
 * return true if they have handled it.
 */
class TagLibFormatSupport {
public:
  /**
   * Constructor.
   */
  TagLibFormatSupport();

  /**
   * Destructor.
   */
  virtual ~TagLibFormatSupport();

  /**
   * Create a TagLib file from a file stream.
   * @param stream file stream
   * @param ext file extension
   * @return TagLib file if supported, else null.
   */
  virtual TagLib::File* createFromExtension(TagLib::IOStream* stream,
                                            const TagLib::String& ext) const = 0;

  /**
   * Read tags from file.
   * @param f TagLibFile
   * @param file TagLib file
   * @return true if supported.
   */
  virtual bool readFile(TagLibFile& f, TagLib::File* file) const = 0;

  /**
   * Write tags to file.
   * @param f TagLibFile
   * @param file TagLib file
   * @param force true to force writing even if not modified
   * @param id3v2Version ID3v2 version
   * @param fileChanged is set to true if the file was changed
   * @return true if supported.
   */
  virtual bool writeFile(TagLibFile& f, TagLib::File* file, bool force,
                         int id3v2Version, bool& fileChanged) const;

  /**
   * Make sure that the tags in TagLibFile are initialized.
   * @param f TagLibFile
   * @param file TagLib file
   * @param tagNr
   * @return true if supported.
   */
  virtual bool makeTagSettable(TagLibFile& f, TagLib::File* file,
                               Frame::TagNumber tagNr) const;

  /**
   * Set the detail info in TagLibFile from TagLib audio properties.
   * @param f TagLibFile
   * @param audioProperties TagLib audio properties
   * @return true if supported.
   */
  virtual bool readAudioProperties(TagLibFile& f,
                                   TagLib::AudioProperties* audioProperties) const = 0;

  /**
   * Get string description of tag format.
   * @param tag TagLib tag
   * @param type the tag type is returned here
   * @return string representation of tag format, null if not supported.
   */
  virtual QString getTagFormat(
    const TagLib::Tag* tag, TaggedFile::TagType& type) const;

  /**
   * Set frame in tags.
   * Implementations must support existing frames with a valid index, as well
   * as new frames with index -1 (usually calling setFrameWithoutIndex()).
   * @param f TagLibFile
   * @param tagNr
   * @param frame frame to set
   * @return true if supported.
   */
  virtual bool setFrame(
    TagLibFile& f, Frame::TagNumber tagNr, const Frame& frame) const;

  /**
   * Add a new frame to the tag.
   * @param f TagLibFile
   * @param tagNr
   * @param frame frame to add
   * @return true if supported.
   */
  virtual bool addFrame(
    TagLibFile& f, Frame::TagNumber tagNr, Frame& frame) const;

  /**
   * Delete a frame from the tag.
   * Such frames should have a valid index.
   * @param f TagLibFile
   * @param tagNr
   * @param frame frame to delete
   * @return true if supported.
   */
  virtual bool deleteFrame(
    TagLibFile& f, Frame::TagNumber tagNr, const Frame& frame) const;

  /**
   * Delete multiple frames.
   * @param f TagLibFile
   * @param tagNr
   * @param flt frame filter
   * @return true if supported.
   */
  virtual bool deleteFrames(
    TagLibFile& f, Frame::TagNumber tagNr, const FrameFilter& flt) const;

  /**
   * Get all frames from the tag.
   * @param f TagLibFile
   * @param tagNr
   * @param frames the frames are returned here
   * @return true if supported.
   */
  virtual bool getAllFrames(
    TagLibFile& f, Frame::TagNumber tagNr, FrameCollection& frames) const;

  /**
   * Get the IDs/keys of all frames which can be added to the tag.
   * @param f TagLibFile
   * @param tagNr
   * @return list of IDs, empty if not supported.
   */
  virtual QStringList getFrameIds(
    const TagLibFile& f, Frame::TagNumber tagNr) const;

protected:
  /**
   * Set the value of a standard frame.
   * Called by setFrameWithoutIndex() to set the value of standard tags,
   * can be reimplemented for different tag formats. The default implementation
   * just uses the standard tag setters of TagLib::Tag.
   * @param f TagLibFile
   * @param tagNr
   * @param type type of frame
   * @param str value of frame
   */
  virtual void setTagValue(TagLibFile& f, Frame::TagNumber tagNr,
                           Frame::Type type, const TagLib::String& str) const;

  /**
   * Set a new standard frame in the tag.
   * This method can be used in implementations of setFrame() to set a standard
   * frame which does not exist yet, and thus has an index of -1.
   * @param f TagLibFile
   * @param tagNr
   * @param frame frame to set
   * @return true if supported.
   */
  bool setFrameWithoutIndex(TagLibFile& f, Frame::TagNumber tagNr,
                            const Frame& frame) const;

  /**
   * Set tag from file ref as tag 2 in TagLibFile.
   * Can be used to implement readFile() for audio formats which support only
   * a single tag.
   * @param f TagLibFile
   */
  static void putFileRefTagInTag2(TagLibFile& f);

  /**
   * Check if any of the tags must be saved.
   * @param f TagLibFile
   * @param force true to force saving even if the tag has no changes
   * @return true if tags must be saved.
   */
  static bool anyTagMustBeSaved(const TagLibFile& f, bool force);

  /**
   * Save the file ref and mark all tags as unchanged.
   * Can be used for tag formats which do not need any special parameters when
   * saving.
   * @param f TagLibFile
   * @return true if ok.
   */
  static bool saveFileRef(TagLibFile& f);
};
