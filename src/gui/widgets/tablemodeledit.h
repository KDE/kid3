/**
 * \file tablemodeledit.h
 * Widget to edit a table model in-place.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2018
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

#ifndef TABLEMODELEDIT_H
#define TABLEMODELEDIT_H

#include "abstractlistedit.h"

class QTableView;

/**
 * Widget to edit a table model in-place.
 */
class TableModelEdit : public AbstractListEdit {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param model item model
   * @param parent parent widget
   */
  explicit TableModelEdit(QAbstractItemModel* model, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~TableModelEdit() override;

  /**
   * Add a new item.
   */
  virtual void addItem() override;

  /**
   * Edit the selected item.
   */
  virtual void editItem() override;

private:
  QTableView* m_tableView;
};

#endif // TABLEMODELEDIT_H
