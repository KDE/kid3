/**
 * \file shortcutsmodel.cpp
 * Keyboard shortcuts configuration tree model.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Dec 2011
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

#include "shortcutsmodel.h"

#ifndef CONFIG_USE_KDE

#include <QAction>
#include <QSettings>
#include "qtcompatmac.h"

/**
 * Constructor.
 * @param parent parent widget
 */
ShortcutsModel::ShortcutsModel(QObject* parent) : QAbstractItemModel(parent)
{
  setObjectName("ShortcutsModel");
}

/**
 * Destructor.
 */
ShortcutsModel::~ShortcutsModel()
{
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags ShortcutsModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index);
  if (index.isValid() && index.column() == ShortcutColumn) {
    itemFlags |= Qt::ItemIsEditable;
  }
  return itemFlags;
}

/**
 * Get group if a model index is a valid index for a group item.
 * @param index index to check
 * @return group if index is for a group item else 0
 */
const ShortcutsModel::ShortcutGroup* ShortcutsModel::shortcutGroupForIndex(
    const QModelIndex& index) const
{
  if (index.column() == 0 &&
      index.row() >= 0 && index.row() < m_shortcutGroups.size() &&
      index.internalId() == -1) {
    return &m_shortcutGroups.at(index.row());
  }
  return 0;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant ShortcutsModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    QModelIndex parentIndex = index.parent();
    if (parentIndex.isValid()) {
      if (const ShortcutGroup* group = shortcutGroupForIndex(parentIndex)) {
        if (index.row() >= 0 && index.row() < group->size()) {
          const ShortcutItem& shortcutItem = group->at(index.row());
          if (index.column() == ActionColumn) {
            if (role == Qt::DisplayRole) {
              return shortcutItem.actionText();
            } else if (role == Qt::FontRole) {
              if (shortcutItem.isCustomShortcutActive()) {
                QFont font;
                font.setBold(true);
                return font;
              }
            }
          } else if (index.column() == ShortcutColumn &&
                     (role == Qt::DisplayRole || role == Qt::EditRole)) {
            return shortcutItem.activeShortcut();
          }
        }
      }
    } else {
      if (const ShortcutGroup* group = shortcutGroupForIndex(index)) {
        if (role == Qt::DisplayRole) {
          return group->context();
        }
      }
    }
  }
  return QVariant();
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool ShortcutsModel::setData(const QModelIndex& index, const QVariant& value,
                             int role)
{
  if (index.isValid() && index.column() == ShortcutColumn && role == Qt::EditRole) {
    QModelIndex parentIndex = index.parent();
    if (parentIndex.isValid()) {
      if (ShortcutGroup* group =
          const_cast<ShortcutGroup*>(shortcutGroupForIndex(parentIndex))) {
        if (index.row() >= 0 && index.row() < group->size()) {
          ShortcutItem si((*group)[index.row()]);
          si.setCustomShortcut(value.toString());
          QString keyString(si.activeShortcut());
          if (!keyString.isEmpty()) {
            foreach (const ShortcutGroup& g, m_shortcutGroups) {
              foreach (const ShortcutItem& i, g) {
                if (i.activeShortcut() == keyString &&
                    si.action() != i.action()) {
                  emit shortcutAlreadyUsed(keyString, g.context(), i.action());
                  return false;
                }
              }
            }
          }
          (*group)[index.row()].setCustomShortcut(value.toString());
          emit dataChanged(index.sibling(index.row(), ActionColumn), index);
          emit shortcutSet(keyString, group->context(), si.action());
          return true;
        }
      }
    }
  }
  return false;
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    if (section == ActionColumn) {
      return i18n("Action");
    } else if (section == ShortcutColumn) {
      return i18n("Shortcut");
    }
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index
 * @return number of rows, if parent is valid number of children
 */
int ShortcutsModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    if (const ShortcutGroup* group = shortcutGroupForIndex(parent)) {
      return group->size();
    }
    return 0;
  } else {
    return m_shortcutGroups.size();
  }
}

/**
 * Get number of columns.
 * @param parent parent model index
 * @return number of columns for children of given @a parent
 */
int ShortcutsModel::columnCount(const QModelIndex& parent) const
{
  return NumColumns;
}

/**
 * Get model index of item.
 * @param row row of item
 * @param column column of item
 * @param parent index of parent item
 * @return model index of item
 */
QModelIndex ShortcutsModel::index(int row, int column,
                                  const QModelIndex& parent) const
{
  if (parent.isValid()) {
    const ShortcutGroup* group;
    if ((group = shortcutGroupForIndex(parent)) != 0 &&
        column >= 0 && column < NumColumns &&
        row >= 0 && row <= group->size()) {
      return createIndex(row, column, parent.row());
    }
  } else {
    if (column == 0 &&
        row >= 0 && row < m_shortcutGroups.size()) {
      return createIndex(row, column, -1);
    }
  }
  return QModelIndex();
}

