/**
 * \file iusercommandprocessor.h
 * Interface for user command processor.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Feb 2015
 *
 * Copyright (C) 2015-2018  Urs Fleisch
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

#pragma once

#include <QtPlugin>
#include "kid3api.h"

class QString;
class QStringList;
class Kid3Application;

/**
 * Interface for user command processor.
 */
class KID3_CORE_EXPORT IUserCommandProcessor {
public:
  /**
   * Destructor.
   */
  virtual ~IUserCommandProcessor();

  /**
   * Get keys of available user commands.
   * @return list of keys.
   */
  virtual QStringList userCommandKeys() const = 0;

  /**
   * Initialize processor.
   * This method can be used to initialize the processor before it is used.
   * @param app application context
   */
  virtual void initialize(Kid3Application* app);

  /**
   * Cleanup processor.
   * This method can be used to clean up resources for which the plugin
   * destruction time is too late.
   */
  virtual void cleanup();

  /**
   * Start user command.
   * @param key user command name
   * @param arguments arguments to pass to command
   * @param showOutput true to enable output in output viewer
   * @return true if command is started.
   *
   * @remarks If @a showOutput is true, command output is emitted using a signal
   * "void commandOutput(QString)". Objects implementing this interface have to
   * be QObjects providing such a signal.
   * @see qobject()
   */
  virtual bool startUserCommand(
      const QString& key, const QStringList& arguments, bool showOutput) = 0;

  /**
   * Return object which emits commandOutput() signal.
   *
   * @return object which emits signals.
   */
  virtual QObject* qobject() = 0;
};

Q_DECLARE_INTERFACE(IUserCommandProcessor,
                    "net.sourceforge.kid3.IUserCommandProcessor")
