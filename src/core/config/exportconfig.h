/**
 * \file exportconfig.h
 * Configuration for export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Jun 2013
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

#include <QStringList>
#include "generalconfig.h"
#include "trackdata.h"

/**
 * Export configuration.
 */
class KID3_CORE_EXPORT ExportConfig : public StoredConfig<ExportConfig> {
  Q_OBJECT
  /** Tag1 to export ID3v1 tags, Tag2 for ID3v2 tags */
  Q_PROPERTY(int exportSource READ exportSource WRITE setExportSourceInt NOTIFY exportSourceChanged)
  /** Names of export formats */
  Q_PROPERTY(QStringList exportFormatNames READ exportFormatNames WRITE setExportFormatNames NOTIFY exportFormatNamesChanged)
  /** regexp describing header export format */
  Q_PROPERTY(QStringList exportFormatHeaders READ exportFormatHeaders WRITE setExportFormatHeaders NOTIFY exportFormatHeadersChanged)
  /** regexp describing track export format */
  Q_PROPERTY(QStringList exportFormatTracks READ exportFormatTracks WRITE setExportFormatTracks NOTIFY exportFormatTracksChanged)
  /** regexp describing trailer export format */
  Q_PROPERTY(QStringList exportFormatTrailers READ exportFormatTrailers WRITE setExportFormatTrailers NOTIFY exportFormatTrailersChanged)
  /** selected export format */
  Q_PROPERTY(int exportFormatIndex READ exportFormatIndex WRITE setExportFormatIndex NOTIFY exportFormatIndexChanged)
  /** export window geometry */
  Q_PROPERTY(QByteArray exportWindowGeometry READ exportWindowGeometry WRITE setExportWindowGeometry NOTIFY exportWindowGeometryChanged)

public:
  /**
   * Constructor.
   */
  ExportConfig();

  /**
   * Destructor.
   */
  virtual ~ExportConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config) override;

  /**
   * Get tag source to export.
   * @return Tag1 to export ID3v1 tags, Tag2 for ID3v2 tags.
   */
  Frame::TagVersion exportSource() const { return m_exportSrcV1; }

  /** Set tag source to export. */
  void setExportSource(Frame::TagVersion exportSource);

  /** Get names of export formats */
  QStringList exportFormatNames() const { return m_exportFormatNames; }

  /** Set names of export formats */
  void setExportFormatNames(const QStringList& exportFormatNames);

  /** Get regexp describing header export format. */
  QStringList exportFormatHeaders() const { return m_exportFormatHeaders; }

  /** Set regexp describing header export format. */
  void setExportFormatHeaders(const QStringList& exportFormatHeaders);

  /** Get regexp describing track export format. */
  QStringList exportFormatTracks() const { return m_exportFormatTracks; }

  /** Set regexp describing track export format. */
  void setExportFormatTracks(const QStringList& exportFormatTracks);

  /** Get regexp describing trailer export format. */
  QStringList exportFormatTrailers() const { return m_exportFormatTrailers; }

  /** Set regexp describing trailer export format. */
  void setExportFormatTrailers(const QStringList& exportFormatTrailers);

  /** Get index of selected export format. */
  int exportFormatIndex() const { return m_exportFormatIdx; }

  /** Set index of selected export format. */
  void setExportFormatIndex(int exportFormatIndex);

  /** Get export window geometry. */
  QByteArray exportWindowGeometry() const { return m_exportWindowGeometry; }

  /** Set export window geometry. */
  void setExportWindowGeometry(const QByteArray& exportWindowGeometry);

signals:
  /** Emitted when @a exportSrcV1 changed. */
  void exportSourceChanged(Frame::TagVersion exportSource);

  /** Emitted when @a exportFormatNames changed. */
  void exportFormatNamesChanged(const QStringList& exportFormatNames);

  /** Emitted when @a exportFormatHeaders changed. */
  void exportFormatHeadersChanged(const QStringList& exportFormatHeaders);

  /** Emitted when @a exportFormatTracks changed. */
  void exportFormatTracksChanged(const QStringList& exportFormatTracks);

  /** Emitted when @a exportFormatTrailers changed. */
  void exportFormatTrailersChanged(const QStringList& exportFormatTrailers);

  /** Emitted when @a exportFormatIdx changed. */
  void exportFormatIndexChanged(int exportFormatIndex);

  /** Emitted when @a exportWindowGeometry changed. */
  void exportWindowGeometryChanged(const QByteArray& exportWindowGeometry);

private:
  friend ExportConfig& StoredConfig<ExportConfig>::instance();

  void setExportSourceInt(int exportSrc) {
    setExportSource(Frame::tagVersionCast(exportSrc));
  }

  Frame::TagVersion m_exportSrcV1;
  QStringList m_exportFormatNames;
  QStringList m_exportFormatHeaders;
  QStringList m_exportFormatTracks;
  QStringList m_exportFormatTrailers;
  int m_exportFormatIdx;
  QByteArray m_exportWindowGeometry;

  /** Index in configuration storage */
  static int s_index;
};
