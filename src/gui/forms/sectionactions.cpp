/**
 * \file sectionactions.cpp
 * Actions for section shortcuts.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 22 Mar 2020
 *
 * Copyright (C) 2020  Urs Fleisch
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

#include "sectionactions.h"
#include <QAction>
#include <QAbstractItemView>

/**
 * Constructor.
 * @param groups action groups to add
 * @param widget widget to which actions are added
 */
SectionActions::SectionActions(ActionGroups groups, QWidget* widget)
  : QObject(widget),
    m_widget(widget),
    m_previousSectionAction(nullptr), m_nextSectionAction(nullptr),
    m_copyAction(nullptr), m_pasteAction(nullptr),
    m_removeAction(nullptr), m_transferAction(nullptr),
    m_editAction(nullptr), m_addAction(nullptr), m_deleteAction(nullptr)
{
  auto shortcutContext = qobject_cast<QAbstractItemView*>(m_widget)
      ? Qt::WidgetShortcut : Qt::WidgetWithChildrenShortcut;
  if (groups & Navigation) {
    m_previousSectionAction = new QAction(m_widget);
    m_previousSectionAction->setShortcut(QKeySequence::Back);
    m_previousSectionAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_previousSectionAction);

    m_nextSectionAction = new QAction(m_widget);
    m_nextSectionAction->setShortcut(QKeySequence::Forward);
    m_nextSectionAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_nextSectionAction);
  }
  if (groups & Transfer) {
    m_transferAction = new QAction(m_widget);
    m_transferAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
    m_transferAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_transferAction);
  }
  if (groups & EditSection) {
    m_copyAction = new QAction(m_widget);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_copyAction);

    m_pasteAction = new QAction(m_widget);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_pasteAction);

    m_removeAction = new QAction(m_widget);
    m_removeAction->setShortcut(Qt::SHIFT + Qt::Key_Delete);
    m_removeAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_removeAction);
  }
  if (groups & EditElement) {
    m_editAction = new QAction(m_widget);
    m_editAction->setShortcut(Qt::Key_F2);
    m_editAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_editAction);

    m_addAction = new QAction(m_widget);
    m_addAction->setShortcut(Qt::Key_Insert);
    m_addAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_addAction);

    m_deleteAction = new QAction(m_widget);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_deleteAction);
  }
}
