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

#include "audioproperties.h"

//! An implementation of audio property reading for DSDIFF

/*!
 * This reads the data from an DSDIFF stream found in the TagLib::AudioProperties
 * API.
 */

class DSDIFFProperties : public TagLib::AudioProperties
{
public:
  /*!
   * Create an instance of DSDIFFProperties with the data read from the
   * ByteVector \a data.
   */
  DSDIFFProperties(const unsigned int sampleRate, const unsigned short channels,
             const unsigned long long samplesCount, const int bitrate,
             TagLib::AudioProperties::ReadStyle style);

  /*!
   * Destroys this DSDIFFProperties instance.
   */
  virtual ~DSDIFFProperties();

  // Reimplementations.

  virtual int length() const;
  virtual int lengthInSeconds() const;
  virtual int lengthInMilliseconds() const;
  virtual int bitrate() const;
  virtual int sampleRate() const;
  virtual int channels() const;

  int bitsPerSample() const;
  long long sampleCount() const;

private:
  DSDIFFProperties(const DSDIFFProperties &);
  DSDIFFProperties &operator=(const DSDIFFProperties &);

  class PropertiesPrivate;
  PropertiesPrivate *d;
};
