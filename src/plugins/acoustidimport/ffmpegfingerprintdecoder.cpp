/**
 * \file ffmpegfingerprintdecoder.cpp
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

/** Needed for UINT64_C, INT64_C macros used by libav includes. */
#define __STDC_CONSTANT_MACROS
#include "ffmpegfingerprintdecoder.h"
#include "acoustidconfig.h"

#include <stdint.h>
#include <stdio.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef HAVE_AVRESAMPLE
#include <libavresample/avresample.h>
#include <libavutil/opt.h>
#elif defined HAVE_SWRESAMPLE
#include <libswresample/swresample.h>
#elif defined HAVE_AV_AUDIO_CONVERT
#include <libavutil/audioconvert.h>
#include <libavutil/samplefmt.h>

/*
 * Declarations taken from "ffmpeg/audioconvert.h", "ffmpeg/samplefmt.h".
 */
//! @cond
struct AVAudioConvert;
typedef struct AVAudioConvert AVAudioConvert;

AVAudioConvert *av_audio_convert_alloc(enum AVSampleFormat out_fmt, int out_channels,
                                       enum AVSampleFormat in_fmt, int in_channels,
                                       const float *matrix, int flags);
void av_audio_convert_free(AVAudioConvert *ctx);
int av_audio_convert(AVAudioConvert *ctx,
                           void * const out[6], const int out_stride[6],
                     const void * const  in[6], const int  in_stride[6], int len);
//! @endcond
#endif
}
#include <QFile>
#include "fingerprintcalculator.h"

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 94, 1)
#define AV_SAMPLE_FMT_S16 SAMPLE_FMT_S16
#define AVMEDIA_TYPE_AUDIO CODEC_TYPE_AUDIO
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 28, 0)
#define avcodec_free_frame av_freep
#endif

/** Bytes needed for 1 second of 48khz 32bit audio. */
#ifdef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define MAX_AUDIO_FRAME_SIZE AVCODEC_MAX_AUDIO_FRAME_SIZE
#else
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#endif

namespace {

const int BUFFER_SIZE = MAX_AUDIO_FRAME_SIZE * 2;

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

  int streamIndex() const { return m_ptr ? m_ptr->stream_index : -1; }

private:
  AVPacket* m_ptr;
};

class Codec {
public:
  explicit Codec(AVCodecContext* ptr = 0) : m_ptr(ptr), m_impl(0), m_frame(0),
    m_opened(false) {
  }

  ~Codec() {
    if (m_frame)
      ::avcodec_free_frame(&m_frame);
    if (m_opened)
      ::avcodec_close(m_ptr);
  }

  bool open() {
    m_opened = false;
    if (m_ptr && m_impl) {
      m_opened =
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 5, 0)
        ::avcodec_open(m_ptr, m_impl) >= 0
#else
        ::avcodec_open2(m_ptr, m_impl, 0) >= 0
#endif
          ;
    }
    return m_opened;
  }

  int channels() const { return m_ptr->channels; }

  AVSampleFormat sampleFormat() const { return m_ptr->sample_fmt; }

  int sampleRate() const { return m_ptr->sample_rate; }

#if defined HAVE_AVRESAMPLE || defined HAVE_SWRESAMPLE
  uint64_t channelLayout() const { return m_ptr->channel_layout; }
#endif

  int decode(int16_t* samples, int* frameSize, AVPacket* pkt) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 23, 0)
    return ::avcodec_decode_audio2(m_ptr,
      samples, frameSize, pkt->data, pkt->size);
#elif LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 25, 0)
    return ::avcodec_decode_audio3(m_ptr,
      samples, frameSize, pkt);
#else
    if (!m_frame)
      m_frame = ::avcodec_alloc_frame();
    ::avcodec_get_frame_defaults(m_frame);
    int decoded = 0;
    int len = ::avcodec_decode_audio4(m_ptr, m_frame, &decoded, pkt);
    if (len >= 0 && decoded) {
      int planar = ::av_sample_fmt_is_planar(m_ptr->sample_fmt);
      int planeSize;
      int dataSize = ::av_samples_get_buffer_size(&planeSize, m_ptr->channels,
                         m_frame->nb_samples, m_ptr->sample_fmt, 1);
      if (*frameSize < dataSize)
        return -1;
      ::memcpy(samples, m_frame->extended_data[0], planeSize);
      if (planar && m_ptr->channels > 1) {
        uint8_t* out = reinterpret_cast<uint8_t*>(samples) + planeSize;
        for (int ch = 1; ch < m_ptr->channels; ++ch) {
          ::memcpy(out, m_frame->extended_data[ch], planeSize);
          out += planeSize;
        }
      }
      *frameSize = dataSize;
    } else {
      *frameSize = 0;
    }
    return len;
