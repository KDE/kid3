/**
 * \file batchimportdialog.cpp
 * Batch import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "batchimportdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QTableView>
#include <QHeaderView>
#include "batchimporter.h"
#include "batchimportsourcedialog.h"
#include "batchimportconfig.h"
#include "batchimportsourcesmodel.h"
#include "serverimporter.h"
#include "contexthelp.h"
#include "abstractlistedit.h"

namespace {

/**
 * Widget to edit a list of import sources.
 */
class BatchImportSourceListEdit : public AbstractListEdit {
public:
  /**
   * Constructor.
   *
   * @param model item model
   * @param parent parent widget
   */
  explicit BatchImportSourceListEdit(BatchImportSourcesModel* model,
                                     QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~BatchImportSourceListEdit() override = default;

  /**
   * Set names of import servers.
   * @param servers server names
   */
  void setServerNames(const QStringList& servers) {
    m_serverNames = servers;
  }

public:
  /**
   * Add a new item.
   */
  virtual void addItem() override;

  /**
   * Edit the selected item.
   */
  virtual void editItem() override;

private:
  Q_DISABLE_COPY(BatchImportSourceListEdit)

  QTableView* m_tableView;
  QStringList m_serverNames;
};

/**
 * Constructor.
 *
 * @param model item model, e.g. a QStringListModel
 * @param parent parent widget
 */
BatchImportSourceListEdit::BatchImportSourceListEdit(
    BatchImportSourcesModel* model, QWidget* parent) :
  AbstractListEdit(m_tableView = new QTableView, model, parent)
{
  setObjectName(QLatin1String("BatchImportSourceListEdit"));
  m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

/**
 * Add a new item.
 */
void BatchImportSourceListEdit::addItem()
{
  auto dialog = new BatchImportSourceDialog(this);
  dialog->setServerNames(m_serverNames);
  if (dialog->exec() == QDialog::Accepted) {
    BatchImportProfile::Source source;
    dialog->getSource(source);
    if (auto model =
        qobject_cast<BatchImportSourcesModel*>(getItemView()->model())) {
      int row = model->rowCount();
      model->insertRow(row);
      model->setBatchImportSource(row, source);
    }
  }
}

/**
 * Edit the selected item.
 */
void BatchImportSourceListEdit::editItem()
{
  QModelIndex index = getItemView()->currentIndex();
  if (index.isValid()) {
    if (auto model =
        qobject_cast<BatchImportSourcesModel*>(getItemView()->model())) {
      BatchImportProfile::Source source;
      model->getBatchImportSource(index.row(), source);
      auto dialog = new BatchImportSourceDialog(this);
      dialog->setServerNames(m_serverNames);
      dialog->setSource(source);
      if (dialog->exec() == QDialog::Accepted) {
        dialog->getSource(source);
        model->setBatchImportSource(index.row(), source);
      }
    }
  }
}

}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
BatchImportDialog::BatchImportDialog(const QList<ServerImporter*>& importers,
                                     QWidget* parent) :
  QDialog(parent), m_importers(importers), m_profileIdx(-1),
  m_isAbortButton(false)
{
  setObjectName(QLatin1String("BatchImportDialog"));
  setWindowTitle(tr("Automatic Import"));
  setSizeGripEnabled(true);

  auto vlayout = new QVBoxLayout(this);
  auto splitter = new QSplitter(Qt::Vertical);

  m_edit = new QTextEdit(this);
  m_edit->setReadOnly(true);
  m_edit->setAcceptRichText(false);
  splitter->addWidget(m_edit);

  QWidget* profileWidget = new QWidget;
  auto profileLayout = new QVBoxLayout(profileWidget);
  profileLayout->setContentsMargins(0, 0, 0, 0);

  auto destLayout = new QHBoxLayout;
  QLabel* destLabel = new QLabel(tr("D&estination:"));
  destLayout->addWidget(destLabel);
  m_destComboBox = new QComboBox;
  m_destComboBox->setEditable(false);
  const QList<QPair<Frame::TagVersion, QString> > tagVersions =
      Frame::availableTagVersions();
  for (auto it = tagVersions.constBegin(); it != tagVersions.constEnd(); ++it) {
    m_destComboBox->addItem(it->second, it->first);
  }
  destLabel->setBuddy(m_destComboBox);
  destLayout->addWidget(m_destComboBox);
  destLayout->addStretch();
  profileLayout->addLayout(destLayout);

  auto nameLayout = new QHBoxLayout;
  QLabel* profileLabel = new QLabel(tr("&Profile:"));
  nameLayout->addWidget(profileLabel);
  m_profileComboBox = new QComboBox;
  m_profileComboBox->setEditable(true);
  connect(m_profileComboBox, static_cast<void (QComboBox::*)(int)>(
            &QComboBox::activated),
          this, &BatchImportDialog::changeProfile);
  connect(m_profileComboBox, &QComboBox::editTextChanged,
          this, &BatchImportDialog::changeProfileName);
  profileLabel->setBuddy(m_profileComboBox);
  nameLayout->addWidget(m_profileComboBox, 1);
  QPushButton* profileAddButton = new QPushButton(tr("Add"));
  connect(profileAddButton, &QAbstractButton::clicked,
          this, &BatchImportDialog::addProfile);
  nameLayout->addWidget(profileAddButton);
  QPushButton* profileRemoveButton = new QPushButton(tr("Remove"));
  connect(profileRemoveButton, &QAbstractButton::clicked,
          this, &BatchImportDialog::removeProfile);
  nameLayout->addWidget(profileRemoveButton);
  profileLayout->addLayout(nameLayout);

  QStringList servers;
  servers.reserve(m_importers.size());
  const auto sis = m_importers;
  for (const ServerImporter* si : sis) {
    servers.append(QString::fromLatin1(si->name()));
  }
  m_profileModel = new BatchImportSourcesModel(this);
  auto profileListEdit =
      new BatchImportSourceListEdit(m_profileModel, this);
  profileListEdit->setServerNames(servers);
  profileLayout->addWidget(profileListEdit);
  splitter->addWidget(profileWidget);
  vlayout->addWidget(splitter);

  auto hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &BatchImportDialog::showHelp);

  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &BatchImportDialog::saveConfig);
  hlayout->addStretch();

  m_startAbortButton = new QPushButton(this);
  setAbortButton(false);
  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
  m_startAbortButton->setAutoDefault(false);
  m_startAbortButton->setDefault(true);
  closeButton->setAutoDefault(false);
  hlayout->addWidget(m_startAbortButton);
  hlayout->addWidget(closeButton);
  connect(m_startAbortButton, &QAbstractButton::clicked, this, &BatchImportDialog::startOrAbortImport);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::reject);
  connect(this, &QDialog::rejected, this, &BatchImportDialog::abort);

  vlayout->addLayout(hlayout);
}

