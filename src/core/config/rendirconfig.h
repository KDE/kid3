/**
 * \file rendirconfig.h
 * Configuration for directory renaming.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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
#include "trackdata.h"
#include "kid3api.h"

/**
 * Configuration for directory renaming.
 */
class KID3_CORE_EXPORT RenDirConfig : public StoredConfig<RenDirConfig> {
  Q_OBJECT
  /** directory name format */
  Q_PROPERTY(QString dirFormat READ dirFormat WRITE setDirFormat
             NOTIFY dirFormatChanged)
  /** available directory name formats */
  Q_PROPERTY(QStringList dirFormats READ dirFormats WRITE setDirFormats
             NOTIFY dirFormatsChanged)
  /** rename directory from tags 1, tags 2, or both */
  Q_PROPERTY(int renDirSource READ renDirSource WRITE setRenDirSrcInt
             NOTIFY renDirSourceChanged)
  /** window geometry */
  Q_PROPERTY(QByteArray windowGeometry READ windowGeometry
             WRITE setWindowGeometry NOTIFY windowGeometryChanged)

public:
  /**
   * Constructor.
   */
  RenDirConfig();

  /**
   * Destructor.
   */
  ~RenDirConfig() override = default;

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

  /** Get directory name format. */
  QString dirFormat() const { return m_dirFormatText; }

  /** Set directory name format. */
  void setDirFormat(const QString& dirFormat);

  /** Get available directory name formats. */
  QStringList dirFormats() const { return m_dirFormatItems; }

  /** Set available directory name formats. */
  void setDirFormats(const QStringList& dirFormatItems);

  /** Get tag source when renaming directory. */
  Frame::TagVersion renDirSource() const { return m_renDirSrc; }

  /** Set tag source when renaming directory. */
  void setRenDirSource(Frame::TagVersion renDirSource);

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
  /** Emitted when @a dirFormatText changed. */
  void dirFormatChanged(const QString& dirFormat);

  /** Emitted when @a dirFormats changed. */
  void dirFormatsChanged(const QStringList& dirFormats);

  /** Emitted when @a renDirSrc changed. */
  void renDirSourceChanged(Frame::TagVersion renDirSource);

  /** Emitted when @a windowGeometry changed. */
  void windowGeometryChanged(const QByteArray& windowGeometry);

private:
  friend RenDirConfig& StoredConfig<RenDirConfig>::instance();

  void setRenDirSrcInt(int renDirSrc) {
    setRenDirSource(Frame::tagVersionCast(renDirSrc));
  }

  QString m_dirFormatText;
  QStringList m_dirFormatItems;
  Frame::TagVersion m_renDirSrc;
  QByteArray m_windowGeometry;

  static const char** s_defaultDirFmtList;

  /** Index in configuration storage */
  static int s_index;
};
