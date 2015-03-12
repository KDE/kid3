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

#ifndef TAGLIB_DSFPROPERTIES_H
#define TAGLIB_DSFPROPERTIES_H

// Kid3: Remove taglib/ from includes
#include <audioproperties.h>

#include "dsfheader.h"

class DSFFile;

//! An implementation of audio property reading for DSF

/*!
 * This reads the data from a DSF stream found in the
 * AudioProperties API.
 */

class DSFProperties : public TagLib::AudioProperties
{
 public:
  /*!
   * Create an instance of DSF::Properties with the data read from the
   * DSF::File \a file.
   */
  DSFProperties(DSFFile *file, 
		TagLib::AudioProperties::ReadStyle style = Average);

  /*!
   * Destroys this DSF Properties instance.
   */
  virtual ~DSFProperties();

  // Reimplementations.

  virtual int length() const;
  virtual int bitrate() const;
  virtual int sampleRate() const;
  virtual int channels() const;

  /*!
   * Returns the DSF Version of the file.
   */
  DSFHeader::Version version() const;

  /*!
   * Returns the channel type
   */
  DSFHeader::ChannelType channelType() const;

  /*!
   * Returns the ID3v2 offset in the file
   */
  uint64_t ID3v2Offset() const;

  /*!
   * Returns the file size
   */
  uint64_t fileSize() const;

  /*!
   * Returns the sample count
   */
  uint64_t sampleCount() const;

  /*!
   * Returns the bits per sample
   */
  int bitsPerSample() const;

 private:
  DSFProperties(const DSFProperties &);
  DSFProperties &operator=(const DSFProperties &);

  void read();

  class PropertiesPrivate;
  PropertiesPrivate *d;
};

#endif
