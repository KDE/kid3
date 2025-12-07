/**
 * \file textcodecstringhandler.h
 * Data encoding in ID3v1 tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Dec 2025
 *
 * Copyright (C) 2025  Urs Fleisch
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

#include <QString>
#if QT_VERSION >= 0x060000
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <id3v1tag.h>

/**
 * Data encoding in ID3v1 tags.
 */
class TextCodecStringHandler : public TagLib::ID3v1::StringHandler {
public:
  /**
   * Constructor.
   */
  TextCodecStringHandler() = default;

  /**
   * Destructor.
   */
  ~TextCodecStringHandler() = default; // override would fail with TagLib 1.x

  TextCodecStringHandler(const TextCodecStringHandler&) = delete;
  TextCodecStringHandler& operator=(const TextCodecStringHandler&) = delete;

  /**
   * Decode a string from data.
   *
   * @param data data to decode
   */
  TagLib::String parse(const TagLib::ByteVector& data) const override;

  /**
   * Encode a byte vector with the data from a string.
   *
   * @param s string to encode
   */
  TagLib::ByteVector render(const TagLib::String& s) const override;

#if QT_VERSION >= 0x060000
  /**
   * Set string decoder.
   * @param encodingName encoding, empty for default behavior (ISO 8859-1)
   */
  static void setStringDecoder(const QString& encodingName) {
    if (auto encoding = QStringConverter::encodingForName(encodingName.toLatin1())) {
      s_encoder = QStringEncoder(*encoding);
      s_decoder = QStringDecoder(*encoding);
    } else {
      s_encoder = QStringEncoder();
      s_decoder = QStringDecoder();
    }
  }
#else
  /**
   * Set text codec.
   * @param codec text codec, 0 for default behavior (ISO 8859-1)
   */
  static void setTextCodec(const QTextCodec* codec) { s_codec = codec; }
#endif

private:
#if QT_VERSION >= 0x060000
  static QStringDecoder s_decoder;
  static QStringEncoder s_encoder;
#else
  static const QTextCodec* s_codec;
#endif
};
