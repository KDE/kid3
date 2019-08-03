/**
 * \file textcliformatter.h
 * CLI formatter for standard text input and output.
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

#include "abstractcliformatter.h"

/**
 * CLI formatter for standard text input and output.
 */
class TextCliFormatter : public AbstractCliFormatter {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param io I/O handler
   */
  explicit TextCliFormatter(AbstractCliIO* io);

  /**
   * Destructor.
   */
  virtual ~TextCliFormatter() override;

  /**
   * Clear parser state.
   */
  virtual void clear() override;

  /**
   * Get command and parameters from input line.
   * @param line input line
   * @return list of command and arguments, empty if not found or incomplete.
   */
  virtual QStringList parseArguments(const QString& line) override;

  /**
   * Get error which occurred in previous method call.
   * @return error message, null if no error.
   */
  virtual QString getErrorMessage() const override;

  /**
   * Check if format was recognized and parsed, but input is will be continued
   * in subsequent lines.
   * @return true if input is incomplete.
   */
  virtual bool isIncomplete() const override;

  /**
   * Check if format was recognized and parsed.
   * @return true if format was recognized.
   */
  virtual bool isFormatRecognized() const override;

  /**
   * Write error message.
   * @param errorCode error code
   */
  virtual void writeError(CliError errorCode) override;

  /**
   * Write error message.
   * @param msg error message
   */
  virtual void writeError(const QString& msg) override;

  /**
   * Write error message.
   * @param msg error message
   * @param errorCode error code
   */
  virtual void writeError(const QString& msg, CliError errorCode) override;

  /**
   * Write result message.
   * @param str result as string
   */
  virtual void writeResult(const QString& str) override;

  /**
   * Write result message.
   * @param map result as map
   */
  virtual void writeResult(const QVariantMap& map) override;

  /**
   * Called when a command is finished.
   */
  virtual void finishWriting() override;

private:
  QString m_errorMessage;
  QStringList m_args;
};
