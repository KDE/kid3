/**
 * \file standardiohandler.h
 * CLI I/O Handler for standard I/O.
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

#ifndef STANDARDIOHANDLER_H
#define STANDARDIOHANDLER_H

#include "abstractcli.h"
#include <QTextStream>

/**
 * CLI I/O Handler for standard I/O.
 */
class StandardIOHandler : public AbstractCliIO {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param prompt command line prompt
   */
  explicit StandardIOHandler(const char* prompt = "");

  /**
   * Destructor.
   */
  virtual ~StandardIOHandler();

  /**
   * Write a line to standard output.
   * @param line line to write
   */
  virtual void writeLine(const QString& line);

  /**
   * Write a line to standard error.
   * @param line line to write
   */
  virtual void writeErrorLine(const QString& line);

  /**
   * Flush the standard output.
   */
  virtual void flushStandardOutput();

  /**
   * Read the next line.
   * This method will asynchronously invoke reading of a line from standard
   * input in the read thread.
   * When the line is ready, lineReady() is emitted.
   */
  virtual void readLine();

public slots:
  /**
   * Start processing.
   * This will start a read thread. lineReady() is emitted when the first
   * line is ready. To request subsequent lines, readLine() has to be called.
   */
  virtual void start();

  /**
   * Stop processing.
   * This will stop the read thread and finally delete this object.
   */
  virtual void stop();

private slots:
  /**
   * Read the next line.
   * This method will block until a line is read from standard input.
   * When the line is ready, lineReady() is emitted.
   */
  void blockingReadLine();

private:
  const char* m_prompt;
  QThread* m_conInThread;
  QTextStream m_cout;
  QTextStream m_cerr;
#ifdef Q_OS_WIN32
  bool m_consoleMode;
#endif
};

#endif // STANDARDIOHANDLER_H
