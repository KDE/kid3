/**
 * \file kid3qmlplugin.h
 * QML plugin for Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Sep 2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#include <QtGlobal>
#include <QScopedPointer>
#include "kid3api.h"

class ICorePlatformTools;
class Kid3Application;
class QmlImageProvider;

#include <QQmlExtensionPlugin>

/**
 * QML plugin for Kid3 application.
 */
class KID3_PLUGIN_EXPORT Kid3QmlPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit Kid3QmlPlugin(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~Kid3QmlPlugin() override;

  /**
   * Register the types used by the QML plugin.
   * @param uri URI of imported module, must be "Kid3"
   */
  void registerTypes(const char* uri) override;

  /**
   * Initialize the QML engine when the plugin is imported.
   * @param engine QML engine
   * @param uri URI of imported module, must be "Kid3"
   */
  void initializeEngine(QQmlEngine* engine, const char* uri) override;

private:
  ICorePlatformTools* m_platformTools;
  Kid3Application* m_kid3App;
  QmlImageProvider* m_imageProvider;
  bool m_ownsKid3App;
};
