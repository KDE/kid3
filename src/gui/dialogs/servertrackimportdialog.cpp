/**
 * \file servertrackimportdialog.cpp
 * Generic dialog for track based import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 *
 * Copyright (C) 2005-2013  Urs Fleisch
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

#include "servertrackimportdialog.h"
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
#include <QCoreApplication>
#include "serverimporterconfig.h"
#include "contexthelp.h"
#include "servertrackimporter.h"
#include "comboboxdelegate.h"
#include "trackdatamodel.h"

/**
 * Constructor.
 *
 * @param parent          parent widget
 * @param trackDataModel track data to be filled with imported values,
 *                        is passed with filenames set
 */
ServerTrackImportDialog::ServerTrackImportDialog(QWidget* parent,
                                                 TrackDataModel* trackDataModel)
  : QDialog(parent), m_statusBar(nullptr),
    m_client(nullptr), m_trackDataModel(trackDataModel)
{
  setObjectName(QLatin1String("ServerTrackImportDialog"));
  setModal(true);

  auto vlayout = new QVBoxLayout(this);
  auto serverLayout = new QHBoxLayout;
  m_serverLabel = new QLabel(tr("&Server:"), this);
  m_serverComboBox = new QComboBox(this);
  m_serverComboBox->setEditable(true);
  m_serverComboBox->setSizePolicy(
    QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  m_serverLabel->setBuddy(m_serverComboBox);
  serverLayout->addWidget(m_serverLabel);
  serverLayout->addWidget(m_serverComboBox);
  vlayout->addLayout(serverLayout);

  m_albumTableModel = new QStandardItemModel(this);
  m_albumTableModel->setColumnCount(2);
  m_albumTableModel->setHorizontalHeaderLabels({
    QLatin1String("08 A Not So Short Title/Medium Sized Artist - And The Album Title [2005]"),
    QLatin1String("A Not So Short State")
  });
  m_albumTable = new QTableView(this);
  m_albumTable->setModel(m_albumTableModel);
  m_albumTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_albumTable->setSelectionMode(QAbstractItemView::NoSelection);
  m_albumTable->resizeColumnsToContents();
  m_albumTable->setItemDelegateForColumn(0, new ComboBoxDelegate(this));
  m_albumTableModel->setHorizontalHeaderLabels({
    tr("Track Title/Artist - Album"),
    tr("State")
  });
  initTable();
  vlayout->addWidget(m_albumTable);

  auto hlayout = new QHBoxLayout;
  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  m_helpButton = new QPushButton(tr("&Help"), this);
  m_helpButton->setAutoDefault(false);
  m_saveButton = new QPushButton(tr("&Save Settings"), this);
  m_saveButton->setAutoDefault(false);
  QPushButton* okButton = new QPushButton(tr("&OK"), this);
  QPushButton* applyButton = new QPushButton(tr("&Apply"), this);
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
  hlayout->addWidget(m_helpButton);
  hlayout->addWidget(m_saveButton);
  hlayout->addItem(hspacer);
  hlayout->addWidget(okButton);
  hlayout->addWidget(applyButton);
  hlayout->addWidget(cancelButton);
  // auto default is switched off to use the return key to set the server
  // configuration
  okButton->setAutoDefault(false);
  okButton->setDefault(true);
  cancelButton->setAutoDefault(false);
  applyButton->setAutoDefault(false);
  connect(m_helpButton, &QAbstractButton::clicked, this, &ServerTrackImportDialog::showHelp);
  connect(m_saveButton, &QAbstractButton::clicked, this, &ServerTrackImportDialog::saveConfig);
  connect(okButton, &QAbstractButton::clicked, this, &ServerTrackImportDialog::accept);
  connect(cancelButton, &QAbstractButton::clicked, this, &ServerTrackImportDialog::reject);
  connect(applyButton, &QAbstractButton::clicked, this, &ServerTrackImportDialog::apply);
  vlayout->addLayout(hlayout);

  m_statusBar = new QStatusBar(this);
  vlayout->addWidget(m_statusBar);
  connect(m_albumTable->selectionModel(),
          &QItemSelectionModel::currentRowChanged,
          this, &ServerTrackImportDialog::showFilenameInStatusBar);
}

/**
 * Destructor.
 */
ServerTrackImportDialog::~ServerTrackImportDialog()
{
  stopClient();
}

/**
 * Set importer to be used.
 *
 * @param source  import source to use
 */
void ServerTrackImportDialog::setImportSource(ServerTrackImporter* source)
{
  if (m_client) {
    disconnect(m_client, &ServerTrackImporter::statusChanged,
               this, &ServerTrackImportDialog::setFileStatus);
    disconnect(m_client, &ServerTrackImporter::resultsReceived,
               this, &ServerTrackImportDialog::setResults);
  }
  m_client = source;

  if (m_client) {
    connect(m_client, &ServerTrackImporter::statusChanged,
            this, &ServerTrackImportDialog::setFileStatus);
    connect(m_client, &ServerTrackImporter::resultsReceived,
            this, &ServerTrackImportDialog::setResults);

    setWindowTitle(QCoreApplication::translate("@default", m_client->name()));
    if (m_client->defaultServer()) {
      m_serverLabel->show();
      m_serverComboBox->show();
      if (m_client->serverList()) {
        QStringList strList;
        for (const char** sl = m_client->serverList(); *sl != nullptr; ++sl) {
          strList += QString::fromLatin1(*sl); // clazy:exclude=reserve-candidates
        }
        m_serverComboBox->clear();
        m_serverComboBox->addItems(strList);
      }
    } else {
      m_serverLabel->hide();
      m_serverComboBox->hide();
    }
    if (m_client->helpAnchor()) {
      m_helpButton->show();
    } else {
      m_helpButton->hide();
    }
    if (m_client->config()) {
      m_saveButton->show();
    } else {
      m_saveButton->hide();
    }
  }
}

/**
 * Initialize the table model.
 * Has to be called before reusing the dialog with new track data.
 */
void ServerTrackImportDialog::initTable()
{
  if (m_client && m_client->config()) {
    setServer(m_client->config()->server());
  }

  int numRows = 0;
  const ImportTrackDataVector& trackDataVector(m_trackDataModel->trackData());
  for (auto it = trackDataVector.constBegin(); it != trackDataVector.constEnd(); ++it) {
    if (it->isEnabled()) {
      ++numRows;
    }
  }

  m_trackResults.resize(numRows);
  m_albumTableModel->setRowCount(numRows);
  for (int i = 0; i < numRows; ++i) {
    auto item = new QStandardItem;
    QStringList cbItems;
    cbItems << tr("No result") << tr("Unknown");
    item->setData(cbItems.first(), Qt::EditRole);
    item->setData(cbItems, Qt::UserRole);
    m_albumTableModel->setItem(i, 0, item);
    item = new QStandardItem(tr("Unknown"));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    m_albumTableModel->setItem(i, 1, item);
  }
  showFilenameInStatusBar(m_albumTable->currentIndex());
}

/**
 * Clear all results.
 */
void ServerTrackImportDialog::clearResults()
{
  const int numRows = m_trackResults.size();
  for (int i = 0; i < numRows; ++i) {
    m_trackResults[i].clear();
    setFileStatus(i, tr("Unknown"));
    updateFileTrackData(i);
  }
}

/**
 * Create and start the track import client.
 */
void ServerTrackImportDialog::startClient()
{
  if (m_client) {
    clearResults();
    ServerImporterConfig cfg;
    cfg.setServer(getServer());
    m_client->setConfig(&cfg);
    m_client->start();
  }
}

/**
 * Stop and destroy the track import client.
 */
void ServerTrackImportDialog::stopClient()
{
  if (m_client) {
    m_client->stop();
  }
}

/**
 * Hides the dialog and sets the result to QDialog::Accepted.
 */
void ServerTrackImportDialog::accept()
{
  apply();
  stopClient();
  QDialog::accept();
}

/**
 * Hides the dialog and sets the result to QDialog::Rejected.
 */
void ServerTrackImportDialog::reject()
{
  stopClient();
  QDialog::reject();
}

/**
 * Apply imported data.
 */
void ServerTrackImportDialog::apply()
{
  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  trackDataVector.setCoverArtUrl(QUrl());
  auto it = trackDataVector.begin();
  bool newTrackData = false;
  int numRows = m_albumTableModel->rowCount();
  for (int index = 0; index < numRows; ++index) {
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
int ServerTrackImportDialog::exec()
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
void ServerTrackImportDialog::setFileStatus(int index, const QString& status)
{
  m_albumTableModel->setData(m_albumTableModel->index(index, 1),
                             status);
}

/**
 * Update the track data combo box of a file.
 *
 * @param index  index of file
 */
void ServerTrackImportDialog::updateFileTrackData(int index)
{
  QStringList stringList;
  const ImportTrackDataVector& trackData = m_trackResults.at(index);
  const int numResults = trackData.size();
  QString str(numResults == 0 ?
              tr("No result") : tr("No result selected"));
  stringList.push_back(str);
  for (auto it = trackData.constBegin(); it != trackData.constEnd(); ++it) {
    str.sprintf("%02d ", (*it).getTrack());
    str += (*it).getTitle();
    str += QLatin1Char('/');
    str += (*it).getArtist();
    str += QLatin1String(" - ");
    str += (*it).getAlbum();
    if ((*it).getYear() > 0) {
      str += QString(QLatin1String(" [%1]")).arg((*it).getYear());
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
 * Set result list for a file.
 *
 * @param index           index of file
 * @param trackDataVector result list
 */
void ServerTrackImportDialog::setResults(
  int index, ImportTrackDataVector& trackDataVector)
{
  m_trackResults[index] = trackDataVector;
  updateFileTrackData(index);
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString ServerTrackImportDialog::getServer() const
{
  QString server(m_serverComboBox->currentText());
  if (server.isEmpty() && m_client && m_client->defaultServer()) {
    server = QString::fromLatin1(m_client->defaultServer());
  }
  return server;
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void ServerTrackImportDialog::setServer(const QString& srv)
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
void ServerTrackImportDialog::saveConfig()
{
  if (m_client && m_client->config()) {
    m_client->config()->setServer(getServer());
  }
}

/**
 * Show help.
 */
void ServerTrackImportDialog::showHelp()
{
  if (m_client && m_client->helpAnchor()) {
    ContextHelp::displayHelp(QString::fromLatin1(m_client->helpAnchor()));
  }
}

/**
 * Show the name of the current track in the status bar.
 *
 * @param index model index
 */
void ServerTrackImportDialog::showFilenameInStatusBar(const QModelIndex& index)
{
  if (m_statusBar) {
    int row = index.row();

    int rowNr = 0;
    const ImportTrackDataVector& trackDataVector(m_trackDataModel->trackData());
    for (auto it = trackDataVector.constBegin();
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
