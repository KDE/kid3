/**
 * \file proxyitemselectionmodel.h
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

#ifndef PROXYITEMSELECTIONMODEL_H
#define PROXYITEMSELECTIONMODEL_H

#include <QItemSelectionModel>
#include "kid3api.h"

/**
 * Item selection model to share selection with proxy model.
 */
class KID3_CORE_EXPORT ProxyItemSelectionModel : public QItemSelectionModel {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param proxyModel proxy model
   * @param sourceSelectionModel item selection model for source model
   * @param parent parent object
   */
  ProxyItemSelectionModel(QAbstractItemModel* proxyModel,
                          QItemSelectionModel* sourceSelectionModel,
                          QObject* parent = nullptr);

  virtual ~ProxyItemSelectionModel() override = default;

  /**
   * Select item at @a index using @a command.
   * @param index index of item to select
   * @param command selection command
   */
  virtual void select(const QModelIndex& index,
                      QItemSelectionModel::SelectionFlags command) override;

  /**
   * Select @a selection using @a command.
   * @param selection item selection
   * @param command selection command
   */
  virtual void select(const QItemSelection& selection,
                      QItemSelectionModel::SelectionFlags command) override;

private slots:
  void onSelectionChanged(const QItemSelection& selected,
                          const QItemSelection& deselected);
  void onProxyCurrentChanged(const QModelIndex& current);
  void onCurrentChanged(const QModelIndex& current);
  void onModelChanged();

private:
  QItemSelection mapSelectionFromProxy(const QItemSelection& selection) const;
  QItemSelection mapSelectionFromModel(const QItemSelection& selection) const;

  QItemSelectionModel* m_proxySelectionModel;
  bool m_ignoreCurrentChanged;
};

#endif // PROXYITEMSELECTIONMODEL_H
