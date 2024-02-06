/**
 * \file shortcutsdelegate.h
 * Keyboard shortcuts item delegate.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Dec 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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

#include <QItemDelegate>

class QKeySequenceEdit;

/**
 * Item delegate to edit and reset keyboard shortcuts.
 */
class ShortcutsDelegate : public QItemDelegate {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit ShortcutsDelegate(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~ShortcutsDelegate() override = default;

  /**
   * Create an editor to edit the cells contents.
   * @param parent parent widget
   * @param option style
   * @param index  index of item
   * @return editor widget
   */
  QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  /**
   * Set data to be edited by the editor.
   * @param editor editor widget
   * @param index  index of item
   */
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  /**
   * Set model data supplied by editor.
   * @param editor editor widget
   * @param model  model
   * @param index  index of item
   */
  void setModelData(
    QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

  /**
   * Updates the geometry of the @a editor for the item with the given
   * @a index, according to the rectangle specified in the @a option.
   * @param editor editor widget
   * @param option style
   * @param index  index of item
   */
  void updateEditorGeometry(
    QWidget* editor, const QStyleOptionViewItem& option,
      const QModelIndex& index) const override;

private slots:
  void clearAndCloseEditor();
  void resetToDefault();
  void commitAndCloseEditor();

private:
  mutable bool m_resetFlag;
};

/**
 * Editor widget for delegate with buttons to clear and reset the value.
 *
 * The editor consists of a line edit to edit the value and buttons to clear and
 * reset the value to the default.
 */
class ShortcutsDelegateEditor : public QFrame {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param lineEdit widget used to edit value
   * @param parent parent widget
   */
  explicit ShortcutsDelegateEditor(QLineEdit* lineEdit,
                                   QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  ~ShortcutsDelegateEditor() override = default;

  /*!
   * Get edit widget.
   * @return editor widget
   */
  QKeySequenceEdit* getEditor() { return m_editor; }

signals:
  /**
   * Emitted when a shortcut has been entered.
   */
  void valueEntered();

  /**
   * Emitted when the clear button is clicked.
   */
  void clearClicked();

  /**
   * Emitted when the reset button is clicked.
   */
  void resetClicked();

private:
  QKeySequenceEdit* m_editor;
};
