/**
 * \file formatbox.cpp
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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

#include "formatbox.h"
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QString>
#include <QVBoxLayout>
#include <QLocale>
#include "formatconfig.h"
#include "configtable.h"
#include "configtablemodel.h"

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 */
FormatBox::FormatBox(const QString& title, QWidget* parent) :
  QGroupBox(title, parent)
{
  m_formatEditingCheckBox = new QCheckBox(tr("Format while editing"),
                                          this);
  m_validationCheckBox = new QCheckBox(tr("Validation"), this);

  QLabel* caseConvLabel = new QLabel(this);
  caseConvLabel->setText(tr("Case conversion:"));

  m_caseConvComboBox = new QComboBox(this);
  m_caseConvComboBox->setEditable(false);
  m_caseConvComboBox->clear();
  m_caseConvComboBox->insertItem(FormatConfig::NoChanges,
                                     tr("No changes"));
  m_caseConvComboBox->insertItem(FormatConfig::AllLowercase,
                                     tr("All lowercase"));
  m_caseConvComboBox->insertItem(FormatConfig::AllUppercase,
                                     tr("All uppercase"));
  m_caseConvComboBox->insertItem(FormatConfig::FirstLetterUppercase,
                                     tr("First letter uppercase"));
  m_caseConvComboBox->insertItem(FormatConfig::AllFirstLettersUppercase,
                                     tr("All first letters uppercase"));

#if QT_VERSION >= 0x040800
  QHBoxLayout* localeLayout = new QHBoxLayout;
  QLabel* localeLabel = new QLabel(tr("Locale:"));
  localeLayout->addWidget(localeLabel);
  m_localeComboBox = new QComboBox(this);
  m_localeComboBox->addItem(tr("None"));
  m_localeComboBox->addItems(QLocale().uiLanguages());
  localeLabel->setBuddy(m_localeComboBox);
  localeLayout->addWidget(m_localeComboBox);
#endif
  m_strRepCheckBox = new QCheckBox(this);
  m_strRepCheckBox->setText(tr("String replacement:"));
  m_strReplTableModel = new ConfigTableModel(this);
  m_strReplTableModel->setLabels(
    QStringList() << tr("From") << tr("To"));
  m_strReplTable = new ConfigTable(m_strReplTableModel, this);
  m_strReplTable->setHorizontalResizeModes(
      m_strReplTableModel->getHorizontalResizeModes());
  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addWidget(m_formatEditingCheckBox);
  vbox->addWidget(m_validationCheckBox);
  vbox->addWidget(caseConvLabel);
  vbox->addWidget(m_caseConvComboBox);
#if QT_VERSION >= 0x040800
  vbox->addLayout(localeLayout);
#endif
  vbox->addWidget(m_strRepCheckBox);
  vbox->addWidget(m_strReplTable);
  setLayout(vbox);
}

/**
 * Destructor.
 */
FormatBox::~FormatBox() {}

/**
 * Set the values from a format configuration.
 *
 * @param cfg format configuration
 */
void FormatBox::fromFormatConfig(const FormatConfig& cfg)
{
  m_formatEditingCheckBox->setChecked(cfg.formatWhileEditing());
  m_validationCheckBox->setChecked(cfg.enableValidation());
  m_caseConvComboBox->setCurrentIndex(cfg.caseConversion());
#if QT_VERSION >= 0x040800
  int localeIndex = m_localeComboBox->findText(cfg.localeName());
  if (localeIndex == -1) {
    localeIndex = 0;
  }
  m_localeComboBox->setCurrentIndex(localeIndex);
#endif
  m_strRepCheckBox->setChecked(cfg.strRepEnabled());
  m_strReplTableModel->setMap(cfg.strRepMap());
}

/**
 * Store the values in a format configuration.
 *
 * @param cfg format configuration
 */
void FormatBox::toFormatConfig(FormatConfig& cfg) const
{
  cfg.setFormatWhileEditing(m_formatEditingCheckBox->isChecked());
  cfg.setEnableValidation(m_validationCheckBox->isChecked());
  cfg.setCaseConversion(
    (FormatConfig::CaseConversion)m_caseConvComboBox->currentIndex());
  if (cfg.caseConversion() >= FormatConfig::NumCaseConversions) {
    cfg.setCaseConversion(FormatConfig::NoChanges);
  }
#if QT_VERSION >= 0x040800
  cfg.setLocaleName(m_localeComboBox->currentIndex() > 0
                     ? m_localeComboBox->currentText() : QString());
#endif
  cfg.setStrRepEnabled(m_strRepCheckBox->isChecked());
  cfg.setStrRepMap(m_strReplTableModel->getMap());
}

/**
 * Hide the validation check box.
 */
void FormatBox::hideValidationCheckBox()
{
  m_validationCheckBox->hide();
}
