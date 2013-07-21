/**
 * \file abstractfingerprintdecoder.h
 * Abstract base class for Chromaprint fingerprint decoder.
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

#ifndef ABSTRACTFINGERPRINTDECODER_H
#define ABSTRACTFINGERPRINTDECODER_H

#include <QObject>

/**
 * Abstract base class for Chromaprint fingerprint decoder.
 */
class AbstractFingerprintDecoder : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit AbstractFingerprintDecoder(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~AbstractFingerprintDecoder() = 0;

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

  /**
   * Check if decoding has been stopped.
   * @return true if stopped.
   */
  virtual bool isStopped() const;

  /**
   * Create concrete fingerprint decoder.
   * @param parent parent object
   * @return fingerprint decoder instance.
   * @remarks This static method will be implemented by the concrete
   * fingerprint decoder which is used.
   */
  static AbstractFingerprintDecoder* createFingerprintDecoder(QObject* parent);

signals:
  /**
   * Emitted when decoding starts.
   * @param sampleRate sample rate of the audio stream (in Hz)
   * @param channelCount numbers of channels in the audio stream (1 or 2)
   */
  void started(int sampleRate, int channelCount);

  /**
   * Emitted when decoded data is available.
   * @param data 16-bit signed integers in native byte-order
   */
  void bufferReady(QByteArray data);

  /**
   * Emitted when an error occurs.
   * @param code error code, enum FingerprintCalculator::Error
   */
  void error(int code);
  
  /**
   * Emitted when decoding finished successfully.
   * @param duration duration of stream in seconds
   */
  void finished(int duration);

private:
  bool m_stopped;
};

#endif // ABSTRACTFINGERPRINTDECODER_H
