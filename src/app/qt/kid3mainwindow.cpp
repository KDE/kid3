/**
 * \file kid3mainwindow.cpp
 * Kid3 main window.
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

#include "kid3mainwindow.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QIcon>
#include <QToolBar>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QStatusBar>
#include "config.h"
#include "qtcompatmac.h"
#include "recentfilesmenu.h"
#include "shortcutsmodel.h"
#include "kid3form.h"
#include "kid3application.h"
#include "configdialog.h"
#include "configstore.h"
#include "contexthelp.h"
#include "serverimporter.h"
#include "platformtools.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
Kid3MainWindow::Kid3MainWindow(QWidget* parent) :
  QMainWindow(parent),
  BaseMainWindow(this, m_platformTools = new PlatformTools),
  m_shortcutsModel(new ShortcutsModel(this)) {
#if !defined Q_OS_WIN32 && defined CFG_DATAROOTDIR
  QPixmap icon;
  if (icon.load(QLatin1String(CFG_DATAROOTDIR) +
#ifndef Q_OS_MAC
                QLatin1String("/icons/hicolor/48x48/apps/kid3-qt.png")
#else
                QLatin1String("/kid3.png")
#endif
        )) {
    setWindowIcon(icon);
  }
#endif
  readFontAndStyleOptions();
  init();
}

/**
 * Destructor.
 */
Kid3MainWindow::~Kid3MainWindow()
{
  delete m_platformTools;
}

/** Only defined for generation of translation files */
#define MAIN_TOOLBAR_FOR_PO QT_TRANSLATE_NOOP("@default", "Main Toolbar")

/**
 * Init menu and toolbar actions.
 */
