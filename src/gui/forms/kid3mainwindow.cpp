/**
 * \file kid3mainwindow.cpp
 * Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#include "kid3mainwindow.h"
#include <QDir>
#include <QCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include "qtcompatmac.h"
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QIcon>
#include <QToolBar>
#include "config.h"

#ifdef CONFIG_USE_KDE
#include <kapplication.h>
#include <kurl.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kshortcutsdialog.h>
#include <krecentfilesaction.h>
#include <kactioncollection.h>
#include <kedittoolbar.h>
#include <kconfigskeleton.h>
#else
#include <QApplication>
#include <QMenuBar>
#include <QAction>
#include <QStatusBar>
#include <QFileDialog>
#include "recentfilesmenu.h"
#include "messagedialog.h"
#include "shortcutsmodel.h"
#endif

#include "kid3form.h"
#include "kid3application.h"
#include "framelist.h"
#include "frametablemodel.h"
#include "frametable.h"
#include "configdialog.h"
#include "importdialog.h"
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
#include "modeliterator.h"
#include "filelist.h"
#include "dirlist.h"
#include "pictureframe.h"
#include "configstore.h"
#include "contexthelp.h"
#include "frame.h"
#include "textexporter.h"
#include "serverimporter.h"
#include "dirrenamer.h"
#include "movetotrash.h"
#include "qtcompatmac.h"
#include "saferename.h"
#ifdef HAVE_PHONON
#include "audioplayer.h"
#endif
#ifdef HAVE_PHONON
#include "playtoolbar.h"
#endif

/**
 * Constructor.
 */
Kid3MainWindow::Kid3MainWindow() :
  m_app(new Kid3Application(this)),
  m_importDialog(0), m_browseCoverArtDialog(0),
  m_exportDialog(0), m_renDirDialog(0),
  m_numberTracksDialog(0), m_filterDialog(0),
  m_downloadDialog(new DownloadDialog(this, i18n("Download"))),
  m_playlistDialog(0)
#ifdef HAVE_PHONON
  , m_playToolBar(0)
#endif
{
  DownloadClient* downloadClient = m_app->getDownloadClient();
  connect(downloadClient, SIGNAL(progress(QString,int,int)),
          m_downloadDialog, SLOT(updateProgressStatus(QString,int,int)));
  connect(downloadClient, SIGNAL(downloadStarted(QString)),
          m_downloadDialog, SLOT(showStartOfDownload(QString)));
  connect(downloadClient, SIGNAL(aborted()),
          m_downloadDialog, SLOT(reset()));
  connect(m_downloadDialog, SIGNAL(canceled()),
          downloadClient, SLOT(cancelDownload()));
  connect(downloadClient, SIGNAL(downloadFinished(const QByteArray&, const QString&, const QString&)),
          m_app, SLOT(imageDownloaded(const QByteArray&, const QString&, const QString&)));

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
#ifdef HAVE_PHONON
  connect(m_app, SIGNAL(aboutToPlayAudio()), this, SLOT(showPlayToolBar()));
#endif

#ifndef CONFIG_USE_KDE
#if !defined _WIN32 && !defined WIN32 && defined CFG_DATAROOTDIR
  QPixmap icon;
  if (icon.load(QString(CFG_DATAROOTDIR) +
#ifndef __APPLE__
                "/icons/hicolor/48x48/apps/kid3-qt.png"
#else
                "/kid3.png"
#endif
        )) {
    setWindowIcon(icon);
  }
#endif
  readFontAndStyleOptions();
#endif

  statusBar()->showMessage(i18n("Ready."));
  m_form = new Kid3Form(m_app, this);
  setCentralWidget(m_form);

  initActions();

  resize(sizeHint());

  readOptions();
}

/**
 * Destructor.
 */
Kid3MainWindow::~Kid3MainWindow()
{
  delete m_importDialog;
  delete m_renDirDialog;
  delete m_numberTracksDialog;
  delete m_filterDialog;
  delete m_browseCoverArtDialog;
  delete m_playlistDialog;
#ifdef HAVE_PHONON
  delete m_playToolBar;
#endif
}

/** Only defined for generation of translation files */
#define MAIN_TOOLBAR_FOR_PO I18N_NOOP("Main Toolbar")

/**
 * Init menu and toolbar actions.
 */
