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
   * Free resources allocated when calling readTags().
   *
   * @param force true to force clearing even if the tags are modified
   */
  virtual void clearTags(bool force);

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
   * @return true if a V1 tag is available.
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
   * @return string describing format of tag 1,
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
   * Close file handle which is held open by the TagLib object.
   */
  virtual void closeFileHandle();

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
    TT_Asf,
    TT_Info
  };

  TagLibFile(const TagLibFile&);
  TagLibFile& operator=(const TagLibFile&);

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
   * This method should be called before accessing m_fileRef, m_tag.
   *
   * @param force true to force reopening of file even if it is already open
   */
  void makeFileOpen(bool force = false) const;

  /**
   * Create tag if it does not already exist so that it can be set.
   *
   * @param tagNr tag number
   * @return true if tag can be set.
   */
  bool makeTagSettable(Frame::TagNumber tagNr);

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

  /**
   * Set m_id3v2Version to 3 or 4 from tag if it exists, else to 0.
   * @param id3v2Tag ID3v2 tag
   */
  void setId3v2VersionFromTag(TagLib::ID3v2::Tag* id3v2Tag);

  /**
   * Set m_id3v2Version from given value (3 or 4) or use default from
   * configuration if not already set to 3 or 4.
   * @param id3v2Version 3 or 4 to force version, 0 to use existing version
   * or default
   */
  void setId3v2VersionOrDefault(int id3v2Version);
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

  static const int NUM_TAGS = 3;

  bool m_tagInformationRead;
  bool m_hasTag[NUM_TAGS];
  bool m_isTagSupported[NUM_TAGS];

  bool m_fileRead;           /**< true if file has been read */

  TagLib::FileRef m_fileRef; /**< file reference */
  TagLib::Tag* m_tag[NUM_TAGS];
#if TAGLIB_VERSION >= 0x010800
  FileIOStream* m_stream;
  int m_id3v2Version;        /**< 3 for ID3v2.3, 4 for ID3v2.4, 0 if none */
#endif
  int m_activatedFeatures;   /**< TF_ID3v23, TF_ID3v24, or 0 */

  /* Cached information updated in readTags() */
  unsigned m_duration;
  TagType m_tagType[NUM_TAGS];
  QString m_tagFormat[NUM_TAGS];
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
