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

#pragma once

#include <tfile.h>
#include <tag.h>
#include "dsdiffproperties.h"

//! An implementation of DSDIFF metadata

/*!
 * This is implementation of DSDIFF metadata.
 *
 * This supports an ID3v2 tag as well as reading stream from the ID3 RIFF
 * chunk as well as properties from the file.
 * Description of the DSDIFF format is available
 * at http://dsd-guide.com/sites/default/files/white-papers/DSDIFF_1.5_Spec.pdf
 * DSDIFF standard does not explicitly specify the ID3V2 chunk
 * It can be found at the root level, but also sometimes inside the PROP chunk
 * In addition, title and artist info are stored as part of the standard
 */

//! An implementation of TagLib::File with DSDIFF specific methods

/*!
 * This implements and provides an interface for DSDIFF files to the
 * TagLib::Tag and TagLib::AudioProperties interfaces by way of implementing
 * the abstract TagLib::File API as well as providing some additional
 * information specific to DSDIFF files.
 */

namespace TagLib { namespace ID3v2 { class Tag; class FrameFactory; } }

class DSDIFFFile : public TagLib::File
{
public:
  /*!
   * Constructs an DSDIFF file from \a file.  If \a readProperties is true
   * the file's audio properties will also be read.
   *
   * \note In the current implementation, \a propertiesStyle is ignored.
   */
  DSDIFFFile(TagLib::FileName file, bool readProperties = true,
       TagLib::AudioProperties::ReadStyle propertiesStyle
       = TagLib::AudioProperties::Average);

  /*!
   * Constructs an DSDIFF file from \a stream.  If \a readProperties is true
   * the file's audio properties will also be read.
   *
   * \note TagLib will *not* take ownership of the stream, the caller is
   * responsible for deleting it after the File object.
   *
   * \note In the current implementation, \a propertiesStyle is ignored.
   */
  DSDIFFFile(TagLib::FileName file, TagLib::ID3v2::FrameFactory *frameFactory,
       bool readProperties = true,
       TagLib::AudioProperties::ReadStyle propertiesStyle
    = TagLib::AudioProperties::Average);

  /*!
   * Constructs an DSDIFF file from \a stream.  If \a readProperties is true the
   * file's audio properties will also be read.
   *
   * \note TagLib will *not* take ownership of the stream, the caller is
   * responsible for deleting it after the File object.
   *
   * If this file contains and ID3v2 tag the frames will be created using
   * \a frameFactory.
   *
   * \note In the current implementation, \a propertiesStyle is ignored.
   */
  DSDIFFFile(TagLib::IOStream *stream, TagLib::ID3v2::FrameFactory *frameFactory,
    bool readProperties = true,
    TagLib::AudioProperties::ReadStyle propertiesStyle
    = TagLib::AudioProperties::Average);

  /*!
   * Destroys this instance of the File.
   */
  virtual ~DSDIFFFile() override;

  /*!
   * Returns a pointer to a ID3v2 tag
   */
  virtual TagLib::Tag *tag() const override;

  /*!
   * Returns the ID3V2 Tag for this file.
   *
   * \note This always returns a valid pointer regardless of whether or not
   * the file on disk has an ID3v2 tag.  Use hasID3v2Tag() to check if the
   * file on disk actually has an ID3v2 tag.
   *
   * \see hasID3v2Tag()
   */
  TagLib::ID3v2::Tag *ID3v2Tag() const;

  /*!
   * Implements the unified property interface -- export function.
   * This method forwards to ID3v2::Tag::properties().
   */
  TagLib::PropertyMap properties() const;

  // NEEDED??
  void removeUnsupportedProperties(const TagLib::StringList &properties);

  /*!
   * Implements the unified property interface -- import function.
   * This method forwards to ID3v2::Tag::setProperties().
   */
  TagLib::PropertyMap setProperties(const TagLib::PropertyMap &);

  /*!
   * Returns the DSDIFF::Properties for this file.  If no audio properties
   * were read then this will return a null pointer.
   */
  virtual TagLib::AudioProperties *audioProperties() const override;

  /*!
   * Save the file.  If at least one tag -- ID3v1 or ID3v2 -- exists this
   * will duplicate its content into the other tag.  This returns true
   * if saving was successful.
   *
   * If neither exists or if both tags are empty, this will strip the tags
   * from the file.
   *
   * This is the same as calling save(AllTags);
   *
   * If you would like more granular control over the content of the tags,
   * with the concession of generality, use paramaterized save call below.
   *
   * \see save(int tags)
   */
  virtual bool save() override;

  /*!
   * Save the file.  If at least one tag -- ID3v1 or ID3v2 -- exists this
   * will duplicate its content into the other tag.  This returns true
   * if saving was successful.
   *
   * If neither exists or if both tags are empty, this will strip the tags
   * from the file.
   *
   * This is the same as calling save(AllTags);
   *
   * If you would like more granular control over the content of the tags,
   * with the concession of generality, use paramaterized save call below.
   *
   * ID3v2Version can be either 4 or 3.
   */
  virtual bool save(int id3v2Version);

  /*!
   * Returns whether or not the file on disk actually has an ID3v2 tag.
   *
   * \see ID3v2Tag()
   */
  bool hasID3v2Tag() const;

  /*!
   * Returns whether or not the given \a stream can be opened as a DSDIFF
   * file.
   *
   * \note This method is designed to do a quick check.  The result may
   * not necessarily be correct.
   */
   static bool isSupported(TagLib::IOStream *stream);

private:
  DSDIFFFile(const DSDIFFFile &);
  DSDIFFFile &operator=(const DSDIFFFile &);

  void removeRootChunk(const TagLib::ByteVector &id);
  void removeRootChunk(unsigned int chunk);
  void removeChildChunk(unsigned int i);

  /*!
   * Sets the data for the the specified chunk at root level to \a data.
   *
   * \warning This will update the file immediately.
   */
  void setRootChunkData(unsigned int i, const TagLib::ByteVector &data);

  /*!
   * Sets the data for the root-level chunk \a name to \a data.
   * If a root-level chunk with the given name already exists
   * it will be overwritten, otherwise it will be
   * created after the existing chunks.
   *
   * \warning This will update the file immediately.
   */
  void setRootChunkData(const TagLib::ByteVector &name, const TagLib::ByteVector &data);

  /*!
   * Sets the data for the the specified child chunk to \a data.
   *
   * If data is null, then remove the chunk
   *
   * \warning This will update the file immediately.
   */
  void setChildChunkData(unsigned int i, const TagLib::ByteVector &data);

  /*!
   * Sets the data for the child chunk \a name to \a data.  If a chunk with
   * the given name already exists it will be overwritten, otherwise it will
   * be created after the existing chunks inside child chunk.
   *
   * If data is null, then remove the chunks with \a name name
   *
   * \warning This will update the file immediately.
   */
  void setChildChunkData(const TagLib::ByteVector &name, const TagLib::ByteVector &data);

  void updateRootChunksStructure(unsigned int startingChunk);

  void read(bool readProperties, TagLib::AudioProperties::ReadStyle propertiesStyle);
  void writeChunk(const TagLib::ByteVector &name, const TagLib::ByteVector &data,
                  unsigned long long offset, unsigned long replace = 0,
                  unsigned int leadingPadding = 0);

  class FilePrivate;
  FilePrivate *d;
};
