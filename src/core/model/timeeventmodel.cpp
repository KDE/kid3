/**
 * \file timeeventmodel.cpp
 * Time event model (synchronized lyrics or event timing codes).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Mar 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include "timeeventmodel.h"
#include <QGuiApplication>
#include <QTextStream>
#include <QPalette>
#include <QBrush>
#include "eventtimingcode.h"

/**
 * Constructor.
 * @param parent parent widget
 */
TimeEventModel::TimeEventModel(QObject* parent)
  : QAbstractTableModel(parent), m_type(SynchronizedLyrics), m_markedRow(-1),
    m_guiApp(qobject_cast<QGuiApplication*>(QCoreApplication::instance())
             != nullptr)
{
  setObjectName(QLatin1String("TimeEventModel"));
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags TimeEventModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid())
    theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  return theFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant TimeEventModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= m_timeEvents.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return QVariant();
  const TimeEvent& timeEvent = m_timeEvents.at(index.row());
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == CI_Time)
      return timeEvent.time;
    else
      return timeEvent.data;
  } else if (role == Qt::BackgroundColorRole && index.column() == CI_Data) {
    return index.row() != m_markedRow ? Qt::NoBrush
                                   : m_guiApp ? QGuiApplication::palette().mid()
                                              : QBrush(Qt::gray);
  }
  return QVariant();
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool TimeEventModel::setData(const QModelIndex& index,
                             const QVariant& value, int role)
{
  if (!index.isValid() || role != Qt::EditRole ||
      index.row() < 0 || index.row() >= m_timeEvents.size() ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return false;
  TimeEvent& timeEvent = m_timeEvents[index.row()]; // clazy:exclude=detaching-member
  if (index.column() == CI_Time) {
    timeEvent.time = value.toTime();
  } else {
    timeEvent.data = value;
  }
  emit dataChanged(index, index);
  return true;
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant TimeEventModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal && section < CI_NumColumns) {
    if (section == CI_Time) {
      return tr("Time");
    } else if (m_type == EventTimingCodes) {
      return tr("Event Code");
    } else {
      return tr("Text");
    }
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index, invalid for table models
 * @return number of rows,
 * if parent is valid number of children (0 for table models)
 */
int TimeEventModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_timeEvents.size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int TimeEventModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : CI_NumColumns;
}

/**
 * Insert rows.
 * @param row rows are inserted before this row, if 0 at the begin,
 * if rowCount() at the end
 * @param count number of rows to insert
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool TimeEventModel::insertRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_timeEvents.insert(row, TimeEvent(QTime(), QVariant()));
    endInsertRows();
  }
  return true;
}

/**
 * Remove rows.
 * @param row rows are removed starting with this row
 * @param count number of rows to remove
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool TimeEventModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
      m_timeEvents.removeAt(row);
    endRemoveRows();
  }
  return true;
}

/**
 * Set the model from a list of time events.
 * @param events list of time events
 */
void TimeEventModel::setTimeEvents(const QList<TimeEvent>& events)
{
  beginResetModel();
  m_timeEvents = events;
  endResetModel();
}

/**
 * Get time event list from the model.
 * @return list of time events.
 */
QList<TimeEventModel::TimeEvent> TimeEventModel::getTimeEvents() const
{
  return m_timeEvents;
}

/**
 * Set the model from a SYLT frame.
 * @param fields ID3v2 SYLT frame fields
 */
void TimeEventModel::fromSyltFrame(const Frame::FieldList& fields)
{
  QVariantList synchedData;
  bool unitIsFrames = false;
  for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
    const Frame::Field& fld = *it;
    if (fld.m_id == Frame::ID_TimestampFormat) {
      unitIsFrames = fld.m_value.toInt() == 1;
    } else if (fld.m_value.type() == QVariant::List) {
      synchedData = fld.m_value.toList();
    }
  }

  bool newLinesStartWithLineBreak = false;
  QList<TimeEvent> timeEvents;
  QListIterator<QVariant> it(synchedData);
  while (it.hasNext()) {
    quint32 milliseconds = it.next().toUInt();
    if (!it.hasNext())
      break;

    QString str = it.next().toString();
    if (timeEvents.isEmpty() && str.startsWith(QLatin1Char('\n'))) {
      // The first entry determines if new lines have to start with a new line
      // character or if all entries are supposed to be new lines.
      newLinesStartWithLineBreak = true;
    }

    bool isNewLine = !newLinesStartWithLineBreak;
    if (str.startsWith(QLatin1Char('\n'))) {
      // New lines start with a new line character, which is removed.
      isNewLine = true;
      str.remove(0, 1);
    }
    if (isNewLine) {
      // If the resulting line starts with one of the special characters
      // (' ', '-', '_'), it is escaped with '#'.
      if (str.length() > 0) {
        QChar ch = str.at(0);
        if (ch == QLatin1Char(' ') || ch == QLatin1Char('-') ||
            ch == QLatin1Char('_')) {
          str.prepend(QLatin1Char('#'));
        }
      }
    } else if (!(str.startsWith(QLatin1Char(' ')) ||
                 str.startsWith(QLatin1Char('-')))) {
      // Continuations of the current line do not start with a new line
      // character. They must start with ' ' or '-'. If the line starts with
      // another character, it is escaped with '_'.
      str.prepend(QLatin1Char('_'));
    }

    QVariant timeStamp = unitIsFrames
        ? QVariant(milliseconds)
        : QVariant(QTime(0, 0).addMSecs(milliseconds));
    timeEvents.append(TimeEvent(timeStamp, str));
  }
  setTimeEvents(timeEvents);
}

