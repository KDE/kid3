/**
 * \file findreplaceconfig.h
 * Configuration for find/replace dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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
#include "config.h"
#include "generalconfig.h"
#include "tagsearcher.h"
#include "kid3api.h"

class BatchImportProfile;

/**
 * Filter configuration.
 */
class KID3_CORE_EXPORT FindReplaceConfig : public StoredConfig<FindReplaceConfig> {
  Q_OBJECT
  /** search parameters */
  Q_PROPERTY(QVariantList parameterList READ parameterList WRITE setParameterList NOTIFY parameterListChanged)
  /** window geometry */
  Q_PROPERTY(QByteArray windowGeometry READ windowGeometry WRITE setWindowGeometry NOTIFY windowGeometryChanged)

public:
  /**
   * Constructor.
   */
  FindReplaceConfig();

  /**
   * Destructor.
   */
  virtual ~FindReplaceConfig() override = default;

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
   * Get search parameters.
   * @return search parameters.
   */
  const TagSearcher::Parameters& getParameters() const { return m_params; }

  /**
   * Set search parameters.
   * @param params search parameters
   */
  void setParameters(const TagSearcher::Parameters& params) {
    m_params = params;
  }

  /**
   * Get search parameters as variant list.
   * @return variant list containing search text, replace text, flags,
   * frameMask.
   */
  QVariantList parameterList() const { return m_params.toVariantList(); }

  /**
   * Set search parameters from variant list.
   * @param lst variant list containing search text, replace text, flags,
   * frameMask.
   */
  void setParameterList(const QVariantList& lst);

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
  /** Emitted when the parameters are changed using setParameterList(). */
  void parameterListChanged();

  /** Emitted when @a windowGeometry changed. */
  void windowGeometryChanged(const QByteArray& windowGeometry);

private:
  friend FindReplaceConfig& StoredConfig<FindReplaceConfig>::instance();

  TagSearcher::Parameters m_params;
  QByteArray m_windowGeometry;

  /** Index in configuration storage */
  static int s_index;
};
