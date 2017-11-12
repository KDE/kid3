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
#include <QFormLayout>
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
  m_formatEditingCheckBox = new QCheckBox(tr("Automatically apply format"),
                                          this);

  m_caseConvComboBox = new QComboBox(this);
  m_caseConvComboBox->addItems(FormatConfig::getCaseConversionNames());

#if QT_VERSION >= 0x040800
  m_localeComboBox = new QComboBox(this);
  m_localeComboBox->addItems(FormatConfig::getLocaleNames());
#endif
  m_strRepCheckBox = new QCheckBox(tr("String replacement:"), this);
  m_strReplTableModel = new ConfigTableModel(this);
  m_strReplTableModel->setLabels(
    QStringList() << tr("From") << tr("To"));
  m_strReplTable = new ConfigTable(m_strReplTableModel, this);
  m_strReplTable->setHorizontalResizeModes(
      m_strReplTableModel->getHorizontalResizeModes());
  QFormLayout* formLayout = new QFormLayout(this);
  formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  formLayout->addRow(m_formatEditingCheckBox);
  formLayout->addRow(tr("Case conversion:"), m_caseConvComboBox);
#if QT_VERSION >= 0x040800
  formLayout->addRow(tr("Locale:"), m_localeComboBox);
#endif
  formLayout->addRow(m_strRepCheckBox);
  formLayout->addRow(m_strReplTable);
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
