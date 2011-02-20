/**
 * \file configtable.cpp
 * Context menu commands configuration table.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jan 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#include "configtable.h"

#include <QMenu>
#include <QHeaderView>

/** Column indices. */
enum ColumnIndex {
	CI_Confirm,
	CI_Output,
	CI_Name,
	CI_Command,
	CI_NumColumns
};

/**
 * Constructor.
 *
 * @param labels column labels
 * @param parent parent widget
 */
ConfigTable::ConfigTable(const QStringList& labels, QWidget* parent) :
	QTableWidget(parent)
{
	const int numColumns = labels.size();
	setRowCount(1);
	setColumnCount(numColumns);
	setHorizontalHeaderLabels(labels);
	horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(cellActivated(int, int)),
			this, SLOT(valueChanged(int, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(customContextMenu(const QPoint&)));
}

/**
 * Destructor.
 */
ConfigTable::~ConfigTable() {}

/**
 * Called when a value in the table is changed.
 * If the command cell in the last row is changed to a non-empty
 * value, a new row is added. If it is changed to an empty value,
 * the row is deleted.
 *
 * @param row table row of changed item
 * @param col table column of changed item
 */
void ConfigTable::valueChanged(int row, int col)
{
	QTableWidgetItem* twi;
	if (row == rowCount() - 1 && col == columnCount() - 1 &&
			(twi = item(row, col)) != 0) {
		if (twi->text().isEmpty()) {
			if (row != 0) {
				deleteRow(row);
			}
		} else {
			addRow(row);
		}
	}
}

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void ConfigTable::addRow(int row)
{
	insertRow(row + 1);

	QTableWidgetItem* twi;
	for (int col = 0; col < columnCount(); ++col) {
		if ((twi = item(row + 1, col)) != 0)
			twi->setText("");
		else
			setItem(row + 1, col, new QTableWidgetItem(""));
	}
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void ConfigTable::deleteRow(int row)
{
	if (rowCount() <= 1) return;
	removeRow(row);
}

/**
 * Clear a row in the table.
 *
 * @param row row to clear
 */
void ConfigTable::clearRow(int row)
{
	QTableWidgetItem* twi;
	for (int col = 0; col < columnCount(); ++col) {
		twi = item(row, col);
		if (twi) twi->setText("");
	}
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
void ConfigTable::executeAction(QAction* action)
{
	if (action) {
		int row = action->data().toInt();
		int cmd = row & 3;
		row >>= 2;
		switch (cmd) {
			case 0:
				addRow(row);
				break;
			case 1:
				deleteRow(row);
				break;
			case 2:
			default:
				clearRow(row);
				break;
		}
	}
}

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void ConfigTable::contextMenu(int row, int /* col */, const QPoint& pos)
{
	QMenu menu(this);
	QAction* action;
	if (row >= -1) {
		action = menu.addAction(i18n("&Insert row"));
		if (action) action->setData((row << 2) | 0);
	}
	if (row >= 0) {
		action = menu.addAction(i18n("&Delete row"));
		if (action) action->setData((row << 2) | 1);
	}
	if (row >= 0) {
		action = menu.addAction(i18n("&Clear row"));
		if (action) action->setData((row << 2) | 2);
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
	menu.setMouseTracking(true);
	menu.exec(pos);
}

/**
 * Display custom context menu.
 *
 * @param pos position where context menu is drawn on screen
 */
void ConfigTable::customContextMenu(const QPoint& pos)
{
	QTableWidgetItem* item = itemAt(pos);
	if (item) {
		contextMenu(item->row(), item->column(), mapToGlobal(pos));
	}
}

/**
 * Set the values from a map.
 *
 * @param map map with keys and values
 */
void ConfigTable::fromMap(const QMap<QString, QString>& map)
{
	int i;
	QMap<QString, QString>::ConstIterator it;
	if (columnCount() < 2) return;
	QTableWidgetItem* twi;
	for (i = 0, it = map.begin(); it != map.end(); ++it, ++i) {
		if (rowCount() <= i) {
			insertRow(i);
		}
		if ((twi = item(i, 0)) != 0)
			twi->setText(it.key());
		else
			setItem(i, 0, new QTableWidgetItem(it.key()));

		if ((twi = item(i, 1)) != 0)
			twi->setText(*it);
		else
			setItem(i, 1, new QTableWidgetItem(*it));
	}
	if (rowCount() <= i) {
		insertRow(i);
	}
	// add an empty row as last row and remove all rows below
	if ((twi = item(i, 0)) != 0)
		twi->setText("");
	else
		setItem(i, 0, new QTableWidgetItem(""));

	if ((twi = item(i, 1)) != 0)
		twi->setText("");
	else
		setItem(i, 1, new QTableWidgetItem(""));

	int row = rowCount();
	while (--row > i) {
		removeRow(row);
	}
}

/**
 * Store the values in a map.
 *
 * @param map to be filled
 */
void ConfigTable::toMap(QMap<QString, QString>& map) const
{
	map.clear();
	if (columnCount() < 2) return;
	for (int i = 0; i < rowCount(); ++i) {
		QString key;
		QTableWidgetItem* twi;
		if ((twi = item(i, 0)) != 0 &&
				!(key = twi->text()).isEmpty() &&
				(twi = item(i, 1)) != 0) {
			map[key] = twi->text();
		}
	}
}
