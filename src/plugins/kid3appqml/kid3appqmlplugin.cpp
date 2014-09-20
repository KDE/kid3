/**
 * \file kid3appqmlplugin.cpp
 * QML plugin for Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Sep 2014
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

#include "kid3appqmlplugin.h"
#include <QCoreApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include "kid3qmlapplication.h"
#include "coreplatformtools.h"
#include "qmlimageprovider.h"
#include "fileproxymodel.h"
#include "dirproxymodel.h"
#include "genremodel.h"
#include "frametablemodel.h"
#include "framelist.h"
#include "frameobjectmodel.h"
#include "taggedfileselection.h"
#include "config.h"

Q_DECLARE_METATYPE(FileProxyModel*)
Q_DECLARE_METATYPE(DirProxyModel*)
Q_DECLARE_METATYPE(GenreModel*)
Q_DECLARE_METATYPE(FrameTableModel*)
Q_DECLARE_METATYPE(FrameList*)
Q_DECLARE_METATYPE(FrameObjectModel*)
Q_DECLARE_METATYPE(TaggedFileSelection*)
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(TrackData::TagVersion)

namespace {

/**
 * Get the plugins path from the QML import path.
 *
 * The plugins path is normally found relative to the directory where the
 * application binary is located. However, when the application is started
 * from a QML script using qmlscene, the path plugins path cannot be found
 * from the location of qmlscene. In this case, the plugins path can be
 * derived from the QML import path, which contains the Kid3 QML plugin.
 *
 * @param engine QML engine
 * @return plugins directory path, empty if not found.
 */
QString getPluginsPathFromImportPathList(QQmlEngine* engine)
{
  QString cfgPluginsDir(QLatin1String(CFG_PLUGINSDIR));
  if (cfgPluginsDir.startsWith(QLatin1String("./"))) {
    cfgPluginsDir.remove(0, 2);
  } else if (cfgPluginsDir.startsWith(QLatin1String("../"))) {
    cfgPluginsDir.remove(0, 3);
  }

  foreach (const QString& path, engine->importPathList()) {
    int index = path.indexOf(cfgPluginsDir);
    if (index != -1) {
      return path.left(index + cfgPluginsDir.length());
    }
  }
  return QString();
}

}

/**
 * Constructor.
 * @param parent parent object
 */
Kid3AppQmlPlugin::Kid3AppQmlPlugin(QObject* parent) :
  QQmlExtensionPlugin(parent),
  m_platformTools(0), m_kid3App(0), m_imageProvider(0)
{
}

/**
 * Destructor.
 */
Kid3AppQmlPlugin::~Kid3AppQmlPlugin()
{
  delete m_kid3App;
  delete m_imageProvider;
  delete m_platformTools;
}

/**
 * Register the types used by the QML plugin.
 * @param uri URI of imported module, must be "Kid3App"
 */
void Kid3AppQmlPlugin::registerTypes(const char *uri)
{
  if (qstrcmp(uri, "Kid3App") == 0) {
    qRegisterMetaType<Kid3QmlApplication*>();
    // @uri Kid3App
    qmlRegisterUncreatableType<FileProxyModel>(uri, 1, 0, "FileProxyModel",
        QLatin1String("Retrieve it using app.fileProxyModel"));
    qmlRegisterUncreatableType<DirProxyModel>(uri, 1, 0, "DirProxyModel",
        QLatin1String("Retrieve it using app.dirProxyModel"));
    qmlRegisterUncreatableType<GenreModel>(uri, 1, 0, "GenreModel",
        QLatin1String("Retrieve it using app.genreModelV1 or app.genreModelV2"));
    qmlRegisterUncreatableType<FrameTableModel>(uri, 1, 0, "FrameTableModel",
        QLatin1String("Retrieve it using app.frameModelV1 or app.frameModelV2"));
    qmlRegisterUncreatableType<FrameList>(uri, 1, 0, "FrameList",
        QLatin1String("Retrieve it using app.frameList"));
    qmlRegisterUncreatableType<FrameObjectModel>(uri, 1, 0, "FrameObjectModel",
        QLatin1String("Argument of app.frameEditFinished()"));
    qmlRegisterUncreatableType<TaggedFileSelection>(uri, 1, 0, "TaggedFileSelection",
        QLatin1String("Retrieve it using app.selectionInfo"));
    qRegisterMetaType<QList<QPersistentModelIndex> >();
    qRegisterMetaType<TrackData::TagVersion>();
  }
}

/**
 * Initialize the QML engine when the plugin is imported.
 * @param engine QML engine
 * @param uri URI of imported module, must be "Kid3App"
 */
void Kid3AppQmlPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
  if (qstrcmp(uri, "Kid3App") == 0) {
    Kid3Application::setPluginsPathFallback(
          getPluginsPathFromImportPathList(engine));
    m_platformTools = new CorePlatformTools;
    m_kid3App = new Kid3QmlApplication(m_platformTools);
    m_imageProvider = new QmlImageProvider(
          m_kid3App->getFileProxyModel()->getIconProvider());
    m_kid3App->setImageProvider(m_imageProvider);
    engine->rootContext()->setContextProperty(QLatin1String("app"), m_kid3App);
    engine->addImageProvider(QLatin1String("kid3"), m_imageProvider);
  }
}
