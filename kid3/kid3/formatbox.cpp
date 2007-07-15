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
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QVBoxLayout>
#else
#include <qtable.h>
#include <qpopupmenu.h>
#endif

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 */
FormatBox::FormatBox(const QString& title, QWidget* parent) :
#if QT_VERSION >= 0x040000
	QGroupBox(title, parent)
#else
	QGroupBox(5, Qt::Vertical, title, parent)
#endif
{
	m_formatEditingCheckBox = new QCheckBox(i18n("Format while editing"),
																					this);

	QLabel* caseConvLabel = new QLabel(this);
	caseConvLabel->setText(i18n("Case conversion:"));

	m_caseConvComboBox = new QComboBox(this);
	m_caseConvComboBox->setEditable(false);
	m_caseConvComboBox->clear();
	m_caseConvComboBox->QCM_insertItem(FormatConfig::NoChanges,
																		 i18n("No changes"));
	m_caseConvComboBox->QCM_insertItem(FormatConfig::AllLowercase,
																		 i18n("All lowercase"));
	m_caseConvComboBox->QCM_insertItem(FormatConfig::AllUppercase,
																		 i18n("All uppercase"));
	m_caseConvComboBox->QCM_insertItem(FormatConfig::FirstLetterUppercase,
																		 i18n("First letter uppercase"));
	m_caseConvComboBox->QCM_insertItem(FormatConfig::AllFirstLettersUppercase,
																		 i18n("All first letters uppercase"));

	m_strRepCheckBox = new QCheckBox(this);
	m_strRepCheckBox->setText(i18n("String replacement:"));
#if QT_VERSION >= 0x040000
	m_strReplTable = new QTableWidget(this);
	m_strReplTable->setRowCount(1);
	m_strReplTable->setColumnCount(2);
	m_strReplTable->setHorizontalHeaderLabels(
		QStringList() << i18n("From") << i18n("To"));
	m_strReplTable->resizeColumnToContents(0);
	QVBoxLayout* vbox = new QVBoxLayout;
	vbox->setMargin(2);
	vbox->addWidget(m_formatEditingCheckBox);
	vbox->addWidget(caseConvLabel);
	vbox->addWidget(m_caseConvComboBox);
	vbox->addWidget(m_strRepCheckBox);
	vbox->addWidget(m_strReplTable);
	setLayout(vbox);
	m_strReplTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_strReplTable, SIGNAL(cellActivated(int,int)),
			this, SLOT(valueChanged(int,int)));
	connect(m_strReplTable, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(customContextMenu(const QPoint&)));
#else
	m_strReplTable = new QTable(this);
	m_strReplTable->setNumRows(1);
	m_strReplTable->setNumCols(2);
	m_strReplTable->horizontalHeader()->setLabel(0, i18n("From"));
	m_strReplTable->horizontalHeader()->setLabel(1, i18n("To"));
	m_strReplTable->adjustColumn(0);
	connect(m_strReplTable, SIGNAL(valueChanged(int,int)),
			this, SLOT(valueChanged(int,int)));
	connect(m_strReplTable, SIGNAL(contextMenuRequested(int,int,const QPoint &)),
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
#if QT_VERSION >= 0x040000
void FormatBox::valueChanged(int row, int)
{
	QTableWidgetItem* twi;
	if (row == m_strReplTable->rowCount() - 1 &&
			(twi = m_strReplTable->item(row, 0)) != 0) {
		if (twi->text().isEmpty()) {
			if (row != 0) {
				deleteRow(row);
			}
		} else {
			insertRow(row);
		}
	}
}
#else
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
#endif

/**
 * Insert a new row into the table.
 *
 * @param row the new row is inserted after this row
 */
void FormatBox::insertRow(int row)
{
#if QT_VERSION >= 0x040000
	m_strReplTable->insertRow(row + 1);
	QTableWidgetItem* twi;
	if ((twi = m_strReplTable->item(row + 1, 0)) != 0)
		twi->setText("");
	else
		m_strReplTable->setItem(row + 1, 0, new QTableWidgetItem(""));

	if ((twi = m_strReplTable->item(row + 1, 1)) != 0)
		twi->setText("");
	else
		m_strReplTable->setItem(row + 1, 1, new QTableWidgetItem(""));
#else
	m_strReplTable->insertRows(row + 1);
#endif
}

/**
 * Delete a row from the table.
 *
 * @param row row to delete
 */
void FormatBox::deleteRow(int row)
{
#if QT_VERSION >= 0x040000
	if (m_strReplTable->rowCount() <= 1) return;
#endif
	m_strReplTable->removeRow(row);
}

/**
 * Clear a cell in the table.
 *
 * @param row_col cell (row << 8 + col) to delete
 */
void FormatBox::clearCell(int row_col)
{
#if QT_VERSION >= 0x040000
	QTableWidgetItem* twi = m_strReplTable->item((row_col >> 8) & 0xff, row_col & 0xff);
	if (twi) twi->setText("");
#else
	m_strReplTable->setText((row_col >> 8) & 0xff, row_col & 0xff, "");
#endif
}

/**
 * Execute a context menu action.
 *
 * @param action action of selected menu
 */
#if QT_VERSION >= 0x040000
void FormatBox::executeAction(QAction* action)
{
	if (action) {
		int param = action->data().toInt();
		int cmd = param & 3;
		param >>= 2;
		switch (cmd) {
			case 0:
				insertRow(param);
				break;
			case 1:
				deleteRow(param);
				break;
			case 2:
			default:
				clearCell(param);
				break;
		}
	}
}
#else
void FormatBox::executeAction(QAction*) {}
#endif

/**
 * Display context menu.
 *
 * @param row row at which context menu is displayed
 * @param col column at which context menu is displayed
 * @param pos position where context menu is drawn on screen
 */
void FormatBox::contextMenu(int row, int col, const QPoint& pos)
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
	if (row >= 0 && row <= 255 && col >= 0 && col <= 255) {
		action = menu.addAction(i18n("&Clear cell"));
		if (action) action->setData((((row << 8) + col) << 2) | 2);
	}
	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(executeAction(QAction*)));
