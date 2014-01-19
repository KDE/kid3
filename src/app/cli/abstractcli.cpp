/**
 * \file abstractcli.cpp
 * Abstract base class for command line interface.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013-2014  Urs Fleisch
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

/**
 * Destructor.
 */
AbstractCliIO::~AbstractCliIO()
{
}


/**
 * Constructor.
 * @param io I/O handler
 * @param parent parent object
 */
AbstractCli::AbstractCli(AbstractCliIO* io, QObject* parent) : QObject(parent),
  m_io(io), m_returnCode(0)
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
  m_io->readLine();
}

/**
 * Set return code of application.
 * @param code return code, 0 means success
 */
void AbstractCli::setReturnCode(int code)
{
  m_returnCode = code;
}

/**
 * Exit application with return code.
 */
void AbstractCli::quitApplicationWithReturnCode()
{
  QCoreApplication::exit(m_returnCode);
}

/**
 * Execute process.
 */
void AbstractCli::execute()
{
  connect(m_io, SIGNAL(lineReady(QString)),
          this, SLOT(readLine(QString)), Qt::QueuedConnection);
  m_io->start();
}

/**
 * Terminate command line processor.
 */
void AbstractCli::terminate()
{
  flushStandardOutput();
  m_io->stop();
  if (m_returnCode == 0) {
    QTimer::singleShot(0, qApp, SLOT(quit()));
  } else {
    QTimer::singleShot(0, this, SLOT(quitApplicationWithReturnCode()));
  }
}

/**
 * Write a line to standard output.
 * @param line line to write
 */
void AbstractCli::writeLine(const QString& line)
{
  m_io->writeLine(line);
}

/**
 * Write a line to standard error.
 * @param line line to write
 */
void AbstractCli::writeErrorLine(const QString& line)
{
  m_io->writeErrorLine(line);
}

/**
 * Flush the standard output.
 */
void AbstractCli::flushStandardOutput()
{
  m_io->flushStandardOutput();
}