#endif
  }

private:
  friend class Format;
  friend class Converter;
  AVCodecContext* m_ptr;
  AVCodec* m_impl;
  AVFrame* m_frame;
  bool m_opened;
};

class Format {
public:
  Format(const char* fileName) : m_ptr(0), m_streamIndex(-1), m_hasError(false) {
    if (
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
        ::av_open_input_file(&m_ptr, fileName, 0, 0, 0) != 0
#else
        ::avformat_open_input(&m_ptr, fileName, 0, 0) != 0
#endif
        ||
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 5, 0)
        ::av_find_stream_info(m_ptr) < 0
#else
        ::avformat_find_stream_info(m_ptr, 0) < 0
#endif
      )
      m_hasError = true;
  }

  ~Format() {
    if (m_ptr)
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 21, 0)
      ::av_close_input_file(m_ptr);
#else
      ::avformat_close_input(&m_ptr);
#endif
  }

  bool hasError() const { return m_hasError; }

  AVStream* findAudioStream(Codec* codec) {
    AVStream* stream = 0;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(52, 91, 0)
    for (unsigned i = 0; i < m_ptr->nb_streams; ++i) {
      codec->m_ptr = m_ptr->streams[i]->codec;
      if (codec->m_ptr && codec->m_ptr->codec_type == AVMEDIA_TYPE_AUDIO) {
        stream = m_ptr->streams[i];
        m_streamIndex = i;
        break;
      }
    }
    codec->m_impl = ::avcodec_find_decoder(codec->m_ptr->codec_id);
#else
    m_streamIndex = ::av_find_best_stream(m_ptr, AVMEDIA_TYPE_AUDIO, -1, -1,
                                          &codec->m_impl, 0);
    if (m_streamIndex >= 0) {
      stream = m_ptr->streams[m_streamIndex];
    }
    if (stream) {
      codec->m_ptr = stream->codec;
    }
#endif
    if (codec->m_ptr) {
      codec->m_ptr->request_sample_fmt = AV_SAMPLE_FMT_S16;
    }
    return stream;
  }

  int64_t duration() const { return m_ptr ? m_ptr->duration : AV_NOPTS_VALUE; }

  int streamIndex() const { return m_streamIndex; }

  bool readFrame(Packet& packet) {
    return ::av_read_frame(m_ptr, packet.data()) >= 0;
  }

private:
  AVFormatContext* m_ptr;
  int m_streamIndex;
  bool m_hasError;
};

#if defined HAVE_AVRESAMPLE || defined HAVE_SWRESAMPLE
class Converter {
public:
  Converter() : m_ptr(0), m_maxDstNumSamples(0), m_isOpen(false) {
    m_dstData[0] = 0;
  }

  ~Converter() {
    if (m_dstData[0]) {
      ::av_freep(&m_dstData[0]);
    }
    if (m_ptr) {
#ifdef HAVE_AVRESAMPLE
      if (m_isOpen) {
        ::avresample_close(m_ptr);
      }
      ::avresample_free(&m_ptr);
#elif defined HAVE_SWRESAMPLE
      ::swr_free(&m_ptr);
#endif
    }
  }

