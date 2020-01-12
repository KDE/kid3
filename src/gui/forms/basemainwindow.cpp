/**
 * \file basemainwindow.cpp
 * Base class for main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#include "basemainwindow.h"
#include <QTimer>
#include <QDir>
#include <QCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <QToolButton>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QIcon>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>
#include "kid3form.h"
#include "kid3application.h"
#include "framelist.h"
#include "frametablemodel.h"
#include "frametable.h"
#include "importdialog.h"
#include "tagimportdialog.h"
#include "batchimportdialog.h"
#include "browsecoverartdialog.h"
#include "exportdialog.h"
#include "findreplacedialog.h"
#include "tagsearcher.h"
#include "numbertracksdialog.h"
#include "filterdialog.h"
#include "rendirdialog.h"
#include "downloadclient.h"
#include "downloaddialog.h"
#include "playlistdialog.h"
#include "playlisteditdialog.h"
#include "editframefieldsdialog.h"
#include "progresswidget.h"
#include "fileproxymodel.h"
#include "fileproxymodeliterator.h"
#include "modeliterator.h"
#include "taggedfileselection.h"
#include "filelist.h"
#include "pictureframe.h"
#include "fileconfig.h"
#include "playlistconfig.h"
#include "playlistmodel.h"
#include "importconfig.h"
#include "exportconfig.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "filterconfig.h"
#include "isettings.h"
#include "contexthelp.h"
#include "frame.h"
#include "textexporter.h"
#include "serverimporter.h"
#include "batchimporter.h"
#include "dirrenamer.h"
#include "iplatformtools.h"
#include "saferename.h"
#include "audioplayer.h"
#include "playtoolbar.h"

/**
 * Constructor.
 *
 * @param mainWin main window widget
 * @param platformTools platform specific tools
 * @param app application context
 */
BaseMainWindowImpl::BaseMainWindowImpl(QMainWindow* mainWin,
                                       IPlatformTools* platformTools,
                                       Kid3Application* app)
  : m_platformTools(platformTools), m_w(mainWin), m_self(nullptr),
    m_deferredItemCountTimer(new QTimer(this)),
    m_deferredSelectionCountTimer(new QTimer(this)),
    m_statusLabel(nullptr), m_form(nullptr), m_app(app),
    m_exportDialog(nullptr), m_findReplaceDialog(nullptr),
    m_downloadDialog(new DownloadDialog(m_w, tr("Download"))),
    m_progressWidget(nullptr), m_progressLabel(nullptr),
    m_progressBar(nullptr), m_progressAbortButton(nullptr),
    m_editFrameDialog(nullptr), m_editFrameTaggedFile(nullptr),
    m_editFrameTagNr(Frame::Tag_2),
    m_progressTerminationHandler(nullptr),
    m_folderCount(0), m_fileCount(0), m_selectionCount(0),
    m_progressDisconnected(false),
    m_findReplaceActive(false), m_expandNotificationNeeded(false)
{
  m_deferredItemCountTimer->setSingleShot(true);
  m_deferredItemCountTimer->setInterval(1000);
  connect(m_deferredItemCountTimer, &QTimer::timeout,
          this, &BaseMainWindowImpl::onItemCountChanged);
  m_deferredSelectionCountTimer->setSingleShot(true);
  m_deferredSelectionCountTimer->setInterval(500);
  connect(m_deferredSelectionCountTimer, &QTimer::timeout,
          this, &BaseMainWindowImpl::onSelectionCountChanged);

  m_downloadDialog->close();
  ContextHelp::init(m_platformTools);

  DownloadClient* downloadClient = m_app->getDownloadClient();
  connect(downloadClient, &HttpClient::progress,
          m_downloadDialog, &DownloadDialog::updateProgressStatus);
  connect(downloadClient, &DownloadClient::downloadStarted,
          m_downloadDialog, &DownloadDialog::showStartOfDownload);
  connect(downloadClient, &DownloadClient::aborted,
          m_downloadDialog, &QProgressDialog::reset);
  connect(m_downloadDialog, &QProgressDialog::canceled,
          downloadClient, &DownloadClient::cancelDownload);
  connect(downloadClient,
    &DownloadClient::downloadFinished,
    m_app,
    &Kid3Application::imageDownloaded);

  connect(m_app, &Kid3Application::fileSelectionUpdateRequested,
          this, &BaseMainWindowImpl::updateCurrentSelection);
  connect(m_app, &Kid3Application::selectedFilesUpdated,
          this, &BaseMainWindowImpl::updateGuiControls);
  connect(m_app, &Kid3Application::selectedFilesChanged,
          this, &BaseMainWindowImpl::applySelectionChange);
  connect(m_app, &Kid3Application::frameModified,
          this, &BaseMainWindowImpl::updateAfterFrameModification);
  connect(m_app, &Kid3Application::confirmedOpenDirectoryRequested,
          this, &BaseMainWindowImpl::confirmedOpenDirectory);
  connect(m_app, &Kid3Application::toggleExpandedRequested,
          this, &BaseMainWindowImpl::toggleExpanded);
  connect(m_app, &Kid3Application::expandFileListRequested,
          this, &BaseMainWindowImpl::expandFileList);
  connect(m_app, &Kid3Application::directoryOpened,
          this, &BaseMainWindowImpl::onDirectoryOpened);
  connect(m_app, &Kid3Application::modifiedChanged,
          this, &BaseMainWindowImpl::updateWindowCaption);
  connect(m_app, &Kid3Application::filteredChanged,
          this, &BaseMainWindowImpl::updateWindowCaption);
  connect(m_app, &Kid3Application::longRunningOperationProgress,
          this, &BaseMainWindowImpl::showOperationProgress);
  connect(m_app, &Kid3Application::aboutToPlayAudio,
          this, &BaseMainWindowImpl::showPlayToolBar);
}

/**
 * Destructor.
 */
BaseMainWindowImpl::~BaseMainWindowImpl()
{
  qDeleteAll(m_playlistEditDialogs);
}

/**
 * Initialize main window.
 * Shall be called at end of constructor body.
 */
void BaseMainWindowImpl::init()
{
  m_statusLabel = new QLabel;
  m_w->statusBar()->addWidget(m_statusLabel);
  m_form = new Kid3Form(m_app, this, m_w);
  m_w->setCentralWidget(m_form);

  m_self->initActions();

  m_w->resize(m_w->sizeHint());

  readOptions();
}

/**
 * Open directory, user has to confirm if current directory modified.
 *
 * @param paths directory or file paths
 */
