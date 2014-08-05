/**
 * \file kid3qtapplication.h
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

#include <QApplication>

/**
 * QApplication subclass with adapted session management.
 */
class Kid3QtApplication : public QApplication {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param argc number of arguments (including command)
   * @param argv arguments
   */
  Kid3QtApplication(int& argc, char** argv);

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
  virtual void commitData(QSessionManager& manager);

  /**
   * Send event to receiver.
   * @param receiver receiver
   * @param event event
   * @return return value from receiver's event handler.
   */
  virtual bool notify(QObject* receiver, QEvent* event);

signals:
  /**
   * Emitted when files have to be opened.
   * @param paths path to file
   */
  void openFileRequested(const QStringList& paths);

protected:
  /**
   * Handle file open events on Mac OS X.
   * @param e event
   * @return true if event handled.
   */
  virtual bool event(QEvent* e);
};
