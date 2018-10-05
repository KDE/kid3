/**
 * \file ffmpegfingerprintdecoder.h
 * Chromaprint fingerprint decoder using FFmpeg.
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

#pragma once

#include "abstractfingerprintdecoder.h"

/**
 * Chromaprint fingerprint decoder using FFmpeg.
 */
class FFmpegFingerprintDecoder : public AbstractFingerprintDecoder {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit FFmpegFingerprintDecoder(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~FFmpegFingerprintDecoder() override;

  /**
   * Run decoder on audio file.
   * @param filePath path to audio file
   */
  virtual void start(const QString& filePath) override;

private:
  qint16* m_buffer1;
  qint16* m_buffer2;
};