/**
 * Start or abort batch import.
 */
void BatchImportDialog::startOrAbortImport()
{
  if (m_isAbortButton) {
    emit abort();
  } else {
    setProfileFromGuiControls();
    if (m_profileIdx >= 0 && m_profileIdx < m_profiles.size()) {
      m_edit->clear();
      m_currentProfile = m_profiles.at(m_profileIdx);
      emit start(
            m_currentProfile,
            Frame::tagVersionCast(
              m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt()));
    }
  }
}

/**
 * Add a new profile to the list of profiles.
 */
void BatchImportDialog::addNewProfile()
{
  BatchImportProfile profile;
  profile.setName(tr("New"));
  m_profiles.append(profile);
  m_profileIdx = m_profiles.size() - 1;
}

/**
 * Add a new profile.
 */
void BatchImportDialog::addProfile()
{
  setProfileFromGuiControls();
  // First search for an existing empty profile.
  for (int index = 0; index < m_profiles.size(); ++index) {
    if (m_profiles.at(index).getSources().isEmpty()) {
      m_profileIdx = index;
      setGuiControlsFromProfile();
      return;
    }
  }

  addNewProfile();
  setGuiControlsFromProfile();
}

/**
 * Remove the selected profile.
 */
void BatchImportDialog::removeProfile()
{
  int index = m_profileComboBox->currentIndex();
  if (index >= 0 && index < m_profiles.size()) {
    m_profiles.removeAt(index);
    if (m_profileIdx >= m_profiles.size())
      m_profileIdx = m_profiles.size() - 1;
    setGuiControlsFromProfile();
  }
}

/**
 * Switch to different profile.
 * @param index combo box index to set
 */
void BatchImportDialog::changeProfile(int index)
{
  setProfileFromGuiControls();
  m_profileIdx = index;
  setGuiControlsFromProfile();
}

/**
 * Change name of current profile.
 * @param name profile name
 */
void BatchImportDialog::changeProfileName(const QString& name)
{
  int index = m_profileComboBox->currentIndex();
  if (index >= 0 && index < m_profiles.size()) {
    m_profiles[index].setName(name);
    m_profileComboBox->setItemText(index, name);
  }
}

/**
 * Update profile from GUI controls.
 */
void BatchImportDialog::setProfileFromGuiControls()
{
  QList<BatchImportProfile::Source> sources =
      m_profileModel->getBatchImportSources();
  if (m_profiles.isEmpty() && !sources.isEmpty()) {
    addNewProfile();
    m_profileComboBox->setEditText(m_profiles.at(0).getName());
  }
  if (m_profileIdx >= 0 && m_profileIdx < m_profiles.size()) {
    BatchImportProfile& profile = m_profiles[m_profileIdx];
    profile.setSources(sources);
  }
}

/**
 * Update GUI controls from profiles.
 */
