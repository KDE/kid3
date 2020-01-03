/**
 * \file checkablelistmodel.h
 * Proxy model to use QAbstractItemModel with QML.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 23 Sep 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include <QAbstractProxyModel>
#include <QItemSelectionModel>

/**
 * Proxy model to map a hierarchical item model to a list model suitable for
 * QML.
 *
 * Compared with VisualDataModel, this model has a selectionModel property,
 * which can be used to have multi selection and queried from delegates using
 * the checkState role. Other functions help to improve the support of
 * QAbstractItemModel in QML.
 */
class CheckableListModel : public QAbstractProxyModel {
  Q_OBJECT
  /** Source model, equivalent to the model property of VisualDataModel. */
  Q_PROPERTY(QObject* sourceModel
             READ sourceModel WRITE setSourceModelObject
             NOTIFY sourceModelChanged)
  /** Selection model used to store the selections with the checkState role. */
  Q_PROPERTY(QObject* selectionModel
             READ selectionModel WRITE setSelectionModelObject
             NOTIFY selectionModelChanged)
  /** Root node in the hierarchical source model. */
  Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex
             NOTIFY rootIndexChanged)
  /** Current row. */
  Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow
             NOTIFY currentRowChanged)
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit CheckableListModel(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~CheckableListModel() override = default;

  /**
   * Get selection model.
   * @return selection model.
   */
  QItemSelectionModel* selectionModel() const;

  /**
   * Set selection model.
   * @param selModel selection model to use
   */
  void setSelectionModel(QItemSelectionModel* selModel);

  /**
   * Get root node in hierarchical source model.
   * @return root model index.
   */
  QModelIndex rootIndex() const;

  /**
   * Set root node in hierarchical source model.
   * @param rootIndex root model index
   */
  void setRootIndex(const QModelIndex& rootIndex);

  /**
   * Get data for @a roleName and @a row from model.
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @return model data.
   */
  Q_INVOKABLE QVariant getDataValue(int row, const QByteArray& roleName) const;

  /**
   * Set data for @a roleName and @a row in model.
   * This method can be used to assign values in the model because this is not
   * supported by the role properties available in the delegate.
   *
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param value model data
   * @return true if ok.
   */
  Q_INVOKABLE bool setDataValue(int row, const QByteArray& roleName,
                                const QVariant& value);

  /**
   * Get model index in the source model.
   * @param row model row
   * @return model index.
   */
  Q_INVOKABLE QModelIndex modelIndex(int row) const;

  /**
   * Get parent of the current root index.
   * This can be used to go up in the hierarchy.
   * @return model index of parent.
   */
  Q_INVOKABLE QModelIndex parentModelIndex() const;

  /**
   * Check if a row has children in the source model.
   * This can be used to go down in the hierarchy using modelIndex().
   * @param row model row
   * @return true if the node has children.
   */
  Q_INVOKABLE bool hasModelChildren(int row) const;

  /**
   * Get the current row.
   * @return model row.
   */
  int currentRow() const;

  /**
   * Clear the selection and select @a row as the current index.
   * @param row row to select
   */
  void setCurrentRow(int row);

  /** Set item flags. */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  /** Get data for a given role. */
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  /** Set data for a given role. */
  virtual bool setData(const QModelIndex& index, const QVariant& value,
                       int role = Qt::EditRole) override;

  /** Set source model. */
  virtual void setSourceModel(QAbstractItemModel* srcModel) override;

  /** Get index in model. */
  virtual QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const override;

  /** Get parent model index. */
  virtual QModelIndex parent(const QModelIndex& child) const override;

  /** Get number of rows under given @a parent. */
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /** Get number of columns under given @a parent. */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /** Map proxy index to index of source model. */
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

  /** Map index of source model to proxy index. */
  virtual QModelIndex mapFromSource(const QModelIndex& srcIndex) const override;

signals:
  /** Emitted when source model is changed. */
  void sourceModelChanged();

  /** Emitted when selection model is changed. */
  void selectionModelChanged();

  /** Emitted when root index is changed. */
  void rootIndexChanged();

  /** Emitted when the current row is changed. */
  void currentRowChanged(int row);

private slots:
  void onModelAboutToBeReset();
  void onModelReset();
  void onDataChanged(const QModelIndex& topLeft,
                     const QModelIndex& bottomRight);
  void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
  void onRowsRemoved(const QModelIndex &parent, int first, int last);
  void onRowsAboutToBeInserted(const QModelIndex& parent, int first, int last);
  void onRowsInserted(const QModelIndex& parent, int first, int last);
  void onSelectionChanged(const QItemSelection& selected,
                          const QItemSelection& deselected);
  void onCurrentChanged(const QModelIndex& current,
                        const QModelIndex& previous);

private:
  void setSourceModelObject(QObject* obj);
  void setSelectionModelObject(QObject* obj);

  QItemSelectionModel* m_selModel;
  QPersistentModelIndex m_rootIndex;
};
