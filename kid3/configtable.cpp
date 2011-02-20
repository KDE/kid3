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

#if QT_VERSION >= 0x040000
#include <QMenu>
#include <QHeaderView>
#else 
#include <qpopupmenu.h>
#endif

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
#if QT_VERSION >= 0x040000
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
#else
ConfigTable::ConfigTable(const QStringList& labels, QWidget* parent) :
	QTable(parent)
{
	const int numColumns = labels.size();
	setNumRows(1);
	setNumCols(numColumns);
	setColumnLabels(labels);
	for (int col = 0; col < numColumns; ++col) {
		setColumnStretchable(col, true);
	}
	connect(this, SIGNAL(valueChanged(int, int)),
			this, SLOT(valueChanged(int, int)));
	connect(this, SIGNAL(contextMenuRequested(int, int, const QPoint&)),
			this, SLOT(contextMenu(int, int, const QPoint&)));
}
#endif

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
#if QT_VERSION >= 0x040000
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
#else
void ConfigTable::valueChanged(int row, int col)
{
	if (row == numRows() - 1 && col == numCols() - 1) {
		if (text(row, col).isEmpty()) {
			if (row != 0) {
				deleteRow(row);
			}
		} else {
			addRow(row);
		}
	}
}
#endif

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void ConfigTable::addRow(int row)
{
#if QT_VERSION >= 0x040000
	insertRow(row + 1);

	QTableWidgetItem* twi;
	for (int col = 0; col < columnCount(); ++col) {
		if ((twi = item(row + 1, col)) != 0)
			twi->setText("");
		else
			setItem(row + 1, col, new QTableWidgetItem(""));
	}
#else
	insertRows(row + 1);
#endif
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void ConfigTable::deleteRow(int row)
{
#if QT_VERSION >= 0x040000
	if (rowCount() <= 1) return;
#endif
	removeRow(row);
}

/**
 * Clear a row in the table.
 *
 * @param row row to clear
 */
void ConfigTable::clearRow(int row)
{
#if QT_VERSION >= 0x040000
	QTableWidgetItem* twi;
	for (int col = 0; col < columnCount(); ++col) {
		twi = item(row, col);
		if (twi) twi->setText("");
	}
#else
	for (int col = 0; col < numCols(); ++col) {
		setText(row, col, "");
	}
#endif
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
#if QT_VERSION >= 0x040000
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
#else
void ConfigTable::executeAction(QAction*) {}
#endif

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void ConfigTable::contextMenu(int row, int /* col */, const QPoint& pos)
{
#if QT_VERSION >= 0x040000
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
#else
	QPopupMenu menu(this);
	if (row >= -1) {
		menu.insertItem(i18n("&Insert row"), this, SLOT(addRow(int)), 0, 0);
		menu.setItemParameter(0, row);
	}
	if (row >= 0) {
		menu.insertItem(i18n("&Delete row"), this, SLOT(deleteRow(int)), 0, 1);
		menu.setItemParameter(1, row);
	}
	if (row >= 0) {
		menu.insertItem(i18n("&Clear row"), this, SLOT(clearRow(int)), 0, 2);
		menu.setItemParameter(2, row);
	}
#endif
	menu.setMouseTracking(true);
	menu.exec(pos);
}

#if QT_VERSION >= 0x040000
/**
 * Display custom context menu.
 *
 * @param pos position where context menu is drawn on screen
 */
void ConfigTable::customContextMenu(const QPoint& pos)
{
	QTableWidgetItem* item = itemAt(pos);
	if (item) {
#if QT_VERSION >= 0x040200
		contextMenu(item->row(), item->column(), mapToGlobal(pos));
#else
		contextMenu(currentRow(), currentColumn(), mapToGlobal(pos));
#endif
	}
}
#else
void ConfigTable::customContextMenu(const QPoint&) {}
#endif

/**
 * Set the values from a map.
 *
 * @param map map with keys and values
 */
void ConfigTable::fromMap(const QMap<QString, QString>& map)
{
	int i;
	QMap<QString, QString>::ConstIterator it;
#if QT_VERSION >= 0x040000
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
#else
	if (numCols() < 2) return;
	for (i = 0, it = map.begin(); it != map.end(); ++it, ++i) {
		if (numRows() <= i) {
			insertRows(i);
		}
		setText(i, 0, it.key());
		setText(i, 1, it.data());
	}
	if (numRows() <= i) {
		insertRows(i);
	}
	// add an empty row as last row and remove all rows below
	setText(i, 0, "");
	setText(i, 1, "");
	int row = numRows();
#endif
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
#if QT_VERSION >= 0x040000
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
#else
	if (numCols() < 2) return;
	for (int i = 0; i < numRows(); ++i) {
		QString key(text(i, 0));
		if (!key.isNull() && !key.isEmpty()) {
			map[key] = text(i, 1);
		}
	}
#endif
}
