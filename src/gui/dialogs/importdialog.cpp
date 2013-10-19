/**
 * \file importdialog.cpp
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "importdialog.h"
#include <QLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QBitArray>
#include <QToolTip>
#include <QTableView>
#include <QHeaderView>
#include <QList>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QMenu>
#include <QCoreApplication>
#include "config.h"
#include "genres.h"
#include "serverimporter.h"
#include "servertrackimporter.h"
#include "serverimportdialog.h"
#include "servertrackimportdialog.h"
#include "textimportdialog.h"
#include "tagimportdialog.h"
#include "contexthelp.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "trackdatamodel.h"
#include "frametablemodel.h"
#include "trackdatamatcher.h"
#include "qtcompatmac.h"
#include "iplatformtools.h"

namespace {

/**
 * Get list of frame types whose visibility can be changed using a context menu.
 * @return list of frame types of Frame::Type or
 *         TrackDataModel::TrackProperties.
 */
QList<int> checkableFrameTypes() {
  return QList<int>()
      << TrackDataModel::FT_FileName << TrackDataModel::FT_FilePath;
}

}

/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param parent        parent widget
 * @param caption       dialog title
 * @param trackDataModel track data to be filled with imported values,
 *                      is passed with durations of files set
 * @param importers     server importers
 * @param trackImporters server track importers
 */