void BaseMainWindowImpl::confirmedOpenDirectory(const QStringList& paths)
{
  if (!saveModified()) {
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  slotStatusMsg(tr("Opening folder..."));

  m_app->openDirectory(paths, false);

  slotClearStatusMsg();
  QApplication::restoreOverrideCursor();
}

/**
 * Update the recent file list and the caption when a new directory
 * is opened.
 */
void BaseMainWindowImpl::onDirectoryOpened()
{
  m_self->addDirectoryToRecentFiles(m_app->getDirName());
  updateWindowCaption();
}

/**
 * Save application options.
 */
void BaseMainWindowImpl::saveOptions()
{
  m_self->saveConfig();
  m_form->saveConfig();
  m_app->saveConfig();
}

/**
 * Load application options.
 */
void BaseMainWindowImpl::readOptions()
{
  m_app->readConfig();
  m_self->readConfig();
  m_form->readConfig();
}

/**
 * Show progress of long running operation in status bar.
 * @param name name of operation
 * @param done amount of work done
 * @param total total amount of work
 * @param abort if not 0, can be set to true to abort operation
 */
void BaseMainWindowImpl::showOperationProgress(const QString& name,
                                               int done, int total, bool* abort)
{
  if (done == -1) {
    // Operation started.
    if (!m_progressLabel) {
      m_progressLabel = new QLabel;
    }
    if (!m_progressBar) {
      m_progressBar = new QProgressBar;
    }
    if (!m_progressAbortButton) {
      m_progressAbortButton = new QToolButton;
      m_progressAbortButton->setIcon(
            QIcon(m_w->style()->standardIcon(QStyle::SP_BrowserStop)));
      m_progressAbortButton->setToolTip(tr("Abort"));
      m_progressAbortButton->setCheckable(true);
    }
    if (m_statusLabel) {
      m_w->statusBar()->removeWidget(m_statusLabel);
    }
    m_w->statusBar()->addPermanentWidget(m_progressLabel);
    m_w->statusBar()->addPermanentWidget(m_progressBar, 1);
    m_w->statusBar()->addPermanentWidget(m_progressAbortButton, 1);
    m_progressLabel->setText(name);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(total);
    m_progressBar->setValue(0);
    m_progressAbortButton->setChecked(false);
  } else if (done == total && total != 0) {
    // Operation finished.
    if (m_progressLabel) {
      m_w->statusBar()->removeWidget(m_progressLabel);
      delete m_progressLabel;
      m_progressLabel = nullptr;
    }
    if (m_progressBar) {
      m_w->statusBar()->removeWidget(m_progressBar);
      delete m_progressBar;
      m_progressBar = nullptr;
    }
    if (m_progressAbortButton) {
      m_w->statusBar()->removeWidget(m_progressAbortButton);
      delete m_progressAbortButton;
      m_progressAbortButton = nullptr;
      if (m_statusLabel) {
        m_w->statusBar()->addWidget(m_statusLabel);
        m_statusLabel->show();
      }
    }
    slotClearStatusMsg();
  } else if (done < total || (done == 0 && total == 0)) {
    // Operation progress.
    if (m_progressBar) {
      m_progressBar->setMaximum(total);
      m_progressBar->setValue(done);
      // Is needed to get abort button events.
      qApp->processEvents();
    }
    if (m_progressAbortButton && m_progressAbortButton->isChecked() && abort) {
      *abort = true;
    }
  }
}

/**
 * Save all changed files.
 *
 * @param updateGui true to update GUI (controls, status, cursor)
 */
void BaseMainWindowImpl::saveDirectory(bool updateGui)
{
  if (updateGui) {
    updateCurrentSelection();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  }

#ifdef Q_OS_WIN32
  // Close player on Windows because it holds file handles which prevent
  // files from being saved.
  if (m_playToolBar) {
    m_playToolBar->close();
    m_playToolBar.reset();
  }
  m_app->deleteAudioPlayer();
#endif

  const QStringList errorFiles = m_app->saveDirectory();

  if (!errorFiles.empty()) {
    QStringList errorMsgs, notWritableFiles;
    errorMsgs.reserve(errorFiles.size());
    for (const QString& filePath : errorFiles) {
      QFileInfo fileInfo(filePath);
      if (!fileInfo.isWritable()) {
        errorMsgs.append(tr("%1 is not writable").arg(fileInfo.fileName()));
        notWritableFiles.append(filePath); // clazy:exclude=reserve-candidates
      } else {
        errorMsgs.append(fileInfo.fileName());
      }
    }
    if (notWritableFiles.isEmpty()) {
      m_platformTools->errorList(
        m_w, tr("Error while writing file:\n"),
        errorMsgs,
        tr("File Error"));
    } else {
      int rc = m_platformTools->warningYesNoList(
        m_w, tr("Error while writing file. "
                "Do you want to change the permissions?"),
        errorMsgs,
        tr("File Error"));
      if (rc == QMessageBox::Yes) {
        auto model =
            qobject_cast<FileProxyModel*>(m_form->getFileList()->model());
        TaggedFile* taggedFile;
        const auto filePaths = notWritableFiles;
        for (const QString& filePath : filePaths) {
          QFile::setPermissions(filePath,
              QFile::permissions(filePath) | QFile::WriteUser);
          if (model &&
              (taggedFile = FileProxyModel::getTaggedFileOfIndex(
                 model->index(filePath))) != nullptr) {
            taggedFile->undoRevertChangedFilename();
          }
        }
        m_app->saveDirectory();
      }
    }
  }

  if (updateGui) {
    QApplication::restoreOverrideCursor();
    updateGuiControls();
  }
}

/**
 * If anything was modified, save after asking user.
 *
 * @param doNotRevert if true, modifications are not reverted, this can be
 * used to skip the possibly long process if the application is not be closed
 *
 * @return false if user canceled.
 */
bool BaseMainWindowImpl::saveModified(bool doNotRevert)
{
  bool completed = true;

  if(m_app->isModified() && !m_app->getDirName().isEmpty())
  {
    int want_save = m_platformTools->warningYesNoCancel(
        m_w,
        tr("The current folder has been modified.\n"
       "Do you want to save it?"),
        tr("Warning"));
    switch(want_save)
    {
    case QMessageBox::Yes:
      saveDirectory();
      completed = true;
      break;

    case QMessageBox::No:
      if (!doNotRevert) {
        if (m_app->getFileSelectionModel())
          m_app->getFileSelectionModel()->clearSelection();
        m_app->revertFileModifications();
      }
      completed = true;
      break;

    case QMessageBox::Cancel:
      completed = false;
      break;

    default:
      completed = false;
      break;
    }
  }

  return completed;
}

/**
 * If a playlist was modified, save after asking user.
 * @return false if user canceled.
 */
bool BaseMainWindowImpl::saveModifiedPlaylists()
{
  if (m_app->hasModifiedPlaylistModel()) {
    int answer = m_platformTools->warningYesNoCancel(
        m_w,
        tr("A playlist has been modified.\n"
           "Do you want to save it?"),
        tr("Warning"));
    if (answer == QMessageBox::Yes) {
      m_app->saveModifiedPlaylistModels();
    }
    if (answer != QMessageBox::Yes && answer != QMessageBox::No) {
      return false;
    }
  }
  return true;
}

/**
 * Free allocated resources.
 * Our destructor may not be called, so cleanup is done here.
 */
void BaseMainWindowImpl::cleanup()
{
  m_app->getSettings()->sync();
}

/**
 * Update modification state before closing.
 * If anything was modified, save after asking user.
 * Save options before closing.
 * This method shall be called by closeEvent() (Qt) or
 * queryClose() (KDE).
 *
 * @return false if user canceled,
 *         true will quit the application.
 */
bool BaseMainWindowImpl::queryBeforeClosing()
{
  updateCurrentSelection();
  if (saveModified(true) && saveModifiedPlaylists()) {
    saveOptions();
    cleanup();
    return true;
  }
  return false;
}

/**
 * Request new directory and open it.
 */
void BaseMainWindowImpl::slotFileOpen()
{
  updateCurrentSelection();
  if(saveModified()) {
    static QString flt = m_app->createFilterString();
    QString filter(FileConfig::instance().nameFilter());
    QStringList dirs = m_platformTools->getOpenFileNames(
      m_w, QString(), m_app->getDirName(), flt, &filter);
    if (!dirs.isEmpty()) {
      m_app->resetFileFilterIfNotMatching(dirs);
      m_app->openDirectory(dirs);
    }
  }
}

/**
 * Request new directory and open it.
 */
void BaseMainWindowImpl::slotFileOpenDirectory()
{
  updateCurrentSelection();
  if(saveModified()) {
    QString dir = m_platformTools->getExistingDirectory(m_w, QString(),
                                                        m_app->getDirName());
    if (!dir.isEmpty()) {
      m_app->openDirectory({dir});
    }
  }
}

/**
 * Reload the current directory.
 */
void BaseMainWindowImpl::slotFileReload()
{
  updateCurrentSelection();
  if(saveModified()) {
    m_app->openDirectoryAfterReset();
  }
}

/**
 * Open recent directory.
 *
 * @param dir directory to open
 */
void BaseMainWindowImpl::openRecentDirectory(const QString& dir)
{
  updateCurrentSelection();
  confirmedOpenDirectory({dir});
}

/**
 * Save modified files.
 */
void BaseMainWindowImpl::slotFileSave()
{
  saveDirectory(true);
}

/**
 * Quit application.
 */
void BaseMainWindowImpl::slotFileQuit()
{
  slotStatusMsg(tr("Exiting..."));
  m_w->close(); /* this will lead to call of closeEvent(), queryClose() */
  slotClearStatusMsg();
}

/**
 * Change visibility of status bar.
 * @param visible true to show status bar
 */
void BaseMainWindowImpl::setStatusBarVisible(bool visible)
{
  auto model =
      qobject_cast<FileProxyModel*>(m_form->getFileList()->model());
  auto selModel = m_app->getFileSelectionModel();
  if (visible) {
    m_w->statusBar()->show();
    if (model && selModel) {
      connect(model, &FileProxyModel::sortingFinished,
              m_deferredItemCountTimer,
              static_cast<void (QTimer::*)()>(&QTimer::start),
              Qt::UniqueConnection);
      connect(model->sourceModel(), &QAbstractItemModel::dataChanged,
              m_deferredItemCountTimer,
              static_cast<void (QTimer::*)()>(&QTimer::start),
              Qt::UniqueConnection);
      connect(selModel, &QItemSelectionModel::selectionChanged,
              m_deferredSelectionCountTimer,
              static_cast<void (QTimer::*)()>(&QTimer::start),
              Qt::UniqueConnection);
    }
    onItemCountChanged();
    onSelectionCountChanged();
  } else {
    m_deferredItemCountTimer->stop();
    m_deferredSelectionCountTimer->stop();
    m_w->statusBar()->hide();
    if (model && selModel) {
      disconnect(model, &FileProxyModel::sortingFinished,
                 m_deferredItemCountTimer,
                 static_cast<void (QTimer::*)()>(&QTimer::start));
      disconnect(model->sourceModel(), &QAbstractItemModel::dataChanged,
                 m_deferredItemCountTimer,
                 static_cast<void (QTimer::*)()>(&QTimer::start));
      disconnect(selModel, &QItemSelectionModel::selectionChanged,
                 m_deferredSelectionCountTimer,
                 static_cast<void (QTimer::*)()>(&QTimer::start));
    }
    m_folderCount = 0;
    m_fileCount = 0;
    m_selectionCount = 0;
    updateStatusLabel();
  }
}

/**
 * Called when the item count of the file proxy model changed.
 */
void BaseMainWindowImpl::onItemCountChanged()
{
  if (auto model =
      qobject_cast<FileProxyModel*>(m_form->getFileList()->model())) {
    model->countItems(m_app->getRootIndex(), m_folderCount, m_fileCount);
    updateStatusLabel();
  }
}

/**
 * Called when the item count of the file selection model changed.
 */
void BaseMainWindowImpl::onSelectionCountChanged()
{
  if (auto selModel = m_app->getFileSelectionModel()) {
    m_selectionCount = selModel->selectedRows().size();
    updateStatusLabel();
  }
}

/**
 * Update label of status bar with information about the number of files.
 */
void BaseMainWindowImpl::updateStatusLabel()
{
  if (m_statusLabel) {
    QStringList counts;
    if (m_folderCount != 0) {
      counts.append(tr("%n folders", "", m_folderCount));
    }
    if (m_fileCount != 0) {
      counts.append(tr("%n files", "", m_fileCount));
    }
    if (m_selectionCount != 0) {
      counts.append(tr("%n selected", "", m_selectionCount));
    }
    if (counts.isEmpty()) {
      m_statusLabel->setText((tr("Ready.")));
    } else {
      m_statusLabel->setText(counts.join(QLatin1String(", ")));
    }
  }
}

/**
 * Change status message.
 *
 * @param text message
 */
void BaseMainWindowImpl::slotStatusMsg(const QString& text)
{
  m_w->statusBar()->showMessage(text);
  // processEvents() is necessary to make the change of the status bar
  // visible when it is changed back again in the same function,
  // i.e. in the same call from the Qt main event loop.
  qApp->processEvents();
}

/**
 * Clear status message.
 * To be called when a message set with slotStatusMsg() is no longer valid.
 */
void BaseMainWindowImpl::slotClearStatusMsg()
{
  m_w->statusBar()->clearMessage();
}

/**
 * Show playlist dialog.
 */
void BaseMainWindowImpl::slotPlaylistDialog()
{
  if (!m_playlistDialog) {
    m_playlistDialog.reset(new PlaylistDialog(m_w));
  }
  m_playlistDialog->readConfig();
  if (m_playlistDialog->exec() == QDialog::Accepted) {
    PlaylistConfig cfg;
    m_playlistDialog->getCurrentConfig(cfg);
    QString newEmptyPlaylistFileName =
        m_playlistDialog->getFileNameForNewEmptyPlaylist();
    if (newEmptyPlaylistFileName.isEmpty()) {
      writePlaylist(cfg);
    } else {
      m_app->writeEmptyPlaylist(cfg, newEmptyPlaylistFileName);
    }
  }
}

/**
 * Write playlist according to playlist configuration.
 *
 * @param cfg playlist configuration to use
 *
 * @return true if ok.
 */
bool BaseMainWindowImpl::writePlaylist(const PlaylistConfig& cfg)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  slotStatusMsg(tr("Creating playlist..."));

  bool ok = m_app->writePlaylist(cfg);

  slotClearStatusMsg();
  QApplication::restoreOverrideCursor();
  return ok;
}

