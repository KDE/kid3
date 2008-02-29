/**
 * \file commandstable.h
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

#ifndef COMMANDSTABLE_H
#define COMMANDSTABLE_H

#include "qtcompatmac.h"
/** The base class depends on the Qt version and is a table widget. */
#if QT_VERSION >= 0x040000
#include <QTableWidget>
typedef QTableWidget CommandsTableBaseClass;
#else 
#include <qtable.h>
typedef QTable CommandsTableBaseClass;
class QAction;
#endif
#include "miscconfig.h"

/**
 * Context menu commands configuration table.
 */
class CommandsTable : public CommandsTableBaseClass {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	CommandsTable(QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	virtual ~CommandsTable();

	/**
	 * Set the table from the command list.
	 *
	 * @param cmdList command list
	 */
	void setCommandList(const MiscConfig::MenuCommandList& cmdList);

	/**
	 * Get the command list from the table.
	 *
	 * @param cmdList the command list is returned here
	 */
	void getCommandList(MiscConfig::MenuCommandList& cmdList) const;

public slots:
	/**
	 * Called when a value in the table is changed.
	 * If the command cell in the last row is changed to a non-empty
	 * value, a new row is added. If it is changed to an empty value,
	 * the row is deleted.
	 *
	 * @param row table row of changed item
	 * @param col table column of changed item
	 */
	void valueChanged(int row, int col);

	/**
	 * Insert a new row into the table.
	 *
	 * @param row the new row is inserted after this row
	 */
	void addRow(int row);

	/**
	 * Delete a row from the table.
	 *
	 * @param row row to delete
	 */
	void deleteRow(int row);

	/**
	 * Clear a row in the table.
	 *
	 * @param row row to clear
	 */
	void clearRow(int row);

	/**
	 * Execute a context menu action.
	 *
	 * @param action action of selected menu
	 */
	void executeAction(QAction* action);

	/**
	 * Display context menu.
	 *
	 * @param row row at which context menu is displayed
	 * @param col column at which context menu is displayed
	 * @param pos position where context menu is drawn on screen
	 */
	void contextMenu(int row, int col, const QPoint& pos);

	/**
	 * Display custom context menu.
	 *
	 * @param pos position where context menu is drawn on screen
	 */
	void customContextMenu(const QPoint& pos);
};

#endif // COMMANDSTABLE_H
