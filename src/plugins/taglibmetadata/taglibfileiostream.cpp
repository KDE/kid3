/**
 * \file taglibfileiostream.cpp
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

#include "taglibfileiostream.h"
#include <QFile>
#include <QList>
#include <QMimeDatabase>
#include <tfilestream.h>
#include "taglibformatsupport.h"

QList<FileIOStream*> FileIOStream::s_openFiles;
QList<TagLibFormatSupport*> FileIOStream::s_formats;

FileIOStream::FileIOStream(const QString& fileName)
  : m_fileName(nullptr), m_fileStream(nullptr), m_offset(0)
{
  setName(fileName);
}

FileIOStream::~FileIOStream()
{
  deregisterOpenFile(this);
  delete m_fileStream;
  delete [] m_fileName;
}

bool FileIOStream::openFileHandle() const
{
  if (!m_fileStream) {
    auto self = const_cast<FileIOStream*>(this);
    self->m_fileStream =
        new TagLib::FileStream(TagLib::FileName(m_fileName));
    if (!self->m_fileStream->isOpen()) {
      delete self->m_fileStream;
      self->m_fileStream = nullptr;
      return false;
    }
    if (m_offset > 0) {
      m_fileStream->seek(m_offset);
    }
    registerOpenFile(self);
  }
  return true;
}

void FileIOStream::closeFileHandle()
{
  if (m_fileStream) {
    m_offset = m_fileStream->tell();
    delete m_fileStream;
    m_fileStream = nullptr;
    deregisterOpenFile(this);
  }
}

void FileIOStream::setName(const QString& fileName)
{
  delete m_fileName;
#ifdef Q_OS_WIN32
  int fnLen = fileName.length();
  m_fileName = new wchar_t[fnLen + 1];
  m_fileName[fnLen] = 0;
  fileName.toWCharArray(m_fileName);
#else
  QByteArray fn = QFile::encodeName(fileName);
  m_fileName = new char[fn.size() + 1];
  qstrcpy(m_fileName, fn.data());
#endif
}

TagLib::FileName FileIOStream::name() const
{
  if (m_fileStream) {
    return m_fileStream->name();
  }
  return TagLib::FileName(m_fileName);
}

TagLib::ByteVector FileIOStream::readBlock(
#if TAGLIB_VERSION >= 0x020000
    size_t length
#else
    ulong length
#endif
)
{
  if (openFileHandle()) {
    return m_fileStream->readBlock(length);
  }
  return {};
}

void FileIOStream::writeBlock(const TagLib::ByteVector &data)
{
  if (openFileHandle()) {
    m_fileStream->writeBlock(data);
  }
}

void FileIOStream::insert(const TagLib::ByteVector &data,
                          taglib_uoffset_t start,
#if TAGLIB_VERSION >= 0x020000
                          size_t replace
#else
                          ulong replace
#endif
)
{
  if (openFileHandle()) {
    m_fileStream->insert(data, start, replace);
  }
}

void FileIOStream::removeBlock(taglib_uoffset_t start,
#if TAGLIB_VERSION >= 0x020000
                               size_t length
#else
                               ulong length
#endif
)
{
  if (openFileHandle()) {
    m_fileStream->removeBlock(start, length);
  }
}

bool FileIOStream::readOnly() const
{
  if (openFileHandle()) {
    return m_fileStream->readOnly();
  }
  return true;
}

bool FileIOStream::isOpen() const
{
  if (m_fileStream) {
    return m_fileStream->isOpen();
  }
  return true;
}

void FileIOStream::seek(taglib_offset_t offset, Position p)
{
  if (openFileHandle()) {
    m_fileStream->seek(offset, p);
  }
}

void FileIOStream::clear()
{
  if (openFileHandle()) {
    m_fileStream->clear();
  }
}

taglib_offset_t FileIOStream::tell() const
{
  if (openFileHandle()) {
    return m_fileStream->tell();
  }
  return 0;
}

taglib_offset_t FileIOStream::length()
{
  if (openFileHandle()) {
    return m_fileStream->length();
  }
  return 0;
}

void FileIOStream::truncate(taglib_offset_t length)
{
  if (openFileHandle()) {
    m_fileStream->truncate(length);
  }
}

TagLib::File* FileIOStream::create(TagLib::IOStream* stream)
{
  TagLib::File* file = createFromExtension(stream);
  if (file && !file->isValid()) {
    delete file;
    file = nullptr;
  }
  if (!file) {
    file = createFromContents(stream);
  }
  return file;
}

TagLib::File* FileIOStream::createFromExtension(TagLib::IOStream* stream)
{
#ifdef Q_OS_WIN32
  TagLib::String fn = stream->name().toString();
#else
  TagLib::String fn = stream->name();
#endif
  const int extPos = fn.rfind(".");
  return extPos != -1
      ? createFromExtension(stream, fn.substr(extPos + 1).upper())
      : nullptr;
}

TagLib::File* FileIOStream::createFromExtension(TagLib::IOStream* stream,
                                                const TagLib::String& ext)
{
  for (auto format : s_formats) {
    if (TagLib::File* file = format->createFromExtension(stream, ext)) {
      return file;
    }
  }
  return nullptr;
}

TagLib::File* FileIOStream::createFromContents(TagLib::IOStream* stream)
{
  static const struct ExtensionForMimeType {
    const char* mime;
    const char* ext;
  } extensionForMimeType[] = {
    { "application/ogg", "OGG" },
    { "application/vnd.ms-asf", "WMA" },
    { "audio/aac", "AAC" },
    { "audio/flac", "FLAC" },
    { "audio/mp4", "MP4" },
    { "audio/mpeg", "MP3" },
    { "audio/x-aiff", "AIFF" },
    { "audio/x-ape", "APE" },
    { "audio/x-flac+ogg", "OGG" },
    { "audio/x-it", "IT" },
    { "audio/x-musepack", "MPC" },
    { "audio/x-opus+ogg", "OPUS" },
    { "audio/x-s3m", "S3M" },
    { "audio/x-speex+ogg", "SPX" },
    { "audio/x-tta", "TTA" },
    { "audio/x-vorbis+ogg", "OGG" },
    { "audio/x-wav", "WAV" },
    { "audio/x-wavpack", "WV" },
    { "audio/x-xm", "XM" },
    { "video/mp4", "MP4" }
  };

  static QMap<QString, TagLib::String> mimeExtMap;
  if (mimeExtMap.empty()) {
    // first time initialization
    for (const auto& [mime, ext] : extensionForMimeType) {
      mimeExtMap.insert(QString::fromLatin1(mime), ext);
    }
  }

  stream->seek(0);
  TagLib::ByteVector bv = stream->readBlock(4096);
  stream->seek(0);
  QMimeDatabase mimeDb;
  auto mimeType =
      mimeDb.mimeTypeForData(QByteArray(bv.data(), static_cast<int>(bv.size())));
  if (TagLib::String ext = mimeExtMap.value(mimeType.name()); !ext.isEmpty()) {
    return createFromExtension(stream, ext);
  }
  return nullptr;
}

void FileIOStream::registerOpenFile(FileIOStream* stream)
{
  if (s_openFiles.contains(stream))
    return;

  if (int numberOfFilesToClose = static_cast<int>(s_openFiles.size()) - 15;
      numberOfFilesToClose > 5) {
    for (auto it = s_openFiles.begin(); it != s_openFiles.end(); ++it) { // clazy:exclude=detaching-member
      (*it)->closeFileHandle();
      if (--numberOfFilesToClose <= 0) {
        break;
      }
    }
  }
  s_openFiles.append(stream);
}

/**
 * Deregister open file.
 *
 * @param stream file which is no longer open
 */
void FileIOStream::deregisterOpenFile(FileIOStream* stream)
{
  s_openFiles.removeAll(stream);
}

void FileIOStream::registerFormatSupport(const QList<TagLibFormatSupport*>& formats)
{
  s_formats = formats;
}
