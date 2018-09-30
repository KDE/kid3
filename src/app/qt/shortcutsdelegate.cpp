/**
 * \file shortcutsdelegate.cpp
 * Keyboard shortcuts item delegate.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Dec 2011
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

#include "shortcutsdelegate.h"

#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequenceEdit>

/**
 * Constructor.
 * @param parent parent object
 */
ShortcutsDelegate::ShortcutsDelegate(QObject* parent) :
  QItemDelegate(parent), m_resetFlag(false)
{
}

/**
 * Destructor.
 */
ShortcutsDelegate::~ShortcutsDelegate()
{
}

/**
 * Create an editor to edit the cells contents.
 * @param parent parent widget
 * @param option style
 * @param index  index of item
 * @return editor widget
 */
QWidget* ShortcutsDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  QWidget* editor = QItemDelegate::createEditor(parent, option, index);
  if (auto le = qobject_cast<QLineEdit*>(editor)) {
    editor = new ShortcutsDelegateEditor(le, parent);
    connect(editor, SIGNAL(clearClicked()), this, SLOT(clearAndCloseEditor()));
    connect(editor, SIGNAL(resetClicked()), this, SLOT(resetToDefault()));
    connect(editor, SIGNAL(valueEntered()), this, SLOT(commitAndCloseEditor()));
  }
  return editor;
}

/**
 * Reset editor to default value.
 */
void ShortcutsDelegate::resetToDefault()
{
  if (auto editor =
      qobject_cast<ShortcutsDelegateEditor*>(sender())) {
    m_resetFlag = true;
    emit commitData(editor);
    emit closeEditor(editor);
  }
}

/**
 * Commit editor value and close editor.
 */
void ShortcutsDelegate::commitAndCloseEditor()
{
  if (auto editor =
      qobject_cast<ShortcutsDelegateEditor*>(sender())) {
    emit commitData(editor);
    emit closeEditor(editor);
  }
}

/**
 * Clear editor value and close editor.
 */
void ShortcutsDelegate::clearAndCloseEditor()
{
  if (auto editor =
      qobject_cast<ShortcutsDelegateEditor*>(sender())) {
    editor->getEditor()->clear();
    emit commitData(editor);
    emit closeEditor(editor);
  }
}

/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void ShortcutsDelegate::setEditorData(
  QWidget* editor, const QModelIndex& index) const
{
  if (auto compoundWidget =
      qobject_cast<ShortcutsDelegateEditor*>(editor)) {
    QItemDelegate::setEditorData(compoundWidget->getEditor(), index);
  }
}

/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void ShortcutsDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (auto compoundWidget =
      qobject_cast<ShortcutsDelegateEditor*>(editor)) {
    if (m_resetFlag) {
      m_resetFlag = false;
      model->setData(index, QVariant(), Qt::EditRole);
    } else {
      QItemDelegate::setModelData(compoundWidget->getEditor(), model, index);
    }
  }
}

/**
 * Updates the geometry of the @a editor for the item with the given
 * @a index, according to the rectangle specified in the @a option.
 * @param editor editor widget
 * @param option style
 * @param index  index of item
 */
void ShortcutsDelegate::updateEditorGeometry(
  QWidget* editor, const QStyleOptionViewItem& option,
  const QModelIndex&) const
{
  // If this method is not overridden, the ShortcutsDelegateEditor
  // is displayed as a thin horizontal line, see also
  // http://stackoverflow.com/questions/5457154/qstyleditemdelegate-how-does-updateeditorgeometry-works
  QRect rect(option.rect);
  QSize sizeHint(editor->sizeHint());

  if (rect.width() < sizeHint.width()) {
    rect.setWidth(sizeHint.width());
  }
  if (rect.height() < sizeHint.height()) {
    int yAdj = (rect.height() - sizeHint.height()) / 2;
    rect.setHeight(sizeHint.height());
    rect.translate(0, yAdj);
  }

  editor->setGeometry(rect);
}


/**
 * Constructor.
 *
 * @param parent parent widget
 */
ShortcutsDelegateEditor::ShortcutsDelegateEditor(
  QLineEdit* lineEdit, QWidget* parent) :
  QFrame(parent) {
  auto hlayout = new QHBoxLayout(this);
  hlayout->setContentsMargins(0, 0, 0, 0);
  delete lineEdit;
  m_editor = new QKeySequenceEdit(parent);
  connect(m_editor, &QKeySequenceEdit::editingFinished, this, &ShortcutsDelegateEditor::valueEntered);
  setFocusProxy(m_editor);
  hlayout->addWidget(m_editor, 0, Qt::AlignLeft);
  auto clearButton = new QToolButton(this);
  clearButton->setText(tr("Clear"));
  connect(clearButton, &QAbstractButton::clicked, this, &ShortcutsDelegateEditor::clearClicked);
  hlayout->addWidget(clearButton);
  auto resetButton = new QToolButton(this);
  resetButton->setText(tr("Reset"));
  connect(resetButton, &QAbstractButton::clicked, this, &ShortcutsDelegateEditor::resetClicked);
  hlayout->addWidget(resetButton);
}

/**
 * Destructor.
 */
ShortcutsDelegateEditor::~ShortcutsDelegateEditor()
{
}