ImportDialog::ImportDialog(IPlatformTools* platformTools,
                           QWidget* parent, QString& caption,
                           TrackDataModel* trackDataModel,
                           const QList<ServerImporter*>& importers,
                           const QList<ServerTrackImporter*>& trackImporters) :
  QDialog(parent), m_platformTools(platformTools),
  m_autoStartSubDialog(-1), m_columnVisibility(0ULL),
  m_trackDataModel(trackDataModel), m_importers(importers),
  m_trackImporters(trackImporters)
{
  setObjectName(QLatin1String("ImportDialog"));
  setModal(false);
  setWindowTitle(caption);
  setSizeGripEnabled(true);

  m_serverImportDialog = 0;
  m_textImportDialog = 0;
  m_tagImportDialog = 0;
  m_serverTrackImportDialog = 0;

  QVBoxLayout* vlayout = new QVBoxLayout(this);

  m_trackDataTable = new QTableView(this);
  m_trackDataTable->setModel(m_trackDataModel);
  m_trackDataTable->resizeColumnsToContents();
  m_trackDataTable->setItemDelegateForColumn(
        m_trackDataModel->columnForFrameType(Frame::FT_Genre),
        new FrameItemDelegate(this));
#if QT_VERSION >= 0x050000
  m_trackDataTable->verticalHeader()->setSectionsMovable(true);
  m_trackDataTable->horizontalHeader()->setSectionsMovable(true);
#else
  m_trackDataTable->verticalHeader()->setMovable(true);
  m_trackDataTable->horizontalHeader()->setMovable(true);
#endif
  m_trackDataTable->horizontalHeader()->setContextMenuPolicy(
        Qt::CustomContextMenu);
  connect(m_trackDataTable->verticalHeader(), SIGNAL(sectionMoved(int,int,int)),
          this, SLOT(moveTableRow(int,int,int)));
  connect(m_trackDataTable->horizontalHeader(),
          SIGNAL(customContextMenuRequested(QPoint)),
      this, SLOT(showTableHeaderContextMenu(QPoint)));
  vlayout->addWidget(m_trackDataTable);

  QHBoxLayout* accuracyLayout = new QHBoxLayout;
  QLabel* accuracyLabel = new QLabel(tr("Accuracy:"));
  accuracyLayout->addWidget(accuracyLabel);
  m_accuracyPercentLabel = new QLabel(QLatin1String("-"));
  m_accuracyPercentLabel->setMinimumWidth(
        m_accuracyPercentLabel->fontMetrics().width(QLatin1String("100%")));
  accuracyLayout->addWidget(m_accuracyPercentLabel);
  QLabel* coverArtLabel = new QLabel(tr("Cover Art:"));
  accuracyLayout->addWidget(coverArtLabel);
  m_coverArtUrlLabel = new QLabel(QLatin1String(" -"));
  accuracyLayout->addWidget(m_coverArtUrlLabel);
  accuracyLayout->addStretch();
  vlayout->addLayout(accuracyLayout);

  QHBoxLayout* butlayout = new QHBoxLayout;
  QPushButton* fileButton = new QPushButton(tr("From F&ile/Clipboard..."));
  fileButton->setAutoDefault(false);
  butlayout->addWidget(fileButton);
  QPushButton* tagsButton = new QPushButton(tr("From T&ags..."));
  tagsButton->setAutoDefault(false);
  butlayout->addWidget(tagsButton);
  QPushButton* serverButton = new QPushButton(tr("&From Server:"));
  serverButton->setAutoDefault(false);
  butlayout->addWidget(serverButton);
  m_serverComboBox = new QComboBox;
  m_serverComboBox->setEditable(false);
  foreach (const ServerImporter* si, m_importers) {
    m_serverComboBox->addItem(QCoreApplication::translate("@default", si->name()));
  }
  foreach (const ServerTrackImporter* si, m_trackImporters) {
    m_serverComboBox->addItem(QCoreApplication::translate("@default", si->name()));
  }
  butlayout->addWidget(m_serverComboBox);
  if (m_serverComboBox->count() == 0) {
    serverButton->hide();
    m_serverComboBox->hide();
  }
  QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  butlayout->addItem(butspacer);
  QLabel* destLabel = new QLabel;
  destLabel->setText(tr("D&estination:"));
  butlayout->addWidget(destLabel);
  m_destComboBox = new QComboBox;
  m_destComboBox->setEditable(false);
  m_destComboBox->addItem(tr("Tag 1"), TrackData::TagV1);
  m_destComboBox->addItem(tr("Tag 2"), TrackData::TagV2);
  m_destComboBox->addItem(tr("Tag 1 and Tag 2"), TrackData::TagV2V1);
  destLabel->setBuddy(m_destComboBox);
  butlayout->addWidget(m_destComboBox);
  QToolButton* revertButton = new QToolButton;
  revertButton->setIcon(
        m_platformTools->iconFromTheme(QLatin1String("document-revert")));
  revertButton->setToolTip(tr("Revert"));
  connect(revertButton, SIGNAL(clicked()),
          this, SLOT(changeTagDestination()));
  butlayout->addWidget(revertButton);
  vlayout->addLayout(butlayout);

  QHBoxLayout* matchLayout = new QHBoxLayout;
  m_mismatchCheckBox = new QCheckBox(
    tr("Check maximum allowable time &difference (sec):"));
  matchLayout->addWidget(m_mismatchCheckBox);
  m_maxDiffSpinBox = new QSpinBox;
  m_maxDiffSpinBox->setMaximum(9999);
  matchLayout->addWidget(m_maxDiffSpinBox);
  QSpacerItem* matchSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                             QSizePolicy::Minimum);
  matchLayout->addItem(matchSpacer);
  QLabel* matchLabel = new QLabel(tr("Match with:"));
  matchLayout->addWidget(matchLabel);
  QPushButton* lengthButton = new QPushButton(tr("&Length"));
  lengthButton->setAutoDefault(false);
  matchLayout->addWidget(lengthButton);
  QPushButton* trackButton = new QPushButton(tr("T&rack"));
  trackButton->setAutoDefault(false);
  matchLayout->addWidget(trackButton);
  QPushButton* titleButton = new QPushButton(tr("&Title"));
  titleButton->setAutoDefault(false);
  matchLayout->addWidget(titleButton);
  vlayout->addLayout(matchLayout);

  connect(fileButton, SIGNAL(clicked()), this, SLOT(fromText()));
  connect(tagsButton, SIGNAL(clicked()), this, SLOT(fromTags()));
  connect(serverButton, SIGNAL(clicked()), this, SLOT(fromServer()));
  connect(m_serverComboBox, SIGNAL(activated(int)), this, SLOT(fromServer()));
  connect(lengthButton, SIGNAL(clicked()), this, SLOT(matchWithLength()));
  connect(trackButton, SIGNAL(clicked()), this, SLOT(matchWithTrack()));
  connect(titleButton, SIGNAL(clicked()), this, SLOT(matchWithTitle()));
  connect(m_mismatchCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPreview()));
  connect(m_maxDiffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxDiffChanged()));
  connect(this, SIGNAL(finished(int)), this, SLOT(hideSubdialogs()));

  QHBoxLayout* hlayout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  QPushButton* okButton = new QPushButton(tr("&OK"), this);
  okButton->setAutoDefault(false);
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
  cancelButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  hlayout->addWidget(saveButton);
  hlayout->addItem(hspacer);
  hlayout->addWidget(okButton);
  hlayout->addWidget(cancelButton);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
