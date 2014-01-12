/**
 * \file standardiohandler.cpp
 * CLI I/O Handler for standard I/O.
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

#include "standardiohandler.h"
#include "cliconfig.h"
#include <QThread>
#ifdef Q_OS_WIN32
#include <windows.h>
#endif
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
 */
StandardIOHandler::StandardIOHandler(const char* prompt) :
  m_prompt(prompt), m_conInThread(0)
#ifndef Q_OS_WIN32
  , m_cout(stdout, QIODevice::WriteOnly), m_cerr(stderr, QIODevice::WriteOnly)
#endif
{
}

/**
 * Destructor.
 */
StandardIOHandler::~StandardIOHandler()
{
#ifdef HAVE_READLINE
  ::rl_cleanup_after_signal();
#endif
}

/**
 * Start processing.
 */
void StandardIOHandler::start()
{
  m_conInThread = new QThread;
  m_conInThread->setObjectName(QLatin1String("conInThread"));

  connect(m_conInThread, SIGNAL(started()), this, SLOT(blockingReadLine()));
  connect(m_conInThread, SIGNAL(finished()), this, SLOT(deleteLater()));
  connect(m_conInThread, SIGNAL(finished()), m_conInThread, SLOT(deleteLater()));
#if QT_VERSION < 0x050000
  connect(m_conInThread, SIGNAL(terminated()), m_conInThread, SLOT(deleteLater()));
#endif
  moveToThread(m_conInThread);

  m_conInThread->start();
}

/**
 * Stop processing.
 */
void StandardIOHandler::stop()
{
  QMetaObject::invokeMethod(m_conInThread, "quit",
                             Qt::QueuedConnection);
}

/**
 * Read the next line.
 * This method will asynchronously invoke reading of a line from standard
 * input in the read thread.
 * When the line is ready, lineReady() is emitted.
 */
void StandardIOHandler::readLine()
{
  QMetaObject::invokeMethod(this, "blockingReadLine",
                             Qt::QueuedConnection);
}

/**
 * Read the next line.
 */
void StandardIOHandler::blockingReadLine()
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

/**
 * Write a line to standard output.
 * @param line line to write
 */
void StandardIOHandler::writeLine(const QString& line)
{
#ifdef Q_OS_WIN32
  QString str = line + QLatin1Char('\n');
  WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE),
      str.utf16(), str.size(), 0, 0);
#else
  m_cout << line;
  m_cout << QLatin1Char('\n');
  m_cout.flush();
#endif
}

/**
 * Write a line to standard error.
 * @param line line to write
 */
void StandardIOHandler::writeErrorLine(const QString& line)
{
#ifdef Q_OS_WIN32
  QString str = line + QLatin1Char('\n');
  WriteConsoleW(GetStdHandle(STD_ERROR_HANDLE),
      str.utf16(), str.size(), 0, 0);
#else
  m_cerr << line;
  m_cerr << QLatin1Char('\n');
  m_cerr.flush();
#endif
}

/**
 * Flush the standard output.
 */
void StandardIOHandler::flushStandardOutput()
{
#ifndef Q_OS_WIN32
  m_cout.flush();
#endif
}