void Kid3MainWindow::initActions()
{
#ifdef CONFIG_USE_KDE
  KAction* fileOpen = KStandardAction::open(
      this, SLOT(slotFileOpen()), actionCollection());
  m_fileOpenRecent = KStandardAction::openRecent(
      this,
      SLOT(slotFileOpenRecentUrl(const KUrl&)),
      actionCollection());
  KAction* fileRevert = KStandardAction::revert(
      m_app, SLOT(revertFileModifications()), actionCollection());
  KAction* fileSave = KStandardAction::save(
      this, SLOT(slotFileSave()), actionCollection());
  KAction* fileQuit = KStandardAction::quit(
      this, SLOT(slotFileQuit()), actionCollection());
  KAction* editSelectAll = KStandardAction::selectAll(
      m_form, SLOT(selectAllFiles()), actionCollection());
  KAction* editDeselect = KStandardAction::deselect(
      m_form, SLOT(deselectAllFiles()), actionCollection());
  setStandardToolBarMenuEnabled(true);
  createStandardStatusBarAction();
  KAction* settingsShortcuts = KStandardAction::keyBindings(
    this, SLOT(slotSettingsShortcuts()), actionCollection());
  KAction* settingsToolbars = KStandardAction::configureToolbars(
    this, SLOT(slotSettingsToolbars()), actionCollection());
  KAction* settingsConfigure = KStandardAction::preferences(
      this, SLOT(slotSettingsConfigure()), actionCollection());

  fileOpen->setStatusTip(i18n("Opens a directory"));
  m_fileOpenRecent->setStatusTip(i18n("Opens a recently used directory"));
  fileRevert->setStatusTip(
      i18n("Reverts the changes of all or the selected files"));
  fileSave->setStatusTip(i18n("Saves the changed files"));
  fileQuit->setStatusTip(i18n("Quits the application"));
  editSelectAll->setStatusTip(i18n("Select all files"));
  editSelectAll->setShortcut(KShortcut("Alt+Shift+A"));
  editDeselect->setStatusTip(i18n("Deselect all files"));
  settingsShortcuts->setStatusTip(i18n("Configure Shortcuts"));
  settingsToolbars->setStatusTip(i18n("Configure Toolbars"));
  settingsConfigure->setStatusTip(i18n("Preferences dialog"));

  KAction* fileOpenDirectory = new KAction(KIcon("document-open"), i18n("O&pen Directory..."), this);
  fileOpenDirectory->setShortcut(KShortcut("Ctrl+D"));
  actionCollection()->addAction("open_directory", fileOpenDirectory);
  connect(fileOpenDirectory, SIGNAL(triggered()), this, SLOT(slotFileOpenDirectory()));
  KAction* fileImport = new KAction(KIcon("document-import"), i18n("&Import..."), this);
  fileImport->setData(-1);
  actionCollection()->addAction("import", fileImport);
  connect(fileImport, SIGNAL(triggered()), this, SLOT(slotImport()));

  int importerIdx = 0;
  foreach (const ServerImporter* si, m_app->getServerImporters()) {
    QString serverName(QCM_translate(si->name()));
    QString actionName = QString(si->name()).toLower().remove(' ');
    int dotPos = actionName.indexOf('.');
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QString("import_") + actionName;
    KAction* fileImportServer =
        new KAction(i18n("Import from %1...").arg(serverName), this);
    fileImportServer->setData(importerIdx);
    actionCollection()->addAction(actionName, fileImportServer);
    connect(fileImportServer, SIGNAL(triggered()), this, SLOT(slotImport()));
    ++importerIdx;
  }
#ifdef HAVE_CHROMAPRINT
  QString serverName(i18n("MusicBrainz Fingerprint"));
  KAction* fileImportMusicBrainz =
      new KAction(i18n("Import from %1...").arg(serverName), this);
  fileImportMusicBrainz->setData(importerIdx);
  actionCollection()->addAction("import_musicbrainz", fileImportMusicBrainz);
  connect(fileImportMusicBrainz, SIGNAL(triggered()), this, SLOT(slotImport()));
  ++importerIdx;
#endif

  KAction* fileBrowseCoverArt = new KAction(i18n("&Browse Cover Art..."), this);
  actionCollection()->addAction("browse_cover_art", fileBrowseCoverArt);
  connect(fileBrowseCoverArt, SIGNAL(triggered()), this, SLOT(slotBrowseCoverArt()));
  KAction* fileExport = new KAction(KIcon("document-export"), i18n("&Export..."), this);
  actionCollection()->addAction("export", fileExport);
  connect(fileExport, SIGNAL(triggered()), this, SLOT(slotExport()));
  KAction* fileCreatePlaylist = new KAction(KIcon("view-media-playlist"), i18n("&Create Playlist..."), this);
  actionCollection()->addAction("create_playlist", fileCreatePlaylist);
  connect(fileCreatePlaylist, SIGNAL(triggered()), this, SLOT(slotPlaylistDialog()));
  KAction* toolsApplyFilenameFormat = new KAction(i18n("Apply &Filename Format"), this);
  actionCollection()->addAction("apply_filename_format", toolsApplyFilenameFormat);
  connect(toolsApplyFilenameFormat, SIGNAL(triggered()), m_app, SLOT(applyFilenameFormat()));
  KAction* toolsApplyId3Format = new KAction(i18n("Apply &Tag Format"), this);
  actionCollection()->addAction("apply_id3_format", toolsApplyId3Format);
  connect(toolsApplyId3Format, SIGNAL(triggered()), m_app, SLOT(applyId3Format()));
  KAction* toolsRenameDirectory = new KAction(i18n("&Rename Directory..."), this);
  actionCollection()->addAction("rename_directory", toolsRenameDirectory);
  connect(toolsRenameDirectory, SIGNAL(triggered()), this, SLOT(slotRenameDirectory()));
  KAction* toolsNumberTracks = new KAction(i18n("&Number Tracks..."), this);
  actionCollection()->addAction("number_tracks", toolsNumberTracks);
  connect(toolsNumberTracks, SIGNAL(triggered()), this, SLOT(slotNumberTracks()));
  KAction* toolsFilter = new KAction(i18n("F&ilter..."), this);
  actionCollection()->addAction("filter", toolsFilter);
  connect(toolsFilter, SIGNAL(triggered()), this, SLOT(slotFilter()));
#ifdef HAVE_TAGLIB
  KAction* toolsConvertToId3v24 = new KAction(i18n("Convert ID3v2.3 to ID3v2.&4"), this);
  actionCollection()->addAction("convert_to_id3v24", toolsConvertToId3v24);
  connect(toolsConvertToId3v24, SIGNAL(triggered()), m_app, SLOT(convertToId3v24()));
#endif
#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  KAction* toolsConvertToId3v23 = new KAction(i18n("Convert ID3v2.4 to ID3v2.&3"), this);
  actionCollection()->addAction("convert_to_id3v23", toolsConvertToId3v23);
  connect(toolsConvertToId3v23, SIGNAL(triggered()), m_app, SLOT(convertToId3v23()));
#endif
#ifdef HAVE_PHONON
  KAction* toolsPlay = new KAction(KIcon("media-playback-start"), i18n("&Play"), this);
  actionCollection()->addAction("play", toolsPlay);
  connect(toolsPlay, SIGNAL(triggered()), m_app, SLOT(playAudio()));
#endif
  m_settingsShowHidePicture = new KToggleAction(i18n("Show &Picture"), this);
  m_settingsShowHidePicture->setCheckable(true);
  actionCollection()->addAction("hide_picture", m_settingsShowHidePicture);
  connect(m_settingsShowHidePicture, SIGNAL(triggered()), this, SLOT(slotSettingsShowHidePicture()));
  m_settingsAutoHideTags = new KToggleAction(i18n("Auto &Hide Tags"), this);
  m_settingsAutoHideTags->setCheckable(true);
  actionCollection()->addAction("auto_hide_tags", m_settingsAutoHideTags);
  connect(m_settingsAutoHideTags, SIGNAL(triggered()), this, SLOT(slotSettingsAutoHideTags()));
  KAction* editPreviousFile = new KAction(KIcon("go-previous"), i18n("&Previous File"), this);
  editPreviousFile->setShortcut(KShortcut("Alt+Up"));
  actionCollection()->addAction("previous_file", editPreviousFile);
  connect(editPreviousFile, SIGNAL(triggered()), m_app, SLOT(previousFile()));
  KAction* editNextFile = new KAction(KIcon("go-next"), i18n("&Next File"), this);
  editNextFile->setShortcut(KShortcut("Alt+Down"));
  actionCollection()->addAction("next_file", editNextFile);
  connect(editNextFile, SIGNAL(triggered()), m_app, SLOT(nextFile()));
  KAction* actionV1FromFilename = new KAction(i18n("Tag 1") + ": " + i18n("From Filename"), this);
  actionCollection()->addAction("v1_from_filename", actionV1FromFilename);
  connect(actionV1FromFilename, SIGNAL(triggered()), m_app, SLOT(getTagsFromFilenameV1()));
  KAction* actionV1FromV2 = new KAction(i18n("Tag 1") + ": " + i18n("From Tag 2"), this);
  actionCollection()->addAction("v1_from_v2", actionV1FromV2);
  connect(actionV1FromV2, SIGNAL(triggered()), m_app, SLOT(copyV2ToV1()));
  KAction* actionV1Copy = new KAction(i18n("Tag 1") + ": " + i18n("Copy"), this);
  actionCollection()->addAction("v1_copy", actionV1Copy);
  connect(actionV1Copy, SIGNAL(triggered()), m_app, SLOT(copyTagsV1()));
  KAction* actionV1Paste = new KAction(i18n("Tag 1") + ": " + i18n("Paste"), this);
  actionCollection()->addAction("v1_paste", actionV1Paste);
  connect(actionV1Paste, SIGNAL(triggered()), m_app, SLOT(pasteTagsV1()));
  KAction* actionV1Remove = new KAction(i18n("Tag 1") + ": " + i18n("Remove"), this);
  actionCollection()->addAction("v1_remove", actionV1Remove);
  connect(actionV1Remove, SIGNAL(triggered()), m_app, SLOT(removeTagsV1()));
  KAction* actionV2FromFilename = new KAction(i18n("Tag 2") + ": " + i18n("From Filename"), this);
  actionCollection()->addAction("v2_from_filename", actionV2FromFilename);
  connect(actionV2FromFilename, SIGNAL(triggered()), m_app, SLOT(getTagsFromFilenameV2()));
  KAction* actionV2FromV1 = new KAction(i18n("Tag 2") + ": " + i18n("From Tag 1"), this);
  actionCollection()->addAction("v2_from_v1", actionV2FromV1);
  connect(actionV2FromV1, SIGNAL(triggered()), m_app, SLOT(copyV1ToV2()));
  KAction* actionV2Copy = new KAction(i18n("Tag 2") + ": " + i18n("Copy"), this);
  actionCollection()->addAction("v2_copy", actionV2Copy);
  connect(actionV2Copy, SIGNAL(triggered()), m_app, SLOT(copyTagsV2()));
  KAction* actionV2Paste = new KAction(i18n("Tag 2") + ": " + i18n("Paste"), this);
  actionCollection()->addAction("v2_paste", actionV2Paste);
  connect(actionV2Paste, SIGNAL(triggered()), m_app, SLOT(pasteTagsV2()));
  KAction* actionV2Remove = new KAction(i18n("Tag 2") + ": " + i18n("Remove"), this);
  actionCollection()->addAction("v2_remove", actionV2Remove);
  connect(actionV2Remove, SIGNAL(triggered()), m_app, SLOT(removeTagsV2()));
  KAction* actionFramesEdit = new KAction(i18n("Frames:") + " " + i18n("Edit"), this);
  actionCollection()->addAction("frames_edit", actionFramesEdit);
  connect(actionFramesEdit, SIGNAL(triggered()), m_form, SLOT(editFrame()));
  KAction* actionFramesAdd = new KAction(i18n("Frames:") + " " + i18n("Add"), this);
  actionCollection()->addAction("frames_add", actionFramesAdd);
  connect(actionFramesAdd, SIGNAL(triggered()), m_form, SLOT(addFrame()));
  KAction* actionFramesDelete = new KAction(i18n("Frames:") + " " + i18n("Delete"), this);
  actionCollection()->addAction("frames_delete", actionFramesDelete);
  connect(actionFramesDelete, SIGNAL(triggered()), m_form, SLOT(deleteFrame()));
  KAction* actionFilenameFromV1 = new KAction(i18n("Filename") + ": " + i18n("From Tag 1"), this);
  actionCollection()->addAction("filename_from_v1", actionFilenameFromV1);
  connect(actionFilenameFromV1, SIGNAL(triggered()), m_form, SLOT(fnFromID3V1()));
  KAction* actionFilenameFromV2 = new KAction(i18n("Filename") + ": " + i18n("From Tag 2"), this);
  actionCollection()->addAction("filename_from_v2", actionFilenameFromV2);
  connect(actionFilenameFromV2, SIGNAL(triggered()), m_form, SLOT(fnFromID3V2()));
  KAction* actionFilenameFocus = new KAction(i18n("Filename") + ": " + i18n("Focus"), this);
  actionCollection()->addAction("filename_focus", actionFilenameFocus);
  connect(actionFilenameFocus, SIGNAL(triggered()), m_form, SLOT(setFocusFilename()));
  KAction* actionV1Focus = new KAction(i18n("Tag 1") + ": " + i18n("Focus"), this);
  actionCollection()->addAction("v1_focus", actionV1Focus);
  connect(actionV1Focus, SIGNAL(triggered()), m_form, SLOT(setFocusV1()));
  KAction* actionV2Focus = new KAction(i18n("Tag 2") + ": " + i18n("Focus"), this);
  actionCollection()->addAction("v2_focus", actionV2Focus);
  connect(actionV2Focus, SIGNAL(triggered()), m_form, SLOT(setFocusV2()));
  KAction* actionFileListFocus = new KAction(i18n("File List") + ": " + i18n("Focus"), this);
  actionCollection()->addAction("filelist_focus", actionFileListFocus);
  connect(actionFileListFocus, SIGNAL(triggered()), m_form, SLOT(setFocusFileList()));
  KAction* actionDirListFocus = new KAction(i18n("Directory List") + ": " + i18n("Focus"), this);
  actionCollection()->addAction("dirlist_focus", actionDirListFocus);
  connect(actionDirListFocus, SIGNAL(triggered()), m_form, SLOT(setFocusDirList()));
  createGUI();

#else
  QToolBar* toolBar = new QToolBar(this);
  toolBar->setObjectName("MainToolbar");
  QMenuBar* menubar = menuBar();
  QString menuTitle(i18n("&File"));
  ShortcutsModel* shortcutsModel = m_app->getConfigStore()->getShortcutsModel();
  QMenu* fileMenu = menubar->addMenu(menuTitle);

  QAction* fileOpen = new QAction(this);
  fileOpen->setStatusTip(i18n("Opens a directory"));
  fileOpen->setText(i18n("&Open..."));
  fileOpen->setShortcut(QKeySequence::Open);
  fileOpen->setIcon(QCM_QIcon_fromTheme("document-open"));
  fileOpen->setObjectName("file_open");
  shortcutsModel->registerAction(fileOpen, menuTitle);
  connect(fileOpen, SIGNAL(triggered()),
    this, SLOT(slotFileOpen()));
  fileMenu->addAction(fileOpen);
  toolBar->addAction(fileOpen);

  m_fileOpenRecent = new RecentFilesMenu(fileMenu);
  connect(m_fileOpenRecent, SIGNAL(loadFile(const QString&)),
          this, SLOT(slotFileOpenRecentDirectory(const QString&)));
  m_fileOpenRecent->setStatusTip(i18n("Opens a recently used directory"));
  m_fileOpenRecent->setTitle(i18n("Open &Recent"));
  m_fileOpenRecent->setIcon(QCM_QIcon_fromTheme("document-open-recent"));
  fileMenu->addMenu(m_fileOpenRecent);

  QAction* fileOpenDirectory = new QAction(this);
  fileOpenDirectory->setStatusTip(i18n("Opens a directory"));
  fileOpenDirectory->setText(i18n("O&pen Directory..."));
  fileOpenDirectory->setShortcut(Qt::CTRL + Qt::Key_D);
  fileOpenDirectory->setIcon(QCM_QIcon_fromTheme("document-open"));
  fileOpenDirectory->setObjectName("open_directory");
  shortcutsModel->registerAction(fileOpenDirectory, menuTitle);
  connect(fileOpenDirectory, SIGNAL(triggered()),
    this, SLOT(slotFileOpenDirectory()));
  fileMenu->addAction(fileOpenDirectory);
  fileMenu->addSeparator();

  QAction* fileSave = new QAction(this);
  fileSave->setStatusTip(i18n("Saves the changed files"));
  fileSave->setText(i18n("&Save"));
  fileSave->setShortcut(QKeySequence::Save);
  fileSave->setIcon(QCM_QIcon_fromTheme("document-save"));
  fileSave->setObjectName("file_save");
  shortcutsModel->registerAction(fileSave, menuTitle);
  connect(fileSave, SIGNAL(triggered()),
    this, SLOT(slotFileSave()));
  fileMenu->addAction(fileSave);
  toolBar->addAction(fileSave);

  QAction* fileRevert = new QAction(this);
  fileRevert->setStatusTip(
      i18n("Reverts the changes of all or the selected files"));
  fileRevert->setText(i18n("Re&vert"));
  fileRevert->setIcon(QCM_QIcon_fromTheme("document-revert"));
  fileRevert->setObjectName("file_revert");
  shortcutsModel->registerAction(fileRevert, menuTitle);
  connect(fileRevert, SIGNAL(triggered()),
    m_app, SLOT(revertFileModifications()));
  fileMenu->addAction(fileRevert);
  toolBar->addAction(fileRevert);
  fileMenu->addSeparator();

  QAction* fileImport = new QAction(this);
  fileImport->setData(-1);
  fileImport->setStatusTip(i18n("Import from file or clipboard"));
  fileImport->setText(i18n("&Import..."));
  fileImport->setIcon(QCM_QIcon_fromTheme("document-import"));
  fileImport->setObjectName("import");
  shortcutsModel->registerAction(fileImport, menuTitle);
  connect(fileImport, SIGNAL(triggered()),
    this, SLOT(slotImport()));
  fileMenu->addAction(fileImport);

  int importerIdx = 0;
  foreach (const ServerImporter* si, m_app->getServerImporters()) {
    QString serverName(QCM_translate(si->name()));
    QString actionName = QString(si->name()).toLower().remove(' ');
    int dotPos = actionName.indexOf('.');
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QString("import_") + actionName;
    QAction* fileImportServer = new QAction(this);
    fileImportServer->setData(importerIdx);
    fileImportServer->setStatusTip(i18n("Import from %1").arg(serverName));
    fileImportServer->setText(i18n("Import from %1...").arg(serverName));
    fileImportServer->setObjectName(actionName);
    shortcutsModel->registerAction(fileImportServer, menuTitle);
    connect(fileImportServer, SIGNAL(triggered()),
      this, SLOT(slotImport()));
    fileMenu->addAction(fileImportServer);
    ++importerIdx;
  }
#ifdef HAVE_CHROMAPRINT
  QAction* fileImportMusicBrainz = new QAction(this);
  QString serverName(i18n("MusicBrainz Fingerprint"));
  fileImportMusicBrainz->setData(importerIdx);
  fileImportMusicBrainz->setStatusTip(i18n("Import from %1").arg(serverName));
  fileImportMusicBrainz->setText(i18n("Import from %1...").arg(serverName));
  fileImportMusicBrainz->setObjectName("import_musicbrainz");
  shortcutsModel->registerAction(fileImportMusicBrainz, menuTitle);
  connect(fileImportMusicBrainz, SIGNAL(triggered()),
    this, SLOT(slotImport()));
  fileMenu->addAction(fileImportMusicBrainz);
  ++importerIdx;
#endif

  QAction* fileBrowseCoverArt = new QAction(this);
  fileBrowseCoverArt->setStatusTip(i18n("Browse album cover artwork"));
  fileBrowseCoverArt->setText(i18n("&Browse Cover Art..."));
  fileBrowseCoverArt->setObjectName("browse_cover_art");
  shortcutsModel->registerAction(fileBrowseCoverArt, menuTitle);
  connect(fileBrowseCoverArt, SIGNAL(triggered()),
    this, SLOT(slotBrowseCoverArt()));
  fileMenu->addAction(fileBrowseCoverArt);

  QAction* fileExport = new QAction(this);
  fileExport->setStatusTip(i18n("Export to file or clipboard"));
  fileExport->setText(i18n("&Export..."));
  fileExport->setIcon(QCM_QIcon_fromTheme("document-export"));
  fileExport->setObjectName("export");
  shortcutsModel->registerAction(fileExport, menuTitle);
  connect(fileExport, SIGNAL(triggered()),
    this, SLOT(slotExport()));
  fileMenu->addAction(fileExport);

  QAction* fileCreatePlaylist = new QAction(this);
  fileCreatePlaylist->setStatusTip(i18n("Create M3U Playlist"));
  fileCreatePlaylist->setText(i18n("&Create Playlist..."));
  fileCreatePlaylist->setIcon(QIcon(":/images/view-media-playlist.png"));
  fileCreatePlaylist->setObjectName("create_playlist");
  shortcutsModel->registerAction(fileCreatePlaylist, menuTitle);
  connect(fileCreatePlaylist, SIGNAL(triggered()),
    this, SLOT(slotPlaylistDialog()));
  fileMenu->addAction(fileCreatePlaylist);
  toolBar->addAction(fileCreatePlaylist);
  fileMenu->addSeparator();

  QAction* fileQuit = new QAction(this);
  fileQuit->setStatusTip(i18n("Quits the application"));
  fileQuit->setText(i18n("&Quit"));
  fileQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
  fileQuit->setIcon(QCM_QIcon_fromTheme("application-exit"));
  fileQuit->setObjectName("file_quit");
  shortcutsModel->registerAction(fileQuit, menuTitle);
  connect(fileQuit, SIGNAL(triggered()),
    this, SLOT(slotFileQuit()));
  fileMenu->addAction(fileQuit);

  menuTitle = i18n("&Edit");
  QMenu* editMenu = menubar->addMenu(menuTitle);
  QAction* editSelectAll = new QAction(this);
  editSelectAll->setStatusTip(i18n("Select all files"));
  editSelectAll->setText(i18n("Select &All"));
  editSelectAll->setShortcut(Qt::ALT + Qt::Key_A);
  editSelectAll->setIcon(QCM_QIcon_fromTheme("edit-select-all"));
  editSelectAll->setObjectName("edit_select_all");
  shortcutsModel->registerAction(editSelectAll, menuTitle);
  connect(editSelectAll, SIGNAL(triggered()),
    m_form, SLOT(selectAllFiles()));
  editMenu->addAction(editSelectAll);

  QAction* editDeselect = new QAction(this);
  editDeselect->setStatusTip(i18n("Deselect all files"));
  editDeselect->setText(i18n("Dese&lect"));
  editDeselect->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);
  editDeselect->setObjectName("edit_deselect");
  shortcutsModel->registerAction(editDeselect, menuTitle);
  connect(editDeselect, SIGNAL(triggered()),
    m_form, SLOT(deselectAllFiles()));
  editMenu->addAction(editDeselect);

  QAction* editPreviousFile = new QAction(this);
  editPreviousFile->setStatusTip(i18n("Select previous file"));
  editPreviousFile->setText(i18n("&Previous File"));
  editPreviousFile->setShortcut(Qt::ALT + Qt::Key_Up);
  editPreviousFile->setIcon(QCM_QIcon_fromTheme("go-previous"));
  editPreviousFile->setObjectName("previous_file");
  shortcutsModel->registerAction(editPreviousFile, menuTitle);
  connect(editPreviousFile, SIGNAL(triggered()),
    m_app, SLOT(previousFile()));
  editMenu->addAction(editPreviousFile);
  toolBar->addAction(editPreviousFile);

  QAction* editNextFile = new QAction(this);
  editNextFile->setStatusTip(i18n("Select next file"));
  editNextFile->setText(i18n("&Next File"));
  editNextFile->setShortcut(Qt::ALT + Qt::Key_Down);
  editNextFile->setIcon(QCM_QIcon_fromTheme("go-next"));
  editNextFile->setObjectName("next_file");
  shortcutsModel->registerAction(editNextFile, menuTitle);
  connect(editNextFile, SIGNAL(triggered()),
    m_app, SLOT(nextFile()));
  editMenu->addAction(editNextFile);
  toolBar->addAction(editNextFile);

  menuTitle = i18n("&Tools");
  QMenu* toolsMenu = menubar->addMenu(menuTitle);
  QAction* toolsApplyFilenameFormat = new QAction(this);
  toolsApplyFilenameFormat->setStatusTip(i18n("Apply Filename Format"));
  toolsApplyFilenameFormat->setText(i18n("Apply &Filename Format"));
  toolsApplyFilenameFormat->setObjectName("apply_filename_format");
  shortcutsModel->registerAction(toolsApplyFilenameFormat, menuTitle);
  connect(toolsApplyFilenameFormat, SIGNAL(triggered()),
    m_app, SLOT(applyFilenameFormat()));
  toolsMenu->addAction(toolsApplyFilenameFormat);

  QAction* toolsApplyId3Format = new QAction(this);
  toolsApplyId3Format->setStatusTip(i18n("Apply Tag Format"));
  toolsApplyId3Format->setText(i18n("Apply &Tag Format"));
  toolsApplyId3Format->setObjectName("apply_id3_format");
  shortcutsModel->registerAction(toolsApplyId3Format, menuTitle);
  connect(toolsApplyId3Format, SIGNAL(triggered()),
    m_app, SLOT(applyId3Format()));
  toolsMenu->addAction(toolsApplyId3Format);

  QAction* toolsRenameDirectory = new QAction(this);
  toolsRenameDirectory->setStatusTip(i18n("Rename Directory"));
  toolsRenameDirectory->setText(i18n("&Rename Directory..."));
  toolsRenameDirectory->setObjectName("rename_directory");
  shortcutsModel->registerAction(toolsRenameDirectory, menuTitle);
  connect(toolsRenameDirectory, SIGNAL(triggered()),
    this, SLOT(slotRenameDirectory()));
  toolsMenu->addAction(toolsRenameDirectory);

  QAction* toolsNumberTracks = new QAction(this);
  toolsNumberTracks->setStatusTip(i18n("Number Tracks"));
  toolsNumberTracks->setText(i18n("&Number Tracks..."));
  toolsNumberTracks->setObjectName("number_tracks");
  shortcutsModel->registerAction(toolsNumberTracks, menuTitle);
  connect(toolsNumberTracks, SIGNAL(triggered()),
    this, SLOT(slotNumberTracks()));
  toolsMenu->addAction(toolsNumberTracks);

  QAction* toolsFilter = new QAction(this);
  toolsFilter->setStatusTip(i18n("Filter"));
  toolsFilter->setText(i18n("F&ilter..."));
  toolsFilter->setObjectName("filter");
  shortcutsModel->registerAction(toolsFilter, menuTitle);
  connect(toolsFilter, SIGNAL(triggered()),
    this, SLOT(slotFilter()));
  toolsMenu->addAction(toolsFilter);

