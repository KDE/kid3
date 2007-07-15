/**
 * \file commandstable.h
 * Context menu commands configuration table.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Oct 2005
 */

#ifndef COMMANDSTABLE_H
#define COMMANDSTABLE_H

#include "qtcompatmac.h"
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