/**
 * Create playlist.
 *
 * @return true if ok.
 */
bool BaseMainWindowImpl::slotCreatePlaylist()
{
  return writePlaylist(PlaylistConfig::instance());
}

/**
 * Open dialog to edit playlist.
 * @param playlistPath path to playlist file
 */
void BaseMainWindowImpl::showPlaylistEditDialog(const QString& playlistPath)
{
  PlaylistEditDialog* dialog = m_playlistEditDialogs.value(playlistPath);
  if (!dialog) {
    PlaylistModel* model = m_app->playlistModel(playlistPath);
    dialog = new PlaylistEditDialog(model,
                                    m_form->getFileList()->selectionModel(),
                                    m_w);
    connect(dialog, &QDialog::finished,
            this, &BaseMainWindowImpl::onPlaylistEditDialogFinished);
    m_playlistEditDialogs.insert(playlistPath, dialog);

    // The playlist windows are placed above the directory list.
    // If multiple playlist windows are open, they are displaced by the height
    // of the title bar.
    QWidget* dirList = m_form->getDirList();
    int titleBarHeight = dialog->style()->pixelMetric(QStyle::PM_TitleBarHeight);
    int yOffset = titleBarHeight * m_playlistEditDialogs.size();
    QRect geometry(dirList->mapToGlobal(QPoint(0, 0)), dirList->size());
    geometry.setTop(geometry.top() + yOffset);
    dialog->setGeometry(geometry);

    QStringList filesNotFound = model->filesNotFound();
    if (!filesNotFound.isEmpty()) {
      m_platformTools->warningDialog(
            m_w, tr("Files not found"), filesNotFound.join(QLatin1Char('\n')),
            tr("Error"));
    }
  }

  dialog->showNormal();
  dialog->raise();
}

