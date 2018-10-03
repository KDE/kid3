/**
 * \file comboboxdelegate.cpp
 * Delegate to select with combo box between items in Qt:UserRole data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 May 2011
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

#include "comboboxdelegate.h"
#include <QComboBox>

/**
 * Constructor.
 * @param parent parent object
 */
ComboBoxDelegate::ComboBoxDelegate(QObject* parent) :
  QStyledItemDelegate(parent)
{
  setObjectName(QLatin1String("ComboBoxDelegate"));
}

/**
 * Create an editor to edit the cells contents.
 * @param parent parent widget
 * @param option style
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* ComboBoxDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  QVariant itemsData(index.data(Qt::UserRole));
  if (itemsData.isValid() && itemsData.type() == QVariant::StringList) {
    QStringList items(itemsData.toStringList());
    int itemIndex = items.indexOf(index.data(Qt::EditRole).toString());
    auto cb = new QComboBox(parent);
    cb->addItems(itemsData.toStringList());
    if (itemIndex >= 0)
      cb->setCurrentIndex(itemIndex);
    return cb;
  } else {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }
}

/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void ComboBoxDelegate::setEditorData(QWidget* editor,
                                     const QModelIndex& index) const
{
  if (auto cb = qobject_cast<QComboBox*>(editor)) {
    int itemIndex = cb->findText(index.data(Qt::EditRole).toString());
    if (itemIndex >= 0)
      cb->setCurrentIndex(itemIndex);
  } else {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                    const QModelIndex& index) const
{
  if (auto cb = qobject_cast<QComboBox*>(editor))
    model->setData(index, cb->currentText(), Qt::EditRole);
  else
    QStyledItemDelegate::setModelData(editor, model, index);
}