ImportDialog::~ImportDialog()
{
  delete m_textImportDialog;
  delete m_tagImportDialog;
  delete m_serverImportDialog;
  delete m_serverTrackImportDialog;
}

/**
 * Import from server and preview in table.
 */
void ImportDialog::fromServer()
{
  if (m_serverComboBox)
    displayServerImportDialog(m_serverComboBox->currentIndex());
}

/**
 * Import from text.
 */
void ImportDialog::fromText()
{
  if (!m_textImportDialog) {
    m_textImportDialog = new TextImportDialog(
          m_platformTools, this, m_trackDataModel);
    connect(m_textImportDialog, SIGNAL(trackDataUpdated()),
            this, SLOT(showPreview()));
  }
  m_textImportDialog->clear();
  m_textImportDialog->show();
}

/**
 * Import from tags.
 */
void ImportDialog::fromTags()
{
  if (!m_tagImportDialog) {
    m_tagImportDialog = new TagImportDialog(this, m_trackDataModel);
    connect(m_tagImportDialog, SIGNAL(trackDataUpdated()),
            this, SLOT(showPreview()));
  }
  m_tagImportDialog->clear();
  m_tagImportDialog->show();
}

/**
 * Display server import dialog.
 *
 * @param importerIdx importer index, if invalid but not negative the
 *                    MusicBrainz Fingerprint dialog is displayed
 */
void ImportDialog::displayServerImportDialog(int importerIdx)
{
  if (importerIdx >= 0) {
    if (importerIdx < m_importers.size()) {
      displayServerImportDialog(m_importers.at(importerIdx));
    } else if (importerIdx - m_importers.size() < m_trackImporters.size()) {
      displayServerTrackImportDialog(
            m_trackImporters.at(importerIdx - m_importers.size()));
    }
  }
}

/**
 * Display server import dialog.
 *
 * @param source import source
 */
void ImportDialog::displayServerImportDialog(ServerImporter* source)
{
  if (!m_serverImportDialog) {
    m_serverImportDialog = new ServerImportDialog(this);
    connect(m_serverImportDialog, SIGNAL(trackDataUpdated()),
            this, SLOT(showPreview()));
    connect(m_serverImportDialog, SIGNAL(accepted()),
            this, SLOT(onServerImportDialogClosed()));
  }
  m_serverImportDialog->setImportSource(source);
  m_serverImportDialog->setArtistAlbum(
        m_trackDataModel->trackData().getArtist(),
        m_trackDataModel->trackData().getAlbum());
  m_serverImportDialog->show();
}

/**
 * Import from track server and preview in table.
 *
 * @param source import source
 */
void ImportDialog::displayServerTrackImportDialog(ServerTrackImporter* source)
{
  if (!m_serverTrackImportDialog) {
    m_serverTrackImportDialog = new ServerTrackImportDialog(this, m_trackDataModel);
    connect(m_serverTrackImportDialog, SIGNAL(trackDataUpdated()),
            this, SLOT(showPreview()));
  }
  m_serverTrackImportDialog->setImportSource(source);
  m_serverTrackImportDialog->initTable();
  m_serverTrackImportDialog->exec();
}

/**
 * Hide subdialogs.
 */
void ImportDialog::hideSubdialogs()
{
  if (m_serverImportDialog)
    m_serverImportDialog->hide();
  if (m_textImportDialog)
    m_textImportDialog->hide();
  if (m_tagImportDialog)
    m_tagImportDialog->hide();
}

/**
 * Shows the dialog as a modeless dialog.
 *
 * @param importerIndex index of importer to use, -1 for none
 */
