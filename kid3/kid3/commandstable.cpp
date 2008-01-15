/**
 * \file commandstable.cpp
 * Context menu commands configuration table.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Oct 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#include "commandstable.h"
#include "filelist.h"
#include <qtooltip.h>

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
 * @param parent parent widget
 */
#if QT_VERSION >= 0x040000
CommandsTable::CommandsTable(QWidget* parent) :
	QTableWidget(parent)
{
	setColumnCount(CI_NumColumns);
	setHorizontalHeaderLabels(
		QStringList() << i18n("Confirm") << i18n("Output") << i18n("Name") <<
		i18n("Command"));
	resizeColumnToContents(CI_Confirm);
	resizeColumnToContents(CI_Output);
	horizontalHeader()->setResizeMode(CI_Command, QHeaderView::Stretch);
	horizontalHeaderItem(CI_Command)->setToolTip(FileList::getFormatToolTip());
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(cellActivated(int, int)),
			this, SLOT(valueChanged(int, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(customContextMenu(const QPoint&)));
}
#else
CommandsTable::CommandsTable(QWidget* parent) :
	QTable(parent)
{
	setNumCols(CI_NumColumns);
	horizontalHeader()->setLabel(CI_Confirm, i18n("Confirm"));
	horizontalHeader()->setLabel(CI_Output, i18n("Output"));
	horizontalHeader()->setLabel(CI_Name, i18n("Name"));
	horizontalHeader()->setLabel(CI_Command, i18n("Command"));
	QToolTip::add(horizontalHeader(), FileList::getFormatToolTip());
	adjustColumn(CI_Confirm);
	adjustColumn(CI_Output);
	setColumnStretchable(CI_Command, true);

	connect(this, SIGNAL(valueChanged(int, int)),
			this, SLOT(valueChanged(int, int)));
	connect(this, SIGNAL(contextMenuRequested(int, int, const QPoint&)),
			this, SLOT(contextMenu(int, int, const QPoint&)));
}
#endif

/**
 * Destructor.
 */
CommandsTable::~CommandsTable() {}

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
void CommandsTable::valueChanged(int row, int col)
{
	QTableWidgetItem* twi;
	if (row == rowCount() - 1 && col == CI_Command &&
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
void CommandsTable::valueChanged(int row, int col)
{
	if (row == numRows() - 1 && col == CI_Command) {
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
void CommandsTable::addRow(int row)
{
#if QT_VERSION >= 0x040000
	insertRow(row + 1);

	QTableWidgetItem* twi;
	if ((twi = item(row + 1, CI_Confirm)) == 0) {
		twi = new QTableWidgetItem;
		setItem(row + 1, CI_Confirm, twi);
	}
	twi->setCheckState(Qt::Unchecked);

	if ((twi = item(row + 1, CI_Output)) == 0) {
		twi = new QTableWidgetItem;
		setItem(row + 1, CI_Output, twi);
	}
	twi->setCheckState(Qt::Unchecked);

	if ((twi = item(row + 1, CI_Name)) != 0)
		twi->setText("");
	else
		setItem(row + 1, CI_Name, new QTableWidgetItem(""));

	if ((twi = item(row + 1, CI_Command)) != 0)
		twi->setText("");
	else
		setItem(row + 1, CI_Command, new QTableWidgetItem(""));
#else
	insertRows(row + 1);
	setItem(row + 1, CI_Confirm, new QCheckTableItem(this, ""));
	setItem(row + 1, CI_Output, new QCheckTableItem(this, ""));
#endif
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void CommandsTable::deleteRow(int row)
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
void CommandsTable::clearRow(int row)
{
#if QT_VERSION >= 0x040000
	QTableWidgetItem* twi = item(row, CI_Name);
	if (twi) twi->setText("");
	twi = item(row, CI_Command);
	if (twi) twi->setText("");
	twi = item(row, CI_Confirm);
	if (twi) twi->setCheckState(Qt::Unchecked);
	twi = item(row, CI_Output);
	if (twi) twi->setCheckState(Qt::Unchecked);
#else
	setText(row, CI_Name, "");
	setText(row, CI_Command, "");
	QTableItem* ti = item(row, CI_Confirm);
	QCheckTableItem* cti = dynamic_cast<QCheckTableItem*>(ti);
	if (cti) {
		cti->setChecked(false);
	}
	ti = item(row, CI_Output);
	cti = dynamic_cast<QCheckTableItem*>(ti);
	if (cti) {
		cti->setChecked(false);
	}
#endif
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
#if QT_VERSION >= 0x040000
void CommandsTable::executeAction(QAction* action)
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
void CommandsTable::executeAction(QAction*) {}
#endif

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void CommandsTable::contextMenu(int row, int /* col */, const QPoint& pos)
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
void CommandsTable::customContextMenu(const QPoint& pos)
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
void CommandsTable::customContextMenu(const QPoint&) {}
#endif

/**
 * Set the table from the command list.
 *
 * @param cmdList command list
 */
void CommandsTable::setCommandList(const MiscConfig::MenuCommandList& cmdList)
{
#if QT_VERSION >= 0x040000
	setRowCount(0);
	int row = 0;
	for (MiscConfig::MenuCommandList::const_iterator it = cmdList.begin();
			 it != cmdList.end();
			 ++it) {
		if (!(*it).getCommand().isEmpty()) {
			insertRow(row);
			QTableWidgetItem* cti = new QTableWidgetItem;
			if (cti) {
				cti->setCheckState((*it).mustBeConfirmed() ? Qt::Checked : Qt::Unchecked);
				setItem(row, CI_Confirm, cti);
			}
			cti = new QTableWidgetItem;
			if (cti) {
				cti->setCheckState((*it).outputShown() ? Qt::Checked : Qt::Unchecked);
				setItem(row, CI_Output, cti);
			}
			cti = new QTableWidgetItem((*it).getName());
			if (cti) {
				setItem(row, CI_Name, cti);
			}
			cti = new QTableWidgetItem((*it).getCommand());
			if (cti) {
				setItem(row, CI_Command, cti);
			}
			++row;
		}
	}
	if (row == 0) {
		// no commands => show at least one row
		addRow(-1);
	}
#else
	setNumRows(0);
	int row = 0;
	for (MiscConfig::MenuCommandList::const_iterator it = cmdList.begin();
			 it != cmdList.end();
			 ++it) {
		if (!(*it).getCommand().isEmpty()) {
			insertRows(row);
			QCheckTableItem* cti = new QCheckTableItem(this, "");
			if (cti) {
				cti->setChecked((*it).mustBeConfirmed());
				setItem(row, CI_Confirm, cti);
			}
			cti = new QCheckTableItem(this, "");
			if (cti) {
				cti->setChecked((*it).outputShown());
				setItem(row, CI_Output, cti);
			}
			setText(row, CI_Name, (*it).getName());
			setText(row, CI_Command, (*it).getCommand());
			++row;
		}
	}
	if (row == 0) {
		// no commands => show at least one row
		addRow(-1);
	}
#endif
}

/**
 * Get the command list from the table.
 *
 * @param cmdList the command list is returned here
 */
void CommandsTable::getCommandList(MiscConfig::MenuCommandList& cmdList) const
{
#if QT_VERSION >= 0x040000
	cmdList.clear();
	int nrRows = rowCount();
	for (int row = 0; row < nrRows; ++row) {
		QTableWidgetItem* twi = item(row, CI_Command);
		if (twi) {
			QString cmd = twi->text();
			if (!cmd.isEmpty()) {
				twi = item(row, CI_Name);
				QString name;
				if (twi) name = twi->text();
				if (name.isEmpty()) {
					name = cmd;
				}
				bool confirm = false;
				bool showOutput = false;
				twi = item(row, CI_Confirm);
				if (twi && twi->checkState() == Qt::Checked) {
					confirm = true;
				}
				twi = item(row, CI_Output);
				if (twi && twi->checkState() == Qt::Checked) {
					showOutput = true;
				}
				cmdList.push_back(MiscConfig::MenuCommand(name, cmd, confirm, showOutput));
			}
		}
	}
#else
	cmdList.clear();
	int nrRows = numRows();
	for (int row = 0; row < nrRows; ++row) {
		QString cmd = text(row, CI_Command);
		if (!cmd.isEmpty()) {
			QString name = text(row, CI_Name);
			if (name.isEmpty()) {
				name = cmd;
			}
			bool confirm = false;
			bool showOutput = false;
			QTableItem* ti = item(row, CI_Confirm);
			QCheckTableItem* cti = dynamic_cast<QCheckTableItem*>(ti);
			if (cti) {
				if (cti->isChecked()) {
					confirm = true;
				}
			}
			ti = item(row, CI_Output);
			cti = dynamic_cast<QCheckTableItem*>(ti);
			if (cti) {
				if (cti->isChecked()) {
					showOutput = true;
				}
			}
			cmdList.push_back(MiscConfig::MenuCommand(name, cmd, confirm, showOutput));
		}
	}
#endif
}
