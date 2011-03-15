/**
 * \file playlistdialog.cpp
 * Create playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#include "playlistdialog.h"
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QToolTip>
#include "qtcompatmac.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include "kid3.h"
#include "importtrackdata.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 */
PlaylistDialog::PlaylistDialog(QWidget* parent):
	QDialog(parent)
{
	setModal(true);
	setWindowTitle(i18n("Create Playlist"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setMargin(6);
	vlayout->setSpacing(6);
	QGroupBox* fnGroupBox = new QGroupBox(this);
	QVBoxLayout* fnGroupBoxLayout = new QVBoxLayout(fnGroupBox);
	fnGroupBoxLayout->setMargin(2);
	fnGroupBoxLayout->setSpacing(4);
	m_sameAsDirNameButton = new QRadioButton(this);
	fnGroupBoxLayout->addWidget(m_sameAsDirNameButton);
	QHBoxLayout* fileNameFormatLayout = new QHBoxLayout;
	m_fileNameFormatButton = new QRadioButton(this);
	m_fileNameFormatComboBox = new QComboBox(this);
	m_fileNameFormatComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
	fileNameFormatLayout->addWidget(m_fileNameFormatButton);
	fileNameFormatLayout->addWidget(m_fileNameFormatComboBox);
	fnGroupBoxLayout->addLayout(fileNameFormatLayout);
	QHBoxLayout* locationLayout = new QHBoxLayout;
	QLabel* locationLabel = new QLabel(this);
	m_locationComboBox = new QComboBox(this);
	locationLayout->addWidget(locationLabel);
	locationLayout->addWidget(m_locationComboBox);
	fnGroupBoxLayout->addLayout(locationLayout);
	vlayout->addWidget(fnGroupBox);

	QGroupBox* pcGroupBox = new QGroupBox(this);
	QVBoxLayout* pcGroupBoxLayout = new QVBoxLayout(pcGroupBox);
	pcGroupBoxLayout->setMargin(2);
	pcGroupBoxLayout->setSpacing(4);
	QHBoxLayout* formatLayout = new QHBoxLayout;
	QLabel* formatLabel = new QLabel(this);
	m_formatComboBox = new QComboBox(this);
	formatLayout->addWidget(formatLabel);
	formatLayout->addWidget(m_formatComboBox);
	pcGroupBoxLayout->addLayout(formatLayout);
	m_onlySelectedFilesCheckBox = new QCheckBox(this);
	pcGroupBoxLayout->addWidget(m_onlySelectedFilesCheckBox);

	QFrame* sortLine = new QFrame(pcGroupBox);
	sortLine->setFrameShape(QFrame::HLine);
	sortLine->setFrameShadow(QFrame::Sunken);
	pcGroupBoxLayout->addWidget(sortLine);
	QButtonGroup* sortButtonGroup = new QButtonGroup(pcGroupBox);
	m_sortFileNameButton = new QRadioButton(this);
	pcGroupBoxLayout->addWidget(m_sortFileNameButton);
	QHBoxLayout* sortTagFieldLayout = new QHBoxLayout;
	m_sortTagFieldButton = new QRadioButton(this);
	m_sortTagFieldComboBox = new QComboBox(this);
	m_sortTagFieldComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
	sortTagFieldLayout->addWidget(m_sortTagFieldButton);
	sortTagFieldLayout->addWidget(m_sortTagFieldComboBox);
	pcGroupBoxLayout->addLayout(sortTagFieldLayout);
	sortButtonGroup->addButton(m_sortFileNameButton);
	sortButtonGroup->addButton(m_sortTagFieldButton);

	QFrame* pathLine = new QFrame(pcGroupBox);
	pathLine->setFrameShape(QFrame::HLine);
	pathLine->setFrameShadow(QFrame::Sunken);
	pcGroupBoxLayout->addWidget(pathLine);
	QButtonGroup* pathButtonGroup = new QButtonGroup(pcGroupBox);
	m_relPathButton = new QRadioButton(this);
	pcGroupBoxLayout->addWidget(m_relPathButton);
	m_fullPathButton = new QRadioButton(this);
	pcGroupBoxLayout->addWidget(m_fullPathButton);
	pathButtonGroup->addButton(m_relPathButton);
	pathButtonGroup->addButton(m_fullPathButton);

	QFrame* writeLine = new QFrame(pcGroupBox);
	writeLine->setFrameShape(QFrame::HLine);
	writeLine->setFrameShadow(QFrame::Sunken);
	pcGroupBoxLayout->addWidget(writeLine);
	QButtonGroup* writeButtonGroup = new QButtonGroup(pcGroupBox);
	m_writeListButton = new QRadioButton(this);
	pcGroupBoxLayout->addWidget(m_writeListButton);
	QHBoxLayout* writeInfoLayout = new QHBoxLayout;
	m_writeInfoButton = new QRadioButton(this);
	m_writeInfoComboBox = new QComboBox(this);
	m_writeInfoComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
	writeInfoLayout->addWidget(m_writeInfoButton);
	writeInfoLayout->addWidget(m_writeInfoComboBox);
	pcGroupBoxLayout->addLayout(writeInfoLayout);
	writeButtonGroup->addButton(m_writeListButton);
	writeButtonGroup->addButton(m_writeInfoButton);
	vlayout->addWidget(pcGroupBox);
	fnGroupBox->setTitle(i18n("Playlist File Name"));
	m_sameAsDirNameButton->setText(i18n("Same as &directory name"));
	m_sameAsDirNameButton->setChecked(true);
	m_fileNameFormatButton->setText(i18n("&Format:"));
	m_fileNameFormatComboBox->setEditable(true);
	m_fileNameFormatComboBox->setEnabled(false);
	m_fileNameFormatComboBox->addItems(
		QStringList() <<
		"%{artist} - %{album}" << "%{artist} - [%{year}] %{album}" << "%{album}" <<
		"playlist_%{artist}_-_%{album}" << "playlist");
	connect(m_fileNameFormatButton, SIGNAL(toggled(bool)),
					m_fileNameFormatComboBox, SLOT(setEnabled(bool)));
	locationLabel->setText(i18n("Cr&eate in:"));
	locationLabel->setBuddy(m_locationComboBox);
	m_locationComboBox->addItems(
		QStringList() <<
		i18n("Current directory") <<
		i18n("Every directory") <<
		i18n("Top-level directory"));
	pcGroupBox->setTitle(i18n("Playlist Content"));
	formatLabel->setText(i18n("For&mat:"));
	formatLabel->setBuddy(m_formatComboBox);
	m_formatComboBox->addItems(QStringList() << "M3U" << "PLS" << "XSPF");
	m_onlySelectedFilesCheckBox->setText(i18n("Incl&ude only the selected files"));
	m_sortFileNameButton->setText(i18n("Sort by file &name"));
	m_sortFileNameButton->setChecked(true);
	m_sortTagFieldButton->setText(i18n("Sort by &tag field"));
	m_sortTagFieldComboBox->setEditable(true);
	m_sortTagFieldComboBox->setEnabled(false);
	QStringList lst;
	for (int type = Frame::FT_FirstFrame; type <= Frame::FT_LastFrame; ++type) {
		QString frameName =
			QString(Frame::getNameFromType(
								static_cast<Frame::Type>(type))).toLower();
		if (frameName == "track number") frameName = "track.3";
		lst.append("%{" + frameName + "}");
	}
	m_sortTagFieldComboBox->addItems(lst);
	connect(m_sortTagFieldButton, SIGNAL(toggled(bool)),
					m_sortTagFieldComboBox, SLOT(setEnabled(bool)));
	m_relPathButton->setText(i18n("Use &relative path for files in playlist"));
	m_relPathButton->setChecked(true);
	m_fullPathButton->setText(i18n("Use full p&ath for files in playlist"));
	m_writeListButton->setText(i18n("Write only &list of files"));
	m_writeListButton->setChecked(true);
	m_writeInfoButton->setText(i18n("Write &info using"));
	m_writeInfoComboBox->setEditable(true);
	m_writeInfoComboBox->setEnabled(false);
	m_writeInfoComboBox->addItems(
		QStringList() <<
		"%{artist} - %{title}" << "%{title}" <<
		"%{track.1}/%{tracks} - %{artist} - %{album} - %{title}");
	connect(m_writeInfoButton, SIGNAL(toggled(bool)),
					m_writeInfoComboBox, SLOT(setEnabled(bool)));

	QHBoxLayout* hlayout = new QHBoxLayout;
	hlayout->setSpacing(6);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	helpButton->setAutoDefault(false);
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
PlaylistDialog::~PlaylistDialog()
{}

/**
 * Read the local settings from the configuration.
 */
void PlaylistDialog::readConfig()
{
	m_fileNameFormatButton->setChecked(
		Kid3App::s_playlistCfg.m_useFileNameFormat);
	m_sameAsDirNameButton->setChecked(
		!Kid3App::s_playlistCfg.m_useFileNameFormat);
	m_onlySelectedFilesCheckBox->setChecked(
		Kid3App::s_playlistCfg.m_onlySelectedFiles);
	m_sortTagFieldButton->setChecked(Kid3App::s_playlistCfg.m_useSortTagField);
	m_sortFileNameButton->setChecked(!Kid3App::s_playlistCfg.m_useSortTagField);
	m_fullPathButton->setChecked(Kid3App::s_playlistCfg.m_useFullPath);
	m_relPathButton->setChecked(!Kid3App::s_playlistCfg.m_useFullPath);
	m_writeInfoButton->setChecked(Kid3App::s_playlistCfg.m_writeInfo);
	m_writeListButton->setChecked(!Kid3App::s_playlistCfg.m_writeInfo);
	m_locationComboBox->setCurrentIndex(Kid3App::s_playlistCfg.m_location);
	m_formatComboBox->setCurrentIndex(Kid3App::s_playlistCfg.m_format);
	m_fileNameFormatComboBox->setEditText(
		Kid3App::s_playlistCfg.m_fileNameFormat);
	m_sortTagFieldComboBox->setEditText(Kid3App::s_playlistCfg.m_sortTagField);
	m_writeInfoComboBox->setEditText(Kid3App::s_playlistCfg.m_infoFormat);
}

/**
 * Get the current dialog configuration.
 *
 * @param cfg the current configuration is returned here
 */
void PlaylistDialog::getCurrentConfig(PlaylistConfig& cfg) const
{
	cfg.m_useFileNameFormat = m_fileNameFormatButton->isChecked();
	cfg.m_onlySelectedFiles = m_onlySelectedFilesCheckBox->isChecked();
	cfg.m_useSortTagField = m_sortTagFieldButton->isChecked();
	cfg.m_useFullPath = m_fullPathButton->isChecked();
	cfg.m_writeInfo = m_writeInfoButton->isChecked();
	cfg.m_location = static_cast<PlaylistConfig::PlaylistLocation>(
		m_locationComboBox->currentIndex());
	cfg.m_format = static_cast<PlaylistConfig::PlaylistFormat>(
		m_formatComboBox->currentIndex());
	cfg.m_fileNameFormat = m_fileNameFormatComboBox->currentText();
	cfg.m_sortTagField = m_sortTagFieldComboBox->currentText();
	cfg.m_infoFormat = m_writeInfoComboBox->currentText();
}

/**
 * Save the local settings to the configuration.
 */
void PlaylistDialog::saveConfig() const
{
	getCurrentConfig(Kid3App::s_playlistCfg);
}

/**
 * Show help.
 */
void PlaylistDialog::showHelp()
{
	Kid3App::displayHelp("create-playlist");
}