/**
 * \file formatbox.h
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef FORMATBOX_H
#define FORMATBOX_H

#include <qgroupbox.h>

class QComboBox;
class QCheckBox;
class QTable;
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
	 * @param name   Qt object name
	 */
	FormatBox(const QString & title, QWidget *parent = 0, const char *name = 0);
	/**
	 * Destructor.
	 */
	~FormatBox();
	/**
	 * Set the values from a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void fromFormatConfig(const FormatConfig *cfg);
	/**
	 * Store the values in a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void toFormatConfig(FormatConfig *cfg) const;
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
	 * Display context menu.
	 *
	 * @param row row at which context menu is displayed
	 * @param col column at which context menu is displayed
	 * @param pos position where context menu is drawn on screen
	 */
	void contextMenu(int row, int col, const QPoint & pos);
private:
	QComboBox *caseConvComboBox;
	QCheckBox *strRepCheckBox;
	QTable *strReplTable;
	QCheckBox* m_formatEditingCheckBox;
};

#endif
