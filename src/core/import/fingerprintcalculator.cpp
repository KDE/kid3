/**
 * \file fingerprintcalculator.cpp
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

#include "fingerprintcalculator.h"

#ifdef HAVE_CHROMAPRINT

#define __STDC_CONSTANT_MACROS
#include <stdio.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef HAVE_AV_AUDIO_CONVERT
#include <libavutil/audioconvert.h>
#include <libavutil/samplefmt.h>

/*
 * Declarations taken from "ffmpeg/audioconvert.h", "ffmpeg/samplefmt.h".
 */
struct AVAudioConvert;
typedef struct AVAudioConvert AVAudioConvert;

AVAudioConvert *av_audio_convert_alloc(enum AVSampleFormat out_fmt, int out_channels,
                                       enum AVSampleFormat in_fmt, int in_channels,
                                       const float *matrix, int flags);
void av_audio_convert_free(AVAudioConvert *ctx);
int av_audio_convert(AVAudioConvert *ctx,
                           void * const out[6], const int out_stride[6],
                     const void * const  in[6], const int  in_stride[6], int len);
#endif
}
#include <chromaprint.h>
#include <QFile>

const int BUFFER_SIZE = AVCODEC_MAX_AUDIO_FRAME_SIZE * 2;

/*
 * The following classes are used to benefit from the C++
 * "Resource Acquisition Is Initialization" (RAII) idiom when dealing with
 * AV resources.
 */

class Packet {
public:
  explicit Packet(AVPacket* packet) : m_ptr(packet) {
  }

  AVPacket* data() { return m_ptr; }

  ~Packet() {
    if (m_ptr && m_ptr->data) {
      ::av_free_packet(m_ptr);
    }
  }

private:
  AVPacket* m_ptr;
};

class Format {
public:
  Format(const char* fileName) : m_ptr(0), m_hasError(false) {
    if (
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
        ::av_open_input_file(&m_ptr, fileName, 0, 0, 0) != 0
#else
        ::avformat_open_input(&m_ptr, fileName, 0, 0) != 0
#endif
        || ::av_find_stream_info(m_ptr) < 0)
      m_hasError = true;
  }

  ~Format() {
    if (m_ptr)
      ::av_close_input_file(m_ptr);
  }

  bool hasError() const { return m_hasError; }

  unsigned int numStreams() const {
    return m_ptr->nb_streams;
  }

  AVStream** streams() const { return m_ptr->streams; }

  bool readFrame(Packet& packet) {
    return ::av_read_frame(m_ptr, packet.data()) >= 0;
  }

private:
  AVFormatContext* m_ptr;
  bool m_hasError;
};

class Codec {
public:
  explicit Codec(AVCodecContext* ptr = 0) : m_ptr(ptr), m_opened(false) {
  }

  ~Codec() {
    if (m_opened)
      ::avcodec_close(m_ptr);
  }

  bool isNull() const { return m_ptr == 0; }

  void assign(AVCodecContext* ptr) { m_ptr = ptr; }

  bool codecTypeIsAudio() const {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 64, 0)
    return m_ptr && m_ptr->codec_type == CODEC_TYPE_AUDIO;
#else
    return m_ptr && m_ptr->codec_type == AVMEDIA_TYPE_AUDIO;
#endif
  }

  bool open() {
    AVCodec* codec;
    m_opened = (m_ptr &&
                (codec = ::avcodec_find_decoder(m_ptr->codec_id)) != 0 &&
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 5, 0)
        ::avcodec_open(m_ptr, codec) >= 0
#else
        ::avcodec_open2(m_ptr, codec, 0) >= 0
#endif
        );
    return m_opened;
  }

  int channels() const { return m_ptr->channels; }

  AVSampleFormat sampleFormat() const { return m_ptr->sample_fmt; }

  int sampleRate() const { return m_ptr->sample_rate; }

  int decode(int16_t* samples, int* frameSize, AVPacket* pkt) {
#if LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(52, 25, 0)
    return ::avcodec_decode_audio2(m_ptr,
      samples, frameSize, pkt->data, pkt->size);
    #else
    return ::avcodec_decode_audio3(m_ptr,
      samples, frameSize, pkt);
#endif
  }

private:
  AVCodecContext* m_ptr;
  bool m_opened;
};

#ifdef HAVE_AV_AUDIO_CONVERT
class Converter {
public:
  Converter() : m_ptr(0) {}

  ~Converter() {
    if (m_ptr)
      ::av_audio_convert_free(m_ptr);
  }

  bool createForCodec(const Codec& codecCtx) {
    m_ptr = ::av_audio_convert_alloc(SAMPLE_FMT_S16, codecCtx.channels(),
                 codecCtx.sampleFormat(), codecCtx.channels(), 0, 0);
    return m_ptr != 0;
  }