#ifdef HAVE_TAGLIB
  QAction* toolsConvertToId3v24 = new QAction(this);
  toolsConvertToId3v24->setStatusTip(i18n("Convert ID3v2.3 to ID3v2.4"));
  toolsConvertToId3v24->setText(i18n("Convert ID3v2.3 to ID3v2.&4"));
  toolsConvertToId3v24->setObjectName("convert_to_id3v24");
  shortcutsModel->registerAction(toolsConvertToId3v24, menuTitle);
  connect(toolsConvertToId3v24, SIGNAL(triggered()),
    m_app, SLOT(convertToId3v24()));
  toolsMenu->addAction(toolsConvertToId3v24);
#endif

#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  QAction* toolsConvertToId3v23 = new QAction(this);
  toolsConvertToId3v23->setStatusTip(i18n("Convert ID3v2.4 to ID3v2.3"));
  toolsConvertToId3v23->setText(i18n("Convert ID3v2.4 to ID3v2.&3"));
  toolsConvertToId3v23->setObjectName("convert_to_id3v23");
  shortcutsModel->registerAction(toolsConvertToId3v23, menuTitle);
  connect(toolsConvertToId3v23, SIGNAL(triggered()),
    m_app, SLOT(convertToId3v23()));
  toolsMenu->addAction(toolsConvertToId3v23);
