/**
 * \file timeeventeditor.cpp
 * Editor for time events (synchronized lyrics and event timing codes).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Mar 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "timeeventeditor.h"
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QItemDelegate>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "config.h"
#include "fileconfig.h"
#include "timeeventmodel.h"
#include "timestampdelegate.h"
#include "eventcodedelegate.h"
#include "kid3application.h"
#include "audioplayer.h"
#include "contexthelp.h"
#include "iplatformtools.h"

/** Table to edit time events. */
class TimeEventTableView : public QTableView {
public:
  /** Constructor. */
  TimeEventTableView(QWidget* parent = nullptr) : QTableView(parent) {}
  /** Destructor. */
  virtual ~TimeEventTableView() override = default;

protected:
  /**
   * Handle key events, delete cell contents if Delete key is pressed.
   * @param event key event
   */
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  Q_DISABLE_COPY(TimeEventTableView)
};

void TimeEventTableView::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Delete) {
    QModelIndex idx = currentIndex();
    QAbstractItemModel* mdl = model();
    if (mdl && idx.isValid()) {
      mdl->setData(idx, QVariant(idx.data().type()));
      return;
    }
  }
  QTableView::keyPressEvent(event);
}


/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param app application context
 * @param parent parent widget
 * @param field  field containing binary data
 * @param taggedFile tagged file
 * @param tagNr tag number
 */
TimeEventEditor::TimeEventEditor(IPlatformTools* platformTools,
                                 Kid3Application* app,
                                 QWidget* parent, const Frame::Field& field,
                                 const TaggedFile* taggedFile,
                                 Frame::TagNumber tagNr) :
  QWidget(parent),
  m_platformTools(platformTools), m_app(app), m_eventCodeDelegate(nullptr),
  m_model(nullptr), m_taggedFile(taggedFile), m_tagNr(tagNr),
  m_byteArray(field.m_value.toByteArray()), m_fileIsPlayed(false)
{
  setObjectName(QLatin1String("TimeEventEditor"));
  auto vlayout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  vlayout->addWidget(m_label);
  vlayout->setContentsMargins(0, 0, 0, 0);
  auto buttonLayout = new QHBoxLayout;
  QPushButton* addButton = new QPushButton(tr("&Add"), this);
  addButton->setAutoDefault(false);
  QPushButton* deleteButton = new QPushButton(tr("&Delete"), this);
  deleteButton->setAutoDefault(false);
  QPushButton* clipButton = new QPushButton(tr("From Clip&board"), this);
  clipButton->setAutoDefault(false);
  QPushButton* importButton = new QPushButton(tr("&Import..."), this);
  importButton->setAutoDefault(false);
  QPushButton* exportButton = new QPushButton(tr("&Export..."), this);
  exportButton->setAutoDefault(false);
  QPushButton* helpButton = new QPushButton(tr("Help"), this);
  helpButton->setAutoDefault(false);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addWidget(addButton);
  buttonLayout->addWidget(deleteButton);
  buttonLayout->addWidget(clipButton);
  buttonLayout->addWidget(importButton);
  buttonLayout->addWidget(exportButton);
  buttonLayout->addWidget(helpButton);
  buttonLayout->addStretch();
  connect(addButton, &QAbstractButton::clicked, this, &TimeEventEditor::addItem);
  connect(deleteButton, &QAbstractButton::clicked, this, &TimeEventEditor::deleteRows);
  connect(clipButton, &QAbstractButton::clicked, this, &TimeEventEditor::clipData);
  connect(importButton, &QAbstractButton::clicked, this, &TimeEventEditor::importData);
  connect(exportButton, &QAbstractButton::clicked, this, &TimeEventEditor::exportData);
  connect(helpButton, &QAbstractButton::clicked, this, &TimeEventEditor::showHelp);
  vlayout->addLayout(buttonLayout);
  m_tableView = new TimeEventTableView;
  m_tableView->verticalHeader()->hide();
  m_tableView->horizontalHeader()->setStretchLastSection(true);
  m_tableView->setItemDelegateForColumn(0, new TimeStampDelegate(this));
  m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tableView, &QWidget::customContextMenuRequested,
      this, &TimeEventEditor::customContextMenu);
  vlayout->addWidget(m_tableView);
}

/**
 * Connect to player when editor is shown.
 * @param event event
 */
void TimeEventEditor::showEvent(QShowEvent* event)
{
  QTimer::singleShot(0, this, &TimeEventEditor::preparePlayer);
  QWidget::showEvent(event);
}

/**
 * Disconnect from player when editor is hidden.
 * @param event event
 */
void TimeEventEditor::hideEvent(QHideEvent* event)
{
  AudioPlayer* player = m_app->getAudioPlayer();
  disconnect(player, nullptr, this, nullptr);
  m_fileIsPlayed = false;
  QWidget::hideEvent(event);
}

/**
 * Set time event model.
 * @param model time event model
 */
