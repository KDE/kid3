/**
 * \file timestampdelegate.cpp
 * Delegate for time stamps in synchronized lyrics and event timing codes.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Mar 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include "timestampdelegate.h"
#include <QLineEdit>
#include "timeeventmodel.h"

/**
 * Constructor.
 * @param parent parent QTableView
 */
TimeStampDelegate::TimeStampDelegate(QObject* parent) : QItemDelegate(parent)
{
  setObjectName(QLatin1String("TimeStampDelegate"));
}

/**
 * Create an editor to edit the cell contents.
 * @param parent parent widget
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* TimeStampDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
  auto lineEdit = new QLineEdit(parent);
  connect(lineEdit, &QLineEdit::editingFinished,
          this, &TimeStampDelegate::commitAndCloseEditor);
  return lineEdit;
}

/**
 * Commit and close the editor.
 */
void TimeStampDelegate::commitAndCloseEditor()
{
  if (auto lineEdit = qobject_cast<QLineEdit*>(sender())) {
    emit commitData(lineEdit);
    emit closeEditor(lineEdit);
  }
}

/**
 * Render delegate.
 * @param painter painter to be used
 * @param option style
 * @param index index of item
 */
void TimeStampDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
  QString text = TimeEventModel::timeStampDataToString(index.data());
  QStyleOptionViewItem opt = option;
  opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
  drawDisplay(painter, opt, opt.rect, text);
  drawFocus(painter, opt, opt.rect);
}

/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void TimeStampDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor)) {
    const QString timeString = TimeEventModel::timeStampDataToString(index.data(Qt::EditRole));
    lineEdit->setText(timeString);
    return;
  }
  QItemDelegate::setEditorData(editor, index);
}

/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void TimeStampDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor)) {
    const QString text = lineEdit->text().trimmed();
    model->setData(index, TimeEventModel::timeStampDataFromString(text), Qt::EditRole);
    return;
  }
  QItemDelegate::setModelData(editor, model, index);
}