  bool createForCodec(const Codec& codecCtx) {
    int64_t channelLayout = codecCtx.channelLayout();
    if (!channelLayout) {
      channelLayout = ::av_get_default_channel_layout(codecCtx.channels());
    }
#ifdef HAVE_AVRESAMPLE
    if ((m_ptr = ::avresample_alloc_context()) != 0) {
      ::av_opt_set_int(m_ptr, "in_channel_layout",  channelLayout, 0);
      ::av_opt_set_int(m_ptr, "in_sample_fmt",      codecCtx.sampleFormat(), 0);
      ::av_opt_set_int(m_ptr, "in_sample_rate",     codecCtx.sampleRate(), 0);
      ::av_opt_set_int(m_ptr, "out_channel_layout", channelLayout, 0);
      ::av_opt_set_int(m_ptr, "out_sample_fmt",     AV_SAMPLE_FMT_S16, 0);
      ::av_opt_set_int(m_ptr, "out_sample_rate",    codecCtx.sampleRate(), 0);
      m_isOpen = ::avresample_open(m_ptr) >= 0;
      return m_isOpen;
    }
#elif defined HAVE_SWRESAMPLE
    if ((m_ptr = ::swr_alloc_set_opts(
           0, channelLayout, AV_SAMPLE_FMT_S16, codecCtx.sampleRate(),
           channelLayout, codecCtx.sampleFormat(), codecCtx.sampleRate(),
           0, 0)) != 0) {
      m_isOpen = ::swr_init(m_ptr) >= 0;
      return m_isOpen;
    }
#endif
    return false;
  }

  int16_t* convert(const Codec& codecCtx,
                   int16_t* buffer1, int16_t* buffer2,
                   int& bufferSize) {
    if (m_ptr) {
      int numSamplesOut;
      int16_t* result;
      if (codecCtx.m_frame) {
        if (codecCtx.m_frame->nb_samples > m_maxDstNumSamples) {
          ::av_freep(&m_dstData[0]);
          int dstLinesize = 0;
          if (::av_samples_alloc(m_dstData, &dstLinesize, codecCtx.channels(),
                      codecCtx.m_frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0) {
            return 0;
          }
          m_maxDstNumSamples = codecCtx.m_frame->nb_samples;
        }
#ifdef HAVE_AVRESAMPLE
#if LIBAVRESAMPLE_VERSION_INT < AV_VERSION_INT(1, 0, 0)
        numSamplesOut = ::avresample_convert(
              m_ptr, reinterpret_cast<void**>(m_dstData), 0,
              codecCtx.m_frame->nb_samples,
              reinterpret_cast<void**>(codecCtx.m_frame->data), 0,
              codecCtx.m_frame->nb_samples);
#else
        numSamplesOut = ::avresample_convert(
              m_ptr, m_dstData, 0, codecCtx.m_frame->nb_samples,
              reinterpret_cast<uint8_t**>(codecCtx.m_frame->data), 0,
              codecCtx.m_frame->nb_samples);
#endif
#elif defined HAVE_SWRESAMPLE
        numSamplesOut = ::swr_convert(
              m_ptr, m_dstData, codecCtx.m_frame->nb_samples,
              const_cast<const uint8_t**>(reinterpret_cast<uint8_t**>(
                                            codecCtx.m_frame->data)),
              codecCtx.m_frame->nb_samples);
#endif
        result = reinterpret_cast<int16_t*>(m_dstData[0]);
      } else {
        int bytesPerSample = ::av_get_bytes_per_sample(codecCtx.sampleFormat());
        int numSamplesIn = bufferSize / bytesPerSample;
        int linesizeIn;
        ::av_samples_get_buffer_size(&linesizeIn, codecCtx.channels(),
            numSamplesIn / codecCtx.channels(), codecCtx.sampleFormat(), 0);
#ifdef HAVE_AVRESAMPLE
#if LIBAVRESAMPLE_VERSION_INT < AV_VERSION_INT(1, 0, 0)
        numSamplesOut = ::avresample_convert(
              m_ptr, reinterpret_cast<void**>(&buffer2), 0, BUFFER_SIZE,
              reinterpret_cast<void**>(&buffer1), linesizeIn, numSamplesIn);
#else
        numSamplesOut = ::avresample_convert(
              m_ptr, reinterpret_cast<uint8_t**>(&buffer2), 0, BUFFER_SIZE,
              reinterpret_cast<uint8_t**>(&buffer1), linesizeIn, numSamplesIn);
#endif
#elif defined HAVE_SWRESAMPLE
        numSamplesOut = ::swr_convert(
              m_ptr, reinterpret_cast<uint8_t**>(&buffer2), BUFFER_SIZE,
              const_cast<const uint8_t**>(reinterpret_cast<uint8_t**>(&buffer1)),
              numSamplesIn);
#endif
        result = buffer2;
      }
      if (numSamplesOut < 0) {
        return 0;
      }
      bufferSize = ::av_samples_get_buffer_size(0, codecCtx.channels(),
                   numSamplesOut, AV_SAMPLE_FMT_S16, 1);
      return result;
    } else {
      return buffer1;
    }
  }

private:
#ifdef HAVE_AVRESAMPLE
  AVAudioResampleContext* m_ptr;
#elif defined HAVE_SWRESAMPLE
  SwrContext* m_ptr;
#endif
  uint8_t* m_dstData[1];
  int m_maxDstNumSamples;
  bool m_isOpen;
};
#elif defined HAVE_AV_AUDIO_CONVERT
class Converter {
public:
  Converter() : m_ptr(0) {}

