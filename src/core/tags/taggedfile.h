/**
 * \file taggedfile.h
 * Base class for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Sep 2005
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

#pragma once

#include <QString>
#include <QStringList>
#include <QList>
#include <QPersistentModelIndex>
#include "frame.h"

class TaggedFileSystemModel;

/** Base class for tagged files. */
class KID3_CORE_EXPORT TaggedFile {
public:
  /**
   * Special features and formats supported.
   * Additional information which cannot be deduced from the file format
   * supported.
   */
  enum Feature {
    TF_ID3v11      = 1 << 0, /**< Supports ID3v1.1 tags */
    TF_ID3v22      = 1 << 1, /**< Supports ID3v2.2 tags */
    TF_ID3v23      = 1 << 2, /**< Supports ID3v2.3 tags */
    TF_ID3v24      = 1 << 3, /**< Supports ID3v2.4 tags */
    TF_OggPictures = 1 << 4, /**< Supports pictures in Ogg files */
    TF_OggFlac     = 1 << 5  /**< Supports Ogg FLAC files */
  };

  /** Information about file. */
  struct KID3_CORE_EXPORT DetailInfo {
    /** Channel mode. */
    enum ChannelMode { CM_None, CM_Stereo, CM_JointStereo };

    /** Constructor. */
    DetailInfo();

    QString format;          /**< format description */
    ChannelMode channelMode; /**< channel mode */
    unsigned channels;       /**< number of channels > 0 */
    unsigned sampleRate;     /**< sample rate in Hz > 0 */
    unsigned bitrate;        /**< 0 < bitrate in kbps < 16384 */
    unsigned long duration;  /**< duration in seconds > 0 */
    bool valid;              /**< true if information valid */
    bool vbr;                /**< true if variable bitrate */

    /**
     * Get string representation of detail information.
     * @return information summary as string.
     */
    QString toString() const;
  };

