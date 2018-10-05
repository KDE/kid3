/**
 * \file gstfingerprintdecoder.h
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

#pragma once

#include <gst/gst.h>
#include "abstractfingerprintdecoder.h"
#include "fingerprintcalculator.h"

/**
 * Chromaprint fingerprint decoder using GStreamer.
 */
class GstFingerprintDecoder : public AbstractFingerprintDecoder {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit GstFingerprintDecoder(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~GstFingerprintDecoder();

  /**
   * Run decoder on audio file.
   * @param filePath path to audio file
   */
  virtual void start(const QString& filePath);

  /**
   * Stop decoder.
   * Can be used to stop the decoder when an error is found after
   * getting bufferReady() data.
   */
  virtual void stop();

private:
  static const int BUFFER_SIZE = 10;
  static const gint64 MAX_LENGTH_NS = 120000000000;
  static const guint TIMEOUT_MS = 5000;

  void raiseError(FingerprintCalculator::Error error);
  static gboolean cb_timeout(gpointer data);
  static void cb_message(GstBus* bus, GstMessage* message, GstFingerprintDecoder* self);
  static void cb_pad_added(GstElement* dec, GstPad* pad, GstFingerprintDecoder* self);
  static void cb_no_more_pads(GstElement* dec, GstFingerprintDecoder* self);
  static void cb_notify_caps(GstPad *pad, GParamSpec* spec, GstFingerprintDecoder* self);
  static void cb_unknown_type(GstElement* dec, GstPad* pad, GstCaps* caps, GstFingerprintDecoder* self);
  static void cb_new_buffer(GstElement* sink, GstFingerprintDecoder* self);

  GMainLoop* m_loop;
  GstElement* m_pipeline;
  GstElement* m_dec;
  GstElement* m_conv;
  FingerprintCalculator::Error m_error;
  int m_duration;
  gint m_channels;
  gint m_rate;
  bool m_gotPad;
};
