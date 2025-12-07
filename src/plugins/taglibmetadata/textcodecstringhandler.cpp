/**
 * \file textcodecstringhandler.cpp
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

#include "textcodecstringhandler.h"

#include <tstring.h>
#include "taglibutils.h"

using namespace TagLibUtils;

#if QT_VERSION >= 0x060000
QStringDecoder TextCodecStringHandler::s_decoder;
QStringEncoder TextCodecStringHandler::s_encoder;
#else
const QTextCodec* TextCodecStringHandler::s_codec = nullptr;
#endif

/**
 * Decode a string from data.
 *
 * @param data data to decode
 */
TagLib::String TextCodecStringHandler::parse(const TagLib::ByteVector& data) const
{
#if QT_VERSION >= 0x060000
  return s_decoder.isValid()
      ? toTString(s_decoder(QByteArray(data.data(), data.size()))).stripWhiteSpace()
      : TagLib::String(data, TagLib::String::Latin1).stripWhiteSpace();
#else
  return s_codec
      ? toTString(s_codec->toUnicode(data.data(), data.size())).stripWhiteSpace()
      : TagLib::String(data, TagLib::String::Latin1).stripWhiteSpace();
#endif
}

/**
 * Encode a byte vector with the data from a string.
 *
 * @param s string to encode
 */
TagLib::ByteVector TextCodecStringHandler::render(const TagLib::String& s) const
{
#if QT_VERSION >= 0x060000
  if (s_encoder.isValid()) {
    QByteArray ba = s_encoder(toQString(s));
    return TagLib::ByteVector(ba.data(), ba.size());
  } else {
    return s.data(TagLib::String::Latin1);
  }
#else
  if (s_codec) {
    QByteArray ba(s_codec->fromUnicode(toQString(s)));
    return TagLib::ByteVector(ba.data(), ba.size());
  }
  return s.data(TagLib::String::Latin1);
#endif
}
