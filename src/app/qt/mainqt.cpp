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

#include <QFile>
#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QDir>
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

#if QT_VERSION >= 0x050100
  // Enable support for high resolution "@2x" images
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= 0x050600
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  Kid3QtApplication app(argc, argv);
  app.setApplicationName(QLatin1String("Kid3"));

  Utils::loadTranslation();

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

  IPlatformTools* platformTools = new PlatformTools;
  Kid3Application* kid3App = new Kid3Application(platformTools);
#ifdef HAVE_QTDBUS
  kid3App->activateDbusInterface();
#endif
  Kid3MainWindow* kid3 = new Kid3MainWindow(platformTools, kid3App);
  kid3->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(&app, SIGNAL(openFileRequested(QStringList)), kid3App, SLOT(openDrop(QStringList)));
  kid3->show();
  if (args.size() > 1) {
    kid3->confirmedOpenDirectory(args.mid(1));
  } else if ((FileConfig::instance().loadLastOpenedFile() ||
              app.isSessionRestored()) &&
             !FileConfig::instance().lastOpenedFile().isEmpty()) {
    kid3->confirmedOpenDirectory(QStringList()
                                 << FileConfig::instance().lastOpenedFile());
  }
  int rc = app.exec();
  delete kid3App;
  delete platformTools;
  return rc;
}