void ImportDialog::showWithSubDialog(int importerIndex)
{
  m_autoStartSubDialog = importerIndex;

  if (importerIndex >= 0 && importerIndex < m_serverComboBox->count()) {
    m_serverComboBox->setCurrentIndex(importerIndex);
  }

  show();
  if (m_autoStartSubDialog >= 0) {
    displayServerImportDialog(m_autoStartSubDialog);
  }
}

/**
 * Clear dialog data.
 */
void ImportDialog::clear()
{
  m_serverComboBox->setCurrentIndex(ImportConfig::instance().m_importServer);
  TrackData::TagVersion importDest = ImportConfig::instance().m_importDest;
  int index = m_destComboBox->findData(importDest);
  m_destComboBox->setCurrentIndex(index);
  if (importDest == TrackData::TagV1 &&
      !m_trackDataModel->trackData().isTagV1Supported()) {
    index = m_destComboBox->findData(TrackData::TagV2);
    m_destComboBox->setCurrentIndex(index);
    changeTagDestination();
  }

  m_mismatchCheckBox->setChecked(ImportConfig::instance().m_enableTimeDifferenceCheck);
  m_maxDiffSpinBox->setValue(ImportConfig::instance().m_maxTimeDifference);
  m_columnVisibility = ImportConfig::instance().m_importVisibleColumns;

  foreach (int frameType, checkableFrameTypes()) {
    if (frameType < 64) {
      int column = m_trackDataModel->columnForFrameType(frameType);
      if (column != -1) {
        m_trackDataTable->setColumnHidden(
              column, (m_columnVisibility & (1ULL << frameType)) == 0ULL);
      }
    }
  }

  if (!ImportConfig::instance().m_importWindowGeometry.isEmpty()) {
    restoreGeometry(ImportConfig::instance().m_importWindowGeometry);
  }

  showPreview();
}

/**
 * Show fields to import in text as preview in table.
 */
void ImportDialog::showPreview()
{
  // make time difference check
  bool diffCheckEnable;
  int maxDiff;
  getTimeDifferenceCheck(diffCheckEnable, maxDiff);
  m_trackDataModel->setTimeDifferenceCheck(diffCheckEnable, maxDiff);
  m_trackDataTable->scrollToTop();
  m_trackDataTable->resizeColumnsToContents();
  m_trackDataTable->resizeRowsToContents();

  int accuracy = m_trackDataModel->calculateAccuracy();
  m_accuracyPercentLabel->setText(accuracy >= 0 && accuracy <= 100
                                  ? QString::number(accuracy) + QLatin1Char('%') : QLatin1String("-"));
  QString coverArtUrl = m_trackDataModel->getTrackData().getCoverArtUrl();
  m_coverArtUrlLabel->setText(coverArtUrl.isEmpty() ? QLatin1String("-") : coverArtUrl);
}

/**
 * Called when server import dialog is closed.
 */
void ImportDialog::onServerImportDialogClosed()
{
  // This is used to prevent that the import dialog is brought behind the
  // main window when the server import dialog is closed, which happened
  // with Qt 5 on Mac OS X.
  show();
  raise();
  activateWindow();
}

/**
 * Get import destination.
 *
 * @return TagV1, TagV2 or TagV2V1 for ID3v1, ID3v2 or both.
 */
TrackData::TagVersion ImportDialog::getDestination() const
{
  return TrackData::tagVersionCast(
    m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt());
}

/**
 * Show help.
 */
void ImportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("import"));
}

/**
 * Save the local settings to the configuration.
 */
void ImportDialog::saveConfig()
{
  ImportConfig::instance().m_importDest = TrackData::tagVersionCast(
    m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt());

  ImportConfig::instance().m_importServer = m_serverComboBox->currentIndex();
  getTimeDifferenceCheck(ImportConfig::instance().m_enableTimeDifferenceCheck,
                         ImportConfig::instance().m_maxTimeDifference);
  ImportConfig::instance().m_importVisibleColumns = m_columnVisibility;

  ImportConfig::instance().m_importWindowGeometry = saveGeometry();
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */
void ImportDialog::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
  enable = m_mismatchCheckBox->isChecked();
  maxDiff = m_maxDiffSpinBox->value();
}

/**
 * Called when the maximum time difference value is changed.
 */