/**
 * Get the model as a SYLT frame.
 * @param fields ID3v2 SYLT frame fields to set
 */
void TimeEventModel::toSyltFrame(Frame::FieldList& fields) const
{
  auto timeStampFormatIt = fields.end();
  auto dataIt = fields.end();
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    if (it->m_id == Frame::ID_TimestampFormat) {
      timeStampFormatIt = it;
    } else if (it->m_value.type() == QVariant::List) {
      dataIt = it;
    }
  }

  QVariantList synchedData;
  bool hasMsTimeStamps = false;
  const auto timeEvents = m_timeEvents;
  for (const TimeEvent& timeEvent : timeEvents) {
    if (!timeEvent.time.isNull()) {
      QString str = timeEvent.data.toString();
      // Remove escaping, restore new line characters.
      if (str.startsWith(QLatin1Char('_'))) {
        str.remove(0, 1);
      } else if (str.startsWith(QLatin1Char('#'))) {
        str.replace(0, 1, QLatin1Char('\n'));
      } else if (!(str.startsWith(QLatin1Char(' ')) ||
                   str.startsWith(QLatin1Char('-')))) {
        str.prepend(QLatin1Char('\n'));
      }

      quint32 milliseconds;
      if (timeEvent.time.type() == QVariant::Time) {
        hasMsTimeStamps = true;
        milliseconds = QTime(0, 0).msecsTo(timeEvent.time.toTime());
      } else {
        milliseconds = timeEvent.data.toUInt();
      }
      synchedData.append(milliseconds);
      synchedData.append(str);
    }
  }

  if (hasMsTimeStamps && timeStampFormatIt != fields.end()) {
    timeStampFormatIt->m_value = 2;
  }
  if (dataIt != fields.end()) {
    dataIt->m_value = synchedData;
  }
}

/**
 * Set the model from a ETCO frame.
 * @param fields ID3v2 ETCO frame fields
 */
void TimeEventModel::fromEtcoFrame(const Frame::FieldList& fields)
{
  QVariantList synchedData;
  bool unitIsFrames = false;
  for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
    const Frame::Field& fld = *it;
    if (fld.m_id == Frame::ID_TimestampFormat) {
      unitIsFrames = fld.m_value.toInt() == 1;
    } else if (fld.m_value.type() == QVariant::List) {
      synchedData = fld.m_value.toList();
    }
  }

  QList<TimeEvent> timeEvents;
  QListIterator<QVariant> it(synchedData);
  while (it.hasNext()) {
    quint32 milliseconds = it.next().toUInt();
    if (!it.hasNext())
      break;

    int code = it.next().toInt();
    QVariant timeStamp = unitIsFrames
        ? QVariant(milliseconds)
        : QVariant(QTime(0, 0).addMSecs(milliseconds));
    timeEvents.append(TimeEvent(timeStamp, code));
  }
  setTimeEvents(timeEvents);
}

/**
 * Get the model as an ETCO frame.
 * @param fields ID3v2 ETCO frame fields to set
 */