/**
 * Called when a playlist edit dialog is closed.
 */
void BaseMainWindowImpl::onPlaylistEditDialogFinished()
{
  if (auto dialog = qobject_cast<PlaylistEditDialog*>(sender())) {
    m_playlistEditDialogs.remove(m_playlistEditDialogs.key(dialog));
    dialog->deleteLater();
  }
}

/**
 * Update track data and create import dialog.
 */
void BaseMainWindowImpl::setupImportDialog()
{
  m_app->filesToTrackDataModel(ImportConfig::instance().importDest());
  if (!m_importDialog) {
    QString caption(tr("Import"));
    m_importDialog.reset(
      new ImportDialog(m_platformTools, m_w, caption,
                       m_app->getTrackDataModel(),
                       m_app->genreModel(Frame::Tag_2),
                       m_app->getServerImporters(),
                       m_app->getServerTrackImporters()));
    connect(m_importDialog.data(), &QDialog::accepted,
            this, &BaseMainWindowImpl::applyImportedTrackData);
  }
  m_importDialog->clear();
}

/**
 * Set tagged files of directory from imported track data model.
 */
void BaseMainWindowImpl::applyImportedTrackData()
{
  m_app->trackDataModelToFiles(m_importDialog->getDestination());
}

/**
 * Import.
 */
void BaseMainWindowImpl::slotImport()
{
  if (auto action = qobject_cast<QAction*>(sender())) {
    setupImportDialog();
    if (m_importDialog) {
      m_importDialog->showWithSubDialog(action->data().toInt());
    }
  }
}

/**
 * Tag import.
 */
void BaseMainWindowImpl::slotTagImport()
{
  if (!m_tagImportDialog) {
    m_tagImportDialog.reset(new TagImportDialog(m_w, nullptr));
    connect(m_tagImportDialog.data(), &TagImportDialog::trackDataUpdated,
            this, [this]() {
      m_app->importFromTagsToSelection(
            m_tagImportDialog->getDestination(),
            m_tagImportDialog->getSourceFormat(),
            m_tagImportDialog->getExtractionFormat());
    });
  }
  m_tagImportDialog->clear();
  m_tagImportDialog->show();
}

/**
 * Batch import.
 */
void BaseMainWindowImpl::slotBatchImport()
{
  if (!m_batchImportDialog) {
    m_batchImportDialog.reset(new BatchImportDialog(m_app->getServerImporters(),
                                                m_w));
    connect(m_batchImportDialog.data(), &BatchImportDialog::start,
            m_app, static_cast<void (Kid3Application::*)(
              const BatchImportProfile&, Frame::TagVersion)>(
              &Kid3Application::batchImport));
    connect(m_app->getBatchImporter(), &BatchImporter::reportImportEvent,
            m_batchImportDialog.data(), &BatchImportDialog::showImportEvent);
    connect(m_batchImportDialog.data(), &BatchImportDialog::abort,
            m_app->getBatchImporter(), &BatchImporter::abort);
    connect(m_app->getBatchImporter(), &BatchImporter::finished,
            this, &BaseMainWindowImpl::updateGuiControls);
  }
  m_app->getBatchImporter()->clearAborted();
  m_batchImportDialog->readConfig();
  m_batchImportDialog->show();
}

/**
 * Browse album cover artwork.
 */
void BaseMainWindowImpl::slotBrowseCoverArt()
{
  if (!m_browseCoverArtDialog) {
    m_browseCoverArtDialog.reset(new BrowseCoverArtDialog(m_app, m_w));
  }
  FrameCollection frames2;
  QModelIndex index = m_form->getFileList()->currentIndex();
  if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
    taggedFile->readTags(false);
    frames2.clear();
    for (Frame::TagNumber tagNr : Frame::allTagNumbers()) {
      if (frames2.empty()) {
        taggedFile->getAllFrames(tagNr, frames2);
      } else {
        FrameCollection frames1;
        taggedFile->getAllFrames(tagNr, frames1);
        frames2.merge(frames1);
      }
    }
  }

  m_browseCoverArtDialog->readConfig();
  m_browseCoverArtDialog->setFrames(frames2);
  m_browseCoverArtDialog->exec();
}

/**
 * Export.
 */
void BaseMainWindowImpl::slotExport()
{
  m_exportDialog = new ExportDialog(
        m_platformTools, m_w, m_app->getTextExporter());
  m_exportDialog->readConfig();
  ImportTrackDataVector trackDataVector;
  m_app->filesToTrackData(ExportConfig::instance().exportSource(),
                          trackDataVector);
  m_app->getTextExporter()->setTrackData(trackDataVector);
  m_exportDialog->showPreview();
  m_exportDialog->exec();
  delete m_exportDialog;
  m_exportDialog = nullptr;
}

/**
 * Toggle auto hiding of tags.
 */
void BaseMainWindowImpl::slotSettingsAutoHideTags()
{
  GuiConfig::instance().setAutoHideTags(m_self->autoHideTagsAction()->isChecked());
  updateCurrentSelection();
  updateGuiControls();
}

/**
 * Show or hide picture.
 */
void BaseMainWindowImpl::slotSettingsShowHidePicture()
{
  GuiConfig::instance().setHidePicture(!m_self->showHidePictureAction()->isChecked());

  m_form->hidePicture(GuiConfig::instance().hidePicture());
  // In Qt3 the picture is displayed too small if Kid3 is started with picture
  // hidden, and then "Show Picture" is triggered while a file with a picture
  // is selected. Thus updating the controls is only done for Qt4, in Qt3 the
  // file has to be selected again for the picture to be shown.
  if (!GuiConfig::instance().hidePicture()) {
    updateGuiControls();
  }
}

/**
 * Apply configuration changes.
 */
void BaseMainWindowImpl::applyChangedConfiguration()
{
  m_app->applyChangedConfiguration();
  if (!FileConfig::instance().markChanges()) {
    m_form->markChangedFilename(false);
  }
}

/**
 * Find and replace in tags of files.
 * @param findOnly true to display only find part of dialog
 */
