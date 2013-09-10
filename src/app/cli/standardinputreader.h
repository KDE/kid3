/**
 * \file standardinputreader.h
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

#ifndef STANDARDINPUTREADER_H
#define STANDARDINPUTREADER_H

#include <QThread>

class QTextStream;

/**
 * Thread reading lines from standard input.
 */
class StandardInputReader : public QThread {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit StandardInputReader(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~StandardInputReader();

  /**
   * Stop thread.
   */
  void stop();

signals:
  /**
   * Emitted when a line from standard input is ready.
   * @param line line read from standard input
   */
  void lineReady(const QString& line);

protected:
  /**
   * Start thread.
   */
  virtual void run();

private:
  QTextStream* m_stdIn;
  volatile bool m_running;
};

#endif // STANDARDINPUTREADER_H