/**
 * Get parent of item.
 * @param index model index of item
 * @return model index of parent item
 */
QModelIndex ShortcutsModel::parent(const QModelIndex& index) const
{
  int id = index.internalId();
  if (id >= 0 && id < m_shortcutGroups.size()) {
    return createIndex(id, 0, -1);
  }
  return QModelIndex();
}

/**
 * Register an action.
 *
 * @param action action to be added to model
 * @param context context of action
 */
void ShortcutsModel::registerAction(QAction* action, const QString& context)
{
  ShortcutItem item(action);
  ShortcutGroup group(context);

  QList<ShortcutGroup>::iterator it;
  for (it = m_shortcutGroups.begin(); it != m_shortcutGroups.end(); ++it) {
    if (it->context() == group.context()) {
      it->append(item);
      break;
    }
  }
  if (it == m_shortcutGroups.end()) {
    group.append(item);
    m_shortcutGroups.append(group);
  }
}

/**
 * Assign the shortcuts which have been changed to their actions.
 *
 * @return true if there was at least one shortcut changed
 */
bool ShortcutsModel::assignChangedShortcuts()
{
  bool changed = false;
  for (QList<ShortcutGroup>::iterator git = m_shortcutGroups.begin();
       git != m_shortcutGroups.end();
       ++git) {
    for (ShortcutGroup::iterator iit = git->begin();
         iit != git->end();
         ++iit) {
      if (iit->isCustomShortcutChanged()) {
        iit->assignCustomShortcut();
        changed = true;
      }
    }
  }
  return changed;
}

/**
 * Forget about all changed shortcuts.
 */
void ShortcutsModel::discardChangedShortcuts()
{
  for (QList<ShortcutGroup>::iterator git = m_shortcutGroups.begin();
       git != m_shortcutGroups.end();
       ++git) {
    for (ShortcutGroup::iterator iit = git->begin();
         iit != git->end();
         ++iit) {
      iit->revertCustomShortcut();
    }
  }
}

/**
 * Save the shortcuts to a given configuration.
 *
 * @param config configuration settings
 */
void ShortcutsModel::writeToConfig(QSettings* config) const
{
  config->beginGroup("/Shortcuts");
  config->remove("");
  for (QList<ShortcutGroup>::const_iterator git = m_shortcutGroups.constBegin();
       git != m_shortcutGroups.constEnd();
       ++git) {
    for (ShortcutGroup::const_iterator iit = git->constBegin();
         iit != git->constEnd();
         ++iit) {
      QString actionName(iit->action() ? iit->action()->objectName() : "");
      if (!actionName.isEmpty()) {
        if (!iit->customShortcut().isEmpty()) {
          config->setValue(actionName, iit->customShortcut());
        }
      } else {
        qWarning("Action %s does not have an object name",
                 qPrintable(iit->actionText()));
      }
    }
  }
  config->endGroup();
}

/**
 * Read the shortcuts from a given configuration.
 *
 * @param config configuration settings
 */
void ShortcutsModel::readFromConfig(QSettings* config)
{
  config->beginGroup("/Shortcuts");
  for (QList<ShortcutGroup>::iterator git = m_shortcutGroups.begin();
       git != m_shortcutGroups.end();
       ++git) {
    for (ShortcutGroup::iterator iit = git->begin();
         iit != git->end();
         ++iit) {
      QString actionName(iit->action() ? iit->action()->objectName() : "");
      if (!actionName.isEmpty() && config->contains(actionName)) {
        QString keyStr(config->value(actionName).toString());
        iit->setCustomShortcut(keyStr);
        iit->assignCustomShortcut();
      }
    }
  }
  config->endGroup();
}


ShortcutsModel::ShortcutItem::ShortcutItem(QAction* act) :
  m_action(act), m_defaultShortcut(m_action->shortcut().toString())
{
}

void ShortcutsModel::ShortcutItem::setCustomShortcut(const QString& shortcut)
{
  m_customShortcut = shortcut != m_defaultShortcut ? shortcut : QString();
}

void ShortcutsModel::ShortcutItem::revertCustomShortcut()
{
  m_customShortcut = m_oldCustomShortcut;
}

void ShortcutsModel::ShortcutItem::assignCustomShortcut()
{
  m_action->setShortcut(QKeySequence(activeShortcut()));
  m_oldCustomShortcut = m_customShortcut;
}

QString ShortcutsModel::ShortcutItem::actionText() const
{
  return m_action ? m_action->text().remove('&') : "";
}


ShortcutsModel::ShortcutGroup::ShortcutGroup(const QString& ctx) :
  m_context(ctx)
{
  m_context.remove('&');
}

#endif // !CONFIG_USE_KDE
