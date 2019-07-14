/**
 * \file timeeventmodel.h
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

#pragma once

#include <QAbstractTableModel>
#include <QTime>
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
    TimeEvent(const QVariant& t, const QVariant& d) : time(t), data(d) {}
    QVariant time; /**< Time from the beginning of the file or frame integer. */
    QVariant data; /**< String (lyrics) or integer (event codes) */

    /**
     * Less than operator.
     * @param rhs right hand side to compare
     * @return true if this < rhs.
     */
    bool operator<(const TimeEvent& rhs) const {
      return time.toTime() < rhs.time.toTime();
    }
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
  virtual ~TimeEventModel() override = default;

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  virtual QVariant data(const QModelIndex& index,
                        int role = Qt::DisplayRole) const override;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role = Qt::EditRole) override;

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const override;

  /**
   * Set data for header section.
   * Not supported.
   * @return false
   */
  virtual bool setHeaderData(int, Qt::Orientation, const QVariant&,
                             int = Qt::EditRole) override { return false; }

  /**
   * Get number of rows.
   * @param parent parent model index, invalid for table models
   * @return number of rows,
   * if parent is valid number of children (0 for table models)
   */
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Get number of columns.
   * @param parent parent model index, invalid for table models
   * @return number of columns,
   * if parent is valid number of children (0 for table models)
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * Insert rows.
   * @param row rows are inserted before this row, if 0 at the begin,
   * if rowCount() at the end
   * @param count number of rows to insert
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool insertRows(int row, int count,
                          const QModelIndex& parent = QModelIndex()) override;

  /**
   * Remove rows.
   * @param row rows are removed starting with this row
   * @param count number of rows to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool removeRows(int row, int count,
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
   * @param timeStamp time
   * @see getMarkedRow()
   */
  void markRowForTimeStamp(const QTime& timeStamp);

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
                 const QString& album = QString());

  /**
   * Format a time suitable for a time stamp.
   * @param time time stamp
   * @return string of the format "mm:ss.zz"
   */
  static QString timeStampToString(const QTime& time);

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