  /**
   * Constructor.
   *
   * @param idx index in tagged file system model
   */
  explicit TaggedFile(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~TaggedFile() = default;

  /**
   * Set file name.
   *
   * @param fn file name
   */
  void setFilename(const QString& fn);

  /**
   * Set file name and format it if format while editing is switched on.
   *
   * @param fn file name
   */
  void setFilenameFormattedIfEnabled(QString fn);

  /**
   * Get file name.
   *
   * @return file name
   */
  const QString& getFilename() const { return m_newFilename; }

  /**
   * Get directory name.
   *
   * @return directory name
   */
  QString getDirname() const;

  /**
   * Get key of tagged file format.
   * @return key.
   */
  virtual QString taggedFileKey() const = 0;

  /**
   * Get features supported.
   * @return bit mask with Feature flags set.
   */
  virtual int taggedFileFeatures() const;

  /**
   * Get currently active tagged file features.
   * @return active tagged file features.
   * @see setActiveTaggedFileFeatures()
   */
  virtual int activeTaggedFileFeatures() const;

  /**
   * Activate some features provided by the tagged file.
   * For example, if the TF_ID3v24 feature is provided, it can be set, so that
   * writeTags() will write ID3v2.4.0 tags. If the feature is deactivated by
   * passing 0, tags in the default format will be written again.
   *
   * @param features bit mask with some of the Feature flags which are
   * provided by this file, as returned by taggedFileFeatures(), 0 to disable
   * special features.
   */
  virtual void setActiveTaggedFileFeatures(int features);

  /**
   * Read tags from file.
   * Implementations should call notifyModelDataChanged().
   *
   * @param force true to force reading even if tags were already read.
   */
  virtual void readTags(bool force) = 0;

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
  virtual bool writeTags(bool force, bool* renamed, bool preserve) = 0;

  /**
   * Free resources allocated when calling readTags().
   * Implementations should call notifyModelDataChanged().
   *
   * @param force true to force clearing even if the tags are modified
   */
  virtual void clearTags(bool force) = 0;

  /**
   * Remove frames.
   *
   * @param tagNr tag number
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFrames(Frame::TagNumber tagNr, const FrameFilter& flt);

  /**
   * Check if file has a tag.
   *
   * @return true if a tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTag(Frame::TagNumber tagNr) const;

  /**
   * Check if tags are supported by the format of this file.
   *
   * @param tagNr tag number
   * @return true if tags are supported.
   */
  virtual bool isTagSupported(Frame::TagNumber tagNr) const;

  /**
   * Check if tag information has already been read.
   *
   * @return true if information is available,
   *         false if the tags have not been read yet, in which case
   *         hasTag() does not return meaningful information.
   */
  virtual bool isTagInformationRead() const = 0;

  /**
   * Get technical detail information.
   *
   * @param info the detail information is returned here
   */
  virtual void getDetailInfo(DetailInfo& info) const = 0;

  /**
   * Get duration of file.
   *
   * @return duration in seconds,
   *         0 if unknown.
   */
  virtual unsigned getDuration() const = 0;

  /**
   * Get file extension including the dot.
   *
   * @return file extension, e.g. ".mp3".
   */
  virtual QString getFileExtension() const = 0;

  /**
   * Get the format of tag.
   *
   * @param tagNr tag number
   * @return string describing format of tag,
   *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
   *         QString::null if unknown.
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
  virtual bool getFrame(Frame::TagNumber tagNr, Frame::Type type, Frame& frame) const = 0;

  /**
   * Set a frame in the tags.
   *
   * @param tagNr tag number
   * @param frame frame to set.
   *
   * @return true if ok.
   */
  virtual bool setFrame(Frame::TagNumber tagNr, const Frame& frame) = 0;

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
   * @param frame frame to delete
   *
   * @return true if ok.
   */
  virtual bool deleteFrame(Frame::TagNumber tagNr, const Frame& frame);

  /**
   * Get a list of frame IDs which can be added.
   * @param tagNr tag number
   * @return list with frame IDs.
   */
  virtual QStringList getFrameIds(Frame::TagNumber tagNr) const = 0;

  /**
   * Get all frames in tag.
   *
   * @param tagNr tag number
   * @param frames frame collection to set.
   */
  virtual void getAllFrames(Frame::TagNumber tagNr, FrameCollection& frames);

  /**
   * Close any file handles which are held open by the tagged file object.
   * The default implementation does nothing. If a concrete subclass holds
   * any file handles open, it has to close them in this method. This method
   * can be used before operations which require that a file is not open,
   * e.g. file renaming on Windows.
   */
  virtual void closeFileHandle();

  /**
   * Add a suitable field list for the frame if missing.
   * If a frame is created, its field list is empty. This method will create
   * a field list appropriate for the frame type and tagged file type if no
   * field list exists. The default implementation does nothing.
   * @param tagNr tag number
   * @param frame frame where field list is added
   */
  virtual void addFieldList(Frame::TagNumber tagNr, Frame& frame) const;

  /**
   * Set frames in tag.
   *
   * @param tagNr tag number
   * @param frames      frame collection
   * @param onlyChanged only frames with value marked as changed are set
   */
  void setFrames(Frame::TagNumber tagNr, const FrameCollection& frames, bool onlyChanged = true);

  /**
   * Get tags from filename.
   * Supported formats:
   * album/track - artist - song
   * artist - album/track song
   * /artist - album - track - song
   * album/artist - track - song
   * artist/album/track song
   * album/artist - song
   *
   * @param frames frames to put result
   * @param fmt format string containing the following codes:
   *            %s title (song)
   *            %l album
   *            %a artist
   *            %c comment
   *            %y year
   *            %t track
   */
  void getTagsFromFilename(FrameCollection& frames, const QString& fmt);

  /**
   * Check if file is changed.
   *
   * @return true if file was changed.
   */
  bool isChanged() const { return m_modified; }

  /**
   * Check if filename is changed.
   *
   * @return true if filename was changed.
   */
  bool isFilenameChanged() const { return m_newFilename != m_filename; }

  /**
   * Get absolute filename.
   *
   * @return absolute file path.
   */
  QString getAbsFilename() const;

  /**
   * Undo reverted modification of filename.
   * When writeTags() fails because the file is not writable, the filename is
   * reverted using revertChangedFilename() so that the file permissions can be
   * changed using the real filename. After changing the permissions, this
   * function can be used to change the filename back before saving the file.
   */
  void undoRevertChangedFilename();

  /**
   * Update the current filename after the file was renamed.
   */
  void updateCurrentFilename();

  /**
   * Check if tag was changed.
   * @param tagNr tag number
   * @return true if tag 1 was changed.
   */
  bool isTagChanged(Frame::TagNumber tagNr) const {
    return tagNr < Frame::Tag_NumValues ? m_changed[tagNr] : false;
  }

  /**
   * Mark tag as changed.
   *
   * @param tagNr tag number
   * @param type type of changed frame
   */
  void markTagChanged(Frame::TagNumber tagNr, Frame::Type type);

  /**
   * Mark tag as unchanged.
   * @param tagNr tag number
   */
  void markTagUnchanged(Frame::TagNumber tagNr);

  /**
   * Get the mask of the frame types changed in tag.
   * @param tagNr tag number
   * @return mask of frame types.
   */
  quint64 getChangedFrames(Frame::TagNumber tagNr) const {
    return tagNr < Frame::Tag_NumValues ? m_changedFrames[tagNr] : 0;
  }

  /**
   * Set the mask of the frame types changed in tag.
   * @param tagNr tag number
   * @param mask mask of frame types
   */
  void setChangedFrames(Frame::TagNumber tagNr, quint64 mask);

  /**
   * Get the truncation flags.
   * @param tagNr tag number
   * @return truncation flags.
   */
  quint64 getTruncationFlags(Frame::TagNumber tagNr) const {
    return tagNr == Frame::Tag_Id3v1 ? m_truncation : 0;
  }

  /**
   * Format track number/total number of tracks with configured digits.
   *
   * @param num track number, <= 0 if empty
   * @param numTracks total number of tracks, <= 0 to disable
   *
   * @return formatted "track/total" string.
   */
  QString trackNumberString(int num, int numTracks) const;

  /**
   * Format the track number (digits, total number of tracks) if enabled.
   *
   * @param value    string containing track number, will be modified
   * @param addTotal true to add total number of tracks if enabled
   *                 "/t" with t = total number of tracks will be appended
   *                 if enabled and value contains a number
   */
  void formatTrackNumberIfEnabled(QString& value, bool addTotal) const;

  /**
   * Get the total number of tracks in the directory.
   *
   * @return total number of tracks, -1 if unavailable.
   */
  int getTotalNumberOfTracksInDir() const;

  /**
   * Get index of tagged file in model.
   * @return index
   */
  const QPersistentModelIndex& getIndex() const { return m_index; }

  /**
   * Check if the file is marked.
   */
  bool isMarked() const { return m_marked; }

  /**
   * Format a time string "h:mm:ss".
   * If the time is less than an hour, the hour is not put into the
   * string and the minute is not padded with zeroes.
   *
   * @param seconds time in seconds
   *
   * @return string with the time in hours, minutes and seconds.
   */
  static QString formatTime(unsigned seconds);

  /**
   * Split a track string into number and total.
   *
   * @param str track
   * @param total the total is returned here if found, else 0
   *
   * @return number, 0 if parsing failed, -1 if str is null
   */
  static int splitNumberAndTotal(const QString& str, int* total=nullptr);

  /**
   * Get access and modification time of file.
   * @param path file path
   * @param actime the last access time is returned here
   * @param modtime the last modification time is returned here
   * @return true if ok.
   */
  static bool getFileTimeStamps(const QString& path,
                                quint64& actime, quint64& modtime);

  /**
   * Set access and modification time of file.
   * @param path file path
   * @param actime last access time
   * @param modtime last modification time
   * @return true if ok.
   */
  static bool setFileTimeStamps(const QString& path,
                                quint64 actime, quint64 modtime);

  /**
   * Free static resources.
   */
  static void staticCleanup();

protected:
  /**
   * Rename a file.
   * This methods takes care of case insensitive filesystems.
   * @return true if ok.
   */
  bool renameFile() const;

  /**
   * Get field name for comment from configuration.
   *
   * @return field name.
   */
  QString getCommentFieldName() const;

    /**
   * Get the total number of tracks if it is enabled.
   *
   * @return total number of tracks,
   *         -1 if disabled or unavailable.
   */
  int getTotalNumberOfTracksIfEnabled() const;

  /**
   * Get the number of track number digits configured.
   *
   * @return track number digits,
   *         1 if invalid or unavailable.
   */
  int getTrackNumberDigits() const;

  /**
   * Get current filename.
   * @return existing name.
   */
  QString currentFilename() const { return m_filename; }

  /**
   * Get current path to file.
   * @return absolute path.
   */
  QString currentFilePath() const;

  /**
   * Mark filename as unchanged.
   */
  void markFilenameUnchanged();

  /**
   * Revert modification of filename.
   */
  void revertChangedFilename();

  /**
   * Check if a string has to be truncated.
   *
   * @param tagNr tag number
   * @param str  string to be checked
   * @param flag flag to be set if string has to be truncated
   * @param len  maximum length of string
   *
   * @return str truncated to len characters if necessary, else QString::null.
   */
  QString checkTruncation(Frame::TagNumber tagNr, const QString& str,
                          quint64 flag, int len = 30);

  /**
   * Check if a number has to be truncated.
   *
   * @param tagNr tag number
   * @param val  value to be checked
   * @param flag flag to be set if number has to be truncated
   * @param max  maximum value
   *
   * @return val truncated to max if necessary, else -1.
   */
  int checkTruncation(Frame::TagNumber tagNr, int val, quint64 flag,
                      int max = 255);

  /**
   * Clear all truncation flags.
   * @param tagNr tag number
   */
  void clearTrunctionFlags(Frame::TagNumber tagNr) {
    if (tagNr == Frame::Tag_Id3v1) m_truncation = 0;
  }

  /**
   * Get tagged file model.
   * @return tagged file model.
   */
  const TaggedFileSystemModel* getTaggedFileSystemModel() const;

  /**
   * Notify model about changes in extra model data, e.g. the information on
   * which the CoreTaggedFileIconProvider depends.
   *
   * This method shall be called when such data changes, e.g. at the end of
   * readTags() implementations.
   *
   * @param priorIsTagInformationRead prior value returned by
   * isTagInformationRead()
   */
  void notifyModelDataChanged(bool priorIsTagInformationRead) const;

  /**
   * Notify model about changes in the truncation state.
   *
   * This method shall be called when truncation is checked.
   *
   * @param priorTruncation prior value of m_truncation != 0
   */
  void notifyTruncationChanged(bool priorTruncation) const;

  /**
   * Update marked property of frames.
   * Mark frames which violate configured rules. This method should be called
   * in reimplementations of getAllFrames().
   *
   * @param tagNr tag number
   * @param frames frames to check
   */
  void updateMarkedState(Frame::TagNumber tagNr, FrameCollection& frames);

private:
  TaggedFile(const TaggedFile&);
  TaggedFile& operator=(const TaggedFile&);

  void updateModifiedState();

  /** Index of file in model */
  QPersistentModelIndex m_index;
  /** File name */
  QString m_filename;
  /** New file name */
  QString m_newFilename;
  /** File name reverted because file was not writable */
  QString m_revertedFilename;
  /** changed tag frame types */
  quint64 m_changedFrames[Frame::Tag_NumValues];
  /** Truncation flags. */
  quint64 m_truncation;
  /** true if tags were changed */
  bool m_changed[Frame::Tag_NumValues];
  /** true if tagged file is modified */
  bool m_modified;
  /** true if tagged file is marked */
  bool m_marked;
};
