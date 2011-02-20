/**
 * \file filterdialog.h
 * Filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
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

#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <qdialog.h>
#include <qstringlist.h>
#include <qtextedit.h>
#include "filefilter.h"

class QLineEdit;
class QComboBox;
class QPushButton;

/**
 * Filter dialog.
 */
class FilterDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	FilterDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	~FilterDialog();

	/**
	 * Read the local settings from the configuration.
	 */
	void readConfig();

	/**
	 * Display information in text view.
	 *
	 * @param text text to display
	 */
	void showInformation(const QString& text) { m_edit->append(text); }

	/**
	 * Clear abort flag.
	 */
	void clearAbortFlag() { m_aborted = false; }

	/**
	 * Check if dialog was aborted.
	 * @return true if aborted.
	 */
	bool getAbortFlag() { return m_aborted; }

signals:
	/**
	 * Is triggered when the selected @a filter has to be applied.
	 */
	void apply(FileFilter&);

public slots:
	/**
	 * Set the filter lineedit to the filter selected in the combo box.
	 *
	 * @param index current index of the combo box
	 */
	void setFilterLineEdit(int index);

private slots:
	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

	/**
	 * Apply filter.
	 */
	void applyFilter();

	/**
	 * Set abort flag.
	 */
	void setAbortFlag();

private:
	/**
	 * Set the filter combo box and line edit from the configuration.
	 */
	void setFiltersFromConfig();

  /** Text editor */
  QTextEdit* m_edit;
	/** cobobox with filter names */
	QComboBox* m_nameComboBox;
	/** LineEdit for filter expression */
	QLineEdit* m_filterLineEdit;
	/** Apply button */
	QPushButton* m_applyButton;
	/** filter names */
	QStringList m_filterNames;
	/** filter expressions */
	QStringList m_filterExpressions;
	/** file filter used */
	FileFilter m_fileFilter;
	/** abort flag */
	bool m_aborted;
};

#endif