void Kid3MainWindow::initActions()
{
  QToolBar* toolBar = new QToolBar(this);
  toolBar->setObjectName(QLatin1String("MainToolbar"));
  QMenuBar* menubar = menuBar();
  QString menuTitle(tr("&File"));
  QMenu* fileMenu = menubar->addMenu(menuTitle);

  QAction* fileOpen = new QAction(this);
  fileOpen->setStatusTip(tr("Opens a directory"));
  fileOpen->setText(tr("&Open..."));
  fileOpen->setShortcut(QKeySequence::Open);
  fileOpen->setIcon(QCM_QIcon_fromTheme("document-open"));
  fileOpen->setObjectName(QLatin1String("file_open"));
  m_shortcutsModel->registerAction(fileOpen, menuTitle);
  connect(fileOpen, SIGNAL(triggered()),
    impl(), SLOT(slotFileOpen()));
  fileMenu->addAction(fileOpen);
  toolBar->addAction(fileOpen);

  m_fileOpenRecent = new RecentFilesMenu(fileMenu);
  connect(m_fileOpenRecent, SIGNAL(loadFile(const QString&)),
          this, SLOT(slotFileOpenRecentDirectory(const QString&)));
  m_fileOpenRecent->setStatusTip(tr("Opens a recently used directory"));
  m_fileOpenRecent->setTitle(tr("Open &Recent"));
  m_fileOpenRecent->setIcon(QCM_QIcon_fromTheme("document-open-recent"));
  fileMenu->addMenu(m_fileOpenRecent);

  QAction* fileOpenDirectory = new QAction(this);
  fileOpenDirectory->setStatusTip(tr("Opens a directory"));
  fileOpenDirectory->setText(tr("O&pen Directory..."));
  fileOpenDirectory->setShortcut(Qt::CTRL + Qt::Key_D);
  fileOpenDirectory->setIcon(QCM_QIcon_fromTheme("document-open"));
  fileOpenDirectory->setObjectName(QLatin1String("open_directory"));
  m_shortcutsModel->registerAction(fileOpenDirectory, menuTitle);
  connect(fileOpenDirectory, SIGNAL(triggered()),
    impl(), SLOT(slotFileOpenDirectory()));
  fileMenu->addAction(fileOpenDirectory);
  fileMenu->addSeparator();

  QAction* fileSave = new QAction(this);
  fileSave->setStatusTip(tr("Saves the changed files"));
  fileSave->setText(tr("&Save"));
  fileSave->setShortcut(QKeySequence::Save);
  fileSave->setIcon(QCM_QIcon_fromTheme("document-save"));
  fileSave->setObjectName(QLatin1String("file_save"));
  m_shortcutsModel->registerAction(fileSave, menuTitle);
  connect(fileSave, SIGNAL(triggered()),
    impl(), SLOT(slotFileSave()));
  fileMenu->addAction(fileSave);
  toolBar->addAction(fileSave);

  QAction* fileRevert = new QAction(this);
  fileRevert->setStatusTip(
      tr("Reverts the changes of all or the selected files"));
  fileRevert->setText(tr("Re&vert"));
  fileRevert->setIcon(QCM_QIcon_fromTheme("document-revert"));
  fileRevert->setObjectName(QLatin1String("file_revert"));
  m_shortcutsModel->registerAction(fileRevert, menuTitle);
  connect(fileRevert, SIGNAL(triggered()),
          app(), SLOT(revertFileModifications()));
  fileMenu->addAction(fileRevert);
  toolBar->addAction(fileRevert);
  fileMenu->addSeparator();

  QAction* fileImport = new QAction(this);
  fileImport->setData(-1);
  fileImport->setStatusTip(tr("Import from file or clipboard"));
  fileImport->setText(tr("&Import..."));
  fileImport->setIcon(QCM_QIcon_fromTheme("document-import"));
  fileImport->setObjectName(QLatin1String("import"));
  m_shortcutsModel->registerAction(fileImport, menuTitle);
  connect(fileImport, SIGNAL(triggered()),
    impl(), SLOT(slotImport()));
  fileMenu->addAction(fileImport);

  int importerIdx = 0;
  foreach (const ServerImporter* si, app()->getServerImporters()) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower().remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    QAction* fileImportServer = new QAction(this);
    fileImportServer->setData(importerIdx);
    fileImportServer->setStatusTip(tr("Import from %1").arg(serverName));
    fileImportServer->setText(tr("Import from %1...").arg(serverName));
    fileImportServer->setObjectName(actionName);
    m_shortcutsModel->registerAction(fileImportServer, menuTitle);
    connect(fileImportServer, SIGNAL(triggered()),
      impl(), SLOT(slotImport()));
    fileMenu->addAction(fileImportServer);
    ++importerIdx;
  }
#ifdef HAVE_CHROMAPRINT
  QAction* fileImportMusicBrainz = new QAction(this);
  QString serverName(tr("MusicBrainz Fingerprint"));
  fileImportMusicBrainz->setData(importerIdx);
  fileImportMusicBrainz->setStatusTip(tr("Import from %1").arg(serverName));
  fileImportMusicBrainz->setText(tr("Import from %1...").arg(serverName));
  fileImportMusicBrainz->setObjectName(QLatin1String("import_musicbrainz"));
  m_shortcutsModel->registerAction(fileImportMusicBrainz, menuTitle);
  connect(fileImportMusicBrainz, SIGNAL(triggered()),
    impl(), SLOT(slotImport()));
  fileMenu->addAction(fileImportMusicBrainz);
  ++importerIdx;
