/***************************************************************************
    copyright            : (C) 2016 by Damien Plisson, Audirvana
    email                : damien78@audirvana.com
    copyright            : (C) 2022 by Thomas Hou
    email                : hytcqq@gmail.com
***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <cstdint>

// Kid3: Remove taglib/ from includes
#include <id3v2tag.h>
#include <id3v2header.h>
#include <tpropertymap.h>
// Kid3: Use debug() instead of std::cerr
#include <tdebug.h>

#include "dsdifffile.h"

/** TagLib version in with 8 bits for major, minor and patch version. */
#define TAGLIB_VERSION (((TAGLIB_MAJOR_VERSION) << 16) + \
                        ((TAGLIB_MINOR_VERSION) << 8) + (TAGLIB_PATCH_VERSION))

namespace
{
  struct Chunk64
  {
    TagLib::ByteVector name;
    unsigned long long offset;
    unsigned long long size;
    char padding;
  };

  typedef std::vector<Chunk64> ChunkList;

  int chunkIndex(const ChunkList &chunks, const TagLib::ByteVector &id)
  {
    for(size_t i = 0; i < chunks.size(); i++) {
      if(chunks[i].name == id)
        return i;
    }

    return -1;
  }

  bool isValidChunkID(const TagLib::ByteVector &name)
  {
    if(name.size() != 4)
      return false;

    for(int i = 0; i < 4; i++) {
      if(name[i] < 32)
        return false;
    }

    return true;
  }

}

class DSDIFFFile::FilePrivate
{
public:
FilePrivate(const TagLib::ID3v2::FrameFactory *frameFactory
        = TagLib::ID3v2::FrameFactory::instance())
  : ID3v2FrameFactory(frameFactory),
    properties(nullptr),
    tag(nullptr),
    id3v2TagChunkID("ID3 "),
    size(0),
    childChunkIndex(-1),
    duplicateID3V2chunkIndex(-1),
    isID3InPropChunk(false),
    hasID3v2(false)
  {
  }

  ~FilePrivate()
  {
    delete properties;
    delete tag;
  }

  const TagLib::ID3v2::FrameFactory *ID3v2FrameFactory;
  DSDIFFProperties *properties;
  TagLib::ID3v2::Tag *tag;

  TagLib::ByteVector type;
  TagLib::ByteVector format;
  TagLib::ByteVector id3v2TagChunkID;

  ChunkList chunks;
  ChunkList childChunks;

  unsigned long long size;
  int childChunkIndex;
  /*
   * ID3 chunks are present. This is then the index of the one in PROP chunk that
   * will be removed upon next save to remove duplicates.
   */
  int duplicateID3V2chunkIndex;
  /*
   * Two possibilities can be found: ID3V2 chunk inside PROP chunk or at root level
   */
  bool isID3InPropChunk;
  bool hasID3v2;
};

////////////////////////////////////////////////////////////////////////////////
// static members
////////////////////////////////////////////////////////////////////////////////

