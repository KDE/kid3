/**
 * \file importdialog.cpp
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2024  Urs Fleisch
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
#include "importconfig.h"
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
#include "frameitemdelegate.h"
#include "trackdatamatcher.h"
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
 * @param genreModel    genre model
 * @param trackDataModel track data to be filled with imported values,
 *                      is passed with durations of files set
 * @param importers     server importers
 * @param trackImporters server track importers
 */
ImportDialog::ImportDialog(IPlatformTools* platformTools,
                           QWidget* parent, const QString& caption,
                           TrackDataModel* trackDataModel,
                           GenreModel* genreModel,
                           const QList<ServerImporter*>& importers,
                           const QList<ServerTrackImporter*>& trackImporters)
  : QDialog(parent), m_platformTools(platformTools),
    m_autoStartSubDialog(-1), m_columnVisibility(0ULL),
    m_trackDataModel(trackDataModel), m_importers(importers),
    m_trackImporters(trackImporters)
{
  setObjectName(QLatin1String("ImportDialog"));
  setModal(false);
  setWindowTitle(caption);
  setSizeGripEnabled(true);

  auto vlayout = new QVBoxLayout(this);

  m_trackDataTable = new QTableView(this);
  m_trackDataTable->setModel(m_trackDataModel);
  m_trackDataTable->resizeColumnsToContents();
  m_trackDataTable->setItemDelegateForColumn(
        m_trackDataModel->columnForFrameType(Frame::FT_Genre),
        new FrameItemDelegate(genreModel, this));
  m_trackDataTable->verticalHeader()->setSectionsMovable(true);
  m_trackDataTable->horizontalHeader()->setSectionsMovable(true);
  m_trackDataTable->horizontalHeader()->setContextMenuPolicy(
        Qt::CustomContextMenu);
  connect(m_trackDataTable->verticalHeader(), &QHeaderView::sectionMoved,
          this, &ImportDialog::moveTableRow);
  connect(m_trackDataTable->horizontalHeader(),
          &QWidget::customContextMenuRequested,
      this, &ImportDialog::showTableHeaderContextMenu);
  vlayout->addWidget(m_trackDataTable);

  auto accuracyLayout = new QHBoxLayout;
  auto accuracyLabel = new QLabel(tr("Accuracy:"));
  accuracyLayout->addWidget(accuracyLabel);
  m_accuracyPercentLabel = new QLabel(QLatin1String("-"));
#if QT_VERSION >= 0x050b00
  m_accuracyPercentLabel->setMinimumWidth(
        m_accuracyPercentLabel->fontMetrics().horizontalAdvance(QLatin1String("100%")));
#else
  m_accuracyPercentLabel->setMinimumWidth(
        m_accuracyPercentLabel->fontMetrics().width(QLatin1String("100%")));
#endif
  accuracyLayout->addWidget(m_accuracyPercentLabel);
  auto coverArtLabel = new QLabel(tr("Cover Art:"));
  accuracyLayout->addWidget(coverArtLabel);
  m_coverArtUrlLabel = new QLabel(QLatin1String(" -"));
  m_coverArtUrlLabel->setSizePolicy(QSizePolicy::Ignored,
                                    QSizePolicy::Preferred);
  accuracyLayout->addWidget(m_coverArtUrlLabel, 1);
  vlayout->addLayout(accuracyLayout);

  auto butlayout = new QHBoxLayout;
  auto fileButton = new QPushButton(tr("From F&ile/Clipboard..."));
  fileButton->setAutoDefault(false);
  butlayout->addWidget(fileButton);
  auto tagsButton = new QPushButton(tr("From T&ags..."));
  tagsButton->setAutoDefault(false);
  butlayout->addWidget(tagsButton);
  auto serverButton = new QPushButton(tr("&From Server..."));
  serverButton->setAutoDefault(false);
  butlayout->addWidget(serverButton);
  m_serverComboBox = new QComboBox;
  m_serverComboBox->setEditable(false);
  const auto sis = m_importers;
  for (const ServerImporter* si : sis) {
    m_serverComboBox->addItem(QCoreApplication::translate("@default", si->name()));
  }
  const auto stis = m_trackImporters;
  for (const ServerTrackImporter* si : stis) {
    m_serverComboBox->addItem(QCoreApplication::translate("@default", si->name()));
  }
  butlayout->addWidget(m_serverComboBox);
  if (m_serverComboBox->count() == 0) {
    serverButton->hide();
    m_serverComboBox->hide();
  }
  auto butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  butlayout->addItem(butspacer);
  auto destLabel = new QLabel;
  destLabel->setText(tr("D&estination:"));
  butlayout->addWidget(destLabel);
  m_destComboBox = new QComboBox;
  m_destComboBox->setEditable(false);
  const QList<QPair<Frame::TagVersion, QString> > tagVersions =
      Frame::availableTagVersions();
  for (auto it = tagVersions.constBegin(); it != tagVersions.constEnd(); ++it) {
    m_destComboBox->addItem(it->second, it->first);
  }
  destLabel->setBuddy(m_destComboBox);
  butlayout->addWidget(m_destComboBox);
  auto revertButton = new QToolButton;
  revertButton->setIcon(
        m_platformTools->iconFromTheme(QLatin1String("document-revert")));
  revertButton->setToolTip(tr("Revert"));
  revertButton->setShortcut(QKeySequence::Undo);
  connect(revertButton, &QAbstractButton::clicked,
          this, &ImportDialog::changeTagDestination);
  butlayout->addWidget(revertButton);
  vlayout->addLayout(butlayout);

  auto matchLayout = new QHBoxLayout;
  m_mismatchCheckBox = new QCheckBox(
    tr("Check maximum allowable time &difference (sec):"));
  matchLayout->addWidget(m_mismatchCheckBox);
  m_maxDiffSpinBox = new QSpinBox;
  m_maxDiffSpinBox->setMaximum(9999);
  matchLayout->addWidget(m_maxDiffSpinBox);
  auto matchSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                             QSizePolicy::Minimum);
  matchLayout->addItem(matchSpacer);
  auto matchLabel = new QLabel(tr("Match with:"));
  matchLayout->addWidget(matchLabel);
  auto lengthButton = new QPushButton(tr("&Length"));
  lengthButton->setAutoDefault(false);
  matchLayout->addWidget(lengthButton);
  auto trackButton = new QPushButton(tr("T&rack"));
  trackButton->setAutoDefault(false);
  matchLayout->addWidget(trackButton);
  auto titleButton = new QPushButton(tr("&Title"));
  titleButton->setAutoDefault(false);
  matchLayout->addWidget(titleButton);
  vlayout->addLayout(matchLayout);

  connect(fileButton, &QAbstractButton::clicked,
          this, &ImportDialog::fromText);
  connect(tagsButton, &QAbstractButton::clicked,
          this, &ImportDialog::fromTags);
  connect(serverButton, &QAbstractButton::clicked,
          this, &ImportDialog::fromServer);
  connect(m_serverComboBox, static_cast<void (QComboBox::*)(int)>(
            &QComboBox::activated), this, &ImportDialog::fromServer);
  connect(lengthButton, &QAbstractButton::clicked,
          this, &ImportDialog::matchWithLength);
  connect(trackButton, &QAbstractButton::clicked,
          this, &ImportDialog::matchWithTrack);
  connect(titleButton, &QAbstractButton::clicked,
          this, &ImportDialog::matchWithTitle);
  connect(m_mismatchCheckBox, &QAbstractButton::toggled,
          this, &ImportDialog::showPreview);
  connect(m_maxDiffSpinBox, static_cast<void (QSpinBox::*)(int)>(
            &QSpinBox::valueChanged), this, &ImportDialog::maxDiffChanged);
  connect(this, &QDialog::finished, this, &ImportDialog::hideSubdialogs);

  auto hlayout = new QHBoxLayout;
  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  auto helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  auto saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  auto okButton = new QPushButton(tr("&OK"), this);
  auto cancelButton = new QPushButton(tr("&Cancel"), this);
  cancelButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  hlayout->addWidget(saveButton);
  hlayout->addItem(hspacer);
  hlayout->addWidget(okButton);
  hlayout->addWidget(cancelButton);
  connect(helpButton, &QAbstractButton::clicked,
          this, &ImportDialog::showHelp);
  connect(saveButton, &QAbstractButton::clicked,
          this, &ImportDialog::saveConfig);
  connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
  connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
  vlayout->addLayout(hlayout);

  auto deleteAction = new QAction(this);
  deleteAction->setShortcut(QKeySequence::Delete);
  connect(deleteAction, &QAction::triggered,
          this, &ImportDialog::deleteSelectedTableRows);
  addAction(deleteAction);
}

