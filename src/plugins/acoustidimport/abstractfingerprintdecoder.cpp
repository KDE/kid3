/**
 * \file abstractfingerprintdecoder.h
 * Abstract base class for Chromaprint fingerprint decoder.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Feb 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "abstractfingerprintdecoder.h"

/**
 * Constructor.
 * @param parent parent object
 */
AbstractFingerprintDecoder::AbstractFingerprintDecoder(QObject* parent)
  : QObject(parent), m_stopped(false)
{
}

/**
 * Destructor.
 */
AbstractFingerprintDecoder::~AbstractFingerprintDecoder()
{
}

/**
 * Run decoder on audio file.
 */
void AbstractFingerprintDecoder::start(const QString&)
{
  m_stopped = false;
}

/**
 * Stop decoder.
 * Can be used to stop the decoder when an error is found after
 * getting bufferReady() data.
 */
void AbstractFingerprintDecoder::stop()
{
  m_stopped = true;
}

/**
 * Check if decoding has been stopped.
 * @return true if stopped.
 */
bool AbstractFingerprintDecoder::isStopped() const
{
  return m_stopped;
}