void BaseMainWindowImpl::findReplace(bool findOnly)
{
  TagSearcher* tagSearcher = m_app->getTagSearcher();
  if (!m_findReplaceDialog) {
    m_findReplaceDialog = new FindReplaceDialog(m_w);
    connect(m_findReplaceDialog, &FindReplaceDialog::findRequested,
            m_app, &Kid3Application::findText);
    connect(m_findReplaceDialog,
            &FindReplaceDialog::replaceRequested,
            m_app, &Kid3Application::replaceText);
    connect(m_findReplaceDialog,
            &FindReplaceDialog::replaceAllRequested,
            m_app, &Kid3Application::replaceAll);
    connect(m_findReplaceDialog, &QDialog::finished,
            this, &BaseMainWindowImpl::deactivateFindReplace);
    connect(tagSearcher, &TagSearcher::progress,
            m_findReplaceDialog, &FindReplaceDialog::showProgress);
  }
  m_findReplaceDialog->init(findOnly);
  m_findReplaceDialog->show();
  if (!m_findReplaceActive) {
    QModelIndexList selItems(m_app->getFileSelectionModel()->selectedRows());
    if (selItems.size() == 1) {
      tagSearcher->setStartIndex(selItems.first());
    }
    connect(tagSearcher, &TagSearcher::textFound,
            this, &BaseMainWindowImpl::showFoundText);
    connect(tagSearcher, &TagSearcher::textReplaced,
            this, &BaseMainWindowImpl::updateReplacedText);
    m_findReplaceActive = true;
  }
}

/**
 * Deactivate showing of find replace results.
 */
void BaseMainWindowImpl::deactivateFindReplace()
{
  if (m_findReplaceActive) {
    TagSearcher* tagSearcher = m_app->getTagSearcher();
    tagSearcher->abort();
    disconnect(tagSearcher, &TagSearcher::textFound,
               this, &BaseMainWindowImpl::showFoundText);
    disconnect(tagSearcher, &TagSearcher::textReplaced,
               this, &BaseMainWindowImpl::updateReplacedText);
    m_findReplaceActive = false;
  }
}

/**
 * Ensure that found text is made visible in the GUI.
 */
