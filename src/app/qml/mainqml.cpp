/**
 * \file mainqml.cpp
 * Main program.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 7 Jun 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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
#ifndef NDEBUG
#define QT_QML_DEBUG
#endif
#include <QQmlApplicationEngine>
#include <QQuickStyle>
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
  virtual ~Kid3QtApplication() override = default;

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
  virtual bool notify(QObject* receiver, QEvent* event) override;
};

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
#ifdef HAVE_QMLDIR_IN_QRC
  Q_INIT_RESOURCE(qmlapp);
#endif
#ifdef HAVE_TRANSLATIONSDIR_IN_QRC
  Q_INIT_RESOURCE(translations);
#endif

  QCoreApplication::setApplicationName(QLatin1String("Kid3"));
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  Kid3QtApplication app(argc, argv);
#ifdef Q_OS_ANDROID
  QQuickStyle::setStyle(QLatin1String("Material"));
#else
  QString style = QQuickStyle::name();
  if (style == QLatin1String("org.kde.desktop")) {
    // The default KDE style currently fails with
    // file:///usr/lib/qt/qml/QtQuick/Controls.2/org.kde.desktop/Dialog.qml:80
    // Cannot assign to non-existent property "buttonBox"
    style = QLatin1String("Default");
  }
  if (!style.isEmpty()) {
    QQuickStyle::setStyle(style);
  }
#endif

  Utils::loadTranslation();
#ifdef Q_OS_MAC
  QDir dir(QCoreApplication::applicationDirPath());
  dir.cdUp();
  dir.cd(QLatin1String("PlugIns"));
  QCoreApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

  QStringList qmlDirs;
#if !defined NDEBUG && defined CFG_QMLSRCDIR
  qmlDirs.append(QLatin1String(CFG_QMLSRCDIR));
#endif
#ifdef CFG_QMLDIR
  qmlDirs.append(QLatin1String(CFG_QMLDIR));
#endif
  QString mainQmlPath;
  const auto constQmlDirs = qmlDirs;
  for (const QString& qmlDir : constQmlDirs) {
    QString qmlPath(qmlDir);
    Utils::prependApplicationDirPathIfRelative(qmlPath);
    qmlPath += QDir::separator();
    qmlPath += QLatin1String("app");
    qmlPath += QDir::separator();
    qmlPath += QLatin1String("Main.qml");
    if (QFile::exists(qmlPath)) {
      mainQmlPath = qmlPath;
      break;
    }
  }
  if (mainQmlPath.isEmpty()) {
    qWarning("Could not find app/Main.qml in the following paths:\n%s",
             qPrintable(qmlDirs.join(QLatin1String("\n"))));
    return 1;
  }

  QQmlApplicationEngine engine;
#ifdef HAVE_QMLDIR_IN_QRC
  engine.addImportPath(QLatin1String(CFG_QMLDIR "imports"));
  QDir pluginsDir;
  if (Kid3Application::findPluginsDirectory(pluginsDir) &&
      pluginsDir.cd(QLatin1String("imports/Kid3"))) {
    engine.addPluginPath(pluginsDir.absolutePath());
  }
  engine.load(QUrl(QLatin1String("qrc:///app/Main.qml")));
#else
  QDir pluginsDir;
  if (Kid3Application::findPluginsDirectory(pluginsDir) &&
      pluginsDir.cd(QLatin1String("imports"))) {
    engine.addImportPath(pluginsDir.absolutePath());
  }
  engine.load(QUrl::fromLocalFile(mainQmlPath));
#endif
  return QApplication::exec();
}
