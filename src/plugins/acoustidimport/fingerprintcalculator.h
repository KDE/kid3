/**
 * \file fingerprintcalculator.h
 * Chromaprint fingerprint calculator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Jan 2012
 *
 * Copyright (C) 2012-2024  Urs Fleisch
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

#pragma once

#include <QObject>
#include <QString>
#include <chromaprint.h>

class AbstractFingerprintDecoder;

/**
 * Calculate Chromaprint audio fingerprints for audio files.
 */
class FingerprintCalculator : public QObject {
  Q_OBJECT
public:
  /** Types of error occurring in fingerprint calculation. */
  enum Error {
    Ok,               /**< Fingerprint calculation OK */
    Pending,          /**< Not started */
    NoStreamFound,    /**< Format not recognized or no audio stream found */
    NoCodecFound,     /**< No codec found */
    NoConverterFound, /**< Sample rate conversion failed or unavailable */
    FingerprintCalculationFailed, /**< Chromaprint error */
    Timeout,          /**< Operation timeout */
    DecoderError      /**< Error while decoding */
  };

  /**
   * Constructor.
   */
  explicit FingerprintCalculator(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~FingerprintCalculator() override;

  /**
   * Calculate audio fingerprint for audio file.
   * When the calculation is finished, finished() is emitted.
   *
   * @param fileName path to audio file
   */
  void start(const QString& fileName);

  /**
   * Stop decoder.
   */
  void stop();

signals:
  /**
   * Emitted when the fingerprint calculation is finished.
   *
   * @param fingerprint Chromaprint fingerprint
   * @param duration duration in seconds
   * @param error error code, enum FingerprintCalculator::Error
   */
  void finished(const QString& fingerprint, int duration, int error);

private slots:
  /**
   * Called when decoding starts.
   * @param sampleRate sample rate of the audio stream (in Hz)
   * @param channelCount numbers of channels in the audio stream (1 or 2)
   */
  void startChromaprint(int sampleRate, int channelCount);

  /**
   * Called when decoded data is available.
   * @param data 16-bit signed integers in native byte-order
   */
  void feedChromaprint(QByteArray data);

  /**
   * Called when an error occurs.
   * @param err error code, enum FingerprintCalculator::Error
   */
  void receiveError(int err);

  /**
   * Called when decoding finished successfully.
   * @param duration duration of stream in seconds
   */
  void finishChromaprint(int duration);

private:
  ChromaprintContext* m_chromaprintCtx;
  AbstractFingerprintDecoder* m_decoder;
};
