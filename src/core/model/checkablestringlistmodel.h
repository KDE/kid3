/**
 * \file checkablestringlistmodel.h
 * String list model with checkable items.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Dec 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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

#ifndef CHECKABLESTRINGLISTMODEL_H
#define CHECKABLESTRINGLISTMODEL_H

#include <QStringListModel>
#include "kid3api.h"

/**
 * String list model with checkable items.
 * Up to 64 items can be stored in such a model because the check states are
 * stored in a 64 bit member variable.
 */
class KID3_CORE_EXPORT CheckableStringListModel : public QStringListModel {
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit CheckableStringListModel(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~CheckableStringListModel() override = default;

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
   * Insert rows.
   * @param row first row
   * @param count number of rows to insert
   * @param parent parent model index
   * @return true if rows were successfully inserted.
   */
  virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

  /**
   * Remove rows.
   * @param row first row
   * @param count number of rows to remove
   * @param parent parent model index
   * @return true if rows were successfully removed.
   */
  virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

  /**
   * Set mask with checked state for the items in the model.
   * @param mask bit mask containing check states, bit 0 is set if the first
   * item is checked, etc.
   */
  void setBitMask(quint64 mask) { m_bitMask = mask; }

  /**
   * Get mask with checked state for the items in the model.
   * @return bit mask containing check states, bit 0 is set if the first
   * item is checked, etc.
   */
  quint64 getBitMask() const { return m_bitMask; }

private:
  quint64 m_bitMask;
};

#endif // CHECKABLESTRINGLISTMODEL_H
