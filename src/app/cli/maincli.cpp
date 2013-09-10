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

#include <QApplication>
#include <QLibraryInfo>
#include <QDir>
#include <QTimer>
#include "kid3cli.h"
#include "loadtranslation.h"

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
  QApplication app(argc, argv);
  app.setApplicationName(QLatin1String("Kid3"));

  Utils::loadTranslation();

#ifdef Q_OS_MAC
 QDir dir(QApplication::applicationDirPath());
 dir.cdUp();
 dir.cd(QLatin1String("PlugIns"));
 QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

 Kid3Cli kid3cli;
 QTimer::singleShot(0, &kid3cli, SLOT(execute()));
 return app.exec();
}
