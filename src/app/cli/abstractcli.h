/**
 * \file abstractcli.h
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

#ifndef ABSTRACTCLI_H
#define ABSTRACTCLI_H

#include <QObject>

/**
 * Abstract base class for command line I/O handler.
 */
class AbstractCliIO : public QObject {
  Q_OBJECT
public:
  /**
   * Destructor.
   */
  virtual ~AbstractCliIO() override;

  /**
   * Write a line to standard output.
   * @param line line to write
   */
  virtual void writeLine(const QString& line) = 0;

  /**
   * Write a line to standard error.
   * @param line line to write
   */
  virtual void writeErrorLine(const QString& line) = 0;

  /**
   * Flush the standard output.
   */
  virtual void flushStandardOutput() = 0;

  /**
   * Read the next line.
   * When the line is ready, lineReady() is emitted.
   */
  virtual void readLine() = 0;

public slots:
  /**
   * Start processing.
   * lineReady() is emitted when the first line is ready. To request
   * subsequent lines, readLine() has to be called.
   */
  virtual void start() = 0;

  /**
   * Stop processing.
   * Implementations must finally call deleteLater() to delete this object.
   */
  virtual void stop() = 0;

signals:
  /**
   * Emitted when a line from standard input is ready.
   * @param line line read from standard input
   */
  void lineReady(const QString& line);
};


/**
 * Abstract base class for command line interface.
 */
class AbstractCli : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param io I/O handler
   * @param parent parent object
   */
  explicit AbstractCli(AbstractCliIO* io, QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~AbstractCli() override;

  /**
   * Write a line to standard output.
   * @param line line to write
   */
  void writeLine(const QString& line);

  /**
   * Write a line to standard error.
   * @param line line to write
   */
  void writeErrorLine(const QString& line);

  /**
   * Flush the standard output.
   */
  void flushStandardOutput();

  /**
   * Prompt next line from standard input.
   * Has to be called when the processing in readLine() is finished and
   * the user shall be prompted for the next line.
   */
  void promptNextLine();

  /**
   * Set return code of application.
   * @param code return code, 0 means success
   */
  void setReturnCode(int code);

public slots:
  /**
   * Execute process.
   */
  virtual void execute();

  /**
   * Terminate command line processor.
   */
  virtual void terminate();

protected slots:
  /**
   * Process command line.
   * Has to be implemented by concrete derived class.
   * @param line command line
   */
  virtual void readLine(const QString& line) = 0;

private slots:
  /**
   * Exit application with return code.
   */
  void quitApplicationWithReturnCode();

private:
  AbstractCliIO* m_io;
  int m_returnCode;
};

#endif // ABSTRACTCLI_H