#endif
  QAction* fileBatchImport = new QAction(this);
  fileBatchImport->setStatusTip(tr("Automatic import"));
  fileBatchImport->setText(tr("Automatic I&mport..."));
  fileBatchImport->setObjectName(QLatin1String("batch_import"));
  m_shortcutsModel->registerAction(fileBatchImport, menuTitle);
  connect(fileBatchImport, SIGNAL(triggered()),
    impl(), SLOT(slotBatchImport()));
  fileMenu->addAction(fileBatchImport);

  QAction* fileBrowseCoverArt = new QAction(this);
  fileBrowseCoverArt->setStatusTip(tr("Browse album cover artwork"));
  fileBrowseCoverArt->setText(tr("&Browse Cover Art..."));
  fileBrowseCoverArt->setObjectName(QLatin1String("browse_cover_art"));
  m_shortcutsModel->registerAction(fileBrowseCoverArt, menuTitle);
  connect(fileBrowseCoverArt, SIGNAL(triggered()),
    impl(), SLOT(slotBrowseCoverArt()));
  fileMenu->addAction(fileBrowseCoverArt);

  QAction* fileExport = new QAction(this);
  fileExport->setStatusTip(tr("Export to file or clipboard"));
  fileExport->setText(tr("&Export..."));
  fileExport->setIcon(QCM_QIcon_fromTheme("document-export"));
  fileExport->setObjectName(QLatin1String("export"));
  m_shortcutsModel->registerAction(fileExport, menuTitle);
  connect(fileExport, SIGNAL(triggered()),
    impl(), SLOT(slotExport()));
  fileMenu->addAction(fileExport);

  QAction* fileCreatePlaylist = new QAction(this);
  fileCreatePlaylist->setStatusTip(tr("Create M3U Playlist"));
  fileCreatePlaylist->setText(tr("&Create Playlist..."));
  fileCreatePlaylist->setIcon(QIcon(QLatin1String(":/images/view-media-playlist.png")));
  fileCreatePlaylist->setObjectName(QLatin1String("create_playlist"));
  m_shortcutsModel->registerAction(fileCreatePlaylist, menuTitle);
  connect(fileCreatePlaylist, SIGNAL(triggered()),
    impl(), SLOT(slotPlaylistDialog()));
  fileMenu->addAction(fileCreatePlaylist);
  toolBar->addAction(fileCreatePlaylist);
  fileMenu->addSeparator();

  QAction* fileQuit = new QAction(this);
  fileQuit->setStatusTip(tr("Quits the application"));
  fileQuit->setText(tr("&Quit"));
  fileQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
  fileQuit->setIcon(QCM_QIcon_fromTheme("application-exit"));
  fileQuit->setObjectName(QLatin1String("file_quit"));
  m_shortcutsModel->registerAction(fileQuit, menuTitle);
  connect(fileQuit, SIGNAL(triggered()),
    impl(), SLOT(slotFileQuit()));
  fileMenu->addAction(fileQuit);

  menuTitle = tr("&Edit");
  QMenu* editMenu = menubar->addMenu(menuTitle);
  QAction* editSelectAll = new QAction(this);
  editSelectAll->setStatusTip(tr("Select all files"));
  editSelectAll->setText(tr("Select &All"));
  editSelectAll->setShortcut(Qt::ALT + Qt::Key_A);
  editSelectAll->setIcon(QCM_QIcon_fromTheme("edit-select-all"));
  editSelectAll->setObjectName(QLatin1String("edit_select_all"));
  m_shortcutsModel->registerAction(editSelectAll, menuTitle);
  connect(editSelectAll, SIGNAL(triggered()),
          form(), SLOT(selectAllFiles()));
  editMenu->addAction(editSelectAll);

  QAction* editDeselect = new QAction(this);
  editDeselect->setStatusTip(tr("Deselect all files"));
  editDeselect->setText(tr("Dese&lect"));
  editDeselect->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);
  editDeselect->setObjectName(QLatin1String("edit_deselect"));
  m_shortcutsModel->registerAction(editDeselect, menuTitle);
  connect(editDeselect, SIGNAL(triggered()),
          form(), SLOT(deselectAllFiles()));
  editMenu->addAction(editDeselect);

  QAction* editSelectAllInDir = new QAction(this);
  editSelectAllInDir->setStatusTip(tr("Select all files in the current directory"));
  editSelectAllInDir->setText(tr("Select All in &Directory"));
  editSelectAllInDir->setObjectName(QLatin1String("select_all_in_directory"));
  m_shortcutsModel->registerAction(editSelectAllInDir, menuTitle);
  connect(editSelectAllInDir, SIGNAL(triggered()),
    form(), SLOT(selectAllInDirectory()));
  editMenu->addAction(editSelectAllInDir);

  QAction* editPreviousFile = new QAction(this);
  editPreviousFile->setStatusTip(tr("Select previous file"));
  editPreviousFile->setText(tr("&Previous File"));
  editPreviousFile->setShortcut(Qt::ALT + Qt::Key_Up);
  editPreviousFile->setIcon(QCM_QIcon_fromTheme("go-previous"));
  editPreviousFile->setObjectName(QLatin1String("previous_file"));
  m_shortcutsModel->registerAction(editPreviousFile, menuTitle);
  connect(editPreviousFile, SIGNAL(triggered()),
    app(), SLOT(previousFile()));
  editMenu->addAction(editPreviousFile);
  toolBar->addAction(editPreviousFile);

  QAction* editNextFile = new QAction(this);
  editNextFile->setStatusTip(tr("Select next file"));
  editNextFile->setText(tr("&Next File"));
  editNextFile->setShortcut(Qt::ALT + Qt::Key_Down);
  editNextFile->setIcon(QCM_QIcon_fromTheme("go-next"));
  editNextFile->setObjectName(QLatin1String("next_file"));
  m_shortcutsModel->registerAction(editNextFile, menuTitle);
  connect(editNextFile, SIGNAL(triggered()),
    app(), SLOT(nextFile()));
  editMenu->addAction(editNextFile);
  toolBar->addAction(editNextFile);

  menuTitle = tr("&Tools");
  QMenu* toolsMenu = menubar->addMenu(menuTitle);
  QAction* toolsApplyFilenameFormat = new QAction(this);
  toolsApplyFilenameFormat->setStatusTip(tr("Apply Filename Format"));
  toolsApplyFilenameFormat->setText(tr("Apply &Filename Format"));
  toolsApplyFilenameFormat->setObjectName(QLatin1String("apply_filename_format"));
  m_shortcutsModel->registerAction(toolsApplyFilenameFormat, menuTitle);
  connect(toolsApplyFilenameFormat, SIGNAL(triggered()),
    app(), SLOT(applyFilenameFormat()));
  toolsMenu->addAction(toolsApplyFilenameFormat);

  QAction* toolsApplyId3Format = new QAction(this);
  toolsApplyId3Format->setStatusTip(tr("Apply Tag Format"));
  toolsApplyId3Format->setText(tr("Apply &Tag Format"));
  toolsApplyId3Format->setObjectName(QLatin1String("apply_id3_format"));
  m_shortcutsModel->registerAction(toolsApplyId3Format, menuTitle);
  connect(toolsApplyId3Format, SIGNAL(triggered()),
    app(), SLOT(applyId3Format()));
  toolsMenu->addAction(toolsApplyId3Format);

  QAction* toolsApplyTextEncoding = new QAction(this);
  toolsApplyTextEncoding->setStatusTip(tr("Apply Text Encoding"));
  toolsApplyTextEncoding->setText(tr("Apply Text &Encoding"));
  toolsApplyTextEncoding->setObjectName(QLatin1String("apply_text_encoding"));
  m_shortcutsModel->registerAction(toolsApplyTextEncoding, menuTitle);
  connect(toolsApplyTextEncoding, SIGNAL(triggered()),
    app(), SLOT(applyTextEncoding()));
  toolsMenu->addAction(toolsApplyTextEncoding);

  QAction* toolsRenameDirectory = new QAction(this);
  toolsRenameDirectory->setStatusTip(tr("Rename Directory"));
  toolsRenameDirectory->setText(tr("&Rename Directory..."));
  toolsRenameDirectory->setObjectName(QLatin1String("rename_directory"));
  m_shortcutsModel->registerAction(toolsRenameDirectory, menuTitle);
  connect(toolsRenameDirectory, SIGNAL(triggered()),
    impl(), SLOT(slotRenameDirectory()));
  toolsMenu->addAction(toolsRenameDirectory);

  QAction* toolsNumberTracks = new QAction(this);
  toolsNumberTracks->setStatusTip(tr("Number Tracks"));
  toolsNumberTracks->setText(tr("&Number Tracks..."));
  toolsNumberTracks->setObjectName(QLatin1String("number_tracks"));
  m_shortcutsModel->registerAction(toolsNumberTracks, menuTitle);
  connect(toolsNumberTracks, SIGNAL(triggered()),
    impl(), SLOT(slotNumberTracks()));
  toolsMenu->addAction(toolsNumberTracks);

  QAction* toolsFilter = new QAction(this);
  toolsFilter->setStatusTip(tr("Filter"));
  toolsFilter->setText(tr("F&ilter..."));
  toolsFilter->setObjectName(QLatin1String("filter"));
  m_shortcutsModel->registerAction(toolsFilter, menuTitle);
  connect(toolsFilter, SIGNAL(triggered()),
    impl(), SLOT(slotFilter()));
  toolsMenu->addAction(toolsFilter);

