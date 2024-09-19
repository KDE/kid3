/**
 * \file mainqt.cpp
 * Main program.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QSettings>
#include "fileconfig.h"
#include "loadtranslation.h"
#include "kid3mainwindow.h"
#include "platformtools.h"
#include "kid3application.h"
#include "kid3qtapplication.h"

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
  Q_INIT_RESOURCE(kid3);

#if QT_VERSION < 0x060000
  // Enable support for high resolution "@2x" images
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= 0x050600
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#endif
  Kid3QtApplication app(argc, argv);
  QCoreApplication::setApplicationName(QLatin1String("Kid3"));

#if defined Q_OS_LINUX && QT_VERSION >= 0x050700
  app.setDesktopFileName(QLatin1String("org.kde.kid3-qt"));
#endif

#ifdef Q_OS_MAC
  QDir dir(QApplication::applicationDirPath());
  dir.cdUp();
  dir.cd(QLatin1String("PlugIns"));
  QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

  QStringList args = QApplication::arguments();
  if (args.size() > 1 && args.at(1) == QLatin1String("--portable")) {
    args.removeAt(1);
    qputenv("KID3_CONFIG_FILE",
            QCoreApplication::applicationDirPath().toLatin1() + "/kid3.ini");
  }

  // The Language setting has to be read bypassing the regular
  // configuration object because the language must be set before
  // the application is created.
  QByteArray configPath = qgetenv("KID3_CONFIG_FILE");
  auto configuredLanguage = configPath.isNull()
      ? QSettings(QSettings::UserScope, QLatin1String("Kid3"),
                  QLatin1String("Kid3"))
        .value(QLatin1String("MainWindow/Language")).toString()
      : QSettings(QFile::decodeName(configPath), QSettings::IniFormat)
        .value(QLatin1String("MainWindow/Language")).toString();
  Utils::loadTranslation(configuredLanguage);

  IPlatformTools* platformTools = new PlatformTools;
  auto kid3App = new Kid3Application(platformTools);
#ifdef HAVE_QTDBUS
  kid3App->activateDbusInterface();
#endif
  auto kid3 = new Kid3MainWindow(platformTools, kid3App);
  kid3->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(&app, &Kid3QtApplication::openFileRequested,
                   kid3App, &Kid3Application::openDrop);
  kid3->show();
  if (args.size() > 1) {
    kid3App->openDirectory(args.mid(1));
  } else if ((FileConfig::instance().loadLastOpenedFile() ||
              app.isSessionRestored()) &&
             !FileConfig::instance().lastOpenedFile().isEmpty()) {
    kid3App->openDirectory(QStringList()
                           << FileConfig::instance().lastOpenedFile());
  }
  int rc = QApplication::exec();
  delete kid3App;
  delete platformTools;
  return rc;
}
