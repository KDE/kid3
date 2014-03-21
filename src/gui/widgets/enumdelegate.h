/**
 * \file enumdelegate.h
 * Abstract base class for delegates which display enums in a combobox.
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

#ifndef ENUMDELEGATE_H
#define ENUMDELEGATE_H

#include <QItemDelegate>

/**
 * Abstract base class for delegates which display enums in a combobox.
 */
class EnumDelegate : public QItemDelegate {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit EnumDelegate(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~EnumDelegate();

  /**
   * Create an editor to edit the cells contents.
   * @param parent parent widget
   * @param option style
   * @param index  index of item
   * @return combo box editor widget.
   */
  virtual QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const;

  /**
   * Set data to be edited by the editor.
   * @param editor editor widget
   * @param index  index of item
   */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

  /**
   * Set model data supplied by editor.
   * @param editor editor widget
   * @param model  model
   * @param index  index of item
   */
  virtual void setModelData(
    QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

  /**
   * Get size needed by delegate.
   * @param option style
   * @param index  index of item
   * @return size needed by delegate.
   */
  virtual QSize sizeHint(const QStyleOptionViewItem& option,
                         const QModelIndex& index) const;

protected:
  /**
   * Render item view text.
   * @param painter painter
   * @param option style
   * @param rect the text has to be rendered within this rectangle
   * @param text to be rendered
   */
  virtual void drawDisplay(
    QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect,
    const QString& text) const;

  /**
   * Get list of string representations for an enum.
   * @return string for enum with index 0, string for enum with index 1, ...
   */
  virtual QStringList getEnumStrings() const = 0;

  /**
   * Get string representation for an enum value.
   * @param enumNr the enum value as an integer
   * @return string representation
   */
  virtual QString getStringForEnum(int enumNr) const = 0;

  /**
   * Get index of an enum value.
   * @param enumNr the enum value as an integer
   * @return index of this enum value in the list returned by getEnumStrings()
   */
  virtual int getIndexForEnum(int enumNr) const = 0;

  /**
   * Get enum value for an index.
   * @param index index of enum in list returned by getEnumStrings()
   * @return corresponding enum value as an integer
   */
  virtual int getEnumForIndex(int index) const = 0;
};

#endif // ENUMDELEGATE_H
