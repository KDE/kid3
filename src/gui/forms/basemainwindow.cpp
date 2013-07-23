/**
 * \file basemainwindow.cpp
 * Base class for main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#include "basemainwindow.h"
#include <QDir>
#include <QCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QIcon>
#include <QToolBar>
#include <QStatusBar>
#include <QApplication>
#include "qtcompatmac.h"
#include "kid3form.h"
#include "kid3application.h"
#include "framelist.h"
#include "frametablemodel.h"
#include "frametable.h"
#include "importdialog.h"
#include "batchimportdialog.h"
#include "browsecoverartdialog.h"
#include "exportdialog.h"
#include "numbertracksdialog.h"
#include "filterdialog.h"
#include "rendirdialog.h"
#include "downloadclient.h"
#include "downloaddialog.h"
#include "playlistdialog.h"
#include "editframedialog.h"
#include "editframefieldsdialog.h"
#include "fileproxymodel.h"
#include "fileproxymodeliterator.h"
#include "modeliterator.h"
#include "filelist.h"
#include "dirlist.h"
#include "pictureframe.h"
#include "fileconfig.h"
#include "playlistconfig.h"
#include "exportconfig.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "filterconfig.h"
#include "contexthelp.h"
#include "frame.h"
#include "textexporter.h"
#include "serverimporter.h"
#include "batchimporter.h"
#include "dirrenamer.h"
#include "iplatformtools.h"
#include "qtcompatmac.h"
#include "saferename.h"
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
#include "audioplayer.h"
#endif
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
#include "playtoolbar.h"
#endif

/**
 * Constructor.
 *
 * @param mainWin main window widget
 * @param platformTools platform specific tools
 */
BaseMainWindowImpl::BaseMainWindowImpl(QMainWindow* mainWin,
                                       IPlatformTools* platformTools) :
  m_platformTools(platformTools), m_w(mainWin), m_self(0),
  m_app(new Kid3Application(m_platformTools, this)),
  m_importDialog(0), m_batchImportDialog(0), m_browseCoverArtDialog(0),
  m_exportDialog(0), m_renDirDialog(0),
  m_numberTracksDialog(0), m_filterDialog(0),
  m_downloadDialog(new DownloadDialog(m_w, tr("Download"))),
  m_playlistDialog(0), m_progressDialog(0)
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  , m_playToolBar(0)
#endif
{
  ContextHelp::init(m_platformTools);

  DownloadClient* downloadClient = m_app->getDownloadClient();
  connect(downloadClient, SIGNAL(progress(QString,int,int)),
          m_downloadDialog, SLOT(updateProgressStatus(QString,int,int)));
  connect(downloadClient, SIGNAL(downloadStarted(QString)),
          m_downloadDialog, SLOT(showStartOfDownload(QString)));
  connect(downloadClient, SIGNAL(aborted()),
          m_downloadDialog, SLOT(reset()));
  connect(m_downloadDialog, SIGNAL(canceled()),
          downloadClient, SLOT(cancelDownload()));
  connect(downloadClient,
    SIGNAL(downloadFinished(const QByteArray&, const QString&, const QString&)),
    m_app,
    SLOT(imageDownloaded(const QByteArray&, const QString&, const QString&)));

  connect(m_app, SIGNAL(fileSelectionUpdateRequested()),
          this, SLOT(updateCurrentSelection()));
  connect(m_app, SIGNAL(selectedFilesUpdated()),
          this, SLOT(updateGuiControls()));
  connect(m_app, SIGNAL(frameModified(TaggedFile*)),
          this, SLOT(updateAfterFrameModification(TaggedFile*)));
  connect(m_app, SIGNAL(fileModified()),
          this, SLOT(updateModificationState()));
  connect(m_app, SIGNAL(confirmedOpenDirectoryRequested(QString)),
          this, SLOT(confirmedOpenDirectory(QString)));
  connect(m_app, SIGNAL(directoryOpened(QModelIndex,QModelIndex)),
          this, SLOT(onDirectoryOpened()));
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  connect(m_app, SIGNAL(aboutToPlayAudio()), this, SLOT(showPlayToolBar()));
#endif
}

