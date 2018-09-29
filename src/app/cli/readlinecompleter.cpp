/**
 * \file readlinecompleter.cpp
 * Abstract base class for readline completer.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Sep 2013
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

#include "readlinecompleter.h"

#include <cstdio>
#include <readline/readline.h>
#include <cstdlib>

/** Installed readline completer. */
ReadlineCompleter* ReadlineCompleter::s_completer = nullptr;

/**
 * Destructor.
 */
ReadlineCompleter::~ReadlineCompleter()
{
}

/**
 * Install this completer to be used with readline.
 */
void ReadlineCompleter::install()
{
  s_completer = this;
  ::rl_attempted_completion_function = completion;
#if RL_READLINE_VERSION > 0x0402
  ::rl_completer_quote_characters = "\"";
  ::rl_filename_quote_characters = " '\"\\\t";
#else
  ::rl_completer_quote_characters = const_cast<char*>("\"");
#endif
}

/**
 * Readline completion function.
 * @param text contents to complete
 * @param start start index in rl_line_buffer of word to complete
 * @param end end index in rl_line_buffer of word to complete
 * @return array of matches or 0 if there aren't any.
 */
char** ReadlineCompleter::completion(const char* text, int start, int end)
{
  Q_UNUSED(end)
  char** matches = nullptr;

  if (start == 0) {
    matches = ::rl_completion_matches(text, commandGenerator);
  } else if (s_completer) {
    if (s_completer->updateParameterList(::rl_line_buffer)) {
      matches = ::rl_completion_matches(text, parameterGenerator);
      if (!matches) {
        ::rl_attempted_completion_over = 1;
      }
    } else {
#if RL_READLINE_VERSION > 0x0402
      ::rl_filename_quoting_desired = 1;
#endif
    }
  }

  return matches;
}

/**
 * Readline command generator.
 * @param text partial word to be completed
 * @param state 0 for first call, >0 for subsequent calls
 * @return next completion string, allocated with malloc(), 0 if there are
 * no more possibilities left.
 */
char* ReadlineCompleter::commandGenerator(const char* text, int state)
{
  if (s_completer) {
    return completionGenerator(s_completer->getCommandList(), text, state);
  }
  return nullptr;
}

/**
 * Readline parameter generator.
 * @param text partial word to be completed
 * @param state 0 for first call, >0 for subsequent calls
 * @return next completion string, allocated with malloc(), 0 if there are
 * no more possibilities left.
 */
char* ReadlineCompleter::parameterGenerator(const char* text, int state)
{
  if (s_completer) {
    return completionGenerator(s_completer->getParameterList(), text, state);
  }
  return nullptr;
}

/**
 * Readline completion generator.
 * @param completions list of completions
 * @param text partial word to be completed
 * @param state 0 for first call, >0 for subsequent calls
 * @return next completion string, allocated with malloc(), 0 if there are
 * no more possibilities left.
 */
char* ReadlineCompleter::completionGenerator(
    const QList<QByteArray>& completions, const char* text, int state)
{
  static int listIndex, textLen;
  if (state == 0) {
    listIndex = 0;
    textLen = qstrlen(text);
  }

  while (listIndex < completions.size()) {
    const QByteArray& name = completions.at(listIndex++);
    if (name.left(textLen) == text) {
      char* r = reinterpret_cast<char*>(::malloc(name.length() + 1));
      qstrcpy(r, name.constData());
      return r;
    }
  }

  return nullptr;
}
