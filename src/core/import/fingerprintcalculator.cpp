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

#define __STDC_CONSTANT_MACROS
#include "fingerprintcalculator.h"

#ifdef HAVE_CHROMAPRINT

#ifdef HAVE_GSTREAMER

#include <string.h>
#include <gst/gst.h>
#include <chromaprint.h>
#include <QFileInfo>
#include <QUrl>

class FingerprintCalculator::Decoder {
public:
  explicit Decoder(ChromaprintContext* ctx);
  ~Decoder();

  bool run(const QString& filePath);

  int getDuration() const { return m_duration; }
  FingerprintCalculator::Result::Error getError() const { return m_error; }

private:
  static const int BUFFER_SIZE = 10;
  static const gint64 MAX_LENGTH_NS = 120000000000;
  static const guint TIMEOUT_MS = 5000;

  void raiseError(FingerprintCalculator::Result::Error error);
  static gboolean cb_timeout(gpointer data);
  static void cb_message(GstBus* bus, GstMessage* message, Decoder* self);
  static void cb_pad_added(GstElement* dec, GstPad* pad, Decoder* self);
  static void cb_no_more_pads(GstElement* dec, Decoder* self);
  static void cb_notify_caps(GstPad *pad, GParamSpec* spec, Decoder* self);
  static void cb_unknown_type(GstElement* dec, GstPad* pad, GstCaps* caps, Decoder* self);
  static void cb_new_buffer(GstElement* sink, Decoder* self);

  ChromaprintContext* m_chromaprint;
  GMainLoop* m_loop;
  GstElement* m_pipeline;
  GstElement* m_dec;
  GstElement* m_conv;
  FingerprintCalculator::Result::Error m_error;
  int m_duration;
  gint m_channels;
  gint m_rate;
  bool m_gotPad;
};

FingerprintCalculator::Decoder::Decoder(ChromaprintContext* ctx) :
  m_chromaprint(ctx), m_error(FingerprintCalculator::Result::Ok),
  m_duration(0), m_channels(0), m_rate(0), m_gotPad(false)
{
  gst_init(NULL, NULL);
//  gst_debug_set_default_threshold(GST_LEVEL_INFO);
//  gst_debug_set_colored(FALSE);
  m_loop = g_main_loop_new(NULL, FALSE);
  m_pipeline = gst_pipeline_new("pipeline");
  m_dec = gst_element_factory_make("uridecodebin", "dec");
  m_conv = gst_element_factory_make("audioconvert", "conv");
  GstElement* sink = gst_element_factory_make("appsink", "sink");

  if (m_loop && m_pipeline && m_dec && m_conv && sink) {
    if (GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline))) {
      gst_bus_add_signal_watch(bus);
      g_signal_connect(bus, "message::eos", G_CALLBACK(cb_message), this);
      g_signal_connect(bus, "message::error", G_CALLBACK(cb_message), this);
      gst_object_unref(GST_OBJECT(bus));
    }

    g_signal_connect(m_dec, "pad-added", G_CALLBACK(cb_pad_added), this);
    g_signal_connect(m_dec, "no-more-pads", G_CALLBACK(cb_no_more_pads), this);
    g_signal_connect(m_dec, "unknown-type", G_CALLBACK(cb_unknown_type), this);

    if (GstCaps* sinkcaps = gst_caps_new_simple("audio/x-raw-int",
      "width", G_TYPE_INT, 16,
      "depth", G_TYPE_INT, 16,
      "signed", G_TYPE_BOOLEAN, TRUE,
      NULL)) {
      g_object_set(G_OBJECT(sink), "caps", sinkcaps, NULL);
      gst_caps_unref(sinkcaps);
    }
    g_object_set(G_OBJECT(sink),
                 "drop", FALSE,
                 "max-buffers", BUFFER_SIZE,
                 "sync", FALSE,
                 "emit-signals", TRUE,
                 NULL);
    g_signal_connect(sink, "new-buffer", G_CALLBACK(cb_new_buffer), this);
    if (GstPad* pad = gst_element_get_static_pad(sink, "sink")) {
      g_signal_connect(pad, "notify::caps", G_CALLBACK(cb_notify_caps), this);
      gst_object_unref(pad);
    }

    gst_bin_add_many(GST_BIN(m_pipeline), m_dec, m_conv, sink, NULL);
    gst_element_link_many(m_conv, sink, NULL);
  } else {
    if (m_loop) {
      g_main_loop_unref(m_loop);
      m_loop = NULL;
    } else {
      g_print("Failed to create main loop.\n");
    }
    if (m_pipeline) {
      gst_object_unref(m_pipeline);
      m_pipeline = NULL;
    } else {
      g_print("Failed to create pipeline.\n");
    }
    if (m_dec) {
      gst_object_unref(m_dec);
      m_dec = NULL;
    } else {
      g_print("Failed to create uridecodebin.\n");
    }
    if (m_conv) {
      gst_object_unref(m_conv);
      m_conv = NULL;
    } else {
      g_print("Failed to create audioconvert.\n");
    }
    if (sink) {
      gst_object_unref(sink);
    } else {
      g_print("Failed to create appsink.\n");
    }
  }
}

