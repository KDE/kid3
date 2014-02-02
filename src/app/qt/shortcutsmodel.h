/**
 * \file shortcutsmodel.h
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

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QAbstractItemModel>
#include <QList>

class QAction;
class ISettings;

/**
 * Keyboard shortcuts configuration tree model.
 *
 * The model is hierarchical with two levels: The keyboard shortcuts have
 * columns with the text of the action and the key sequences and have context
 * parent items, which describe the menu or section in the GUI where the
 * action can be found. The model can be used in a QTreeView, to edit the
 * @ref ShortcutColumn, a ShortcutsDelegate can be used.
 */
class ShortcutsModel : public QAbstractItemModel {
  Q_OBJECT
public:
  /**
   * Columns in model.
   */
  enum Columns {
    ActionColumn,   /**< Action text */
    ShortcutColumn, /**< Shortcut key sequence */
    NumColumns      /**< Number of columns */
  };

  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit ShortcutsModel(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~ShortcutsModel();

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  virtual QVariant data(const QModelIndex& index,
                        int role = Qt::DisplayRole) const;

  /**
   * Set data for a given role.
   * @param index model index
   * @param value data value
   * @param role item data role
   * @return true if successful
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role = Qt::EditRole);

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;

  /**
   * Get number of rows.
   * @param parent parent model index
   * @return number of rows, if parent is valid number of children
   */
  virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;

  /**
   * Get number of columns.
   * @param parent parent model index
   * @return number of columns for children of given \a parent
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get model index of item.
   * @param row row of item
   * @param column column of item
   * @param parent index of parent item
   * @return model index of item
   */
  virtual QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get parent of item.
   * @param index model index of item
   * @return model index of parent item
   */
  virtual QModelIndex parent(const QModelIndex& index) const;

  /**
   * Register an action.
   *
   * @param action action to be added to model
   * @param context context of action
   */
  void registerAction(QAction* action, const QString& context);

  /**
   * Assign the shortcuts which have been changed to their actions.
   *
   * @return true if there was at least one shortcut changed
   */
  bool assignChangedShortcuts();

  /**
   * Save the shortcuts to a given configuration.
   *
   * @param config configuration settings
   */
  void writeToConfig(ISettings* config) const;

  /**
   * Read the shortcuts from a given configuration.
   *
   * @param config configuration settings
   */
  void readFromConfig(ISettings* config);

public slots:
  /**
   * Forget about all changed shortcuts.
   */
  void discardChangedShortcuts();

signals:
  /**
   * Emitted if a keyboard shortcut is already used.
   * @param key string representation of key sequence
   * @param context context of action
   * @param action action using @a key
   */
  void shortcutAlreadyUsed(const QString& key, const QString& context,
                           const QAction* action);

  /**
   * Emitted if a keyboard shortcut is set.
   * Can be used to clear a previously displayed "already used" warning.
   * @param key string representation of key sequence
   * @param context context of action
   * @param action action using @a key
   */
  void shortcutSet(const QString& key, const QString& context,
                   const QAction* action);

private:
  class ShortcutItem {
  public:
    explicit ShortcutItem(QAction* act);

    QAction* action() { return m_action; }
    const QAction* action() const { return m_action; }

    QString customShortcut() const { return m_customShortcut; }
    void setCustomShortcut(const QString& shortcut);

    void revertCustomShortcut();

    void assignCustomShortcut();

    bool isCustomShortcutChanged() const {
      return m_customShortcut != m_oldCustomShortcut ||
          m_customShortcut.isNull() != m_oldCustomShortcut.isNull();
    }

    QString activeShortcut() const {
      return m_customShortcut.isNull() ? m_defaultShortcut : m_customShortcut;
    }

    bool isCustomShortcutActive() const {
      return !m_customShortcut.isNull();
    }

    QString actionText() const;

  private:
    QAction* m_action;
    QString m_defaultShortcut;
    QString m_customShortcut;
    QString m_oldCustomShortcut;
  };

  class ShortcutGroup : public QList<ShortcutItem> {
  public:
    explicit ShortcutGroup(const QString& ctx);

    QString context() const { return m_context; }

  private:
    QString m_context;
  };

  const ShortcutGroup* shortcutGroupForIndex(const QModelIndex& index) const;

  QList<ShortcutGroup> m_shortcutGroups;
};

#endif // SHORTCUTSMODEL_H
