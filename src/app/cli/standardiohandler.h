/**
 * \file standardiohandler.h
 * CLI I/O Handler for standard I/O.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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
  ~StandardIOHandler() override = default;

  /**
   * Restore terminal on cleanup.
   */
  void cleanup() override;

  /**
   * Write a line to standard output.
   * @param line line to write
   */
  void writeLine(const QString& line) override;

  /**
   * Write a line to standard error.
   * @param line line to write
   */
  void writeErrorLine(const QString& line) override;

  /**
   * Flush the standard output.
   */
  void flushStandardOutput() override;

  /**
   * Read the next line.
   * This method will asynchronously invoke reading of a line from standard
   * input in the read thread.
   * When the line is ready, lineReady() is emitted.
   */
  void readLine() override;

public slots:
  /**
   * Start processing.
   * This will start a read thread. lineReady() is emitted when the first
   * line is ready. To request subsequent lines, readLine() has to be called.
   */
  void start() override;

  /**
   * Stop processing.
   * This will stop the read thread and finally delete this object.
   */
  void stop() override;

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
  bool m_consoleMode;
};