/**
 * Destructor.
 */
ImportDialog::~ImportDialog()
{
  // Must not be inline because of forward declared QScopedPointer.
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
    m_textImportDialog.reset(new TextImportDialog(
          m_platformTools, this, m_trackDataModel));
    connect(m_textImportDialog.data(), &TextImportDialog::trackDataUpdated,
            this, &ImportDialog::showPreview);
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
    m_tagImportDialog.reset(new TagImportDialog(this, m_trackDataModel));
    connect(m_tagImportDialog.data(), &TagImportDialog::trackDataUpdated,
            this, &ImportDialog::showPreview);
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
    m_serverImportDialog.reset(new ServerImportDialog(this));
    connect(m_serverImportDialog.data(), &ServerImportDialog::trackDataUpdated,
            this, &ImportDialog::showPreview);
    connect(m_serverImportDialog.data(), &QDialog::accepted,
            this, &ImportDialog::onServerImportDialogClosed);
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
    m_serverTrackImportDialog.reset(new ServerTrackImportDialog(this, m_trackDataModel));
    connect(m_serverTrackImportDialog.data(), &ServerTrackImportDialog::trackDataUpdated,
            this, &ImportDialog::showPreview);
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
  const ImportConfig& importCfg = ImportConfig::instance();
  m_serverComboBox->setCurrentIndex(importCfg.importServer());
  Frame::TagVersion importDest = importCfg.importDest();
  int index = m_destComboBox->findData(importDest);
  m_destComboBox->setCurrentIndex(index);

  if (!m_trackDataModel->trackData().isTagSupported(
        Frame::tagNumberFromMask(importDest))) {
    index = m_destComboBox->findData(Frame::TagV2);
    m_destComboBox->setCurrentIndex(index);
    changeTagDestination();
  }

  m_mismatchCheckBox->setChecked(importCfg.enableTimeDifferenceCheck());
  m_maxDiffSpinBox->setValue(importCfg.maxTimeDifference());
  m_columnVisibility = importCfg.importVisibleColumns();

  const auto frameTypes = checkableFrameTypes();
  for (int frameType : frameTypes) {
    if (frameType < 64) {
      if (int column = m_trackDataModel->columnForFrameType(frameType);
          column != -1) {
        m_trackDataTable->setColumnHidden(
              column, (m_columnVisibility & (1ULL << frameType)) == 0ULL);
      }
    }
  }

  if (!importCfg.importWindowGeometry().isEmpty()) {
    restoreGeometry(importCfg.importWindowGeometry());
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
                                  ? QString::number(accuracy) + QLatin1Char('%')
                                  : QLatin1String("-"));
  QUrl coverArtUrl = m_trackDataModel->getTrackData().getCoverArtUrl();
  m_coverArtUrlLabel->setText(coverArtUrl.isEmpty() ? QLatin1String("-")
                                                    : coverArtUrl.toString());
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
Frame::TagVersion ImportDialog::getDestination() const
{
  return Frame::tagVersionCast(
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
  ImportConfig& importCfg = ImportConfig::instance();
  importCfg.setImportDest(Frame::tagVersionCast(
    m_destComboBox->itemData(m_destComboBox->currentIndex()).toInt()));

  importCfg.setImportServer(m_serverComboBox->currentIndex());
  bool enable;
  int maxDiff;
  getTimeDifferenceCheck(enable, maxDiff);
  importCfg.setEnableTimeDifferenceCheck(enable);
  importCfg.setMaxTimeDifference(maxDiff);
  importCfg.setImportVisibleColumns(m_columnVisibility);

  importCfg.setImportWindowGeometry(saveGeometry());
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
 * @param toIndex   index of position moved to
 */
void ImportDialog::moveTableRow(int, int fromIndex, int toIndex) {
  if (auto vHeader = qobject_cast<QHeaderView*>(sender())) {
    // revert movement, but avoid recursion
    disconnect(vHeader, &QHeaderView::sectionMoved, nullptr, nullptr);
    vHeader->moveSection(toIndex, fromIndex);
    connect(vHeader, &QHeaderView::sectionMoved,
            this, &ImportDialog::moveTableRow);
  }

  // Allow dragging multiple rows when pressing Ctrl by adding selected rows.
  ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
  auto numTracks = trackDataVector.size();

  int diff = toIndex - fromIndex;
  QList<int> fromList;
  if (fromIndex >= 0 && fromIndex < numTracks &&
      toIndex >= 0 && toIndex < numTracks) {
    fromList.append(fromIndex);
  }
  const QModelIndexList selectedRows =
      m_trackDataTable->selectionModel()->selectedRows();
  for (const QModelIndex& index : selectedRows) {
    int from = index.row();
    int to = from + diff;
    if (!fromList.contains(from) &&
        from >= 0 && from < numTracks &&
        to >= 0 && to < numTracks) {
      fromList.append(from);
    }
  }
  std::sort(fromList.begin(), fromList.end());

  for (auto it = fromList.constBegin(); it != fromList.constEnd(); ++it) {
    fromIndex = *it;
    toIndex = fromIndex + diff;
    // swap elements but keep file durations and names
    ImportTrackData fromData(trackDataVector[fromIndex]);
    ImportTrackData toData(trackDataVector[toIndex]);
    trackDataVector[fromIndex].setFrameCollection(toData.getFrameCollection());
    trackDataVector[toIndex].setFrameCollection(fromData.getFrameCollection());
    trackDataVector[fromIndex].setImportDuration(toData.getImportDuration());
    trackDataVector[toIndex].setImportDuration(fromData.getImportDuration());
  }
  if (!fromList.isEmpty()) {
    m_trackDataModel->setTrackData(trackDataVector);
    // redisplay the table
    showPreview();
  }
}

/**
 * Delete the selected table rows.
 */
void ImportDialog::deleteSelectedTableRows()
{
  QSet<int> rows;
  const QModelIndexList selectedRows =
      m_trackDataTable->selectionModel()->selectedRows();
  for (const QModelIndex& index : selectedRows) {
    rows.insert(index.row());
  }
  if (rows.isEmpty()) {
    if (auto index = m_trackDataTable->currentIndex(); index.isValid()) {
      rows.insert(index.row());
    }
  }
  if (!rows.isEmpty()) {
    ImportTrackDataVector trackDataVector(m_trackDataModel->getTrackData());
    int numTracks = trackDataVector.size();
    int fromIndex = 0;
    for (auto toIt = trackDataVector.begin(), fromIt = trackDataVector.begin();
         toIt != trackDataVector.end();) {
      if (fromIndex >= numTracks) {
        trackDataVector.erase(toIt, trackDataVector.end());
        break;
      }
      if (!rows.contains(fromIndex)) {
        if (toIt != fromIt) {
          toIt->setFrameCollection(fromIt->getFrameCollection());
          toIt->setImportDuration(fromIt->getImportDuration());
        }
        ++toIt;
      }
      ++fromIt;
      ++fromIndex;
    }
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
    const auto frameTypes = checkableFrameTypes();
    for (int frameType : frameTypes) {
      if (int column = m_trackDataModel->columnForFrameType(frameType);
          column != -1) {
        auto action = new QAction(&menu);
        action->setText(
              m_trackDataModel->headerData(column, Qt::Horizontal).toString());
        action->setData(frameType);
        action->setCheckable(true);
        action->setChecked((m_columnVisibility & (1ULL << frameType)) != 0ULL);
        connect(action, &QAction::triggered,
                this, &ImportDialog::toggleTableColumnVisibility);
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
  if (auto action = qobject_cast<QAction*>(sender())) {
    bool ok;
    if (int frameType = action->data().toInt(&ok); ok && frameType < 64) {
      if (visible) {
        m_columnVisibility |= 1ULL << frameType;
      } else {
        m_columnVisibility &= ~(1ULL << frameType);
      }
      if (int column = m_trackDataModel->columnForFrameType(frameType);
          column != -1) {
        m_trackDataTable->setColumnHidden(column, !visible);
      }
    }
    if (visible) {
      m_trackDataTable->resizeColumnsToContents();
    }
  }
}
