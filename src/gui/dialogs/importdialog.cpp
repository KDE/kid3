/**
 * \file importdialog.cpp
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kicon.h>
#endif
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
#include "genres.h"
#include "serverimporter.h"
#include "serverimportdialog.h"
#include "textimportdialog.h"
#include "tagimportdialog.h"
#include "configstore.h"
#include "contexthelp.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "trackdatamodel.h"
#include "frametablemodel.h"
#include "trackdatamatcher.h"
#include "qtcompatmac.h"
#include "config.h"
#ifdef HAVE_CHROMAPRINT
#include "musicbrainzdialog.h"
#include "musicbrainzconfig.h"
#endif

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
 * @param parent        parent widget
 * @param caption       dialog title
 * @param trackDataModel track data to be filled with imported values,
 *                      is passed with durations of files set
 * @param importers     server importers
 */
ImportDialog::ImportDialog(QWidget* parent, QString& caption,
                           TrackDataModel* trackDataModel,
                           const QList<ServerImporter*>& importers) :
  QDialog(parent),
  m_autoStartSubDialog(-1), m_columnVisibility(0ULL),
  m_trackDataModel(trackDataModel), m_importers(importers)
{
  setObjectName("ImportDialog");
  setModal(true);
  setWindowTitle(caption);
  setSizeGripEnabled(true);

  m_serverImportDialog = 0;
  m_textImportDialog = 0;
  m_tagImportDialog = 0;
#ifdef HAVE_CHROMAPRINT
  m_musicBrainzDialog = 0;
#endif

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  vlayout->setSpacing(6);
  vlayout->setMargin(6);

  m_trackDataTable = new QTableView(this);
  m_trackDataTable->setModel(m_trackDataModel);
  m_trackDataTable->resizeColumnsToContents();
  m_trackDataTable->setItemDelegateForColumn(
        m_trackDataModel->columnForFrameType(Frame::FT_Genre),
        new FrameItemDelegate(this));
  m_trackDataTable->verticalHeader()->setMovable(true);
  m_trackDataTable->horizontalHeader()->setMovable(true);
  m_trackDataTable->horizontalHeader()->setContextMenuPolicy(
        Qt::CustomContextMenu);
  connect(m_trackDataTable->verticalHeader(), SIGNAL(sectionMoved(int, int, int)),
          this, SLOT(moveTableRow(int, int, int)));
  connect(m_trackDataTable->horizontalHeader(),
          SIGNAL(customContextMenuRequested(QPoint)),
      this, SLOT(showTableHeaderContextMenu(QPoint)));
  vlayout->addWidget(m_trackDataTable);

  QWidget* butbox = new QWidget(this);
  QHBoxLayout* butlayout = new QHBoxLayout(butbox);
  butlayout->setMargin(0);
  butlayout->setSpacing(6);
  QPushButton* fileButton = new QPushButton(i18n("From F&ile/Clipboard..."),
                                            butbox);
  fileButton->setAutoDefault(false);
  butlayout->addWidget(fileButton);
  QPushButton* tagsButton = new QPushButton(i18n("From T&ags..."),
                                            butbox);
  tagsButton->setAutoDefault(false);
  butlayout->addWidget(tagsButton);
  QPushButton* serverButton = new QPushButton(i18n("&From Server:"), butbox);
  serverButton->setAutoDefault(false);
  butlayout->addWidget(serverButton);
  m_serverComboBox = new QComboBox(butbox);
  m_serverComboBox->setEditable(false);
  foreach (const ServerImporter* si, m_importers) {
    m_serverComboBox->addItem(QCM_translate(si->name()));
  }
#ifdef HAVE_CHROMAPRINT
  m_serverComboBox->addItem(i18n("MusicBrainz Fingerprint"));
#endif
  butlayout->addWidget(m_serverComboBox);
  QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  butlayout->addItem(butspacer);
  QLabel* destLabel = new QLabel(butbox);
  destLabel->setText(i18n("D&estination:"));
  butlayout->addWidget(destLabel);
  m_destComboBox = new QComboBox(butbox);
  m_destComboBox->setEditable(false);
  m_destComboBox->addItem(i18n("Tag 1"), TrackData::TagV1);
  m_destComboBox->addItem(i18n("Tag 2"), TrackData::TagV2);
  m_destComboBox->addItem(i18n("Tag 1 and Tag 2"), TrackData::TagV2V1);
  destLabel->setBuddy(m_destComboBox);
  butlayout->addWidget(m_destComboBox);
  QToolButton* revertButton = new QToolButton(butbox);
  revertButton->setIcon(
#ifdef CONFIG_USE_KDE
        KIcon("document-revert")
#else
        QIcon(":/images/document-revert.png")
#endif
        );
  revertButton->setToolTip(i18n("Revert"));
  connect(revertButton, SIGNAL(clicked()),
          this, SLOT(changeTagDestination()));
  butlayout->addWidget(revertButton);
  vlayout->addWidget(butbox);

  QWidget* matchBox = new QWidget(this);
  QHBoxLayout* matchLayout = new QHBoxLayout(matchBox);
  matchLayout->setMargin(0);
  matchLayout->setSpacing(6);
  m_mismatchCheckBox = new QCheckBox(
    i18n("Check maximum allowable time &difference (sec):"), matchBox);
  matchLayout->addWidget(m_mismatchCheckBox);
  m_maxDiffSpinBox = new QSpinBox(matchBox);
  m_maxDiffSpinBox->setMaximum(9999);
  matchLayout->addWidget(m_maxDiffSpinBox);
  QSpacerItem* matchSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                             QSizePolicy::Minimum);
  matchLayout->addItem(matchSpacer);
  QLabel* matchLabel = new QLabel(i18n("Match with:"), matchBox);
  matchLayout->addWidget(matchLabel);
  QPushButton* lengthButton = new QPushButton(i18n("&Length"), matchBox);
  lengthButton->setAutoDefault(false);
  matchLayout->addWidget(lengthButton);
  QPushButton* trackButton = new QPushButton(i18n("T&rack"), matchBox);
  trackButton->setAutoDefault(false);
  matchLayout->addWidget(trackButton);
  QPushButton* titleButton = new QPushButton(i18n("&Title"), matchBox);
  titleButton->setAutoDefault(false);
  matchLayout->addWidget(titleButton);
  vlayout->addWidget(matchBox);

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
  QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
  helpButton->setAutoDefault(false);
  QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  QPushButton* okButton = new QPushButton(i18n("&OK"), this);
  okButton->setAutoDefault(false);
  QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
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
#ifdef HAVE_CHROMAPRINT
  delete m_musicBrainzDialog;
