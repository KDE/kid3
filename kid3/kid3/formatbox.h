/**
 * \file formatbox.h
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#ifndef FORMATBOX_H
#define FORMATBOX_H

#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QGroupBox>
class QTableWidget;
#else
#include <qgroupbox.h>
class QTable;
class QAction;
#endif

class QComboBox;
class QCheckBox;
class QString;
class QPoint;
class FormatConfig;

/**
 * Group box containing format options.
 */
class FormatBox : public QGroupBox
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param title  title
	 * @param parent parent widget
	 */
	FormatBox(const QString& title, QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	~FormatBox();

	/**
	 * Set the values from a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void fromFormatConfig(const FormatConfig* cfg);

	/**
	 * Store the values in a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void toFormatConfig(FormatConfig* cfg) const;

public slots:
	/**
	 * Called when a value in the string replacement table is changed.
	 * If the first cell in the last row is changed to a non-empty
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
	void insertRow(int row);

	/**
	 * Delete a row from the table.
	 *
	 * @param row row to delete
	 */
	void deleteRow(int row);

	/**
	 * Clear a cell in the table.
	 *
	 * @param row_col cell (row << 8 + col) to delete
	 */
	void clearCell(int row_col);

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

private:
	QComboBox* m_caseConvComboBox;
	QCheckBox* m_strRepCheckBox;
#if QT_VERSION >= 0x040000
	QTableWidget* m_strReplTable;
#else
	QTable* m_strReplTable;
#endif
	QCheckBox* m_formatEditingCheckBox;
};

#endif
