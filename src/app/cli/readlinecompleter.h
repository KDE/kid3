/**
 * \file readlinecompleter.h
 * Abstract base class for readline completer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Sep 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include <QList>
#include <QByteArray>

/**
 * Abstract base class for readline completer.
 */
class ReadlineCompleter {
public:
  /**
   * Destructor.
   */
  virtual ~ReadlineCompleter();

  /**
   * Get list of available commands.
   * @return command list.
   */
  virtual QList<QByteArray> getCommandList() const = 0;

  /**
   * Get list of available parameter values.
   * @return list of possible parameter values.
   */
  virtual QList<QByteArray> getParameterList() const = 0;

  /**
   * Update the list of possible parameter values.
   * @param buffer buffer containing command name and partial parameters
   * @return true if list updated, false if file name completion shall be used.
   */
  virtual bool updateParameterList(const char* buffer) = 0;

  /**
   * Install this completer to be used with readline.
   */
  void install();

private:
  static char** completion(const char* text, int start, int end);
  static char* commandGenerator(const char* text, int state);
  static char* parameterGenerator(const char* text, int state);
  static char* completionGenerator(
      const QList<QByteArray>& completions, const char* text, int state);

  static ReadlineCompleter* s_completer;
};
