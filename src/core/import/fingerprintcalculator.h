/**
 * \file fingerprintcalculator.h
 * Chromaprint fingerprint calculator.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Jan 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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

#ifndef FINGERPRINTCALCULATOR_H
#define FINGERPRINTCALCULATOR_H

#include "config.h"

#ifdef HAVE_CHROMAPRINT

#include <QString>

/**
 * Calculate Chromaprint audio fingerprints for audio files.
 */
class FingerprintCalculator {
public:
  /**
   * Result of fingerprint calculation.
   */
  class Result {
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

    /** Constructor. */
    Result() : m_duration(0), m_error(Pending) {}

    /**
     * Get Chromaprint fingerprint.
     * @return fingerprint.
     */
    QString getFingerprint() { return m_fingerprint; }

    /**
     * Get duration in seconds.
     * @return duration.
     */
    int getDuration() const { return m_duration; }

    /**
     * Get error code.
     * @return Ok if OK, else error code.
     */
    Error getError() const { return m_error; }

  private:
    friend class FingerprintCalculator;

    QString m_fingerprint;
    int m_duration;
    Error m_error;
  };

  /**
   * Constructor.
   */
  FingerprintCalculator();

  /**
   * Destructor.
   */
  ~FingerprintCalculator();

  /**
   * Calculate audio fingerprint for audio file.
   *
   * @param fileName path to audio file
   *
   * @return result of fingerprint calculation.
   */
  Result calculateFingerprint(const QString& fileName);

private:
  class Decoder;

  Result::Error decodeAudioFile(const QString &filePath, int &duration);

  void** m_chromaprintCtx;
  Decoder* m_decoder;
};

#endif // HAVE_CHROMAPRINT

#endif // FINGERPRINTCALCULATOR_H
