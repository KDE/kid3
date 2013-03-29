/**
 * \file filterdialog.cpp
 * Filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008-2013  Urs Fleisch
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
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "configstore.h"
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

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  m_edit = new QTextEdit(this);
  m_edit->setReadOnly(true);
  m_edit->setTabStopWidth(20);
  m_edit->setAcceptRichText(false);
  vlayout->addWidget(m_edit);

  m_formatListEdit = new FormatListEdit(
        QStringList() << tr("&Filter:")
                      << tr("&Expression:"),
        QStringList() << QString()
                      << FileFilter::getFormatToolTip(),
        this);
  vlayout->addWidget(m_formatListEdit);

  QHBoxLayout* hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));

  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  hlayout->addItem(hspacer);

  m_applyButton = new QPushButton(this);
  setAbortButton(false);
  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
  m_applyButton->setAutoDefault(false);
  closeButton->setAutoDefault(false);
  hlayout->addWidget(m_applyButton);
  hlayout->addWidget(closeButton);
  connect(m_applyButton, SIGNAL(clicked()), this, SLOT(applyOrAbortFilter()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(this, SIGNAL(rejected()), &m_fileFilter, SLOT(abort()));

  vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
FilterDialog::~FilterDialog()
{}

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
  }
}

/**
 * Set the filter combo box and line edit from the configuration.
 */
void FilterDialog::setFiltersFromConfig()
{
  m_formatListEdit->setFormats(
        QList<QStringList>() << ConfigStore::s_filterCfg.m_filterNames
                             << ConfigStore::s_filterCfg.m_filterExpressions,
        ConfigStore::s_filterCfg.m_filterIdx);
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

  if (!ConfigStore::s_filterCfg.m_windowGeometry.isEmpty()) {
    restoreGeometry(ConfigStore::s_filterCfg.m_windowGeometry);
  }
}

/**
 * Save the local settings to the configuration.
 */
void FilterDialog::saveConfig()
{
  QList<QStringList> formats = m_formatListEdit->getFormats(
        &ConfigStore::s_filterCfg.m_filterIdx);
  ConfigStore::s_filterCfg.m_filterNames = formats.at(0);
  ConfigStore::s_filterCfg.m_filterExpressions = formats.at(1);
  ConfigStore::s_filterCfg.m_windowGeometry = saveGeometry();

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
void FilterDialog::showFilterEvent(FileFilter::FilterEventType type,
                                   const QString& fileName) {
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
