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

#ifndef TAGLIB_DSFFILE_H
#define TAGLIB_DSFFILE_H

// Kid3: Remove taglib/ from includes
#include <tfile.h>
#include <tag.h>

#include "dsfproperties.h"


//! An implementation of TagLib::File with DSF (DSD) specific methods


//! An DSF file class with some useful methods specific to DSF

/*!
 * This implements the generic TagLib::File API and additionally provides
 * access to properties that are distinct to DSF files, notably access
 * to the different ID3 tags.
 */

namespace TagLib { namespace ID3v2 { class Tag; class FrameFactory; } }

class DSFFile : public TagLib::File
{
 public:
  /*!
   * Constructs an DSF file from \a file.  If \a readProperties is true the
   * file's audio properties will also be read.
   *
   * \note In the current implementation, \a propertiesStyle is ignored.
   *
   * \deprecated This constructor will be dropped in favor of the one below
   * in a future version.
   */
  DSFFile(TagLib::FileName file, bool readProperties = true,
       TagLib::AudioProperties::ReadStyle propertiesStyle 
       = TagLib::AudioProperties::Average);

  /*!
   * Average an DSF file from \a file.  If \a readProperties is true the
   * file's audio properties will also be read.
   *
   * If this file contains and ID3v2 tag the frames will be created using
   * \a frameFactory.
   *
   * \note In the current implementation, \a propertiesStyle is ignored.
   */
  // BIC: merge with the above constructor
  DSFFile(TagLib::FileName file, TagLib::ID3v2::FrameFactory *frameFactory,
       bool readProperties = true,
       TagLib::AudioProperties::ReadStyle propertiesStyle 
	  = TagLib::AudioProperties::Average);

  /*!
   * Constructs an DSF file from \a stream.  If \a readProperties is true the
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
  DSFFile(TagLib::IOStream *stream, TagLib::ID3v2::FrameFactory *frameFactory,
	  bool readProperties = true,
	  TagLib::AudioProperties::ReadStyle propertiesStyle = 
	  TagLib::AudioProperties::Average);

  /*!
   * Destroys this instance of the File.
   */
  virtual ~DSFFile();

  /*!
   * Returns a pointer to a ID3v2 tag
   */
  virtual TagLib::Tag *tag() const;

  /*!
   * Implements the reading part of the unified property interface.
   */
  TagLib::PropertyMap properties() const;

  // NEEDED??
  void removeUnsupportedProperties(const TagLib::StringList &properties);

  /*!
   * Implements the writing part of the unified tag dictionary interface.
   * In order to avoid problems with deprecated tag formats, this method
   * always creates an ID3v2 tag if necessary.
   * If an ID3v1 tag  exists, it will be updated as well, within the
   * limitations of that format.
   * The returned PropertyMap refers to the ID3v2 tag only.
   */
  TagLib::PropertyMap setProperties(const TagLib::PropertyMap &);

  /*!
   * Returns the DSF::Properties for this file.  If no audio properties
   * were read then this will return a null pointer.
   */
  virtual TagLib::AudioProperties *audioProperties() const;

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
  virtual bool save();

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
  virtual bool save(int id3v2Version, bool shrink = true);

  /*!
   * Returns a pointer to the ID3v2 tag of the file.
   *
   * If \a create is false (the default) this may return a null pointer
   * if there is no valid ID3v2 tag.  If \a create is true it will create
   * an ID3v2 tag if one does not exist and returns a valid pointer.
   *
   * \note This may return a valid pointer regardless of whether or not the 
   * file on disk has an ID3v2 tag.  Use hasID3v2Tag() to check if the file 
   * on disk actually has an ID3v2 tag.
   *
   * \note The Tag <b>is still</b> owned by the DSF::File and should not be
   * deleted by the user.  It will be deleted when the file (object) is
   * destroyed.
   *
   * \see hasID3v2Tag()
   */
  TagLib::ID3v2::Tag *ID3v2Tag() const;


  /*!
   * This will strip the tags that match the OR-ed together TagTypes from the
   * file.  By default it strips all tags.  It returns true if the tags are
   * successfully stripped.
   *
   * This is equivalent to strip(tags, true)
   *
   * If \a freeMemory is true the ID3 and APE tags will be deleted and
   * pointers to them will be invalidated.
   */
  //bool strip(bool freeMemory);

  /*!
   * Set the ID3v2::FrameFactory to something other than the default.
   *
   * \see ID3v2FrameFactory
   */
  void setID3v2FrameFactory(const TagLib::ID3v2::FrameFactory *factory);

  /*
   * Returns the position in the file of the first frame.
   */
  //long firstFrameOffset();

  /*
   * Returns the position in the file of the next frame,
   * using the current position as start
   */
  //long nextFrameOffset(long position);

  /*
   * Returns the position in the file of the previous frame,
   * using the current position as start
   */
  //long previousFrameOffset(long position);

  /*
   * Returns the position in the file of the last DSF frame.
   */
  //long lastFrameOffset();


  /*!
   * Returns whether or not the file on disk actually has an ID3v2 tag.
   *
   * \see ID3v2Tag()
   */
  bool hasID3v2Tag() const;

 private:
  DSFFile(const DSFFile &);
  DSFFile &operator=(const DSFFile &);

  // Read the actual audio file for tags
  void read(bool readProperties, 
	    TagLib::AudioProperties::ReadStyle propertiesStyle);

  /*!
   * ID3V2 frames can be recognized by the bit pattern 11111111 111, so the
   * first byte is easy to check for, however checking to see if the second
   * byte starts with \e 111 is a bit more tricky, hence this member function.
   */
  static bool secondSynchByte(char byte);

  class FilePrivate;
  FilePrivate *d;
};

#endif