  ~Converter() {
    if (m_ptr)
      ::av_audio_convert_free(m_ptr);
  }

  bool createForCodec(const Codec& codecCtx) {
    m_ptr = ::av_audio_convert_alloc(AV_SAMPLE_FMT_S16, codecCtx.channels(),
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

}


FFmpegFingerprintDecoder::FFmpegFingerprintDecoder(QObject* parent) :
  AbstractFingerprintDecoder(parent)
{
  ::av_register_all();
  ::av_log_set_level(AV_LOG_ERROR);

  m_buffer1 = reinterpret_cast<qint16*>(::av_malloc(BUFFER_SIZE + 16));
  m_buffer2 = reinterpret_cast<qint16*>(::av_malloc(BUFFER_SIZE + 16));
}

FFmpegFingerprintDecoder::~FFmpegFingerprintDecoder()
{
  ::av_free(m_buffer1);
  ::av_free(m_buffer2);
}

void FFmpegFingerprintDecoder::start(const QString& filePath)
{
  AbstractFingerprintDecoder::start(filePath);
  /*
   * The code here is based on fpcalc.c from chromaprint-0.6/examples.
   */
  FingerprintCalculator::Error err = FingerprintCalculator::Ok;
  int duration = 0;
  QByteArray fileName(QFile::encodeName(filePath));
  Format format(fileName.constData());
  if (format.hasError()) {
    err = FingerprintCalculator::NoStreamFound;
    emit error(err);
    return;
  }

  Codec codec;
  AVStream* stream = format.findAudioStream(&codec);
  if (!stream) {
    err = FingerprintCalculator::NoStreamFound;
    emit error(err);
    return;
  }

  if (!codec.open() || codec.channels() <= 0) {
    err = FingerprintCalculator::NoCodecFound;
    emit error(err);
    return;
  }

  Converter converter;
  if (codec.sampleFormat() != AV_SAMPLE_FMT_S16) {
    if (!converter.createForCodec(codec)) {
      err = FingerprintCalculator::NoConverterFound;
      emit error(err);
      return;
    }
  }

  if (stream->duration != static_cast<int64_t>(AV_NOPTS_VALUE)) {
    duration = stream->time_base.num * stream->duration / stream->time_base.den;
  } else if (format.duration() != static_cast<int64_t>(AV_NOPTS_VALUE)) {
    duration = format.duration() / AV_TIME_BASE;
  } else {
    err = FingerprintCalculator::NoStreamFound;
    emit error(err);
    return;
  }

  AVPacket packet, packetTemp;
  ::av_init_packet(&packet);
  ::av_init_packet(&packetTemp);

  const int MAX_LENGTH = 120;
  int remaining = MAX_LENGTH * codec.channels() * codec.sampleRate();
  emit started(codec.sampleRate(), codec.channels());

  while (remaining > 0) {
    Packet pkt(&packet);
    if (!format.readFrame(pkt))
      break;

    if (pkt.streamIndex() == format.streamIndex()) {
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
        emit bufferReady(QByteArray(reinterpret_cast<char*>(buffer), length * 2));
        if (isStopped()) {
          err = FingerprintCalculator::FingerprintCalculationFailed;
          emit error(err);
          return;
        }

        remaining -= length;
        if (remaining <= 0) {
          break;
        }
      }
    }
  }
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
  return new FFmpegFingerprintDecoder(parent);
}
