/**
 * \file textcliformatter.cpp
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

#include "textcliformatter.h"
#include <QDir>
#include <QStringBuilder>
#include "clierror.h"
#include "abstractcli.h"
#include "frame.h"

/** @cond */
namespace {

/**
 * Split string into command line arguments supporting quotes and escape
 * characters.
 * @param str command line string
 * @return list of arguments.
 */
QStringList splitArgs(const QString& str)
{
  QStringList params;

  for (int pos = 0; ; ) {
    QChar c;
    do {
      if (pos >= str.size())
        return params;
      c = str.at(pos++);
    } while (c.isSpace());
    QString param = QLatin1String("");
    if (c == QLatin1Char('~')) {
      if (pos >= str.size() || str.at(pos).isSpace()) {
        params.append(QDir::homePath());
        continue;
      }
      if (str.at(pos) == QLatin1Char('/')) {
        param = QDir::homePath();
        c = QLatin1Char('/');
        ++pos;
      }
    }
    do {
      if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
        const QChar quote = c;
        for (;;) {
          if (pos >= str.size())
            return QStringList();
          c = str.at(pos++);
          if (c == quote)
            break;
          if (c == QLatin1Char('\\')) {
            if (pos >= str.size())
              return QStringList();
            c = str.at(pos++);
            if (c != quote && c != QLatin1Char('\\'))
              param += QLatin1Char('\\');
          }
          param += c;
        }
      } else {
        if (c == QLatin1Char('\\')) {
          if (pos >= str.size())
            return QStringList();
          c = str.at(pos++);
        }
        param += c;
      }
      if (pos >= str.size())
        break;
      c = str.at(pos++);
    } while (!c.isSpace());
    params.append(param);
  }
}

/**
 * Print list of files.
 * @param io CLI I/O
 * @param lst file properties
 * @param indent number of spaces to indent
 */
void printFiles(AbstractCliIO* io, const QVariantList& lst, int indent)
{
  if (lst.isEmpty())
    return;

  for (const QVariant& var : lst) {
    const QVariantMap map = var.toMap();
    QString propsStr = map.value(QLatin1String("selected")).toBool()
        ? QLatin1String(">") : QLatin1String(" ");
    propsStr +=
        (map.value(QLatin1String("changed")).toBool() ? QLatin1String("*") : QLatin1String(" "));
    if (map.contains(QLatin1String("tags"))) {
      const QVariantList tags = map.value(QLatin1String("tags")).toList();
      FOR_ALL_TAGS(tagNr) {
        propsStr += tags.contains(1 + tagNr)
            ? QLatin1Char('1' + tagNr) : QLatin1Char('-');
      }
    } else {
      propsStr += QString(Frame::Tag_NumValues, QLatin1Char(' '));
    }
    io->writeLine(propsStr + QString(indent, QLatin1Char(' ')) +
                    map.value(QLatin1String("fileName")).toString());
    if (map.contains(QLatin1String("files"))) {
      printFiles(io, map.value(QLatin1String("files")).toList(), indent + 2);
    }
  }
};

}
/** @endcond */


TextCliFormatter::TextCliFormatter(AbstractCliIO* io)
  : AbstractCliFormatter(io)
{
}

TextCliFormatter::~TextCliFormatter()
{
}

void TextCliFormatter::clear()
{
  m_errorMessage.clear();
  m_args.clear();
}

QStringList TextCliFormatter::parseArguments(const QString& line)
{
  m_errorMessage.clear();
  m_args = splitArgs(line);
  return m_args;
}

QString TextCliFormatter::getErrorMessage() const
{
  return m_errorMessage;
}

bool TextCliFormatter::isIncomplete() const
{
  return false;
}

bool TextCliFormatter::isFormatRecognized() const
{
  return !m_args.isEmpty();
}

void TextCliFormatter::writeError(CliError errorCode)
{
  QString errorMsg;
  switch (errorCode) {
  case CliError::MethodNotFound:
#if QT_VERSION >= 0x050600
    errorMsg = tr("Unknown command '%1'. Type 'help' for help.")
        .arg(m_args.isEmpty() ? QLatin1String("") : m_args.constFirst());
#else
    errorMsg = tr("Unknown command '%1'. Type 'help' for help.")
        .arg(m_args.isEmpty() ? QLatin1String("") : m_args.first());
#endif
    break;
  default:
    ;
  }
  if (!errorMsg.isEmpty()) {
    writeError(errorMsg);
  }
}

void TextCliFormatter::writeError(const QString& msg)
{
  io()->writeErrorLine(msg);
}

void TextCliFormatter::writeError(const QString& msg, CliError errorCode)
{
  if (errorCode == CliError::Usage) {
    io()->writeLine(tr("Usage:"));
  }
  writeError(msg);
}

void TextCliFormatter::writeResult(const QString& str)
{
  io()->writeLine(str);
}

/**
 * Write result message.
 * @param strs result as string list
 */
void TextCliFormatter::writeResult(const QStringList& strs)
{
  for (const QString& str : strs) {
    io()->writeLine(str);
  }
}