void BaseMainWindowImpl::showFoundText()
{
  const TagSearcher::Position& pos = m_app->getTagSearcher()->getPosition();
  if (pos.isValid()) {
    m_app->getFileSelectionModel()->setCurrentIndex(pos.getFileIndex(),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    if (pos.getPart() == TagSearcher::Position::FileName) {
      m_form->setFilenameSelection(pos.getMatchedPos(), pos.getMatchedLength());
    } else {
      m_form->frameTable(TagSearcher::Position::partToTagNumber(pos.getPart()))
          ->setValueSelection(pos.getFrameIndex(), pos.getMatchedPos(),
                              pos.getMatchedLength());
    }
  }
}

/**
 * Update GUI controls after text has been replaced.
 */
void BaseMainWindowImpl::updateReplacedText()
{
  const TagSearcher::Position& pos = m_app->getTagSearcher()->getPosition();
  if (pos.isValid()) {
    m_app->getFileSelectionModel()->setCurrentIndex(pos.getFileIndex(),
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    updateGuiControls();
  }
}

/**
 * Rename directory.
 */
void BaseMainWindowImpl::slotRenameDirectory()
{
  if (saveModified()) {
    if (!m_renDirDialog) {
      m_renDirDialog.reset(new RenDirDialog(m_w, m_app->getDirRenamer()));
      connect(m_renDirDialog.data(), &RenDirDialog::actionSchedulingRequested,
              m_app, &Kid3Application::scheduleRenameActions);
      connect(m_app->getDirRenamer(), &DirRenamer::actionScheduled,
              m_renDirDialog.data(), &RenDirDialog::displayActionPreview);
    }
    if (TaggedFile* taggedFile =
      TaggedFileOfDirectoryIterator::first(m_app->currentOrRootIndex())) {
      m_renDirDialog->startDialog(taggedFile);
    } else {
      m_renDirDialog->startDialog(nullptr, m_app->getDirName());
    }
    if (m_renDirDialog->exec() == QDialog::Accepted) {
      QString errorMsg(m_app->performRenameActions());
      if (!errorMsg.isEmpty()) {
#ifdef Q_OS_WIN32
        if (m_platformTools->warningContinueCancelList(
              m_w, tr("Error while renaming:\n") +
              tr("Retry after closing directories?"), QStringList(errorMsg),
              tr("File Error"))) {
          m_app->tryRenameActionsAfterReset();
        }
#else
        m_platformTools->warningDialog(m_w, tr("Error while renaming:\n"),
                                       errorMsg, tr("File Error"));
#endif
      }
    }
  }
}

/**
 * Number tracks.
 */
void BaseMainWindowImpl::slotNumberTracks()
{
  if (!m_numberTracksDialog) {
    m_numberTracksDialog.reset(new NumberTracksDialog(m_w));
  }
  m_numberTracksDialog->setTotalNumberOfTracks(
    m_app->getTotalNumberOfTracksInDir(),
        TagConfig::instance().enableTotalNumberOfTracks());
  if (m_numberTracksDialog->exec() == QDialog::Accepted) {
    int nr = m_numberTracksDialog->getStartNumber();
    bool totalEnabled;
    int total = m_numberTracksDialog->getTotalNumberOfTracks(&totalEnabled);
    if (!totalEnabled)
      total = 0;
    TagConfig::instance().setEnableTotalNumberOfTracks(totalEnabled);
    Kid3Application::NumberTrackOptions options;
    if (m_numberTracksDialog->isTrackNumberingEnabled())
      options |= Kid3Application::NumberTracksEnabled;
    if (m_numberTracksDialog->isDirectoryCounterResetEnabled())
      options |= Kid3Application::NumberTracksResetCounterForEachDirectory;
    m_app->numberTracks(nr, total, m_numberTracksDialog->getDestination(),
                        options);
  }
}

/**
 * Filter.
 */
void BaseMainWindowImpl::slotFilter()
{
  if (saveModified()) {
    if (!m_filterDialog) {
      m_filterDialog.reset(new FilterDialog(m_w));
      connect(m_filterDialog.data(), &FilterDialog::apply,
              m_app, static_cast<void (Kid3Application::*)(FileFilter&)>(
                &Kid3Application::applyFilter));
      connect(m_app, &Kid3Application::fileFiltered,
              m_filterDialog.data(), &FilterDialog::showFilterEvent);
      connect(m_app, &Kid3Application::fileFiltered,
              this, &BaseMainWindowImpl::filterProgress);
    }
    FilterConfig::instance().setFilenameFormat(
          FileConfig::instance().toFilenameFormat());
    m_filterDialog->readConfig();
    m_filterDialog->show();
  }
}

/**
 * Show filter operation progress.
 * @param type filter event type
 * @param fileName name of file processed
 * @param passed number of files which passed the filter
 * @param total total number of files checked
 */
void BaseMainWindowImpl::filterProgress(int type, const QString& fileName,
                                        int passed, int total)
{
  Q_UNUSED(fileName)
  switch (type) {
  case FileFilter::Started:
    startProgressMonitoring(tr("Filter"), &BaseMainWindowImpl::terminateFilter,
                            true);
    break;
  case FileFilter::Finished:
  case FileFilter::Aborted:
    stopProgressMonitoring();
    break;
  default:
    checkProgressMonitoring(0, 0, QString::number(passed) +
                            QLatin1Char('/') + QString::number(total));
  }
}

/**
 * Terminate filtering the file list.
 */
void BaseMainWindowImpl::terminateFilter()
{
  if (m_filterDialog) {
    m_filterDialog->abort();
  }
}

/**
 * Play audio file.
 */
void BaseMainWindowImpl::slotPlayAudio()
{
  m_app->playAudio();
}

/**
 * Show play tool bar.
 */
void BaseMainWindowImpl::showPlayToolBar()
{
  if (!m_playToolBar) {
    if (AudioPlayer* player =
        qobject_cast<AudioPlayer*>(m_app->getAudioPlayer())) {
      m_playToolBar.reset(new PlayToolBar(player, m_w));
      m_playToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
      m_w->addToolBar(Qt::BottomToolBarArea, m_playToolBar.data());
      connect(m_playToolBar.data(), &PlayToolBar::errorMessage,
              this, &BaseMainWindowImpl::slotStatusMsg);
#ifdef HAVE_QTDBUS
      connect(m_playToolBar.data(), &PlayToolBar::closed,
              m_app, &Kid3Application::deactivateMprisInterface);
#endif
#ifdef Q_OS_WIN32
      // Phonon on Windows cannot play if the file is open.
      connect(m_playToolBar.data(), &PlayToolBar::aboutToPlay,
              m_app, &Kid3Application::closeFileHandle);
#endif
    }
  }
  m_playToolBar->show();
}

/**
 * Set window title with information from directory, filter and modification
 * state.
 */
void BaseMainWindowImpl::updateWindowCaption()
{
  QString cap;
  if (!m_app->getDirName().isEmpty()) {
    cap += QDir(m_app->getDirName()).dirName();
  }
  if (m_app->isFiltered()) {
    cap += tr(" [filtered %1/%2]")
        .arg(m_app->filterPassedCount()).arg(m_app->filterTotalCount());
  }
  m_self->setWindowCaption(cap, m_app->isModified());
}

/**
 * Update files of current selection.
 */
void BaseMainWindowImpl::updateCurrentSelection()
{
  TaggedFileSelection* selection = m_app->selectionInfo();
  if (!selection->isEmpty()) {
    FOR_ALL_TAGS(tagNr) {
      m_form->frameTable(tagNr)->acceptEdit();
    }
    m_app->frameModelsToTags();

    selection->setFilename(m_form->getFilename());
  }
}

/**
 * Apply selection change and update GUI controls.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 * @param selected selected items
 * @param deselected deselected items
 */
void BaseMainWindowImpl::applySelectionChange(const QItemSelection& selected,
                                              const QItemSelection& deselected)
{
  if (!deselected.isEmpty()) {
    m_app->tagsToFrameModels();
  } else {
    m_app->selectedTagsToFrameModels(selected);
  }
  updateGuiControlsFromSelection();
}

/**
 * Update GUI controls from the tags in the files.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void BaseMainWindowImpl::updateGuiControls()
{
  m_app->tagsToFrameModels();
  updateGuiControlsFromSelection();
}

/**
 * Update GUI controls from the current selection.
 */
void BaseMainWindowImpl::updateGuiControlsFromSelection()
{
  TaggedFileSelection* selection = m_app->selectionInfo();
  m_form->setFilename(selection->getFilename());
  m_form->setFilenameEditEnabled(selection->isSingleFileSelected());
  m_form->setDetailInfo(selection->getDetailInfo());
  FOR_ALL_TAGS(tagNr) {
    m_form->setTagFormat(tagNr, selection->getTagFormat(tagNr));
  }
  if (FileConfig::instance().markChanges()) {
    m_form->markChangedFilename(selection->isFilenameChanged());
  }

  if (!GuiConfig::instance().hidePicture()) {
    m_form->setPictureData(selection->getPicture());
  }

  bool selectionEmpty = selection->isEmpty();
  bool autoHideTags = GuiConfig::instance().autoHideTags();
  FOR_ALL_TAGS(tagNr) {
    m_form->enableControls(tagNr,
        selection->isTagUsed(tagNr) || selectionEmpty);

    if (autoHideTags) {
      m_form->hideTag(tagNr, !selection->hasTag(tagNr));
    }
  }
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 *
 * @param taggedFile the selected file
 * @param tagNr tag number
 */
void BaseMainWindowImpl::updateAfterFrameModification(TaggedFile* taggedFile,
                                                      Frame::TagNumber tagNr)
{
  if (taggedFile) {
    FrameCollection frames;
    taggedFile->getAllFrames(tagNr, frames);
    m_app->frameModel(tagNr)->transferFrames(frames);
  }
}

/**
 * Let user select a frame type.
 * frameSelected() is emitted when the edit dialog is closed with the selected
 * frame as a parameter if a frame is selected.
 *
 * @param frame is filled with the selected frame
 * @param taggedFile tagged file for which frame has to be selected
 */
void BaseMainWindowImpl::selectFrame(Frame* frame, const TaggedFile* taggedFile)
{
  bool ok = false;
  if (taggedFile && frame) {
    QStringList frameIds = taggedFile->getFrameIds(m_editFrameTagNr);
    QMap<QString, QString> nameMap = Frame::getDisplayNameMap(frameIds);
    QString displayName = QInputDialog::getItem(
      m_w, tr("Add Frame"),
      tr("Select the frame ID"), nameMap.keys(), 0, true, &ok);
    if (ok) {
      if (displayName.startsWith(QLatin1Char('!'))) {
        QString name = displayName.mid(1);
        Frame::ExtendedType type(Frame::FT_Other, name);
        *frame = Frame(type, QLatin1String(""), -1);
      } else {
        QString name = nameMap.value(displayName, displayName);
        Frame::Type type = Frame::getTypeFromName(name);
        *frame = Frame(type, QLatin1String(""), name, -1);
      }
    }
  }
  emit frameSelected(m_editFrameTagNr, ok ? frame : nullptr);
}

/**
 * Return object which emits frameSelected(), frameEdited() signals.
 *
 * @return object which emits signals.
 */
QObject* BaseMainWindowImpl::qobject()
{
  return this;
}

/**
 * Get the tag number of the edited frame.
 * @return tag number.
 */
Frame::TagNumber BaseMainWindowImpl::tagNumber() const
{
  return m_editFrameTagNr;
}

/**
 * Set the tag number of the edited frame.
 * @param tagNr tag number
 */
void BaseMainWindowImpl::setTagNumber(Frame::TagNumber tagNr)
{
  m_editFrameTagNr = tagNr;
}

/**
 * Create dialog to edit a frame and update the fields
 * if Ok is returned.
 * frameEdited() is emitted when the edit dialog is closed with the edited
 * frame as a parameter if it was accepted.
 *
 * @param frame frame to edit
 * @param taggedFile tagged file where frame has to be set
 */
void BaseMainWindowImpl::editFrameOfTaggedFile(const Frame* frame,
                                               TaggedFile* taggedFile)
{
  if (!frame || !taggedFile) {
    emit frameEdited(m_editFrameTagNr, nullptr);
    return;
  }

  m_editFrame = *frame;
  m_editFrameTaggedFile = taggedFile;
  QString name(m_editFrame.getInternalName());
  if (name.isEmpty()) {
    name = m_editFrame.getName();
  }
  if (!name.isEmpty()) {
    int nlPos = name.indexOf(QLatin1Char('\n'));
    if (nlPos > 0) {
      // probably "TXXX - User defined text information\nDescription" or
      // "WXXX - User defined URL link\nDescription"
      name.truncate(nlPos);
    }
    name = QCoreApplication::translate("@default", name.toLatin1().data());
  }
  if (!m_editFrameDialog) {
    m_editFrameDialog = new EditFrameFieldsDialog(m_platformTools, m_app, m_w);
    connect(m_editFrameDialog, &QDialog::finished,
            this, &BaseMainWindowImpl::onEditFrameDialogFinished);
  }
  m_editFrameDialog->setWindowTitle(name);
  m_editFrameDialog->setFrame(m_editFrame, m_editFrameTaggedFile,
                              m_editFrameTagNr);
  m_editFrameDialog->show();
}

/**
 * Called when the edit fram dialog is finished.
 * @param result dialog result
 */
void BaseMainWindowImpl::onEditFrameDialogFinished(int result)
{
  if (auto dialog =
      qobject_cast<EditFrameFieldsDialog*>(sender())) {
    if (result == QDialog::Accepted) {
      const Frame::FieldList& fields = dialog->getUpdatedFieldList();
      if (fields.isEmpty()) {
        m_editFrame.setValue(dialog->getFrameValue());
      } else {
        m_editFrame.setFieldList(fields);
        m_editFrame.setValueFromFieldList();
      }
      if (m_editFrameTaggedFile->setFrame(m_editFrameTagNr, m_editFrame)) {
        m_editFrameTaggedFile->markTagChanged(m_editFrameTagNr,
                                              m_editFrame.getType());
      }
    }
  }
  emit frameEdited(m_editFrameTagNr,
                   result == QDialog::Accepted ? &m_editFrame : nullptr);
}

/**
 * Rename the selected file(s).
 */
void BaseMainWindowImpl::renameFile()
{
  QItemSelectionModel* selectModel = m_app->getFileSelectionModel();
  auto model =
      qobject_cast<FileProxyModel*>(m_form->getFileList()->model());
  if (!selectModel || !model)
    return;

  QList<QPersistentModelIndex> selItems;
  const auto indexes = selectModel->selectedRows();
  selItems.reserve(indexes.size());
  for (const QModelIndex& index : indexes)
    selItems.append(index);
  const auto selectedIndexes = selItems;
  for (const QPersistentModelIndex& index : selectedIndexes) {
    TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index);
    QString absFilename, dirName, fileName;
    if (taggedFile) {
      absFilename = taggedFile->getAbsFilename();
      dirName = taggedFile->getDirname();
      fileName = taggedFile->getFilename();
    } else {
      QFileInfo fi(model->fileInfo(index));
      absFilename = fi.filePath();
      dirName = fi.dir().path();
      fileName = fi.fileName();
    }
    bool ok;
    QString newFileName = QInputDialog::getText(
      m_w,
      tr("Rename File"),
      tr("Enter new file name:"),
      QLineEdit::Normal, fileName, &ok);
    if (ok && !newFileName.isEmpty() && newFileName != fileName) {
      if (taggedFile) {
        if (taggedFile->isChanged()) {
          taggedFile->setFilename(newFileName);
          if (selItems.size() == 1)
            m_form->setFilename(newFileName);
          continue;
        }
        // This will close the file.
        // The file must be closed before renaming on Windows.
        taggedFile->closeFileHandle();
      } else if (model->isDir(index)) {
        // The directory must be closed before renaming on Windows.
        TaggedFileIterator::closeFileHandles(index);
      }
      QString newPath = dirName + QLatin1Char('/') + newFileName;
      if (model->rename(index, newFileName)) {
        if (taggedFile) {
          taggedFile->updateCurrentFilename();
          if (selItems.size() == 1) {
            m_form->setFilename(newFileName);
          }
        }
      } else {
#ifdef Q_OS_WIN32
        if (QMessageBox::warning(
              0, tr("File Error"),
              tr("Error while renaming:\n") +
              tr("Rename %1 to %2 failed\n").arg(fileName).arg(newFileName) +
              tr("Retry after closing folders?"),
              QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
          m_app->tryRenameAfterReset(absFilename, newPath);
        }
#else
        QMessageBox::warning(
          nullptr, tr("File Error"),
          tr("Error while renaming:\n") +
          tr("Rename %1 to %2 failed\n").arg(fileName, newFileName),
          QMessageBox::Ok, Qt::NoButton);
#endif
      }
    }
  }
}

/** Only defined for generation of translation files */
#define WANT_TO_DELETE_FOR_PO \
  QT_TRANSLATE_NOOP("@default", "Do you really want to move these %1 items to the trash?")

/**
 * Delete the selected file(s).
 */
void BaseMainWindowImpl::deleteFile()
{
  QItemSelectionModel* selectModel = m_app->getFileSelectionModel();
  auto model =
      qobject_cast<FileProxyModel*>(m_form->getFileList()->model());
  if (!selectModel || !model)
    return;

  QStringList files;
  QList<QPersistentModelIndex> selItems;
  const auto indexes = selectModel->selectedRows();
  selItems.reserve(indexes.size());
  for (const QModelIndex& index : indexes)
    selItems.append(index);
  const auto selectedIndexes = selItems;
  for (const QPersistentModelIndex& index : selectedIndexes) {
    files.append(model->filePath(index)); // clazy:exclude=reserve-candidates
  }

  const int numFiles = files.size();
  if (numFiles > 0) {
    if (m_platformTools->warningContinueCancelList(
          m_w,
          numFiles > 1
          ? tr("Do you really want to move these %1 items to the trash?")
            .arg(numFiles)
          : tr("Do you really want to move this item to the trash?"),
          files,
          tr("Move to Trash"))) {
      bool rmdirError = false;
      files.clear();
      for (const QPersistentModelIndex& index : selectedIndexes) {
        QString absFilename(model->filePath(index));
        if (!QFileInfo(absFilename).isWritable()) {
          QFile::setPermissions(absFilename,
              QFile::permissions(absFilename) | QFile::WriteUser);
        }
        if (model->isDir(index)) {
          if (!m_platformTools->moveToTrash(absFilename)) {
            rmdirError = true;
            files.append(absFilename); // clazy:exclude=reserve-candidates
          }
        } else {
          if (TaggedFile* taggedFile =
              FileProxyModel::getTaggedFileOfIndex(index)) {
            // This will close the file.
            // The file must be closed before deleting on Windows.
            taggedFile->closeFileHandle();
          }
          if (!m_platformTools->moveToTrash(absFilename)) {
            files.append(absFilename); // clazy:exclude=reserve-candidates
          }
        }
      }
      if (!files.isEmpty()) {
        QString txt;
        if (rmdirError)
          txt += tr("Folder must be empty.\n");
        txt += tr("Could not move these files to the Trash");
        m_platformTools->errorList(m_w, txt, files, tr("File Error"));
      }
    }
  }
}

/**
 * Toggle expanded state of directory in the file list.
 * @param index index of directory
 */
void BaseMainWindowImpl::toggleExpanded(const QModelIndex& index)
{
  QTreeView* fileList = m_form->getFileList();
  fileList->setExpanded(index, !fileList->isExpanded(index));
}

/**
 * Expand the file list.
 */
void BaseMainWindowImpl::expandFileList()
{
  m_expandNotificationNeeded = sender() == m_app;
  connect(m_app->getFileProxyModelIterator(),
          &FileProxyModelIterator::nextReady,
          this, &BaseMainWindowImpl::expandNextDirectory);
  // If this slot is invoked from the file list menu action and the
  // shift key is pressed, only expand the current subtree.
  QObject* emitter = sender();
  bool sentFromAction =
      emitter && emitter->metaObject() == &QAction::staticMetaObject;
  bool expandOnlySubtree =
      sentFromAction && QApplication::keyboardModifiers() == Qt::ShiftModifier;
  startProgressMonitoring(tr("Expand All"),
                          &BaseMainWindowImpl::terminateExpandFileList,
                          !expandOnlySubtree);
  m_app->getFileProxyModelIterator()->start(expandOnlySubtree
        ? m_form->getFileList()->currentIndex()
        : m_form->getFileList()->rootIndex());
}

/**
 * Expand item if it is a directory.
 *
 * @param index index of file in file proxy model
 */
void BaseMainWindowImpl::expandNextDirectory(const QPersistentModelIndex& index)
{
  if (index.isValid()) {
    if (m_app->getFileProxyModel()->isDir(index)) {
      m_form->getFileList()->expand(index);
    }
    int done = m_app->getFileProxyModelIterator()->getWorkDone();
    int total = m_app->getFileProxyModelIterator()->getWorkToDo() + done;
    checkProgressMonitoring(done, total, QString());
  } else {
    stopProgressMonitoring();
  }
}

/**
 * Terminate expanding the file list.
 */
void BaseMainWindowImpl::terminateExpandFileList()
{
  m_app->getFileProxyModelIterator()->abort();
  disconnect(m_app->getFileProxyModelIterator(),
             &FileProxyModelIterator::nextReady,
             this, &BaseMainWindowImpl::expandNextDirectory);
  if (m_expandNotificationNeeded) {
    m_expandNotificationNeeded = false;
    m_app->notifyExpandFileListFinished();
  }
}

/**
 * Start monitoring the progress of a possibly long operation.
 *
 * If the operation takes longer than 3 seconds, a progress widget is shown.
 *
 * @param title title to be displayed in progress widget
 * @param terminationHandler method to be called to terminate operation
 * @param disconnectModel true to disconnect the file list models while the
 * progress widget is shown
 */
void BaseMainWindowImpl::startProgressMonitoring(
    const QString& title, void (BaseMainWindowImpl::*terminationHandler)(),
    bool disconnectModel)
{
  if (!m_progressTitle.isEmpty() && m_progressTitle != title) {
    stopProgressMonitoring();
  }
  m_progressTitle = title;
  m_progressTerminationHandler = terminationHandler;
  m_progressDisconnected = disconnectModel;
  m_progressStartTime = QDateTime::currentDateTime();
}

/**
 * Start monitoring the progress started with startProgressMonitoring().
 */
void BaseMainWindowImpl::stopProgressMonitoring()
{
  if (m_progressWidget) {
    m_form->removeLeftSideWidget(m_progressWidget);
    m_progressWidget->reset();
    if (m_progressDisconnected) {
      m_form->getDirList()->reconnectModel();
      m_form->getFileList()->reconnectModel();
      m_form->getFileList()->expandAll();
    }
  }
  if (m_progressTerminationHandler) {
    (this->*m_progressTerminationHandler)();
  }
  m_progressTitle.clear();
  m_progressTerminationHandler = nullptr;
}

/**
 * Check progress of a possibly long operation.
 *
 * Progress monitoring is started with startProgressMonitoring(). This method
 * will check if the opeation is running long enough to show a progress widget
 * and update the progress information. It will call stopProgressMonitoring()
 * when the operation is aborted.
 *
 * @param done amount of work done
 * @param total total amount of work
 * @param text text for progress label
 */
void BaseMainWindowImpl::checkProgressMonitoring(int done, int total,
                                                 const QString& text)
{
  if (m_progressStartTime.isValid() &&
      m_progressStartTime.secsTo(QDateTime::currentDateTime()) >= 3) {
    // Operation is taking some time, show dialog to abort it.
    m_progressStartTime = QDateTime();
    if (!m_progressWidget) {
      m_progressWidget = new ProgressWidget(m_w);
    }
    m_progressWidget->setWindowTitle(m_progressTitle);
    m_progressWidget->setLabelText(QString());
    m_progressWidget->setCancelButtonText(tr("A&bort"));
    m_progressWidget->setMinimum(0);
    m_progressWidget->setMaximum(0);
    m_form->setLeftSideWidget(m_progressWidget);
    if (m_progressDisconnected) {
      m_form->getFileList()->disconnectModel();
      m_form->getDirList()->disconnectModel();
    }
  }
  if (m_progressWidget) {
    m_progressWidget->setValueAndMaximum(done, total);
    m_progressWidget->setLabelText(text);
    if (m_progressWidget->wasCanceled()) {
      stopProgressMonitoring();
    }
  }
}


/**
 * Constructor.
 *
 * @param mainWin main window
 * @param platformTools platform specific tools
 * @param app application context
 */
BaseMainWindow::BaseMainWindow(QMainWindow* mainWin,
                               IPlatformTools* platformTools,
                               Kid3Application* app) :
  m_impl(new BaseMainWindowImpl(mainWin, platformTools, app))
{
  m_impl->setBackPointer(this);
}

/**
 * Destructor.
 */
BaseMainWindow::~BaseMainWindow()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Initialize main window.
 * Shall be called at end of constructor body in derived classes.
 */
void BaseMainWindow::init()
{
  m_impl->init();
}

/**
 * Play audio file.
 */
void BaseMainWindow::slotPlayAudio()
{
  m_impl->slotPlayAudio();
}

/**
 * Change visibility of status bar.
 * @param visible true to show status bar
 */
void BaseMainWindow::setStatusBarVisible(bool visible)
{
  m_impl->setStatusBarVisible(visible);
}

/**
 * Update files of current selection.
 */
void BaseMainWindow::updateCurrentSelection()
{
  m_impl->updateCurrentSelection();
}

/**
 * Open directory, user has to confirm if current directory modified.
 *
 * @param paths directory or file paths
 */
void BaseMainWindow::confirmedOpenDirectory(const QStringList& paths)
{
  m_impl->confirmedOpenDirectory(paths);
}

/**
 * Update modification state before closing.
 * If anything was modified, save after asking user.
 * Save options before closing.
 * This method shall be called by closeEvent() (Qt) or
 * queryClose() (KDE).
 *
 * @return false if user canceled,
 *         true will quit the application.
 */
bool BaseMainWindow::queryBeforeClosing()
{
  return m_impl->queryBeforeClosing();
}

/**
 * Open recent directory.
 *
 * @param dir directory to open
 */
void BaseMainWindow::openRecentDirectory(const QString& dir)
{
  m_impl->openRecentDirectory(dir);
}

/**
 * Set window title with information from directory, filter and modification
 * state.
 */
void BaseMainWindow::updateWindowCaption()
{
  m_impl->updateWindowCaption();
}

/**
 * Access to application.
 * @return application.
 */
Kid3Application* BaseMainWindow::app()
{
  return m_impl->app();
}

/**
 * Access to main form.
 * @return main form.
 */
Kid3Form* BaseMainWindow::form()
{
  return m_impl->form();
}
