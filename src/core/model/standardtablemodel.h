/**
 * \file standardtablemodel.h
 * Table model with containing values for multiple roles.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Jul 2019
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

#include <QAbstractTableModel>
#include "kid3api.h"

/**
 * Table model with containing values for multiple roles.
 */
class KID3_CORE_EXPORT StandardTableModel : public QAbstractTableModel {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit StandardTableModel(QObject* parent = nullptr);

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

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
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param value header data
   * @param role item data role
   * @return true if ok.
   */
  virtual bool setHeaderData(int section, Qt::Orientation orientation,
                             const QVariant& value,
                             int role = Qt::EditRole) override;

  /**
   * Get supported drop actions.
   * @return supported drop actions.
   */
  virtual Qt::DropActions supportedDropActions() const override;

  /**
   * Get number of rows.
   * @param parent parent model index
   * @return number of rows, if parent is valid number of children
   */
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
  override;

  /**
   * Get number of columns.
   * @param parent parent model index
   * @return number of columns for children of given \a parent
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
  override;

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
   * Set number of columns.
   * @param columns number of colums, default is 1
   */
  void setColumnCount(int columns);

  /**
   * Set horizontal header labels.
   * @param labels header data
   */
  void setHorizontalHeaderLabels(const QStringList& labels);

  /**
   * Clear all rows.
   * The number of columns and the header data are not affected.
   */
  void clear();

private:
  QVector<QString> m_horizontalHeaderLabels;
  QVector<QVector<QMap<int, QVariant>>> m_cont;
  int m_numColumns;
};