/**
 * Destructor.
 */
BaseMainWindowImpl::~BaseMainWindowImpl()
{
  delete m_importDialog;
  delete m_batchImportDialog;
  delete m_renDirDialog;
  delete m_numberTracksDialog;
  delete m_filterDialog;
  delete m_browseCoverArtDialog;
  delete m_playlistDialog;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  delete m_playToolBar;
#endif
}

/**
 * Initialize main window.
 * Shall be called at end of constructor body.
 */
void BaseMainWindowImpl::init()
{
  m_w->statusBar()->showMessage(tr("Ready."));
  m_form = new Kid3Form(m_app, this, m_w);
  m_w->setCentralWidget(m_form);

  m_self->initActions();

  m_w->resize(m_w->sizeHint());

  readOptions();
}

/**
 * Open directory, user has to confirm if current directory modified.
 *
 * @param dir directory or file path
 */
void BaseMainWindowImpl::confirmedOpenDirectory(const QString& dir)
{
  if (!saveModified()) {
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  slotStatusMsg(tr("Opening directory..."));

  m_app->openDirectory(dir, false);

  slotStatusMsg(tr("Ready."));
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
 * Save all changed files.
 *
 * @param updateGui true to update GUI (controls, status, cursor)
 */
void BaseMainWindowImpl::saveDirectory(bool updateGui)
{
  if (updateGui) {
    updateCurrentSelection();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    slotStatusMsg(tr("Saving directory..."));
  }

  QProgressBar* progress = new QProgressBar;
  m_w->statusBar()->addPermanentWidget(progress);
  progress->setMinimum(0);
  connect(m_app, SIGNAL(saveStarted(int)),
          progress, SLOT(setMaximum(int)));
  connect(m_app, SIGNAL(saveProgress(int)),
          progress, SLOT(setValue(int)));
  qApp->processEvents();

  QStringList errorFiles = m_app->saveDirectory();

  m_w->statusBar()->removeWidget(progress);
  delete progress;
  updateModificationState();
  if (!errorFiles.empty()) {
    m_platformTools->errorList(
      m_w, tr("Error while writing file:\n"),
      errorFiles,
      tr("File Error"));
  }

  if (updateGui) {
    slotStatusMsg(tr("Ready."));
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
  bool completed=true;

  if(m_app->isModified() && !m_app->getDirName().isEmpty())
  {
    int want_save = m_platformTools->warningYesNoCancel(
        m_w,
        tr("The current directory has been modified.\n"
       "Do you want to save it?"),
        tr("Warning"));
    switch(want_save)
    {
    case QMessageBox::Yes:
      saveDirectory();
      completed=true;
      break;

    case QMessageBox::No:
      if (!doNotRevert) {
        if (m_form->getFileList()->selectionModel())
          m_form->getFileList()->selectionModel()->clearSelection();
        m_app->revertFileModifications();
        m_app->setModified(false);
      }
      completed=true;
      break;

    case QMessageBox::Cancel:
      completed=false;
      break;

    default:
      completed=false;
      break;
    }
  }

  return completed;
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
  if (saveModified(true)) {
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
    QString filter(FileConfig::instance().m_nameFilter);
    QString dir = m_platformTools->getOpenFileName(
      m_w, QString(), m_app->getDirName(), flt, &filter);
    if (!dir.isEmpty()) {
      if (!filter.isEmpty()) {
        FileConfig::instance().m_nameFilter = filter;
      }
      m_app->openDirectory(dir);
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
      m_app->openDirectory(dir);
    }
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
  confirmedOpenDirectory(dir);
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
 * Show playlist dialog.
 */
void BaseMainWindowImpl::slotPlaylistDialog()
{
  if (!m_playlistDialog) {
    m_playlistDialog = new PlaylistDialog(m_w);
  }
  m_playlistDialog->readConfig();
  if (m_playlistDialog->exec() == QDialog::Accepted) {
    PlaylistConfig cfg;
    m_playlistDialog->getCurrentConfig(cfg);
    writePlaylist(cfg);
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

  slotStatusMsg(tr("Ready."));
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
 * Update track data and create import dialog.
 */
void BaseMainWindowImpl::setupImportDialog()
{
  m_app->filesToTrackDataModel(ImportConfig::instance().m_importDest);
  if (!m_importDialog) {
    QString caption(tr("Import"));
    m_importDialog =
      new ImportDialog(m_platformTools, m_w,
                       caption, m_app->getTrackDataModel(),
                       m_app->getServerImporters(),
                       m_app->getServerTrackImporters());
    connect(m_importDialog, SIGNAL(accepted()),
            this, SLOT(applyImportedTrackData()));
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
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    setupImportDialog();
    if (m_importDialog) {
      m_importDialog->showWithSubDialog(action->data().toInt());
    }
  }
}

/**
 * Batch import.
 */
void BaseMainWindowImpl::slotBatchImport()
{
  if (!m_batchImportDialog) {
    m_batchImportDialog = new BatchImportDialog(m_app->getServerImporters(),
                                                m_w);
    connect(m_batchImportDialog,
            SIGNAL(start(BatchImportProfile,TrackData::TagVersion)),
            m_app,
            SLOT(batchImport(BatchImportProfile,TrackData::TagVersion)));
    connect(m_app->getBatchImporter(),
            SIGNAL(reportImportEvent(BatchImportProfile::ImportEventType,
                                     QString)),
            m_batchImportDialog,
            SLOT(showImportEvent(BatchImportProfile::ImportEventType,
                                 QString)));
    connect(m_batchImportDialog, SIGNAL(abort()),
            m_app->getBatchImporter(), SLOT(abort()));
    connect(m_app->getBatchImporter(), SIGNAL(finished()),
            this, SLOT(updateGuiControls()));
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
    m_browseCoverArtDialog = new BrowseCoverArtDialog(m_w);
  }
  FrameCollection frames2;
  QModelIndex index = m_form->getFileList()->currentIndex();
  if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
    taggedFile->readTags(false);
    FrameCollection frames1;
    taggedFile->getAllFramesV1(frames1);
    taggedFile->getAllFramesV2(frames2);
    frames2.merge(frames1);
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
  m_app->filesToTrackData(ExportConfig::instance().m_exportSrcV1,
                          trackDataVector);
  m_app->getTextExporter()->setTrackData(trackDataVector);
  m_exportDialog->showPreview();
  m_exportDialog->exec();
  delete m_exportDialog;
  m_exportDialog = 0;
}

/**
 * Toggle auto hiding of tags.
 */
void BaseMainWindowImpl::slotSettingsAutoHideTags()
{
  GuiConfig::instance().m_autoHideTags = m_self->autoHideTagsAction()->isChecked();
  updateCurrentSelection();
  updateGuiControls();
}

/**
 * Show or hide picture.
 */
void BaseMainWindowImpl::slotSettingsShowHidePicture()
{
  GuiConfig::instance().m_hidePicture = !m_self->showHidePictureAction()->isChecked();

  m_form->hidePicture(GuiConfig::instance().m_hidePicture);
  // In Qt3 the picture is displayed too small if Kid3 is started with picture
  // hidden, and then "Show Picture" is triggered while a file with a picture
  // is selected. Thus updating the controls is only done for Qt4, in Qt3 the
  // file has to be selected again for the picture to be shown.
  if (!GuiConfig::instance().m_hidePicture) {
    updateGuiControls();
  }
}

/**
 * Apply configuration changes.
 */
void BaseMainWindowImpl::applyChangedConfiguration()
{
  m_app->saveConfig();
  if (!TagConfig::instance().markTruncations()) {
    m_app->frameModelV1()->markRows(0);
  }
  if (!FileConfig::instance().m_markChanges) {
    m_app->frameModelV1()->markChangedFrames(0);
    m_app->frameModelV2()->markChangedFrames(0);
    m_form->markChangedFilename(false);
  }
  m_app->notifyConfigurationChange();
  quint64 oldQuickAccessFrames = FrameCollection::getQuickAccessFrames();
  if (TagConfig::instance().quickAccessFrames() != oldQuickAccessFrames) {
    FrameCollection::setQuickAccessFrames(
          TagConfig::instance().quickAccessFrames());
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
      m_renDirDialog = new RenDirDialog(m_w, m_app->getDirRenamer());
      connect(m_renDirDialog, SIGNAL(actionSchedulingRequested()),
              m_app, SLOT(scheduleRenameActions()));
      connect(m_app->getDirRenamer(), SIGNAL(actionScheduled(QStringList)),
              m_renDirDialog, SLOT(displayActionPreview(QStringList)));
    }
    if (TaggedFile* taggedFile =
      TaggedFileOfDirectoryIterator::first(m_app->currentOrRootIndex())) {
      m_renDirDialog->startDialog(taggedFile);
    } else {
      m_renDirDialog->startDialog(0, m_app->getDirName());
    }
    if (m_renDirDialog->exec() == QDialog::Accepted) {
      QString errorMsg(m_app->performRenameActions());
      if (!errorMsg.isEmpty()) {
        m_platformTools->warningDialog(m_w, tr("Error while renaming:\n"),
                                       errorMsg, tr("File Error"));
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
    m_numberTracksDialog = new NumberTracksDialog(m_w);
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
    m_app->numberTracks(nr, total, m_numberTracksDialog->getDestination());
  }
}

/**
 * Filter.
 */
void BaseMainWindowImpl::slotFilter()
{
  if (saveModified()) {
    if (!m_filterDialog) {
      m_filterDialog = new FilterDialog(m_w);
      connect(m_filterDialog, SIGNAL(apply(FileFilter&)),
              m_app, SLOT(applyFilter(FileFilter&)));
      connect(m_app, SIGNAL(fileFiltered(FileFilter::FilterEventType,QString)),
              m_filterDialog,
              SLOT(showFilterEvent(FileFilter::FilterEventType,QString)));
    }
    FilterConfig::instance().setFilenameFormat(
          m_app->getTagsToFilenameFormat());
    m_filterDialog->readConfig();
    m_filterDialog->show();
  }
}

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
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
    m_playToolBar = new PlayToolBar(m_app->getAudioPlayer(), m_w);
    m_playToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    m_w->addToolBar(Qt::BottomToolBarArea, m_playToolBar);
    connect(m_playToolBar, SIGNAL(errorMessage(const QString&)),
            this, SLOT(slotStatusMsg(const QString&)));
  }
  m_playToolBar->show();
}
#endif

/**
 * Update modification state, caption and listbox entries.
 */
void BaseMainWindowImpl::updateModificationState()
{
  bool modified = false;
  TaggedFileIterator it(m_form->getFileList()->rootIndex());
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    if (taggedFile->isChanged()) {
      modified = true;
      m_form->getFileList()->dataChanged(taggedFile->getIndex(),
                                         taggedFile->getIndex());
    }
  }
  m_app->setModified(modified);
  updateWindowCaption();
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
    cap += tr(" [filtered]");
  }
  m_self->setWindowCaption(cap, m_app->isModified());
}

/**
 * Update files of current selection.
 */
void BaseMainWindowImpl::updateCurrentSelection()
{
  const QList<QPersistentModelIndex>& selItems =
    m_form->getFileList()->getCurrentSelection();
  int numFiles = selItems.size();
  if (numFiles > 0) {
    m_form->frameTableV1()->acceptEdit();
    m_form->frameTableV2()->acceptEdit();
    FrameCollection framesV1(m_app->frameModelV1()->getEnabledFrames());
    FrameCollection framesV2(m_app->frameModelV2()->getEnabledFrames());
    for (QList<QPersistentModelIndex>::const_iterator it = selItems.begin();
         it != selItems.end();
         ++it) {
      if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(*it)) {
        taggedFile->setFramesV1(framesV1);
        taggedFile->setFramesV2(framesV2);
        if (m_form->isFilenameEditEnabled()) {
          taggedFile->setFilename(m_form->getFilename());
        }
      }
    }
  }
  updateModificationState();
}

/**
 * Update GUI controls from the tags in the files.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void BaseMainWindowImpl::updateGuiControls()
{
  TaggedFile* single_v2_file = 0;
  int num_v1_selected = 0;
  int num_v2_selected = 0;
  bool tagV1Supported = false;
  bool hasTagV1 = false;
  bool hasTagV2 = false;

  m_form->getFileList()->updateCurrentSelection();
  const QList<QPersistentModelIndex>& selItems =
      m_form->getFileList()->getCurrentSelection();

  for (QList<QPersistentModelIndex>::const_iterator it = selItems.begin();
       it != selItems.end();
       ++it) {
    TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(*it);
    if (taggedFile) {
      taggedFile->readTags(false);

      taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);

      if (taggedFile->isTagV1Supported()) {
        if (num_v1_selected == 0) {
          FrameCollection frames;
          taggedFile->getAllFramesV1(frames);
          m_app->frameModelV1()->transferFrames(frames);
        }
        else {
          FrameCollection fileFrames;
          taggedFile->getAllFramesV1(fileFrames);
          m_app->frameModelV1()->filterDifferent(fileFrames);
        }
        ++num_v1_selected;
        tagV1Supported = true;
      }
      if (num_v2_selected == 0) {
        FrameCollection frames;
        taggedFile->getAllFramesV2(frames);
        m_app->frameModelV2()->transferFrames(frames);
        single_v2_file = taggedFile;
      }
      else {
        FrameCollection fileFrames;
        taggedFile->getAllFramesV2(fileFrames);
        m_app->frameModelV2()->filterDifferent(fileFrames);
        single_v2_file = 0;
      }
      ++num_v2_selected;

      hasTagV1 = hasTagV1 || taggedFile->hasTagV1();
      hasTagV2 = hasTagV2 || taggedFile->hasTagV2();
    }
  }

  TaggedFile::DetailInfo info;
  if (single_v2_file) {
    m_app->getFrameList()->setTaggedFile(single_v2_file);
    m_form->setFilenameEditEnabled(true);
    m_form->setFilename(single_v2_file->getFilename());
    single_v2_file->getDetailInfo(info);
    m_form->setDetailInfo(info);
    m_form->setTagFormatV1(single_v2_file->getTagFormatV1());
    m_form->setTagFormatV2(single_v2_file->getTagFormatV2());

    if (TagConfig::instance().markTruncations()) {
      m_app->frameModelV1()->markRows(single_v2_file->getTruncationFlags());
    }
    if (FileConfig::instance().m_markChanges) {
      m_app->frameModelV1()->markChangedFrames(
        single_v2_file->getChangedFramesV1());
      m_app->frameModelV2()->markChangedFrames(
        single_v2_file->getChangedFramesV2());
      m_form->markChangedFilename(single_v2_file->isFilenameChanged());
    }
  }
  else {
    if (num_v2_selected > 1) {
      m_form->setFilenameEditEnabled(false);
      m_form->setFilename(Frame::differentRepresentation());
    }
    m_form->setDetailInfo(info);
    m_form->setTagFormatV1(QString::null);
    m_form->setTagFormatV2(QString::null);

    if (TagConfig::instance().markTruncations()) {
      m_app->frameModelV1()->markRows(0);
    }
    if (FileConfig::instance().m_markChanges) {
      m_app->frameModelV1()->markChangedFrames(0);
      m_app->frameModelV2()->markChangedFrames(0);
      m_form->markChangedFilename(false);
    }
  }
  if (!GuiConfig::instance().m_hidePicture) {
    FrameCollection::const_iterator it =
      m_app->frameModelV2()->frames().find(Frame(Frame::FT_Picture, QLatin1String(""), QLatin1String(""), -1));
    if (it == m_app->frameModelV2()->frames().end() ||
        it->isInactive()) {
      m_form->setPictureData(0);
    } else {
      QByteArray data;
      m_form->setPictureData(PictureFrame::getData(*it, data) ? &data : 0);
    }
  }
  m_app->frameModelV1()->setAllCheckStates(num_v1_selected == 1);
  m_app->frameModelV2()->setAllCheckStates(num_v2_selected == 1);
  updateModificationState();

  if (num_v1_selected == 0 && num_v2_selected == 0) {
    tagV1Supported = true;
  }
  m_form->enableControlsV1(tagV1Supported);

  if (GuiConfig::instance().m_autoHideTags) {
    // If a tag is supposed to be absent, make sure that there is really no
    // unsaved data in the tag.
    if (!hasTagV1 && tagV1Supported) {
      const FrameCollection& frames = m_app->frameModelV1()->frames();
      for (FrameCollection::const_iterator it = frames.begin();
           it != frames.end();
           ++it) {
        if (!(*it).getValue().isEmpty()) {
          hasTagV1 = true;
          break;
        }
      }
    }
    if (!hasTagV2) {
      const FrameCollection& frames = m_app->frameModelV2()->frames();
      for (FrameCollection::const_iterator it = frames.begin();
           it != frames.end();
           ++it) {
        if (!(*it).getValue().isEmpty()) {
          hasTagV2 = true;
          break;
        }
      }
    }
    m_form->hideV1(!hasTagV1);
    m_form->hideV2(!hasTagV2);
  }
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 *
 * @param taggedFile the selected file
 */
void BaseMainWindowImpl::updateAfterFrameModification(TaggedFile* taggedFile)
{
  if (taggedFile) {
    FrameCollection frames;
    taggedFile->getAllFramesV2(frames);
    m_app->frameModelV2()->transferFrames(frames);
    updateModificationState();
  }
}

/**
 * Get type of frame from translated name.
 *
 * @param name name, spaces and case are ignored
 *
 * @return type.
 */
static Frame::Type getTypeFromTranslatedName(QString name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
      Frame::Type type = static_cast<Frame::Type>(i);
      strNumMap.insert(Frame::ExtendedType(type, QLatin1String("")).getTranslatedName().
                       remove(QLatin1Char(' ')).toUpper(), type);
    }
  }
  QMap<QString, int>::const_iterator it =
    strNumMap.find(name.remove(QLatin1Char(' ')).toUpper());
  if (it != strNumMap.end()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::FT_Other;
}

/**
 * Let user select a frame type.
 *
 * @param frame is filled with the selected frame if true is returned
 * @param taggedFile tagged file for which frame has to be selected
 *
 * @return false if no frame selected.
 */
bool BaseMainWindowImpl::selectFrame(Frame* frame, const TaggedFile* taggedFile)
{
  bool ok = false;
  if (taggedFile && frame) {
    QString name = QInputDialog::getItem(
      m_w, tr("Add Frame"),
      tr("Select the frame ID"), taggedFile->getFrameIds(), 0, true, &ok);
    if (ok) {
      Frame::Type type = getTypeFromTranslatedName(name);
      *frame = Frame(type, QLatin1String(""), name, -1);
    }
  }
  return ok;
}

/**
 * Create dialog to edit a frame and update the fields
 * if Ok is returned.
 *
 * @param frame frame to edit
 * @param taggedFile tagged file where frame has to be set
 *
 * @return true if Ok selected in dialog.
 */
bool BaseMainWindowImpl::editFrameOfTaggedFile(Frame* frame, TaggedFile* taggedFile)
{
  if (!frame || !taggedFile)
    return false;

  bool result = true;
  QString name(frame->getInternalName());
  if (!name.isEmpty()) {
    int nlPos = name.indexOf(QLatin1Char('\n'));
    if (nlPos > 0) {
      // probably "TXXX - User defined text information\nDescription" or
      // "WXXX - User defined URL link\nDescription"
      name.truncate(nlPos);
    }
    name = QCoreApplication::translate("@default", name.toLatin1().data());
  }
  if (frame->getFieldList().empty()) {
    EditFrameDialog* dialog =
      new EditFrameDialog(m_w, name, frame->getValue());
    result = dialog && dialog->exec() == QDialog::Accepted;
    if (result) {
      frame->setValue(dialog->getText());
    }
  } else {
    EditFrameFieldsDialog* dialog =
      new EditFrameFieldsDialog(m_platformTools,
                                m_w, name, *frame, taggedFile);
    result = dialog && dialog->exec() == QDialog::Accepted;
    if (result) {
      frame->setFieldList(dialog->getUpdatedFieldList());
      frame->setValueFromFieldList();
    }
  }
  if (result) {
    if (taggedFile->setFrameV2(*frame)) {
      taggedFile->markTag2Changed(frame->getType());
    }
  }
  return result;
}

/**
 * Rename the selected file(s).
 */
void BaseMainWindowImpl::renameFile()
{
  QItemSelectionModel* selectModel = m_form->getFileList()->selectionModel();
  FileProxyModel* model =
      qobject_cast<FileProxyModel*>(m_form->getFileList()->model());
  if (!selectModel || !model)
    return;

  QList<QPersistentModelIndex> selItems;
  foreach (QModelIndex index, selectModel->selectedIndexes())
    selItems.append(index);
  foreach (QPersistentModelIndex index, selItems) {
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
      if (!Utils::safeRename(absFilename, newPath)) {
        QMessageBox::warning(
          0, tr("File Error"),
          tr("Error while renaming:\n") +
          tr("Rename %1 to %2 failed\n").arg(fileName).arg(newFileName),
          QMessageBox::Ok, Qt::NoButton);
      }
    }
  }
}

/** Only defined for generation of translation files */
#define WANT_TO_DELETE_FOR_PO QT_TRANSLATE_NOOP("@default", "Do you really want to move these %1 items to the trash?")

/**
 * Delete the selected file(s).
 */
void BaseMainWindowImpl::deleteFile()
{
  QItemSelectionModel* selectModel = m_form->getFileList()->selectionModel();
  FileProxyModel* model =
      qobject_cast<FileProxyModel*>(m_form->getFileList()->model());
  if (!selectModel || !model)
    return;

  QStringList files;
  QList<QPersistentModelIndex> selItems;
  foreach (QModelIndex index, selectModel->selectedIndexes())
    selItems.append(index);
  foreach (QPersistentModelIndex index, selItems) {
    files.append(model->filePath(index));
  }

  unsigned numFiles = files.size();
  if (numFiles > 0) {
    if (m_platformTools->warningContinueCancelList(
          m_w,
          numFiles > 1
          ? tr("Do you really want to move these %1 items to the trash?").
            arg(numFiles)
          : tr("Do you really want to move this item to the trash?"),
          files,
          tr("Move to Trash"))) {
      bool rmdirError = false;
      files.clear();
      foreach (QPersistentModelIndex index, selItems) {
        QString absFilename(model->filePath(index));
        if (model->isDir(index)) {
          if (!m_platformTools->moveToTrash(absFilename)) {
            rmdirError = true;
            files.append(absFilename);
          }
        } else {
          if (TaggedFile* taggedFile =
              FileProxyModel::getTaggedFileOfIndex(index)) {
            // This will close the file.
            // The file must be closed before deleting on Windows.
            taggedFile->closeFileHandle();
          }
          if (!m_platformTools->moveToTrash(absFilename)) {
            files.append(absFilename);
          }
        }
      }
      if (!files.isEmpty()) {
        QString txt;
        if (rmdirError)
          txt += tr("Directory must be empty.\n");
        txt += tr("Could not move these files to the Trash");
        m_platformTools->errorList(m_w, txt, files, tr("File Error"));
      }
    }
  }
}

/**
 * Expand the file list.
 */
void BaseMainWindowImpl::expandFileList()
{
  m_expandFileListStartTime = QDateTime::currentDateTime();
  connect(m_app->getFileProxyModelIterator(),
          SIGNAL(nextReady(QPersistentModelIndex)),
          this, SLOT(expandNextDirectory(QPersistentModelIndex)));
  m_app->getFileProxyModelIterator()->start(m_form->getFileList()->rootIndex());
}

/**
 * Expand item if it is a directory.
 *
 * @param index index of file in file proxy model
 */
void BaseMainWindowImpl::expandNextDirectory(const QPersistentModelIndex& index)
{
  bool terminated = !index.isValid();
  if (!terminated) {
    if (m_app->getFileProxyModel()->isDir(index)) {
      m_form->getFileList()->expand(index);
    }
    if (m_expandFileListStartTime.isValid() &&
        m_expandFileListStartTime.secsTo(QDateTime::currentDateTime()) >= 3) {
      // Operation is taking some time, show dialog to abort it.
      m_expandFileListStartTime = QDateTime();
      if (!m_progressDialog) {
        m_progressDialog = new QProgressDialog(m_w);
      }
      m_progressDialog->setWindowTitle(tr("Expand All"));
      m_progressDialog->setLabelText(QString());
      m_progressDialog->setCancelButtonText(tr("A&bort"));
      m_progressDialog->setMinimum(0);
      m_progressDialog->setMaximum(0);
      m_progressDialog->setAutoClose(true);
      m_progressDialog->show();
    }
    if (m_progressDialog && m_progressDialog->wasCanceled()) {
      terminated = true;
    }
  }
  if (terminated) {
    m_app->getFileProxyModelIterator()->abort();
    disconnect(m_app->getFileProxyModelIterator(),
               SIGNAL(nextReady(QPersistentModelIndex)),
               this, SLOT(expandNextDirectory(QPersistentModelIndex)));
    if (m_progressDialog) {
      m_progressDialog->reset();
    }
  }
}


/**
 * Constructor.
 *
 * @param platformTools platform specific tools
 */
BaseMainWindow::BaseMainWindow(QMainWindow* mainWin,
                               IPlatformTools* platformTools) :
  m_impl(new BaseMainWindowImpl(mainWin, platformTools))
{
  m_impl->setBackPointer(this);
}

/**
 * Destructor.
 */
BaseMainWindow::~BaseMainWindow()
{
  delete m_impl;
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
 * Let user select a frame type.
 *
 * @param frame is filled with the selected frame if true is returned
 * @param taggedFile tagged file for which frame has to be selected
 *
 * @return false if no frame selected.
 */
bool BaseMainWindow::selectFrame(Frame* frame, const TaggedFile* taggedFile)
{
  return m_impl->selectFrame(frame, taggedFile);
}

/**
 * Create dialog to edit a frame and update the fields
 * if Ok is returned.
 *
 * @param frame frame to edit
 * @param taggedFile tagged file where frame has to be set
 *
 * @return true if Ok selected in dialog.
 */
bool BaseMainWindow::editFrameOfTaggedFile(Frame* frame, TaggedFile* taggedFile)
{
  return m_impl->editFrameOfTaggedFile(frame, taggedFile);
}

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
/**
 * Play audio file.
 */
void BaseMainWindow::slotPlayAudio()
{
  m_impl->slotPlayAudio();
}
#endif

/**
 * Change status message.
 *
 * @param text message
 */
void BaseMainWindow::slotStatusMsg(const QString& text)
{
  m_impl->slotStatusMsg(text);
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
 * @param dir directory or file path
 */
void BaseMainWindow::confirmedOpenDirectory(const QString& dir)
{
  m_impl->confirmedOpenDirectory(dir);
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