#else
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
void FormatBox::customContextMenu(const QPoint& pos)
{
	QTableWidgetItem* item = m_strReplTable->itemAt(pos);
	if (item) {
#if QT_VERSION >= 0x040200
		contextMenu(item->row(), item->column(), m_strReplTable->mapToGlobal(pos));
#else
		contextMenu(m_strReplTable->currentRow(), m_strReplTable->currentColumn(),
								m_strReplTable->mapToGlobal(pos));
#endif
	}
}
#else
void FormatBox::customContextMenu(const QPoint&) {}
#endif

/**
 * Set the values from a format configuration.
 *
 * @param cfg format configuration
 */
void FormatBox::fromFormatConfig(const FormatConfig* cfg)
{
	int i;
	m_formatEditingCheckBox->setChecked(cfg->m_formatWhileEditing);
	m_caseConvComboBox->QCM_setCurrentIndex(cfg->m_caseConversion);
	m_strRepCheckBox->setChecked(cfg->m_strRepEnabled);
	QMap<QString, QString>::ConstIterator it;
#if QT_VERSION >= 0x040000
	QTableWidgetItem* twi;
	for (i = 0, it = cfg->m_strRepMap.begin();
		 it != cfg->m_strRepMap.end();
		 ++it, ++i) {
		if (m_strReplTable->rowCount() <= i) {
			m_strReplTable->insertRow(i);
		}
		if ((twi = m_strReplTable->item(i, 0)) != 0)
			twi->setText(it.key());
		else
			m_strReplTable->setItem(i, 0, new QTableWidgetItem(it.key()));

		if ((twi = m_strReplTable->item(i, 1)) != 0)
			twi->setText(*it);
		else
			m_strReplTable->setItem(i, 1, new QTableWidgetItem(*it));
	}
	if (m_strReplTable->rowCount() <= i) {
		m_strReplTable->insertRow(i);
	}
	// add an empty row as last row and remove all rows below
	if ((twi = m_strReplTable->item(i, 0)) != 0)
		twi->setText("");
	else
		m_strReplTable->setItem(i, 0, new QTableWidgetItem(""));

	if ((twi = m_strReplTable->item(i, 1)) != 0)
		twi->setText("");
	else
		m_strReplTable->setItem(i, 1, new QTableWidgetItem(""));

	int row = m_strReplTable->rowCount();
#else
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
#endif
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
		(FormatConfig::CaseConversion)m_caseConvComboBox->QCM_currentIndex();
	if (cfg->m_caseConversion >= FormatConfig::NumCaseConversions) {
		cfg->m_caseConversion = FormatConfig::NoChanges;
	}
	cfg->m_strRepEnabled = m_strRepCheckBox->isChecked();
	cfg->m_strRepMap.clear();
#if QT_VERSION >= 0x040000
	for (int i = 0; i < m_strReplTable->rowCount(); ++i) {
		QString key;
		QTableWidgetItem* twi;
		if ((twi = m_strReplTable->item(i, 0)) != 0 &&
				!(key = twi->text()).isEmpty() &&
				(twi = m_strReplTable->item(i, 1)) != 0) {
			cfg->m_strRepMap[key] = twi->text();
		}
	}
#else
	for (int i = 0; i < m_strReplTable->numRows(); ++i) {
		QString key(m_strReplTable->text(i, 0));
		if (!key.isNull() && !key.isEmpty()) {
			cfg->m_strRepMap[key] = m_strReplTable->text(i, 1);
		}
	}
#endif
}
