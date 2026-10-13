/**
 * \file shortcutsdelegate.cpp
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

#include "shortcutsdelegate.h"

#include <QToolButton>
#include <QApplication>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QAbstractItemView>
#if defined Q_OS_MAC && QT_VERSION >= 0x060800 && !defined QT_NO_ACCESSIBILITY
#include "accessibilityhelper.h"
#endif

/**
 * Constructor.
 * @param parent parent object
 */
ShortcutsDelegate::ShortcutsDelegate(QObject* parent)
  : QItemDelegate(parent), m_resetFlag(false)
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
    auto shortcutsEditor = new ShortcutsDelegateEditor(le, parent);
    editor = shortcutsEditor;
    connect(shortcutsEditor, &ShortcutsDelegateEditor::clearClicked,
            this, &ShortcutsDelegate::clearAndCloseEditor);
    connect(shortcutsEditor, &ShortcutsDelegateEditor::resetClicked,
            this, &ShortcutsDelegate::resetToDefault);
    connect(shortcutsEditor, &ShortcutsDelegateEditor::valueEntered,
            this, &ShortcutsDelegate::commitAndCloseEditor);
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
    closeEditorKeepingFocus(editor);
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
    closeEditorKeepingFocus(editor);
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
    closeEditorKeepingFocus(editor);
  }
}

/**
 * Close editor, keeping the focus on the item view.
 * @param editor editor widget
 */
void ShortcutsDelegate::closeEditorKeepingFocus(ShortcutsDelegateEditor* editor)
{
  // The view only takes the focus back if the key edit has the focus.
  // If a button has it, the focus would move to the next widget when the
  // editor is hidden.
  if (QWidget* fw = QApplication::focusWidget();
      fw && fw != editor->getEditor() && editor->isAncestorOf(fw)) {
    QWidget* view = editor->parentWidget();
    while (view && !qobject_cast<QAbstractItemView*>(view)) {
      view = view->parentWidget();
    }
    if (view) {
      view->setFocus(Qt::OtherFocusReason);
    }
  }
  emit closeEditor(editor);
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
    const QString name = index.sibling(index.row(), 0).data().toString();
    compoundWidget->setAccessibleName(name);
    compoundWidget->getEditor()->setAccessibleName(name);
    if (auto lineEdit = compoundWidget->getEditor()->findChild<QLineEdit*>()) {
      lineEdit->setAccessibleName(name);
    }
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
  QLineEdit* lineEdit, QWidget* parent)
  : QFrame(parent) {
  auto hlayout = new QHBoxLayout(this);
  hlayout->setContentsMargins(0, 0, 0, 0);
  delete lineEdit;
  m_editor = new QKeySequenceEdit(parent);
  connect(m_editor, &QKeySequenceEdit::editingFinished, this, [this] {
    // The key edit also finishes when it loses focus to one of the buttons.
    if (QWidget* fw = QApplication::focusWidget();
        fw && fw != m_editor && isAncestorOf(fw)) {
      return;
    }
    emit valueEntered();
  });
  m_editor->installEventFilter(this);
  m_editor->setAccessibleDescription(
        tr("Press the new key combination. Press Tab to go to the Clear and "
           "Reset buttons."));
  setFocusProxy(m_editor);
  hlayout->addWidget(m_editor, 0, Qt::AlignLeft);
  m_clearButton = new QToolButton(this);
  m_clearButton->setText(tr("Clear"));
  m_clearButton->setAccessibleDescription(tr("Remove the keyboard shortcut"));
  // Tool buttons are skipped by Tab on macOS without full keyboard access.
  m_clearButton->setFocusPolicy(Qt::StrongFocus);
  connect(m_clearButton, &QAbstractButton::clicked,
          this, &ShortcutsDelegateEditor::clearClicked);
  hlayout->addWidget(m_clearButton);
  m_resetButton = new QToolButton(this);
  m_resetButton->setText(tr("Reset"));
  m_resetButton->setAccessibleDescription(
        tr("Restore the default keyboard shortcut"));
  m_resetButton->setFocusPolicy(Qt::StrongFocus);
  m_clearButton->installEventFilter(this);
  m_resetButton->installEventFilter(this);
  connect(m_resetButton, &QAbstractButton::clicked,
          this, &ShortcutsDelegateEditor::resetClicked);
  hlayout->addWidget(m_resetButton);
}

bool ShortcutsDelegateEditor::eventFilter(QObject* watched, QEvent* event)
{
#if defined Q_OS_MAC && QT_VERSION >= 0x060800 && !defined QT_NO_ACCESSIBILITY
  if (event->type() == QEvent::FocusIn &&
      (watched == m_editor || watched == m_clearButton ||
       watched == m_resetButton)) {
    // The table reports its current cell as focused instead of the editor.
    auto widget = static_cast<QWidget*>(watched);
    QString name = widget->accessibleName();
    if (auto button = qobject_cast<QAbstractButton*>(widget)) {
      name = button->text();
    }
    AccessibilityHelper::announce(
          widget, name + QLatin1String(", ") + widget->accessibleDescription());
  }
#endif
  if (watched == m_editor && event->type() == QEvent::KeyPress) {
    const auto keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Tab &&
        keyEvent->modifiers() == Qt::NoModifier) {
      m_clearButton->setFocus(Qt::TabFocusReason);
      return true;
    }
    if (keyEvent->key() == Qt::Key_Backtab) {
      m_resetButton->setFocus(Qt::BacktabFocusReason);
      return true;
    }
  } else if ((watched == m_clearButton || watched == m_resetButton) &&
             event->type() == QEvent::KeyPress) {
    // Leaving the editor with the keyboard accepts it.
    const int key = static_cast<QKeyEvent*>(event)->key();
    if (key == Qt::Key_Escape ||
        (key == Qt::Key_Tab && watched == m_resetButton)) {
      emit valueEntered();
      return true;
    }
  }
  return QFrame::eventFilter(watched, event);
}
