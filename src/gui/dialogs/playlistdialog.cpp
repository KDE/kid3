/**
 * \file playlistdialog.cpp
 * Create playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include "playlistconfig.h"
#include "contexthelp.h"
#include "trackdata.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 */
PlaylistDialog::PlaylistDialog(QWidget* parent):
  QDialog(parent)
{
  setObjectName(QLatin1String("PlaylistDialog"));
  setModal(true);
  setWindowTitle(tr("Create Playlist"));
  setSizeGripEnabled(true);

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  QGroupBox* fnGroupBox = new QGroupBox(this);
  QVBoxLayout* fnGroupBoxLayout = new QVBoxLayout(fnGroupBox);
  m_sameAsDirNameButton = new QRadioButton(this);
  fnGroupBoxLayout->addWidget(m_sameAsDirNameButton);
  QHBoxLayout* fileNameFormatLayout = new QHBoxLayout;
  m_fileNameFormatButton = new QRadioButton(this);
  m_fileNameFormatComboBox = new QComboBox(this);
  m_fileNameFormatComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
  fileNameFormatLayout->addWidget(m_fileNameFormatButton);
  fileNameFormatLayout->addWidget(m_fileNameFormatComboBox);
  fnGroupBoxLayout->addLayout(fileNameFormatLayout);

  QHBoxLayout* fileNameForEmptyLayout = new QHBoxLayout;
  m_fileNameForEmptyButton = new QRadioButton(this);
  m_fileNameForEmptyEdit = new QLineEdit(this);
  fileNameForEmptyLayout->addWidget(m_fileNameForEmptyButton);
  fileNameForEmptyLayout->addWidget(m_fileNameForEmptyEdit);
  fnGroupBoxLayout->addLayout(fileNameForEmptyLayout);

  QHBoxLayout* locationLayout = new QHBoxLayout;
  QLabel* locationLabel = new QLabel(this);
  m_locationComboBox = new QComboBox(this);
  locationLayout->addWidget(locationLabel);
  locationLayout->addWidget(m_locationComboBox);
  fnGroupBoxLayout->addLayout(locationLayout);
  vlayout->addWidget(fnGroupBox);

  QGroupBox* pcGroupBox = new QGroupBox(this);
  QVBoxLayout* pcGroupBoxLayout = new QVBoxLayout(pcGroupBox);
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
  fnGroupBox->setTitle(tr("Playlist File Name"));
  m_sameAsDirNameButton->setText(tr("Same as &directory name"));
  m_sameAsDirNameButton->setChecked(true);
  m_fileNameFormatButton->setText(tr("&Format:"));
  m_fileNameFormatComboBox->setEditable(true);
  m_fileNameFormatComboBox->setEnabled(false);
  m_fileNameFormatComboBox->addItems({
    QLatin1String("%{artist} - %{album}"),
    QLatin1String("%{artist} - %{\"[\"year\"] \"}%{album}"),
    QLatin1String("%{album}"),
    QLatin1String("playlist_%{artist}_-_%{album}"),
    QLatin1String("playlist")
  });
  connect(m_fileNameFormatButton, SIGNAL(toggled(bool)),
          m_fileNameFormatComboBox, SLOT(setEnabled(bool)));
  m_fileNameForEmptyButton->setText(tr("Create ne&w empty playlist:"));
  m_fileNameForEmptyEdit->setText(tr("New"));
  m_fileNameForEmptyEdit->setEnabled(false);
  // Position line edit aligned with combo box.
  m_fileNameForEmptyEdit->setSizePolicy(m_fileNameFormatComboBox->sizePolicy());
  connect(m_fileNameForEmptyButton, SIGNAL(toggled(bool)),
          m_fileNameForEmptyEdit, SLOT(setEnabled(bool)));
  locationLabel->setText(tr("Cr&eate in:"));
  locationLabel->setBuddy(m_locationComboBox);
  m_locationComboBox->addItems({
    tr("Current directory"),
    tr("Every directory"),
    tr("Top-level directory")
  });
  pcGroupBox->setTitle(tr("Playlist Content"));
  formatLabel->setText(tr("For&mat:"));
  formatLabel->setBuddy(m_formatComboBox);
  m_formatComboBox->addItems(
    {QLatin1String("M3U"), QLatin1String("PLS"), QLatin1String("XSPF")});
  m_onlySelectedFilesCheckBox->setText(tr("Incl&ude only the selected files"));
  m_sortFileNameButton->setText(tr("Sort by file &name"));
  m_sortFileNameButton->setChecked(true);
  m_sortTagFieldButton->setText(tr("Sort by &tag field"));
  m_sortTagFieldComboBox->setEditable(true);
  m_sortTagFieldComboBox->setEnabled(false);
  QStringList lst;
  for (int type = Frame::FT_FirstFrame; type <= Frame::FT_LastFrame; ++type) {
    QString frameName =
        Frame::ExtendedType(static_cast<Frame::Type>(type), QLatin1String("")).getName().
        toLower();
    if (frameName == QLatin1String("track number")) frameName = QLatin1String("track.3");
    lst.append(QLatin1String("%{") + frameName + QLatin1String("}"));
  }
  m_sortTagFieldComboBox->addItems(lst);
  connect(m_sortTagFieldButton, SIGNAL(toggled(bool)),
          m_sortTagFieldComboBox, SLOT(setEnabled(bool)));
  m_relPathButton->setText(tr("Use &relative path for files in playlist"));
  m_relPathButton->setChecked(true);
  m_fullPathButton->setText(tr("Use full p&ath for files in playlist"));
  m_writeListButton->setText(tr("Write only &list of files"));
  m_writeListButton->setChecked(true);
  m_writeInfoButton->setText(tr("Write &info using"));
  m_writeInfoComboBox->setEditable(true);
  m_writeInfoComboBox->setEnabled(false);
  m_writeInfoComboBox->addItems({
    QLatin1String("%{artist} - %{title}"), QLatin1String("%{title}"),
    QLatin1String("%{track.1}/%{tracks} - %{artist} - %{album} - %{title}")
  });
  connect(m_writeInfoButton, SIGNAL(toggled(bool)),
          m_writeInfoComboBox, SLOT(setEnabled(bool)));

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

  QPushButton* okButton = new QPushButton(tr("&OK"), this);
  hlayout->addWidget(okButton);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
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
  const PlaylistConfig& playlistCfg = PlaylistConfig::instance();
  m_fileNameFormatButton->setChecked(
    playlistCfg.useFileNameFormat());
  m_sameAsDirNameButton->setChecked(
    !playlistCfg.useFileNameFormat());
  m_onlySelectedFilesCheckBox->setChecked(
    playlistCfg.onlySelectedFiles());
  m_sortTagFieldButton->setChecked(playlistCfg.useSortTagField());
  m_sortFileNameButton->setChecked(!playlistCfg.useSortTagField());
  m_fullPathButton->setChecked(playlistCfg.useFullPath());
  m_relPathButton->setChecked(!playlistCfg.useFullPath());
  m_writeInfoButton->setChecked(playlistCfg.writeInfo());
  m_writeListButton->setChecked(!playlistCfg.writeInfo());
  m_locationComboBox->setCurrentIndex(playlistCfg.location());
  m_formatComboBox->setCurrentIndex(playlistCfg.format());
  m_fileNameFormatComboBox->setEditText(
    playlistCfg.fileNameFormat());
  m_sortTagFieldComboBox->setEditText(playlistCfg.sortTagField());
  m_writeInfoComboBox->setEditText(playlistCfg.infoFormat());
}

