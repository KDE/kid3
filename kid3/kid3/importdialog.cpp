/**
 * \file importdialog.cpp
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#else
#include <qhbox.h>
#endif
#include "importselector.h"
#include "importdialog.h"
#include "kid3.h"

/**
 * Constructor.
 *
 * @param parent        parent widget
 * @param caption       dialog title
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with durations of files set
 */
ImportDialog::ImportDialog(QWidget* parent, QString& caption,
													 ImportTrackDataVector& trackDataList) :
	QDialog(parent),
	m_autoStartSubDialog(ASD_None),
	m_trackDataVector(trackDataList)
{
	setModal(true);
	QCM_setWindowTitle(caption);

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);
	m_impsel = new ImportSelector(this, m_trackDataVector);
	vlayout->addWidget(m_impsel);

	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && okButton && saveButton && cancelButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(true);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), m_impsel, SLOT(saveConfig()));
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
	}
}

/**
 * Destructor.
 */
ImportDialog::~ImportDialog()
{}

/**
 * Shows the dialog as a modal dialog.
 */
int ImportDialog::exec()
{
	switch (m_autoStartSubDialog) {
		case ASD_Freedb:
			show();
			m_impsel->fromFreedb();
			break;
		case ASD_TrackType:
			show();
			m_impsel->fromTrackType();
			break;
		case ASD_Discogs:
			show();
			m_impsel->fromDiscogs();
			break;
		case ASD_MusicBrainzRelease:
			show();
			m_impsel->fromMusicBrainzRelease();
			break;
		case ASD_MusicBrainz:
			show();
			m_impsel->fromMusicBrainz();
			break;
		case ASD_None:
			break;
	}
	return QDialog::exec();
}

/**
 * Clear dialog data.
 */
void ImportDialog::clear()
{
	m_impsel->clear();
}

/**
 * Get import destination.
 *
 * @return DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both.
 */
ImportConfig::ImportDestination ImportDialog::getDestination() const
{
	return m_impsel->getDestination();
}

/**
 * Show help.
 */
void ImportDialog::showHelp()
{
	Kid3App::displayHelp("import");
}
