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

namespace {

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
    return m_ptr && m_ptr->codec_type == AVMEDIA_TYPE_AUDIO;
  }

  bool open() {
    AVCodec* codec;
    m_opened = false;
    if (m_ptr &&
        (codec = ::avcodec_find_decoder(m_ptr->codec_id)) != 0) {
      m_ptr->request_sample_fmt = AV_SAMPLE_FMT_S16;
      m_opened =
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 5, 0)
        ::avcodec_open(m_ptr, codec) >= 0
#else
        ::avcodec_open2(m_ptr, codec, 0) >= 0
#endif
          ;
    }
    return m_opened;
  }

  int channels() const { return m_ptr->channels; }

  AVSampleFormat sampleFormat() const { return m_ptr->sample_fmt; }

  int sampleRate() const { return m_ptr->sample_rate; }

  uint64_t channelLayout() const { return m_ptr->channel_layout; }

  int decode(int16_t* samples, int* frameSize, AVPacket* pkt) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 23, 0)
    return ::avcodec_decode_audio2(m_ptr,
      samples, frameSize, pkt->data, pkt->size);
#elif LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 25, 0)
    return ::avcodec_decode_audio3(m_ptr,
      samples, frameSize, pkt);
#else
    AVFrame frame;
    ::memset(&frame, 0, sizeof(frame));
    ::avcodec_get_frame_defaults(&frame);
    int decoded = 0;
    int len = ::avcodec_decode_audio4(m_ptr, &frame, &decoded, pkt);
    if (len >= 0 && decoded) {
      int planar = ::av_sample_fmt_is_planar(m_ptr->sample_fmt);
      int planeSize;
      int dataSize = ::av_samples_get_buffer_size(&planeSize, m_ptr->channels,
                         frame.nb_samples, m_ptr->sample_fmt, 1);
      if (*frameSize < dataSize)
        return -1;
      ::memcpy(samples, frame.extended_data[0], planeSize);
      if (planar && m_ptr->channels > 1) {
        uint8_t* out = reinterpret_cast<uint8_t*>(samples) + planeSize;
        for (int ch = 1; ch < m_ptr->channels; ++ch) {
          ::memcpy(out, frame.extended_data[ch], planeSize);
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
  AVCodecContext* m_ptr;
  bool m_opened;
};

#ifdef HAVE_AVRESAMPLE
class Converter {
public:
  Converter() : m_ptr(0), m_isOpen(false) {}

  ~Converter() {
    if (m_ptr) {
      if (m_isOpen) {
        ::avresample_close(m_ptr);
      }
      ::avresample_close(m_ptr);
    }
  }

  bool createForCodec(const Codec& codecCtx) {
    if ((m_ptr = ::avresample_alloc_context()) != 0) {
      ::av_opt_set_int(m_ptr, "in_channel_layout",  codecCtx.channelLayout(), 0);
      ::av_opt_set_int(m_ptr, "in_sample_fmt",      codecCtx.sampleFormat(), 0);
      ::av_opt_set_int(m_ptr, "in_sample_rate",     codecCtx.sampleRate(), 0);
      ::av_opt_set_int(m_ptr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
      ::av_opt_set_int(m_ptr, "out_sample_fmt",     AV_SAMPLE_FMT_S16, 0);
      ::av_opt_set_int(m_ptr, "out_sample_rate",    44100, 0);
      m_isOpen = ::avresample_open(m_ptr) >= 0;
      return m_isOpen;
    }
    return false;
  }

  int16_t* convert(const Codec& codecCtx,
                   int16_t* buffer1, int16_t* buffer2,
                   int& bufferSize) {
    if (m_ptr) {
      int bytesPerSample = ::av_get_bytes_per_sample(codecCtx.sampleFormat());
      int numSamplesIn = bufferSize / bytesPerSample;
      int linesizeIn;
      ::av_samples_get_buffer_size(&linesizeIn, codecCtx.channels(),
          numSamplesIn / codecCtx.channels(), codecCtx.sampleFormat(), 0);
#if LIBAVRESAMPLE_VERSION_INT < AV_VERSION_INT(1, 0, 0)
      int numSamplesOut = ::avresample_convert(
            m_ptr, reinterpret_cast<void**>(&buffer2), 0, BUFFER_SIZE,
            reinterpret_cast<void**>(&buffer1), linesizeIn, numSamplesIn);
#else
      int numSamplesOut = ::avresample_convert(
            m_ptr, reinterpret_cast<uint8_t**>(&buffer2), 0, BUFFER_SIZE,
            reinterpret_cast<uint8_t**>(&buffer1), linesizeIn, numSamplesIn);
#endif
      if (numSamplesOut < 0) {
        return 0;
      }
      bufferSize = numSamplesOut * 2;
      return buffer2;
    } else {
      return buffer1;
    }
  }

private:
  AVAudioResampleContext* m_ptr;
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

  AVStream* stream = 0;
  Codec codec;
  for (unsigned i = 0; i < format.numStreams(); ++i) {
    codec.assign(format.streams()[i]->codec);
    if (codec.codecTypeIsAudio()) {
      stream = format.streams()[i];
      break;
    }
  }
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

  duration = stream->time_base.num * stream->duration / stream->time_base.den;

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
