/**
 * \file formatbox.cpp
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qstring.h>
#include "formatconfig.h"
#include "formatbox.h"
#if QT_VERSION >= 0x040000
#include <Q3Table>
#include <Q3PopupMenu>
#else
#include <qtable.h>
#include <qpopupmenu.h>
#endif

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 * @param name   Qt object name
 */
FormatBox::FormatBox(const QString& title, QWidget* parent, const char* name) :
	Q3GroupBox(5, Qt::Vertical, title, parent, name)
{
	m_formatEditingCheckBox = new QCheckBox(i18n("Format while editing"),
																					this, "formatEditingCheckBox");

	QLabel* caseConvLabel = new QLabel(this, "caseConvLabel");
	caseConvLabel->setText(i18n("Case Conversion:"));

	m_caseConvComboBox = new QComboBox(false, this, "caseConvComboBox");
	m_caseConvComboBox->clear();
	m_caseConvComboBox->insertItem(i18n("No changes"),
								 FormatConfig::NoChanges);
	m_caseConvComboBox->insertItem(i18n("All lowercase"),
								 FormatConfig::AllLowercase);
	m_caseConvComboBox->insertItem(i18n("All uppercase"),
								 FormatConfig::AllUppercase);
	m_caseConvComboBox->insertItem(i18n("First letter uppercase"),
								 FormatConfig::FirstLetterUppercase);
	m_caseConvComboBox->insertItem(i18n("All first letters uppercase"),
								 FormatConfig::AllFirstLettersUppercase);

	m_strRepCheckBox = new QCheckBox(this, "strRepCheckBox");
	m_strRepCheckBox->setText(i18n("String Replacement:"));
	m_strReplTable = new Q3Table(this, "strReplTable");
	m_strReplTable->setNumRows(1);
	m_strReplTable->setNumCols(2);
	m_strReplTable->horizontalHeader()->setLabel(0, i18n("From"));
	m_strReplTable->horizontalHeader()->setLabel(1, i18n("To"));
	m_strReplTable->adjustColumn(0);
	connect(m_strReplTable, SIGNAL(valueChanged(int,int)),
			this, SLOT(valueChanged(int,int)));
	connect(m_strReplTable, SIGNAL(contextMenuRequested(int,int,const QPoint &)),
			this, SLOT(contextMenu(int,int,const QPoint&)));
}

/**
 * Destructor.
 */
FormatBox::~FormatBox() {}

/**
 * Called when a value in the string replacement table is changed.
 * If the first cell in the last row is changed to a non-empty
 * value, a new row is added. If it is changed to an empty value,
 * the row is deleted.
 *
 * @param row table row of changed item
 * @param col table column of changed item
 */
void FormatBox::valueChanged(int row, int col)
{
	if (row == m_strReplTable->numRows() - 1 && col == 0) {
		if (m_strReplTable->text(row, col).isEmpty()) {
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
void FormatBox::insertRow(int row)
{
	m_strReplTable->insertRows(row + 1);
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void FormatBox::deleteRow(int row)
{
	m_strReplTable->removeRow(row);
}

/**
 * Clear a cell in the table.
 *
 * @param row_col cell (row << 8 + col) to delete
 */
void FormatBox::clearCell(int row_col)
{
	m_strReplTable->setText((row_col >> 8) & 0xff, row_col & 0xff, "");
}

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void FormatBox::contextMenu(int row, int col, const QPoint& pos)
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
	if (row >= 0 && row <= 255 && col >= 0 && col <= 255) {
		menu.insertItem(i18n("&Clear cell"), this, SLOT(clearCell(int)), 0, 2);
		menu.setItemParameter(2, (row << 8) + col);
	}
	menu.setMouseTracking(true);
	menu.exec(pos);
}

/**
 * Set the values from a format configuration.
 *
 * @param cfg format configuration
 */
void FormatBox::fromFormatConfig(const FormatConfig* cfg)
{
	int i;
	m_formatEditingCheckBox->setChecked(cfg->m_formatWhileEditing);
	m_caseConvComboBox->setCurrentItem(cfg->m_caseConversion);
	m_strRepCheckBox->setChecked(cfg->m_strRepEnabled);
	QMap<QString, QString>::ConstIterator it;
	for (i = 0, it = cfg->m_strRepMap.begin();
		 it != cfg->m_strRepMap.end();
		 ++it, ++i) {
		if (m_strReplTable->numRows() <= i) {
			m_strReplTable->insertRows(i);
		}
		m_strReplTable->setText(i, 0, it.key());
		m_strReplTable->setText(i, 1, it.data());
	}
	if (m_strReplTable->numRows() <= i) {
		m_strReplTable->insertRows(i);
	}
	// add an empty row as last row and remove all rows below
	m_strReplTable->setText(i, 0, "");
	m_strReplTable->setText(i, 1, "");
	int row = m_strReplTable->numRows();
	while (--row > i) {
		m_strReplTable->removeRow(row);
	}
}

/**
 * Store the values in a format configuration.
 *
 * @param cfg format configuration
 */
void FormatBox::toFormatConfig(FormatConfig* cfg) const
{
	cfg->m_formatWhileEditing = m_formatEditingCheckBox->isChecked();
	cfg->m_caseConversion =
		(FormatConfig::CaseConversion)m_caseConvComboBox->currentItem();
	if (cfg->m_caseConversion >= FormatConfig::NumCaseConversions) {
		cfg->m_caseConversion = FormatConfig::NoChanges;
	}
	cfg->m_strRepEnabled = m_strRepCheckBox->isOn();
	cfg->m_strRepMap.clear();
	for (int i = 0; i < m_strReplTable->numRows(); ++i) {
		QString key(m_strReplTable->text(i, 0));
		if (!key.isNull() && !key.isEmpty()) {
			cfg->m_strRepMap[key] = m_strReplTable->text(i, 1);
		}
	}
}
