/**
 * \file configobjects.h
 * Access to configurations as QObjects.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2014
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

#include <QObject>
#include "kid3api.h"

/**
 * Access to configurations as QObjects.
 */
class KID3_PLUGIN_EXPORT ConfigObjects : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit ConfigObjects(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~ConfigObjects() override = default;

  /** Get batch import configuration. */
  Q_INVOKABLE static QObject* batchImportConfig();

  /** Get filter configuration. */
  Q_INVOKABLE static QObject* filterConfig();

  /** Get file name formate configuration. */
  Q_INVOKABLE static QObject* filenameFormatConfig();

  /** Get tag format configuration. */
  Q_INVOKABLE static QObject* tagFormatConfig();

  /** Get import configuration. */
  Q_INVOKABLE static QObject* importConfig();

  /** Get export configuration. */
  Q_INVOKABLE static QObject* exportConfig();

  /** Get tag configuration. */
  Q_INVOKABLE static QObject* tagConfig();

  /** Get file configuration. */
  Q_INVOKABLE static QObject* fileConfig();

  /** Get rename directory configuration. */
  Q_INVOKABLE static QObject* renDirConfig();

  /** Get number tracks configuration. */
  Q_INVOKABLE static QObject* numberTracksConfig();

  /** Get user actions configuration. */
  Q_INVOKABLE static QObject* userActionsConfig();

  /** Get GUI configuration. */
  Q_INVOKABLE static QObject* guiConfig();

  /** Get network configuration. */
  Q_INVOKABLE static QObject* networkConfig();

  /** Get playlist configuration. */
  Q_INVOKABLE static QObject* playlistConfig();

  /** Get find/replace configuration. */
  Q_INVOKABLE static QObject* findReplaceConfig();

  /** Get main window configuration. */
  Q_INVOKABLE static QObject* mainWindowConfig();
};
