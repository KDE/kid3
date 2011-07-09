/**
 * \file numbertracksdialog.cpp
 * Number tracks dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 May 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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
#include "kid3mainwindow.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
NumberTracksDialog::NumberTracksDialog(QWidget* parent) :
	QDialog(parent)
{
	setModal(true);
	setWindowTitle(i18n("Number Tracks"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setMargin(6);
		vlayout->setSpacing(6);
		QHBoxLayout* trackLayout = new QHBoxLayout;
		if (trackLayout) {
			trackLayout->setSpacing(6);
			QLabel* trackLabel = new QLabel(i18n("&Start number:"), this);
			m_trackSpinBox = new QSpinBox(this);
			if (trackLabel && m_trackSpinBox) {
				m_trackSpinBox->setMaximum(999);
				m_trackSpinBox->setValue(Kid3MainWindow::s_miscCfg.m_numberTracksStart);
				trackLayout->addWidget(trackLabel);
				trackLayout->addWidget(m_trackSpinBox);
				trackLabel->setBuddy(m_trackSpinBox);
			}
			QSpacerItem* trackSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																								 QSizePolicy::Minimum);
			trackLayout->addItem(trackSpacer);

			QLabel* destLabel = new QLabel(i18n("&Destination:"), this);
			m_destComboBox = new QComboBox(this);
			if (destLabel && m_destComboBox) {
				m_destComboBox->setEditable(false);
				m_destComboBox->insertItem(DestV1, i18n("Tag 1"));
				m_destComboBox->insertItem(DestV2, i18n("Tag 2"));
				m_destComboBox->insertItem(DestV1V2, i18n("Tag 1 and Tag 2"));
				m_destComboBox->setCurrentIndex(Kid3MainWindow::s_miscCfg.m_numberTracksDst);
				trackLayout->addWidget(destLabel);
				trackLayout->addWidget(m_destComboBox);
				destLabel->setBuddy(m_destComboBox);
			}
			vlayout->addLayout(trackLayout);
		}

		QHBoxLayout* totalLayout = new QHBoxLayout;
		if (totalLayout) {
			totalLayout->setSpacing(6);
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
		}

		QHBoxLayout* hlayout = new QHBoxLayout;
		if (hlayout) {
			hlayout->setSpacing(6);
			QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
			if (helpButton) {
				hlayout->addWidget(helpButton);
				connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
			}
			QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
			if (saveButton) {
				saveButton->setAutoDefault(false);
				hlayout->addWidget(saveButton);
				connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
			}
			QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
			hlayout->addItem(hspacer);

			QPushButton* okButton = new QPushButton(i18n("&OK"), this);
			if (okButton) {
				hlayout->addWidget(okButton);
				connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
			}
			QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
			if (cancelButton) {
				hlayout->addWidget(cancelButton);
				connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
			}
			vlayout->addLayout(hlayout);
		}
	}
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
 * @return DestV1, DestV2 or DestV1V2 if ID3v1, ID2v2 or both are destination
 */
NumberTracksDialog::Destination NumberTracksDialog::getDestination() const
{
	return static_cast<Destination>(m_destComboBox->currentIndex());
}

/**
 * Save the local settings to the configuration.
 */
void NumberTracksDialog::saveConfig()
{
	Kid3MainWindow::s_miscCfg.m_numberTracksDst = m_destComboBox->currentIndex();
	Kid3MainWindow::s_miscCfg.m_numberTracksStart = m_trackSpinBox->value();
}

/**
 * Show help.
 */
void NumberTracksDialog::showHelp()
{
	Kid3MainWindow::displayHelp("number-tracks");
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
