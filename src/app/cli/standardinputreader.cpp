/**
 * \file standardinputreader.cpp
 * Reader for lines from standard input.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
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

#include "standardinputreader.h"
#include "cliconfig.h"
#ifdef HAVE_READLINE
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#if RL_READLINE_VERSION < 0x0600
#include <cstdlib>
#endif
#elif defined Q_OS_WIN32
#include <windows.h>
#else
#include <QTextStream>
#endif

/**
 * Constructor.
 * @param prompt command line prompt
 * @param parent parent object
 */
StandardInputReader::StandardInputReader(const char* prompt, QObject* parent) :
  QObject(parent), m_prompt(prompt)
{
}

/**
 * Destructor.
 */
StandardInputReader::~StandardInputReader()
{
#ifdef HAVE_READLINE
  ::rl_cleanup_after_signal();
#endif
}

/**
 * Read the next line.
 */
void StandardInputReader::readLine()
{
#ifdef HAVE_READLINE
  char* lineRead = ::readline(m_prompt);
  if (lineRead && *lineRead) {
    ::add_history(lineRead);
  }
  QString line = QString::fromLocal8Bit(lineRead);
#if RL_READLINE_VERSION >= 0x0600
  ::rl_free(lineRead);
#else
  ::free(lineRead);
#endif
#elif defined Q_OS_WIN32
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE),
                m_prompt, qstrlen(m_prompt), 0, 0);
  const int numCharsInBuf = 512;
  wchar_t buf[numCharsInBuf];
  DWORD numCharsRead;
  QString line;
  do {
    ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE),
        buf, numCharsInBuf, &numCharsRead, 0);
    line += QString::fromWCharArray(buf, numCharsRead);
  } while (numCharsRead > 0 && line[line.length() - 1] != QLatin1Char('\n'));
  while (line.length() > 0 &&
         (line[line.length() - 1] == QLatin1Char('\r') ||
          line[line.length() - 1] == QLatin1Char('\n'))) {
    line.truncate(line.length() - 1);
  }
#else
  QTextStream stdOut(stdout, QIODevice::WriteOnly);
  stdOut << m_prompt;
  stdOut.flush();
  QTextStream stdIn(stdin, QIODevice::ReadOnly);
  QString line = stdIn.readLine();
#endif
  emit lineReady(line);
}