  int16_t* convert(const Codec& codecCtx,
                   int16_t* buffer1, int16_t* buffer2,
                   int& bufferSize) {
    if (m_ptr) {
      const void *ibuf[6] = { buffer1 };
      void *obuf[6] = { buffer2 };
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(51, 4, 0)
      int istride[6] = { ::av_get_bits_per_sample_format(codecCtx.sampleFormat()) / 8 };
#else
      int istride[6] = { ::av_get_bytes_per_sample(codecCtx.sampleFormat()) };
#endif
      int ostride[6] = { 2 };
      int len = bufferSize / istride[0];
      if (::av_audio_convert(m_ptr, obuf, ostride, ibuf, istride, len) < 0) {
        return 0;
      }
      bufferSize = len * ostride[0];
      return buffer2;
    } else {
      return buffer1;
    }
  }

private:
  AVAudioConvert* m_ptr;
};
#else
class Converter {
public:
  bool createForCodec(const Codec&) { return false; }
  int16_t* convert(const Codec&, int16_t* buffer1, int16_t*, int&) {
    return buffer1;
  }
};
#endif


/**
 * Constructor.
 */
FingerprintCalculator::FingerprintCalculator()
{
  ::av_register_all();
  ::av_log_set_level(AV_LOG_ERROR);

  m_buffer1 = reinterpret_cast<qint16*>(::av_malloc(BUFFER_SIZE + 16));
  m_buffer2 = reinterpret_cast<qint16*>(::av_malloc(BUFFER_SIZE + 16));
  m_chromaprintCtx = ::chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
}

/**
 * Destructor.
 */
FingerprintCalculator::~FingerprintCalculator()
{
  ::chromaprint_free(m_chromaprintCtx);
  ::av_free(m_buffer1);
  ::av_free(m_buffer2);
}

/**
 * Calculate audio fingerprint for audio file.
 *
 * @param fileName path to audio file
 *
 * @return result of fingerprint calculation.
 */
FingerprintCalculator::Result FingerprintCalculator::calculateFingerprint(
    const QString &fileName) {
  Result result;
  result.m_error = decodeAudioFile(fileName, result.m_duration);
  if (result.m_error == Result::Ok) {
    char* fingerprint;
    if (::chromaprint_get_fingerprint(m_chromaprintCtx, &fingerprint)) {
      result.m_fingerprint = QString::fromAscii(fingerprint);
      ::chromaprint_dealloc(fingerprint);
    } else {
      result.m_error = Result::FingerprintCalculationFailed;
    }
  }
  return result;
}

/**
 * Decode audio file.
 *
 * @param filePath path to audio file
 * @param duration the length of the audio file in seconds is returned here
 *
 * @return Ok if OK, else error code.
 */
FingerprintCalculator::Result::Error FingerprintCalculator::decodeAudioFile(
    const QString& filePath, int& duration)
{
  /*
   * The code here is based on fpcalc.c from chromaprint-0.6/examples.
   */
  QByteArray fileName(QFile::encodeName(filePath));
  Format format(fileName.constData());
  if (format.hasError())
    return Result::NoStreamFound;

  AVStream* stream = 0;
  Codec codec;
  for (unsigned i = 0; i < format.numStreams(); i++) {
    codec.assign(format.streams()[i]->codec);
    if (codec.codecTypeIsAudio()) {
      stream = format.streams()[i];
      break;
    }
  }
  if (!stream) {
    return Result::NoStreamFound;
  }

  if (!codec.open() || codec.channels() <= 0)
    return Result::NoCodecFound;

  Converter converter;
  if (codec.sampleFormat() != SAMPLE_FMT_S16) {
    if (!converter.createForCodec(codec))
      return Result::NoConverterFound;
  }

  duration = stream->time_base.num * stream->duration / stream->time_base.den;

  AVPacket packet, packetTemp;
  ::av_init_packet(&packet);
  ::av_init_packet(&packetTemp);

  const int MAX_LENGTH = 120;
  int remaining = MAX_LENGTH * codec.channels() * codec.sampleRate();
  ::chromaprint_start(m_chromaprintCtx, codec.sampleRate(), codec.channels());

  while (remaining > 0) {
    Packet pkt(&packet);
    if (!format.readFrame(pkt))
      break;

    packetTemp.data = packet.data;
    packetTemp.size = packet.size;

    while (packetTemp.size > 0) {
      int bufferSize = BUFFER_SIZE;
      int consumed = codec.decode(m_buffer1, &bufferSize, &packetTemp);

      if (consumed < 0) {
        break;
      }

      packetTemp.data += consumed;
      packetTemp.size -= consumed;

      if (bufferSize <= 0 || bufferSize > BUFFER_SIZE) {
        continue;
      }

      int16_t *buffer = converter.convert(codec, m_buffer1, m_buffer2, bufferSize);
      if (!buffer)
        break;

      int length = qMin(remaining, bufferSize / 2);
      if (!::chromaprint_feed(m_chromaprintCtx, buffer, length)) {
        return Result::FingerprintCalculationFailed;
      }

      remaining -= length;
      if (remaining <= 0) {
        break;
      }
    }
  }

  if (!::chromaprint_finish(m_chromaprintCtx)) {
    return Result::FingerprintCalculationFailed;
  }

  return Result::Ok;
}

#endif // HAVE_CHROMAPRINT
