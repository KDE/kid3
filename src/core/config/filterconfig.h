/**
 * \file filterconfig.h
 * Configuration for filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
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

#ifndef FILTERCONFIG_H
#define FILTERCONFIG_H

#include <QStringList>
#include "config.h"
#include "generalconfig.h"
#include "kid3api.h"

/**
 * Filter configuration.
 */
class KID3_CORE_EXPORT FilterConfig : public StoredConfig<FilterConfig> {
  Q_OBJECT
  /** Names of filter expressions */
  Q_PROPERTY(QStringList filterNames READ filterNames WRITE setFilterNames NOTIFY filterNamesChanged)
  /** Filter expressions */
  Q_PROPERTY(QStringList filterExpressions READ filterExpressions WRITE setFilterExpressions NOTIFY filterExpressionsChanged)
  /** Selected filter */
  Q_PROPERTY(int filterIndex READ filterIndex WRITE setFilterIndex NOTIFY filterIndexChanged)
  /** Window geometry */
  Q_PROPERTY(QByteArray windowGeometry READ windowGeometry WRITE setWindowGeometry NOTIFY windowGeometryChanged)

public:
  /**
   * Constructor.
   */
  FilterConfig();

  /**
   * Destructor.
   */
  virtual ~FilterConfig() override;

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
   * Set the filename format in the "Filename Tag Mismatch" filter.
   *
   * @param format filename format
   */
  void setFilenameFormat(const QString& format);

  /** Get names of filter expressions. */
  QStringList filterNames() const { return m_filterNames; }

  /** Set names of filter expressions. */
  void setFilterNames(const QStringList& filterNames);

  /** Get filter expressions. */
  QStringList filterExpressions() const { return m_filterExpressions; }

  /** Set filter expressions. */
  void setFilterExpressions(const QStringList& filterExpressions);

  /** Get index of selected filter. */
  int filterIndex() const { return m_filterIdx; }

  /** Set index of selected filter. */
  void setFilterIndex(int filterIndex);

  /** Get window geometry. */
  QByteArray windowGeometry() const { return m_windowGeometry; }

  /** Set window geometry. */
  void setWindowGeometry(const QByteArray& windowGeometry);

signals:
  /** Emitted when @a filterNames changed. */
  void filterNamesChanged(const QStringList& filterNames);

  /** Emitted when @a filterExpressions changed. */
  void filterExpressionsChanged(const QStringList& filterExpressions);

  /** Emitted when @a filterIdx changed. */
  void filterIndexChanged(int filterIndex);

  /** Emitted when @a windowGeometry changed. */
  void windowGeometryChanged(const QByteArray& windowGeometry);

private:
  friend FilterConfig& StoredConfig<FilterConfig>::instance();

  QStringList m_filterNames;
  QStringList m_filterExpressions;
  int m_filterIdx;
  QByteArray m_windowGeometry;

  /** Index in configuration storage */
  static int s_index;
};

#endif
