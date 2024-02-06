/**
 * \file jsoncliformatter.h
 * CLI formatter with JSON input and output.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Jul 2019
 *
 * Copyright (C) 2019-2024  Urs Fleisch
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

#include <QJsonObject>
#include "abstractcliformatter.h"

class QJsonObject;

/**
 * CLI formatter with JSON input and output.
 */
class JsonCliFormatter : public AbstractCliFormatter {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param io I/O handler
   */
  explicit JsonCliFormatter(AbstractCliIO* io);

  /**
   * Destructor.
   */
  ~JsonCliFormatter() override;

  /**
   * Clear parser state.
   */
  void clear() override;

  /**
   * Get command and parameters from input line.
   * @param line input line
   * @return list of command and arguments, empty if not found or incomplete.
   */
  QStringList parseArguments(const QString& line) override;

  /**
   * Get error which occurred in previous method call.
   * @return error message, null if no error.
   */
  QString getErrorMessage() const override;

  /**
   * Check if format was recognized and parsed, but input is will be continued
   * in subsequent lines.
   * @return true if input is incomplete.
   */
  bool isIncomplete() const override;

  /**
   * Check if format was recognized and parsed.
   * @return true if format was recognized.
   */
  bool isFormatRecognized() const override;


  /**
   * Write error message.
   * @param errorCode error code
   */
  void writeError(CliError errorCode) override;

  /**
   * Write error message.
   * @param msg error message
   */
  void writeError(const QString& msg) override;

  /**
   * Write error message.
   * @param msg error message
   * @param errorCode error code
   */
  void writeError(const QString& msg, CliError errorCode) override;

  /**
   * Write result message.
   * @param str result as string
   */
  void writeResult(const QString& str) override;

  /**
   * Write result message.
   * @param strs result as string list
   */
  void writeResult(const QStringList& strs) override;

  /**
   * Write result message.
   * @param map result as map
   */
  void writeResult(const QVariantMap& map) override;

  /**
   * Write result message.
   * @param result result as boolean
   */
  void writeResult(bool result) override;

  /**
   * Called when a command is finished.
   */
  void finishWriting() override;

private:
  void writeErrorMessage(const QString& msg, int code);

  QString m_jsonRequest;
  QString m_jsonId;
  QString m_errorMessage;
  QStringList m_args;
  QJsonObject m_response;
  bool m_compact;
};