#endif

#ifdef HAVE_PHONON
  QAction* toolsPlay = new QAction(this);
  toolsPlay->setStatusTip(i18n("Play"));
  toolsPlay->setText(i18n("&Play"));
  toolsPlay->setIcon(QIcon(style()->standardIcon(QStyle::SP_MediaPlay)));
  toolsPlay->setObjectName("play");
  shortcutsModel->registerAction(toolsPlay, menuTitle);
  connect(toolsPlay, SIGNAL(triggered()),
    m_app, SLOT(playAudio()));
  toolsMenu->addAction(toolsPlay);
  toolBar->addAction(toolsPlay);
#endif

  menuTitle = i18n("&Settings");
  QMenu* settingsMenu = menubar->addMenu(menuTitle);
  m_viewToolBar = toolBar->toggleViewAction();
  if (m_viewToolBar) {
    m_viewToolBar->setStatusTip(i18n("Enables/disables the toolbar"));
    m_viewToolBar->setText(i18n("Show &Toolbar"));
    m_viewToolBar->setObjectName("options_configure_toolbars");
    shortcutsModel->registerAction(m_viewToolBar, menuTitle);
  }
  if (ConfigStore::s_miscCfg.m_hideToolBar)
    toolBar->hide();
  m_viewToolBar->setChecked(!ConfigStore::s_miscCfg.m_hideToolBar);
  settingsMenu->addAction(m_viewToolBar);

  m_viewStatusBar = new QAction(this);
  m_viewStatusBar->setStatusTip(i18n("Enables/disables the statusbar"));
  m_viewStatusBar->setText(i18n("Show St&atusbar"));
  m_viewStatusBar->setCheckable(true);
  m_viewStatusBar->setObjectName("options_show_statusbar");
  shortcutsModel->registerAction(m_viewStatusBar, menuTitle);
  connect(m_viewStatusBar, SIGNAL(triggered()),
    this, SLOT(slotViewStatusBar()));
  settingsMenu->addAction(m_viewStatusBar);

  m_settingsShowHidePicture = new QAction(this);
  m_settingsShowHidePicture->setStatusTip(i18n("Show Picture"));
  m_settingsShowHidePicture->setText(i18n("Show &Picture"));
  m_settingsShowHidePicture->setCheckable(true);
  m_settingsShowHidePicture->setObjectName("hide_picture");
  shortcutsModel->registerAction(m_settingsShowHidePicture, menuTitle);
  connect(m_settingsShowHidePicture, SIGNAL(triggered()),
    this, SLOT(slotSettingsShowHidePicture()));
  settingsMenu->addAction(m_settingsShowHidePicture);

  m_settingsAutoHideTags = new QAction(this);
  m_settingsAutoHideTags->setStatusTip(i18n("Auto Hide Tags"));
  m_settingsAutoHideTags->setText(i18n("Auto &Hide Tags"));
  m_settingsAutoHideTags->setCheckable(true);
  m_settingsAutoHideTags->setObjectName("auto_hide_tags");
  shortcutsModel->registerAction(m_settingsAutoHideTags, menuTitle);
  connect(m_settingsAutoHideTags, SIGNAL(triggered()),
    this, SLOT(slotSettingsAutoHideTags()));
  settingsMenu->addAction(m_settingsAutoHideTags);

  QAction* settingsConfigure = new QAction(this);
  settingsConfigure->setStatusTip(i18n("Configure Kid3"));
  settingsConfigure->setText(i18n("&Configure Kid3..."));
  settingsConfigure->setIcon(QCM_QIcon_fromTheme("preferences-system"));