void BatchImportDialog::setGuiControlsFromProfile()
{
  if (m_profiles.isEmpty()) {
    m_profileIdx = -1;
    m_profileComboBox->clear();
    m_profileModel->setBatchImportSources(QList<BatchImportProfile::Source>());
    return;
  }

  if (m_profileIdx < 0 || m_profileIdx >= m_profiles.size())
    m_profileIdx = 0;
  m_profileModel->setBatchImportSources(m_profiles.at(m_profileIdx).getSources());
  if (m_profileComboBox->count() == m_profiles.size()) {
    m_profileComboBox->setItemText(m_profileIdx, m_profiles.at(m_profileIdx).getName());
  } else {
    m_profileComboBox->clear();
    const auto profiles = m_profiles;
    for (const BatchImportProfile& profile : profiles) {
      m_profileComboBox->addItem(profile.getName());
    }
  }
  m_profileComboBox->setCurrentIndex(m_profileIdx);
}

/**
 * Set the filter combo box and line edit from the configuration.
 */
void BatchImportDialog::setProfileFromConfig()
{
  const BatchImportConfig& batchImportCfg = BatchImportConfig::instance();
  const QStringList names = batchImportCfg.profileNames();
  const QStringList sources = batchImportCfg.profileSources();

  m_profiles.clear();
  for (auto namesIt = names.constBegin(), sourcesIt = sources.constBegin();
       namesIt != names.constEnd() && sourcesIt != sources.constEnd();
       ++namesIt, ++sourcesIt) {
    BatchImportProfile profile;
    profile.setName(*namesIt);
    profile.setSourcesFromString(*sourcesIt);
    m_profiles.append(profile);
  }
  m_profileIdx = batchImportCfg.profileIndex();
  setGuiControlsFromProfile();
}

/**
 * Read the local settings from the configuration.
 */
void BatchImportDialog::readConfig()
{
  m_edit->clear();
  setAbortButton(false);

  const BatchImportConfig& batchImportCfg = BatchImportConfig::instance();
  Frame::TagVersion importDest = batchImportCfg.importDest();
  int index = m_destComboBox->findData(importDest);
  m_destComboBox->setCurrentIndex(index);

  setProfileFromConfig();

  if (!batchImportCfg.windowGeometry().isEmpty()) {
    restoreGeometry(batchImportCfg.windowGeometry());
  }
}

/**
 * Save the local settings to the configuration.
 */
void BatchImportDialog::saveConfig()
{
  BatchImportConfig& batchImportCfg = BatchImportConfig::instance();
  batchImportCfg.setImportDest(Frame::tagVersionCast(
    m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt()));

  QStringList names, sources;
  setProfileFromGuiControls();
  names.reserve(m_profiles.size());
  sources.reserve(m_profiles.size());
  const auto profiles = m_profiles;
  for (const BatchImportProfile& profile : profiles) {
    names.append(profile.getName());
    sources.append(profile.getSourcesAsString());
  }
  batchImportCfg.setProfileNames(names);
  batchImportCfg.setProfileSources(sources);
  batchImportCfg.setProfileIndex(m_profileComboBox->currentIndex());
  batchImportCfg.setWindowGeometry(saveGeometry());
}

/**
 * Show help.
 */
void BatchImportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("batch-import"));
}

/**
 * Show information about import event.
 * @param type import event type, enum BatchImporter::ImportEventType
 * @param text text to display
 */
void BatchImportDialog::showImportEvent(int type, const QString& text)
{
  QString eventText;
  switch (type) {
  case BatchImporter::ReadingDirectory:
    setAbortButton(true);
    eventText = tr("Reading Directory");
    break;
  case BatchImporter::Started:
    setAbortButton(true);
    eventText = tr("Started");
    break;
  case BatchImporter::SourceSelected:
    eventText = tr("Source");
    break;
  case BatchImporter::QueryingAlbumList:
    eventText = tr("Querying");
    break;
  case BatchImporter::FetchingTrackList:
  case BatchImporter::FetchingCoverArt:
    eventText = tr("Fetching");
    break;
  case BatchImporter::TrackListReceived:
    eventText = tr("Data received");
    break;
  case BatchImporter::CoverArtReceived:
    eventText = tr("Cover");
    break;
  case BatchImporter::Finished:
    setAbortButton(false);
    eventText = tr("Finished");
    break;
  case BatchImporter::Aborted:
    setAbortButton(false);
    eventText = tr("Aborted");
    break;
  case BatchImporter::Error:
    eventText = tr("Error");
  }
  if (!text.isEmpty()) {
    eventText += QLatin1String(": ");
    eventText += text;
  }
  m_edit->append(eventText);
}

/**
 * Set button to Start or Abort.
 * @param enableAbort true to set Abort button
 */
void BatchImportDialog::setAbortButton(bool enableAbort)
{
  m_isAbortButton = enableAbort;
  m_startAbortButton->setText(m_isAbortButton ? tr("A&bort") : tr("S&tart"));
}