#ifdef HAVE_TAGLIB
  QAction* toolsConvertToId3v24 = new QAction(this);
  toolsConvertToId3v24->setStatusTip(tr("Convert ID3v2.3 to ID3v2.4"));
  toolsConvertToId3v24->setText(tr("Convert ID3v2.3 to ID3v2.&4"));
  toolsConvertToId3v24->setObjectName(QLatin1String("convert_to_id3v24"));
  m_shortcutsModel->registerAction(toolsConvertToId3v24, menuTitle);
  connect(toolsConvertToId3v24, SIGNAL(triggered()),
    app(), SLOT(convertToId3v24()));
  toolsMenu->addAction(toolsConvertToId3v24);
#endif

#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  QAction* toolsConvertToId3v23 = new QAction(this);
  toolsConvertToId3v23->setStatusTip(tr("Convert ID3v2.4 to ID3v2.3"));
  toolsConvertToId3v23->setText(tr("Convert ID3v2.4 to ID3v2.&3"));
  toolsConvertToId3v23->setObjectName(QLatin1String("convert_to_id3v23"));
  m_shortcutsModel->registerAction(toolsConvertToId3v23, menuTitle);
  connect(toolsConvertToId3v23, SIGNAL(triggered()),
    app(), SLOT(convertToId3v23()));
  toolsMenu->addAction(toolsConvertToId3v23);
