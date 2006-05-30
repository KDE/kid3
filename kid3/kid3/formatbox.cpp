/**
 * \file formatbox.cpp
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtable.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include "formatconfig.h"
#include "formatbox.h"

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 * @param name   Qt object name
 */
FormatBox::FormatBox(const QString & title, QWidget *parent, const char *name) :
	QGroupBox(5, Qt::Vertical, title, parent, name)
{
	m_formatEditingCheckBox = new QCheckBox(i18n("Format while editing"),
																					this, "formatEditingCheckBox");

	QLabel *caseConvLabel = new QLabel(this, "caseConvLabel");
	caseConvLabel->setText(i18n("Case Conversion:"));

	caseConvComboBox = new QComboBox(FALSE, this, "caseConvComboBox");
	caseConvComboBox->clear();
	caseConvComboBox->insertItem(i18n("No changes"),
								 FormatConfig::NoChanges);
	caseConvComboBox->insertItem(i18n("All lowercase"),
								 FormatConfig::AllLowercase);
	caseConvComboBox->insertItem(i18n("All uppercase"),
								 FormatConfig::AllUppercase);
	caseConvComboBox->insertItem(i18n("First letter uppercase"),
								 FormatConfig::FirstLetterUppercase);
	caseConvComboBox->insertItem(i18n("All first letters uppercase"),
								 FormatConfig::AllFirstLettersUppercase);

	strRepCheckBox = new QCheckBox(this, "strRepCheckBox");
	strRepCheckBox->setText(i18n("String Replacement:"));
	strReplTable = new QTable(this, "strReplTable");
	strReplTable->setNumRows(1);
	strReplTable->setNumCols(2);
	strReplTable->horizontalHeader()->setLabel(0, i18n("From"));
	strReplTable->horizontalHeader()->setLabel(1, i18n("To"));
	strReplTable->adjustColumn(0);
	connect(strReplTable, SIGNAL(valueChanged(int,int)),
			this, SLOT(valueChanged(int,int)));
#if QT_VERSION >= 300
	connect(strReplTable, SIGNAL(contextMenuRequested(int,int,const QPoint &)),
			this, SLOT(contextMenu(int,int,const QPoint&)));
#endif
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
	if (row == strReplTable->numRows() - 1 && col == 0) {
		if (strReplTable->text(row, col).isEmpty()) {
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
#if QT_VERSION >= 300
	strReplTable->insertRows(row + 1);
#else
	strReplTable->setNumRows(row + 2);
#endif
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void FormatBox::deleteRow(int row)
{
#if QT_VERSION >= 300
	strReplTable->removeRow(row);
#else
	strReplTable->setNumRows(row);
#endif
}

/**
 * Clear a cell in the table.
 *
 * @param row_col cell (row << 8 + col) to delete
 */
void FormatBox::clearCell(int row_col)
{
	strReplTable->setText((row_col >> 8) & 0xff, row_col & 0xff, "");
}

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void FormatBox::contextMenu(int row, int col, const QPoint &pos)
{
	QPopupMenu menu(this);

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
void FormatBox::fromFormatConfig(const FormatConfig *cfg)
{
	int i;
	m_formatEditingCheckBox->setChecked(cfg->m_formatWhileEditing);
	caseConvComboBox->setCurrentItem(cfg->caseConversion);
	strRepCheckBox->setChecked(cfg->strRepEnabled);
	QMap<QString, QString>::ConstIterator it;
	for (i = 0, it = cfg->strRepMap.begin();
		 it != cfg->strRepMap.end();
		 ++it, ++i) {
		if (strReplTable->numRows() <= i) {
#if QT_VERSION >= 300
			strReplTable->insertRows(i);
#else
			strReplTable->setNumRows(i + 1);
#endif
		}
		strReplTable->setText(i, 0, it.key());
		strReplTable->setText(i, 1, it.data());
	}
	if (strReplTable->numRows() <= i) {
#if QT_VERSION >= 300
		strReplTable->insertRows(i);
#else
		strReplTable->setNumRows(i + 1);
#endif
	}
	// add an empty row as last row and remove all rows below
	strReplTable->setText(i, 0, "");
	strReplTable->setText(i, 1, "");
	int row = strReplTable->numRows();
#if QT_VERSION >= 300
	while (--row > i) {
		strReplTable->removeRow(row);
	}
#else
	strReplTable->setNumRows(i + 1);
#endif
}

/**
 * Store the values in a format configuration.
 *
 * @param cfg format configuration
 */
void FormatBox::toFormatConfig(FormatConfig *cfg) const
{
	cfg->m_formatWhileEditing = m_formatEditingCheckBox->isChecked();
	cfg->caseConversion =
		(FormatConfig::CaseConversion)caseConvComboBox->currentItem();
	if (cfg->caseConversion >= FormatConfig::NumCaseConversions) {
		cfg->caseConversion = FormatConfig::NoChanges;
	}
	cfg->strRepEnabled = strRepCheckBox->isOn();
	cfg->strRepMap.clear();
	for (int i = 0; i < strReplTable->numRows(); ++i) {
		QString key(strReplTable->text(i, 0));
		if (!key.isNull() && !key.isEmpty()) {
			cfg->strRepMap[key] = strReplTable->text(i, 1);
		}
	}
}
