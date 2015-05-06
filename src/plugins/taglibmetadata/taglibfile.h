/**
 * \file taglibfile.h
 * Handling of tagged files using TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Sep 2006
 *
 * Copyright (C) 2006-2012  Urs Fleisch
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

#ifndef TAGLIBFILE_H
#define TAGLIBFILE_H

#include "taglibconfig.h"

#include <QtGlobal>
#include "taggedfile.h"
#include "tagconfig.h"
#include <taglib.h>
#include <fileref.h>
#include <tag.h>
#include <id3v2frame.h>

/** TagLib version in with 8 bits for major, minor and patch version. */
#define TAGLIB_VERSION (((TAGLIB_MAJOR_VERSION) << 16) + \
                        ((TAGLIB_MINOR_VERSION) << 8) + (TAGLIB_PATCH_VERSION))
/* Workaround for the wrong TAGLIB_MINOR_VERSION in TagLib 1.8.0 */
#if defined HAVE_TAGLIB_ID3V23_SUPPORT && TAGLIB_VERSION == 0x010700
#undef TAGLIB_VERSION
#define TAGLIB_VERSION 0x010800
#endif

class QTextCodec;
#if TAGLIB_VERSION >= 0x010800
class FileIOStream;
#endif

#if TAGLIB_VERSION >= 0x010600 && defined TAGLIB_WITH_MP4
  namespace TagLib {
    namespace MP4 {
      class Tag;
    }
  }
#endif

/** List box item containing tagged file. */
class TagLibFile : public TaggedFile {
public:
  /**
   * Constructor.
   *
   * @param idx index in file proxy model
   */
  explicit TagLibFile(const QPersistentModelIndex& idx);

  /**
   * Destructor.
   */
  virtual ~TagLibFile();

  /**
   * Get key of tagged file format.
   * @return "TaglibMetadata".
   */
  virtual QString taggedFileKey() const;

  /**
   * Get features supported.
   * @return bit mask with Feature flags set.
   */
  virtual int taggedFileFeatures() const;

  /**
   * Get currently active tagged file features.
   * @return active tagged file features (TF_ID3v23, TF_ID3v24, or 0).
   * @see setActiveTaggedFileFeatures()
   */
  virtual int activeTaggedFileFeatures() const;

  /**
   * Activate some features provided by the tagged file.
   * TagLibFile provides the TF_ID3v23 and TF_ID3v24 features, which determine
   * the ID3v2 version used in writeTags() (the overload without id3v2Version).
   * If 0 is set, the default behavior applies, i.e. for new files,
   * TagConfig::id3v2Version() is used, else the existing version.
   *
   * @param features TF_ID3v23, TF_ID3v24, or 0
   */
  virtual void setActiveTaggedFileFeatures(int features);

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
   * Write tags to file and rename it if necessary.
   *
   * @param force    true to force writing even if file was not changed.
   * @param renamed  will be set to true if the file was renamed,
   *                 i.e. the file name is no longer valid, else *renamed
   *                 is left unchanged
   * @param preserve true to preserve file time stamps
   * @param id3v2Version ID3v2 version to use, 0 to use existing or preferred,
   *                     3 to force ID3v2.3.0, 4 to force ID3v2.4.0. Is ignored
   *                     if TagLib version is less than 1.8.0.
   *
   * @return true if ok, false if the file could not be written or renamed.
   */
  bool writeTags(bool force, bool* renamed, bool preserve, int id3v2Version);

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
  virtual QString getTitleV1() const;

  /**
   * Get ID3v1 artist.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getArtistV1() const;

  /**
   * Get ID3v1 album.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getAlbumV1() const;

  /**
   * Get ID3v1 comment.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getCommentV1() const;

  /**
   * Get ID3v1 year.
   *
   * @return number,
   *         0 if the field does not exist,
   *         -1 if the tags do not exist.
   */
  virtual int getYearV1() const;

  /**
   * Get ID3v1 track.
   *
   * @return number,
   *         0 if the field does not exist,
   *         -1 if the tags do not exist.
   */
  virtual int getTrackNumV1() const;

  /**
   * Get ID3v1 genre.
   *
   * @return string,
   *         "" if the field does not exist,
   *         QString::null if the tags do not exist.
   */
  virtual QString getGenreV1() const;

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
   *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
   *         QString::null if unknown.
   */
  virtual QString getTagFormatV1() const;

  /**
   * Get the format of tag 2.
   *
   * @return string describing format of tag 2,
   *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
   *         QString::null if unknown.
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
   * Close file handle which is held open by the TagLib object.
   */
  virtual void closeFileHandle();

  /**
   * Get a list of frame IDs which can be added.
   *
   * @return list with frame IDs.
   */
  virtual QStringList getFrameIds() const;

  /**
   * Static initialization.
   * Registers file types.
   */
  static void staticInit();

  /**
   * Get the default text encoding.
   * @return default text encoding.
   */
  static TagLib::String::Type getDefaultTextEncoding() { return s_defaultTextEncoding; }

