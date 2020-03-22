/**
 * \file sectionactions.h
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

#pragma once

#include <QObject>

class QWidget;
class QAction;

/**
 * Actions for section shortcuts.
 * Can be used to add actions to a section to navigate to other sections
 * and edit. The keyboard shortcuts are only active when the section has the
 * focus.
 */
class SectionActions : public QObject {
  Q_OBJECT
public:
  /** Which actions to include */
  enum ActionGroup {
    Navigation  = 1 << 0, /**< previous section, next section */
    Transfer    = 1 << 1, /**< transfer (from other tag) */
    EditSection = 1 << 2, /**< copy, paste, remove */
    EditElement = 1 << 3  /**< edit, add, delete */
  };
  Q_DECLARE_FLAGS(ActionGroups, ActionGroup)

  /**
   * Constructor.
   * @param groups action groups to add
   * @param widget widget to which actions are added
   */
  SectionActions(ActionGroups groups, QWidget* widget);

  /** Get action for previous section. */
  QAction* previousSectionAction() const {
    return m_previousSectionAction;
  }

  /** Get action for next section. */
  QAction* nextSectionAction() const {
    return m_nextSectionAction;
  }

  /** Get action for copy. */
  QAction* copyAction() const {
    return m_copyAction;
  }

  /** Get action for paste. */
  QAction* pasteAction() const {
    return m_pasteAction;
  }

  /** Get action for remove. */
  QAction* removeAction() const {
    return m_removeAction;
  }

  /** Get action for transfer to other tag. */
  QAction* transferAction() const {
    return m_transferAction;
  }

  /** Get action for edit. */
  QAction* editAction() const {
    return m_editAction;
  }

  /** Get action for add. */
  QAction* addAction() const {
    return m_addAction;
  }

  /** Get action for delete. */
  QAction* deleteAction() const {
    return m_deleteAction;
  }

private:
  QWidget* m_widget;
  QAction* m_previousSectionAction;
  QAction* m_nextSectionAction;
  QAction* m_copyAction;
  QAction* m_pasteAction;
  QAction* m_removeAction;
  QAction* m_transferAction;
  QAction* m_editAction;
  QAction* m_addAction;
  QAction* m_deleteAction;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SectionActions::ActionGroups)
