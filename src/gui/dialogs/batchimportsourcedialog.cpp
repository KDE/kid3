/**
 * \file batchimportdialog.cpp
 * Dialog to add or edit a batch import source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "batchimportsourcedialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QDialogButtonBox>

/**
 * Constructor.
 * @param parent parent widget
 */
BatchImportSourceDialog::BatchImportSourceDialog(QWidget* parent) :
  QDialog(parent)
{
  setObjectName(QLatin1String("BatchImportSourceDialog"));
  setWindowTitle(tr("Import Source"));
  setSizeGripEnabled(true);

  auto vlayout = new QVBoxLayout(this);

  auto serverLayout = new QHBoxLayout;
  QLabel* serverLabel = new QLabel(tr("&Server:"));
  serverLayout->addWidget(serverLabel);
  m_serverComboBox = new QComboBox;
  serverLabel->setBuddy(m_serverComboBox);
  serverLayout->addWidget(m_serverComboBox);
  vlayout->addLayout(serverLayout);

  auto accuracyLayout = new QHBoxLayout;
  QLabel* accuracyLabel = new QLabel(tr("&Accuracy:"));
  accuracyLayout->addWidget(accuracyLabel);
  m_accuracySpinBox = new QSpinBox;
  m_accuracySpinBox->setRange(0, 100);
  m_accuracySpinBox->setValue(75);
  accuracyLabel->setBuddy(m_accuracySpinBox);
  accuracyLayout->addWidget(m_accuracySpinBox);
  vlayout->addLayout(accuracyLayout);

  auto tagsCoverLayout = new QHBoxLayout;
  m_standardTagsCheckBox = new QCheckBox(tr("&Standard Tags"));
  m_standardTagsCheckBox->setChecked(true);
  m_additionalTagsCheckBox = new QCheckBox(tr("&Additional Tags"));
  m_additionalTagsCheckBox->setChecked(true);
  m_coverArtCheckBox = new QCheckBox(tr("C&over Art"));
  m_coverArtCheckBox->setChecked(true);
  tagsCoverLayout->addWidget(m_standardTagsCheckBox);
  tagsCoverLayout->addWidget(m_additionalTagsCheckBox);
  tagsCoverLayout->addWidget(m_coverArtCheckBox);
  vlayout->addLayout(tagsCoverLayout);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  vlayout->addWidget(buttonBox);
}

/**
 * Destructor.
 */
BatchImportSourceDialog::~BatchImportSourceDialog()
{
}

/**
 * Set names of import servers.
 * @param servers server names
 */
void BatchImportSourceDialog::setServerNames(const QStringList& servers)
{
  if (m_serverComboBox) {
    m_serverComboBox->clear();
    m_serverComboBox->addItems(servers);
    m_serverComboBox->setCurrentIndex(servers.size() - 1);
  }
}

/**
 * Fill batch import source from dialog controls.
 * @param source batch import source to be filled
 */
void BatchImportSourceDialog::getSource(BatchImportProfile::Source& source)
{
  source.setName(m_serverComboBox->currentText());
  source.setRequiredAccuracy(m_accuracySpinBox->value());
  source.enableStandardTags(m_standardTagsCheckBox->isChecked());
  source.enableAdditionalTags(m_additionalTagsCheckBox->isChecked());
  source.enableCoverArt(m_coverArtCheckBox->isChecked());
}

/**
 * Set dialog controls from batch import source.
 * @param source batch import source containing properties to be set
 */
void BatchImportSourceDialog::setSource(const BatchImportProfile::Source& source)
{
  int index = m_serverComboBox->findText(source.getName());
  if (index != -1) {
    m_serverComboBox->setCurrentIndex(index);
  }
  m_accuracySpinBox->setValue(source.getRequiredAccuracy());
  m_standardTagsCheckBox->setChecked(source.standardTagsEnabled());
  m_additionalTagsCheckBox->setChecked(source.additionalTagsEnabled());
  m_coverArtCheckBox->setChecked(source.coverArtEnabled());
}
