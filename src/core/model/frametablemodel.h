/**
 * \file frametablemodel.h
 * Model for table with frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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

#ifndef FRAMETABLEMODEL_H
#define FRAMETABLEMODEL_H

#include <QAbstractTableModel>
#include <QBitArray>
#include <QItemDelegate>
#include "frame.h"
#include "kid3api.h"

/**
 * Model for table with frames.
 */
class KID3_CORE_EXPORT FrameTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  /** Custom role. */
  enum Roles {
    FrameTypeRole = Qt::UserRole + 1
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
   * @param parent parent widget
   */
  explicit FrameTableModel(bool id3v1, QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~FrameTableModel();

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  virtual QVariant data(const QModelIndex& index,
                        int role=Qt::DisplayRole) const;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role=Qt::EditRole);

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role=Qt::DisplayRole) const;

  /**
   * Set data for header section.
   * Not supported.
   * @return false
   */
  virtual bool setHeaderData(int, Qt::Orientation, const QVariant&,
                             int=Qt::EditRole) { return false; }

  /**
   * Get number of rows.
   * @param parent parent model index, invalid for table models
   * @return number of rows,
   * if parent is valid number of children (0 for table models)
   */
  virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;

  /**
   * Get number of columns.
   * @param parent parent model index, invalid for table models
   * @return number of columns,
   * if parent is valid number of children (0 for table models)
   */
  virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;

  /**
   * Remove rows.
   * @param row rows are removed starting with this row
   * @param count number of rows to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool removeRows(int row, int count,
                          const QModelIndex& parent=QModelIndex());

  /**
   * Insert rows.
   * @param row rows are inserted before this row, if 0 at the begin,
   * if rowCount() at the end
   * @param count number of rows to insert
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool insertRows(int row, int count,
                          const QModelIndex& parent=QModelIndex());

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
  void markRows(quint64 rowMask) { m_markedRows = rowMask; }


  /**
   * Mark changed frames.
   * @param frameMask mask with bits of frame types to mark
   */
  void markChangedFrames(quint64 frameMask) {
    m_changedFrames = frameMask;
  }

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
   * Set values which are different inactive.
   *
   * @param others frames to compare, will be modified
   */
  void filterDifferent(FrameCollection& others);

  /**
   * Get a display representation of the a frame name.
   * For ID3v2-IDs with description, only the ID is returned.
   * Other non-empty strings are translated.
   *
   * @param str frame name
   *
   * @return display representation of name.
   */
  static QString getDisplayName(const QString& str);

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

private:
  /**
   * Get the frame at a specific position in the collection.
   * @param row position of frame
   * @return const iterator to frame
   */
  FrameCollection::const_iterator frameAt(int row) const;

  /**
   * Get the frame at a specific position in the collection.
   * @param row position of frame
   * @return iterator to frame
   */
  FrameCollection::iterator frameAt(int row);

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

  QBitArray m_frameSelected;
  quint64 m_markedRows;
  quint64 m_changedFrames;
  FrameCollection m_frames;
  bool m_id3v1;
};


/** Delegate for table widget items. */
class KID3_CORE_EXPORT FrameItemDelegate : public QItemDelegate {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent QTableView
   */
  explicit FrameItemDelegate(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~FrameItemDelegate();

  /**
   * Create an editor to edit the cells contents.
   * @param parent parent widget
   * @param option style
   * @param index  index of item
   * @return combo box editor widget.
   */
  virtual QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const;

  /**
   * Set data to be edited by the editor.
   * @param editor editor widget
   * @param index  index of item
   */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

  /**
   * Set model data supplied by editor.
   * @param editor editor widget
   * @param model  model
   * @param index  index of item
   */
  virtual void setModelData(
    QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

private slots:
  /**
   * Format text if enabled.
   * @param txt text to format and set in line edit
   */
  void formatTextIfEnabled(const QString& txt);

private:
  QValidator* m_trackNumberValidator;
};

#endif // FRAMETABLEMODEL_H
