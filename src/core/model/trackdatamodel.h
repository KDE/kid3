/**
 * \file trackdatamodel.h
 * Model for table with track data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 May 2011
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
#include <QList>
#include "trackdata.h"
#include "kid3api.h"

class CoreTaggedFileIconProvider;

/**
 * Model for table with track data.
 */
class KID3_CORE_EXPORT TrackDataModel : public QAbstractTableModel {
  Q_OBJECT
public:
  /** Additional track properties extending Frame::Type. */
  enum TrackProperties {
    FT_FirstTrackProperty = Frame::FT_UnknownFrame + 1,
    FT_FilePath = FT_FirstTrackProperty,
    FT_Duration,
    FT_ImportDuration,
    FT_FileName
  };

  /**
   * Constructor.
   * @param colorProvider colorProvider
   * @param parent parent widget
   */
  explicit TrackDataModel(CoreTaggedFileIconProvider* colorProvider,
                          QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~TrackDataModel() override = default;

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
   * Insert columns.
   * @param column columns are inserted before this column, if 0 at the begin,
   * if columnCount() at the end
   * @param count number of columns to insert
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  bool insertColumns(int column, int count,
                     const QModelIndex& parent = QModelIndex()) override;
  /**
   * Remove columns.
   * @param column columns are removed starting with this column
   * @param count number of columns to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  bool removeColumns(int column, int count,
                     const QModelIndex& parent = QModelIndex()) override;

  /**
   * Set the check state of all tracks in the table.
   *
   * @param checked true to check the tracks
   */
  void setAllCheckStates(bool checked);

  /**
   * Set time difference check configuration.
   *
   * @param enable  true to enable check
   * @param maxDiff maximum allowed time difference
   */
  void setTimeDifferenceCheck(bool enable, int maxDiff);

  /**
   * Calculate accuracy of imported track data.
   * @return accuracy in percent, -1 if unknown.
   */
  int calculateAccuracy() const;

  /**
   * Get frame for index.
   * @param index model index
   * @return frame, 0 if no frame.
   */
  const Frame* getFrameOfIndex(const QModelIndex& index) const;

  /**
   * Set track data.
   * @param trackDataVector track data
   */
  void setTrackData(const ImportTrackDataVector& trackDataVector);

  /**
   * Get track data.
   * @return track data
   */
  ImportTrackDataVector getTrackData() const;

  /**
   * Constant reference to track data.
   * @return track data
   */
  const ImportTrackDataVector& trackData() const { return m_trackDataVector; }

  /**
   * Get the frame type for a column.
   * @param column model column
   * @return frame type of Frame::Type or TrackDataModel::TrackProperties,
   *         -1 if column invalid.
   */
  int frameTypeForColumn(int column) const;

  /**
   * Get column for a frame type.
   * @param frameType frame type of Frame::Type or
   *                  TrackDataModel::TrackProperties.
   * @return model column, -1 if not found.
   */
  int columnForFrameType(int frameType) const;

private:
  ImportTrackDataVector m_trackDataVector;
  QList<Frame::ExtendedType> m_frameTypes;
  CoreTaggedFileIconProvider* m_colorProvider;
  int m_maxDiff;
  bool m_diffCheckEnabled;
};