#endif

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  QAction* toolsPlay = new QAction(this);
  toolsPlay->setStatusTip(tr("Play"));
  toolsPlay->setText(tr("&Play"));
  toolsPlay->setIcon(QIcon(style()->standardIcon(QStyle::SP_MediaPlay)));
  toolsPlay->setObjectName(QLatin1String("play"));
  m_shortcutsModel->registerAction(toolsPlay, menuTitle);
  connect(toolsPlay, SIGNAL(triggered()),
    app(), SLOT(playAudio()));
  toolsMenu->addAction(toolsPlay);
  toolBar->addAction(toolsPlay);
#endif

  menuTitle = tr("&Settings");
  QMenu* settingsMenu = menubar->addMenu(menuTitle);
  m_viewToolBar = toolBar->toggleViewAction();
  if (m_viewToolBar) {
    m_viewToolBar->setStatusTip(tr("Enables/disables the toolbar"));
    m_viewToolBar->setText(tr("Show &Toolbar"));
    m_viewToolBar->setObjectName(QLatin1String("options_configure_toolbars"));
    m_shortcutsModel->registerAction(m_viewToolBar, menuTitle);
  }
  if (ConfigStore::s_miscCfg.m_hideToolBar)
    toolBar->hide();
  m_viewToolBar->setChecked(!ConfigStore::s_miscCfg.m_hideToolBar);
  settingsMenu->addAction(m_viewToolBar);

  m_viewStatusBar = new QAction(this);
  m_viewStatusBar->setStatusTip(tr("Enables/disables the statusbar"));
  m_viewStatusBar->setText(tr("Show St&atusbar"));
  m_viewStatusBar->setCheckable(true);
  m_viewStatusBar->setObjectName(QLatin1String("options_show_statusbar"));
  m_shortcutsModel->registerAction(m_viewStatusBar, menuTitle);
  connect(m_viewStatusBar, SIGNAL(triggered()),
    this, SLOT(slotViewStatusBar()));
  settingsMenu->addAction(m_viewStatusBar);

  m_settingsShowHidePicture = new QAction(this);
  m_settingsShowHidePicture->setStatusTip(tr("Show Picture"));
  m_settingsShowHidePicture->setText(tr("Show &Picture"));
  m_settingsShowHidePicture->setCheckable(true);
  m_settingsShowHidePicture->setObjectName(QLatin1String("hide_picture"));
  m_shortcutsModel->registerAction(m_settingsShowHidePicture, menuTitle);
  connect(m_settingsShowHidePicture, SIGNAL(triggered()),
    impl(), SLOT(slotSettingsShowHidePicture()));
  settingsMenu->addAction(m_settingsShowHidePicture);

  m_settingsAutoHideTags = new QAction(this);
  m_settingsAutoHideTags->setStatusTip(tr("Auto Hide Tags"));
  m_settingsAutoHideTags->setText(tr("Auto &Hide Tags"));
  m_settingsAutoHideTags->setCheckable(true);
  m_settingsAutoHideTags->setObjectName(QLatin1String("auto_hide_tags"));
  m_shortcutsModel->registerAction(m_settingsAutoHideTags, menuTitle);
  connect(m_settingsAutoHideTags, SIGNAL(triggered()),
    impl(), SLOT(slotSettingsAutoHideTags()));
  settingsMenu->addAction(m_settingsAutoHideTags);

  QAction* settingsConfigure = new QAction(this);
  settingsConfigure->setStatusTip(tr("Configure Kid3"));
  settingsConfigure->setText(tr("&Configure Kid3..."));
  settingsConfigure->setIcon(QCM_QIcon_fromTheme("preferences-system"));
