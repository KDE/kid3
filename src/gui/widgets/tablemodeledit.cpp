/**
 * \file tablemodeledit.cpp
 * Widget to edit a table model in-place.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2018
 *
 * Copyright (C) 2018-2024  Urs Fleisch
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

#include "tablemodeledit.h"
#include <QTableView>
#include <QHeaderView>

TableModelEdit::TableModelEdit(QAbstractItemModel* model, QWidget* parent)
  : AbstractListEdit(m_tableView = new QTableView, model, parent)
{
  setObjectName(QLatin1String("TableModelEdit"));
  setAddButtonText(tr("&Add"));
  hideEditButton();
  m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void TableModelEdit::addItem()
{
  QAbstractItemModel* model = getItemView()->model();
  int row = -1;
  if (QItemSelectionModel* selModel = getItemView()->selectionModel()) {
    if (QModelIndexList indexes = selModel->selectedIndexes();
        !indexes.isEmpty()) {
      row = indexes.first().row();
    }
  }
  if (row < 0 || row > model->rowCount()) {
    row = model->rowCount();
  }
  model->insertRow(row);
}

void TableModelEdit::editItem()
{
}
