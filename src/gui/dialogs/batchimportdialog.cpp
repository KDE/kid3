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
#include "batchimportsourcedialog.h"
#include "batchimportconfig.h"
#include "batchimportsourcesmodel.h"
#include "serverimporter.h"
#include "configstore.h"
#include "contexthelp.h"
#include "abstractlistedit.h"
#include "qtcompatmac.h"

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
                                     QWidget* parent = 0);

  /**
   * Destructor.
   */
  virtual ~BatchImportSourceListEdit();

  /**
   * Set names of import servers.
   * @param servers server names
   */
  void setServerNames(const QStringList& servers) {
    m_serverNames = servers;
  }

public slots:
  /**
   * Add a new item.
   */
  virtual void addItem();

  /**
   * Edit the selected item.
   */
  virtual void editItem();

private:
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
  setObjectName("BatchImportSourceListEdit");
  m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
#if QT_VERSION >= 0x050000
  m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
  m_tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
}

/**
 * Destructor.
 */
BatchImportSourceListEdit::~BatchImportSourceListEdit()
{
}

/**
 * Add a new item.
 */
void BatchImportSourceListEdit::addItem()
{
  BatchImportSourceDialog* dialog = new BatchImportSourceDialog(this);
  dialog->setServerNames(m_serverNames);
  if (dialog->exec() == QDialog::Accepted) {
    BatchImportProfile::Source source;
    dialog->getSource(source);
    if (BatchImportSourcesModel* model =
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
    if (BatchImportSourcesModel* model =
        qobject_cast<BatchImportSourcesModel*>(getItemView()->model())) {
      BatchImportProfile::Source source;
      model->getBatchImportSource(index.row(), source);
      BatchImportSourceDialog* dialog = new BatchImportSourceDialog(this);
      dialog->setServerNames(m_serverNames);
      dialog->setSource(source);
      if (dialog->exec() == QDialog::Accepted) {
        dialog->getSource(source);
        model->setBatchImportSource(index.row(), source);
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
  setObjectName("BatchImportDialog");
  setWindowTitle(i18n("Automatic Import"));
  setSizeGripEnabled(true);

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  vlayout->setMargin(6);
  vlayout->setSpacing(6);
  QSplitter* splitter = new QSplitter(Qt::Vertical);

  m_edit = new QTextEdit(this);
  m_edit->setReadOnly(true);
  m_edit->setAcceptRichText(false);
  splitter->addWidget(m_edit);

  QWidget* profileWidget = new QWidget;
  QVBoxLayout* profileLayout = new QVBoxLayout(profileWidget);

  QHBoxLayout* destLayout = new QHBoxLayout;
  QLabel* destLabel = new QLabel(i18n("D&estination:"));
  destLayout->addWidget(destLabel);
  m_destComboBox = new QComboBox;
  m_destComboBox->setEditable(false);
  m_destComboBox->addItem(i18n("Tag 1"), TrackData::TagV1);
  m_destComboBox->addItem(i18n("Tag 2"), TrackData::TagV2);
  m_destComboBox->addItem(i18n("Tag 1 and Tag 2"), TrackData::TagV2V1);
  destLabel->setBuddy(m_destComboBox);
  destLayout->addWidget(m_destComboBox);
  destLayout->addStretch();
  profileLayout->addLayout(destLayout);

  QHBoxLayout* nameLayout = new QHBoxLayout;
  nameLayout->setSpacing(6);
  QLabel* profileLabel = new QLabel(i18n("&Profile:"));
  nameLayout->addWidget(profileLabel);
  m_profileComboBox = new QComboBox;
  m_profileComboBox->setEditable(true);
  connect(m_profileComboBox, SIGNAL(activated(int)),
          this, SLOT(changeProfile(int)));
  connect(m_profileComboBox, SIGNAL(editTextChanged(QString)),
          this, SLOT(changeProfileName(QString)));
  profileLabel->setBuddy(m_profileComboBox);
  nameLayout->addWidget(m_profileComboBox, 1);
  QPushButton* profileAddButton = new QPushButton(i18n("Add"));
  connect(profileAddButton, SIGNAL(clicked()),
          this, SLOT(addProfile()));
  nameLayout->addWidget(profileAddButton);
  QPushButton* profileRemoveButton = new QPushButton(i18n("Remove"));
  connect(profileRemoveButton, SIGNAL(clicked()),
          this, SLOT(removeProfile()));
  nameLayout->addWidget(profileRemoveButton);
  profileLayout->addLayout(nameLayout);

  QStringList servers;
  foreach (const ServerImporter* si, m_importers) {
    servers.append(si->name());
  }
  m_profileModel = new BatchImportSourcesModel(this);
  BatchImportSourceListEdit* profileListEdit =
      new BatchImportSourceListEdit(m_profileModel, this);
  profileListEdit->setServerNames(servers);
  profileLayout->addWidget(profileListEdit);
  splitter->addWidget(profileWidget);
  vlayout->addWidget(splitter);

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
  hlayout->addStretch();

  m_startAbortButton = new QPushButton(this);
  setAbortButton(false);
  QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
  m_startAbortButton->setAutoDefault(false);
  closeButton->setAutoDefault(false);
  hlayout->addWidget(m_startAbortButton);
  hlayout->addWidget(closeButton);
  connect(m_startAbortButton, SIGNAL(clicked()), this, SLOT(startOrAbortImport()));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(this, SIGNAL(rejected()), this, SIGNAL(abort()));

  vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
BatchImportDialog::~BatchImportDialog()
{}

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
            TrackData::tagVersionCast(
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
  profile.setName(i18n("New"));
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
    foreach (const BatchImportProfile& profile, m_profiles) {
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
  const QStringList& names = ConfigStore::s_batchImportCfg.m_profileNames;
  const QStringList& sources = ConfigStore::s_batchImportCfg.m_profileSources;

  m_profiles.clear();
  QStringList::const_iterator namesIt, sourcesIt;
  for (namesIt = names.constBegin(), sourcesIt = sources.constBegin();
       namesIt != names.constEnd() && sourcesIt != sources.constEnd();
       ++namesIt, ++sourcesIt) {
    BatchImportProfile profile;
    profile.setName(*namesIt);
    profile.setSourcesFromString(*sourcesIt);
    m_profiles.append(profile);
  }
  m_profileIdx = ConfigStore::s_batchImportCfg.m_profileIdx;
  setGuiControlsFromProfile();
}

/**
 * Read the local settings from the configuration.
 */
void BatchImportDialog::readConfig()
{
  m_edit->clear();
  setAbortButton(false);

  TrackData::TagVersion importDest = ConfigStore::s_batchImportCfg.m_importDest;
  int index = m_destComboBox->findData(importDest);
  m_destComboBox->setCurrentIndex(index);

  setProfileFromConfig();

  if (!ConfigStore::s_batchImportCfg.m_windowGeometry.isEmpty()) {
    restoreGeometry(ConfigStore::s_batchImportCfg.m_windowGeometry);
  }
}

/**
 * Save the local settings to the configuration.
 */
void BatchImportDialog::saveConfig()
{
  ConfigStore::s_batchImportCfg.m_importDest = TrackData::tagVersionCast(
    m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt());

  QStringList& names = ConfigStore::s_batchImportCfg.m_profileNames;
  QStringList& sources = ConfigStore::s_batchImportCfg.m_profileSources;
  names.clear();
  sources.clear();
  setProfileFromGuiControls();
  foreach (const BatchImportProfile& profile, m_profiles) {
    names.append(profile.getName());
    sources.append(profile.getSourcesAsString());
  }
  ConfigStore::s_batchImportCfg.m_profileIdx = m_profileComboBox->currentIndex();
  ConfigStore::s_batchImportCfg.m_windowGeometry = saveGeometry();
}

/**
 * Show help.
 */
void BatchImportDialog::showHelp()
{
  ContextHelp::displayHelp("batch-import");
}

/**
 * Show information about import event.
 * @param type import event type
 * @param text text to display
 */
void BatchImportDialog::showImportEvent(BatchImportProfile::ImportEventType type,
                                        const QString& text)
{
  QString eventText;
  switch (type) {
  case BatchImportProfile::ReadingDirectory:
    setAbortButton(true);
    eventText = i18n("Reading Directory");
    break;
  case BatchImportProfile::Started:
    setAbortButton(true);
    eventText = i18n("Started");
    break;
  case BatchImportProfile::SourceSelected:
    eventText = i18n("Source");
    break;
  case BatchImportProfile::QueryingAlbumList:
    eventText = i18n("Querying");
    break;
  case BatchImportProfile::FetchingTrackList:
  case BatchImportProfile::FetchingCoverArt:
    eventText = i18n("Fetching");
    break;
  case BatchImportProfile::TrackListReceived:
    eventText = i18n("Data received");
    break;
  case BatchImportProfile::CoverArtReceived:
    eventText = i18n("Cover");
    break;
  case BatchImportProfile::Finished:
    setAbortButton(false);
    eventText = i18n("Finished");
    break;
  case BatchImportProfile::Aborted:
    setAbortButton(false);
    eventText = i18n("Aborted");
    break;
  case BatchImportProfile::Error:
    eventText = i18n("Error");
  }
  if (!text.isEmpty()) {
    eventText += ": ";
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
  m_startAbortButton->setText(m_isAbortButton ? i18n("A&bort") : i18n("S&tart"));
}
