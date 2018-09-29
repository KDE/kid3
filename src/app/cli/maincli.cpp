/**
 * \file maincli.cpp
 * Main program for command line interface.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QDir>
#include <QTimer>
#include "kid3cli.h"
#include "loadtranslation.h"
#include "standardiohandler.h"
#include "coreplatformtools.h"
#include "kid3application.h"

#if defined Q_OS_WIN32 && defined Q_CC_MINGW
// Disable command line globbing to avoid crash in QCoreApplication::arguments()
// QTBUG-30330
int _CRT_glob = 0;
#endif

/**
 * Main program for command line interface.
 *
 * @param argc number of arguments including command name
 * @param argv arguments, argv[0] is command name
 *
 * @return exit code of application.
 */
int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  app.setApplicationName(QLatin1String("Kid3"));

  Utils::loadTranslation();

#ifdef Q_OS_MAC
  QDir dir(QCoreApplication::applicationDirPath());
  dir.cdUp();
  dir.cd(QLatin1String("PlugIns"));
  QCoreApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

  QStringList args = QCoreApplication::arguments();
  if (args.size() > 1 && args.at(1) == QLatin1String("--portable")) {
    args.removeAt(1);
    qputenv("KID3_CONFIG_FILE",
            QCoreApplication::applicationDirPath().toLatin1() + "/kid3.ini");
  }

  ICorePlatformTools* platformTools = new CorePlatformTools;
  auto kid3App = new Kid3Application(platformTools);
  Kid3Cli kid3cli(kid3App, new StandardIOHandler("kid3-cli> "), args);
  QTimer::singleShot(0, &kid3cli, SLOT(execute()));
  int rc = app.exec();
  delete kid3App;
  delete platformTools;
  return rc;
}
