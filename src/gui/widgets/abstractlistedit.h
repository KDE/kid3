/**
 * \file abstractlistedit.h
 * Widget to edit a list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include <QWidget>

class QPushButton;
class QAbstractItemView;
class QAbstractItemModel;

/**
 * Widget to edit a string list.
 */
class AbstractListEdit : public QWidget {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param itemView item view, e.g. a QListView
   * @param model item model, e.g. a QStringListModel
   * @param parent parent widget
   */
  AbstractListEdit(QAbstractItemView* itemView,
                   QAbstractItemModel* model, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~AbstractListEdit() override = default;

  /**
   * Disable editing of items.
   * When editing is disabled, the Add, Edit and Remove buttons are hidden.
   * @param disable true to disable, false (default) to enable editing.
   */
  void setEditingDisabled(bool disable);

  /**
   * Set text for Add button.
   * @param text button text
   */
  void setAddButtonText(const QString& text);

public slots:
  /**
   * Add a new item.
   */
  virtual void addItem() = 0;

  /**
   * Remove the selected item.
   */
  void removeItem();

  /**
   * Edit the selected item.
   */
  virtual void editItem() = 0;

  /**
   * Move the selected item up.
   */
  void moveUpItem();

  /**
   * Move the selected item down.
   */
  void moveDownItem();

  /**
   * Change state of buttons according to the current item and the count.
   */
  void setButtonEnableState();

protected:
  /**
   * Get item view.
   * @return item view.
   */
  const QAbstractItemView* getItemView() const { return m_itemView; }

  /**
   * Hide the Edit button.
   */
  void hideEditButton();

private:
  QAbstractItemView* m_itemView;
  QPushButton* m_addPushButton;
  QPushButton* m_moveUpPushButton;
  QPushButton* m_moveDownPushButton;
  QPushButton* m_editPushButton;
  QPushButton* m_removePushButton;
};
