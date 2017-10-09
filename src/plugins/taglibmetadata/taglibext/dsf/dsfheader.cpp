/***************************************************************************
    copyright            : (C) 2014 by Peking Duck Labs
    email                : pekingducklabs@gmail.com
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
 ***************************************************************************/

#include <stdint.h>

#include <bitset>

// Kid3: Remove taglib/ from includes
#include <tbytevector.h>
#include <tstring.h>
#include <trefcounter.h>
// Kid3: Use debug() instead of std::cerr
#include <tdebug.h>

#include "dsfheader.h"

class DSFHeader::HeaderPrivate : public TagLib::RefCounter
{
public:
  HeaderPrivate() :
    isValid(false),
    version(Version1),
    sampleCount(0),
    channelType(Stereo),
    channelNum(2),
    sampleRate(0),
    bitsPerSample(0),
    ID3v2Offset(0),
    fileSize(0)
  {}

  bool isValid;
  Version version; // format version
  uint64_t sampleCount;
  ChannelType channelType;
  unsigned short channelNum;
  unsigned int sampleRate; 
  unsigned short bitsPerSample;
  uint64_t ID3v2Offset;
  uint64_t fileSize;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSFHeader::DSFHeader(const TagLib::ByteVector &data)
{
  d = new HeaderPrivate;
  parse(data);
}

DSFHeader::DSFHeader(const DSFHeader &h) : d(h.d)
{
  d->ref();
}

DSFHeader::~DSFHeader()
{
  if (d->deref())
    delete d;
}

bool DSFHeader::isValid() const
{
  return d->isValid;
}

DSFHeader::Version DSFHeader::version() const
{
  return d->version;
}

unsigned int DSFHeader::sampleRate() const
{
  return d->sampleRate;
}

DSFHeader::ChannelType DSFHeader::channelType() const
{
  return d->channelType;
}

unsigned short DSFHeader::channelNum() const 
{
  return d->channelNum;
}

uint64_t DSFHeader::sampleCount() const
{
  return d->sampleCount;
}

uint64_t DSFHeader::ID3v2Offset() const
{
  return d->ID3v2Offset;
}

uint64_t DSFHeader::fileSize() const
{
  return d->fileSize;
}

unsigned short DSFHeader::bitsPerSample() const
{
  return d->bitsPerSample;
}

DSFHeader &DSFHeader::operator=(const DSFHeader &h)
{
  if(&h == this)
    return *this;

  if(d->deref())
    delete d;

  d = h.d;
  d->ref();
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void DSFHeader::parse(const TagLib::ByteVector &data)
{
  if (data.size() < DSD_HEADER_SIZE + FMT_HEADER_SIZE) {
    debug("DSFHeader::parse(): header size incorrect");
    return;
  }

  const char *hdr = data.data();
  size_t offset = 0;

  //
  // ******** DSD chunk header ********
  // DSD header chunk should start with "DSD ".
  //
  if (hdr[0] != 'D' || hdr[1] != 'S' || hdr[2] != 'D' || hdr[3] != ' ') 
  {
    debug("DSD::Header::parse(): DSD header's first 4 bytes != 'DSD '");
    return;
  }
  offset += 4;
  
  // The next 8 bytes contain the size of DSD header
  // (numerical data is stored in little endian)
  if (data.toLongLong(offset, false) != DSD_HEADER_SIZE)
  {
    debug("DSD::Header::parse(): DSD header size is incorrect");
    return;
  }
  offset += LONG_INT_SIZE;

  // The next 8 bytes contains the file size
  d->fileSize = bytesToUInt64(&hdr[0], offset);
  offset += LONG_INT_SIZE;

  // The next 8 bytes contains the offset to id3v2 tag (0 if not exists)
  d->ID3v2Offset = bytesToUInt64(&hdr[0], offset);
  offset += LONG_INT_SIZE;

  // 
  // ********* FMT chunk ********
  //
  // FMT header chunk should start with "fmt ".
  //
  if (hdr[offset] != 'f' || hdr[offset + 1] != 'm' || 
      hdr[offset + 2] != 't' || hdr[offset + 3] != ' ') 
  {
    debug("DSD::Header::parse(): FMT header's first 4 bytes != 'fmt '");
    return;
  }
  offset += 4;

  // The next 8 bytes contain the size of FMT header, which should be 52
  if (data.toLongLong(offset, false) != FMT_HEADER_SIZE)
  {
    debug("DSD::Header::parse(): FMT header size is incorrect");
    return;
  }
  offset += LONG_INT_SIZE;

  // Format version
  // There's only version 1 for now...
  unsigned int ver = data.toUInt(offset, false);
  if (ver != 1) {
    debug("DSD::Header::parse(): format version != 1");
    return;
  }
  d->version = static_cast<Version>(ver);
  offset += INT_SIZE;

  // Format ID
  if (data.toUInt(offset, false) != 0) {
    debug("DSD::Header::parse(): format ID != 0");
    return;
  }
  offset += INT_SIZE;

  // Channel Type
  unsigned int ct = data.toUInt(offset, false);
  if (ct < 1 || ct > 7) {
    debug("DSD::Header::parse(): channel type out of range");
    return;
  }
  d->channelType = static_cast<ChannelType>(ct);
  offset += INT_SIZE;

  // Channel Num
  d->channelNum = data.toUInt(offset, false);
  // Kid3: Removed check "d->channelNum < MinType || ", is always false.
  if (d->channelNum > MaxType) {
    debug("DSD::Header::parse(): channel num out of range");
    return;
  }
  offset += INT_SIZE;

  // Sampling frequency
  d->sampleRate = data.toUInt(offset, false);
  if (d->sampleRate != 2822400 && d->sampleRate != 5644800) {
    debug("DSD::Header::parse(): invalid sampling frequency");
  }
  offset += INT_SIZE;

  // Bits per sample
  d->bitsPerSample = data.toUInt(offset, false);
  if (d->bitsPerSample != 1 && d->bitsPerSample != 8) {
    debug("DSD::Header::parse(): bits per sample invalid");
    return;
  }
  offset += INT_SIZE;

  // Sample count
  d->sampleCount = bytesToUInt64(&hdr[0], offset);
  offset += LONG_INT_SIZE;

  // Block size per channel
  if (data.toUInt(offset, false) != 4096) {
    debug("DSD::Header::parse(): block size != 4096");
    return;
  }
    //offset += 4;

  // Reserved
  // offset += 4;

  d->isValid = true;
}
