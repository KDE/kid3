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
#include <typeinfo>
#include "fileconfig.h"
#include "loadtranslation.h"
#include "kdemainwindow.h"
#include "kdeplatformtools.h"
#include "kid3application.h"

/** To use a constructor with arguments in RESTORE(). */
#define KdeMainWindow_pt_app KdeMainWindow(platformTools, kid3App)

/**
 * KApplication subclass which catches exceptions.
 */
class Kid3KdeApplication : public KApplication {
public:
  /**
   * Send event to receiver.
   * @param receiver receiver
   * @param event event
   * @return return value from receiver's event handler.
   */
  virtual bool notify(QObject* receiver, QEvent* event);
};

/**
 * Send event to receiver.
 * @param receiver receiver
 * @param event event
 * @return return value from receiver's event handler.
 */
bool Kid3KdeApplication::notify(QObject* receiver, QEvent* event)
{
  try {
    return KApplication::notify(receiver, event);
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
  Kid3KdeApplication app;

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
#if KDE_VERSION >= 0x040500
  // Should not be used, but seems to be the only way to update the "about data"
  // with translated information.
  KGlobal::activeComponent().setAboutData(aboutData);
#endif

  IPlatformTools* platformTools = new KdePlatformTools;
  Kid3Application* kid3App = new Kid3Application(platformTools);
  if (app.isSessionRestored()) {
    RESTORE(KdeMainWindow_pt_app)
  } else {
    KdeMainWindow* kid3 = new KdeMainWindow(platformTools, kid3App);
    kid3->show();

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    if (args->count()) {
#if KDE_VERSION >= 0x040600
      kid3->confirmedOpenDirectory(args->allArguments().mid(1));
#else
      QStringList args1;
      for (int i = 0; i < args->count(); ++i)
        args1.append(args->arg(i));
      kid3->confirmedOpenDirectory(args1);
#endif
    } else if (FileConfig::instance().m_loadLastOpenedFile &&
               !FileConfig::instance().m_lastOpenedFile.isEmpty()) {
      kid3->confirmedOpenDirectory(QStringList()
                                   << FileConfig::instance().m_lastOpenedFile);
    }
    args->clear();
  }

  int rc = app.exec();
  delete kid3App;
  delete platformTools;
  return rc;
}