void TimeEventModel::toEtcoFrame(Frame::FieldList& fields) const
{
  auto timeStampFormatIt = fields.end();
  auto dataIt = fields.end();
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    if (it->m_id == Frame::ID_TimestampFormat) {
      timeStampFormatIt = it;
    } else if (it->m_value.type() == QVariant::List) {
      dataIt = it;
    }
  }

  QVariantList synchedData;
  bool hasMsTimeStamps = false;
  const auto timeEvents = m_timeEvents;
  for (const TimeEvent& timeEvent : timeEvents) {
    if (!timeEvent.time.isNull()) {
      int code = timeEvent.data.toInt();

      quint32 milliseconds;
      if (timeEvent.time.type() == QVariant::Time) {
        hasMsTimeStamps = true;
        milliseconds = QTime(0, 0).msecsTo(timeEvent.time.toTime());
      } else {
        milliseconds = timeEvent.data.toUInt();
      }
      synchedData.append(milliseconds);
      synchedData.append(code);
    }
  }

  if (timeStampFormatIt != fields.end() && hasMsTimeStamps) {
    timeStampFormatIt->m_value = 2;
  }
  if (dataIt != fields.end()) {
    dataIt->m_value = synchedData;
  }
}

/**
 * Mark row for a time stamp.
 * Marks the first row with time >= @a timeStamp.
 * @param timeStamp time
 * @see getMarkedRow()
 */
void TimeEventModel::markRowForTimeStamp(const QTime& timeStamp)
{
  int row = 0, oldRow = m_markedRow, newRow = -1;
  for (auto it = m_timeEvents.constBegin(); it != m_timeEvents.constEnd(); ++it) {
    const TimeEvent& timeEvent = *it;
    QTime time = timeEvent.time.toTime();
    if (time.isValid() && time >= timeStamp) {
      if (timeStamp.msecsTo(time) > 1000 && row > 0) {
        --row;
      }
      if (row == 0 && timeStamp == QTime(0, 0) &&
          m_timeEvents.at(0).time.toTime() != timeStamp) {
        row = -1;
      }
      newRow = row;
      break;
    }
    ++row;
  }
  if (newRow != oldRow &&
      !(newRow == -1 && oldRow == m_timeEvents.size() - 1)) {
    m_markedRow = newRow;
    if (oldRow != -1) {
      QModelIndex idx = index(oldRow, CI_Data);
      emit dataChanged(idx, idx);
    }
    if (newRow != -1) {
      QModelIndex idx = index(newRow, CI_Data);
      emit dataChanged(idx, idx);
    }
  }
}

/**
 * Clear the marked row.
 */
void TimeEventModel::clearMarkedRow()
{
  if (m_markedRow != -1) {
    QModelIndex idx = index(m_markedRow, CI_Data);
    m_markedRow = -1;
    emit dataChanged(idx, idx);
  }
}

/**
 * Set the model from an LRC file.
 * @param stream LRC file stream
 */
void TimeEventModel::fromLrcFile(QTextStream& stream)
{
  QRegExp timeStampRe(QLatin1String(
                        R"(([[<])(\d\d):(\d\d)(?:\.(\d{1,3}))?([\]>]))"));
  QList<TimeEvent> timeEvents;
  bool isFirstLine = true;
  forever {
    QString line = stream.readLine();
    if (line.isNull())
      break;

    if (line.isEmpty())
      continue;

    // If the first line does not contain a '[' character, assume that this is
    // not an LRC file and only import lines without time stamps.
    if (isFirstLine) {
      if (line.contains(QLatin1Char('['))) {
        isFirstLine = false;
      } else {
        stream.seek(0);
        fromTextFile(stream);
        return;
      }
    }

    QList<QTime> emptyEvents;
    char firstChar = '\0';
    int pos = timeStampRe.indexIn(line, 0);
    while (pos != -1) {
      bool newLine = timeStampRe.cap(1) == QLatin1String("[");
      QString millisecondsStr = timeStampRe.cap(4);
      int milliseconds = millisecondsStr.toInt();
      if (millisecondsStr.length() == 2) {
        milliseconds *= 10;
      } else if (millisecondsStr.length() == 1) {
        milliseconds *= 100;
      }
      QTime timeStamp(0,
                      timeStampRe.cap(2).toInt(),
                      timeStampRe.cap(3).toInt(),
                      milliseconds);
      int textBegin = pos + timeStampRe.matchedLength();

      pos = timeStampRe.indexIn(line, textBegin);
      int textLen = pos != -1 ? pos - textBegin : -1;
      QString str = line.mid(textBegin, textLen);
      if (m_type == EventTimingCodes) {
        EventTimeCode etc =
            EventTimeCode::fromString(str.toLatin1().constData());
        if (etc.isValid()) {
          timeEvents.append(TimeEvent(timeStamp, etc.getCode()));
        }
      } else {
        if (firstChar != '\0') {
          str.prepend(QChar::fromLatin1(firstChar));
          firstChar = '\0';
        }
        if (newLine) {
          if (str.startsWith(QLatin1Char(' ')) ||
              str.startsWith(QLatin1Char('-')) ||
              str.startsWith(QLatin1Char('_'))) {
            str.prepend(QLatin1Char('#'));
          }
        } else if (!(str.startsWith(QLatin1Char(' ')) ||
                     str.startsWith(QLatin1Char('-')))) {
          str.prepend(QLatin1Char('_'));
        }
        if (pos != -1) {
          if (timeStampRe.cap(1) == QLatin1String("<")) {
            if (str.endsWith(QLatin1Char(' ')) ||
                str.endsWith(QLatin1Char('-'))) {
              firstChar = str.at(str.length() - 1).toLatin1();
              str.truncate(str.length() - 1);
            }
          }
          if (str.isEmpty()) {
            // The next time stamp follows immediately with a common text.
            emptyEvents.append(timeStamp);
            continue;
          }
        }
        const auto times = emptyEvents;
        for (const QTime& time : times) {
          timeEvents.append(TimeEvent(time, str));
        }
        timeEvents.append(TimeEvent(timeStamp, str));
      }
    }
  }
  qSort(timeEvents);
  setTimeEvents(timeEvents);
}