void ImportDialog::maxDiffChanged() {
  if (m_mismatchCheckBox->isChecked()) {
    showPreview();
  }
}

/**
 * Move a table row.
 *
 * The first parameter @a section is not used.
 * @param fromIndex index of position moved from
 * @param fromIndex index of position moved to
 */
void ImportDialog::moveTableRow(int, int fromIndex, int toIndex) {
  QHeaderView* vHeader = qobject_cast<QHeaderView*>(sender());
  if (vHeader) {
    // revert movement, but avoid recursion
    disconnect(vHeader, SIGNAL(sectionMoved(int,int,int)), 0, 0);
    vHeader->moveSection(toIndex, fromIndex);
    connect(vHeader, SIGNAL(sectionMoved(int,int,int)), this, SLOT(moveTableRow(int,int,int)));
  }
  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  int numTracks = static_cast<int>(trackDataVector.size());
  if (fromIndex < numTracks && toIndex < numTracks) {
    // swap elements but keep file durations and names
    ImportTrackData fromData(trackDataVector[fromIndex]);
    ImportTrackData toData(trackDataVector[toIndex]);
    trackDataVector[fromIndex].setFrameCollection(toData.getFrameCollection());
    trackDataVector[toIndex].setFrameCollection(fromData.getFrameCollection());
    trackDataVector[fromIndex].setImportDuration(toData.getImportDuration());
    trackDataVector[toIndex].setImportDuration(fromData.getImportDuration());
    m_trackDataModel->setTrackData(trackDataVector);
    // redisplay the table
    showPreview();
  }
}

/**
 * Called when the destination combo box value is changed.
 */
void ImportDialog::changeTagDestination()
{
  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  trackDataVector.readTags(getDestination());
  m_trackDataModel->setTrackData(trackDataVector);
  showPreview();
}

/**
 * Match import data with length.
 */
void ImportDialog::matchWithLength()
{
  bool diffCheckEnable;
  int maxDiff;
  getTimeDifferenceCheck(diffCheckEnable, maxDiff);
  if (TrackDataMatcher::matchWithLength(m_trackDataModel, diffCheckEnable, maxDiff))
    showPreview();
}

/**
 * Match import data with track number.
 */
void ImportDialog::matchWithTrack()
{
  if (TrackDataMatcher::matchWithTrack(m_trackDataModel))
    showPreview();
}

/**
 * Match import data with title.
 */
void ImportDialog::matchWithTitle()
{
  if (TrackDataMatcher::matchWithTitle(m_trackDataModel))
    showPreview();
}

/**
 * Display custom context menu for horizontal table header.
 *
 * @param pos position where context menu is drawn on screen
 */
void ImportDialog::showTableHeaderContextMenu(const QPoint& pos)
{
  if (QWidget* widget = qobject_cast<QWidget*>(sender())) {
    QMenu menu(widget);
    foreach (int frameType, checkableFrameTypes()) {
      int column = m_trackDataModel->columnForFrameType(frameType);
      if (column != -1) {
        QAction* action = new QAction(&menu);
        action->setText(
              m_trackDataModel->headerData(column, Qt::Horizontal).toString());
        action->setData(frameType);
        action->setCheckable(true);
        action->setChecked((m_columnVisibility & (1ULL << frameType)) != 0ULL);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(toggleTableColumnVisibility(bool)));
        menu.addAction(action);
      }
    }
    menu.setMouseTracking(true);
    menu.exec(widget->mapToGlobal(pos));
  }
}

/**
 * Toggle visibility of table column.
 *
 * @param visible true to make column visible
 */
void ImportDialog::toggleTableColumnVisibility(bool visible)
{
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    bool ok;
    int frameType = action->data().toInt(&ok);
    if (ok && frameType < 64) {
      if (visible) {
        m_columnVisibility |= 1ULL << frameType;
      } else {
        m_columnVisibility &= ~(1ULL << frameType);
      }
      int column = m_trackDataModel->columnForFrameType(frameType);
      if (column != -1) {
        m_trackDataTable->setColumnHidden(column, !visible);
      }
    }
    if (visible) {
      m_trackDataTable->resizeColumnsToContents();
    }
  }
}
