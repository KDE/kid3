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

#ifndef CONFIG_USE_KDE

#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "qtcompatmac.h"

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
  if (QLineEdit* le = qobject_cast<QLineEdit*>(editor)) {
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
  if (ShortcutsDelegateEditor* editor =
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
  if (ShortcutsDelegateEditor* editor =
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
  if (ShortcutsDelegateEditor* editor =
      qobject_cast<ShortcutsDelegateEditor*>(sender())) {
    editor->getLineEdit()->clear();
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
  if (ShortcutsDelegateEditor* compoundWidget =
      qobject_cast<ShortcutsDelegateEditor*>(editor)) {
    QItemDelegate::setEditorData(compoundWidget->getLineEdit(), index);
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
  if (ShortcutsDelegateEditor* compoundWidget =
      qobject_cast<ShortcutsDelegateEditor*>(editor)) {
    if (m_resetFlag) {
      m_resetFlag = false;
      model->setData(index, QVariant(), Qt::EditRole);
    } else {
      QItemDelegate::setModelData(compoundWidget->getLineEdit(), model, index);
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
  QFrame(parent), m_lineEdit(lineEdit) {
  QHBoxLayout* hlayout = new QHBoxLayout(this);
  hlayout->setContentsMargins(0, 0, 0, 0);
  hlayout->addWidget(m_lineEdit, 0, Qt::AlignLeft);
  QToolButton* clearButton = new QToolButton(this);
  clearButton->setText(i18n("Clear"));
  connect(clearButton, SIGNAL(clicked()), this, SIGNAL(clearClicked()));
  hlayout->addWidget(clearButton);
  QToolButton* resetButton = new QToolButton(this);
  resetButton->setText(i18n("Reset"));
  connect(resetButton, SIGNAL(clicked()), this, SIGNAL(resetClicked()));
  hlayout->addWidget(resetButton);
  setFocusProxy(m_lineEdit);
  m_lineEdit->setReadOnly(true);
  m_lineEdit->installEventFilter(this);
}

/**
 * Destructor.
 */
ShortcutsDelegateEditor::~ShortcutsDelegateEditor()
{
}


bool ShortcutsDelegateEditor::event(QEvent* ev)
{
  QEvent::Type eventType = ev->type();
  if (eventType != QEvent::ShortcutOverride &&
      eventType != QEvent::KeyPress &&
      eventType != QEvent::KeyRelease)
    return QFrame::event(ev);

  if (eventType == QEvent::ShortcutOverride) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);
    int keyCode = 0;

    // Check modifiers pressed
    if (keyEvent->modifiers() & Qt::ControlModifier)
      keyCode |= Qt::ControlModifier;
    if (keyEvent->modifiers() & Qt::AltModifier)
      keyCode |= Qt::AltModifier;
    if (keyEvent->modifiers() & Qt::ShiftModifier)
      keyCode |= Qt::ShiftModifier;
    if (keyEvent->modifiers() & Qt::MetaModifier)
      keyCode |= Qt::MetaModifier;

    switch (keyEvent->key()) {
    // These keys can't be used
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
    case Qt::Key_Menu:
    case Qt::Key_Hyper_L:
    case Qt::Key_Hyper_R:
    case Qt::Key_Help:
    case Qt::Key_Direction_L:
    case Qt::Key_Direction_R:
      break;

    default:
      keyCode |= keyEvent->key();
    }

    QString keyString = QKeySequence(keyCode).toString();
    if (!keyString.endsWith(QLatin1Char('+'))) {
      m_lineEdit->setText(keyString);
      emit valueEntered();
    }
  }
  return true;
}

/**
 * Filters events if this object has been installed as an event filter for
 * the @a watched object.
 * @param watched watched object
 * @param ev event
 * @return true to stop further event handling
 */
bool ShortcutsDelegateEditor::eventFilter(QObject* watched, QEvent* ev)
{
  QEvent::Type eventType = ev->type();
  if (eventType == QEvent::KeyPress ||
      eventType == QEvent::KeyRelease ||
      eventType == QEvent::ShortcutOverride)
    return event(ev);
  else
    return QFrame::eventFilter(watched, ev);
}

#endif // !CONFIG_USE_KDE
