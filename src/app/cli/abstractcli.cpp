/**
 * \file abstractcli.cpp
 * Abstract base class for command line interface.
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

#include "abstractcli.h"
#include <QTimer>
#include <QCoreApplication>
#include <QThread>
#ifdef Q_OS_WIN32
#include <windows.h>
#endif
#include "standardinputreader.h"

/**
 * Constructor.
 * @param prompt command line prompt
 * @param parent parent object
 */
AbstractCli::AbstractCli(const char* prompt, QObject* parent) : QObject(parent),
#ifndef Q_OS_WIN32
  m_cout(stdout, QIODevice::WriteOnly), m_cerr(stderr, QIODevice::WriteOnly),
#endif
  m_stdinReader(new StandardInputReader(prompt))
{
}

/**
 * Destructor.
 */
AbstractCli::~AbstractCli()
{
}

/**
 * Prompt next line from standard input.
 * Has to be called when the processing in readLine() is finished and
 * the user shall be prompted for the next line.
 */
void AbstractCli::promptNextLine()
{
  emit requestNextLine();
}

/**
 * Execute process.
 */
void AbstractCli::execute()
{
  QThread* conInThread = new QThread;
  conInThread->setObjectName(QLatin1String("conInThread"));

  connect(conInThread, SIGNAL(started()), m_stdinReader, SLOT(readLine()));
  connect(conInThread, SIGNAL(finished()), m_stdinReader, SLOT(deleteLater()));
  connect(m_stdinReader, SIGNAL(lineReady(QString)),
          this, SLOT(readLine(QString)), Qt::QueuedConnection);
  connect(this, SIGNAL(requestNextLine()),
          m_stdinReader, SLOT(readLine()), Qt::QueuedConnection);
  connect(this, SIGNAL(requestTermination()), conInThread, SLOT(quit()));
  connect(conInThread, SIGNAL(finished()), conInThread, SLOT(deleteLater()));
#if QT_VERSION < 0x050000
  connect(conInThread, SIGNAL(terminated()), conInThread, SLOT(deleteLater()));
#endif
  m_stdinReader->moveToThread(conInThread);

  conInThread->start();
}

/**
 * Terminate command line processor.
 */
void AbstractCli::terminate()
{
  flushStandardOutput();
  emit requestTermination();
  QTimer::singleShot(0, qApp, SLOT(quit()));
}

/**
 * Write a line to standard output.
 * @param line line to write
 */
void AbstractCli::writeLine(const QString& line)
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
void AbstractCli::writeErrorLine(const QString& line)
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
void AbstractCli::flushStandardOutput()
{
#ifndef Q_OS_WIN32
  m_cout.flush();
#endif
}
