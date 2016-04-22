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

#ifndef TAGLIB_DSFHEADER_H
#define TAGLIB_DSFHEADER_H

//#include <taglib/taglib_export.h>

//! An implementation of DSF header

/*!
 * This is an implementation of DSF header. Check out
 * <a href="http://dsd-guide.com/sites/default/files/white-papers/DSFFileFormatSpec_E.pdf">this</a>
 * document as a reference.
 */

// Kid3: For uint64_t
#include <stdint.h>

// Kid3: Remove taglib/ from includes
#include <tbytevector.h>

class DSFHeader
{
 public:
  static const int DSD_HEADER_SIZE = 28;
  static const int FMT_HEADER_SIZE = 52;
  static const int LONG_INT_SIZE = 8;    // width of a long integer
  static const int INT_SIZE = 4;         // width of an integer

  /*!
   * Parses an DSF header based on \a data.
   */
  explicit DSFHeader(const TagLib::ByteVector &data);

  /*!
   * Does a shallow copy of \a h.
   */
  DSFHeader(const DSFHeader &h);

  /*!
   * Destroys this Header instance.
   */
  virtual ~DSFHeader();

  /*!
   * Returns true if header has legal values.
   */
  bool isValid() const;

  /*!
   * The DSD file format version
   */
  enum Version {
    //! DSD Version 1
    Version1 = 1
  };

  /*!
   * Channel Type:
   */
  enum ChannelType {
    MinType = 0,

    //! 1: mono
    Mono = 1,
    //! 2: stereo (front left, front right)
    Stereo = 2,
    //! 3: 3 channels (front left, front right, center)
    ThreeChannels = 3,
    //! 4: quad (front left/right, back left/right)
    Quad = 4,
    //! 5: 4 channels (front left, front right, low frequency, center)
    FourChannels = 5,
    //! 6: 5 channels (front left/right, back left/right, center)
    FiveChannels = 6,
    //! 7: 5.1 channels (front left/right, back left/right, center, low freq.)
    FiveOneChannels = 7,

    MaxType = 8
  };

  /*!
   * Returns the DSD Version of the header.
   */
  Version version() const;

  /*!
   * Returns the Channel Type of the header
   */
  ChannelType channelType() const;

  /*!
   * Returns the Channel Num of the header
   */
  unsigned short channelNum() const;

  /*!
   * Returns the sample rate in Hz.
   */
  unsigned int sampleRate() const;

  /*!
   * Returns the sample count
   */
  uint64_t sampleCount() const;

  /*!
   * Returns the bits per sample
   */
  unsigned short bitsPerSample() const;

  /*!
   * Returns the offset to the metadata block
   */
  uint64_t ID3v2Offset() const;

  /*!
   * Returns the file size
   */
  uint64_t fileSize() const;

  /*!
   * Makes a shallow copy of the header.
   */
  DSFHeader &operator=(const DSFHeader &h);

  // Assume LSB comes first
  inline uint64_t bytesToUInt64(const char *v, uint64_t offset = 0) {
    uint64_t res = 0;
    for (int i = 0; i < LONG_INT_SIZE; i++) {
      res |= static_cast<uint64_t>(static_cast<unsigned char>(v[offset + i])) 
	<< (i * 8);
    }
    return res;
  }
 private:
  void parse(const TagLib::ByteVector &data);

  class HeaderPrivate;
  HeaderPrivate *d;
};


#endif
