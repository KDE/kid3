/**
 * \file kid3qtapplication.cpp
 * QApplication subclass with adapted session management.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Aug 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "kid3qtapplication.h"
#include <typeinfo>
#include <QStringList>
#ifdef Q_OS_MAC
#include <QFileOpenEvent>
#endif

/**
 * Constructor.
 * @param argc number of arguments (including command)
 * @param argv arguments
 */
Kid3QtApplication::Kid3QtApplication(int& argc, char** argv) : QApplication(argc, argv)
{
#if QT_VERSION >= 0x050100
  // Enable support for high resolution "@2x" images
  setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

/**
 * Destructor.
 */
Kid3QtApplication::~Kid3QtApplication()
{
}

/**
 * Called when session manager wants application to commit all its data.
 *
 * This method is reimplemented to avoid closing all top level widgets and
 * make restoring with the KDE window manager working.
 *
 * @param manager session manager
 */
void Kid3QtApplication::commitData(QSessionManager& manager)
{
  emit commitDataRequest(manager);
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
 * Handle file open events on Mac OS X.
 * @param e event
 * @return true if event handled.
 */
bool Kid3QtApplication::event(QEvent* e)
{
#ifdef Q_OS_MAC
  if (e->type() == QEvent::FileOpen) {
    emit openFileRequested(QStringList() << static_cast<QFileOpenEvent*>(e)->file());
    return true;
  }
#endif
  return QApplication::event(e);
}
