/**
 * \file timestampdelegate.h
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

#ifndef TIMESTAMPDELEGATE_H
#define TIMESTAMPDELEGATE_H

#include <QItemDelegate>

/**
 * Delegate for time stamps in synchronized lyrics and event timing codes.
 */
class TimeStampDelegate : public QItemDelegate {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit TimeStampDelegate(QObject* parent);

  /**
   * Destructor.
   */
  virtual ~TimeStampDelegate();

  /**
   * Create an editor to edit the cell contents.
   * @param parent parent widget
   * @param option style
   * @param index  index of item
   * @return time editor widget.
   */
  virtual QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const;

  /**
   * Render delegate.
   * @param painter painter to be used
   * @param option style
   * @param index index of item
   */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const;

private slots:
  void commitAndCloseEditor();
};

#endif // TIMESTAMPDELEGATE_H
