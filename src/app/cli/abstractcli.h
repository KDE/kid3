/**
 * \file abstractcli.h
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

#ifndef ABSTRACTCLI_H
#define ABSTRACTCLI_H

#include <QObject>
#ifndef Q_OS_WIN32
#include <QTextStream>
#endif

class StandardInputReader;

/**
 * Abstract base class for command line interface.
 */
class AbstractCli : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit AbstractCli(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~AbstractCli();

  /**
   * Set prompt.
   * @param prompt command line prompt
   */
  void setPrompt(const char* prompt);

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

private:
#ifndef Q_OS_WIN32
  QTextStream m_cout;
  QTextStream m_cerr;
#endif
  StandardInputReader* m_stdinReader;
};

#endif // ABSTRACTCLI_H
