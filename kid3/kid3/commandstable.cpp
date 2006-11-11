/**
 * \file commandstable.cpp
 * Context menu commands configuration table.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Oct 2005
 */

#include "commandstable.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#if QT_VERSION >= 0x040000
#include <Q3PopupMenu>
#else 
#include <qpopupmenu.h>
#endif

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 * @param name   Qt object name
 */
CommandsTable::CommandsTable(QWidget* parent, const char* name) :
	Q3Table(parent, name) {
	setNumCols(2);
	horizontalHeader()->setLabel(0, i18n("Confirm"));
	horizontalHeader()->setLabel(1, i18n("Command"));
	adjustColumn(0);
	setColumnStretchable(1, true);

	connect(this, SIGNAL(valueChanged(int, int)),
			this, SLOT(valueChanged(int, int)));
	connect(this, SIGNAL(contextMenuRequested(int, int, const QPoint&)),
			this, SLOT(contextMenu(int, int, const QPoint&)));
}

/**
 * Destructor.
 */
CommandsTable::~CommandsTable() {}

/**
 * Called when a value in the table is changed.
 * If the second cell in the last row is changed to a non-empty
 * value, a new row is added. If it is changed to an empty value,
 * the row is deleted.
 *
 * @param row table row of changed item
 * @param col table column of changed item
 */
void CommandsTable::valueChanged(int row, int col)
{
	if (row == numRows() - 1 && col == 1) {
		if (text(row, col).isEmpty()) {
			if (row != 0) {
				deleteRow(row);
			}
		} else {
			insertRow(row);
		}
	}
}

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void CommandsTable::insertRow(int row)
{
	insertRows(row + 1);
	setItem(row + 1, 0, new Q3CheckTableItem(this, ""));
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void CommandsTable::deleteRow(int row)
{
	removeRow(row);
}

/**
 * Clear a row in the table.
 *
 * @param row row to clear
 */
void CommandsTable::clearRow(int row)
{
	setText(row, 1, "");
	Q3TableItem* ti = item(row, 0);
	Q3CheckTableItem* cti = dynamic_cast<Q3CheckTableItem*>(ti);
	if (cti) {
		cti->setChecked(false);
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
	Q3PopupMenu menu(this);

	if (row >= -1) {
		menu.insertItem(i18n("&Insert row"), this, SLOT(insertRow(int)), 0, 0);
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
	menu.setMouseTracking(true);
	menu.exec(pos);
}

/**
 * Set the table from the command list.
 *
 * @param cmdList command list
 */
void CommandsTable::setCommandList(const QStringList& cmdList)
{
	setNumRows(0);
	int row = 0;
	for (QStringList::const_iterator it = cmdList.begin();
			 it != cmdList.end();
			 ++it) {
		QString cmd = *it;
		bool confirm = false;
		if (cmd[0] == '!') {
			cmd = cmd.mid(1);
			confirm = true;
		}
		if (!cmd.isEmpty()) {
			insertRows(row);
			Q3CheckTableItem* cti = new Q3CheckTableItem(this, "");
			if (cti) {
				cti->setChecked(confirm);
				setItem(row, 0, cti);
			}
			setText(row, 1, cmd);
			++row;
		}
	}
	if (row == 0) {
		// no commands => show at least one row
		insertRow(-1);
	}
}

/**
 * Get the command list from the table.
 *
 * @param cmdList the command list is returned here
 */
void CommandsTable::getCommandList(QStringList& cmdList) const
{
	cmdList.clear();
	int nrRows = numRows();
	for (int row = 0; row < nrRows; ++row) {
		QString cmd = text(row, 1);
		if (!cmd.isEmpty()) {
			Q3TableItem* ti = item(row, 0);
			Q3CheckTableItem* cti = dynamic_cast<Q3CheckTableItem*>(ti);
			if (cti) {
				if (cti->isChecked()) {
					cmd = "!" + cmd;
				}
			}
			cmdList.push_back(cmd);
		}
	}
}
