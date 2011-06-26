/**
 * \file tagimportdialog.cpp
 * Dialog to import from other tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Jun 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include "tagimportdialog.h"
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "textimporter.h"
#include "importparser.h"
#include "trackdatamodel.h"
#include "kid3.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataModel track data to be filled with imported values
 */
TagImportDialog::TagImportDialog(QWidget* parent,
																 TrackDataModel* trackDataModel) :
	QDialog(parent), m_trackDataModel(trackDataModel)
{
	setObjectName("TagImportDialog");
	setWindowTitle(i18n("Import from Tags"));
	setSizeGripEnabled(true);

	QVBoxLayout* vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(6);

	m_formatComboBox = new QComboBox(this);
	m_formatComboBox->setEditable(true);
	m_sourceLineEdit = new QLineEdit(this);
	m_extractionLineEdit = new QLineEdit(this);
	m_sourceLineEdit->setToolTip(TrackDataFormatReplacer::getToolTip());
	m_extractionLineEdit->setToolTip(ImportParser::getFormatToolTip());
	connect(m_formatComboBox, SIGNAL(activated(int)),
					this, SLOT(setFormatLineEdit(int)));
	QFormLayout* formatLayout = new QFormLayout;
	formatLayout->addRow(i18n("Format:"), m_formatComboBox);
	formatLayout->addRow(i18n("Source:"), m_sourceLineEdit);
	formatLayout->addRow(i18n("Extraction:"), m_extractionLineEdit);
	vboxLayout->addLayout(formatLayout);

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	helpButton->setAutoDefault(false);
	buttonLayout->addWidget(helpButton);
	connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	saveButton->setAutoDefault(false);
	buttonLayout->addWidget(saveButton);
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
	buttonLayout->addStretch();
	QPushButton* applyButton = new QPushButton(i18n("&Apply"), this);
	applyButton->setAutoDefault(false);
	buttonLayout->addWidget(applyButton);
	connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
	QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
	closeButton->setAutoDefault(false);
	buttonLayout->addWidget(closeButton);
	connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
	vboxLayout->addLayout(buttonLayout);
}

/**
 * Destructor.
 */
TagImportDialog::~TagImportDialog()
{
}

/**
 * Clear dialog data.
 */
void TagImportDialog::clear()
{
	setFormatFromConfig();
}

/**
 * Apply import to track data.
 */
void TagImportDialog::apply()
{
	ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
	TextImporter::importFromTags(m_sourceLineEdit->text(),
															 m_extractionLineEdit->text(),
															 trackDataVector);
	m_trackDataModel->setTrackData(trackDataVector);
	emit trackDataUpdated();
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void TagImportDialog::setFormatFromConfig()
{
	m_formatSources = Kid3App::s_genCfg.m_importTagsSources;
	m_formatExtractions = Kid3App::s_genCfg.m_importTagsExtractions;
	m_formatComboBox->clear();
	m_formatComboBox->addItems(Kid3App::s_genCfg.m_importTagsNames);
	m_formatComboBox->setCurrentIndex(Kid3App::s_genCfg.m_importTagsIdx);
	setFormatLineEdit(Kid3App::s_genCfg.m_importTagsIdx);
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void TagImportDialog::setFormatLineEdit(int index)
{
	if (index < static_cast<int>(m_formatSources.size())) {
		m_sourceLineEdit->setText(m_formatSources[index]);
		m_extractionLineEdit->setText(m_formatExtractions[index]);
	} else {
		m_sourceLineEdit->clear();
		m_extractionLineEdit->clear();
	}
}

/**
 * Save the local settings to the configuration.
 */
void TagImportDialog::saveConfig()
{
	Kid3App::s_genCfg.m_importTagsIdx = m_formatComboBox->currentIndex();
	if (Kid3App::s_genCfg.m_importTagsIdx < static_cast<int>(Kid3App::s_genCfg.m_importTagsNames.size())) {
		Kid3App::s_genCfg.m_importTagsNames[Kid3App::s_genCfg.m_importTagsIdx] = m_formatComboBox->currentText();
		Kid3App::s_genCfg.m_importTagsSources[Kid3App::s_genCfg.m_importTagsIdx] = m_sourceLineEdit->text();
		Kid3App::s_genCfg.m_importTagsExtractions[Kid3App::s_genCfg.m_importTagsIdx] = m_extractionLineEdit->text();
	} else {
		Kid3App::s_genCfg.m_importTagsIdx = Kid3App::s_genCfg.m_importTagsNames.size();
		Kid3App::s_genCfg.m_importTagsNames.append(m_formatComboBox->currentText());
		Kid3App::s_genCfg.m_importTagsSources.append(m_sourceLineEdit->text());
		Kid3App::s_genCfg.m_importTagsExtractions.append(m_extractionLineEdit->text());
	}

	setFormatFromConfig();
}

/**
 * Show help.
 */
void TagImportDialog::showHelp()
{
	Kid3App::displayHelp("import-tags");
}
