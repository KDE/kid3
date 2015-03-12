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

// Kid3: Remove taglib/ from includes
#include <id3v2tag.h>
#include <id3v2header.h>
#include <tpropertymap.h>
// Kid3: Use debug() instead of std::cerr
#include <tdebug.h>

#include <bitset>

#include "dsffile.h"
#include "dsfheader.h"

//using namespace TagLib;

class DSFFile::FilePrivate
{
public:
  FilePrivate(TagLib::ID3v2::FrameFactory *frameFactory 
	      = TagLib::ID3v2::FrameFactory::instance()) :
    ID3v2FrameFactory(frameFactory),
    ID3v2Location(0),
    ID3v2OriginalSize(0),
    fileSize(0),
    tag(0),
    hasID3v2(false),
    properties(0)
  {}

  ~FilePrivate()
  {
    if (properties) delete properties;
    if (tag) delete tag;
  }

  const TagLib::ID3v2::FrameFactory *ID3v2FrameFactory;

  uint64_t ID3v2Location; // For DSD it's always > 0 if tag is present
  uint64_t ID3v2OriginalSize;
  uint64_t fileSize;
  TagLib::ID3v2::Tag *tag;

  // These indicate whether the file *on disk* has these tags, not if
  // this data structure does.  This is used in computing offsets.

  bool hasID3v2;

  DSFProperties *properties;

  static inline TagLib::ByteVector& uint64ToVector(uint64_t num, 
						   TagLib::ByteVector &v) 
  {
    char raw[8];
    
    for (int i = 0; i < 8; i++) {
      raw[i] = ((0xff << (i * 8)) & num) >> (i * 8);
    }
    v.setData(raw, 8);
    return v;
  }

  // 
  // ID3v2::Tag::render() fills up space previous occupied by 
  // deleted frames with 0's, presumably to avoid rewriting 
  // all audio data (in an mp3 file, the ID3v2 tag comes before the audio 
  // data). 
  // However in a DSD file the ID3v2 chunk is located at the end.
  // 
  // This function attempts to shrink the ID3v2 Tag by replacing
  // the old ID3v2::Tag object with a new one to free up that space.
  //
  void shrinkTag();
};

void DSFFile::FilePrivate::shrinkTag() {
  TagLib::ID3v2::FrameList olist = tag->frameList();
  TagLib::ID3v2::FrameList nlist;
  TagLib::ID3v2::FrameList::ConstIterator it;

  for (it = olist.begin(); it != olist.end(); it++) {
    nlist.append(*it);
  }

  TagLib::ID3v2::Tag *ntag = new TagLib::ID3v2::Tag();
  for (it = nlist.begin(); it != nlist.end(); it++) {
    tag->removeFrame(*it, false);  // Don't delete, just transfer the ownership
    ntag->addFrame(*it);
  }
  
  delete tag;
  tag = ntag;
}


////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

