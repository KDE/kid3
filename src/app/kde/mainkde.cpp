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
#include <QApplication>
#include <KAboutData>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QCommandLineParser>
#include <typeinfo>
#include "fileconfig.h"
#include "loadtranslation.h"
#include "kdemainwindow.h"
#include "kdeplatformtools.h"
#include "kid3application.h"

/**
 * QApplication subclass which catches exceptions.
 */
class Kid3KdeApplication : public QApplication {
public:
  /**
   * Constructor.
   * @param argc number of command line arguments
   * @param argv array of command line arguments
   */
  Kid3KdeApplication(int& argc, char** argv) : QApplication(argc, argv) {}

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
  // Enable support for high resolution "@2x" images
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= 0x050600
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  Kid3KdeApplication app(argc, argv);
  KAboutData aboutData(QStringLiteral("kid3"),
                       QStringLiteral("Kid3"),
                       QStringLiteral(VERSION),
                       QStringLiteral("Audio Tag Editor"), KAboutLicense::GPL,
                       QStringLiteral("(c) 2003-" RELEASE_YEAR " Urs Fleisch"),
                       QString(),
                       QStringLiteral("http://kid3.sourceforge.net"),
                       QStringLiteral("ufleisch@users.sourceforge.net"));
  aboutData.setOrganizationDomain(QByteArray("sourceforge.net"));
  aboutData.addAuthor(QStringLiteral("Urs Fleisch"), QString(),
                      QStringLiteral("ufleisch@users.sourceforge.net"));
  aboutData.setProductName(QByteArray("kid3"));
  KAboutData::setApplicationData(aboutData);
  app.setApplicationName(aboutData.componentName());
  app.setApplicationDisplayName(aboutData.displayName());
  app.setOrganizationDomain(aboutData.organizationDomain());
  app.setApplicationVersion(aboutData.version());
  QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kid3")));

  QCommandLineParser parser;
  aboutData.setupCommandLine(&parser);
  parser.setApplicationDescription(aboutData.shortDescription());
  parser.addHelpOption();
  parser.addVersionOption();
  const char* const directoryToOpenStr =
      QT_TRANSLATE_NOOP("@default", "directory to open");
  parser.addPositionalArgument(
        QStringLiteral("dir"), QCoreApplication::translate("@default",
        directoryToOpenStr),
        QStringLiteral("[dir...]"));
  parser.process(app);
  aboutData.processCommandLine(&parser);

  QString configuredLanguage =
      KConfigGroup(KSharedConfig::openConfig(), "Locale").readEntry("Language");
  Utils::loadTranslation(configuredLanguage);

  const char* const audioTagEditorStr =
      QT_TRANSLATE_NOOP("@default", "Audio Tag Editor");
  aboutData.setShortDescription(
        QCoreApplication::translate("@default", audioTagEditorStr));
  aboutData.setTranslator(
        QCoreApplication::translate("@default",
            // i18n NAME OF TRANSLATORS
            QT_TRANSLATE_NOOP("@default", "Your names")),
        QCoreApplication::translate("@default",
            // i18n EMAIL OF TRANSLATORS
            QT_TRANSLATE_NOOP("@default", "Your emails")));

  IPlatformTools* platformTools = new KdePlatformTools;
  Kid3Application* kid3App = new Kid3Application(platformTools);
#ifdef HAVE_QTDBUS
  kid3App->activateDbusInterface();
#endif
  if (app.isSessionRestored()) {
    int n = 1;
    while (KMainWindow::canBeRestored(n)) {
      (new KdeMainWindow(platformTools, kid3App))->restore(n);
      n++;
    }
  } else {
    KdeMainWindow* kid3 = new KdeMainWindow(platformTools, kid3App);
    kid3->show();

    if (parser.positionalArguments().count()) {
      kid3->confirmedOpenDirectory(parser.positionalArguments());
    } else if (FileConfig::instance().loadLastOpenedFile() &&
               !FileConfig::instance().lastOpenedFile().isEmpty()) {
      kid3->confirmedOpenDirectory(QStringList()
                                   << FileConfig::instance().lastOpenedFile());
    }
  }

  int rc = app.exec();
  delete kid3App;
  delete platformTools;
  return rc;
}
