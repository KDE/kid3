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
#include <QToolTip>

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
 * @param parent parent widget
 */
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

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void CommandsTable::addRow(int row)
{
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
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void CommandsTable::deleteRow(int row)
{
	if (rowCount() <= 1) return;
	removeRow(row);
}

/**
 * Clear a row in the table.
 *
 * @param row row to clear
 */
void CommandsTable::clearRow(int row)
{
	QTableWidgetItem* twi = item(row, CI_Name);
	if (twi) twi->setText("");
	twi = item(row, CI_Command);
	if (twi) twi->setText("");
	twi = item(row, CI_Confirm);
	if (twi) twi->setCheckState(Qt::Unchecked);
	twi = item(row, CI_Output);
	if (twi) twi->setCheckState(Qt::Unchecked);
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
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

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void CommandsTable::contextMenu(int row, int /* col */, const QPoint& pos)
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
void CommandsTable::customContextMenu(const QPoint& pos)
{
	QTableWidgetItem* item = itemAt(pos);
	if (item) {
		contextMenu(item->row(), item->column(), mapToGlobal(pos));
	}
}

/**
 * Set the table from the command list.
 *
 * @param cmdList command list
 */
void CommandsTable::setCommandList(const QList<MiscConfig::MenuCommand>& cmdList)
{
	setRowCount(0);
	int row = 0;
	for (QList<MiscConfig::MenuCommand>::const_iterator it = cmdList.begin();
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
}

/**
 * Get the command list from the table.
 *
 * @param cmdList the command list is returned here
 */
void CommandsTable::getCommandList(QList<MiscConfig::MenuCommand>& cmdList) const
{
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
}