#if QT_VERSION >= 0x040600
  settingsConfigure->setShortcut(QKeySequence::Preferences);
#endif
  settingsConfigure->setObjectName("options_configure");
  shortcutsModel->registerAction(settingsConfigure, menuTitle);
  connect(settingsConfigure, SIGNAL(triggered()),
    this, SLOT(slotSettingsConfigure()));
  settingsMenu->addSeparator();
  settingsMenu->addAction(settingsConfigure);
  toolBar->addAction(settingsConfigure);

  menuTitle = i18n("&Help");
  QMenu* helpMenu = menubar->addMenu(menuTitle);
  QAction* helpHandbook = new QAction(this);
  helpHandbook->setStatusTip(i18n("Kid3 Handbook"));
  helpHandbook->setText(i18n("Kid3 &Handbook"));
  helpHandbook->setIcon(QCM_QIcon_fromTheme("help-contents"));
  helpHandbook->setShortcut(QKeySequence::HelpContents);
  helpHandbook->setObjectName("help_contents");
  shortcutsModel->registerAction(helpHandbook, menuTitle);
  connect(helpHandbook, SIGNAL(triggered()),
    this, SLOT(slotHelpHandbook()));
  helpMenu->addAction(helpHandbook);

  QAction* helpAbout = new QAction(this);
  helpAbout->setStatusTip(i18n("About Kid3"));
  helpAbout->setText(i18n("&About Kid3"));
  helpAbout->setObjectName("help_about_app");
  shortcutsModel->registerAction(helpAbout, menuTitle);
  connect(helpAbout, SIGNAL(triggered()),
    this, SLOT(slotHelpAbout()));
  helpMenu->addAction(helpAbout);

  QAction* helpAboutQt = new QAction(this);
  helpAboutQt->setStatusTip(i18n("About Qt"));
  helpAboutQt->setText(i18n("About &Qt"));
  helpAboutQt->setObjectName("help_about_qt");
  shortcutsModel->registerAction(helpAboutQt, menuTitle);
  connect(helpAboutQt, SIGNAL(triggered()),
    this, SLOT(slotHelpAboutQt()));
  helpMenu->addAction(helpAboutQt);

  addToolBar(toolBar);

  updateWindowCaption();

  m_form->initActions();
#endif
}

/**
 * Update the recent file list and the caption when a new directory
 * is opened.
 */
void Kid3MainWindow::onDirectoryOpened()
{
#ifdef CONFIG_USE_KDE
  KUrl url;
  url.setPath(m_app->getDirName());
  m_fileOpenRecent->addUrl(url);
#else
  m_fileOpenRecent->addDirectory(m_app->getDirName());
#endif
  updateWindowCaption();
}

/**
 * Open directory, user has to confirm if current directory modified.
 *
 * @param dir directory or file path
 */
