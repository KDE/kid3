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
#if QT_VERSION >= 0x050000
#include <QQmlComponent>
#include <QQmlContext>
#else
#include <QDeclarativeComponent>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#endif
#include "kid3application.h"
#include "coreplatformtools.h"
#include "qmlimageprovider.h"
#include "fileproxymodel.h"
#include "dirproxymodel.h"
#include "genremodel.h"
#include "frametablemodel.h"
#include "framelist.h"
#include "frameeditorobject.h"
#include "frameobjectmodel.h"
#include "taggedfileselection.h"
#include "scriptutils.h"
#include "configobjects.h"
#include "formatconfig.h"
#include "playlistconfig.h"
#include "tagconfig.h"
#include "checkablelistmodel.h"
#include "config.h"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Kid3AppQmlPlugin, Kid3AppQmlPlugin)
Q_DECLARE_METATYPE(QModelIndex)
#endif

Q_DECLARE_METATYPE(Kid3Application*)
Q_DECLARE_METATYPE(QAbstractItemModel*)
Q_DECLARE_METATYPE(FileProxyModel*)
Q_DECLARE_METATYPE(DirProxyModel*)
Q_DECLARE_METATYPE(GenreModel*)
Q_DECLARE_METATYPE(FrameTableModel*)
Q_DECLARE_METATYPE(FrameList*)
Q_DECLARE_METATYPE(FrameEditorObject*)
Q_DECLARE_METATYPE(FrameObjectModel*)
Q_DECLARE_METATYPE(TaggedFileSelection*)
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(Frame::TagVersion)
Q_DECLARE_METATYPE(Frame)
Q_DECLARE_METATYPE(ScriptUtils*)
Q_DECLARE_METATYPE(ConfigObjects*)
Q_DECLARE_METATYPE(CheckableListModel*)
Q_DECLARE_METATYPE(QItemSelectionModel*)

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
QString getPluginsPathFromImportPathList(
#if QT_VERSION >= 0x050000
    QQmlEngine* engine
#else
    QDeclarativeEngine* engine
#endif
    )
{
  QString cfgPluginsDir(QLatin1String(CFG_PLUGINSDIR));
  if (cfgPluginsDir.startsWith(QLatin1String("./"))) {
    cfgPluginsDir.remove(0, 2);
  } else if (cfgPluginsDir.startsWith(QLatin1String("../"))) {
    cfgPluginsDir.remove(0, 3);
  }

  QString pluginsPath;
  foreach (const QString& path, engine->importPathList()) {
    int index = path.indexOf(cfgPluginsDir);
    if (index != -1) {
      pluginsPath = path.left(index + cfgPluginsDir.length());
      break;
    } else if (pluginsPath.isEmpty() &&
               (index = path.indexOf(QLatin1String("plugins"))) != -1) {
      pluginsPath = path.left(index + 7);
      // Probably a path in the build directory, use it if CFG_PLUGINSDIR is
      // not found.
    }
  }
  return pluginsPath;
}

}

/**
 * Constructor.
 * @param parent parent object
 */
Kid3AppQmlPlugin::Kid3AppQmlPlugin(QObject* parent) :
#if QT_VERSION >= 0x050000
  QQmlExtensionPlugin(parent)
#else
  QDeclarativeExtensionPlugin(parent)
#endif
  , m_platformTools(0), m_kid3App(0), m_imageProvider(0)
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
    qRegisterMetaType<QList<QPersistentModelIndex> >();
    qRegisterMetaType<Frame::TagVersion>();
    qRegisterMetaType<Kid3Application*>();
    qRegisterMetaType<QAbstractItemModel*>();
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
    qmlRegisterType<FrameEditorObject>(uri, 1, 0, "FrameEditorObject");
    qmlRegisterUncreatableType<FrameObjectModel>(uri, 1, 0, "FrameObjectModel",
        QLatin1String("Argument of FrameEditorObject.frameEditFinished()"));
    qmlRegisterUncreatableType<TaggedFileSelection>(
          uri, 1, 0, "TaggedFileSelection",
        QLatin1String("Retrieve it using app.selectionInfo"));
    qmlRegisterUncreatableType<QItemSelectionModel>(
          uri, 1, 0, "QItemSelectionModel",
        QLatin1String("Retrieve it using app.fileSelectionModel"));
    qmlRegisterType<ScriptUtils>(uri, 1, 0, "ScriptUtils");
    qmlRegisterType<ConfigObjects>(uri, 1, 0, "ConfigObjects");
    qmlRegisterType<CheckableListModel>(uri, 1, 0, "CheckableListModel");
    qmlRegisterUncreatableType<Frame>(uri, 1, 0, "Frame",
                                      QLatin1String("Only enum container"));
    qmlRegisterUncreatableType<FormatConfig>(
          uri, 1, 0, "FormatConfig", QLatin1String("Only enum container"));
    qmlRegisterUncreatableType<PlaylistConfig>(
          uri, 1, 0, "PlaylistConfig", QLatin1String("Only enum container"));
    qmlRegisterUncreatableType<TagConfig>(
          uri, 1, 0, "TagConfig", QLatin1String("Only enum container"));
#if QT_VERSION < 0x050000
    qRegisterMetaType<QModelIndex>();
#endif
  }
}

/**
 * Initialize the QML engine when the plugin is imported.
 * @param engine QML engine
 * @param uri URI of imported module, must be "Kid3App"
 */
void Kid3AppQmlPlugin::initializeEngine(
#if QT_VERSION >= 0x050000
    QQmlEngine* engine
#else
    QDeclarativeEngine* engine
#endif
    , const char* uri)
{
  if (qstrcmp(uri, "Kid3App") == 0) {
    Kid3Application::setPluginsPathFallback(
          getPluginsPathFromImportPathList(engine));
    m_platformTools = new CorePlatformTools;
    m_kid3App = new Kid3Application(m_platformTools);
    m_imageProvider = new QmlImageProvider(
          m_kid3App->getFileProxyModel()->getIconProvider());
    m_kid3App->setImageProvider(m_imageProvider);
    engine->rootContext()->setContextProperty(QLatin1String("app"), m_kid3App);
    engine->addImageProvider(QLatin1String("kid3"), m_imageProvider);
  }
}
