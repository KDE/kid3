/**
 * \file abstractlistedit.cpp
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

#include "abstractlistedit.h"
#include <QPushButton>
#include <QLayout>
#include <QAbstractItemView>

/**
 * Constructor.
 *
 * @param itemView item view, e.g. a QListView
 * @param model item model, e.g. a QStringListModel
 * @param parent parent widget
 */
AbstractListEdit::AbstractListEdit(QAbstractItemView* itemView,
                                   QAbstractItemModel* model, QWidget* parent)
  : QWidget(parent)
{
  setObjectName(QLatin1String("AbstractListEdit"));
  auto hlayout = new QHBoxLayout(this);
  m_itemView = itemView;
  m_itemView->setModel(model);
  hlayout->setContentsMargins(0, 0, 0, 0);
  hlayout->addWidget(m_itemView);
  auto vlayout = new QVBoxLayout;
  m_addPushButton = new QPushButton(tr("&Add..."), this);
  m_moveUpPushButton = new QPushButton(tr("Move &Up"), this);
  m_moveDownPushButton = new QPushButton(tr("Move &Down"), this);
  m_editPushButton = new QPushButton(tr("&Edit..."), this);
  m_removePushButton = new QPushButton(tr("&Remove"), this);
  vlayout->addWidget(m_addPushButton);
  vlayout->addWidget(m_moveUpPushButton);
  vlayout->addWidget(m_moveDownPushButton);
  vlayout->addWidget(m_editPushButton);
  vlayout->addWidget(m_removePushButton);
  vlayout->addStretch();

  connect(m_addPushButton, &QAbstractButton::clicked, this, &AbstractListEdit::addItem);
  connect(m_moveUpPushButton, &QAbstractButton::clicked, this, &AbstractListEdit::moveUpItem);
  connect(m_moveDownPushButton, &QAbstractButton::clicked, this, &AbstractListEdit::moveDownItem);
  connect(m_editPushButton, &QAbstractButton::clicked, this, &AbstractListEdit::editItem);
  connect(m_removePushButton, &QAbstractButton::clicked, this, &AbstractListEdit::removeItem);
  connect(m_itemView->selectionModel(),
          &QItemSelectionModel::currentChanged,
          this, &AbstractListEdit::setButtonEnableState);

  setButtonEnableState();
  hlayout->addLayout(vlayout);
}

/**
 * Disable editing of items.
 * When editing is disabled, the Add, Edit and Remove buttons are hidden.
 * @param disable true to disable, false (default) to enable editing.
 */
void AbstractListEdit::setEditingDisabled(bool disable)
{
  m_addPushButton->setHidden(disable);
  m_editPushButton->setHidden(disable);
  m_removePushButton->setHidden(disable);
}

/**
 * Set text for Add button.
 * @param text button text
 */
void AbstractListEdit::setAddButtonText(const QString& text)
{
  m_addPushButton->setText(text);
}

/**
 * Remove the selected item.
 */
void AbstractListEdit::removeItem()
{
  QModelIndex index = m_itemView->currentIndex();
  if (index.isValid()) {
    QAbstractItemModel* model = m_itemView->model();
    model->removeRow(index.row());
    setButtonEnableState();
  }
}

/**
 * Move the selected item up.
 */
void AbstractListEdit::moveUpItem()
{
  QModelIndex index = m_itemView->currentIndex();
  if (index.isValid() && index.row() > 0) {
    int row = index.row();
    QAbstractItemModel* model = m_itemView->model();
    const int numColumns = model->columnCount();
    QVector<QVariant> editValues(numColumns);
    QVector<QVariant> checkValues(numColumns);
    for (int column = 0; column < numColumns; ++column) {
      QModelIndex idx = model->index(row, column);
      editValues[column] = idx.data(Qt::EditRole);
      checkValues[column] = idx.data(Qt::CheckStateRole);
    }
    model->removeRow(row);
    model->insertRow(row - 1);
    for (int column = 0; column < numColumns; ++column) {
      QModelIndex idx = model->index(row - 1, column);
      model->setData(idx, editValues.at(column), Qt::EditRole);
      model->setData(idx, checkValues.at(column), Qt::CheckStateRole);
    }
    QModelIndex newIndex = model->index(row - 1, index.column());
    m_itemView->setCurrentIndex(newIndex);
  }
}

/**
 * Move the selected item down.
 */
void AbstractListEdit::moveDownItem()
{
  QModelIndex index = m_itemView->currentIndex();
  QAbstractItemModel* model = m_itemView->model();
  if (index.isValid() && index.row() < model->rowCount() - 1) {
    const int numColumns = model->columnCount();
    int row = index.row();
    QVector<QVariant> editValues(numColumns);
    QVector<QVariant> checkValues(numColumns);
    for (int column = 0; column < numColumns; ++column) {
      QModelIndex idx = model->index(row, column);
      editValues[column] = idx.data(Qt::EditRole);
      checkValues[column] = idx.data(Qt::CheckStateRole);
    }
    model->removeRow(row);
    model->insertRow(row + 1);
    for (int column = 0; column < numColumns; ++column) {
      QModelIndex idx = model->index(row + 1, column);
      model->setData(idx, editValues.at(column), Qt::EditRole);
      model->setData(idx, checkValues.at(column), Qt::CheckStateRole);
    }
    QModelIndex newIndex = model->index(row + 1, index.column());
    m_itemView->setCurrentIndex(newIndex);
  }
}

/**
 * Change state of buttons according to the current item and the count.
 */
void AbstractListEdit::setButtonEnableState()
{
  QModelIndex index = m_itemView->currentIndex();
  QAbstractItemModel* model = m_itemView->model();
  int idx = -1;
  if (index.isValid())
    idx = index.row();
  m_moveUpPushButton->setEnabled(idx > 0);
  m_moveDownPushButton->setEnabled(
      idx >= 0 &&
      idx < model->rowCount() - 1);
  m_editPushButton->setEnabled(idx >= 0);
  m_removePushButton->setEnabled(idx >= 0);
}

/**
 * Hide the Edit button.
 */
void AbstractListEdit::hideEditButton()
{
  m_editPushButton->hide();
}
