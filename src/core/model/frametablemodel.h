/**
 * \file frametablemodel.h
 * Model for table with frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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
#include <QVector>
#include <QSet>
#include <QBitArray>
#include "frame.h"
#include "kid3api.h"

class CoreTaggedFileIconProvider;

/**
 * Model for table with frames.
 */
class KID3_CORE_EXPORT FrameTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  /** Custom role. */
  enum Roles {
    FrameTypeRole = Qt::UserRole + 1,
    NameRole = Qt::UserRole + 2,
    ValueRole =  Qt::UserRole + 3,
    ModifiedRole =  Qt::UserRole + 4,
    TruncatedRole =  Qt::UserRole + 5,
    InternalNameRole =  Qt::UserRole + 6,
    FieldIdsRole =  Qt::UserRole + 7,
    FieldValuesRole =  Qt::UserRole + 8,
    CompletionsRole =  Qt::UserRole + 9,
    NoticeRole =  Qt::UserRole + 10,
    NoticeWarningRole =  Qt::UserRole + 11
  };

  /** Column indices. */
  enum ColumnIndex {
    CI_Enable,
    CI_Value,
    CI_NumColumns
  };

  /**
   * Constructor.
   * @param id3v1  true if model for ID3v1 frames
   * @param colorProvider colorProvider
   * @param parent parent widget
   */
  FrameTableModel(bool id3v1, CoreTaggedFileIconProvider* colorProvider,
                  QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~FrameTableModel() override = default;

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
   * Remove rows.
   * @param row rows are removed starting with this row
   * @param count number of rows to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  bool removeRows(int row, int count,
                  const QModelIndex& parent = QModelIndex()) override;

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
   * Map role identifiers to role property names in scripting languages.
   * @return hash mapping role identifiers to names.
   */
  QHash<int,QByteArray> roleNames() const override;

  /**
   * Insert a frame.
   * @param frame frame to insert
   */
  void insertFrame(const Frame& frame);

  /**
   * Check if model is for ID3v1 frames.
   * @return true if for ID3v1.
   */
  bool isId3v1() const { return m_id3v1; }

  /**
   * Mark rows.
   * @param rowMask mask with bits of rows to mark
   */
  void markRows(quint64 rowMask);


  /**
   * Mark changed frames.
   * @param types frame types to mark
   */
  void markChangedFrames(const QList<Frame::ExtendedType>& types);

  /**
   * Get frame for index.
   * @param index model index
   * @return frame, 0 if no frame.
   */
  const Frame* getFrameOfIndex(const QModelIndex& index) const;

  /**
   * Get row with frame with a specific frame index.
   * @param index frame index
   * @return row number, -1 if not found.
   */
  int getRowWithFrameIndex(int index) const;

  /**
   * Get row with frame with a specific frame name.
   * @param name name of frame
   * @return row number, -1 if not found.
   */
  int getRowWithFrameName(const QString& name) const;

  /**
   * Get filter with enabled frames.
   *
   * @param allDisabledToAllEnabled true to enable all if all are disabled
   *
   * @return filter with enabled frames.
   */
  FrameFilter getEnabledFrameFilter(bool allDisabledToAllEnabled = false) const;

  /**
   * Set the check state of all frames in the table.
   *
   * @param checked true to check the frames
   */
  void setAllCheckStates(bool checked);

  /**
   * Get reference to frame collection.
   * @return frame collection.
   */
  const FrameCollection& frames() const { return m_frames; }

  /**
   * Get enabled frames.
   * @return frame collection with enabled frames.
   */
  FrameCollection getEnabledFrames() const;

  /**
   * Clear frame collection.
   */
  void clearFrames();

  /**
   * Transfer frames to frame collection.
   * @param src frames to move into frame collection, will be cleared
   */
  void transferFrames(FrameCollection& src);

  /**
   * Start filtering different values.
   */
  void beginFilterDifferent();

  /**
   * Set values which are different inactive.
   *
   * @param others frames to compare, will be modified
   */
  void filterDifferent(FrameCollection& others);

  /**
   * End filtering different values.
   */
  void endFilterDifferent();

  /**
   * Get the different values which have been filtered for a frame type.
   * @param type frame type
   * @return different values.
   */
  QSet<QString> getCompletionsForType(Frame::ExtendedType type) const;

  /**
   * Set if headerData() shall return empty values.
   * This can be used to avoid having text fragments visible in the header of
   * frame table columns.
   * @param empty true to use empty values, default is false
   */
  void setHeadersEmpty(bool empty) { m_emptyHeaders = empty; }
  bool headersEmpty() const { return m_emptyHeaders; }

public slots:
  /**
   * Select all frames in the table.
   */
  void selectAllFrames();

  /**
   * Deselect all frames in the table.
   */
  void deselectAllFrames();

  /**
   * Select changed frames in the table.
   */
  void selectChangedFrames();

  /**
   * Set order of frames in frame table.
   * @param frameTypes ordered sequence of frame types
   * @see TagConfig::quickAccessFrameOrder().
   */
  void setFrameOrder(const QList<int>& frameTypes);

private:
  /**
   * Get the frame at a specific position in the collection.
   * @param row position of frame
   * @return iterator to frame
   */
  FrameCollection::iterator frameAt(int row) const;

  /**
   * Get the row corresponding to a frame iterator.
   * @param frameIt frame iterator
   * @return row number, number of rows if not found.
   */
  int rowOf(FrameCollection::iterator frameIt) const;

  /**
   * Resize the bit array with the frame selection to match the frames size.
   */
  void resizeFrameSelected();

  /**
   * Update the frame to row mapping.
   */
  void updateFrameRowMapping();

  QBitArray m_frameSelected;
  quint64 m_markedRows;
  quint64 m_changedFrames;
  QSet<QString> m_changedOtherFrameNames;
  FrameCollection m_frames;
  QVector<FrameCollection::iterator> m_frameOfRow;
  QHash<Frame::ExtendedType, QSet<QString>> m_differentValues;
  QVector<int> m_frameTypeSeqNr;
  CoreTaggedFileIconProvider* m_colorProvider;
  bool m_id3v1;
  bool m_emptyHeaders;
};