#endif
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
    m_textImportDialog = new TextImportDialog(this, m_trackDataModel);
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
    } else {
      // special case for MusicBrainz Fingerprint
#ifdef HAVE_CHROMAPRINT
      fromMusicBrainz();
#endif
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
  }
  if (m_serverImportDialog) {
    m_serverImportDialog->setImportSource(source);
    m_serverImportDialog->setArtistAlbum(
          m_trackDataModel->trackData().getArtist(),
          m_trackDataModel->trackData().getAlbum());
    m_serverImportDialog->show();
  }
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

#ifdef HAVE_CHROMAPRINT
/**
 * Import from MusicBrainz and preview in table.
 */
void ImportDialog::fromMusicBrainz()
{
  if (!m_musicBrainzDialog) {
    m_musicBrainzDialog = new MusicBrainzDialog(this, m_trackDataModel);
    connect(m_musicBrainzDialog, SIGNAL(trackDataUpdated()),
            this, SLOT(showPreview()));
  }
  if (m_musicBrainzDialog) {
    m_musicBrainzDialog->initTable();
    (void)m_musicBrainzDialog->exec();
  }
}
#endif

/**
 * Shows the dialog as a modal dialog.
 */
int ImportDialog::exec()
{
  if (m_autoStartSubDialog >= 0) {
    show();
    displayServerImportDialog(m_autoStartSubDialog);
  }
  return QDialog::exec();
}

/**
 * Set dialog to be started automatically.
 *
 * @param importerIndex index of importer to use
 */
void ImportDialog::setAutoStartSubDialog(int importerIndex)
{
  m_autoStartSubDialog = importerIndex;

  if (importerIndex >= 0 && importerIndex < m_serverComboBox->count()) {
    m_serverComboBox->setCurrentIndex(importerIndex);
  }
}

/**
 * Clear dialog data.
 */
void ImportDialog::clear()
{
  m_serverComboBox->setCurrentIndex(ConfigStore::s_genCfg.m_importServer);
  TrackData::TagVersion importDest = ConfigStore::s_genCfg.m_importDest;
  int index = m_destComboBox->findData(importDest);
  m_destComboBox->setCurrentIndex(index);
  if (importDest == TrackData::TagV1 &&
      !m_trackDataModel->trackData().isTagV1Supported()) {
    index = m_destComboBox->findData(TrackData::TagV2);
    m_destComboBox->setCurrentIndex(index);
    changeTagDestination();
  }

  m_mismatchCheckBox->setChecked(ConfigStore::s_genCfg.m_enableTimeDifferenceCheck);
  m_maxDiffSpinBox->setValue(ConfigStore::s_genCfg.m_maxTimeDifference);
  m_columnVisibility = ConfigStore::s_genCfg.m_importVisibleColumns;

  foreach (int frameType, checkableFrameTypes()) {
    if (frameType < 64) {
      int column = m_trackDataModel->columnForFrameType(frameType);
      if (column != -1) {
        m_trackDataTable->setColumnHidden(
              column, (m_columnVisibility & (1ULL << frameType)) == 0ULL);
      }
    }
  }

  if (ConfigStore::s_genCfg.m_importWindowWidth > 0 &&
      ConfigStore::s_genCfg.m_importWindowHeight > 0) {
    resize(ConfigStore::s_genCfg.m_importWindowWidth,
           ConfigStore::s_genCfg.m_importWindowHeight);
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
  ContextHelp::displayHelp("import");
}

/**
 * Save the local settings to the configuration.
 */
void ImportDialog::saveConfig()
{
  ConfigStore::s_genCfg.m_importDest = TrackData::tagVersionCast(
    m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt());

  ConfigStore::s_genCfg.m_importServer = m_serverComboBox->currentIndex();
  getTimeDifferenceCheck(ConfigStore::s_genCfg.m_enableTimeDifferenceCheck,
                         ConfigStore::s_genCfg.m_maxTimeDifference);
  ConfigStore::s_genCfg.m_importVisibleColumns = m_columnVisibility;

  ConfigStore::s_genCfg.m_importWindowWidth = size().width();
  ConfigStore::s_genCfg.m_importWindowHeight = size().height();
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
    disconnect(vHeader, SIGNAL(sectionMoved(int, int, int)), 0, 0);
    vHeader->moveSection(toIndex, fromIndex);
    connect(vHeader, SIGNAL(sectionMoved(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
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