#if QT_VERSION >= 0x040600
  settingsConfigure->setShortcut(QKeySequence::Preferences);
#endif
  settingsConfigure->setObjectName(QLatin1String("options_configure"));
  m_shortcutsModel->registerAction(settingsConfigure, menuTitle);
  connect(settingsConfigure, SIGNAL(triggered()),
    this, SLOT(slotSettingsConfigure()));
  settingsMenu->addSeparator();
  settingsMenu->addAction(settingsConfigure);
  toolBar->addAction(settingsConfigure);

  menuTitle = tr("&Help");
  QMenu* helpMenu = menubar->addMenu(menuTitle);
  QAction* helpHandbook = new QAction(this);
  helpHandbook->setStatusTip(tr("Kid3 Handbook"));
  helpHandbook->setText(tr("Kid3 &Handbook"));
  helpHandbook->setIcon(QCM_QIcon_fromTheme("help-contents"));
  helpHandbook->setShortcut(QKeySequence::HelpContents);
  helpHandbook->setObjectName(QLatin1String("help_contents"));
  m_shortcutsModel->registerAction(helpHandbook, menuTitle);
  connect(helpHandbook, SIGNAL(triggered()),
    this, SLOT(slotHelpHandbook()));
  helpMenu->addAction(helpHandbook);

  QAction* helpAbout = new QAction(this);
  helpAbout->setStatusTip(tr("About Kid3"));
  helpAbout->setText(tr("&About Kid3"));
  helpAbout->setObjectName(QLatin1String("help_about_app"));
  m_shortcutsModel->registerAction(helpAbout, menuTitle);
  connect(helpAbout, SIGNAL(triggered()),
    this, SLOT(slotHelpAbout()));
  helpMenu->addAction(helpAbout);

  QAction* helpAboutQt = new QAction(this);
  helpAboutQt->setStatusTip(tr("About Qt"));
  helpAboutQt->setText(tr("About &Qt"));
  helpAboutQt->setObjectName(QLatin1String("help_about_qt"));
  m_shortcutsModel->registerAction(helpAboutQt, menuTitle);
  connect(helpAboutQt, SIGNAL(triggered()),
    this, SLOT(slotHelpAboutQt()));
  helpMenu->addAction(helpAboutQt);

  addToolBar(toolBar);

  updateWindowCaption();

  initFormActions();
}