  /**
   * Notify about configuration change.
   * This method shall be called when the configuration changes.
   */
  static void notifyConfigurationChange();

private:
  /** Tag type for cached information. */
  enum TagType {
    TT_Unknown,
    TT_Id3v1,
    TT_Id3v2,
    TT_Vorbis,
    TT_Ape,
    TT_Mp4,
    TT_Asf
  };

  TagLibFile(const TagLibFile&);
  TagLibFile& operator=(const TagLibFile&);

  /**
   * Modify an ID3v2 frame.
   *
   * @param id3Frame original ID3v2 frame
   * @param frame    frame with fields to set in new frame
   */
  void setId3v2Frame(
    TagLib::ID3v2::Frame* id3Frame, const Frame& frame) const;

  /**
   * Close file handle.
   * TagLib keeps the file handle open until the FileRef is destroyed.
   * This causes problems when the operating system has a limited number of
   * open file handles. This method closes the file by assigning a new file
   * reference. Note that this will also invalidate the tag pointers.
   * The file is only closed if there are no unsaved tag changes or if the
   * @a force parameter is set.
   *
   * @param force true to close the file even if tags are changed
   */
  void closeFile(bool force = false);

  /**
   * Make sure that file is open.
   * This method should be called before accessing m_fileRef, m_tagV1, m_tagV2.
   *
   * @param force true to force reopening of file even if it is already open
   */
  void makeFileOpen(bool force = false) const;

  /**
   * Create m_tagV1 if it does not already exist so that it can be set.
   *
   * @return true if m_tagV1 can be set.
   */
  bool makeTagV1Settable();

  /**
   * Create m_tagV2 if it does not already exist so that it can be set.
   *
   * @return true if m_tagV2 can be set.
   */
  bool makeTagV2Settable();

  /**
   * Cache technical detail information.
   */
  void readAudioProperties();

#if TAGLIB_VERSION >= 0x010800
  /**
   * Get tracker name of a module file.
   *
   * @return tracker name, null if not found.
   */
  QString getTrackerName() const;
#endif

#if TAGLIB_VERSION >= 0x010600 && defined TAGLIB_WITH_MP4
  /**
   * Set a frame in an MP4 tag.
   * @param frame frame to set
   * @param mp4Tag MP4 tag
   */
  void setMp4Frame(const Frame& frame, TagLib::MP4::Tag* mp4Tag);
#endif

  /**
   * Get the format of a tag.
   *
   * @param tag tag, 0 if no tag available
   * @param type the tag type is returned here
   *
   * @return string describing format of tag,
   *         e.g. "ID3v1.1", "ID3v2.3", "Vorbis", "APE",
   *         QString::null if unknown.
   */
  static QString getTagFormat(const TagLib::Tag* tag, TagType& type);

#if TAGLIB_VERSION < 0x010800
  /**
   * Register open TagLib file, so that the number of open files can be limited.
   * If the number of open files exceeds a limit, files are closed.
   *
   * @param tagLibFile new open file to be registered
   */
  static void registerOpenFile(TagLibFile* tagLibFile);

  /**
   * Deregister open TagLib file.
   *
   * @param tagLibFile file which is no longer open
   */
  static void deregisterOpenFile(TagLibFile* tagLibFile);
#endif

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

  bool m_tagInformationRead;
  bool m_hasTagV1;
  bool m_hasTagV2;
  bool m_isTagV1Supported;

  bool m_fileRead;           /**< true if file has been read */

  TagLib::FileRef m_fileRef; /**< file reference */
  TagLib::Tag* m_tagV1;      /**< ID3v1 tags */
  TagLib::Tag* m_tagV2;      /**< ID3v2 tags */
#if TAGLIB_VERSION >= 0x010800
  FileIOStream* m_stream;
  int m_id3v2Version;        /**< 3 for ID3v2.3, 4 for ID3v2.4, 0 if none */
#endif
  int m_activatedFeatures;   /**< TF_ID3v23, TF_ID3v24, or 0 */

  /* Cached information updated in readTags() */
  unsigned m_duration;
  TagType m_tagTypeV1;
  TagType m_tagTypeV2;
  QString m_tagFormatV1;
  QString m_tagFormatV2;
  QString m_fileExtension;
  DetailInfo m_detailInfo;

#if TAGLIB_VERSION >= 0x010700
  class Pictures : public QList<Frame> {
  public:
    Pictures() : m_read(false) {}
    bool isRead() const { return m_read; }
    void setRead(bool read) { m_read = read; }

  private:
    bool m_read;
  };

  Pictures m_pictures;
#endif

  /** default text encoding */
  static TagLib::String::Type s_defaultTextEncoding;

#if TAGLIB_VERSION < 0x010800
  /** list of TagLib files with open file descriptor */
  static QList<TagLibFile*> s_openFiles;
#endif
};

#endif // TAGLIBFILE_H
