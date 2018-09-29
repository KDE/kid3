/**
 * \file starratingmappingsmodel.h
 * Star rating mappings configuration table model.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

#ifndef STARRATINGMAPPINGSMODEL_H
#define STARRATINGMAPPINGSMODEL_H

#include <QAbstractTableModel>
#include "kid3api.h"

/**
 * Star rating mappings configuration table model.
 */
class KID3_CORE_EXPORT StarRatingMappingsModel : public QAbstractTableModel {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit StarRatingMappingsModel(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~StarRatingMappingsModel() override;

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
                        int role=Qt::DisplayRole) const override;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role=Qt::EditRole) override;

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role=Qt::DisplayRole) const override;

  /**
   * Set data for header section.
   * Not supported.
   * @return false
   */
  virtual bool setHeaderData(int, Qt::Orientation, const QVariant&,
                             int=Qt::EditRole) override { return false; }

  /**
   * Get number of rows.
   * @param parent parent model index, invalid for table models
   * @return number of rows,
   * if parent is valid number of children (0 for table models)
   */
  virtual int rowCount(const QModelIndex& parent=QModelIndex()) const override;

  /**
   * Get number of columns.
   * @param parent parent model index, invalid for table models
   * @return number of columns,
   * if parent is valid number of children (0 for table models)
   */
  virtual int columnCount(const QModelIndex& parent=QModelIndex()) const override;

  /**
   * Insert rows.
   * @param row rows are inserted before this row, if 0 at the begin,
   * if rowCount() at the end
   * @param count number of rows to insert
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool insertRows(int row, int count,
                          const QModelIndex& parent=QModelIndex()) override;

  /**
   * Remove rows.
   * @param row rows are removed starting with this row
   * @param count number of rows to remove
   * @param parent parent model index, invalid for table models
   * @return true if successful
   */
  virtual bool removeRows(int row, int count,
                          const QModelIndex& parent=QModelIndex()) override;

  /**
   * Set the model from the star count mappings.
   * @param maps star count mappings
   */
  void setMappings(const QList<QPair<QString, QVector<int> > >& maps);

  /**
   * Get the start count mappings from the model.
   * @return star count mappings
   */
  QList<QPair<QString, QVector<int> > > getMappings() const;

private:
  void makeRowValid(int row);

  QList<QPair<QString, QVector<int> > > m_maps;
};

#endif // STARRATINGMAPPINGSMODEL_H
