/**
 * \file tagimportdialog.cpp
 * Dialog to import from other tags.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 20 Jun 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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
#include <QFormLayout>
#include <QComboBox>
#include <QCoreApplication>
#include "textimporter.h"
#include "importparser.h"
#include "trackdatamodel.h"
#include "importconfig.h"
#include "contexthelp.h"
#include "formatlistedit.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataModel track data to be filled with imported values,
 *        nullptr if dialog is used independent from import dialog
 */
TagImportDialog::TagImportDialog(QWidget* parent,
                                 TrackDataModel* trackDataModel)
  : QDialog(parent), m_trackDataModel(trackDataModel)
{
  setObjectName(QLatin1String("TagImportDialog"));
  setWindowTitle(tr("Import from Tags"));
  setSizeGripEnabled(true);

  auto vboxLayout = new QVBoxLayout(this);

  m_formatListEdit = new FormatListEdit(
        {tr("Format:"), tr("Source:"), tr("Extraction:")},
        {QString(), TrackDataFormatReplacer::getToolTip(),
         getExtractionToolTip()},
        this);
  vboxLayout->addWidget(m_formatListEdit);

  if (!trackDataModel) {
    auto destLayout = new QFormLayout;
    destLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_destComboBox = new QComboBox;
    const auto tagVersions = Frame::availableTagVersions();
    for (auto it = tagVersions.constBegin(); it != tagVersions.constEnd(); ++it) {
      m_destComboBox->addItem(it->second, it->first);
    }
    destLayout->addRow(tr("D&estination:"), m_destComboBox);
    vboxLayout->addLayout(destLayout);
  } else {
    m_destComboBox = nullptr;
  }

  auto buttonLayout = new QHBoxLayout;
  auto helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  buttonLayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &TagImportDialog::showHelp);
  auto saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  buttonLayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &TagImportDialog::saveConfig);
  buttonLayout->addStretch();
  auto applyButton = new QPushButton(tr("&Apply"), this);
  applyButton->setAutoDefault(false);
  buttonLayout->addWidget(applyButton);
  connect(applyButton, &QAbstractButton::clicked, this, &TagImportDialog::apply);
  auto closeButton = new QPushButton(tr("&Close"), this);
  closeButton->setAutoDefault(false);
  buttonLayout->addWidget(closeButton);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
  vboxLayout->addLayout(buttonLayout);
}

/**
 * Clear dialog data.
 */
void TagImportDialog::clear()
{
  setFormatFromConfig();

  if (m_destComboBox) {
    const ImportConfig& importCfg = ImportConfig::instance();
    Frame::TagVersion importDest = importCfg.importDest();
    int index = m_destComboBox->findData(importDest);
    m_destComboBox->setCurrentIndex(index);
  }
}

/**
 * Get import destination.
 * Is only available if dialog is not opened from import dialog.
 * @return TagV1, TagV2 or TagV2V1 for ID3v1, ID3v2 or both.
 */
Frame::TagVersion TagImportDialog::getDestination() const
{
  return m_destComboBox
      ? Frame::tagVersionCast(
          m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt())
      : ImportConfig::instance().importDest();
}

/**
 * Get selected source format.
 * @return source format.
 */
QString TagImportDialog::getSourceFormat() const
{
  return m_formatListEdit->getCurrentFormat(1);
}

/**
 * Get selected extraction format.
 * @return extraction format.
 */
QString TagImportDialog::getExtractionFormat() const
{
  return m_formatListEdit->getCurrentFormat(2);
}

/**
 * Apply import to track data.
 */
void TagImportDialog::apply()
{
  if (m_trackDataModel) {
    ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
    TextImporter::importFromTags(m_formatListEdit->getCurrentFormat(1),
                                 m_formatListEdit->getCurrentFormat(2),
                                 trackDataVector);
    m_trackDataModel->setTrackData(trackDataVector);
  }
  emit trackDataUpdated();
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void TagImportDialog::setFormatFromConfig()
{
  const ImportConfig& importCfg = ImportConfig::instance();
  m_formatListEdit->setFormats(
        {importCfg.importTagsNames(), importCfg.importTagsSources(),
         importCfg.importTagsExtractions()},
        importCfg.importTagsIndex());
}

/**
 * Save the local settings to the configuration.
 */
void TagImportDialog::saveConfig()
{
  ImportConfig& importCfg = ImportConfig::instance();
  int idx;
  QList<QStringList> formats = m_formatListEdit->getFormats(&idx);
  importCfg.setImportTagsIndex(idx);
  importCfg.setImportTagsNames(formats.at(0));
  importCfg.setImportTagsSources(formats.at(1));
  importCfg.setImportTagsExtractions(formats.at(2));

  if (m_destComboBox) {
    importCfg.setImportDest(Frame::tagVersionCast(
      m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt()));
  }

  setFormatFromConfig();
}

/**
 * Show help.
 */
void TagImportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("import-tags"));
}

/**
 * Get help text for format codes supported in extraction field.
 * @return help text.
 */
QString TagImportDialog::getExtractionToolTip()
{
  QString str;
  str += QLatin1String("<table>\n");
  str += ImportParser::getFormatToolTip(true);

  str += QLatin1String("<tr><td>%f</td><td>%{file}</td><td>");
  str += QCoreApplication::translate("@default", "Filename");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("</table>\n");
  return str;
}
