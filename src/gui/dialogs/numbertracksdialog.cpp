/**
 * \file numbertracksdialog.cpp
 * Number tracks dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 May 2006
 *
 * Copyright (C) 2006-2013  Urs Fleisch
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

#include "numbertracksdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "configstore.h"
#include "contexthelp.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
NumberTracksDialog::NumberTracksDialog(QWidget* parent) :
  QDialog(parent)
{
  setObjectName(QLatin1String("NumberTracksDialog"));
  setModal(true);
  setWindowTitle(i18n("Number Tracks"));

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  QHBoxLayout* trackLayout = new QHBoxLayout;
  QLabel* trackLabel = new QLabel(i18n("&Start number:"), this);
  m_trackSpinBox = new QSpinBox(this);
  m_trackSpinBox->setMaximum(9999);
  m_trackSpinBox->setValue(ConfigStore::s_miscCfg.m_numberTracksStart);
  trackLayout->addWidget(trackLabel);
  trackLayout->addWidget(m_trackSpinBox);
  trackLabel->setBuddy(m_trackSpinBox);

  QSpacerItem* trackSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                             QSizePolicy::Minimum);
  trackLayout->addItem(trackSpacer);

  QLabel* destLabel = new QLabel(i18n("&Destination:"), this);
  m_destComboBox = new QComboBox(this);
  m_destComboBox->setEditable(false);
  m_destComboBox->addItem(i18n("Tag 1"), TrackData::TagV1);
  m_destComboBox->addItem(i18n("Tag 2"), TrackData::TagV2);
  m_destComboBox->addItem(i18n("Tag 1 and Tag 2"), TrackData::TagV2V1);
  m_destComboBox->setCurrentIndex(
      m_destComboBox->findData(ConfigStore::s_miscCfg.m_numberTracksDst));
  trackLayout->addWidget(destLabel);
  trackLayout->addWidget(m_destComboBox);
  destLabel->setBuddy(m_destComboBox);

  vlayout->addLayout(trackLayout);

  QHBoxLayout* totalLayout = new QHBoxLayout;
  m_totalNumTracksCheckBox = new QCheckBox(i18n("&Total number of tracks:"),
                                           this);
  m_totalNumTrackSpinBox = new QSpinBox(this);
  if (m_totalNumTracksCheckBox && m_totalNumTrackSpinBox) {
    m_totalNumTrackSpinBox->setMaximum(999);
    totalLayout->addWidget(m_totalNumTracksCheckBox);
    totalLayout->addWidget(m_totalNumTrackSpinBox);
  }
  QSpacerItem* totalSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                             QSizePolicy::Minimum);
  totalLayout->addItem(totalSpacer);
  vlayout->addLayout(totalLayout);

  QHBoxLayout* hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
  hlayout->addWidget(helpButton);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));

  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  hlayout->addItem(hspacer);

  QPushButton* okButton = new QPushButton(i18n("&OK"), this);
  hlayout->addWidget(okButton);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

  QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
  hlayout->addWidget(cancelButton);
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
NumberTracksDialog::~NumberTracksDialog()
{}

/**
 * Get start number.
 */
int NumberTracksDialog::getStartNumber() const
{
  return m_trackSpinBox->value();
}

/**
 * Get destination.
 *
  * @return TagV1, TagV2 or TagV2V1 if ID3v1, ID2v2 or both are destination
  */
TrackData::TagVersion NumberTracksDialog::getDestination() const
{
  return TrackData::tagVersionCast(
        m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt());
}

/**
 * Save the local settings to the configuration.
 */
void NumberTracksDialog::saveConfig()
{
  ConfigStore::s_miscCfg.m_numberTracksDst = getDestination();
  ConfigStore::s_miscCfg.m_numberTracksStart = m_trackSpinBox->value();
}

/**
 * Show help.
 */
void NumberTracksDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("number-tracks"));
}

/**
 * Set the total number of tracks.
 *
 * @param numTracks number of tracks
 * @param enable    true to enable setting of total
 */
void NumberTracksDialog::setTotalNumberOfTracks(int numTracks, bool enable)
{
  m_totalNumTrackSpinBox->setValue(numTracks);
  m_totalNumTracksCheckBox->setChecked(enable);
}

/**
 * Get the total number of tracks.
 *
 * @param enable true is returned here if total number of tracks is checked
 *
 * @return number of tracks entered
 */
int NumberTracksDialog::getTotalNumberOfTracks(bool* enable) const
{
  *enable = m_totalNumTracksCheckBox->isChecked();
  return m_totalNumTrackSpinBox->value();
}
