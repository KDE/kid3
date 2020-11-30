/**
 * \file jsoncliformatter.cpp
 * CLI formatter with JSON input and output.
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

#include "jsoncliformatter.h"
#include <climits>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>
#include <QStringBuilder>
#include "clierror.h"
#include "abstractcli.h"
#include "frame.h"

namespace {

int jsonRpcErrorCode(CliError errorCode)
{
  // See error codes at https://www.jsonrpc.org/specification
  int code = -1;
  switch (errorCode) {
  case CliError::Ok:
    code = 0;
    break;
  case CliError::ApplicationError:
    code = -1;
    break;
  case CliError::ParseError:
    code = -32700;
    break;
  case CliError::InvalidRequest:
  case CliError::Usage:
    code = -32600;
    break;
  case CliError::MethodNotFound:
    code = -32601;
    break;
  case CliError::InvalidParams:
    code = -32602;
    break;
  case CliError::InternalError:
    code = -32603;
    break;
  }
  return code;
}

}


JsonCliFormatter::JsonCliFormatter(AbstractCliIO* io)
  : AbstractCliFormatter(io), m_compact(false)
{
}

JsonCliFormatter::~JsonCliFormatter()
{
}

void JsonCliFormatter::clear()
{
  m_jsonRequest.clear();
  m_jsonId.clear();
  m_errorMessage.clear();
  m_args.clear();
  m_response = QJsonObject();
  m_compact = false;
}

QStringList JsonCliFormatter::parseArguments(const QString& line)
{
  m_errorMessage.clear();
  m_args.clear();
  if (m_jsonRequest.isEmpty()) {
    m_jsonRequest = line.trimmed();
    if (!m_jsonRequest.startsWith(QLatin1Char('{'))) {
      m_jsonRequest.clear();
    }
  } else {
    m_jsonRequest.append(line.trimmed());
  }
  if (!m_jsonRequest.isEmpty()) {
    if (!m_jsonRequest.endsWith(QLatin1Char('}'))) {
      // Probably partial JSON request
      return QStringList();
    }
    m_compact = m_jsonRequest.contains(QLatin1String("\"method\":\""));
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(m_jsonRequest.toUtf8(), &error);
    if (!doc.isNull()) {
      QJsonObject obj = doc.object();
      if (!obj.isEmpty()) {
        auto method = obj.value(QLatin1String("method")).toString();
        if (!method.isEmpty()) {
          m_args.append(method);
          const auto params = obj.value(QLatin1String("params")).toArray();
          for (const auto& param : params) {
            QString arg = param.toString();
            if (arg.isEmpty()) {
              if (param.isArray()) {
                // Special handling for tags parameter of the form [1, 2]
                const auto elements = param.toArray();
                for (const auto& element : elements) {
                  int tagNr = element.toInt();
                  if (tagNr > 0 && tagNr <= Frame::Tag_NumValues) {
                    arg += QLatin1Char('0' + static_cast<char>(tagNr));
                  } else {
                    arg.clear();
                    break;
                  }
                }
              } else if (param.isDouble()) {
                // Allow integer numbers, for example for track numbers
                int argInt = param.toInt(INT_MIN);
                if (argInt != INT_MIN) {
                  arg = QString::number(argInt);
                }
              } else if (param.isBool()) {
                arg = QLatin1String(param.toBool() ? "true" : "false");
              }
            }
            m_args.append(arg);
          }
          // A JSON-RPC ID is used in the response and to store that a JSON
          // request is running.
          m_jsonId = obj.value(QLatin1String("id"))
              .toString(QLatin1String(""));
        }
      }
    }
    if (m_args.isEmpty()) {
      auto errStr = error.error != QJsonParseError::NoError
          ? error.errorString() : QLatin1String("missing method");
      if (!errStr.isEmpty()) {
        m_errorMessage = errStr + QLatin1String(": ") + m_jsonRequest;
      }
      m_jsonRequest.clear();
      return QStringList();
    }
    m_jsonRequest.clear();
  } else {
    m_jsonId.clear();
  }
  return m_args;
}

QString JsonCliFormatter::getErrorMessage() const
{
  return m_errorMessage;
}

bool JsonCliFormatter::isIncomplete() const
{
  return !m_jsonRequest.isEmpty();
}

bool JsonCliFormatter::isFormatRecognized() const
{
  return !m_jsonId.isNull() || !m_jsonRequest.isEmpty() ||
      !m_errorMessage.isEmpty();
}

void JsonCliFormatter::writeError(CliError errorCode)
{
  QString msg;
  if (errorCode == CliError::MethodNotFound) {
#if QT_VERSION >= 0x050600
    msg = tr("Unknown command '%1'")
        .arg(m_args.isEmpty() ? QLatin1String("") : m_args.constFirst());
#else
    msg = tr("Unknown command '%1'")
        .arg(m_args.isEmpty() ? QLatin1String("") : m_args.first());
#endif
  }
  writeErrorMessage(msg, jsonRpcErrorCode(errorCode));
}

void JsonCliFormatter::writeError(const QString& msg)
{
  writeErrorMessage(msg, -1);
}

void JsonCliFormatter::writeError(const QString& msg, CliError errorCode)
{
  QString errorMsg = msg;
  if (errorCode == CliError::Usage) {
    errorMsg = tr("Usage:") % QLatin1Char(' ') % errorMsg;
  }
  writeErrorMessage(errorMsg, jsonRpcErrorCode(errorCode));
}

void JsonCliFormatter::writeErrorMessage(const QString& msg, int code)
{
  QJsonObject error;
  error.insert(QLatin1String("code"), code);
  error.insert(QLatin1String("message"), msg);
  m_response.insert(QLatin1String("error"), error);
}

void JsonCliFormatter::writeResult(const QString& str)
{
  m_response.insert(QLatin1String("result"), str);
}

void JsonCliFormatter::writeResult(const QStringList& strs)
{
  m_response.insert(QLatin1String("result"), QJsonArray::fromStringList(strs));
}

void JsonCliFormatter::writeResult(const QVariantMap& map)
{
  QJsonObject result;
  if (map.size() == 1 && map.contains(QLatin1String("event"))) {
    result = m_response.value(QLatin1String("result")).toObject();
    auto events = result.value(QLatin1String("events")).toArray();
    events.append(QJsonValue::fromVariant(map.value(QLatin1String("event"))));
    result.insert(QLatin1String("events"), events);
  } else {
    result = QJsonObject::fromVariantMap(map);
  }
  m_response.insert(QLatin1String("result"), result);
}

void JsonCliFormatter::writeResult(bool result)
{
  m_response.insert(QLatin1String("result"), result);
}

void JsonCliFormatter::finishWriting()
{
  if (m_response.isEmpty()) {
    m_response.insert(QLatin1String("result"), QJsonValue::Null);
  }
  if (!m_jsonId.isEmpty()) {
    m_response.insert(QLatin1String("jsonrpc"), QLatin1String("2.0"));
    m_response.insert(QLatin1String("id"), m_jsonId);
  }
  io()->writeLine(QString::fromUtf8(
                    QJsonDocument(m_response).toJson(
                      m_compact ? QJsonDocument::Compact
                                : QJsonDocument::Indented)));
}