void TextCliFormatter::writeResult(const QVariantMap& map)
{
  for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
    const QString& key = it.key();
    if (key == QLatin1String("tags")) {
      QVariantList value = it.value().toList();
      QString tagStr;
      for (const QVariant& var : value) {
        if (!tagStr.isEmpty()) {
          tagStr += QLatin1String(", ");
        }
        tagStr += var.toString();
      }
      if (tagStr.isEmpty()) {
        tagStr = QLatin1String("-");
      }
      io()->writeLine(tr("Tags") + QLatin1String(": ") + tagStr);
    } else if (key == QLatin1String("taggedFile")) {
      QVariantMap value = it.value().toMap();
      QString m_detailInfo = value.value(QLatin1String("format")).toString();
      QString m_filename = value.value(QLatin1String("fileName")).toString();
      bool m_fileNameChanged = value.value(QLatin1String("fileNameChanged")).toBool();
      if (!m_detailInfo.isEmpty()) {
        io()->writeLine(tr("File") + QLatin1String(": ") + m_detailInfo);
      }
      if (!m_filename.isEmpty()) {
        QString line = m_fileNameChanged ? QLatin1String("*") : QLatin1String(" ");
        line += QLatin1Char(' ');
        line += tr("Name");
        line += QLatin1String(": ");
        line += m_filename;
        io()->writeLine(line);
      }
      FOR_ALL_TAGS(tagNr) {
        QString tagNrStr = Frame::tagNumberToString(tagNr);
        const QVariantMap tag = value.value(QLatin1String("tag") + tagNrStr).toMap();
        if (!tag.isEmpty()) {
          const QVariantList frames = tag.value(QLatin1String("frames")).toList();
          if (!frames.isEmpty()) {
            int maxLength = 0;
            for (const QVariant& var : frames) {
              QString name = var.toMap().value(QLatin1String("name")).toString();
              maxLength = qMax(name.size(), maxLength);
            }
            QString tagStr = tag.value(QLatin1String("format")).toString();
            if (!tagStr.isEmpty()) {
              tagStr.prepend(QLatin1Char(' '));
            }
            tagStr.prepend(QLatin1Char(':'));
            tagStr.prepend(tr("Tag %1").arg(tagNrStr));
            io()->writeLine(tagStr);
            for (const QVariant& var : frames) {
              QString name = var.toMap().value(QLatin1String("name")).toString();
              QString value = var.toMap().value(QLatin1String("value")).toString();
              bool changed =  var.toMap().value(QLatin1String("changed")).toBool();
              QString line = changed ? QLatin1String("*") : QLatin1String(" ");
              line += QLatin1Char(' ');
              line += name;
              line += QString(maxLength - name.size() + 2,
                              QLatin1Char(' '));
              line += value;
              io()->writeLine(line);
            }
          }
        }
      }
    } else if (key == QLatin1String("files")) {
      printFiles(io(), it.value().toList(), 1);
    } else if (key == QLatin1String("timeout")) {
      QString value = it.value().toString();
      io()->writeLine(tr("Timeout") % QLatin1String(": ") % value);
    } else if (key == QLatin1String("event")) {
      QVariantMap value = it.value().toMap();
      QString type = value.value(QLatin1String("type")).toString();
      QString eventText;
      if (type == QLatin1String("readingDirectory")) {
        eventText = tr("Reading Directory");
      } else if (type == QLatin1String("started")) {
        eventText = tr("Started");
      } else if (type == QLatin1String("source")) {
        eventText = tr("Source");
      } else if (type == QLatin1String("querying")) {
        eventText = tr("Querying");
      } else if (type == QLatin1String("fetching")) {
        eventText = tr("Fetching");
      } else if (type == QLatin1String("dataReceived")) {
        eventText = tr("Data received");
      } else if (type == QLatin1String("cover")) {
        eventText = tr("Cover");
      } else if (type == QLatin1String("finished")) {
        eventText = tr("Finished");
      } else if (type == QLatin1String("aborted")) {
        eventText = tr("Aborted");
      } else if (type == QLatin1String("error")) {
        eventText = tr("Error");
      } else if (type == QLatin1String("parseError")) {
        eventText = QLatin1String("parse error");
      } else {
        eventText = type;
      }
      QVariant data = value.value(QLatin1String("data"));
      if (data.type() == QVariant::String) {
        QString text = data.toString();
        if (!text.isEmpty()) {
          if (type == QLatin1String("filterEntered")) {
            eventText = QLatin1String("  ") + text;
          } else if (type == QLatin1String("filterPassed")) {
            eventText = QLatin1String("+ ") + text;
          } else if (type == QLatin1String("filteredOut")) {
            eventText = QLatin1String("- ") + text;
          } else {
            eventText += QLatin1String(": ");
            eventText += text;
          }
        }
      } else if (data.type() == QVariant::Map) {
        // Event maps with source and destination are used with
        // rename directory events.
        QVariantMap dataMap = data.toMap();
        if (dataMap.contains(QLatin1String("source"))) {
          eventText += QLatin1String("  ");
          eventText += dataMap.value(QLatin1String("source")).toString();
        }
        if (dataMap.contains(QLatin1String("destination"))) {
          eventText += QLatin1String("\n  ");
          eventText += dataMap.value(QLatin1String("destination")).toString();
        }
      }
      io()->writeLine(eventText);
    }
  }
}

void TextCliFormatter::finishWriting()
{
}
