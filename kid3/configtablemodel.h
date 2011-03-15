/**
 * \file configtablemodel.h
 * Model for table with context menu to add and remove rows.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Mar 2011
 *
 * Copyright (C) 2005-2011  Urs Fleisch
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

#ifndef CONFIGTABLEMODEL_H
#define CONFIGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QStringList>
#include <QPair>
#include <QMap>

/**
 * Context menu commands configuration table model.
 */
class ConfigTableModel : public QAbstractTableModel {
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 */
	explicit ConfigTableModel(QObject* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~ConfigTableModel();

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
												int role=Qt::DisplayRole) const;

	/**
	 * Set data for a given role.
	 * @param index model index
	 * @param value data value
	 * @param role item data role
	 * @return true if successful
	 */
	virtual bool setData(const QModelIndex& index, const QVariant& value,
											 int role=Qt::EditRole);

	/**
	 * Get data for header section.
	 * @param section column or row
	 * @param orientation horizontal or vertical
	 * @param role item data role
	 * @return header data for role
	 */
	virtual QVariant headerData(int section, Qt::Orientation orientation,
															int role=Qt::DisplayRole) const;

	/**
	 * Set data for header section.
	 * Not supported.
	 * @return false
	 */
	virtual bool setHeaderData(int, Qt::Orientation, const QVariant&,
														 int=Qt::EditRole) { return false; }

	/**
	 * Get number of rows.
	 * @param parent parent model index, invalid for table models
	 * @return number of rows,
	 * if parent is valid number of children (0 for table models)
	 */
	virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;

	/**
	 * Get number of columns.
	 * @param parent parent model index, invalid for table models
	 * @return number of columns,
	 * if parent is valid number of children (0 for table models)
	 */
	virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;

	/**
	 * Insert rows.
	 * @param row rows are inserted before this row, if 0 at the begin,
	 * if rowCount() at the end
	 * @param count number of rows to insert
	 * @param parent parent model index, invalid for table models
	 * @return true if successful
	 */
	virtual bool insertRows(int row, int count,
													const QModelIndex& parent=QModelIndex());

	/**
	 * Remove rows.
	 * @param row rows are removed starting with this row
	 * @param count number of rows to remove
	 * @param parent parent model index, invalid for table models
	 * @return true if successful
	 */
	virtual bool removeRows(int row, int count,
													const QModelIndex& parent=QModelIndex());

	/**
	 * Get the resize modes to be used for the columns.
	 * @return list of resize modes for the columns
	 */
	QList<QHeaderView::ResizeMode> getHorizontalResizeModes() const;

	/**
	 * Set the column labels.
	 * @param labels column labels
	 */
	void setLabels(const QStringList& labels);

	/**
	 * Set the model from a map.
	 * @param map map with keys and values
	 */
	void setMap(const QMap<QString, QString>& map);

	/**
	 * Get map from the model.
	 * @return map with keys and values
	 */
	QMap<QString, QString> getMap() const;

private:
	QStringList m_labels;
	QList<QPair<QString, QString> > m_keyValues;
};

#endif // CONFIGTABLEMODEL_H
