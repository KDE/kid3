/**
 * \file abstractcliformatter.h
 * Abstract base class for CLI formatter.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Jul 2019
 *
 * Copyright (C) 2019  Urs Fleisch
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

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class AbstractCliIO;
enum class CliError : int;

/**
 * Abstract base class for CLI formatter.
 */
class AbstractCliFormatter : public QObject {
public:
  /**
   * Constructor.
   * @param io I/O handler
   */
  explicit AbstractCliFormatter(AbstractCliIO* io);

  /**
   * Destructor.
   */
  virtual ~AbstractCliFormatter();

  /**
   * Clear parser state.
   */
  virtual void clear() = 0;

  /**
   * Get command and parameters from input line.
   * @param line input line
   * @return list of command and arguments, empty if not found or incomplete.
   */
  virtual QStringList parseArguments(const QString& line) = 0;

  /**
   * Get error which occurred in previous method call.
   * @return error message, null if no error.
   */
  virtual QString getErrorMessage() const = 0;

  /**
   * Check if format was recognized and parsed, but input is will be continued
   * in subsequent lines.
   * @return true if input is incomplete.
   */
  virtual bool isIncomplete() const = 0;

  /**
   * Check if format was recognized and parsed.
   * @return true if format was recognized.
   */
  virtual bool isFormatRecognized() const = 0;

  /**
   * Write error message.
   * @param errorCode error code
   */
  virtual void writeError(CliError errorCode) = 0;

  /**
   * Write error message.
   * @param msg error message
   */
  virtual void writeError(const QString& msg) = 0;

  /**
   * Write error message.
   * @param msg error message
   * @param errorCode error code
   */
  virtual void writeError(const QString& msg, CliError errorCode) = 0;

  /**
   * Write result message.
   * @param str result as string
   */
  virtual void writeResult(const QString& str) = 0;

  /**
   * Write result message.
   * @param map result as map
   */
  virtual void writeResult(const QVariantMap& map) = 0;

  /**
   * Called when a command is finished.
   */
  virtual void finishWriting() = 0;

protected:
  /**
   * Access to CLI I/O.
   * @return CLI I/O.
   */
  AbstractCliIO* io() const { return m_io; }

private:
  AbstractCliIO* const m_io;
};