/**
 * Set the model from a text file.
 * @param stream text file stream
 */
void TimeEventModel::fromTextFile(QTextStream& stream)
{
  QList<TimeEvent> timeEvents;
  forever {
    QString line = stream.readLine();
    if (line.isNull())
      break;

    timeEvents.append(TimeEvent(QTime(), line));
  }
  setTimeEvents(timeEvents);
}

/**
 * Store the model in an LRC file.
 * @param stream LRC file stream
 * @param title optional title
 * @param artist optional artist
 * @param album optional album
 */
void TimeEventModel::toLrcFile(QTextStream& stream, const QString& title,
                               const QString& artist, const QString& album)
{
  bool atBegin = true;
  if (!title.isEmpty()) {
    stream << QLatin1String("[ti:") << title << QLatin1String("]\r\n");
    atBegin = false;
  }
  if (!artist.isEmpty()) {
    stream << QLatin1String("[ar:") << artist << QLatin1String("]\r\n");
    atBegin = false;
  }
  if (!album.isEmpty()) {
    stream << QLatin1String("[al:") << album << QLatin1String("]\r\n");
    atBegin = false;
  }
  const auto timeEvents = m_timeEvents;
  for (const TimeEvent& timeEvent : timeEvents) {
    QTime time = timeEvent.time.toTime();
    if (time.isValid()) {
      char firstChar = '\0';
      bool newLine = true;
      QString str;
      if (m_type == EventTimingCodes) {
        str = EventTimeCode(timeEvent.data.toInt()).toString();
      } else {
        str = timeEvent.data.toString();
        if (str.startsWith(QLatin1Char('_'))) {
          str.remove(0, 1);
          newLine = false;
        } else if (str.startsWith(QLatin1Char('#'))) {
          str.remove(0, 1);
        } else if (str.startsWith(QLatin1Char(' ')) ||
                   str.startsWith(QLatin1Char('-'))) {
          firstChar = str.at(0).toLatin1();
          str.remove(0, 1);
          newLine = false;
        }
      }

      if (newLine) {
        if (!atBegin) {
          stream << QLatin1String("\r\n");
        }
        stream << QLatin1Char('[') << timeStampToString(time).toLatin1()
               << QLatin1Char(']') << str.toLatin1();
      } else {
        if (firstChar != '\0') {
          stream << firstChar;
        }
        stream << QLatin1Char('<') << timeStampToString(time).toLatin1()
               << QLatin1Char('>') << str.toLatin1();
      }
      atBegin = false;
    }
  }
  if (!atBegin) {
    stream << QLatin1String("\r\n");
  }
}

/**
 * Format a time suitable for a time stamp.
 * @param time time stamp
 * @return string of the format "mm:ss.zz"
 */
QString TimeEventModel::timeStampToString(const QTime& time)
{
  int hour = time.hour();
  int min = time.minute();
  int sec = time.second();
  int msec = time.msec();
  if (hour < 0) hour = 0;
  if (min < 0)  min = 0;
  if (sec < 0)  sec = 0;
  if (msec < 0) msec = 0;
  QString text = QString(QLatin1String("%1:%2.%3"))
      .arg(min, 2, 10, QLatin1Char('0'))
      .arg(sec, 2, 10, QLatin1Char('0'))
      .arg(msec / 10, 2, 10, QLatin1Char('0'));
  if (hour != 0) {
    text.prepend(QString::number(hour) + QLatin1Char(':'));
  }
  return text;
}
