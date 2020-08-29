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
    m_previousSectionAction->setObjectName(QLatin1String("previous_section"));
    m_previousSectionAction->setShortcut(QKeySequence::Back);
    m_previousSectionAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_previousSectionAction);

    m_nextSectionAction = new QAction(m_widget);
    m_nextSectionAction->setObjectName(QLatin1String("next_section"));
    m_nextSectionAction->setShortcut(QKeySequence::Forward);
    m_nextSectionAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_nextSectionAction);
  }
  if (groups & Transfer) {
    m_transferAction = new QAction(m_widget);
    m_transferAction->setObjectName(QLatin1String("transfer_section"));
    m_transferAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
    m_transferAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_transferAction);
  }
  if (groups & EditSection) {
    m_copyAction = new QAction(m_widget);
    m_copyAction->setObjectName(QLatin1String("copy_section"));
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_copyAction);

    m_pasteAction = new QAction(m_widget);
    m_pasteAction->setObjectName(QLatin1String("paste_section"));
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_pasteAction);

    m_removeAction = new QAction(m_widget);
    m_removeAction->setObjectName(QLatin1String("remove_section"));
    m_removeAction->setShortcut(Qt::SHIFT + Qt::Key_Delete);
    m_removeAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_removeAction);
  }
  if (groups & EditElement) {
    m_editAction = new QAction(m_widget);
    m_editAction->setObjectName(QLatin1String("edit_section_element"));
    m_editAction->setShortcut(Qt::Key_F2);
    m_editAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_editAction);

    m_addAction = new QAction(m_widget);
    m_addAction->setObjectName(QLatin1String("add_section_element"));
    m_addAction->setShortcut(Qt::Key_Insert);
    m_addAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_addAction);

    m_deleteAction = new QAction(m_widget);
    m_deleteAction->setObjectName(QLatin1String("delete_section_element"));
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setShortcutContext(shortcutContext);
    m_widget->addAction(m_deleteAction);
  }
}

/**
 * Set keyboard shortcuts for section actions.
 * @param map map of action names to key sequences
 */
void SectionActions::setShortcuts(const QMap<QString, QKeySequence>& map)
{
  const QList<QAction*> actions = {
    m_previousSectionAction,
    m_nextSectionAction,
    m_copyAction,
    m_pasteAction,
    m_removeAction,
    m_transferAction,
    m_editAction,
    m_addAction,
    m_deleteAction
  };
  for (QAction* action : actions) {
    if (action) {
      QString name = action->objectName();
      if (!name.isEmpty()) {
        const auto it = map.constFind(name);
        if (it != map.constEnd()) {
          action->setShortcut(*it);
        }
      }
    }
  }
}

/**
 * Get section action default shortcut information.
 * @return list with name, display name, shortcut for all section actions.
 */
QList<std::tuple<QString, QString, QKeySequence>>
SectionActions::defaultShortcuts()
{
  return {
    // All these "std::tuple<QString, QString, QKeySequence>" are quite awkward,
    // but seem to be necessary to build with GCC 4.8.
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("previous_section"), tr("Previous"), QKeySequence::Back},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("next_section"), tr("Next"), QKeySequence::Forward},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("transfer_section"), tr("Transfer"),
          Qt::CTRL + Qt::SHIFT + Qt::Key_V},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("copy_section"), tr("Copy"), QKeySequence::Copy},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("paste_section"), tr("Paste"), QKeySequence::Paste},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("remove_section"), tr("Remove"),
          Qt::SHIFT + Qt::Key_Delete},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("edit_section_element"), tr("Edit"), Qt::Key_F2},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("add_section_element"), tr("Add"), Qt::Key_Insert},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("delete_section_element"), tr("Delete"),
          QKeySequence::Delete},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("open_parent"), tr("Open Parent Folder"),
          Qt::CTRL + Qt::Key_Up},
    std::tuple<QString, QString, QKeySequence>{
      QLatin1String("open_current"), tr("Open Current Folder"),
          Qt::CTRL + Qt::Key_Down}
  };
}
