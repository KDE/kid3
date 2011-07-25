/**
 * \file stringlistedit.cpp
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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

#include "stringlistedit.h"
#include <QPushButton>
#include <QLineEdit>
#include <QInputDialog>
#include <QLayout>
#include <QListView>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param model item model, e.g. a QStringListModel
 * @param parent parent widget
 */
StringListEdit::StringListEdit(QAbstractItemModel* model, QWidget* parent) :
  QWidget(parent)
{
  setObjectName("StringListEdit");
  QHBoxLayout* hlayout = new QHBoxLayout(this);
  m_stringListBox = new QListView(this);
  if (hlayout && m_stringListBox) {
    m_stringListBox->setModel(model);
    hlayout->setSpacing(6);
    hlayout->addWidget(m_stringListBox);
    QVBoxLayout* vlayout = new QVBoxLayout;
    m_addPushButton = new QPushButton(i18n("&Add..."), this);
    m_moveUpPushButton = new QPushButton(i18n("Move &Up"), this);
    m_moveDownPushButton = new QPushButton(i18n("Move &Down"), this);
    m_editPushButton = new QPushButton(i18n("&Edit..."), this);
    m_removePushButton = new QPushButton(i18n("&Remove"), this);
    if (vlayout && m_addPushButton && m_moveUpPushButton &&
        m_moveDownPushButton && m_editPushButton && m_removePushButton) {
      vlayout->addWidget(m_addPushButton);
      vlayout->addWidget(m_moveUpPushButton);
      vlayout->addWidget(m_moveDownPushButton);
      vlayout->addWidget(m_editPushButton);
      vlayout->addWidget(m_removePushButton);
      vlayout->addStretch();

      connect(m_addPushButton, SIGNAL(clicked()), this, SLOT(addItem()));
      connect(m_moveUpPushButton, SIGNAL(clicked()), this, SLOT(moveUpItem()));
      connect(m_moveDownPushButton, SIGNAL(clicked()), this, SLOT(moveDownItem()));
      connect(m_editPushButton, SIGNAL(clicked()), this, SLOT(editItem()));
      connect(m_removePushButton, SIGNAL(clicked()), this, SLOT(removeItem()));
      connect(m_stringListBox->selectionModel(),
              SIGNAL(currentChanged(QModelIndex,QModelIndex)),
              this, SLOT(setButtonEnableState()));

      setButtonEnableState();
      hlayout->addLayout(vlayout);
    }
  }
}

/**
 * Destructor.
 */
StringListEdit::~StringListEdit()
{
}

/**
 * Add a new item.
 */
void StringListEdit::addItem()
{
  bool ok;
  QString txt = QInputDialog::getText(
    this, i18n("Add Item"), QString::null, QLineEdit::Normal,
    QString::null, &ok);
  if (ok && !txt.isEmpty()) {
    QAbstractItemModel* model = m_stringListBox->model();
    int row = model->rowCount();
    model->insertRow(row);
    model->setData(model->index(row, 0), txt);
  }
}

/**
 * Remove the selected item.
 */
void StringListEdit::removeItem()
{
  QModelIndex index = m_stringListBox->currentIndex();
  if (index.isValid()) {
    QAbstractItemModel* model = m_stringListBox->model();
    model->removeRow(index.row());
    setButtonEnableState();
  }
}

/**
 * Edit the selected item.
 */
void StringListEdit::editItem()
{
  QModelIndex index = m_stringListBox->currentIndex();
  if (index.isValid()) {
    QAbstractItemModel* model = m_stringListBox->model();
    bool ok;
    QString txt = QInputDialog::getText(
      this, i18n("Edit Item"), QString::null, QLineEdit::Normal,
      model->data(index, Qt::EditRole).toString(), &ok);
    if (ok && !txt.isEmpty()) {
      model->setData(index, txt);
    }
  }
}

/**
 * Move the selected item up.
 */
void StringListEdit::moveUpItem()
{
  QModelIndex index = m_stringListBox->currentIndex();
  if (index.isValid() && index.row() > 0) {
    QAbstractItemModel* model = m_stringListBox->model();
    QString txt = model->data(index, Qt::EditRole).toString();
    model->removeRow(index.row());
    model->insertRow(index.row() - 1);
    QModelIndex newIndex = model->index(index.row() - 1, index.column());
    model->setData(newIndex, txt);
    m_stringListBox->setCurrentIndex(newIndex);
  }
}

/**
 * Move the selected item down.
 */
void StringListEdit::moveDownItem()
{
  QModelIndex index = m_stringListBox->currentIndex();
  QAbstractItemModel* model = m_stringListBox->model();
  if (index.isValid() && index.row() < model->rowCount() - 1) {
    QString txt = model->data(index, Qt::EditRole).toString();
    model->removeRow(index.row());
    model->insertRow(index.row() + 1);
    QModelIndex newIndex = model->index(index.row() + 1, index.column());
    model->setData(newIndex, txt);
    m_stringListBox->setCurrentIndex(newIndex);
  }
}

/**
 * Change state of buttons according to the current item and the count.
 */
void StringListEdit::setButtonEnableState()
{
  QModelIndex index = m_stringListBox->currentIndex();
  QAbstractItemModel* model = m_stringListBox->model();
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
