/**
 * \file taglibmpegsupport.h
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

#pragma once

#include "taglibformatsupport.h"

class TagLibMpegSupport : public TagLibFormatSupport {
public:
  TagLib::File* createFromExtension(TagLib::IOStream* stream,
                                    const TagLib::String& ext) const override;
  bool readFile(TagLibFile& f, TagLib::File* file) const override;
  bool writeFile(TagLibFile& f, TagLib::File* file, bool force,
    int id3v2Version, bool& fileChanged) const override;
  bool makeTagSettable(TagLibFile& f, TagLib::File* file,
    Frame::TagNumber tagNr) const override;
  bool readAudioProperties(TagLibFile& f,
    TagLib::AudioProperties* audioProperties) const override;
  QString getTagFormat(const TagLib::Tag* tag,
    TaggedFile::TagType& type) const override;
  bool setFrame(TagLibFile& f, Frame::TagNumber tagNr,
    const Frame& frame) const override;
  bool addFrame(TagLibFile& f, Frame::TagNumber tagNr,
    Frame& frame) const override;
  bool deleteFrame(TagLibFile& f, Frame::TagNumber tagNr,
    const Frame& frame) const override;
  bool deleteFrames(TagLibFile& f, Frame::TagNumber tagNr,
    const FrameFilter& flt) const override;
  bool getAllFrames(TagLibFile& f, Frame::TagNumber tagNr,
    FrameCollection& frames) const override;
  QStringList getFrameIds(const TagLibFile& f,
    Frame::TagNumber tagNr) const override;

  /**
   * Add a suitable field list for the frame if missing.
   * If a frame is created, its field list is empty. This method will create
   * a field list appropriate for the frame type and tagged file type if no
   * field list exists.
   * @param f file
   * @param tagNr tag number
   * @param frame frame where field list is added
   */
  static void addFieldList(const TagLibFile& f, Frame::TagNumber tagNr, Frame& frame);

protected:
  void setTagValue(TagLibFile& f, Frame::TagNumber tagNr, Frame::Type type,
    const TagLib::String& str) const override;

private:
  static TagLib::String truncateIfNeeded(
    TagLibFile& f, Frame::TagNumber tagNr, const TagLib::String& str,
    quint64 flag, int len);
};
