/**
 * \file m4afile.h
 * Handling of MPEG-4 audio files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Oct 2007
 *
 * Copyright (C) 2007-2011  Urs Fleisch
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

#ifndef M4AFILE_H
#define M4AFILE_H

#include "taggedfile.h"
#include <QMap>

/** MPEG-4 audio file */
class M4aFile : public TaggedFile {
public:
  /**
   * Constructor.
   *
   * @param idx index in file proxy model
   */
  explicit M4aFile(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~M4aFile();

  /**
   * Get key of tagged file format.
   * @return "Mp4v2Metadata".
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
   * Remove ID3v2 frames.
   *
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFramesV2(const FrameFilter& flt);

  /**
   * Check if tag information has already been read.
   *
   * @return true if information is available,
   *         false if the tags have not been read yet, in which case
   *         hasTagV1() and hasTagV2() do not return meaningful information.
   */
  virtual bool isTagInformationRead() const;

  /**
   * Check if file has an ID3v2 tag.
   *
   * @return true if a V2 tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTagV2() const;

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
   * @return file extension ".m4a".
   */
  virtual QString getFileExtension() const;

  /**
   * Get the format of tag 2.
   *
   * @return "Vorbis".
   */
  virtual QString getTagFormatV2() const;

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
   * Get all frames in tag 2.
   *
   * @param frames frame collection to set.
   */
  virtual void getAllFramesV2(FrameCollection& frames);

  /**
   * Get a list of frame IDs which can be added.
   *
   * @return list with frame IDs.
   */
  virtual QStringList getFrameIds() const;

private:
  M4aFile(const M4aFile&);
  M4aFile& operator=(const M4aFile&);

  /**
   * Get ID3v2 title.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getTitleV2() const;

  /**
   * Get ID3v2 artist.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getArtistV2() const;

  /**
   * Get ID3v2 album.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getAlbumV2() const;

  /**
   * Get ID3v2 comment.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getCommentV2() const;

  /**
   * Get ID3v2 year.
   *
   * @return number,
   *         0 if the field does not exist,
   *         -1 if the tags do not exist.
   */
  virtual int getYearV2() const;

  /**
   * Get ID3v2 track.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getTrackV2() const;

  /**
   * Get ID3v2 genre as text.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getGenreV2() const;

  /**
   * Set ID3v2 title.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setTitleV2(const QString& str);

  /**
   * Set ID3v2 artist.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setArtistV2(const QString& str);

  /**
   * Set ID3v2 album.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setAlbumV2(const QString& str);

  /**
   * Set ID3v2 comment.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setCommentV2(const QString& str);

  /**
   * Set ID3v2 year.
   *
   * @param num number to set, 0 to remove field.
   */
  virtual void setYearV2(int num);

  /**
   * Set ID3v2 track.
   *
   * @param track string to set, "" to remove field, QString::null to ignore.
   */
  virtual void setTrackV2(const QString& track);

  /**
   * Set ID3v2 genre as text.
   *
   * @param str string to set, "" to remove field, QString::null to ignore.
   */
  virtual void setGenreV2(const QString& str);

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
                    Frame::Type type);

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
};

#endif // M4AFILE_H
