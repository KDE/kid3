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
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param title  title
 * @param parent parent widget
 */
FormatBox::FormatBox(const QString& title, QWidget* parent) :
  QGroupBox(title, parent)
{
  m_formatEditingCheckBox = new QCheckBox(i18n("Format while editing"),
                                          this);

  QLabel* caseConvLabel = new QLabel(this);
  caseConvLabel->setText(i18n("Case conversion:"));

  m_caseConvComboBox = new QComboBox(this);
  m_caseConvComboBox->setEditable(false);
  m_caseConvComboBox->clear();
  m_caseConvComboBox->insertItem(FormatConfig::NoChanges,
                                     i18n("No changes"));
  m_caseConvComboBox->insertItem(FormatConfig::AllLowercase,
                                     i18n("All lowercase"));
  m_caseConvComboBox->insertItem(FormatConfig::AllUppercase,
                                     i18n("All uppercase"));
  m_caseConvComboBox->insertItem(FormatConfig::FirstLetterUppercase,
                                     i18n("First letter uppercase"));
  m_caseConvComboBox->insertItem(FormatConfig::AllFirstLettersUppercase,
                                     i18n("All first letters uppercase"));

#if QT_VERSION >= 0x040800
  QHBoxLayout* localeLayout = new QHBoxLayout;
  localeLayout->setMargin(2);
  QLabel* localeLabel = new QLabel(i18n("&Locale:"));
  localeLayout->addWidget(localeLabel);
  m_localeComboBox = new QComboBox(this);
  m_localeComboBox->addItem(i18n("None"));
  m_localeComboBox->addItems(QLocale().uiLanguages());
  localeLabel->setBuddy(m_localeComboBox);
  localeLayout->addWidget(m_localeComboBox);
#endif
  m_strRepCheckBox = new QCheckBox(this);
  m_strRepCheckBox->setText(i18n("String replacement:"));
  m_strReplTable = new ConfigTable(this);
  m_strReplTableModel = new ConfigTableModel(this);
  m_strReplTableModel->setLabels(
    QStringList() << i18n("From") << i18n("To"));
  m_strReplTable->setModel(m_strReplTableModel);
  m_strReplTable->setHorizontalResizeModes(
      m_strReplTableModel->getHorizontalResizeModes());
  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->setMargin(2);
  vbox->addWidget(m_formatEditingCheckBox);
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
void FormatBox::fromFormatConfig(const FormatConfig* cfg)
{
  m_formatEditingCheckBox->setChecked(cfg->m_formatWhileEditing);
  m_caseConvComboBox->setCurrentIndex(cfg->m_caseConversion);
#if QT_VERSION >= 0x040800
  int localeIndex = m_localeComboBox->findText(cfg->getLocaleName());
  if (localeIndex == -1) {
    localeIndex = 0;
  }
  m_localeComboBox->setCurrentIndex(localeIndex);
#endif
  m_strRepCheckBox->setChecked(cfg->m_strRepEnabled);
  m_strReplTableModel->setMap(cfg->m_strRepMap);
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
    (FormatConfig::CaseConversion)m_caseConvComboBox->currentIndex();
  if (cfg->m_caseConversion >= FormatConfig::NumCaseConversions) {
    cfg->m_caseConversion = FormatConfig::NoChanges;
  }
#if QT_VERSION >= 0x040800
  cfg->setLocaleName(m_localeComboBox->currentIndex() > 0
                     ? m_localeComboBox->currentText() : QString());
#endif
  cfg->m_strRepEnabled = m_strRepCheckBox->isChecked();
  cfg->m_strRepMap = m_strReplTableModel->getMap();
}
