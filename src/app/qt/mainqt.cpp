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
#include <typeinfo>
#include "fileconfig.h"
#include "loadtranslation.h"
#include "kid3mainwindow.h"
#include "platformtools.h"
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
  Q_INIT_RESOURCE(kid3);

  Kid3QtApplication app(argc, argv);
  app.setApplicationName(QLatin1String("Kid3"));

  Utils::loadTranslation();

#ifdef Q_OS_MAC
 QDir dir(QApplication::applicationDirPath());
 dir.cdUp();
 dir.cd(QLatin1String("PlugIns"));
 QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

  IPlatformTools* platformTools = new PlatformTools;
  Kid3Application* kid3App = new Kid3Application(platformTools);
  Kid3MainWindow* kid3 = new Kid3MainWindow(platformTools, kid3App);
  kid3->setAttribute(Qt::WA_DeleteOnClose);
  kid3->show();
  if (argc > 1) {
    kid3->confirmedOpenDirectory(QApplication::arguments().mid(1));
  } else if ((FileConfig::instance().m_loadLastOpenedFile ||
              app.isSessionRestored()) &&
             !FileConfig::instance().m_lastOpenedFile.isEmpty()) {
    kid3->confirmedOpenDirectory(QStringList()
                                 << FileConfig::instance().m_lastOpenedFile);
  }
  int rc = app.exec();
  delete kid3App;
  delete platformTools;
  return rc;
}
