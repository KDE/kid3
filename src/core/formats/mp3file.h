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

#include "config.h"
#ifdef HAVE_ID3LIB

#include "taggedfile.h"
#include "miscconfig.h"
#include <id3/globals.h> /* ID3_FrameID */

class ID3_Tag;
class ID3_Field;
class ID3_Frame;
class QTextCodec;

/** List box item containing MP3 file */
class Mp3File : public TaggedFile {
public:
  /** File type resolution. */
  class Resolver : public TaggedFile::Resolver {
    /**
     * Create an Mp3File object if it supports the filename's extension.
     *
     * @param dn directory name
     * @param fn filename
     * @param idx model index
     *
     * @return tagged file, 0 if type not supported.
     */
    virtual TaggedFile* createFile(const QString& dn, const QString& fn,
                                   const QPersistentModelIndex& idx) const;

    /**
     * Get a list with all extensions supported by Mp3File.
     *
     * @return list of file extensions.
     */
    virtual QStringList getSupportedFileExtensions() const;
  };


  /**
   * Constructor.
   *
   * @param dn directory name
   * @param fn filename
   * @param idx model index
   */
  Mp3File(const QString& dn, const QString& fn,
          const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~Mp3File();

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
   * Remove ID3v1 frames.
   *
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFramesV1(const FrameFilter& flt);

  /**
   * Remove ID3v2 frames.
   *
   * @param flt filter specifying which frames to remove
   */
  virtual void deleteFramesV2(const FrameFilter& flt);

  /**
   * Get ID3v1 title.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getTitleV1();

  /**
   * Get ID3v1 artist.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getArtistV1();

  /**
   * Get ID3v1 album.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getAlbumV1();

  /**
   * Get ID3v1 comment.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getCommentV1();

  /**
   * Get ID3v1 year.
   *
   * @return number,
   *         0 if the field does not exist,
   *         -1 if the tags do not exist.
   */
  virtual int getYearV1();

  /**
   * Get ID3v1 track.
   *
   * @return number,
   *         0 if the field does not exist,
   *         -1 if the tags do not exist.
   */
  virtual int getTrackNumV1();

  /**
   * Get ID3v1 genre.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getGenreV1();

  /**
   * Get ID3v2 title.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getTitleV2();

  /**
   * Get ID3v2 artist.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getArtistV2();

  /**
   * Get ID3v2 album.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getAlbumV2();

  /**
   * Get ID3v2 comment.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getCommentV2();

  /**
   * Get ID3v2 year.
   *
   * @return number,
   *         0 if the field does not exist,
   *         -1 if the tags do not exist.
   */
  virtual int getYearV2();

  /**
   * Get ID3v2 track.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getTrackV2();

  /**
   * Get ID3v2 genre as text.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getGenreV2();

  /**
   * Set ID3v1 title.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setTitleV1(const QString& str);

  /**
   * Set ID3v1 artist.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setArtistV1(const QString& str);

  /**
   * Set ID3v1 album.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setAlbumV1(const QString& str);

  /**
   * Set ID3v1 comment.
   *
   * @param str string to set, "" to remove field.
   */
  virtual void setCommentV1(const QString& str);

  /**
   * Set ID3v1 year.
   *
   * @param num number to set, 0 to remove field.
   */
  virtual void setYearV1(int num);

  /**
   * Set ID3v1 track.
   *
   * @param num number to set, 0 to remove field.
   */
  virtual void setTrackNumV1(int num);

  /**
   * Set ID3v1 genre as text.
   *
   * @param str string to set, "" to remove field, QString::null to ignore.
   */
  virtual void setGenreV1(const QString& str);

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
   * Check if tag information has already been read.
   *
   * @return true if information is available,
   *         false if the tags have not been read yet, in which case
   *         hasTagV1() and hasTagV2() do not return meaningful information.
   */
  virtual bool isTagInformationRead() const;

  /**
   * Check if file has an ID3v1 tag.
   *
   * @return true if a V1 tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTagV1() const;

  /**
   * Check if file has an ID3v2 tag.
   *
   * @return true if a V2 tag is available.
   * @see isTagInformationRead()
   */
  virtual bool hasTagV2() const;

  /**
   * Check if ID3v1 tags are supported by the format of this file.
   *
   * @return true.
   */
  virtual bool isTagV1Supported() const;

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
   * Get the format of tag 1.
   *
   * @return string describing format of tag 1,
   *         e.g. "ID3v1.1".
   */
  virtual QString getTagFormatV1() const;

  /**
   * Get the format of tag 2.
   *
   * @return string describing format of tag 2,
   *         e.g. "ID3v2.3", "ID3v2.4".
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
   * @param frame frame to add, a field list may be added by this method
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
  static void setDefaultTextEncoding(MiscConfig::TextEncoding textEnc);

  /**
   * Get the default text encoding.
   * @return default text encoding.
   */
  static ID3_TextEnc getDefaultTextEncoding() { return s_defaultTextEncoding; }

private:
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

  Mp3File(const Mp3File&);
  Mp3File& operator=(const Mp3File&);

  /** ID3v1 tags */
  ID3_Tag* m_tagV1;

  /** ID3v2 tags */
  ID3_Tag* m_tagV2;

  /** Text codec for ID3v1 tags, 0 to use default (ISO 8859-1) */
  static const QTextCodec* s_textCodecV1;

  /** Default text encoding */
  static ID3_TextEnc s_defaultTextEncoding;
};

#endif // HAVE_ID3LIB

#endif // MP3FILE_H