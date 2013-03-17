/**
 * \file gstfingerprintdecoder.cpp
 * Chromaprint fingerprint decoder using GStreamer.
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

#include "gstfingerprintdecoder.h"

#if defined HAVE_CHROMAPRINT && defined HAVE_GSTREAMER

#include <string.h>
#include <QFileInfo>
#include <QUrl>

/**
 * Constructor.
 * @param parent parent object
 */
GstFingerprintDecoder::GstFingerprintDecoder(QObject* parent) :
  AbstractFingerprintDecoder(parent),
  m_error(FingerprintCalculator::Ok),
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

    if (GstCaps* sinkcaps = gst_caps_new_simple(
#if GST_CHECK_VERSION(1, 0, 0)
      "audio/x-raw",
      "format", G_TYPE_STRING, "S16LE",
      "layout", G_TYPE_STRING, "interleaved",
      "rate", G_TYPE_INT, 44100,
      "channels", G_TYPE_INT, 2,
      "channel-mask", GST_TYPE_BITMASK, (gint64)0x3,
#else
      "audio/x-raw-int",
      "width", G_TYPE_INT, 16,
      "depth", G_TYPE_INT, 16,
      "signed", G_TYPE_BOOLEAN, TRUE,
#endif
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
#if GST_CHECK_VERSION(1, 0, 0)
    g_signal_connect(sink, "new-sample", G_CALLBACK(cb_new_buffer), this);
#else
    g_signal_connect(sink, "new-buffer", G_CALLBACK(cb_new_buffer), this);
#endif
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

/**
 * Destructor.
 */
GstFingerprintDecoder::~GstFingerprintDecoder()
{
  if (m_pipeline) {
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    gst_object_unref(m_pipeline);
  }
  if (m_loop) {
    g_main_loop_unref(m_loop);
  }
}

void GstFingerprintDecoder::raiseError(
    FingerprintCalculator::Error error)
{
  m_error = error;
  g_main_loop_quit(m_loop);
}

gboolean GstFingerprintDecoder::cb_timeout(gpointer data)
{
  GstFingerprintDecoder* self = reinterpret_cast<GstFingerprintDecoder*>(data);
  self->raiseError(FingerprintCalculator::Timeout);
  return FALSE;
}

void GstFingerprintDecoder::cb_message(GstBus*, GstMessage* message,
                                  GstFingerprintDecoder* self)
{
  switch (GST_MESSAGE_TYPE(message)) {
  case GST_MESSAGE_ERROR: {
    GError *err;
    gchar *debug;
    gst_message_parse_error(message, &err, &debug);
    g_print("Error: %s\n", err->message);
    g_error_free(err);
    g_free(debug);
    self->raiseError(FingerprintCalculator::DecoderError);
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

void GstFingerprintDecoder::cb_pad_added(GstElement*, GstPad* pad,
                                    GstFingerprintDecoder* self)
{
  if (GstCaps* caps =
#if GST_CHECK_VERSION(1, 0, 0)
      gst_pad_query_caps(pad, 0)
#else
      gst_pad_get_caps(pad)
#endif
      ) {
    const GstStructure* str = gst_caps_get_structure(caps, 0);
    const gchar* name = gst_structure_get_name(str);
    if (name && strncmp(name, "audio/x-raw", 11) == 0) {
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

void GstFingerprintDecoder::cb_no_more_pads(GstElement*, GstFingerprintDecoder* self)
{
  if (!self->m_gotPad) {
    self->raiseError(FingerprintCalculator::NoStreamFound);
  }
}

void GstFingerprintDecoder::cb_notify_caps(GstPad *pad, GParamSpec*, GstFingerprintDecoder* self)
{
  if (GstCaps* caps =
#if GST_CHECK_VERSION(1, 0, 0)
      gst_pad_get_current_caps(pad)
#else
      gst_pad_get_negotiated_caps(pad)
#endif
      ) {
    const GstStructure* str = gst_caps_get_structure(caps, 0);
    if (gst_structure_get_int(str, "channels", &self->m_channels) &&
        gst_structure_get_int(str, "rate", &self->m_rate)) {
      emit self->started(self->m_rate, self->m_channels);
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

void GstFingerprintDecoder::cb_unknown_type(GstElement*, GstPad*, GstCaps* caps,
                                       GstFingerprintDecoder* self)
{
  bool isAudio = false;
  if (gchar* streaminfo = gst_caps_to_string(caps)) {
    isAudio = strncmp(streaminfo, "audio/", 6) == 0;
    g_free(streaminfo);
  }
  if (!isAudio)
    return;
  self->raiseError(FingerprintCalculator::NoCodecFound);
}

void GstFingerprintDecoder::cb_new_buffer(GstElement* sink, GstFingerprintDecoder* self)
{
#if GST_CHECK_VERSION(1, 0, 0)
  GstSample* sample = 0;
  g_signal_emit_by_name(sink, "pull-sample", &sample);
  if (sample) {
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    gint64 buf_pos = GST_BUFFER_TIMESTAMP(buffer);
    GstMapInfo mapinfo = {0, };
    gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
    emit self->bufferReady(QByteArray(reinterpret_cast<char*>(mapinfo.data),
                                      mapinfo.size));
    gst_buffer_unmap(buffer, &mapinfo);
    gst_sample_unref(sample);
#else
  GstBuffer* buffer = 0;
  g_signal_emit_by_name(sink, "pull-buffer", &buffer);
  if (buffer) {
    gint64 buf_pos = GST_BUFFER_TIMESTAMP(buffer);
    size_t len = GST_BUFFER_SIZE(buffer);
    guint8* data = GST_BUFFER_DATA(buffer);
    emit self->bufferReady(QByteArray(reinterpret_cast<char*>(data), len));
    gst_buffer_unref(buffer);
#endif
    if (self->isStopped()) {
      self->raiseError(FingerprintCalculator::FingerprintCalculationFailed);
    }
    if (buf_pos >= MAX_LENGTH_NS) {
      g_main_loop_quit(self->m_loop);
    }
  }
}

/**
 * Run decoder on audio file.
 * @param filePath path to audio file
 */
void GstFingerprintDecoder::start(const QString& filePath)
{
  AbstractFingerprintDecoder::start(filePath);
  if (!m_loop) {
    // Initialization failed
    m_error = FingerprintCalculator::DecoderError;
    emit error(m_error);
    return;
  }

  m_error = FingerprintCalculator::Ok;
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
  if (m_error == FingerprintCalculator::Ok) {
    emit finished(m_duration);
  } else {
    emit error(m_error);
  }
}

/**
 * Stop decoder.
 * Can be used to stop the decoder when an error is found after
 * getting bufferReady() data.
 */
void GstFingerprintDecoder::stop()
{
  AbstractFingerprintDecoder::stop();
  if (m_loop) {
    g_main_loop_quit(m_loop);
  }
}

#endif
