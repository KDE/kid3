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
#include "timeeventmodel.h"
#include "timestampdelegate.h"
#include "eventcodedelegate.h"
#include "kid3application.h"
#include "audioplayer.h"
#include "iplatformtools.h"

/** Table to edit time events. */
class TimeEventTableView : public QTableView {
public:
  /** Constructor. */
  TimeEventTableView(QWidget* parent = 0) : QTableView(parent) {}
  /** Destructor. */
  virtual ~TimeEventTableView() {}

protected:
  /**
   * Handle key events, delete cell contents if Delete key is pressed.
   * @param event key event
   */
  virtual void keyPressEvent(QKeyEvent* event);
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
 */
TimeEventEditor::TimeEventEditor(IPlatformTools* platformTools,
                                 Kid3Application* app,
                                 QWidget* parent, const Frame::Field& field,
                                 const TaggedFile* taggedFile) :
  QWidget(parent),
  m_platformTools(platformTools), m_app(app), m_eventCodeDelegate(0),
  m_model(0), m_taggedFile(taggedFile), m_byteArray(field.m_value.toByteArray()),
  m_fileIsPlayed(false)
{
  setObjectName(QLatin1String("TimeEventEditor"));
  QVBoxLayout* vlayout = new QVBoxLayout(this);
  m_label = new QLabel(this);
  vlayout->addWidget(m_label);
  vlayout->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* buttonLayout = new QHBoxLayout;
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
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addWidget(addButton);
  buttonLayout->addWidget(deleteButton);
  buttonLayout->addWidget(clipButton);
  buttonLayout->addWidget(importButton);
  buttonLayout->addWidget(exportButton);
  buttonLayout->addStretch();
  connect(addButton, SIGNAL(clicked()), this, SLOT(addItem()));
  connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteRows()));
  connect(clipButton, SIGNAL(clicked()), this, SLOT(clipData()));
  connect(importButton, SIGNAL(clicked()), this, SLOT(importData()));
  connect(exportButton, SIGNAL(clicked()), this, SLOT(exportData()));
  vlayout->addLayout(buttonLayout);
  m_tableView = new TimeEventTableView;
  m_tableView->verticalHeader()->hide();
  m_tableView->horizontalHeader()->setStretchLastSection(true);
  m_tableView->setItemDelegateForColumn(0, new TimeStampDelegate(this));
  m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tableView, SIGNAL(customContextMenuRequested(QPoint)),
      this, SLOT(customContextMenu(QPoint)));
  vlayout->addWidget(m_tableView);
}

/**
 * Destructor.
 */
TimeEventEditor::~TimeEventEditor()
{
}

/**
 * Connect to player when editor is shown.
 * @param event event
 */
void TimeEventEditor::showEvent(QShowEvent* event)
{
  QTimer::singleShot(0, this, SLOT(preparePlayer()));
  QWidget::showEvent(event);
}

/**
 * Disconnect from player when editor is hidden.
 * @param event event
 */
void TimeEventEditor::hideEvent(QHideEvent* event)
{
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  AudioPlayer* player = m_app->getAudioPlayer();
  disconnect(player, 0, this, 0);
  m_fileIsPlayed = false;
#endif
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
    m_tableView->setItemDelegateForColumn(1, 0);
  }
  m_tableView->setModel(m_model);
}

/**
 * Make sure that player is visible and in the edited file.
 */
void TimeEventEditor::preparePlayer()
{
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  m_app->showAudioPlayer();
  AudioPlayer* player = m_app->getAudioPlayer();
  QString filePath = m_taggedFile->getAbsFilename();
  if (player->getFileName() != filePath) {
    player->setFiles(QStringList() << filePath, -1);
  }
  m_fileIsPlayed = true;
  connect(player, SIGNAL(trackChanged(QString,bool,bool)),
          this, SLOT(onTrackChanged(QString)), Qt::UniqueConnection);
  connect(player, SIGNAL(positionChanged(qint64)),
          this, SLOT(onPositionChanged(qint64)), Qt::UniqueConnection);
#endif
}

/**
 * Add a time event at the current player position.
 */
void TimeEventEditor::addItem()
{
  QTime timeStamp;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  preparePlayer();
  AudioPlayer* player = m_app->getAudioPlayer();
  timeStamp = QTime(0, 0).addMSecs(player->getCurrentPosition());
#endif
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
        m_taggedFile->getDirname(), getLrcNameFilter(), 0);
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
        this, QString(), suggestedFileName, getLrcNameFilter(), 0);
  if (!saveFileName.isEmpty()) {
    QFile file(saveFileName);
    if (file.open(QIODevice::WriteOnly)) {
      QTextStream stream(&file);
      m_model->toLrcFile(stream, m_taggedFile->getTitleV2(),
                                 m_taggedFile->getArtistV2(),
                                 m_taggedFile->getAlbumV2());
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
  return m_platformTools->fileDialogNameFilter(
        QList<QPair<QString, QString> >()
        << qMakePair(QCoreApplication::translate("@default", lyricsStr),
                     QString(QLatin1String("*.lrc")))
        << qMakePair(QCoreApplication::translate("@default", allFilesStr),
                     QString(QLatin1Char('*'))));
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
    foreach (const QModelIndex& index, selModel->selectedIndexes()) {
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
    foreach (const QModelIndex& index, selModel->selectedIndexes()) {
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
    foreach (const QModelIndex& index, selModel->selectedIndexes()) {
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
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  QModelIndex index = m_tableView->currentIndex();
  if (index.isValid() && m_fileIsPlayed) {
    QTime timeStamp =
        index.sibling(index.row(), TimeEventModel::CI_Time).data().toTime();
    if (timeStamp.isValid()) {
      AudioPlayer* player = m_app->getAudioPlayer();
      player->setCurrentPosition(QTime(0, 0).msecsTo(timeStamp));
    }
  }
#endif
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
  connect(action, SIGNAL(triggered()), this, SLOT(insertRow()));
  QModelIndex index = m_tableView->indexAt(pos);
  if (index.isValid()) {
    action = menu.addAction(tr("&Delete rows"));
    connect(action, SIGNAL(triggered()), this, SLOT(deleteRows()));
    action = menu.addAction(tr("C&lear"));
    connect(action, SIGNAL(triggered()), this, SLOT(clearCells()));
    action = menu.addAction(tr("&Add offset..."));
    connect(action, SIGNAL(triggered()), this, SLOT(addOffset()));
    action = menu.addAction(tr("&Seek to position"));
    connect(action, SIGNAL(triggered()), this, SLOT(seekPosition()));
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