void TimeEventEditor::setModel(TimeEventModel* model)
{
  m_model = model;
  if (m_model->getType() == TimeEventModel::EventTimingCodes) {
    m_label->setText(tr("Events"));
    if (!m_eventCodeDelegate) {
      m_eventCodeDelegate = new EventCodeDelegate(this);
    }
    m_tableView->setItemDelegateForColumn(1, m_eventCodeDelegate);
  } else {
    m_label->setText(tr("Lyrics"));
    m_tableView->setItemDelegateForColumn(1, nullptr);
  }
  m_tableView->setModel(m_model);
}

/**
 * Make sure that player is visible and in the edited file.
 */
void TimeEventEditor::preparePlayer()
{
  m_app->showAudioPlayer();
  AudioPlayer* player = m_app->getAudioPlayer();
  QString filePath = m_taggedFile->getAbsFilename();
  if (player->getFileName() != filePath) {
    player->setFiles({filePath}, -1);
  }
  m_fileIsPlayed = true;
  connect(player, &AudioPlayer::trackChanged,
          this, &TimeEventEditor::onTrackChanged, Qt::UniqueConnection);
  connect(player, &AudioPlayer::positionChanged,
          this, &TimeEventEditor::onPositionChanged, Qt::UniqueConnection);
}

/**
 * Add a time event at the current player position.
 */
void TimeEventEditor::addItem()
{
  QTime timeStamp;
  preparePlayer();
  AudioPlayer* player = m_app->getAudioPlayer();
  timeStamp = QTime(0, 0).addMSecs(player->getCurrentPosition());
  if (m_model) {
    // If the current row is empty, set the time stamp there, else insert a new
    // row sorted by time stamps or use the first empty row.
    QModelIndex index = m_tableView->currentIndex();
    if (!(index.isValid() &&
          (index = index.sibling(index.row(), TimeEventModel::CI_Time)).
          data().isNull())) {
      int row = 0;
      bool insertRow = true;
      while (row < m_model->rowCount()) {
        QTime time = m_model->index(row, TimeEventModel::CI_Time).
            data().toTime();
        if (time.isNull()) {
          insertRow = false;
          break;
        } else if (time > timeStamp) {
          break;
        }
        ++row;
      }
      if (insertRow) {
        m_model->insertRow(row);
      }
      index = m_model->index(row, TimeEventModel::CI_Time);
    }
    m_model->setData(index, timeStamp);
    m_tableView->scrollTo(index);
  }
}

/**
 * Load LRC data from clipboard.
 */
void TimeEventEditor::clipData()
{
  QClipboard* cb = QApplication::clipboard();
  if (cb && cb->mimeData()->hasText()) {
    QString text = cb->text();
    QTextStream stream(&text, QIODevice::ReadOnly);
    m_model->fromLrcFile(stream);
  }
}

/**
 * Import data in LRC format.
 */
void TimeEventEditor::importData()
{
  if (!m_model)
    return;

  QString loadFileName = m_platformTools->getOpenFileName(this, QString(),
        m_taggedFile->getDirname(), getLrcNameFilter(), nullptr);
  if (!loadFileName.isEmpty()) {
    QFile file(loadFileName);
    if (file.open(QIODevice::ReadOnly)) {
      QTextStream stream(&file);
      m_model->fromLrcFile(stream);
      file.close();
    }
  }
}

/**
 * Export data in LRC format.
 */
void TimeEventEditor::exportData()
{
  if (!m_model)
    return;

  QString suggestedFileName = m_taggedFile->getAbsFilename();
  int dotPos = suggestedFileName.lastIndexOf(QLatin1Char('.'));
  if (dotPos >= 0 && dotPos >= suggestedFileName.length() - 5) {
    suggestedFileName.truncate(dotPos);
  }
  suggestedFileName += QLatin1String(".lrc");
  QString saveFileName = m_platformTools->getSaveFileName(
        this, QString(), suggestedFileName, getLrcNameFilter(), nullptr);
  if (!saveFileName.isEmpty()) {
    QFile file(saveFileName);
    if (file.open(QIODevice::WriteOnly)) {
      QTextStream stream(&file);
      QString codecName = FileConfig::instance().textEncoding();
      if (codecName != QLatin1String("System")) {
        stream.setCodec(codecName.toLatin1());
      }
      QString title, artist, album;
      Frame frame;
      if (m_taggedFile->getFrame(m_tagNr, Frame::FT_Title, frame)) {
        title = frame.getValue();
      }
      if (m_taggedFile->getFrame(m_tagNr, Frame::FT_Artist, frame)) {
        artist = frame.getValue();
      }
      if (m_taggedFile->getFrame(m_tagNr, Frame::FT_Album, frame)) {
        album = frame.getValue();
      }
      m_model->toLrcFile(stream, title, artist, album);
      file.close();
    }
  }
}

/**
 * Get file name filter string for LRC files.
 * @return filter string.
 */
