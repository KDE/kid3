/**
 * \file timestampdelegate.cpp
 * Delegate for time stamps in synchronized lyrics and event timing codes.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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
#include <QTime>
#include <QTimeEdit>
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
 * Destructor.
 */
TimeStampDelegate::~TimeStampDelegate()
{
}

/**
 * Create an editor to edit the cell contents.
 * @param parent parent widget
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* TimeStampDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
  QTime time = index.data().toTime();
  QTimeEdit* timeEdit = new QTimeEdit(parent);
  timeEdit->setDisplayFormat(time.hour() == 0 ? QLatin1String("mm:ss.zzz")
                                              : QLatin1String("hh:mm:ss.zzz"));
  connect(timeEdit, SIGNAL(editingFinished()),
          this, SLOT(commitAndCloseEditor()));
  return timeEdit;
}

/**
 * Commit and close the editor.
 */
void TimeStampDelegate::commitAndCloseEditor()
{
  if (QTimeEdit* editor = qobject_cast<QTimeEdit*>(sender())) {
    emit commitData(editor);
    emit closeEditor(editor);
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
  QTime time = index.data().toTime();
  QString text = TimeEventModel::timeStampToString(time);
  QStyleOptionViewItem opt = option;
  opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
  drawDisplay(painter, opt, opt.rect, text);
  drawFocus(painter, opt, opt.rect);
}
