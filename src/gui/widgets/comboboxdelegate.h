/**
 * \file comboboxdelegate.cpp
 * Delegate to select with combo box between items in Qt:UserRole data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 May 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include <QStyledItemDelegate>

/**
 * Delegate to select with combo box between items in Qt:UserRole data.
 */
class ComboBoxDelegate : public QStyledItemDelegate {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit ComboBoxDelegate(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~ComboBoxDelegate() override = default;

  /**
   * Create an editor to edit the cells contents.
   * @param parent parent widget
   * @param option style
   * @param index  index of item
   * @return combo box editor widget.
   */
  virtual QWidget* createEditor(QWidget* parent,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const override;

  /**
   * Set data to be edited by the editor.
   * @param editor editor widget
   * @param index  index of item
   */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
                            const QModelIndex& index) const override;
};