bool DSDIFFFile::isSupported(TagLib::IOStream *stream)
{
  // A DSDIFF file has to start with "FRM8????????DSD ".
  const long originalPosition = stream->tell();
  stream->seek(0);
  const TagLib::ByteVector id = stream->readBlock(16);
  stream->seek(originalPosition);
  return id.startsWith("FRM8") && id.containsAt("DSD ", 12);
}

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSDIFFFile::DSDIFFFile(TagLib::FileName file,
    bool readProperties,
    TagLib::AudioProperties::ReadStyle propertiesStyle)
 : TagLib::File(file)
{
  d = new FilePrivate;
  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSDIFFFile::DSDIFFFile(TagLib::FileName file,
    const TagLib::ID3v2::FrameFactory *frameFactory,
    bool readProperties,
    TagLib::AudioProperties::ReadStyle propertiesStyle)
  : TagLib::File(file)
{
  d = new FilePrivate(frameFactory);

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSDIFFFile::DSDIFFFile(TagLib::IOStream *stream,
    const TagLib::ID3v2::FrameFactory *frameFactory,
    bool readProperties,
    TagLib::AudioProperties::ReadStyle propertiesStyle)
  : TagLib::File(stream)
{
  d = new FilePrivate(frameFactory);

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSDIFFFile::~DSDIFFFile()
{
  delete d;
}

TagLib::Tag *DSDIFFFile::tag() const
{
  return d->tag;
}

TagLib::ID3v2::Tag *DSDIFFFile::ID3v2Tag() const
{
  return d->tag;
}

bool DSDIFFFile::hasID3v2Tag() const
{
  return d->hasID3v2;
}

TagLib::PropertyMap DSDIFFFile::properties() const
{
  if(d->hasID3v2)
    return d->tag->properties();

  return TagLib::PropertyMap();
}

void DSDIFFFile::removeUnsupportedProperties(const TagLib::StringList &unsupported)
{
  if(d->hasID3v2)
    d->tag->removeUnsupportedProperties(unsupported);
}

TagLib::PropertyMap DSDIFFFile::setProperties(const TagLib::PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

TagLib::AudioProperties *DSDIFFFile::audioProperties() const
{
  return d->properties;
}

bool DSDIFFFile::save()
{
  return save(4);
}

bool DSDIFFFile::save(int id3v2Version)
{
  if(readOnly()) {
    debug("DSDIFFFile::save() -- File is read only.");
    return false;
  }

  if(!isValid()) {
    debug("DSDIFFFile::save() -- Trying to save invalid file.");
    return false;
  }

  // First: save ID3V2 chunk

  if(TagLib::ID3v2::Tag *id3v2Tag = d->tag) {
    if(d->isID3InPropChunk) {
      if(!id3v2Tag->isEmpty()) {
#if TAGLIB_VERSION >= 0x010c00
        TagLib::ByteVector id3v2_v = id3v2Tag->render(
          id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3);
#else
        TagLib::ByteVector id3v2_v = id3v2Tag->render(id3v2Version);
#endif
        setChildChunkData(d->id3v2TagChunkID, id3v2_v);
        d->hasID3v2 = true;
      }
      else {
        // Empty tag: remove it
        setChildChunkData(d->id3v2TagChunkID, TagLib::ByteVector());
        d->hasID3v2 = false;
      }
    }
    else {
      if(!id3v2Tag->isEmpty()) {
#if TAGLIB_VERSION >= 0x010c00
        TagLib::ByteVector id3v2_v = id3v2Tag->render(
          id3v2Version == 4 ? TagLib::ID3v2::v4 : TagLib::ID3v2::v3);
#else
        TagLib::ByteVector id3v2_v = id3v2Tag->render(id3v2Version);
#endif
        setRootChunkData(d->id3v2TagChunkID, id3v2_v);
        d->hasID3v2 = true;
      }
      else {
        // Empty tag: remove it
        setRootChunkData(d->id3v2TagChunkID, TagLib::ByteVector());
        d->hasID3v2 = false;
      }
    }
  }

  // Second: remove the duplicate ID3V2 chunk (inside PROP chunk) if any

  if(d->duplicateID3V2chunkIndex >= 0) {
    setChildChunkData(d->duplicateID3V2chunkIndex, TagLib::ByteVector());
    d->duplicateID3V2chunkIndex = -1;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSDIFFFile::removeRootChunk(unsigned int i)
{
  unsigned long long chunkSize = d->chunks[i].size + d->chunks[i].padding + 12;

  d->size -= chunkSize;
  insert(TagLib::ByteVector::fromLongLong(d->size), 4, 8);

  removeBlock(d->chunks[i].offset - 12, chunkSize);

  // Update the internal offsets

  for(unsigned long r = i + 1; r < d->chunks.size(); r++)
    d->chunks[r].offset = d->chunks[r - 1].offset + 12
      + d->chunks[r - 1].size + d->chunks[r - 1].padding;

  d->chunks.erase(d->chunks.begin() + i);
}

void DSDIFFFile::removeRootChunk(const TagLib::ByteVector &id)
{
  if(int i = chunkIndex(d->chunks, id); i >= 0)
    removeRootChunk(i);
}

void DSDIFFFile::setRootChunkData(unsigned int i, const TagLib::ByteVector &data)
{
  if(data.isEmpty()) {
    removeRootChunk(i);
    return;
  }

  // Non null data: update chunk
  // First we update the global size

  d->size += ((data.size() + 1) & ~1) - (d->chunks[i].size + d->chunks[i].padding);
  insert(TagLib::ByteVector::fromLongLong(d->size), 4, 8);

  // Now update the specific chunk

  writeChunk(d->chunks[i].name,
             data,
             d->chunks[i].offset - 12,
             d->chunks[i].size + d->chunks[i].padding + 12);

  d->chunks[i].size = data.size();
  d->chunks[i].padding = data.size() & 0x01 ? 1 : 0;

  // Finally update the internal offsets

  updateRootChunksStructure(i + 1);
}

void DSDIFFFile::setRootChunkData(const TagLib::ByteVector &name,
                                  const TagLib::ByteVector &data)
{
  if(d->chunks.size() == 0) {
    debug("DSDIFFFile::setPropChunkData - No valid chunks found.");
    return;
  }

  int i = chunkIndex(d->chunks, name);

  if(i >= 0) {
    setRootChunkData(i, data);
    return;
  }

  // Couldn't find an existing chunk, so let's create a new one.
  i = d->chunks.size() - 1;
  unsigned long offset = d->chunks[i].offset + d->chunks[i].size + d->chunks[i].padding;

  // First we update the global size
  d->size += (offset & 1) + ((data.size() + 1) & ~1) + 12;
  insert(TagLib::ByteVector::fromLongLong(d->size), 4, 8);

  // Now add the chunk to the file
  writeChunk(name,
             data,
             offset,
             std::max<unsigned long long>(0, length() - offset),
             (offset & 1) ? 1 : 0);

  Chunk64 chunk;
  chunk.name = name;
  chunk.size = data.size();
  chunk.offset = offset + 12;
  chunk.padding = (data.size() & 0x01) ? 1 : 0;

  d->chunks.push_back(chunk);
}

void DSDIFFFile::removeChildChunk(unsigned int i)
{
  ChunkList &childChunks = d->childChunks;

  // Update global size

  unsigned long long removedChunkTotalSize = childChunks[i].size + childChunks[i].padding + 12;
  d->size -= removedChunkTotalSize;
  insert(TagLib::ByteVector::fromLongLong(d->size), 4, 8);

  // Update child chunk size

  d->chunks[d->childChunkIndex].size -= removedChunkTotalSize;
  insert(TagLib::ByteVector::fromLongLong(d->chunks[d->childChunkIndex].size),
         d->chunks[d->childChunkIndex].offset - 8, 8);
  // Remove the chunk

  removeBlock(childChunks[i].offset - 12, removedChunkTotalSize);

  // Update the internal offsets
  // For child chunks

  if(i + 1 < childChunks.size()) {
    childChunks[i + 1].offset = childChunks[i].offset;
    i++;
    for(i++; i < childChunks.size(); i++)
      childChunks[i].offset = childChunks[i - 1].offset + 12
        + childChunks[i - 1].size + childChunks[i - 1].padding;
  }

  // And for root chunks

  for(i = d->childChunkIndex + 1; i < d->chunks.size(); i++)
    d->chunks[i].offset = d->chunks[i - 1].offset + 12
      + d->chunks[i - 1].size + d->chunks[i - 1].padding;

  childChunks.erase(childChunks.begin() + i);
}

void DSDIFFFile::setChildChunkData(unsigned int i,
                                   const TagLib::ByteVector &data)
{
  ChunkList &childChunks = d->childChunks;

  if(data.isEmpty()) {
    removeChildChunk(i);
    return;
  }

  // Non null data: update chunk
  // First we update the global size

  d->size += ((data.size() + 1) & ~1) - (childChunks[i].size + childChunks[i].padding);

  insert(TagLib::ByteVector::fromLongLong(d->size), 4, 8);

  // And the PROP chunk size

  d->chunks[d->childChunkIndex].size +=
    ((data.size() + 1) & ~1) - (childChunks[i].size + childChunks[i].padding);
  insert(TagLib::ByteVector::fromLongLong(d->chunks[d->childChunkIndex].size),
         d->chunks[d->childChunkIndex].offset - 8, 8);

  // Now update the specific chunk

  writeChunk(childChunks[i].name,
             data,
             childChunks[i].offset - 12,
             childChunks[i].size + childChunks[i].padding + 12);

  childChunks[i].size = data.size();
  childChunks[i].padding = (data.size() & 0x01) ? 1 : 0;

  // Now update the internal offsets
  // For child Chunks
  for(i++; i < childChunks.size(); i++)
    childChunks[i].offset = childChunks[i - 1].offset + 12
                            + childChunks[i - 1].size + childChunks[i - 1].padding;

  // And for root chunks
  updateRootChunksStructure(d->childChunkIndex + 1);
}

void DSDIFFFile::setChildChunkData(const TagLib::ByteVector &name,
                                   const TagLib::ByteVector &data)
{
  ChunkList &childChunks = d->childChunks;

  if(childChunks.size() == 0) {
    debug("DSDIFFFile::setPropChunkData - No valid chunks found.");
    return;
  }

  for(unsigned int i = 0; i < childChunks.size(); i++) {
    if(childChunks[i].name == name) {
      setChildChunkData(i, data);
      return;
    }
  }

  // Do not attempt to remove a non existing chunk

  if(data.isEmpty())
    return;

  // Couldn't find an existing chunk, so let's create a new one.

  unsigned int i = childChunks.size() - 1;
  unsigned long offset = childChunks[i].offset + childChunks[i].size + childChunks[i].padding;

  // First we update the global size

  d->size += (offset & 1) + ((data.size() + 1) & ~1) + 12;
  insert(TagLib::ByteVector::fromLongLong(d->size), 4, 8);

  // And the child chunk size

  d->chunks[d->childChunkIndex].size += (offset & 1)
    + ((data.size() + 1) & ~1) + 12;
  insert(TagLib::ByteVector::fromLongLong(d->chunks[d->childChunkIndex].size),
         d->chunks[d->childChunkIndex].offset - 8, 8);

  // Now add the chunk to the file

  unsigned long long nextRootChunkIdx = length();
  if(d->childChunkIndex + 1 < static_cast<int>(d->chunks.size()))
    nextRootChunkIdx = d->chunks[d->childChunkIndex + 1].offset - 12;

  writeChunk(name, data, offset,
             std::max<unsigned long long>(0, nextRootChunkIdx - offset),
             (offset & 1) ? 1 : 0);

  // For root chunks

  updateRootChunksStructure(d->childChunkIndex + 1);

  Chunk64 chunk;
  chunk.name = name;
  chunk.size = data.size();
  chunk.offset = offset + 12;
  chunk.padding = (data.size() & 0x01) ? 1 : 0;

  childChunks.push_back(chunk);
}

void DSDIFFFile::updateRootChunksStructure(unsigned int startingChunk)
{
  for(unsigned int i = startingChunk; i < d->chunks.size(); i++)
    d->chunks[i].offset = d->chunks[i - 1].offset + 12
      + d->chunks[i - 1].size + d->chunks[i - 1].padding;

  // Update child chunks structure as well

  if(d->childChunkIndex >= static_cast<int>(startingChunk)) {
    if(ChunkList &childChunksToUpdate = d->childChunks;
       childChunksToUpdate.size() > 0) {
      childChunksToUpdate[0].offset = d->chunks[d->childChunkIndex].offset + 12;
      for(unsigned int i = 1; i < childChunksToUpdate.size(); i++)
        childChunksToUpdate[i].offset = childChunksToUpdate[i - 1].offset + 12
          + childChunksToUpdate[i - 1].size + childChunksToUpdate[i - 1].padding;
    }
  }
}

void DSDIFFFile::read(bool readProperties, TagLib::AudioProperties::ReadStyle propertiesStyle)
{
  d->type = readBlock(4);
  d->size = readBlock(8).toLongLong();
  d->format = readBlock(4);

  // + 12: chunk header at least, fix for additional junk bytes

  while(tell() + 12 <= length()) {
    TagLib::ByteVector chunkName = readBlock(4);
    unsigned long long chunkSize = readBlock(8).toLongLong();

    if(!isValidChunkID(chunkName)) {
      debug("DSDIFFFile::read() -- Chunk '" + chunkName + "' has invalid ID");
      setValid(false);
      break;
    }

    if(static_cast<unsigned long long>(tell()) + chunkSize >
       static_cast<unsigned long long>(length())) {
      debug("DSDIFFFile::read() -- Chunk '" + chunkName
            + "' has invalid size (larger than the file size)");
      setValid(false);
      break;
    }

    Chunk64 chunk;
    chunk.name = chunkName;
    chunk.size = chunkSize;
    chunk.offset = tell();

    seek(chunk.size, Current);

    // Check padding

    chunk.padding = 0;
    if(long uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
      if(TagLib::ByteVector iByte = readBlock(1);
         iByte.size() != 1 || iByte[0] != 0)
        // Not well formed, re-seek
        seek(uPosNotPadded, Beginning);
      else
        chunk.padding = 1;
    }
    d->chunks.push_back(chunk);
  }

  // For DSD uncompressed
  unsigned long long lengthDSDSamplesTimeChannels = 0;
  // For computing bitrate
  unsigned long long audioDataSizeinBytes = 0;
  // For DST compressed frames
  unsigned long dstNumFrames = 0;
  // For DST compressed frames
  unsigned short dstFrameRate = 0;

  for(unsigned int i = 0; i < d->chunks.size(); i++) {
    if(d->chunks[i].name == "DSD ") {
      lengthDSDSamplesTimeChannels = d->chunks[i].size * 8;
      audioDataSizeinBytes = d->chunks[i].size;
    }
    else if(d->chunks[i].name == "DST ") {
      // Now decode the chunks inside the DST chunk to read the DST Frame Information one
      long long dstChunkEnd = d->chunks[i].offset + d->chunks[i].size;
      seek(d->chunks[i].offset);

      audioDataSizeinBytes = d->chunks[i].size;

      while(tell() + 12 <= dstChunkEnd) {
        TagLib::ByteVector dstChunkName = readBlock(4);
        long long dstChunkSize = readBlock(8).toLongLong();

        if(!isValidChunkID(dstChunkName)) {
          debug("DSDIFFFile::read() -- DST Chunk '" + dstChunkName + "' has invalid ID");
          setValid(false);
          break;
        }

        if(static_cast<long long>(tell()) + dstChunkSize > dstChunkEnd) {
          debug("DSDIFFFile::read() -- DST Chunk '" + dstChunkName
                + "' has invalid size (larger than the DST chunk)");
          setValid(false);
          break;
        }

        if(dstChunkName == "FRTE") {
          // Found the DST frame information chunk
          dstNumFrames = readBlock(4).toUInt();
          dstFrameRate = readBlock(2).toUShort();
          // Found the wanted one, no need to look at the others
          break;
        }

        seek(dstChunkSize, Current);

        // Check padding
        if(long uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
          if(TagLib::ByteVector iByte = readBlock(1);
             iByte.size() != 1 || iByte[0] != 0)
            // Not well formed, re-seek
            seek(uPosNotPadded, Beginning);
        }
      }
    }
    else if(d->chunks[i].name == "PROP") {
      d->childChunkIndex = i;
      // Now decodes the chunks inside the PROP chunk
      long long propChunkEnd = d->chunks[i].offset + d->chunks[i].size;
      // +4 to remove the 'SND ' marker at beginning of 'PROP' chunk
      seek(d->chunks[i].offset + 4);
      while(tell() + 12 <= propChunkEnd) {
        TagLib::ByteVector propChunkName = readBlock(4);
        long long propChunkSize = readBlock(8).toLongLong();

        if(!isValidChunkID(propChunkName)) {
          debug("DSDIFFFile::read() -- PROP Chunk '" + propChunkName + "' has invalid ID");
          setValid(false);
          break;
        }

        if(static_cast<long long>(tell()) + propChunkSize > propChunkEnd) {
          debug("DSDIFFFile::read() -- PROP Chunk '" + propChunkName
                + "' has invalid size (larger than the PROP chunk)");
          setValid(false);
          break;
        }

        Chunk64 chunk;
        chunk.name = propChunkName;
        chunk.size = propChunkSize;
        chunk.offset = tell();

        seek(chunk.size, Current);

        // Check padding
        chunk.padding = 0;
        if(long uPosNotPadded = tell(); (uPosNotPadded & 0x01) != 0) {
          if(TagLib::ByteVector iByte = readBlock(1);
             iByte.size() != 1 || iByte[0] != 0)
            // Not well formed, re-seek
            seek(uPosNotPadded, Beginning);
          else
            chunk.padding = 1;
        }
        d->childChunks.push_back(chunk);
      }
    }
    else if(d->chunks[i].name == "ID3 " || d->chunks[i].name == "id3 ") {
      d->id3v2TagChunkID = d->chunks[i].name;
      d->tag = new TagLib::ID3v2::Tag(this, d->chunks[i].offset);
      d->isID3InPropChunk = false;
      d->hasID3v2 = true;
    }
  }

  if(!isValid())
    return;

  if(d->childChunkIndex < 0) {
    debug("DSDIFFFile::read() -- no PROP chunk found");
    setValid(false);
    return;
  }

  // Read properties

  unsigned int sampleRate = 0;
  unsigned short channels = 0;

  for(unsigned int i = 0; i < d->childChunks.size(); i++) {
    if(d->childChunks[i].name == "ID3 " ||
       d->childChunks[i].name == "id3 ") {
      if(d->hasID3v2) {
        d->duplicateID3V2chunkIndex = i;
        // ID3V2 tag has already been found at root level
        continue;
      }
      d->id3v2TagChunkID = d->childChunks[i].name;
      d->tag = new TagLib::ID3v2::Tag(this, d->childChunks[i].offset);
      d->isID3InPropChunk = true;
      d->hasID3v2 = true;
    }
    else if(d->childChunks[i].name == "FS  ") {
      // Sample rate
      seek(d->childChunks[i].offset);
      sampleRate = readBlock(4).toUInt();
    }
    else if(d->childChunks[i].name == "CHNL") {
      // Channels
      seek(d->childChunks[i].offset);
      channels = readBlock(2).toShort();
    }
  }

  if(readProperties) {
    if(lengthDSDSamplesTimeChannels == 0) {
      // DST compressed signal : need to compute length of DSD uncompressed frames
      if(dstFrameRate > 0)
        lengthDSDSamplesTimeChannels = static_cast<unsigned long long>(dstNumFrames) *
                                       static_cast<unsigned long long>(sampleRate) /
                                       static_cast<unsigned long long>(dstFrameRate);
      else
        lengthDSDSamplesTimeChannels = 0;
    }
    else {
      // In DSD uncompressed files, the read number of samples is the total for each channel
      if(channels > 0)
        lengthDSDSamplesTimeChannels /= channels;
    }
    int bitrate = 0;
    if(lengthDSDSamplesTimeChannels > 0)
      bitrate = (audioDataSizeinBytes * 8 * sampleRate) / lengthDSDSamplesTimeChannels / 1000;

    d->properties = new DSDIFFProperties(sampleRate,
                                   channels,
                                   lengthDSDSamplesTimeChannels,
                                   bitrate,
                                   propertiesStyle);
  }

  if(!ID3v2Tag()) {
    // No ID3v2 Tag found, create an empty object.
    d->tag = new TagLib::ID3v2::Tag();
    // By default, ID3 chunk is at root level
    d->isID3InPropChunk = false;
    d->hasID3v2 = false;
  }
}

void DSDIFFFile::writeChunk(const TagLib::ByteVector &name, const TagLib::ByteVector &data,
                              unsigned long long offset, unsigned long replace,
                              unsigned int leadingPadding)
{
  TagLib::ByteVector combined;
  if(leadingPadding)
    combined.append(TagLib::ByteVector(leadingPadding, '\x00'));

  combined.append(name);
  combined.append(TagLib::ByteVector::fromLongLong(data.size()));
  combined.append(data);
  if((data.size() & 0x01) != 0)
    combined.append('\x00');

  insert(combined, offset, replace);
}
