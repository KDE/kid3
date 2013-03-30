/**
 * \file mainkde.cpp
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
#include <kdeversion.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include "configstore.h"
#include "loadtranslation.h"
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
  KAboutData aboutData(
    "kid3", "kdelibs4", ki18n("Kid3"),
    VERSION, ki18n("Kid3 ID3 Tagger"), KAboutData::License_GPL,
    ki18n("(c) 2003-" RELEASE_YEAR " Urs Fleisch"), KLocalizedString(),
    "http://kid3.sourceforge.net",
    "ufleisch@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Urs Fleisch"), KLocalizedString(),
                      "ufleisch@users.sourceforge.net");
  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
  options.add("+[Dir]", ki18n("%1").subs(QCoreApplication::translate("@default",
                          QT_TRANSLATE_NOOP("@default", "directory to open"))));
  KCmdLineArgs::addCmdLineOptions(options);
  KApplication app;

  QString configuredLanguage =
      KConfigGroup(KGlobal::config(), "Locale").readEntry("Language");
  Utils::loadTranslation(configuredLanguage);

  aboutData.setShortDescription(
        ki18n("%1").subs(QCoreApplication::translate("@default",
            QT_TRANSLATE_NOOP("@default", "Kid3 ID3 Tagger"))));
  aboutData.setTranslator(
        ki18n("%1").subs(QCoreApplication::translate("@default",
            // i18n NAME OF TRANSLATORS
            QT_TRANSLATE_NOOP("@default", "Your names"))),
        ki18n("%1").subs(QCoreApplication::translate("@default",
            // i18n EMAIL OF TRANSLATORS
            QT_TRANSLATE_NOOP("@default", "Your emails"))));
  // Should not be used, but seems to be the only way to update the "about data"
  // with translated information.
  KGlobal::activeComponent().setAboutData(aboutData);

  if (app.isSessionRestored()) {
    RESTORE(Kid3MainWindow);
  }
  else {
    Kid3MainWindow* kid3 = new Kid3MainWindow;
    kid3->show();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    if (args->count()) {
      kid3->confirmedOpenDirectory(args->arg(0));
    } else if (ConfigStore::s_miscCfg.m_loadLastOpenedFile &&
               !ConfigStore::s_miscCfg.m_lastOpenedFile.isEmpty()) {
      kid3->confirmedOpenDirectory(ConfigStore::s_miscCfg.m_lastOpenedFile);
    }
    args->clear();
  }

  return app.exec();
}
