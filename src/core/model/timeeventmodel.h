/**
 * \file timeeventmodel.h
 * Time event model (synchronized lyrics or event timing codes).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Mar 2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#include <QAbstractTableModel>
#include "frame.h"
#include "kid3api.h"

class QTextStream;
class CoreTaggedFileIconProvider;

/**
 * Time event model.
 */
class KID3_CORE_EXPORT TimeEventModel : public QAbstractTableModel {
  Q_OBJECT
public:
  /** Type of data. */
  enum Type {
    SynchronizedLyrics, /**< Synchronized lyrics text */
    EventTimingCodes    /**< Event codes */
  };

  /** Column indexes. */
  enum ColumnIndex {
    CI_Time,
    CI_Data,
    CI_NumColumns
  };

  /** Time and data. */
  struct TimeEvent {
    /** Constructor. */
    TimeEvent() = default;
    /** Constructor. */
    TimeEvent(const QVariant& t, const QVariant& d) : time(t), data(d) {}
    /**
     * Time from the beginning of the file (ms, stored as qulonglong) or
     * frame integer (number, stored as uint). The exact QVariant type
     * determines the time stamp format, as it is used in SYLT frames.
     */
    QVariant time;
    QVariant data; /**< String (lyrics) or integer (event codes) */

    /**
     * Less than operator.
     * @param rhs right hand side to compare
     * @return true if this < rhs.
     */
    bool operator<(const TimeEvent& rhs) const {
      return time.toULongLong() < rhs.time.toULongLong();
    }

    bool isTimeInFrames() const
    {
#if QT_VERSION >= 0x060000
      return time.typeId() == QMetaType::UInt;
#else
      return time.type() == QVariant::UInt;
#endif
    }
    quint32 timeInMs() const { return static_cast<quint32>(time.toULongLong()); }
    void setTimeInMs(quint32 ms) { time.setValue(static_cast<qulonglong>(ms)); }
    uint timeInFrames() const { return time.toUInt(); }
    void setTimeInFrames(uint frames) { time.setValue(frames); }
  };

  /**
   * Constructor.
   * @param colorProvider color provider
   * @param parent parent widget
   */
  explicit TimeEventModel(CoreTaggedFileIconProvider* colorProvider = nullptr,
                          QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~TimeEventModel() override = default;

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  /**
   * Set data for header section.
   * Not supported.
   * @return false
   */
  bool setHeaderData(int, Qt::Orientation, const QVariant&,
                     int = Qt::EditRole) override { return false; }

  /**
   * Get number of rows.
   * @param parent parent model index, invalid for table models
   * @return number of rows,
   * if parent is valid number of children (0 for table models)
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Get number of columns.
   * @param parent parent model index, invalid for table models
   * @return number of columns,
   * if parent is valid number of children (0 for table models)
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Insert rows.
   * @param row rows are inserted before this row, if 0 at the begin,
   * if rowCount() at the end
   * @param count number of rows to insert
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  bool insertRows(int row, int count,
                  const QModelIndex& parent = QModelIndex()) override;

  /**
   * Remove rows.
   * @param row rows are removed starting with this row
   * @param count number of rows to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  bool removeRows(int row, int count,
                  const QModelIndex& parent = QModelIndex()) override;

  /**
   * Set type of data.
   * @param type SynchronizedLyrics (default) or EventTimingCodes
   */
  void setType(Type type) { m_type = type; }

  /**
   * Get type of data.
   * @return SynchronizedLyrics or EventTimingCodes.
   */
  Type getType() const { return m_type; }

  /**
   * Set the model from a list of time events.
   * @param events list of time events
   */
  void setTimeEvents(const QList<TimeEvent>& events);

  /**
   * Get time event list from the model.
   * @return list of time events.
   */
  QList<TimeEvent> getTimeEvents() const;

  /**
   * Get marked row.
   * @return row number.
   */
  int getMarkedRow() const { return m_markedRow; }

  /**
   * Mark row for a time stamp.
   * Marks the first row with time >= @a timeStamp.
   * @param timeStamp time in ms
   * @see getMarkedRow()
   */
  void markRowForTimeStamp(qint64 timeStamp);

  /**
   * Clear the marked row.
   */
  void clearMarkedRow();

  /**
   * Set the model from a SYLT frame.
   * @param fields ID3v2 SYLT frame fields
   */
  void fromSyltFrame(const Frame::FieldList& fields);

  /**
   * Get the model as a SYLT frame.
   * @param fields ID3v2 SYLT frame fields to set
   */
  void toSyltFrame(Frame::FieldList& fields) const;

  /**
   * Set the model from a ETCO frame.
   * @param fields ID3v2 ETCO frame fields
   */
  void fromEtcoFrame(const Frame::FieldList& fields);

  /**
   * Get the model as an ETCO frame.
   * @param fields ID3v2 ETCO frame fields to set
   */
  void toEtcoFrame(Frame::FieldList& fields) const;

  /**
   * Set the model from an LRC file.
   * @param stream LRC file stream
   */
  void fromLrcFile(QTextStream& stream);

  /**
   * Store the model in an LRC file.
   * @param stream LRC file stream
   * @param title optional title
   * @param artist optional artist
   * @param album optional album
   */
  void toLrcFile(QTextStream& stream,
                 const QString& title = QString(),
                 const QString& artist = QString(),
                 const QString& album = QString()) const;

  /**
   * Format a time suitable for a time stamp.
   * @param timeMs time stamp in milliseconds
   * @return string of the format "hh:mm:ss.zzz"
   */
  static QString timeStampToString(unsigned int timeMs);

  /**
   * Convert the string representation of a time stamp to milliseconds.
   * @param timeStr time stamp in the format "hh:mm:ss.zzz"
   * @return time stamp as milliseconds.
   */
  static unsigned int timeStampFromString(const QString& timeStr);

  /**
   * Format a time suitable for a time stamp.
   * @param timeData time stamp in milliseconds in a QVariant
   * @return string of the format "hh:mm:ss.zzz"
   */
  static QString timeStampDataToString(const QVariant& timeData);

  /**
   * Convert the string representation of a time stamp to milliseconds.
   * @param timeStr time stamp in the format "hh:mm:ss.zzz"
   * @return time stamp as milliseconds in a QVariant.
   */
  static QVariant timeStampDataFromString(const QString& timeStr);

  /**
   * Check if a time event time value has frames as unit.
   * @param time TimeEvent time value
   * @return true if value is frame number, else time stamp in ns.
   */
  static bool isTimeEventInFrames(const QVariant& time)
  {
#if QT_VERSION >= 0x060000
    return time.typeId() == QMetaType::UInt;
#else
    return time.type() == QVariant::UInt;
#endif
  }

private:
  /**
   * Set the model from a text file.
   * @param stream text file stream
   */
  void fromTextFile(QTextStream& stream);

  Type m_type;
  int m_markedRow;
  QList<TimeEvent> m_timeEvents;
  CoreTaggedFileIconProvider* m_colorProvider;
};
