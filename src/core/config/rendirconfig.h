/**
 * \file rendirconfig.h
 * Configuration for directory renaming.
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
  /** index of directory name format selected */
  Q_PROPERTY(int dirFormatIndex READ dirFormatIndex WRITE setDirFormatIndex
             NOTIFY dirFormatIndexChanged)
  /** rename directory from tags 1, tags 2, or both */
  Q_PROPERTY(int renDirSource READ renDirSource WRITE setRenDirSrcInt
             NOTIFY renDirSourceChanged)

public:
  /**
   * Constructor.
   */
  RenDirConfig();

  /**
   * Destructor.
   */
  virtual ~RenDirConfig() override = default;

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

  /** Get directory name format. */
  QString dirFormat() const { return m_dirFormatText; }

  /** Set directory name format. */
  void setDirFormat(const QString& dirFormat);

  /** Get index of directory name format selected. */
  int dirFormatIndex() const { return m_dirFormatItem; }

  /** Set index of directory name format selected. */
  void setDirFormatIndex(int dirFormatIndex);

  /** Get tag source when renaming directory. */
  Frame::TagVersion renDirSource() const { return m_renDirSrc; }

  /** Set tag source when renaming directory. */
  void setRenDirSource(Frame::TagVersion renDirSource);

  /** Get default directory format list. */
  Q_INVOKABLE static QStringList getDefaultDirFormatList();

signals:
  /** Emitted when @a dirFormatText changed. */
  void dirFormatChanged(const QString& dirFormat);

  /** Emitted when @a dirFormatItem changed. */
  void dirFormatIndexChanged(int dirFormatIndex);

  /** Emitted when @a renDirSrc changed. */
  void renDirSourceChanged(Frame::TagVersion renDirSource);

private:
  friend RenDirConfig& StoredConfig<RenDirConfig>::instance();

  void setRenDirSrcInt(int renDirSrc) {
    setRenDirSource(Frame::tagVersionCast(renDirSrc));
  }

  QString m_dirFormatText;
  int m_dirFormatItem;
  Frame::TagVersion m_renDirSrc;

  static const char** s_defaultDirFmtList;

  /** Index in configuration storage */
  static int s_index;
};