FingerprintCalculator::Decoder::~Decoder()
{
  if (m_pipeline) {
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    gst_object_unref(m_pipeline);
  }
  if (m_loop) {
    g_main_loop_unref(m_loop);
  }
}

void FingerprintCalculator::Decoder::raiseError(
    FingerprintCalculator::Result::Error error)
{
  m_error = error;
  g_main_loop_quit(m_loop);
}

gboolean FingerprintCalculator::Decoder::cb_timeout(gpointer data)
{
  Decoder* self = reinterpret_cast<Decoder*>(data);
  self->raiseError(FingerprintCalculator::Result::Timeout);
  return FALSE;
}

void FingerprintCalculator::Decoder::cb_message(GstBus*, GstMessage* message,
                                  Decoder* self)
{
  switch (GST_MESSAGE_TYPE(message)) {
  case GST_MESSAGE_ERROR: {
    GError *err;
    gchar *debug;
    gst_message_parse_error(message, &err, &debug);
    g_print("Error: %s\n", err->message);
    g_error_free(err);
    g_free(debug);
    self->raiseError(FingerprintCalculator::Result::DecoderError);
    break;
  }
  case GST_MESSAGE_EOS:
    // end-of-stream
    g_main_loop_quit(self->m_loop);
    break;
  default:
    break;
  }
}

void FingerprintCalculator::Decoder::cb_pad_added(GstElement*, GstPad* pad,
                                    Decoder* self)
{
  if (GstCaps* caps = gst_pad_get_caps(pad)) {
    const GstStructure* str = gst_caps_get_structure(caps, 0);
    const gchar* name = gst_structure_get_name(str);
    if (name && strncmp(name, "audio/x-raw-", 12) == 0) {
      if (GstPad* nextpad = gst_element_get_static_pad(self->m_conv, "sink")) {
        if (!gst_pad_is_linked(nextpad)) {
          if (gst_pad_link(pad, nextpad) == GST_PAD_LINK_OK) {
            self->m_gotPad = true;
          } else {
            g_print("Failed to link pads\n");
          }
        }
        gst_object_unref(nextpad);
      }
    }
    gst_caps_unref(caps);
  }
}

void FingerprintCalculator::Decoder::cb_no_more_pads(GstElement*, Decoder* self)
{
  if (!self->m_gotPad) {
    self->raiseError(FingerprintCalculator::Result::NoStreamFound);
  }
}

void FingerprintCalculator::Decoder::cb_notify_caps(GstPad *pad, GParamSpec*, Decoder* self)
{
  if (GstCaps* caps = gst_pad_get_negotiated_caps(pad)) {
    const GstStructure* str = gst_caps_get_structure(caps, 0);
    if (gst_structure_get_int(str, "channels", &self->m_channels) &&
        gst_structure_get_int(str, "rate", &self->m_rate)) {
      ::chromaprint_start(self->m_chromaprint, self->m_rate, self->m_channels);
    } else {
      g_print("No channels/rate available\n");
    }
    gst_caps_unref(caps);
  }
  if (GstQuery* query = gst_query_new_duration(GST_FORMAT_TIME)) {
    if (GstPad* peer = gst_pad_get_peer(pad)) {
      if (gst_pad_query(peer, query)) {
        GstFormat format;
        gint64 length;
        gst_query_parse_duration(query, &format, &length);
        if (format == GST_FORMAT_TIME) {
          self->m_duration = length / 1000000000;
        }
      }
      gst_object_unref(peer);
    }
    gst_query_unref(query);
  }
}

void FingerprintCalculator::Decoder::cb_unknown_type(GstElement*, GstPad*, GstCaps* caps,
                                       Decoder* self)
{
  bool isAudio = false;
  if (gchar* streaminfo = gst_caps_to_string(caps)) {
    isAudio = strncmp(streaminfo, "audio/", 6) == 0;
    g_free(streaminfo);
  }
  if (!isAudio)
    return;
  self->raiseError(FingerprintCalculator::Result::NoCodecFound);
}

void FingerprintCalculator::Decoder::cb_new_buffer(GstElement* sink, Decoder* self)
{
  GstBuffer *buffer;
  g_signal_emit_by_name(sink, "pull-buffer", &buffer);
  if (buffer) {
    gint64 buf_pos = GST_BUFFER_TIMESTAMP(buffer);
    size_t len = GST_BUFFER_SIZE(buffer);
    guint8* data = GST_BUFFER_DATA(buffer);
    bool error = !::chromaprint_feed(self->m_chromaprint, data, len / 2);
    gst_buffer_unref(buffer);
    if (error) {
      self->raiseError(FingerprintCalculator::Result::FingerprintCalculationFailed);
    }
    if (buf_pos >= MAX_LENGTH_NS) {
      g_main_loop_quit(self->m_loop);
    }
  }
}

