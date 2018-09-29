/**
 * \file batchimportconfig.h
 * Configuration for batch import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2013
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

#ifndef BATCHIMPORTCONFIG_H
#define BATCHIMPORTCONFIG_H

#include <QStringList>
#include "config.h"
#include "generalconfig.h"
#include "trackdata.h"
#include "kid3api.h"

class BatchImportProfile;

/**
 * Filter configuration.
 */
class KID3_CORE_EXPORT BatchImportConfig : public StoredConfig<BatchImportConfig> {
  Q_OBJECT
  /** tag version to import */
  Q_PROPERTY(int importDest READ importDest WRITE setImportDestInt NOTIFY importDestChanged)
  /** Names of profiles */
  Q_PROPERTY(QStringList profileNames READ profileNames WRITE setProfileNames NOTIFY profileNamesChanged)
  /** Profile import sources */
  Q_PROPERTY(QStringList profileSources READ profileSources WRITE setProfileSources NOTIFY profileSourcesChanged)
  /** Selected profile */
  Q_PROPERTY(int profileIndex READ profileIndex WRITE setProfileIndex NOTIFY profileIndexChanged)
  /** Window geometry */
  Q_PROPERTY(QByteArray windowGeometry READ windowGeometry WRITE setWindowGeometry NOTIFY windowGeometryChanged)

public:
  /**
   * Constructor.
   */
  BatchImportConfig();

  /**
   * Destructor.
   */
  virtual ~BatchImportConfig() override;

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(ISettings* config) override;

  /**
   * Get a batch import profile.
   *
   * @param name name of profile
   * @param profile the profile will be returned here
   * @return true if profile with @a name found.
   */
  bool getProfileByName(const QString& name, BatchImportProfile& profile) const;

  /** Get tag version to import. */
  Frame::TagVersion importDest() const { return m_importDest; }

  /** Set tag version to import. */
  void setImportDest(Frame::TagVersion importDest);

  /** Get names of profiles. */
  QStringList profileNames() const { return m_profileNames; }

  /** Set names of profiles. */
  void setProfileNames(const QStringList& profileNames);

  /** Get profile import sources. */
  QStringList profileSources() const { return m_profileSources; }

  /** Set profile import sources. */
  void setProfileSources(const QStringList& profileSources);

  /** Get index of selected profile. */
  int profileIndex() const { return m_profileIdx; }

  /** Set index of selected profile. */
  void setProfileIndex(int profileIdx);

  /** Get window geometry. */
  QByteArray windowGeometry() const { return m_windowGeometry; }

  /** Set window geometry. */
  void setWindowGeometry(const QByteArray& windowGeometry);

signals:
  /** Emitted when @a importDest changed. */
  void importDestChanged(Frame::TagVersion importDest);

  /** Emitted when @a profileNames changed. */
  void profileNamesChanged(const QStringList& profileNames);

  /** Emitted when @a profileSources changed. */
  void profileSourcesChanged(const QStringList& profileSources);

  /** Emitted when @a profileIdx changed. */
  void profileIndexChanged(int profileIdx);

  /** Emitted when @a windowGeometry changed. */
  void windowGeometryChanged(const QByteArray& windowGeometry);

private:
  friend BatchImportConfig& StoredConfig<BatchImportConfig>::instance();

  void setImportDestInt(int importDest) {
    setImportDest(Frame::tagVersionCast(importDest));
  }

  Frame::TagVersion m_importDest;
  QStringList m_profileNames;
  QStringList m_profileSources;
  int m_profileIdx;
  QByteArray m_windowGeometry;

  /** Index in configuration storage */
  static int s_index;
};

#endif
