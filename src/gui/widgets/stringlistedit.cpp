/**
 * \file stringlistedit.cpp
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
 *
 * Copyright (C) 2007-2012  Urs Fleisch
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

#include "stringlistedit.h"
#include <QLineEdit>
#include <QInputDialog>
#include <QLayout>
#include <QListView>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param model item model, e.g. a QStringListModel
 * @param parent parent widget
 */
StringListEdit::StringListEdit(QAbstractItemModel* model, QWidget* parent) :
  AbstractListEdit(new QListView, model, parent)
{
  setObjectName("StringListEdit");
}

/**
 * Destructor.
 */
StringListEdit::~StringListEdit()
{
}

/**
 * Add a new item.
 */
void StringListEdit::addItem()
{
  bool ok;
  QString txt = QInputDialog::getText(
    this, i18n("Add Item"), QString::null, QLineEdit::Normal,
    QString::null, &ok);
  if (ok && !txt.isEmpty()) {
    QAbstractItemModel* model = getItemView()->model();
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 0), txt);
  }
}

/**
 * Edit the selected item.
 */
void StringListEdit::editItem()
{
  QModelIndex index = getItemView()->currentIndex();
  if (index.isValid()) {
    QAbstractItemModel* model = getItemView()->model();
    bool ok;
    QString txt = QInputDialog::getText(
      this, i18n("Edit Item"), QString::null, QLineEdit::Normal,
      model->data(index, Qt::EditRole).toString(), &ok);
    if (ok && !txt.isEmpty()) {
      model->setData(index, txt);
    }
  }
}
