/**
 * \file numbertracksconfig.h
 * Configuration for track numbering.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "generalconfig.h"
#include "frame.h"
#include "kid3api.h"

/**
 * Configuration for track numbering.
 */
class KID3_CORE_EXPORT NumberTracksConfig
    : public StoredConfig<NumberTracksConfig> {
  Q_OBJECT
  /** number tracks in tags 1, tags 2, or both */
  Q_PROPERTY(int numberTracksDestination READ numberTracksDestination
             WRITE setNumberTracksDstInt NOTIFY numberTracksDestinationChanged)
  /** number tracks start number */
  Q_PROPERTY(int numberTracksStart READ numberTracksStart
             WRITE setNumberTracksStart NOTIFY numberTracksStartChanged)
  /** enable track numbering */
  Q_PROPERTY(bool trackNumberingEnabled READ isTrackNumberingEnabled
             WRITE setTrackNumberingEnabled NOTIFY trackNumberingEnabledChanged)
  /** reset of counter for each directory */
  Q_PROPERTY(bool directoryCounterResetEnabled READ isDirectoryCounterResetEnabled
             WRITE setDirectoryCounterResetEnabled NOTIFY directoryCounterResetEnabledChanged)
  /** window geometry */
  Q_PROPERTY(QByteArray windowGeometry READ windowGeometry
             WRITE setWindowGeometry NOTIFY windowGeometryChanged)
public:
  /**
   * Constructor.
   */
  NumberTracksConfig();

  /**
   * Destructor.
   */
  ~NumberTracksConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  void readFromConfig(ISettings* config) override;

  /** Get destination tag. */
  Frame::TagVersion numberTracksDestination() const { return m_numberTracksDst; }

  /** Set destination tag. */
  void setNumberTracksDestination(Frame::TagVersion numberTracksDestination);

  /** Get start number when numbering tracks. */
  int numberTracksStart() const { return m_numberTracksStart; }

  /** Set start number when numbering tracks. */
  void setNumberTracksStart(int numberTracksStart);

  /**
   * Check if track numbering is enabled.
   * @return true if enabled.
   */
  bool isTrackNumberingEnabled() const { return m_trackNumberingEnabled; }

  /**
   * Enable or disable track numbering.
   * @param enable true to enable
   */
  void setTrackNumberingEnabled(bool enable);

  /**
   * Check if counter has to be reset for each directory.
   * @return true if enabled.
   */
  bool isDirectoryCounterResetEnabled() const {
    return m_directoryCounterResetEnabled;
  }

  /**
   * Enable reset of counter for each directory.
   * @param enable true to enable
   */
  void setDirectoryCounterResetEnabled(bool enable);

  /**
   * Get window geometry.
   * @return window geometry.
   */
  QByteArray windowGeometry() const { return m_windowGeometry; }

  /**
   * Set window geometry.
   * @param windowGeometry geometry
   */
  void setWindowGeometry(const QByteArray& windowGeometry);

signals:
  /** Emitted when @a numberTracksDst changed. */
  void numberTracksDestinationChanged(Frame::TagVersion numberTracksDestination);

  /** Emitted when @a numberTracksStart changed. */
  void numberTracksStartChanged(int numberTracksStart);

  /** Emitted when @a trackNumberingEnabled changed. */
  void trackNumberingEnabledChanged(bool trackNumberingEnabled);

  /** Emitted when @a directoryCounterResetEnabled changed. */
  void directoryCounterResetEnabledChanged(bool directoryCounterResetEnabled);

  /** Emitted when @a windowGeometry changed. */
  void windowGeometryChanged(const QByteArray& windowGeometry);

private:
  friend NumberTracksConfig& StoredConfig<NumberTracksConfig>::instance();

  void setNumberTracksDstInt(int numberTracksDst) {
    setNumberTracksDestination(Frame::tagVersionCast(numberTracksDst));
  }

  QByteArray m_windowGeometry;
  Frame::TagVersion m_numberTracksDst;
  int m_numberTracksStart;
  bool m_trackNumberingEnabled;
  bool m_directoryCounterResetEnabled;

  /** Index in configuration storage */
  static int s_index;
};