/**
 * Get the current dialog configuration.
 *
 * @param cfg the current configuration is returned here
 */
void PlaylistDialog::getCurrentConfig(PlaylistConfig& cfg) const
{
  cfg.setUseFileNameFormat(m_fileNameFormatButton->isChecked());
  cfg.setOnlySelectedFiles(m_onlySelectedFilesCheckBox->isChecked());
  cfg.setUseSortTagField(m_sortTagFieldButton->isChecked());
  cfg.setUseFullPath(m_fullPathButton->isChecked());
  cfg.setWriteInfo(m_writeInfoButton->isChecked());
  cfg.setLocation(static_cast<PlaylistConfig::PlaylistLocation>(
    m_locationComboBox->currentIndex()));
  cfg.setFormat(static_cast<PlaylistConfig::PlaylistFormat>(
    m_formatComboBox->currentIndex()));
  cfg.setFileNameFormat(m_fileNameFormatComboBox->currentText());
  cfg.setSortTagField(m_sortTagFieldComboBox->currentText());
  cfg.setInfoFormat(m_writeInfoComboBox->currentText());
}

/**
 * Get the entered file name to create a new empty playlist.
 * @return file name if "Create new empty playlist" is selected, else empty.
 */
QString PlaylistDialog::getFileNameForNewEmptyPlaylist() const
{
  return m_fileNameForEmptyButton->isChecked()
      ? m_fileNameForEmptyEdit->text() : QString();
}

/**
 * Save the local settings to the configuration.
 */
void PlaylistDialog::saveConfig() const
{
  getCurrentConfig(PlaylistConfig::instance());
}

/**
 * Show help.
 */
void PlaylistDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("create-playlist"));
}
