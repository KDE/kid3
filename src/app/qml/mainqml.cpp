/**
 * \file mainqml.cpp
 * Main program.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jun 2014
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

#include <QFile>
#include <QApplication>
#include <QTranslator>
#include <QDir>
#if QT_VERSION >= 0x050000
#define QT_QML_DEBUG
#include <QQuickView>
#include <QQmlApplicationEngine>
#else
#define QT_DECLARATIVE_DEBUG
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#endif
#include <typeinfo>
#include "config.h"
#include "loadtranslation.h"
#include "kid3application.h"

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
             qPrintable(qmlDirs.join(QLatin1String("\n"))));
    return 1;
  }

#if QT_VERSION >= 0x050000
  // To load a qml file containing an Item for a QQuickView.
  QQuickView view;
  QDir pluginsDir;
  if (Kid3Application::findPluginsDirectory(pluginsDir) &&
      pluginsDir.cd(QLatin1String("imports"))) {
    view.engine()->addImportPath(pluginsDir.absolutePath());
  }
  view.setSource(QUrl::fromLocalFile(mainQmlPath));
  view.setResizeMode(QQuickView::SizeRootObjectToView);
  QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
  view.show();
#else
  QDeclarativeView view;
  QDir pluginsDir;
  if (Kid3Application::findPluginsDirectory(pluginsDir) &&
      pluginsDir.cd(QLatin1String("imports"))) {
    view.engine()->addImportPath(pluginsDir.absolutePath());
  }
  view.setSource(QUrl::fromLocalFile(mainQmlPath));
  view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
  QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));
  view.show();
#endif
  return app.exec();
}
