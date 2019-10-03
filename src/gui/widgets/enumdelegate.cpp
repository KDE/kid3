/**
 * \file enumdelegate.cpp
 * Abstract base class for delegates which display enums in a combobox.
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

#include "enumdelegate.h"
#include <QComboBox>
#include <QFontMetrics>

/**
 * Constructor.
 * @param parent parent object
 */
EnumDelegate::EnumDelegate(QObject* parent) : QItemDelegate(parent)
{
}

/**
 * Create an editor to edit the cells contents.
 * @param parent parent widget
 * @param option style
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* EnumDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
  auto cb = new QComboBox(parent);
  cb->addItems(getEnumStrings());
  return cb;
}

/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void EnumDelegate::setEditorData(
  QWidget* editor, const QModelIndex& index) const
{
  auto cb = qobject_cast<QComboBox*>(editor);
  int enumNr = index.data(Qt::EditRole).toInt();
  if (cb && enumNr >= 0) {
    cb->setCurrentIndex(getIndexForEnum(enumNr));
  } else {
    QItemDelegate::setEditorData(editor, index);
  }
}

/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void EnumDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  auto cb = qobject_cast<QComboBox*>(editor);
  if (cb) {
    int enumNr = getEnumForIndex(cb->currentIndex());
    if (enumNr >= 0) {
      model->setData(index, enumNr, Qt::EditRole);
      return;
    }
  }
  QItemDelegate::setModelData(editor, model, index);
}

/**
 * Get size needed by delegate.
 * @param option style
 * @param index  index of item
 * @return size needed by delegate.
 */
QSize EnumDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
  QSize size(QItemDelegate::sizeHint(option, index));
  bool ok;
  int enumNr = index.data(Qt::EditRole).toInt(&ok);
  if (ok) {
    QFont fnt(qvariant_cast<QFont>(index.data(Qt::FontRole))
              .resolve(option.font));
    QFontMetrics fm(fnt);
#if QT_VERSION >= 0x050b00
    int origWidth = fm.horizontalAdvance(QString::number(enumNr));
    int delegateWidth = fm.horizontalAdvance(getStringForEnum(enumNr));
#else
    int origWidth = fm.width(QString::number(enumNr));
    int delegateWidth = fm.width(getStringForEnum(enumNr));
#endif
    size.setWidth(size.width() + delegateWidth - origWidth);
  }
  return size;
}

/**
 * Render item view text.
 * @param painter painter
 * @param option style
 * @param rect the text has to be rendered within this rectangle
 * @param text to be rendered
 */
void EnumDelegate::drawDisplay(
  QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect,
  const QString& text) const
{
  bool ok;
  int enumNr = text.toInt(&ok);
  QItemDelegate::drawDisplay(painter, option, rect,
                             ok ? getStringForEnum(enumNr) : text);
}
