/**
 * \file oggfile.hpp
 * Handling of Ogg files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 26 Sep 2005
 *
 * Copyright (C) 2005-2023  Urs Fleisch
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

#include <QList>
#include "oggflacconfig.h"
#include "taggedfile.h"


 /** List box item containing OGG file */
class OggFile : public TaggedFile {
public:
  /**
   * Constructor.
   *
   * @param idx index in tagged file system model
   */
  explicit OggFile(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~OggFile() override = default;

  /**
   * Get key of tagged file format.
   * @return "OggMetadata".
   */
  virtual QString taggedFileKey() const override;

  /**
   * Get features supported.
   * @return bit mask with Feature flags set.
   */
  virtual int taggedFileFeatures() const override;

  /**
   * Read tags from file.
   *
   * @param force true to force reading even if tags were already read.
   */
  virtual void readTags(bool force) override;

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
  virtual bool writeTags(bool force, bool* renamed, bool preserve) override;

  /**
   * Free resources allocated when calling readTags().
   *
   * @param force true to force clearing even if the tags are modified
   */
  virtual void clearTags(bool force) override;

  /**
   * Remove frames.
   *
   * @param tagNr tag number
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt) override;


  /**
   * Check if tag information has already been read.
   *
   * @return true if information is available,
   *         false if the tags have not been read yet, in which case
   *         hasTag() does not return meaningful information.
   */
  virtual bool isTagInformationRead() const override;

  /**
   * Check if file has a tag.
   *
   * @param tagNr tag number
   * @return true if a tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTag(Frame::TagNumber tagNr) const override;

  /**
   * Get technical detail information.
   *
   * @param info the detail information is returned here
   */
  virtual void getDetailInfo(DetailInfo& info) const override;

  /**
   * Get duration of file.
   *
   * @return duration in seconds,
   *         0 if unknown.
   */
  virtual unsigned getDuration() const override;

  /**
   * Get file extension including the dot.
   *
   * @return file extension ".ogg".
   */
  virtual QString getFileExtension() const override;

  /**
   * Get the format of tag.
   *
   * @param tagNr tag number
   * @return "Vorbis".
   */
  virtual QString getTagFormat(Frame::TagNumber tagNr) const override;

  /**
   * Get a specific frame from the tags.
   *
   * @param tagNr tag number
   * @param type  frame type
   * @param frame the frame is returned here
   *
   * @return true if ok.
   */
  virtual bool getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const override;

  /**
   * Set a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to set
   *
   * @return true if ok.
   */
  virtual bool setFrame(Frame::TagNumber tagNr, const Frame& frame) override;

  /**
   * Add a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to add
   *
   * @return true if ok.
   */
  virtual bool addFrame(Frame::TagNumber tagNr, Frame& frame) override;

  /**
   * Delete a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to delete.
   *
   * @return true if ok.
   */
  virtual bool deleteFrame(Frame::TagNumber tagNr, const Frame& frame) override;

  /**
   * Get all frames in tag.
   *
   * @param tagNr tag number
   * @param frames frame collection to set.
   */
  virtual void getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames) override;

  /**
   * Get a list of frame IDs which can be added.
   * @param tagNr tag number
   * @return list with frame IDs.
   */
  virtual QStringList getFrameIds(Frame::TagNumber tagNr) const override;

protected:
  /** Vorbis comment field. */
  class CommentField {
  public:
    /** Constructor. */
    CommentField(const QString& name = QString(),
                 const QString& value = QString())
      : m_name(name), m_value(value) {}
    /**
     * Get name.
     * @return name.
     */
    QString getName() const { return m_name; }
    /**
     * Get value.
     * @return value.
     */
    QString getValue() const { return m_value; }
    /**
     * Set value.
     * @param value value
     */
    void setValue(const QString& value) { m_value = value; }

  private:
    QString m_name;
    QString m_value;
  };

  /** Vorbis comment list. */
  class CommentList : public QList<CommentField> {
  public:
    /** Constructor. */
    CommentList() {}
    /** Destructor. */
    ~CommentList() {}
    /**
     * Get value.
     * @param name name
     * @return value, "" if not found.
     */
    QString getValue(const QString& name) const;
    /**
     * Set value.
     * @param name name
     * @param value value
     * @return true if value was changed.
     */
    bool setValue(const QString& name, const QString& value);
  };

  /**
   * Get text field.
   *
   * @param name name
   * @return value, "" if not found,
   *         QString::null if the tags have not been read yet.
   */
  QString getTextField(const QString& name) const;

  /**
   * Set text field.
   * If value is null or the tags have not been read yet, nothing is changed.
   * If value is different from the current value, changed is set.
   *
   * @param name name
   * @param value value, "" to remove, QString::null to do nothing
   * @param type frame type
   */
  void setTextField(const QString& name, const QString& value,
                    const Frame::ExtendedType& type);

protected:
  /** true if file has been read. */
  bool m_fileRead;
  /** Comments of this file. */
  CommentList m_comments;

  /** Information about Ogg/Vorbis file. */
  struct FileInfo {
    /** Constructor. */
    FileInfo() : version(0), channels(0),
      sampleRate(0), bitrate(0), duration(0), valid(false) {}

    int version;     /**< vorbis encoder version */
    int channels;    /**< number of channels */
    long sampleRate; /**< sample rate in Hz */
    long bitrate;    /**< bitrate in bits/s */
    long duration;   /**< duration in seconds */
    bool valid;      /**< true if read() was successful */
  };

  /** Info about file. */
  FileInfo m_fileInfo;

private:
  OggFile(const OggFile&);
  OggFile& operator=(const OggFile&);

#ifdef HAVE_VORBIS
  /**
   * Read information about an Ogg/Vorbis file.
   * @param info file info to fill
   * @param fn file name
   * @return true if ok.
   */
  bool readFileInfo(FileInfo& info, const QString& fn) const;
#endif // HAVE_VORBIS
};
