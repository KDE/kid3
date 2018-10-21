/**
 * \file fingerprintcalculator.cpp
 * Chromaprint fingerprint calculator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Jan 2012
 *
 * Copyright (C) 2012-2018  Urs Fleisch
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

#define __STDC_CONSTANT_MACROS
#include "fingerprintcalculator.h"
#include "config.h"
#include "abstractfingerprintdecoder.h"

/**
 * Constructor.
 */
FingerprintCalculator::FingerprintCalculator(QObject* parent) : QObject(parent),
  m_chromaprintCtx(nullptr),
  m_decoder(AbstractFingerprintDecoder::createFingerprintDecoder(this))
{
  connect(m_decoder, &AbstractFingerprintDecoder::started,
          this, &FingerprintCalculator::startChromaprint);
  connect(m_decoder, &AbstractFingerprintDecoder::bufferReady,
          this, &FingerprintCalculator::feedChromaprint);
  connect(m_decoder, &AbstractFingerprintDecoder::error,
          this, &FingerprintCalculator::receiveError);
  connect(m_decoder, &AbstractFingerprintDecoder::finished,
          this, &FingerprintCalculator::finishChromaprint);
}

/**
 * Destructor.
 */
FingerprintCalculator::~FingerprintCalculator()
{
  if (m_chromaprintCtx) {
    ::chromaprint_free(m_chromaprintCtx);
  }
}

/**
 * Calculate audio fingerprint for audio file.
 * When the calculation is finished, finished() is emitted.
 *
 * @param fileName path to audio file
 */
void FingerprintCalculator::start(const QString& fileName) {
  if (!m_chromaprintCtx) {
    // Lazy initialization to save resources if not used
    m_chromaprintCtx = ::chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
  }
  m_decoder->start(fileName);
}

/**
 * Stop decoder.
 */
void FingerprintCalculator::stop() {
  m_decoder->stop();
}

/**
 * Called when decoding starts.
 * @param sampleRate sample rate of the audio stream (in Hz)
 * @param channelCount numbers of channels in the audio stream (1 or 2)
 */
void FingerprintCalculator::startChromaprint(int sampleRate, int channelCount)
{
  ::chromaprint_start(m_chromaprintCtx, sampleRate, channelCount);
}

/**
 * Called when decoded data is available.
 * @param data 16-bit signed integers in native byte-order
 */
void FingerprintCalculator::feedChromaprint(QByteArray data)
{
  if (!::chromaprint_feed(m_chromaprintCtx,
                          reinterpret_cast<qint16*>(data.data()),
                          data.size() / 2)) {
    m_decoder->stop();
    emit finished(QString(), 0, FingerprintCalculationFailed);
  }
}

/**
 * Called when an error occurs.
 * @param err error code, enum FingerprintCalculator::Error
 */
void FingerprintCalculator::receiveError(int err)
{
  emit finished(QString(), 0, err);
}

/**
 * Called when decoding finished successfully.
 * @param duration duration of stream in seconds
 */
void FingerprintCalculator::finishChromaprint(int duration)
{
  int err = Ok;
  QString fingerprint;
  char* fp;
  if (::chromaprint_finish(m_chromaprintCtx) &&
      ::chromaprint_get_fingerprint(m_chromaprintCtx, &fp)) {
    fingerprint = QString::fromLatin1(fp);
    ::chromaprint_dealloc(fp);
  } else {
    err = FingerprintCalculationFailed;
  }
  emit finished(fingerprint, duration, err);
}
