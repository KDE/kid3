/**
 * \file filterdialog.cpp
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

#include "filterdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QLineEdit>
#include <QComboBox>
#include <QToolTip>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "configstore.h"
#include "contexthelp.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
FilterDialog::FilterDialog(QWidget* parent) :
	QDialog(parent), m_aborted(false)
{
	setModal(true);
	setWindowTitle(i18n("Filter"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setMargin(6);
		vlayout->setSpacing(6);
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->setReadOnly(true);
			m_edit->setTabStopWidth(20);
			m_edit->setAcceptRichText(false);
			vlayout->addWidget(m_edit);
		}

		QGroupBox* fltbox = new QGroupBox(i18n("&Filter"), this);
		if (fltbox) {
			m_nameComboBox = new QComboBox(fltbox);
			m_nameComboBox->setEditable(true);
			m_filterLineEdit = new QLineEdit(fltbox);
			m_filterLineEdit->setToolTip(FileFilter::getFormatToolTip());
			QVBoxLayout* vbox = new QVBoxLayout;
			vbox->setMargin(2);
			vbox->addWidget(m_nameComboBox);
			vbox->addWidget(m_filterLineEdit);
			fltbox->setLayout(vbox);
			vlayout->addWidget(fltbox);
			connect(m_nameComboBox, SIGNAL(activated(int)), this,
							SLOT(setFilterLineEdit(int)));
		}

		QHBoxLayout* hlayout = new QHBoxLayout;
		if (hlayout) {
			hlayout->setSpacing(6);
			QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
			if (helpButton) {
				helpButton->setAutoDefault(false);
				hlayout->addWidget(helpButton);
				connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
			}
			QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
			if (saveButton) {
				saveButton->setAutoDefault(false);
				hlayout->addWidget(saveButton);
				connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
			}
			QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
			hlayout->addItem(hspacer);

			m_applyButton = new QPushButton(i18n("&Apply"), this);
			QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
			if (m_applyButton && closeButton) {
				m_applyButton->setAutoDefault(false);
				closeButton->setAutoDefault(false);
				hlayout->addWidget(m_applyButton);
				hlayout->addWidget(closeButton);
				connect(m_applyButton, SIGNAL(clicked()), this, SLOT(applyFilter()));
				connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
				connect(closeButton, SIGNAL(clicked()), this, SLOT(setAbortFlag()));
			}
			vlayout->addLayout(hlayout);
		}
	}
}

/**
 * Destructor.
 */
FilterDialog::~FilterDialog()
{}

/**
 * Apply filter.
 */
void FilterDialog::applyFilter()
{
	m_edit->clear();
	m_fileFilter.setFilterExpression(m_filterLineEdit->text());
	m_fileFilter.initParser();
	m_applyButton->setEnabled(false);
	emit apply(m_fileFilter);
	m_applyButton->setEnabled(true);
}

/**
 * Set the filter lineedit to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void FilterDialog::setFilterLineEdit(int index)
{
	if (index < static_cast<int>(m_filterNames.size())) {
		m_filterLineEdit->setText(m_filterExpressions[index]);
	} else {
		m_filterLineEdit->clear();
	}
}

/**
 * Set the filter combo box and line edit from the configuration.
 */
void FilterDialog::setFiltersFromConfig()
{
	m_filterNames = ConfigStore::s_filterCfg.m_filterNames;
	m_filterExpressions = ConfigStore::s_filterCfg.m_filterExpressions;
	m_nameComboBox->clear();
	m_nameComboBox->addItems(ConfigStore::s_filterCfg.m_filterNames);
	m_nameComboBox->setCurrentIndex(ConfigStore::s_filterCfg.m_filterIdx);
	setFilterLineEdit(ConfigStore::s_filterCfg.m_filterIdx);
}

/**
 * Read the local settings from the configuration.
 */
void FilterDialog::readConfig()
{
	clearAbortFlag();
	m_edit->clear();
	m_applyButton->setEnabled(true);

	setFiltersFromConfig();

	if (ConfigStore::s_filterCfg.m_windowWidth > 0 &&
			ConfigStore::s_filterCfg.m_windowHeight > 0) {
		resize(ConfigStore::s_filterCfg.m_windowWidth,
					 ConfigStore::s_filterCfg.m_windowHeight);
	}
}

/**
 * Save the local settings to the configuration.
 */
void FilterDialog::saveConfig()
{
	ConfigStore::s_filterCfg.m_filterIdx = m_nameComboBox->currentIndex();
	if (ConfigStore::s_filterCfg.m_filterIdx <
			static_cast<int>(ConfigStore::s_filterCfg.m_filterNames.size())) {
		ConfigStore::s_filterCfg.m_filterNames[ConfigStore::s_filterCfg.m_filterIdx] =
			m_nameComboBox->currentText();
		ConfigStore::s_filterCfg.m_filterExpressions[ConfigStore::s_filterCfg.m_filterIdx] =
			m_filterLineEdit->text();
	} else {
		ConfigStore::s_filterCfg.m_filterIdx =
			ConfigStore::s_filterCfg.m_filterNames.size();
		ConfigStore::s_filterCfg.m_filterNames.append(m_nameComboBox->currentText());
		ConfigStore::s_filterCfg.m_filterExpressions.append(m_filterLineEdit->text());
	}
	ConfigStore::s_filterCfg.m_windowWidth = size().width();
	ConfigStore::s_filterCfg.m_windowHeight = size().height();

	setFiltersFromConfig();
}

/**
 * Show help.
 */
void FilterDialog::showHelp()
{
	ContextHelp::displayHelp("filter");
}

/**
 * Set abort flag.
 */
void FilterDialog::setAbortFlag()
{
	m_aborted = true;
}