DSFFile::DSFFile(TagLib::FileName file, bool readProperties,
		 TagLib::AudioProperties::ReadStyle propertiesStyle) 
  : TagLib::File(file)
{
  d = new FilePrivate;

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSFFile::DSFFile(TagLib::FileName file, 
		 TagLib::ID3v2::FrameFactory *frameFactory,
		 bool readProperties, 
		 TagLib::AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(file)
{
  d = new FilePrivate(frameFactory);

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSFFile::DSFFile(TagLib::IOStream *stream, 
		 TagLib::ID3v2::FrameFactory *frameFactory,
		 bool readProperties, 
		 TagLib::AudioProperties::ReadStyle propertiesStyle) :
  TagLib::File(stream)
{
  d = new FilePrivate(frameFactory);

  if(isOpen())
    read(readProperties, propertiesStyle);
}

DSFFile::~DSFFile()
{
  delete d;
}

TagLib::Tag *DSFFile::tag() const
{
  return d->tag;
}

TagLib::PropertyMap DSFFile::properties() const
{
  if(d->hasID3v2)
    return d->tag->properties();
  return TagLib::PropertyMap();
}

void DSFFile::removeUnsupportedProperties(const TagLib::StringList &properties)
{
  if(d->hasID3v2)
    d->tag->removeUnsupportedProperties(properties);
}

TagLib::PropertyMap DSFFile::setProperties(const TagLib::PropertyMap &properties)
{
  return d->tag->setProperties(properties);
}

TagLib::AudioProperties *DSFFile::audioProperties() const
{
  return d->properties;
}

bool DSFFile::save() 
{
  return save(4);
}

bool DSFFile::save(int id3v2Version, bool shrink)
{
  if(readOnly()) {
    debug("DSFFile::save() -- File is read only.");
    return false;
  }

  bool success = true;

  if(ID3v2Tag() && !ID3v2Tag()->isEmpty()) {
    if (shrink) // remove padding 0's
      d->shrinkTag();

    TagLib::ByteVector id3v2_v = ID3v2Tag()->render(id3v2Version);
    uint64_t fileSize = d->fileSize + id3v2_v.size() - d->ID3v2OriginalSize;
    TagLib::ByteVector fileSize_v;

    // Write new file size to DSD header 
    DSFFile::FilePrivate::uint64ToVector(fileSize, fileSize_v);
    insert(fileSize_v, 12, DSFHeader::LONG_INT_SIZE);

    // The file didn't have an ID3v2 metadata block
    // Make the offset point to the end of file
    if (d->ID3v2Location == 0) {
      d->ID3v2Location = d->fileSize;

      TagLib::ByteVector offset_v;
      DSFFile::FilePrivate::uint64ToVector(d->fileSize, offset_v);
      insert(offset_v, 20, DSFHeader::LONG_INT_SIZE);
    }

    // Write ID3v2 to the end of the file
    insert(id3v2_v, d->ID3v2Location, d->ID3v2OriginalSize);
    
    // Reset header info
    d->fileSize = fileSize;
    d->ID3v2OriginalSize = id3v2_v.size();
    d->hasID3v2 = true;
  } else {
    //
    // All frames have been deleted. Remove ID3v2 block
    //

    TagLib::ByteVector nulls_v(DSFHeader::LONG_INT_SIZE, 0);
    TagLib::ByteVector fileSize_v;

    DSFFile::FilePrivate::uint64ToVector(d->ID3v2Location, fileSize_v);
    insert(fileSize_v, 12, DSFHeader::LONG_INT_SIZE); // new file size
    insert(nulls_v, 20, DSFHeader::LONG_INT_SIZE); // set metadata offset to 0
    removeBlock(d->ID3v2Location, d->ID3v2OriginalSize);

    // Reset header info
    d->ID3v2OriginalSize = 0;
    d->fileSize = d->ID3v2Location;
    d->ID3v2Location = 0;
    d->hasID3v2 = false;
  }

  // Reinitialize properties because DSD header may have been changed
  delete d->properties;
  d->properties = new DSFProperties(this, 
				    TagLib::AudioProperties::Average);

  return success;
}

TagLib::ID3v2::Tag *DSFFile::ID3v2Tag() const
{
  return d->tag;
}

void DSFFile::setID3v2FrameFactory(const TagLib::ID3v2::FrameFactory *factory)
{
  d->ID3v2FrameFactory = factory;
}

bool DSFFile::hasID3v2Tag() const
{
  return d->hasID3v2;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

bool DSFFile::secondSynchByte(char byte)
{
  std::bitset<8> b(byte);

  // check to see if the byte matches 111xxxxx
  return b.test(7) && b.test(6) && b.test(5);
}

void DSFFile::read(bool readProperties, 
		   TagLib::AudioProperties::ReadStyle propertiesStyle)
{
  if(readProperties)
    d->properties = new DSFProperties(this, propertiesStyle);

  d->ID3v2Location = d->properties->ID3v2Offset();
  d->fileSize = d->properties->fileSize();

  if(d->ID3v2Location > 0) {
    d->tag = new TagLib::ID3v2::Tag(this, d->ID3v2Location, d->ID3v2FrameFactory);
    d->ID3v2OriginalSize = d->tag->header()->completeTagSize();

    if(d->tag->header()->tagSize() > 0)
      d->hasID3v2 = true;
  } else {
    // No ID3v2 Tag found, create an empty object.
    d->tag = new TagLib::ID3v2::Tag();
  }
}
