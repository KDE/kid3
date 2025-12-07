/**
 * \file taglibfileiostream.h
 * File stream reducing the number of open file descriptors.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Nov 2025
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

#include <QList>
#include <tiostream.h>

#include "taglibutils.h"

namespace TagLib {
  class FileStream;
  class File;
}

class QString;

/**
 * Wrapper around TagLib::FileStream which reduces the number of open file
 * descriptors.
 *
 * Using streams, closing the file descriptor is also possible for modified
 * files because the TagLib file does not have to be deleted just to close the
 * file descriptor.
 */
class FileIOStream : public TagLib::IOStream {
public:
  /**
   * Constructor.
   * @param fileName path to file
   */
  explicit FileIOStream(const QString& fileName);

  /**
   * Destructor.
   */
  ~FileIOStream() override;

  FileIOStream(const FileIOStream&) = delete;
  FileIOStream& operator=(const FileIOStream&) = delete;

  /**
   * Close the file handle.
   * The file will automatically be opened again if needed.
   */
  void closeFileHandle();

  /**
   * Change the file name.
   * Can be used to modify the file name when it has changed because a path
   * component was renamed.
   * @param fileName path to file
   */
  void setName(const QString& fileName);

  // Reimplemented from TagLib::IOStream, delegate to TagLib::FileStream.
  /** File name in local file system encoding. */
  TagLib::FileName name() const override;
#if TAGLIB_VERSION >= 0x020000
  /** Read block of size @a length at current pointer. */
  TagLib::ByteVector readBlock(size_t length) override;
  /**
   * Insert @a data at position @a start in the file overwriting @a replace
   * bytes of the original content.
   */
  void insert(const TagLib::ByteVector &data,
              taglib_uoffset_t start, size_t replace) override;
  /** Remove block starting at @a start for @a length bytes. */
  void removeBlock(taglib_uoffset_t start, size_t length) override;
#else
  /** Read block of size @a length at current pointer. */
  TagLib::ByteVector readBlock(ulong length) override;
  /**
   * Insert @a data at position @a start in the file overwriting @a replace
   * bytes of the original content.
   */
  void insert(const TagLib::ByteVector &data,
              taglib_uoffset_t start = 0, ulong replace = 0) override;
  /** Remove block starting at @a start for @a length bytes. */
  void removeBlock(taglib_uoffset_t start = 0, ulong length = 0) override;
#endif
  /** Write block @a data at current pointer. */
  void writeBlock(const TagLib::ByteVector &data) override;
  /** True if the file is read only. */
  bool readOnly() const override;
  /** Check if open in constructor succeeded. */
  bool isOpen() const override;
  /** Move I/O pointer to @a offset in the file from position @a p. */
  void seek(taglib_offset_t offset, Position p) override;
  /** Reset the end-of-file and error flags on the file. */
  void clear() override;
  /** Current offset within the file. */
  taglib_offset_t tell() const override;
  /** Length of the file. */
  taglib_offset_t length() override;
  /** Truncate the file to @a length. */
  void truncate(taglib_offset_t length) override;

  /**
   * Create a TagLib file for a stream.
   * TagLib::FileRef::create() adapted for IOStream.
   * @param stream stream with name() of which the extension is used to deduce
   * the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* create(IOStream* stream);

  /**
   * Register handlers for supported audio formats.
   *
   * @param formats format supporters
   */
  static void registerFormatSupport(const QList<TagLibFormatSupport*>& formats);

private:
  /**
   * Open file handle, is called by operations which need a file handle.
   *
   * @return true if file is open.
   */
  bool openFileHandle() const;

  /**
   * Create a TagLib file for a stream.
   * @param stream stream with name() of which the extension is used to deduce
   * the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* createFromExtension(IOStream* stream);

  /**
   * Create a TagLib file for a stream.
   * @param stream stream
   * @param ext uppercase extension used to deduce the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* createFromExtension(TagLib::IOStream* stream,
                                           const TagLib::String& ext);

  /**
   * Create a TagLib file for a stream.
   * @param stream stream where the contents are used to deduce the file type
   * @return file, 0 if not supported.
   */
  static TagLib::File* createFromContents(IOStream* stream);

  /**
   * Register open files, so that the number of open files can be limited.
   * If the number of open files exceeds a limit, files are closed.
   *
   * @param stream new open file to be registered
   */
  static void registerOpenFile(FileIOStream* stream);

  /**
   * Deregister open file.
   *
   * @param stream file which is no longer open
   */
  static void deregisterOpenFile(FileIOStream* stream);

#ifdef Q_OS_WIN32
  wchar_t* m_fileName;
#else
  char* m_fileName;
#endif
  TagLib::FileStream* m_fileStream;
  long m_offset;

  /** list of file streams with open file descriptor */
  static QList<FileIOStream*> s_openFiles;
  /** format support */
  static QList<TagLibFormatSupport*> s_formats;
};
