/**
 * \file qtfingerprintdecoder.h
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

#ifndef QTFINGERPRINTDECODER_H
#define QTFINGERPRINTDECODER_H

#include <QtGlobal>
#include "abstractfingerprintdecoder.h"

class QAudioDecoder;
class QTimer;

/**
 * Chromaprint fingerprint decoder using QAudioDecoder.
 */
class QtFingerprintDecoder : public AbstractFingerprintDecoder {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit QtFingerprintDecoder(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~QtFingerprintDecoder();

  /**
   * Run decoder on audio file.
   * @param filePath path to audio file
   */
  virtual void start(const QString& filePath);

  /**
   * Stop decoder.
   * Can be used to stop the decoder when an error is found after
   * getting bufferReady() data.
   */
  virtual void stop();

private slots:
  /**
   * Receive a buffer with decoded audio data.
   */
  void receiveBuffer();

  /**
   * Receive an error from the decoder.
   */
  void receiveError();

  /**
   * Receive a timeout.
   */
  void receiveTimeout();

  /**
   * Finish decoding.
   */
  void finishDecoding();

private:
  QAudioDecoder* m_decoder;
  QTimer* m_timer;
};

#endif // QTFINGERPRINTDECODER_H
