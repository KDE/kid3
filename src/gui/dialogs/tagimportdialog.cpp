/**
 * \file tagimportdialog.cpp
 * Dialog to import from other tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Jun 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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
#include <QPushButton>
#include "textimporter.h"
#include "importparser.h"
#include "trackdatamodel.h"
#include "configstore.h"
#include "contexthelp.h"
#include "formatlistedit.h"

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
  setObjectName(QLatin1String("TagImportDialog"));
  setWindowTitle(tr("Import from Tags"));
  setSizeGripEnabled(true);

  QVBoxLayout* vboxLayout = new QVBoxLayout(this);

  m_formatListEdit = new FormatListEdit(
        QStringList() << tr("Format:")
                      << tr("Source:")
                      << tr("Extraction:"),
        QStringList() << QString()
                      << TrackDataFormatReplacer::getToolTip()
                      << ImportParser::getFormatToolTip(),
        this);
  vboxLayout->addWidget(m_formatListEdit);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  buttonLayout->addWidget(helpButton);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  buttonLayout->addWidget(saveButton);
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
  buttonLayout->addStretch();
  QPushButton* applyButton = new QPushButton(tr("&Apply"), this);
  applyButton->setAutoDefault(false);
  buttonLayout->addWidget(applyButton);
  connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
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
  TextImporter::importFromTags(m_formatListEdit->getCurrentFormat(1),
                               m_formatListEdit->getCurrentFormat(2),
                               trackDataVector);
  m_trackDataModel->setTrackData(trackDataVector);
  emit trackDataUpdated();
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void TagImportDialog::setFormatFromConfig()
{
  m_formatListEdit->setFormats(
        QList<QStringList>() << ConfigStore::s_importCfg.m_importTagsNames
                             << ConfigStore::s_importCfg.m_importTagsSources
                             << ConfigStore::s_importCfg.m_importTagsExtractions,
        ConfigStore::s_importCfg.m_importTagsIdx);
}

/**
 * Save the local settings to the configuration.
 */
void TagImportDialog::saveConfig()
{
  QList<QStringList> formats = m_formatListEdit->getFormats(
        &ConfigStore::s_importCfg.m_importTagsIdx);
  ConfigStore::s_importCfg.m_importTagsNames = formats.at(0);
  ConfigStore::s_importCfg.m_importTagsSources = formats.at(1);
  ConfigStore::s_importCfg.m_importTagsExtractions = formats.at(2);

  setFormatFromConfig();
}

/**
 * Show help.
 */
void TagImportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("import-tags"));
}
