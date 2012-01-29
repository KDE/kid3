/**
 * \file musicbrainzdialog.cpp
 * MusicBrainz import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 *
 * Copyright (C) 2005-2012  Urs Fleisch
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

#include "musicbrainzdialog.h"
#ifdef HAVE_CHROMAPRINT

#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QStatusBar>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "configstore.h"
#include "contexthelp.h"
#include "musicbrainzclient.h"
#include "comboboxdelegate.h"
#include "trackdatamodel.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent          parent widget
 * @param trackDataModel track data to be filled with imported values,
 *                        is passed with filenames set
 */
MusicBrainzDialog::MusicBrainzDialog(QWidget* parent,
                                     TrackDataModel* trackDataModel)
  : QDialog(parent), m_statusBar(0),
    m_client(0), m_trackDataModel(trackDataModel)
{
  setObjectName("MusicBrainzDialog");
  setModal(true);
  setWindowTitle(i18n("MusicBrainz Fingerprint"));

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  if (!vlayout) {
    return;
  }
  vlayout->setMargin(6);
  vlayout->setSpacing(6);
  QHBoxLayout* serverLayout = new QHBoxLayout;
  QLabel* serverLabel = new QLabel(i18n("&Server:"), this);
  m_serverComboBox = new QComboBox(this);
  if (serverLayout && serverLabel && m_serverComboBox) {
    m_serverComboBox->setEditable(true);
    static const char* serverList[] = {
      "musicbrainz.org:80",
      "de.musicbrainz.org:80",
      "nl.musicbrainz.org:80",
      0                  // end of StrList
    };
    QStringList strList;
    for (const char** sl = serverList; *sl != 0; ++sl) {
      strList += *sl;
    }
    m_serverComboBox->addItems(strList);
    m_serverComboBox->setSizePolicy(
      QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    serverLabel->setBuddy(m_serverComboBox);
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(m_serverComboBox);
    connect(m_serverComboBox, SIGNAL(activated(int)),
            this, SLOT(setClientConfig()));
    vlayout->addLayout(serverLayout);
  }

  m_albumTableModel = new QStandardItemModel(this);
  m_albumTableModel->setColumnCount(2);
  m_albumTableModel->setHorizontalHeaderLabels(
    QStringList() <<
    "08 A Not So Short Title/Medium Sized Artist - And The Album Title [2005]" <<
    "A Not So Short State");
  m_albumTable = new QTableView(this);
  m_albumTable->setModel(m_albumTableModel);
  m_albumTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
  m_albumTable->setSelectionMode(QAbstractItemView::NoSelection);
  m_albumTable->resizeColumnsToContents();
  m_albumTable->setItemDelegateForColumn(0, new ComboBoxDelegate(this));
  m_albumTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
  m_albumTableModel->setHorizontalHeaderLabels(
    QStringList() <<
    i18n("Track Title/Artist - Album") <<
    i18n("State"));
  initTable();
  vlayout->addWidget(m_albumTable);

  QHBoxLayout* hlayout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
  QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
  QPushButton* okButton = new QPushButton(i18n("&OK"), this);
  QPushButton* applyButton = new QPushButton(i18n("&Apply"), this);
  QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
  if (hlayout && helpButton && saveButton &&
      okButton && cancelButton && applyButton) {
    hlayout->addWidget(helpButton);
    hlayout->addWidget(saveButton);
    hlayout->addItem(hspacer);
    hlayout->addWidget(okButton);
    hlayout->addWidget(applyButton);
    hlayout->addWidget(cancelButton);
    // auto default is switched off to use the return key to set the server
    // configuration
    okButton->setAutoDefault(false);
    cancelButton->setAutoDefault(false);
    applyButton->setAutoDefault(false);
    connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    vlayout->addLayout(hlayout);
  }

  m_statusBar = new QStatusBar(this);
  if (m_statusBar) {
    vlayout->addWidget(m_statusBar);
    if (m_albumTable) {
      connect(m_albumTable->selectionModel(),
              SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
              this, SLOT(showFilenameInStatusBar(QModelIndex)));
    }
  }
}

/**
 * Destructor.
 */
MusicBrainzDialog::~MusicBrainzDialog()
{
  stopClient();
}

/**
 * Initialize the table model.
 * Has to be called before reusing the dialog with new track data.
 */
void MusicBrainzDialog::initTable()
{
  setServer(ConfigStore::s_musicBrainzCfg.m_server);

  unsigned numRows = 0;
  const ImportTrackDataVector& trackDataVector(m_trackDataModel->trackData());
  for (ImportTrackDataVector::const_iterator it = trackDataVector.constBegin();
       it != trackDataVector.constEnd();
       ++it) {
    if (it->isEnabled()) {
      ++numRows;
    }
  }

  m_trackResults.resize(numRows);
  m_albumTableModel->setRowCount(numRows);
  for (unsigned i = 0; i < numRows; ++i) {
    QStandardItem* item = new QStandardItem;
    QStringList cbItems;
    cbItems << i18n("No result") << i18n("Unknown");
    item->setData(cbItems.first(), Qt::EditRole);
    item->setData(cbItems, Qt::UserRole);
    m_albumTableModel->setItem(i, 0, item);
    item = new QStandardItem(i18n("Unknown"));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    m_albumTableModel->setItem(i, 1, item);
  }
  showFilenameInStatusBar(m_albumTable->currentIndex());
}

/**
 * Clear all results.
 */
void MusicBrainzDialog::clearResults()
{
  unsigned numRows = m_trackResults.size();
  for (unsigned i = 0; i < numRows; ++i) {
    m_trackResults[i].clear();
    setFileStatus(i, i18n("Unknown"));
    updateFileTrackData(i);
  }
}

/**
 * Set the configuration in the client.
 */
void MusicBrainzDialog::setClientConfig()
{
  if (m_client) {
    m_client->setConfig(getServer());
  }
}

/**
 * Create and start the MusicBrainz client.
 */
void MusicBrainzDialog::startClient()
{
  clearResults();
  if (!m_client) {
    m_client = new MusicBrainzClient(m_trackDataModel);
    setClientConfig();
    connect(m_client, SIGNAL(statusChanged(int, QString)),
            this, SLOT(setFileStatus(int, QString)));
    connect(m_client, SIGNAL(metaDataReceived(int, ImportTrackData&)),
            this, SLOT(setMetaData(int, ImportTrackData&)));
    connect(m_client, SIGNAL(resultsReceived(int, ImportTrackDataVector&)),
            this, SLOT(setResults(int, ImportTrackDataVector&)));
    m_client->addFiles();
  }
}

/**
 * Stop and destroy the MusicBrainz client.
 */
void MusicBrainzDialog::stopClient()
{
  if (m_client) {
    m_client->disconnect();
    delete m_client;
    m_client = 0;
  }
}

/**
 * Hides the dialog and sets the result to QDialog::Accepted.
 */
void MusicBrainzDialog::accept()
{
  apply();
  stopClient();
  QDialog::accept();
}

/**
 * Hides the dialog and sets the result to QDialog::Rejected.
 */
void MusicBrainzDialog::reject()
{
  stopClient();
  QDialog::reject();
}

/**
 * Apply imported data.
 */
void MusicBrainzDialog::apply()
{
  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  ImportTrackDataVector::iterator it = trackDataVector.begin();
  bool newTrackData = false;
  unsigned numRows = m_albumTableModel->rowCount();
  for (unsigned index = 0; index < numRows; ++index) {
    while (it != trackDataVector.end() && !it->isEnabled()) {
      ++it;
    }
    if (it == trackDataVector.end()) {
      break;
    }
    QModelIndex idx(m_albumTableModel->index(index, 0));
    if (idx.isValid()) {
      int selectedItem = idx.data(Qt::UserRole).toStringList().indexOf(
            idx.data(Qt::EditRole).toString());
      if (selectedItem > 0) {
        const ImportTrackData& selectedData =
          m_trackResults[index][selectedItem - 1];
        it->setTitle(selectedData.getTitle());
        it->setArtist(selectedData.getArtist());
        it->setAlbum(selectedData.getAlbum());
        it->setTrack(selectedData.getTrack());
        it->setYear(selectedData.getYear());
        it->setImportDuration(selectedData.getImportDuration());
        newTrackData = true;
      }
    }
    ++it;
  }
  if (newTrackData) {
    m_trackDataModel->setTrackData(trackDataVector);
    emit trackDataUpdated();
  }
}

/**
 * Shows the dialog as a modal dialog.
 */
int MusicBrainzDialog::exec()
{
  startClient();
  return QDialog::exec();
}

/**
 * Set the status of a file.
 *
 * @param index  index of file
 * @param status status string
 */
void MusicBrainzDialog::setFileStatus(int index, QString status)
{
  m_albumTableModel->setData(m_albumTableModel->index(index, 1),
                             status);
}

/**
 * Update the track data combo box of a file.
 *
 * @param index  index of file
 */
void MusicBrainzDialog::updateFileTrackData(int index)
{
  QStringList stringList;
  unsigned numResults = m_trackResults[index].size();
  QString str(numResults == 0 ?
              i18n("No result") : i18n("No result selected"));
  stringList.push_back(str);
  for (ImportTrackDataVector::const_iterator it = m_trackResults[index].begin();
       it != m_trackResults[index].end();
       ++it) {
    str.sprintf("%02d ", (*it).getTrack());
    str += (*it).getTitle();
    str += '/';
    str += (*it).getArtist();
    str += " - ";
    str += (*it).getAlbum();
    if ((*it).getYear() > 0) {
      str += QString(" [%1]").arg((*it).getYear());
    }
    stringList.push_back(str);
  }
  m_albumTableModel->setData(m_albumTableModel->index(index, 0),
                             stringList, Qt::UserRole);
  m_albumTableModel->setData(m_albumTableModel->index(index, 0),
                             stringList.at(numResults == 1 ? 1 : 0),
                             Qt::EditRole);
}

/**
 * Set meta data for a file.
 *
 * @param index     index of file
 * @param trackData meta data
 */
void MusicBrainzDialog::setMetaData(int index, ImportTrackData& trackData)
{
  m_trackResults[index].clear();
  m_trackResults[index].push_back(trackData);
  updateFileTrackData(index);
}

/**
 * Set result list for a file.
 *
 * @param index           index of file
 * @param trackDataVector result list
 */
void MusicBrainzDialog::setResults(
  int index, ImportTrackDataVector& trackDataVector)
{
  m_trackResults[index] = trackDataVector;
  updateFileTrackData(index);
  for (
     ImportTrackDataVector::const_iterator
       it = trackDataVector.begin();
       it != trackDataVector.end();
       ++it) {
  }
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString MusicBrainzDialog::getServer() const
{
  QString server(m_serverComboBox->currentText());
  if (server.isEmpty()) {
    server = "musicbrainz.org:80";
  }
  return server;
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void MusicBrainzDialog::setServer(const QString& srv)
{
  int idx = m_serverComboBox->findText(srv);
  if (idx >= 0) {
    m_serverComboBox->setCurrentIndex(idx);
  } else {
    m_serverComboBox->addItem(srv);
    m_serverComboBox->setCurrentIndex(m_serverComboBox->count() - 1);
  }
}

/**
 * Save the local settings to the configuration.
 */
void MusicBrainzDialog::saveConfig()
{
  ConfigStore::s_musicBrainzCfg.m_server = getServer();
}

/**
 * Show help.
 */
void MusicBrainzDialog::showHelp()
{
  ContextHelp::displayHelp("import-musicbrainz");
}

/**
 * Show the name of the current track in the status bar.
 *
 * @param row table row
 */
void MusicBrainzDialog::showFilenameInStatusBar(const QModelIndex& index)
{
  if (m_statusBar) {
    int row = index.row();

    int rowNr = 0;
    const ImportTrackDataVector& trackDataVector(m_trackDataModel->trackData());
    for (ImportTrackDataVector::const_iterator it = trackDataVector.constBegin();
         it != trackDataVector.constEnd();
         ++it) {
      if (it->isEnabled()) {
        if (rowNr == row) {
          m_statusBar->showMessage(it->getFilename());
          return;
        }
        ++rowNr;
      }
    }
    m_statusBar->clearMessage();
  }
}

#endif // HAVE_CHROMAPRINT
