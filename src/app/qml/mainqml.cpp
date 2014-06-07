/**
 * \file mainqt.cpp
 * Main program.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#define QT_QML_DEBUG
#include <QtQuick>
#include <QFile>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <typeinfo>
#include "fileconfig.h"
#include "loadtranslation.h"
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

Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(FileProxyModel*)
Q_DECLARE_METATYPE(DirProxyModel*)
Q_DECLARE_METATYPE(GenreModel*)
Q_DECLARE_METATYPE(FrameTableModel*)
Q_DECLARE_METATYPE(FrameList*)
Q_DECLARE_METATYPE(FrameObjectModel*)
Q_DECLARE_METATYPE(TaggedFileSelection*)
Q_DECLARE_METATYPE(TrackData::TagVersion)

/**
 * QApplication subclass with adapted session management.
 */
class Kid3QtApplication : public QApplication {
public:
  /**
   * Constructor.
   * @param argc number of arguments (including command)
   * @param argv arguments
   */
  Kid3QtApplication(int& argc, char** argv) : QApplication(argc, argv) {}

  /**
   * Destructor.
   */
  virtual ~Kid3QtApplication();

  /**
   * Called when session manager wants application to commit all its data.
   *
   * This method is reimplemented to avoid closing all top level widgets and
   * make restoring with the KDE window manager working.
   *
   * @param manager session manager
   */
  virtual void commitData(QSessionManager& manager) {
    emit commitDataRequest(manager);
  }

  /**
   * Send event to receiver.
   * @param receiver receiver
   * @param event event
   * @return return value from receiver's event handler.
   */
  virtual bool notify(QObject* receiver, QEvent* event);
};

/**
 * Destructor.
 */
Kid3QtApplication::~Kid3QtApplication()
{
}

/**
 * Send event to receiver.
 * @param receiver receiver
 * @param event event
 * @return return value from receiver's event handler.
 */
bool Kid3QtApplication::notify(QObject* receiver, QEvent* event)
{
  try {
    return QApplication::notify(receiver, event);
  } catch (std::exception& ex) {
    qWarning("Exception %s (%s) was caught", typeid(ex).name(), ex.what());
  }
  return false;
}


/**
 * Main program.
 *
 * @param argc number of arguments including command name
 * @param argv arguments, argv[0] is command name
 *
 * @return exit code of application.
 */

int main(int argc, char* argv[])
{
  Kid3QtApplication app(argc, argv);
  app.setApplicationName(QLatin1String("Kid3"));

  Utils::loadTranslation();

#ifdef Q_OS_MAC
  QDir dir(QApplication::applicationDirPath());
  dir.cdUp();
  dir.cd(QLatin1String("PlugIns"));
  QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

  QStringList qmlDirs;
#ifdef CFG_QMLSRCDIR
  qmlDirs.append(QLatin1String(CFG_QMLSRCDIR));
#endif
#ifdef CFG_QMLDIR
  qmlDirs.append(QLatin1String(CFG_QMLDIR));
#endif
  QString mainQmlPath;
  foreach (const QString& qmlDir, qmlDirs) {
    QString qmlPath(qmlDir);
    Utils::prependApplicationDirPathIfRelative(qmlPath);
    qmlPath += QDir::separator();
    qmlPath += QLatin1String("main.qml");
    if (QFile::exists(qmlPath)) {
      mainQmlPath = qmlPath;
      break;
    }
  }
  if (mainQmlPath.isEmpty()) {
    qWarning("Could not find main.qml in the following paths:\n%s",
             qPrintable(qmlDirs.join(QLatin1Char('\n'))));
    return 1;
  }

  qRegisterMetaType<Kid3QmlApplication*>();
  qRegisterMetaType<QList<QPersistentModelIndex> >();
  qRegisterMetaType<FileProxyModel*>();
  qRegisterMetaType<DirProxyModel*>();
  qRegisterMetaType<GenreModel*>();
  qRegisterMetaType<FrameTableModel*>();
  qRegisterMetaType<FrameList*>();
  qRegisterMetaType<FrameObjectModel*>();
  qRegisterMetaType<TaggedFileSelection*>();
  qRegisterMetaType<TrackData::TagVersion>();
  ICorePlatformTools* platformTools = new CorePlatformTools;
  Kid3QmlApplication* kid3App = new Kid3QmlApplication(platformTools);
  QmlImageProvider* imageProvider = new QmlImageProvider(
        kid3App->getFileProxyModel()->getIconProvider());
  kid3App->setImageProvider(imageProvider);
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty(QLatin1String("app"), kid3App);
  engine.addImageProvider(QLatin1String("kid3"), imageProvider);
  engine.load(mainQmlPath);
  return app.exec();
  // Deleting kid3App before exit would upset the QML engine.
}
