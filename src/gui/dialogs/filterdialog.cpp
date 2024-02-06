/**
 * \file filterdialog.cpp
 * Filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008-2024  Urs Fleisch
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
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "filterconfig.h"
#include "contexthelp.h"
#include "formatlistedit.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
FilterDialog::FilterDialog(QWidget* parent) : QDialog(parent),
  m_isAbortButton(false)
{
  setObjectName(QLatin1String("FilterDialog"));
  setWindowTitle(tr("Filter"));
  setSizeGripEnabled(true);

  auto vlayout = new QVBoxLayout(this);
  m_previewBox = new QGroupBox(tr("&Preview"));
  m_previewBox->setCheckable(true);
  m_previewBox->setChecked(false);
  auto previewLayout = new QVBoxLayout(m_previewBox);
  m_edit = new QTextEdit;
  m_edit->setReadOnly(true);
#if QT_VERSION >= 0x050a00
  m_edit->setTabStopDistance(20);
#else
  m_edit->setTabStopWidth(20);
#endif
  m_edit->setAcceptRichText(false);
  previewLayout->addWidget(m_edit);
  vlayout->addWidget(m_previewBox);

  m_formatListEdit = new FormatListEdit(
        {tr("&Filter:"), tr("&Expression:")},
        {QString(), FileFilter::getFormatToolTip()},
        this);
  vlayout->addWidget(m_formatListEdit);

  auto hlayout = new QHBoxLayout;
  auto helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &FilterDialog::showHelp);

  auto saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &FilterDialog::saveConfig);

  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  hlayout->addItem(hspacer);

  m_applyButton = new QPushButton(this);
  setAbortButton(false);
  auto closeButton = new QPushButton(tr("&Close"), this);
  m_applyButton->setAutoDefault(false);
  m_applyButton->setDefault(true);
  closeButton->setAutoDefault(false);
  hlayout->addWidget(m_applyButton);
  hlayout->addWidget(closeButton);
  connect(m_applyButton, &QAbstractButton::clicked, this, &FilterDialog::applyOrAbortFilter);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::reject);
  connect(this, &FilterDialog::rejected, &m_fileFilter, &FileFilter::abort);

  vlayout->addLayout(hlayout);
}

/**
 * Apply or abort filter.
 */
void FilterDialog::applyOrAbortFilter()
{
  if (m_isAbortButton) {
    m_fileFilter.abort();
  } else {
    m_edit->clear();
    m_fileFilter.setFilterExpression(m_formatListEdit->getCurrentFormat(1));
    m_fileFilter.initParser();
    emit apply(m_fileFilter);
    if (!m_previewBox->isChecked()) {
      accept();
    }
  }
}

/**
 * Set the filter combo box and line edit from the configuration.
 */
void FilterDialog::setFiltersFromConfig()
{
  const FilterConfig& filterCfg = FilterConfig::instance();
  m_formatListEdit->setFormats(
        {filterCfg.filterNames(), filterCfg.filterExpressions()},
        filterCfg.filterIndex());
}

/**
 * Read the local settings from the configuration.
 */
void FilterDialog::readConfig()
{
  m_fileFilter.clearAborted();
  m_edit->clear();
  setAbortButton(false);

  setFiltersFromConfig();

  if (!FilterConfig::instance().windowGeometry().isEmpty()) {
    restoreGeometry(FilterConfig::instance().windowGeometry());
  }
}

/**
 * Save the local settings to the configuration.
 */
void FilterDialog::saveConfig()
{
  int idx;
  FilterConfig& filterCfg = FilterConfig::instance();
  QList<QStringList> formats = m_formatListEdit->getFormats(&idx);
  filterCfg.setFilterIndex(idx);
  filterCfg.setFilterNames(formats.at(0));
  filterCfg.setFilterExpressions(formats.at(1));
  filterCfg.setWindowGeometry(saveGeometry());

  setFiltersFromConfig();
}

/**
 * Show help.
 */
void FilterDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("filter"));
}

/**
 * Show information about filter event.
 */
void FilterDialog::showFilterEvent(int type, const QString& fileName) {
  if (!m_previewBox->isChecked())
    return;

  switch (type) {
  case FileFilter::Started:
    m_edit->append(tr("Started"));
    setAbortButton(true);
    break;
  case FileFilter::Directory:
    m_edit->append(QLatin1Char('\t') + fileName);
    break;
  case FileFilter::ParseError:
    m_edit->append(QLatin1String("parse error"));
    break;
  case FileFilter::FilePassed:
    m_edit->append(QLatin1String("+\t") + fileName);
    break;
  case FileFilter::FileFilteredOut:
    m_edit->append(QLatin1String("-\t") + fileName);
    break;
  case FileFilter::Finished:
    m_edit->append(tr("Finished"));
    setAbortButton(false);
    break;
  case FileFilter::Aborted:
    m_edit->append(tr("Aborted"));
    setAbortButton(false);
    break;
  }
}

/**
 * Set button to Start or Abort.
 * @param enableAbort true to set Abort button
 */
void FilterDialog::setAbortButton(bool enableAbort)
{
  m_isAbortButton = enableAbort;
  m_applyButton->setText(m_isAbortButton ? tr("A&bort") : tr("&Apply"));
}
