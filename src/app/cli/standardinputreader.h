/**
 * \file standardinputreader.h
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

#ifndef STANDARDINPUTREADER_H
#define STANDARDINPUTREADER_H

#include <QObject>

/**
 * Reader for lines from standard input.
 *
 * An instance of this class can be used as a worker in a worker thread.
 * The blocking readLine() method can be called when the thread is
 * started and then after lines have been processed. Availability of a new
 * line is signaled with lineReady(). The controlling thread should only
 * communicate with the worker thread using queued signal-slot connections.
 */
class StandardInputReader : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param prompt command line prompt
   * @param parent parent object
   */
  explicit StandardInputReader(const char* prompt = "", QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~StandardInputReader();

public slots:
  /**
   * Read the next line.
   * This method will block until a line is read from standard input.
   * When the line is ready, lineReady() is emitted.
   */
  void readLine();

signals:
  /**
   * Emitted when a line from standard input is ready.
   * @param line line read from standard input
   */
  void lineReady(const QString& line);

private:
  const char* m_prompt;
};

#endif // STANDARDINPUTREADER_H
