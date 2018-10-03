/**
 * \file proxyitemselectionmodel.cpp
 * Item selection model to share selection with proxy model.
 *
 * This is a stripped down version of KLinkItemSelectionModel from kitemmodels.
 * Copyright (C) 2010 Klarälvdalens Datakonsult AB,
 *     a KDAB Group company, info@kdab.net,
 *     author Stephen Kelly <stephen@kdab.com>
 * Copyright (c) 2016 Ableton AG <info@ableton.com>
 *     Author Stephen Kelly <stephen.kelly@ableton.com>
 * Copyright (C) 2018  Urs Fleisch
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.

 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.

 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "proxyitemselectionmodel.h"
#include <QAbstractProxyModel>

ProxyItemSelectionModel::ProxyItemSelectionModel(
    QAbstractItemModel* proxyModel, QItemSelectionModel* sourceSelectionModel,
    QObject* parent)
  : QItemSelectionModel(proxyModel, parent),
    m_proxySelectionModel(sourceSelectionModel), m_ignoreCurrentChanged(false)
{
  connect(this, &QItemSelectionModel::currentChanged,
          this, &ProxyItemSelectionModel::onCurrentChanged);
#if QT_VERSION >= 0x050500
  connect(this, &QItemSelectionModel::modelChanged,
          this, &ProxyItemSelectionModel::onModelChanged);
#endif

  connect(m_proxySelectionModel,
          &QItemSelectionModel::selectionChanged,
          this, &ProxyItemSelectionModel::onSelectionChanged);
  connect(m_proxySelectionModel,
          &QItemSelectionModel::currentChanged,
          this, &ProxyItemSelectionModel::onProxyCurrentChanged);
#if QT_VERSION >= 0x050500
  connect(m_proxySelectionModel, &QItemSelectionModel::modelChanged,
          this, &ProxyItemSelectionModel::onModelChanged);
#endif
  onModelChanged();
}

void ProxyItemSelectionModel::select(
    const QModelIndex& index, QItemSelectionModel::SelectionFlags command)
{
  if (m_ignoreCurrentChanged) {
    return;
  }
  QItemSelection itemSelection(index, index);
  QItemSelectionModel::select(itemSelection, command);
  if (index.isValid()) {
    m_proxySelectionModel->select(
          mapSelectionFromProxy(itemSelection), command);
  } else {
    m_proxySelectionModel->clearSelection();
  }
}

void ProxyItemSelectionModel::select(
    const QItemSelection& selection,
    QItemSelectionModel::SelectionFlags command)
{
  m_ignoreCurrentChanged = true;
  QItemSelection itemSelection = selection;
  QItemSelectionModel::select(itemSelection, command);
  m_proxySelectionModel->select(mapSelectionFromProxy(itemSelection), command);
  m_ignoreCurrentChanged = false;
}

void ProxyItemSelectionModel::onCurrentChanged(const QModelIndex& current)
{
  const QItemSelection selection =
      mapSelectionFromProxy(QItemSelection(current, current));
  if (selection.isEmpty()) {
    return;
  }
  m_proxySelectionModel->setCurrentIndex(selection.indexes().first(),
                                         QItemSelectionModel::NoUpdate);
}

void ProxyItemSelectionModel::onSelectionChanged(const QItemSelection& selected,
                                               const QItemSelection& deselected)
{
  QItemSelectionModel::select(mapSelectionFromModel(deselected),
                              QItemSelectionModel::Deselect);
  QItemSelectionModel::select(mapSelectionFromModel(selected),
                              QItemSelectionModel::Select);
}

void ProxyItemSelectionModel::onProxyCurrentChanged(const QModelIndex& current)
{
  const QItemSelection selection =
      mapSelectionFromModel(QItemSelection(current, current));
  if (selection.isEmpty()) {
    return;
  }
  setCurrentIndex(selection.indexes().first(), QItemSelectionModel::NoUpdate);
}

void ProxyItemSelectionModel::onModelChanged()
{
  if (!model() || !m_proxySelectionModel || !m_proxySelectionModel->model()) {
    return;
  }
  QItemSelectionModel::select(
        mapSelectionFromModel(m_proxySelectionModel->selection()),
        QItemSelectionModel::ClearAndSelect);
}

QItemSelection ProxyItemSelectionModel::mapSelectionFromProxy(
    const QItemSelection& selection) const
{
  if (selection.isEmpty() || !model()) {
    return QItemSelection();
  }

  Q_ASSERT(qobject_cast<const QAbstractProxyModel*>(model()));
  return static_cast<const QAbstractProxyModel*>(model())
      ->mapSelectionToSource(selection);
}

QItemSelection ProxyItemSelectionModel::mapSelectionFromModel(
    const QItemSelection& selection) const
{
  if (selection.isEmpty() || !model()) {
    return QItemSelection();
  }

  Q_ASSERT(qobject_cast<const QAbstractProxyModel*>(model()));
  return static_cast<const QAbstractProxyModel*>(model())
      ->mapSelectionFromSource(selection);
}