/**
 * Init actions of form.
 */
void Kid3MainWindow::initFormActions()
{
  QString ctx(tr("Filename"));
  initAction(tr("From Tag 1"), QLatin1String("filename_from_v1"), form(), SLOT(fnFromID3V1()), ctx);
  initAction(tr("From Tag 2"), QLatin1String("filename_from_v2"), form(), SLOT(fnFromID3V2()), ctx);
  initAction(tr("Focus"), QLatin1String("filename_focus"), form(), SLOT(setFocusFilename()), ctx);
  ctx = tr("Tag 1");
  initAction(tr("From Filename"), QLatin1String("v1_from_filename"), app(), SLOT(getTagsFromFilenameV1()), ctx);
  initAction(tr("From Tag 2"), QLatin1String("v1_from_v2"), app(), SLOT(copyV2ToV1()), ctx);
  initAction(tr("Copy"), QLatin1String("v1_copy"), app(), SLOT(copyTagsV1()), ctx);
  initAction(tr("Paste"), QLatin1String("v1_paste"), app(), SLOT(pasteTagsV1()), ctx);
  initAction(tr("Remove"), QLatin1String("v1_remove"), app(), SLOT(removeTagsV1()), ctx);
  initAction(tr("Focus"), QLatin1String("v1_focus"), form(), SLOT(setFocusV1()), ctx);
  ctx = tr("Tag 2");
  initAction(tr("From Filename"), QLatin1String("v2_from_filename"), app(), SLOT(getTagsFromFilenameV2()), ctx);
  initAction(tr("From Tag 1"), QLatin1String("v2_from_v1"), app(), SLOT(copyV1ToV2()), ctx);
  initAction(tr("Copy"), QLatin1String("v2_copy"), app(), SLOT(copyTagsV2()), ctx);
  initAction(tr("Paste"), QLatin1String("v2_paste"), app(), SLOT(pasteTagsV2()), ctx);
  initAction(tr("Remove"), QLatin1String("v2_remove"), app(), SLOT(removeTagsV2()), ctx);
  initAction(tr("Edit"), QLatin1String("frames_edit"), form(), SLOT(editFrame()), ctx);
  initAction(tr("Add"), QLatin1String("frames_add"), form(), SLOT(addFrame()), ctx);
  initAction(tr("Delete"), QLatin1String("frames_delete"), form(), SLOT(deleteFrame()), ctx);
  initAction(tr("Focus"), QLatin1String("v2_focus"), form(), SLOT(setFocusV2()), ctx);
  ctx = tr("File List");
  initAction(tr("Focus"), QLatin1String("filelist_focus"), form(), SLOT(setFocusFileList()), ctx);
  ctx = tr("Directory List");
  initAction(tr("Focus"), QLatin1String("dirlist_focus"), form(), SLOT(setFocusDirList()), ctx);
}

/**
 * Init action of form.
 */
void Kid3MainWindow::initAction(const QString& text, const QString& name,
                                const QObject* receiver, const char* slot,
                                const QString& context)
{
  QAction* action = new QAction(form());
  action->setStatusTip(text);
  action->setText(text);
  action->setObjectName(name);
  m_shortcutsModel->registerAction(action, context);
  connect(action, SIGNAL(triggered()), receiver, slot);
  addAction(action);
}

/**
 * Add directory to recent files list.
 *
 * @param dirName path to directory
 */
void Kid3MainWindow::addDirectoryToRecentFiles(const QString& dirName)
{
  m_fileOpenRecent->addDirectory(dirName);
}

/**
 * Read settings from the configuration.
 */
