/**
 * \file qtfingerprintdecoder.cpp
 * Chromaprint fingerprint decoder using QAudioDecoder.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Feb 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "qtfingerprintdecoder.h"
#include <QAudioDecoder>
#include <QTimer>
#include "fingerprintcalculator.h"

/**
 * Constructor.
 * @param parent parent object
 */
QtFingerprintDecoder::QtFingerprintDecoder(QObject* parent) :
  AbstractFingerprintDecoder(parent),
  m_decoder(new QAudioDecoder(this)), m_timer(new QTimer(this))
{
  QAudioFormat desiredFormat;
  desiredFormat.setChannelCount(2);
  desiredFormat.setCodec(QLatin1String("audio/x-raw-int"));
  desiredFormat.setSampleType(QAudioFormat::SignedInt);
  desiredFormat.setSampleRate(44100);
  desiredFormat.setSampleSize(16);

  m_decoder->setAudioFormat(desiredFormat);
  connect(m_decoder, SIGNAL(bufferReady()), this, SLOT(receiveBuffer()));
  connect(m_decoder, SIGNAL(error(QAudioDecoder::Error)),
          this, SLOT(receiveError()));
  connect(m_decoder, SIGNAL(finished()), this, SLOT(finishDecoding()));

  m_timer->setSingleShot(true);
  m_timer->setInterval(5000);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(receiveTimeout()));
}

/**
 * Destructor.
 */
QtFingerprintDecoder::~QtFingerprintDecoder()
{
}

/**
 * Run decoder on audio file.
 * @param filePath path to audio file
 */
void QtFingerprintDecoder::start(const QString& filePath)
{
  AbstractFingerprintDecoder::start(filePath);
  m_decoder->setSourceFilename(filePath);

  QAudioFormat format = m_decoder->audioFormat();
  emit started(format.sampleRate(), format.channelCount());
  m_timer->start();
  m_decoder->start();
}

/**
 * Stop decoder.
 * Can be used to stop the decoder when an error is found after
 * getting bufferReady() data.
 */
void QtFingerprintDecoder::stop()
{
  AbstractFingerprintDecoder::stop();
  m_timer->stop();
  m_decoder->stop();
}

/**
 * Receive a buffer with decoded audio data.
 */
void QtFingerprintDecoder::receiveBuffer()
{
  m_timer->stop();
  QAudioBuffer buffer = m_decoder->read();
  if (!buffer.isValid()) {
    return;
  }
  if (buffer.startTime() > 120000000LL) {
    finishDecoding();
    return;
  }
  emit bufferReady(QByteArray(buffer.constData<char>(), buffer.byteCount()));
  m_timer->start();
}

/**
 * Receive an error from the decoder.
 */
void QtFingerprintDecoder::receiveError()
{
  m_timer->stop();
  m_decoder->stop();
  emit error(FingerprintCalculator::DecoderError);
}

/**
 * Receive a timeout.
 */
void QtFingerprintDecoder::receiveTimeout()
{
  m_decoder->stop();
  emit error(FingerprintCalculator::Timeout);
}

/**
 * Finish decoding.
 */
void QtFingerprintDecoder::finishDecoding()
{
  m_timer->stop();
  int duration = m_decoder->duration() / 1000LL;
  m_decoder->stop();
  emit finished(duration);
}


/**
 * Create concrete fingerprint decoder.
 * @param parent parent object
 * @return fingerprint decoder instance.
 * @remarks This static method will be implemented by the concrete
 * fingerprint decoder which is used.
 */
AbstractFingerprintDecoder*
AbstractFingerprintDecoder::createFingerprintDecoder(QObject* parent) {
  return new QtFingerprintDecoder(parent);
}
