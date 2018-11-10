/**
 * \file dirproxymodel.h
 * Proxy for filesystem model which filters directories.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19-Mar-2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include <QSortFilterProxyModel>

/**
 * Proxy for filesystem model which filters directories.
 */
class DirProxyModel : public QSortFilterProxyModel {
public:
  /**
   * Constructor.
   *
   * @param parent parent object
   */
  explicit DirProxyModel(QObject* parent = nullptr);

  /**
   * Get item flags.
   * @param index index of item
   * @return default flags without editable.
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Reset the model.
   */
  void resetModel();

protected:
  /**
   * Check if row should be included in model.
   *
   * @param srcRow source row
   * @param srcParent source parent
   *
   * @return true to include row.
   */
  virtual bool filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const override;

  /**
   * Sort comparison function.
   * @param left index of left item in source model
   * @param right index of right item in source model
   * @return true if left is less than right.
   */
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};
