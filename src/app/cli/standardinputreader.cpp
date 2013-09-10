/**
 * \file standardinputreader.cpp
 * Thread reading lines from standard input.
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
#include <QTextStream>

/**
 * Constructor.
 * @param parent parent object
 */
StandardInputReader::StandardInputReader(QObject* parent) :
  QThread(parent),
  m_stdIn(new QTextStream(stdin, QIODevice::ReadOnly)), m_running(false)
{
}

/**
 * Destructor.
 */
StandardInputReader::~StandardInputReader()
{
  delete m_stdIn;
}

/**
 * Stop thread.
 */
void StandardInputReader::stop()
{
  m_running = false;
  m_stdIn->flush();
  terminate();
  wait();
}

/**
 * Start thread.
 */
void StandardInputReader::run()
{
  m_running = true;
  while (m_running) {
    QString line = m_stdIn->readLine();
    if (line.isNull()) {
      break;
    }
    emit lineReady(line);
  }
}