QString TimeEventEditor::getLrcNameFilter() const
{
  const char* const lyricsStr = QT_TRANSLATE_NOOP("@default", "Lyrics");
  const char* const allFilesStr = QT_TRANSLATE_NOOP("@default", "All Files");
  return m_platformTools->fileDialogNameFilter({
        qMakePair(QCoreApplication::translate("@default", lyricsStr),
                  QString(QLatin1String("*.lrc"))),
        qMakePair(QCoreApplication::translate("@default", allFilesStr),
                  QString(QLatin1Char('*')))
  });
}

/**
 * Insert a new row after the current row.
 */
void TimeEventEditor::insertRow()
{
  if (!m_model)
    return;

  m_model->insertRow(m_tableView->currentIndex().isValid()
                     ? m_tableView->currentIndex().row() + 1 : 0);
}

/**
 * Delete the selected rows.
 */
void TimeEventEditor::deleteRows()
{
  if (!m_model)
    return;

  QMap<int, int> rows;
  if (QItemSelectionModel* selModel = m_tableView->selectionModel()) {
    const auto indexes = selModel->selectedIndexes();
    for (const QModelIndex& index : indexes) {
      rows.insert(index.row(), 0);
    }
  }

  QMapIterator<int, int> it(rows);
  it.toBack();
  while (it.hasPrevious()) {
    it.previous();
    m_model->removeRow(it.key());
  }
}

/**
 * Clear the selected cells.
 */
void TimeEventEditor::clearCells()
{
  if (!m_model)
    return;

  QVariant emptyData(m_model->getType() == TimeEventModel::EventTimingCodes
                     ? QVariant::Int : QVariant::String);
  QVariant emptyTime(QVariant::Time);
  if (QItemSelectionModel* selModel = m_tableView->selectionModel()) {
    const auto indexes = selModel->selectedIndexes();
    for (const QModelIndex& index : indexes) {
      m_model->setData(index, index.column() == TimeEventModel::CI_Time
                       ? emptyTime : emptyData);
    }
  }
}

/**
 * Add offset to time stamps.
 */
void TimeEventEditor::addOffset()
{
  if (!m_model)
    return;

  int offset = QInputDialog::getInt(this, tr("Offset"), tr("Milliseconds"));
  if (QItemSelectionModel* selModel = m_tableView->selectionModel()) {
    const auto indexes = selModel->selectedIndexes();
    for (const QModelIndex& index : indexes) {
      if (index.column() == TimeEventModel::CI_Time) {
        m_model->setData(index, index.data().toTime().addMSecs(offset));
      }
    }
  }
}

/**
 * Seek to position of current time stamp.
 */
void TimeEventEditor::seekPosition()
{
  QModelIndex index = m_tableView->currentIndex();
  if (index.isValid() && m_fileIsPlayed) {
    QTime timeStamp =
        index.sibling(index.row(), TimeEventModel::CI_Time).data().toTime();
    if (timeStamp.isValid()) {
      AudioPlayer* player = m_app->getAudioPlayer();
      player->setCurrentPosition(QTime(0, 0).msecsTo(timeStamp));
    }
  }
}

/**
 * Display custom context menu.
 *
 * @param pos position where context menu is drawn on screen
 */
void TimeEventEditor::customContextMenu(const QPoint& pos)
{
  QMenu menu(this);
  QAction* action = menu.addAction(tr("&Insert row"));
  connect(action, &QAction::triggered, this, &TimeEventEditor::insertRow);
  QModelIndex index = m_tableView->indexAt(pos);
  if (index.isValid()) {
    action = menu.addAction(tr("&Delete rows"));
    connect(action, &QAction::triggered, this, &TimeEventEditor::deleteRows);
    action = menu.addAction(tr("C&lear"));
    connect(action, &QAction::triggered, this, &TimeEventEditor::clearCells);
    action = menu.addAction(tr("&Add offset..."));
    connect(action, &QAction::triggered, this, &TimeEventEditor::addOffset);
    action = menu.addAction(tr("&Seek to position"));
    connect(action, &QAction::triggered, this, &TimeEventEditor::seekPosition);
  }
  menu.setMouseTracking(true);
  menu.exec(m_tableView->mapToGlobal(pos));
}

/**
 * Called when the played track changed.
 * @param filePath path to file being played
 */
void TimeEventEditor::onTrackChanged(const QString& filePath)
{
  m_fileIsPlayed = filePath == m_taggedFile->getAbsFilename();
  if (m_model) {
    m_model->clearMarkedRow();
  }
}

/**
 * Called when the player position changed.
 * @param position time in ms
 */
void TimeEventEditor::onPositionChanged(qint64 position)
{
  if (!m_fileIsPlayed || !m_model)
    return;

  int oldRow = m_model->getMarkedRow();
  m_model->markRowForTimeStamp(QTime(0, 0).addMSecs(position));
  int row = m_model->getMarkedRow();
  if (row != oldRow && row != -1) {
    m_tableView->scrollTo(m_model->index(row, TimeEventModel::CI_Time),
                          QAbstractItemView::PositionAtCenter);
  }
}

/**
 * Show help.
 */
void TimeEventEditor::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("synchronized-lyrics"));
}