bool FingerprintCalculator::Decoder::run(const QString& filePath)
{
  if (!m_loop) {
    // Initialization failed
    m_error = FingerprintCalculator::Result::DecoderError;
    return false;
  }

  m_error = FingerprintCalculator::Result::Ok;
  m_duration = 0;
  m_channels = 0;
  m_rate = 0;
  m_gotPad = false;

  QByteArray url(
      QUrl::fromLocalFile(QFileInfo(filePath).absoluteFilePath()).toEncoded());
  g_object_set(G_OBJECT(m_dec), "uri", url.constData(), NULL);

  gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING);

  guint timeoutFuncId = g_timeout_add(TIMEOUT_MS, cb_timeout, this);
  g_main_loop_run(m_loop);
  g_source_remove(timeoutFuncId);

  gst_element_set_state(m_pipeline, GST_STATE_READY);
  return m_error == FingerprintCalculator::Result::Ok;
}

#else // HAVE_GSTREAMER

#include <stdint.h>
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
#elif LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 25, 0)
    return ::avcodec_decode_audio3(m_ptr,
      samples, frameSize, pkt);
#else
    AVFrame frame;
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

#ifdef HAVE_AV_AUDIO_CONVERT
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

class FingerprintCalculator::Decoder {
public:
  explicit Decoder(ChromaprintContext* ctx);
  ~Decoder();

  bool run(const QString& filePath);

  int getDuration() const { return m_duration; }
  FingerprintCalculator::Result::Error getError() const { return m_error; }

private:
  ChromaprintContext* m_chromaprint;
  qint16* m_buffer1;
  qint16* m_buffer2;
  int m_duration;
  FingerprintCalculator::Result::Error m_error;
};

FingerprintCalculator::Decoder::Decoder(ChromaprintContext* ctx) :
  m_chromaprint(ctx), m_duration(0), m_error(FingerprintCalculator::Result::Ok)
{
  ::av_register_all();
  ::av_log_set_level(AV_LOG_ERROR);

  m_buffer1 = reinterpret_cast<qint16*>(::av_malloc(BUFFER_SIZE + 16));
  m_buffer2 = reinterpret_cast<qint16*>(::av_malloc(BUFFER_SIZE + 16));
}

FingerprintCalculator::Decoder::~Decoder()
{
  ::av_free(m_buffer1);
  ::av_free(m_buffer2);
}

bool FingerprintCalculator::Decoder::run(const QString& filePath)
{
  /*
   * The code here is based on fpcalc.c from chromaprint-0.6/examples.
   */
  m_error = FingerprintCalculator::Result::Ok;
  m_duration = 0;
  QByteArray fileName(QFile::encodeName(filePath));
  Format format(fileName.constData());
  if (format.hasError()) {
    m_error = Result::NoStreamFound;
    return false;
  }

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
    m_error = Result::NoStreamFound;
    return false;
  }

  if (!codec.open() || codec.channels() <= 0) {
    m_error = Result::NoCodecFound;
    return false;
  }

  Converter converter;
  if (codec.sampleFormat() != AV_SAMPLE_FMT_S16) {
    if (!converter.createForCodec(codec)) {
      m_error = Result::NoConverterFound;
      return false;
    }
  }

  m_duration = stream->time_base.num * stream->duration / stream->time_base.den;

  AVPacket packet, packetTemp;
  ::av_init_packet(&packet);
  ::av_init_packet(&packetTemp);

  const int MAX_LENGTH = 120;
  int remaining = MAX_LENGTH * codec.channels() * codec.sampleRate();
  ::chromaprint_start(m_chromaprint, codec.sampleRate(), codec.channels());

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
      if (!::chromaprint_feed(m_chromaprint, buffer, length)) {
        m_error = Result::FingerprintCalculationFailed;
        return false;
      }

      remaining -= length;
      if (remaining <= 0) {
        break;
      }
    }
  }

  return true;
}

#endif // HAVE_GSTREAMER

/**
 * Constructor.
 */
FingerprintCalculator::FingerprintCalculator() :
  m_chromaprintCtx(0), m_decoder(0)
{
}

/**
 * Destructor.
 */
FingerprintCalculator::~FingerprintCalculator()
{
  if (m_chromaprintCtx) {
    delete m_decoder;
    ::chromaprint_free(m_chromaprintCtx);
  }
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
  if (!m_chromaprintCtx) {
    // Lazy intialization to save resources if not used
    m_chromaprintCtx = ::chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
    m_decoder = new Decoder(m_chromaprintCtx);
  }
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
  if (!m_decoder->run(filePath)) {
    return m_decoder->getError();
  }
  duration = m_decoder->getDuration();
  if (!::chromaprint_finish(m_chromaprintCtx)) {
    return Result::FingerprintCalculationFailed;
  }

  return Result::Ok;
}

#endif // HAVE_CHROMAPRINT
