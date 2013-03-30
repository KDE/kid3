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
#include "configstore.h"
#include "loadtranslation.h"
#include "platformtools.h"
#include "kid3mainwindow.h"

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

  QApplication app(argc, argv);
  app.setApplicationName(QLatin1String("Kid3"));

  Utils::loadTranslation();

#ifdef Q_OS_MAC
 QDir dir(QApplication::applicationDirPath());
 dir.cdUp();
 dir.cd(QLatin1String("PlugIns"));
 QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

  PlatformTools platformTools;
  Kid3MainWindow* kid3 = new Kid3MainWindow(&platformTools);
  kid3->setAttribute(Qt::WA_DeleteOnClose);
  kid3->show();
  if (argc > 1) {
    kid3->confirmedOpenDirectory(QFile::decodeName(argv[1]));
  } else if (ConfigStore::s_miscCfg.m_loadLastOpenedFile &&
             !ConfigStore::s_miscCfg.m_lastOpenedFile.isEmpty()) {
    kid3->confirmedOpenDirectory(ConfigStore::s_miscCfg.m_lastOpenedFile);
  }
  return app.exec();
}