void Kid3MainWindow::readConfig()
{
  if (ConfigStore::s_miscCfg.m_hideStatusBar)
    statusBar()->hide();
  m_viewStatusBar->setChecked(!ConfigStore::s_miscCfg.m_hideStatusBar);
  m_settingsShowHidePicture->setChecked(!ConfigStore::s_miscCfg.m_hidePicture);
  m_settingsAutoHideTags->setChecked(ConfigStore::s_miscCfg.m_autoHideTags);
  m_fileOpenRecent->loadEntries(app()->getSettings());
  m_shortcutsModel->readFromConfig(app()->getSettings());
  restoreGeometry(ConfigStore::s_miscCfg.m_geometry);
  restoreState(ConfigStore::s_miscCfg.m_windowState);
}

/**
 * Store geometry and recent files in settings.
 */
void Kid3MainWindow::saveConfig()
{
  m_fileOpenRecent->saveEntries(app()->getSettings());
  m_shortcutsModel->writeToConfig(app()->getSettings());
  ConfigStore::s_miscCfg.m_hideToolBar = !m_viewToolBar->isChecked();
  ConfigStore::s_miscCfg.m_geometry = saveGeometry();
  ConfigStore::s_miscCfg.m_windowState = saveState();
}

/**
 * Set main window caption.
 *
 * @param caption caption without application name
 * @param modified true if any file is modified
 */
void Kid3MainWindow::setWindowCaption(const QString& caption, bool modified)
{
  QString cap(caption);
  if (modified) {
    cap += tr(" [modified]");
  }
  if (!cap.isEmpty()) {
    cap += QLatin1String(" - ");
  }
  cap += QLatin1String("Kid3");
  setWindowTitle(cap);
}

/**
 * Get action for Settings/Auto Hide Tags.
 * @return action.
 */
QAction* Kid3MainWindow::autoHideTagsAction()
{
  return m_settingsAutoHideTags;
}

/**
 * Get action for Settings/Hide Picture.
 * @return action.
 */
QAction* Kid3MainWindow::showHidePictureAction()
{
 return m_settingsShowHidePicture;
}

/**
 * Window is closed.
 *
 * @param ce close event
 */
void Kid3MainWindow::closeEvent(QCloseEvent* ce)
{
  if (queryBeforeClosing()) {
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
  ConfigStore::s_miscCfg.readFromConfig(app()->getSettings());
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

/**
 * Open recent directory.
 *
 * @param dir directory to open
 */
void Kid3MainWindow::slotFileOpenRecentDirectory(const QString& dir)
{
  openRecentDirectory(dir);
}

/**
 * Turn status bar on or off.
 */
void Kid3MainWindow::slotViewStatusBar()
{
  ConfigStore::s_miscCfg.m_hideStatusBar = !m_viewStatusBar->isChecked();
  slotStatusMsg(tr("Toggle the statusbar..."));
  if(ConfigStore::s_miscCfg.m_hideStatusBar) {
    statusBar()->hide();
  }
  else {
    statusBar()->show();
  }
  slotStatusMsg(tr("Ready."));
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
    this, QLatin1String("Kid3"),
    QLatin1String("Kid3 " VERSION
    "\n(c) 2003-" RELEASE_YEAR " Urs Fleisch\nufleisch@users.sourceforge.net"));
}

/**
 * Display "About Qt" dialog.
 */
void Kid3MainWindow::slotHelpAboutQt()
{
  QMessageBox::aboutQt(this, QLatin1String("Kid3"));
}

/**
 * Preferences.
 */
void Kid3MainWindow::slotSettingsConfigure()
{
  QString caption(tr("Configure - Kid3"));
  ConfigDialog* dialog = new ConfigDialog(this, caption, m_shortcutsModel);
  dialog->setConfig(app()->getConfigStore());
  if (dialog->exec() == QDialog::Accepted) {
    dialog->getConfig(app()->getConfigStore());
    impl()->applyChangedConfiguration();
  }
}