void Kid3MainWindow::confirmedOpenDirectory(const QString& dir)
{
  if (!saveModified()) {
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  slotStatusMsg(i18n("Opening directory..."));

  m_app->openDirectory(dir, false);

  slotStatusMsg(i18n("Ready."));
  QApplication::restoreOverrideCursor();
}

/**
 * Save application options.
 */
void Kid3MainWindow::saveOptions()
{
#ifdef CONFIG_USE_KDE
  m_fileOpenRecent->saveEntries(KConfigGroup(m_app->getSettings(),
                                             "Recent Files"));
#else
  m_fileOpenRecent->saveEntries(m_app->getSettings());
  m_app->getConfigStore()->getShortcutsModel()->writeToConfig(
        m_app->getSettings());
  ConfigStore::s_miscCfg.m_hideToolBar = !m_viewToolBar->isChecked();
  ConfigStore::s_miscCfg.m_geometry = saveGeometry();
  ConfigStore::s_miscCfg.m_windowState = saveState();
#endif
  m_form->saveConfig();
  m_app->saveConfig();
}

/**
 * Load application options.
 */
void Kid3MainWindow::readOptions()
{
  m_app->readConfig();
#ifdef CONFIG_USE_KDE
  setAutoSaveSettings();
  m_settingsShowHidePicture->setChecked(!ConfigStore::s_miscCfg.m_hidePicture);
  m_settingsAutoHideTags->setChecked(ConfigStore::s_miscCfg.m_autoHideTags);
  m_fileOpenRecent->loadEntries(KConfigGroup(m_app->getSettings(),
                                             "Recent Files"));
#else
  if (ConfigStore::s_miscCfg.m_hideStatusBar)
    statusBar()->hide();
  m_viewStatusBar->setChecked(!ConfigStore::s_miscCfg.m_hideStatusBar);
  m_settingsShowHidePicture->setChecked(!ConfigStore::s_miscCfg.m_hidePicture);
  m_settingsAutoHideTags->setChecked(ConfigStore::s_miscCfg.m_autoHideTags);
  m_fileOpenRecent->loadEntries(m_app->getSettings());
  m_app->getConfigStore()->getShortcutsModel()->readFromConfig(
        m_app->getSettings());
  restoreGeometry(ConfigStore::s_miscCfg.m_geometry);
  restoreState(ConfigStore::s_miscCfg.m_windowState);
#endif
  m_form->readConfig();
}

#ifdef CONFIG_USE_KDE
/**
 * Saves the window properties to the session config file.
 *
 * @param cfg application configuration
 */
void Kid3MainWindow::saveProperties(KConfigGroup& cfg)
{
  cfg.writeEntry("dirname", m_app->getDirName());
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param cfg application configuration
 */
void Kid3MainWindow::readProperties(const KConfigGroup& cfg)
{
  m_app->openDirectory(cfg.readEntry("dirname", ""));
}

#else /* CONFIG_USE_KDE */

/**
 * Window is closed.
 *
 * @param ce close event
 */
void Kid3MainWindow::closeEvent(QCloseEvent* ce)
{
  if (queryClose()) {
    ce->accept();
  }
  else {
    ce->ignore();
  }
}

/**
 * Read font and style options.
 */
void Kid3MainWindow::readFontAndStyleOptions()
{
  ConfigStore::s_miscCfg.readFromConfig(m_app->getSettings());
  if (ConfigStore::s_miscCfg.m_useFont &&
      !ConfigStore::s_miscCfg.m_fontFamily.isEmpty() &&
      ConfigStore::s_miscCfg.m_fontSize > 0) {
    QApplication::setFont(QFont(ConfigStore::s_miscCfg.m_fontFamily,
                                ConfigStore::s_miscCfg.m_fontSize));
  }
  if (!ConfigStore::s_miscCfg.m_style.isEmpty()) {
    QApplication::setStyle(ConfigStore::s_miscCfg.m_style);
  }
}

#endif /* CONFIG_USE_KDE */

/**
 * Save all changed files.
 *
 * @param updateGui true to update GUI (controls, status, cursor)
 */
void Kid3MainWindow::saveDirectory(bool updateGui)
{
  if (updateGui) {
    updateCurrentSelection();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    slotStatusMsg(i18n("Saving directory..."));
  }

  QProgressBar* progress = new QProgressBar;
  statusBar()->addPermanentWidget(progress);
  progress->setMinimum(0);
  connect(m_app, SIGNAL(saveStarted(int)),
          progress, SLOT(setMaximum(int)));
  connect(m_app, SIGNAL(saveProgress(int)),
          progress, SLOT(setValue(int)));
#ifdef CONFIG_USE_KDE
  kapp->processEvents();
#else
  qApp->processEvents();
#endif

  QStringList errorFiles = m_app->saveDirectory();

  statusBar()->removeWidget(progress);
  delete progress;
  updateModificationState();
  if (!errorFiles.empty()) {
#ifdef CONFIG_USE_KDE
    KMessageBox::errorList(
      this, i18n("Error while writing file:\n"),
      errorFiles,
      i18n("File Error"));
#else
    MessageDialog::warningList(
      this, i18n("File Error"), i18n("Error while writing file:\n"),
      errorFiles, QMessageBox::Ok);
#endif
  }

  if (updateGui) {
    slotStatusMsg(i18n("Ready."));
    QApplication::restoreOverrideCursor();
    updateGuiControls();
  }
}

/**
 * If anything was modified, save after asking user.
 *
 * @return false if user canceled.
 */
bool Kid3MainWindow::saveModified()
{
  bool completed=true;

  if(m_app->isModified() && !m_app->getDirName().isEmpty())
  {
    Kid3MainWindow* win=(Kid3MainWindow *) parent();
#ifdef CONFIG_USE_KDE
    const int Yes = KMessageBox::Yes;
    const int No = KMessageBox::No;
    const int Cancel = KMessageBox::Cancel;
    int want_save = KMessageBox::warningYesNoCancel(
        win,
        i18n("The current directory has been modified.\n"
       "Do you want to save it?"),
        i18n("Warning"));
#else
    const int Yes = QMessageBox::Yes;
    const int No = QMessageBox::No;
    const int Cancel = QMessageBox::Cancel;
    int want_save = QMessageBox::warning(
      win,
      i18n("Warning - Kid3"),
      i18n("The current directory has been modified.\n"
           "Do you want to save it?"),
      QMessageBox::Yes | QMessageBox::Default,
      QMessageBox::No,
      QMessageBox::Cancel | QMessageBox::Escape);
#endif
    switch(want_save)
    {
    case Yes:
      saveDirectory();
      completed=true;
      break;

    case No:
      if (m_form->getFileList()->selectionModel())
        m_form->getFileList()->selectionModel()->clearSelection();
      m_app->revertFileModifications();
      m_app->setModified(false);
      completed=true;
      break;

    case Cancel:
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
void Kid3MainWindow::cleanup()
{
  m_app->getSettings()->sync();
  TaggedFile::staticCleanup();
  ContextHelp::staticCleanup();
}

/**
 * Update modification state before closing.
 * Called on closeEvent() of window.
 * If anything was modified, save after asking user.
 * Save options before closing.
 * This method is called by closeEvent(), which occurs when the
 * window is closed or slotFileQuit() (Quit menu) is selected.
 *
 * @return false if user canceled,
 *         true will quit the application.
 */
bool Kid3MainWindow::queryClose()
{
  updateCurrentSelection();
  if (saveModified()) {
    saveOptions();
    cleanup();
    return true;
  }
  return false;
}

/**
 * Request new directory and open it.
 */
void Kid3MainWindow::slotFileOpen()
{
  updateCurrentSelection();
  if(saveModified()) {
    static QString flt = m_app->createFilterString();
    QString dir, filter(ConfigStore::s_miscCfg.m_nameFilter);
#ifdef CONFIG_USE_KDE
    KFileDialog diag(m_app->getDirName(), flt, this);
    diag.setWindowTitle(i18n("Open"));
    if (diag.exec() == QDialog::Accepted) {
      dir = diag.selectedFile();
      filter = diag.currentFilter();
    }
#else
    dir = QFileDialog::getOpenFileName(
      this, QString(), m_app->getDirName(), flt, &filter,
      // filter does not work with the KDE style file dialog
      ConfigStore::s_miscCfg.m_dontUseNativeDialogs
      ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0));
#endif
    if (!dir.isEmpty()) {
      if (!filter.isEmpty()) {
        ConfigStore::s_miscCfg.m_nameFilter = filter;
      }
      m_app->openDirectory(dir);
    }
  }
}

/**
 * Request new directory and open it.
 */
void Kid3MainWindow::slotFileOpenDirectory()
{
  updateCurrentSelection();
  if(saveModified()) {
    QString dir;
#ifdef CONFIG_USE_KDE
    dir = KFileDialog::getExistingDirectory(m_app->getDirName(), this);
#else
    dir = QFileDialog::getExistingDirectory(this, QString(), m_app->getDirName(),
      ConfigStore::s_miscCfg.m_dontUseNativeDialogs
      ? QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog
      : QFileDialog::ShowDirsOnly);
#endif
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
#ifdef CONFIG_USE_KDE
void Kid3MainWindow::slotFileOpenRecentUrl(const KUrl& url)
{
  updateCurrentSelection();
  QString dir = url.path();
  confirmedOpenDirectory(dir);
}
#else /* CONFIG_USE_KDE */
void Kid3MainWindow::slotFileOpenRecentDirectory(const QString& dir)
{
  updateCurrentSelection();
  confirmedOpenDirectory(dir);
}
#endif /* CONFIG_USE_KDE */

/**
 * Save modified files.
 */
void Kid3MainWindow::slotFileSave()
{
  saveDirectory(true);
}

/**
 * Quit application.
 */
void Kid3MainWindow::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
  close(); /* this will lead to call of closeEvent(), queryClose() */
}

#ifdef CONFIG_USE_KDE

void Kid3MainWindow::slotViewStatusBar() {}

/**
 * Shortcuts configuration.
 */
void Kid3MainWindow::slotSettingsShortcuts()
{
  KShortcutsDialog::configure(
    actionCollection(),
    KShortcutsEditor::LetterShortcutsDisallowed, this);
}

/**
 * Toolbars configuration.
 */
void Kid3MainWindow::slotSettingsToolbars()
{
  KEditToolBar dlg(actionCollection());
  if (dlg.exec()) {
    createGUI();
  }
}

void Kid3MainWindow::slotHelpHandbook() {}
void Kid3MainWindow::slotHelpAbout() {}
void Kid3MainWindow::slotHelpAboutQt() {}

#else /* CONFIG_USE_KDE */

void Kid3MainWindow::slotSettingsShortcuts() {}
void Kid3MainWindow::slotSettingsToolbars() {}

/**
 * Turn status bar on or off.
 */
void Kid3MainWindow::slotViewStatusBar()
{
  ConfigStore::s_miscCfg.m_hideStatusBar = !m_viewStatusBar->isChecked();
  slotStatusMsg(i18n("Toggle the statusbar..."));
  if(ConfigStore::s_miscCfg.m_hideStatusBar) {
    statusBar()->hide();
  }
  else {
    statusBar()->show();
  }
  slotStatusMsg(i18n("Ready."));
}

/**
 * Display handbook.
 */
void Kid3MainWindow::slotHelpHandbook()
{
  ContextHelp::displayHelp();
}

/**
 * Display "About" dialog.
 */
void Kid3MainWindow::slotHelpAbout()
{
  QMessageBox::about(
    this, "Kid3",
    "Kid3 " VERSION
    "\n(c) 2003-" RELEASE_YEAR " Urs Fleisch\nufleisch@users.sourceforge.net");
}

/**
 * Display "About Qt" dialog.
 */
void Kid3MainWindow::slotHelpAboutQt()
{
  QMessageBox::aboutQt(this, "Kid3");
}
#endif /* CONFIG_USE_KDE */

/**
 * Change status message.
 *
 * @param text message
 */
void Kid3MainWindow::slotStatusMsg(const QString& text)
{
  statusBar()->showMessage(text);
  // processEvents() is necessary to make the change of the status bar
  // visible when it is changed back again in the same function,
  // i.e. in the same call from the Qt main event loop.
#ifdef CONFIG_USE_KDE
  kapp->processEvents();
#else
  qApp->processEvents();
#endif
}

/**
 * Show playlist dialog.
 */
void Kid3MainWindow::slotPlaylistDialog()
{
  if (!m_playlistDialog) {
    m_playlistDialog = new PlaylistDialog(this);
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
bool Kid3MainWindow::writePlaylist(const PlaylistConfig& cfg)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  slotStatusMsg(i18n("Creating playlist..."));

  bool ok = m_app->writePlaylist(cfg);

  slotStatusMsg(i18n("Ready."));
  QApplication::restoreOverrideCursor();
  return ok;
}

/**
 * Create playlist.
 *
 * @return true if ok.
 */
bool Kid3MainWindow::slotCreatePlaylist()
{
  return writePlaylist(ConfigStore::s_playlistCfg);
}

/**
 * Update track data and create import dialog.
 */
void Kid3MainWindow::setupImportDialog()
{
  m_app->filesToTrackDataModel(ConfigStore::s_genCfg.m_importDest);
  if (!m_importDialog) {
    QString caption(i18n("Import"));
    m_importDialog =
      new ImportDialog(this, caption, m_app->getTrackDataModel(),
                       m_app->getServerImporters(),
                       m_app->getMusicBrainzClient());
  }
  m_importDialog->clear();
}

/**
 * Execute the import dialog.
 */
void Kid3MainWindow::execImportDialog()
{
  if (m_importDialog &&
      m_importDialog->exec() == QDialog::Accepted) {
    m_app->trackDataModelToFiles(m_importDialog->getDestination());
  }
}

/**
 * Import.
 */
void Kid3MainWindow::slotImport()
{
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    setupImportDialog();
    if (m_importDialog) {
      m_importDialog->setAutoStartSubDialog(action->data().toInt());
      execImportDialog();
    }
  }
}

/**
 * Browse album cover artwork.
 */
void Kid3MainWindow::slotBrowseCoverArt()
{
  if (!m_browseCoverArtDialog) {
    m_browseCoverArtDialog = new BrowseCoverArtDialog(this);
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
void Kid3MainWindow::slotExport()
{
  m_exportDialog = new ExportDialog(this, m_app->getTextExporter());
  m_exportDialog->readConfig();
  ImportTrackDataVector trackDataVector;
  m_app->filesToTrackData(ConfigStore::s_genCfg.m_exportSrcV1,
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
void Kid3MainWindow::slotSettingsAutoHideTags()
{
#ifdef CONFIG_USE_KDE
  ConfigStore::s_miscCfg.m_autoHideTags = m_settingsAutoHideTags->isChecked();
#else
  ConfigStore::s_miscCfg.m_autoHideTags = m_settingsAutoHideTags->isChecked();
#endif
  updateCurrentSelection();
  updateGuiControls();
}

/**
 * Show or hide picture.
 */
void Kid3MainWindow::slotSettingsShowHidePicture()
{
#ifdef CONFIG_USE_KDE
  ConfigStore::s_miscCfg.m_hidePicture = !m_settingsShowHidePicture->isChecked();
#else
  ConfigStore::s_miscCfg.m_hidePicture = !m_settingsShowHidePicture->isChecked();
#endif

  m_form->hidePicture(ConfigStore::s_miscCfg.m_hidePicture);
  // In Qt3 the picture is displayed too small if Kid3 is started with picture
  // hidden, and then "Show Picture" is triggered while a file with a picture
  // is selected. Thus updating the controls is only done for Qt4, in Qt3 the
  // file has to be selected again for the picture to be shown.
  if (!ConfigStore::s_miscCfg.m_hidePicture) {
    updateGuiControls();
  }
}

/**
 * Preferences.
 */
void Kid3MainWindow::slotSettingsConfigure()
{
  QString caption(i18n("Configure - Kid3"));
#ifdef CONFIG_USE_KDE
  KConfigSkeleton* configSkeleton = new KConfigSkeleton;
  ConfigDialog* dialog = new ConfigDialog(this, caption, configSkeleton);
#else
  ConfigDialog* dialog = new ConfigDialog(this, caption);
#endif
  dialog->setConfig(m_app->getConfigStore());
  if (dialog->exec() == QDialog::Accepted) {
    dialog->getConfig(m_app->getConfigStore());
    m_app->saveConfig();
    if (!ConfigStore::s_miscCfg.m_markTruncations) {
      m_app->frameModelV1()->markRows(0);
    }
    if (!ConfigStore::s_miscCfg.m_markChanges) {
      m_app->frameModelV1()->markChangedFrames(0);
      m_app->frameModelV2()->markChangedFrames(0);
      m_form->markChangedFilename(false);
    }
    m_app->setTextEncodings();
  }
#ifdef CONFIG_USE_KDE
  delete configSkeleton;
#endif
}

/**
 * Rename directory.
 */
void Kid3MainWindow::slotRenameDirectory()
{
  if (saveModified()) {
    if (!m_renDirDialog) {
      m_renDirDialog = new RenDirDialog(this, m_app->getDirRenamer());
      connect(m_renDirDialog, SIGNAL(actionSchedulingRequested()),
              m_app, SLOT(scheduleRenameActions()));
    }
    m_app->fetchAllDirectories();
    if (TaggedFile* taggedFile =
      TaggedFileOfDirectoryIterator::first(m_app->currentOrRootIndex())) {
      m_renDirDialog->startDialog(taggedFile);
    } else {
      m_renDirDialog->startDialog(0, m_app->getDirName());
    }
    if (m_renDirDialog->exec() == QDialog::Accepted) {
      QString errorMsg(m_app->performRenameActions());
      if (!errorMsg.isEmpty()) {
#ifdef CONFIG_USE_KDE
        KMessageBox::error(this, i18n("Error while renaming:\n") + errorMsg,
                           i18n("File Error"));
#else
        MessageDialog dialog(this);
        dialog.setWindowTitle(i18n("File Error"));
        dialog.setText(i18n("Error while renaming:\n"));
        dialog.setInformativeText(errorMsg);
        dialog.setIcon(QMessageBox::Warning);
        dialog.exec();
#endif
      }
    }
  }
}

/**
 * Number tracks.
 */
void Kid3MainWindow::slotNumberTracks()
{
  if (!m_numberTracksDialog) {
    m_numberTracksDialog = new NumberTracksDialog(this);
  }
  m_numberTracksDialog->setTotalNumberOfTracks(
    m_app->getTotalNumberOfTracksInDir(),
    ConfigStore::s_miscCfg.m_enableTotalNumberOfTracks);
  if (m_numberTracksDialog->exec() == QDialog::Accepted) {
    int nr = m_numberTracksDialog->getStartNumber();
    bool totalEnabled;
    int total = m_numberTracksDialog->getTotalNumberOfTracks(&totalEnabled);
    if (!totalEnabled)
      total = 0;
    ConfigStore::s_miscCfg.m_enableTotalNumberOfTracks = totalEnabled;
    m_app->numberTracks(nr, total, m_numberTracksDialog->getDestination());
  }
}

/**
 * Filter.
 */
void Kid3MainWindow::slotFilter()
{
  if (saveModified()) {
    if (!m_filterDialog) {
      m_filterDialog = new FilterDialog(this);
      connect(m_filterDialog, SIGNAL(apply(FileFilter&)),
              m_app, SLOT(applyFilter(FileFilter&)));
      connect(m_app, SIGNAL(fileFiltered(FileFilter::FilterEventType,QString)),
              m_filterDialog,
              SLOT(showFilterEvent(FileFilter::FilterEventType,QString)));
    }
    m_app->fetchAllDirectories();
    ConfigStore::s_filterCfg.setFilenameFormat(
          m_app->getTagsToFilenameFormat());
    m_filterDialog->readConfig();
    m_filterDialog->exec();
  }
}

#ifdef HAVE_PHONON
/**
 * Play audio file.
 */
void Kid3MainWindow::slotPlayAudio()
{
  m_app->playAudio();
}

/**
 * Show play tool bar.
 */
void Kid3MainWindow::showPlayToolBar()
{
  if (!m_playToolBar) {
    m_playToolBar = new PlayToolBar(m_app->getAudioPlayer(), this);
    m_playToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBar(Qt::BottomToolBarArea, m_playToolBar);
    connect(m_playToolBar, SIGNAL(errorMessage(const QString&)),
            this, SLOT(slotStatusMsg(const QString&)));
  }
  m_playToolBar->show();
}
#endif

/**
 * Update modification state, caption and listbox entries.
 */
void Kid3MainWindow::updateModificationState()
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
void Kid3MainWindow::updateWindowCaption()
{
  QString cap;
  if (!m_app->getDirName().isEmpty()) {
    cap += QDir(m_app->getDirName()).dirName();
  }
  if (m_app->isFiltered()) {
    cap += i18n(" [filtered]");
  }
#ifdef CONFIG_USE_KDE
  setCaption(cap, m_app->isModified());
#else
  if (m_app->isModified()) {
    cap += i18n(" [modified]");
  }
  if (!cap.isEmpty()) {
    cap += " - ";
  }
  cap += "Kid3";
  setWindowTitle(cap);
#endif
}

/**
 * Update files of current selection.
 */
void Kid3MainWindow::updateCurrentSelection()
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
void Kid3MainWindow::updateGuiControls()
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

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
      taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif

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

    if (ConfigStore::s_miscCfg.m_markTruncations) {
      m_app->frameModelV1()->markRows(single_v2_file->getTruncationFlags());
    }
    if (ConfigStore::s_miscCfg.m_markChanges) {
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
    }
    m_form->setDetailInfo(info);
    m_form->setTagFormatV1(QString::null);
    m_form->setTagFormatV2(QString::null);

    if (ConfigStore::s_miscCfg.m_markTruncations) {
      m_app->frameModelV1()->markRows(0);
    }
    if (ConfigStore::s_miscCfg.m_markChanges) {
      m_app->frameModelV1()->markChangedFrames(0);
      m_app->frameModelV2()->markChangedFrames(0);
      m_form->markChangedFilename(false);
    }
  }
  if (!ConfigStore::s_miscCfg.m_hidePicture) {
    FrameCollection::const_iterator it =
      m_app->frameModelV2()->frames().find(Frame(Frame::FT_Picture, "", "", -1));
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

  if (ConfigStore::s_miscCfg.m_autoHideTags) {
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
void Kid3MainWindow::updateAfterFrameModification(TaggedFile* taggedFile)
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
      strNumMap.insert(Frame::ExtendedType(type, "").getTranslatedName().
                       remove(' ').toUpper(), type);
    }
  }
  QMap<QString, int>::const_iterator it =
    strNumMap.find(name.remove(' ').toUpper());
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
bool Kid3MainWindow::selectFrame(Frame* frame, const TaggedFile* taggedFile)
{
  bool ok = false;
  if (taggedFile && frame) {
    QString name = QInputDialog::getItem(
      this, i18n("Add Frame"),
      i18n("Select the frame ID"), taggedFile->getFrameIds(), 0, true, &ok);
    if (ok) {
      Frame::Type type = getTypeFromTranslatedName(name);
      *frame = Frame(type, "", name, -1);
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
bool Kid3MainWindow::editFrameOfTaggedFile(Frame* frame, TaggedFile* taggedFile)
{
  if (!frame || !taggedFile)
    return false;

  bool result = true;
  QString name(frame->getInternalName());
  if (!name.isEmpty()) {
    int nlPos = name.indexOf("\n");
    if (nlPos > 0) {
      // probably "TXXX - User defined text information\nDescription" or
      // "WXXX - User defined URL link\nDescription"
      name.truncate(nlPos);
    }
    name = QCM_translate(name.toLatin1().data());
  }
  if (frame->getFieldList().empty()) {
    EditFrameDialog* dialog =
      new EditFrameDialog(this, name, frame->getValue());
    result = dialog && dialog->exec() == QDialog::Accepted;
    if (result) {
      frame->setValue(dialog->getText());
    }
  } else {
    EditFrameFieldsDialog* dialog =
      new EditFrameFieldsDialog(this, name, *frame, taggedFile);
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
void Kid3MainWindow::renameFile()
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
      this,
      i18n("Rename File"),
      i18n("Enter new file name:"),
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
      QString newPath = dirName + '/' + newFileName;
      if (!Utils::safeRename(absFilename, newPath)) {
        QMessageBox::warning(
          0, i18n("File Error"),
          i18n("Error while renaming:\n") +
          i18n("Rename %1 to %2 failed\n").arg(fileName).arg(newFileName),
          QMessageBox::Ok, Qt::NoButton);
      }
    }
  }
}

/** Only defined for generation of translation files */
#define WANT_TO_DELETE_FOR_PO I18N_NOOP("Do you really want to move these %1 items to the trash?")

/**
 * Delete the selected file(s).
 */
void Kid3MainWindow::deleteFile()
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
#ifdef CONFIG_USE_KDE
    if (KMessageBox::warningContinueCancelList(
          this,
          i18np("Do you really want to move this item to the trash?",
                "Do you really want to move these %1 items to the trash?",
                numFiles),
          files,
          i18n("Move to Trash"),
          KStandardGuiItem::ok(), KStandardGuiItem::cancel(), QString(),
          KMessageBox::Dangerous) == KMessageBox::Continue)
#else
    if (MessageDialog::warningList(
          this,
          i18n("Move to Trash"),
          numFiles > 1
          ? i18n("Do you really want to move these %1 items to the trash?").
            arg(numFiles)
          : i18n("Do you really want to move this item to the trash?"),
          files) == QMessageBox::Ok)
#endif
    {
      bool rmdirError = false;
      files.clear();
      foreach (QPersistentModelIndex index, selItems) {
        QString absFilename(model->filePath(index));
        if (model->isDir(index)) {
          if (!Utils::moveToTrash(absFilename)) {
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
          if (!Utils::moveToTrash(absFilename)) {
            files.append(absFilename);
          }
        }
      }
      if (!files.isEmpty()) {
        QString txt;
        if (rmdirError)
          txt += i18n("Directory must be empty.\n");
        txt += i18n("Could not move these files to the Trash");
#ifdef CONFIG_USE_KDE
        KMessageBox::errorList(this, txt, files, i18n("File Error"));
#else
        MessageDialog::warningList(
          this, i18n("File Error"), txt, files, QMessageBox::Ok);
#endif
      }
    }
  }
}
