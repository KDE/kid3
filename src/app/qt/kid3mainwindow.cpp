/**
 * \file kid3mainwindow.cpp
 * Kid3 main window.
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
#include <QSessionManager>
#include "config.h"
#include "recentfilesmenu.h"
#include "shortcutsmodel.h"
#include "mainwindowconfig.h"
#include "kid3form.h"
#include "filelist.h"
#include "kid3application.h"
#include "configdialog.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"
#include "useractionsconfig.h"
#include "contexthelp.h"
#include "serverimporter.h"
#include "servertrackimporter.h"
#include "loadtranslation.h"
#include "fileproxymodel.h"

/**
 * Constructor.
 *
 * @param platformTools platform specific tools
 * @param app application context
 * @param parent parent widget
 */
Kid3MainWindow::Kid3MainWindow(IPlatformTools* platformTools,
                               Kid3Application* app, QWidget* parent)
  : QMainWindow(parent), BaseMainWindow(this, platformTools, app),
    m_platformTools(platformTools),
    m_fileOpenRecent(nullptr), m_shortcutsModel(new ShortcutsModel(this)),
    m_viewToolBar(nullptr), m_viewStatusBar(nullptr),
    m_settingsAutoHideTags(nullptr), m_settingsShowHidePicture(nullptr)
{
#if !defined Q_OS_WIN32 && defined CFG_DATAROOTDIR
  QString dataRootDir(QLatin1String(CFG_DATAROOTDIR));
  Utils::prependApplicationDirPathIfRelative(dataRootDir);

  QPixmap icon;
  if (icon.load(dataRootDir +
#ifndef Q_OS_MAC
                QLatin1String("/icons/hicolor/128x128/apps/kid3-qt.png")
#else
                QLatin1String("/kid3.png")
#endif
        )) {
    setWindowIcon(icon);
  }
#endif
  readFontAndStyleOptions();
  init();

  connect(qApp, &QGuiApplication::commitDataRequest,
          this, &Kid3MainWindow::onCommitDataRequest);
}

/** Only defined for generation of translation files */
#define MAIN_TOOLBAR_FOR_PO QT_TRANSLATE_NOOP("@default", "Main Toolbar")

/**
 * Init menu and toolbar actions.
 */
void Kid3MainWindow::initActions()
{
  auto toolBar = new QToolBar(this);
  toolBar->setObjectName(QLatin1String("MainToolbar"));
#ifdef Q_OS_MAC
  toolBar->setStyleSheet(QLatin1String("QToolButton { border: 0; }"));
#endif
  QMenuBar* menubar = menuBar();
  QString menuTitle(tr("&File"));
  QMenu* fileMenu = menubar->addMenu(menuTitle);

  auto fileOpen = new QAction(this);
  fileOpen->setStatusTip(tr("Open files"));
  fileOpen->setText(tr("&Open..."));
  fileOpen->setShortcut(QKeySequence::Open);
  fileOpen->setIcon(QIcon::fromTheme(QLatin1String("document-open"),
      QIcon(QLatin1String(":/images/document-open.png"))));
  fileOpen->setObjectName(QLatin1String("file_open"));
  m_shortcutsModel->registerAction(fileOpen, menuTitle);
  connect(fileOpen, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotFileOpen);
  fileMenu->addAction(fileOpen);
  toolBar->addAction(fileOpen);

  m_fileOpenRecent = new RecentFilesMenu(fileMenu);
  connect(m_fileOpenRecent, &RecentFilesMenu::loadFile,
          this, &Kid3MainWindow::slotFileOpenRecentDirectory);
  m_fileOpenRecent->setStatusTip(tr("Opens a recently used directory"));
  m_fileOpenRecent->setTitle(tr("Open &Recent"));
  m_fileOpenRecent->setIcon(QIcon::fromTheme(
      QLatin1String("document-open-recent"),
      QIcon(QLatin1String(":/images/document-open-recent.png"))));
  fileMenu->addMenu(m_fileOpenRecent);

  auto fileOpenDirectory = new QAction(this);
  fileOpenDirectory->setStatusTip(tr("Opens a directory"));
  fileOpenDirectory->setText(tr("O&pen Directory..."));
  fileOpenDirectory->setShortcut(Qt::CTRL + Qt::Key_D);
  fileOpenDirectory->setIcon(QIcon::fromTheme(QLatin1String("document-open"),
      QIcon(QLatin1String(":/images/document-open.png"))));
  fileOpenDirectory->setObjectName(QLatin1String("open_directory"));
  m_shortcutsModel->registerAction(fileOpenDirectory, menuTitle);
  connect(fileOpenDirectory, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotFileOpenDirectory);
  fileMenu->addAction(fileOpenDirectory);

  auto fileReload = new QAction(this);
  fileReload->setStatusTip(tr("Reload directory"));
  fileReload->setText(tr("Re&load"));
  fileReload->setShortcut(QKeySequence::Refresh);
  fileReload->setIcon(QIcon::fromTheme(QLatin1String("view-refresh"),
      QIcon(QLatin1String(":/images/view-refresh.png"))));
  fileReload->setObjectName(QLatin1String("reload"));
  m_shortcutsModel->registerAction(fileReload, menuTitle);
  connect(fileReload, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotFileReload);
  fileMenu->addAction(fileReload);

  auto fileUnload = new QAction(this);
  fileUnload->setText(tr("Unload"));
  fileUnload->setObjectName(QLatin1String("unload"));
  m_shortcutsModel->registerAction(fileUnload, menuTitle);
  connect(fileUnload, &QAction::triggered,
          app(), &Kid3Application::unloadAllTags);
  addAction(fileUnload);

  fileMenu->addSeparator();

  auto fileSave = new QAction(this);
  fileSave->setStatusTip(tr("Saves the changed files"));
  fileSave->setText(tr("&Save"));
  fileSave->setShortcut(QKeySequence::Save);
  fileSave->setIcon(QIcon::fromTheme(QLatin1String("document-save"),
      QIcon(QLatin1String(":/images/document-save.png"))));
  fileSave->setObjectName(QLatin1String("file_save"));
  m_shortcutsModel->registerAction(fileSave, menuTitle);
  connect(fileSave, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotFileSave);
  fileMenu->addAction(fileSave);
  toolBar->addAction(fileSave);

  auto fileRevert = new QAction(this);
  fileRevert->setStatusTip(
      tr("Reverts the changes of all or the selected files"));
  fileRevert->setText(tr("Re&vert"));
  fileRevert->setShortcut(QKeySequence::Undo);
  fileRevert->setIcon(QIcon::fromTheme(QLatin1String("document-revert"),
      QIcon(QLatin1String(":/images/document-revert.png"))));
  fileRevert->setObjectName(QLatin1String("file_revert"));
  m_shortcutsModel->registerAction(fileRevert, menuTitle);
  connect(fileRevert, &QAction::triggered,
          app(), &Kid3Application::revertFileModifications);
  fileMenu->addAction(fileRevert);
  toolBar->addAction(fileRevert);
  fileMenu->addSeparator();

  auto fileImport = new QAction(this);
  fileImport->setData(-1);
  fileImport->setStatusTip(tr("Import from file or clipboard"));
  fileImport->setText(tr("&Import..."));
  fileImport->setIcon(QIcon::fromTheme(QLatin1String("document-import"),
      QIcon(QLatin1String(":/images/document-import.png"))));
  fileImport->setObjectName(QLatin1String("import"));
  m_shortcutsModel->registerAction(fileImport, menuTitle);
  connect(fileImport, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotImport);
  fileMenu->addAction(fileImport);

  int importerIdx = 0;
  const auto sis = app()->getServerImporters();
  for (const ServerImporter* si : sis) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(
          si->name()).toLower().remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    auto fileImportServer = new QAction(this);
    fileImportServer->setData(importerIdx);
    fileImportServer->setStatusTip(tr("Import from %1").arg(serverName));
    fileImportServer->setText(tr("Import from %1...").arg(serverName));
    fileImportServer->setObjectName(actionName);
    m_shortcutsModel->registerAction(fileImportServer, menuTitle);
    connect(fileImportServer, &QAction::triggered,
      impl(), &BaseMainWindowImpl::slotImport);
    fileMenu->addAction(fileImportServer);
    ++importerIdx;
  }

  const auto stis = app()->getServerTrackImporters();
  for (const ServerTrackImporter* si : stis) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(
          si->name()).toLower().remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    auto fileImportServer = new QAction(this);
    fileImportServer->setData(importerIdx);
    fileImportServer->setStatusTip(tr("Import from %1").arg(serverName));
    fileImportServer->setText(tr("Import from %1...").arg(serverName));
    fileImportServer->setObjectName(actionName);
    m_shortcutsModel->registerAction(fileImportServer, menuTitle);
    connect(fileImportServer, &QAction::triggered,
      impl(), &BaseMainWindowImpl::slotImport);
    fileMenu->addAction(fileImportServer);
    ++importerIdx;
  }

  auto fileTagImport = new QAction(this);
  fileTagImport->setStatusTip(tr("Import from Tags"));
  fileTagImport->setText(tr("Import from Tags..."));
  fileTagImport->setObjectName(QLatin1String("import_tags"));
  m_shortcutsModel->registerAction(fileTagImport, menuTitle);
  connect(fileTagImport, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotTagImport);
  fileMenu->addAction(fileTagImport);

  auto fileBatchImport = new QAction(this);
  fileBatchImport->setStatusTip(tr("Automatic import"));
  fileBatchImport->setText(tr("Automatic I&mport..."));
  fileBatchImport->setObjectName(QLatin1String("batch_import"));
  m_shortcutsModel->registerAction(fileBatchImport, menuTitle);
  connect(fileBatchImport, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotBatchImport);
  fileMenu->addAction(fileBatchImport);

  auto fileBrowseCoverArt = new QAction(this);
  fileBrowseCoverArt->setStatusTip(tr("Browse album cover artwork"));
  fileBrowseCoverArt->setText(tr("&Browse Cover Art..."));
  fileBrowseCoverArt->setObjectName(QLatin1String("browse_cover_art"));
  m_shortcutsModel->registerAction(fileBrowseCoverArt, menuTitle);
  connect(fileBrowseCoverArt, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotBrowseCoverArt);
  fileMenu->addAction(fileBrowseCoverArt);
  fileMenu->addSeparator();

  auto fileExport = new QAction(this);
  fileExport->setStatusTip(tr("Export to file or clipboard"));
  fileExport->setText(tr("&Export..."));
  fileExport->setIcon(QIcon::fromTheme(QLatin1String("document-export"),
      QIcon(QLatin1String(":/images/document-export.png"))));
  fileExport->setObjectName(QLatin1String("export"));
  m_shortcutsModel->registerAction(fileExport, menuTitle);
  connect(fileExport, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotExport);
  fileMenu->addAction(fileExport);

  auto fileCreatePlaylist = new QAction(this);
  fileCreatePlaylist->setStatusTip(tr("Create M3U Playlist"));
  fileCreatePlaylist->setText(tr("&Create Playlist..."));
  fileCreatePlaylist->setIcon(
        QIcon::fromTheme(QLatin1String("format-justify-fill"),
            QIcon(QLatin1String(":/images/view-media-playlist.png"))));
  fileCreatePlaylist->setObjectName(QLatin1String("create_playlist"));
  m_shortcutsModel->registerAction(fileCreatePlaylist, menuTitle);
  connect(fileCreatePlaylist, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotPlaylistDialog);
  fileMenu->addAction(fileCreatePlaylist);
  toolBar->addAction(fileCreatePlaylist);
  fileMenu->addSeparator();

  auto fileQuit = new QAction(this);
  fileQuit->setStatusTip(tr("Quits the application"));
  fileQuit->setText(tr("&Quit"));
  fileQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
  fileQuit->setIcon(QIcon::fromTheme(QLatin1String("application-exit"),
      QIcon(QLatin1String(":/images/application-exit.png"))));
  fileQuit->setMenuRole(QAction::QuitRole);
  fileQuit->setObjectName(QLatin1String("file_quit"));
  m_shortcutsModel->registerAction(fileQuit, menuTitle);
  connect(fileQuit, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotFileQuit);
  fileMenu->addAction(fileQuit);

  menuTitle = tr("&Edit");
  QMenu* editMenu = menubar->addMenu(menuTitle);
  auto editSelectAll = new QAction(this);
  editSelectAll->setStatusTip(tr("Select all files"));
  editSelectAll->setText(tr("Select &All"));
  editSelectAll->setShortcut(Qt::ALT + Qt::Key_A);
  editSelectAll->setIcon(QIcon::fromTheme(QLatin1String("edit-select-all"),
      QIcon(QLatin1String(":/images/edit-select-all.png"))));
#if defined Q_OS_MAC && QT_VERSION == 0x050400
  // Workaround for QTBUG-43471 from QTBUG-39934
  editSelectAll->setMenuRole(QAction::NoRole);
#endif
  editSelectAll->setObjectName(QLatin1String("edit_select_all"));
  m_shortcutsModel->registerAction(editSelectAll, menuTitle);
  connect(editSelectAll, &QAction::triggered,
          form(), &Kid3Form::selectAllFiles);
  editMenu->addAction(editSelectAll);

  auto editDeselect = new QAction(this);
  editDeselect->setStatusTip(tr("Deselect all files"));
  editDeselect->setText(tr("Dese&lect"));
  editDeselect->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);
  editDeselect->setObjectName(QLatin1String("edit_deselect"));
  m_shortcutsModel->registerAction(editDeselect, menuTitle);
  connect(editDeselect, &QAction::triggered,
          form(), &Kid3Form::deselectAllFiles);
  editMenu->addAction(editDeselect);

  auto editSelectAllInDir = new QAction(this);
  editSelectAllInDir->setStatusTip(
        tr("Select all files in the current directory"));
  editSelectAllInDir->setText(tr("Select All in &Directory"));
  editSelectAllInDir->setObjectName(QLatin1String("select_all_in_directory"));
  m_shortcutsModel->registerAction(editSelectAllInDir, menuTitle);
  connect(editSelectAllInDir, &QAction::triggered,
    app(), &Kid3Application::selectAllInDirectory);
  editMenu->addAction(editSelectAllInDir);

  auto editPreviousFile = new QAction(this);
  editPreviousFile->setStatusTip(tr("Select previous file"));
  editPreviousFile->setText(tr("&Previous File"));
  editPreviousFile->setShortcut(Qt::ALT + Qt::Key_Up);
  editPreviousFile->setIcon(QIcon::fromTheme(QLatin1String("go-previous"),
      QIcon(QLatin1String(":/images/go-previous.png"))));
  editPreviousFile->setObjectName(QLatin1String("previous_file"));
  m_shortcutsModel->registerAction(editPreviousFile, menuTitle);
  connect(editPreviousFile, &QAction::triggered,
    form(), &Kid3Form::selectPreviousTaggedFile);
  editMenu->addAction(editPreviousFile);
  toolBar->addAction(editPreviousFile);

  auto editNextFile = new QAction(this);
  editNextFile->setStatusTip(tr("Select next file"));
  editNextFile->setText(tr("&Next File"));
  editNextFile->setShortcut(Qt::ALT + Qt::Key_Down);
  editNextFile->setIcon(QIcon::fromTheme(QLatin1String("go-next"),
      QIcon(QLatin1String(":/images/go-next.png"))));
  editNextFile->setObjectName(QLatin1String("next_file"));
  m_shortcutsModel->registerAction(editNextFile, menuTitle);
  connect(editNextFile, &QAction::triggered,
    form(), &Kid3Form::selectNextTaggedFile);
  editMenu->addAction(editNextFile);
  toolBar->addAction(editNextFile);
  editMenu->addSeparator();

  auto editFind = new QAction(this);
  editFind->setStatusTip(tr("Find"));
  editFind->setText(tr("&Find..."));
  editFind->setShortcut(QKeySequence::Find);
#ifndef Q_OS_MAC
  editFind->setIcon(QIcon::fromTheme(QLatin1String("edit-find"),
      QIcon(QLatin1String(":/images/edit-find.png"))));
#endif
  editFind->setObjectName(QLatin1String("edit_find"));
  m_shortcutsModel->registerAction(editFind, menuTitle);
  connect(editFind, &QAction::triggered,
    impl(), &BaseMainWindowImpl::find);
  editMenu->addAction(editFind);

  auto editReplace = new QAction(this);
  editReplace->setStatusTip(tr("Find and replace"));
  editReplace->setText(tr("&Replace..."));
  editReplace->setShortcut(QKeySequence::Replace);
#ifndef Q_OS_MAC
  editReplace->setIcon(QIcon::fromTheme(QLatin1String("edit-find-replace"),
      QIcon(QLatin1String(":/images/edit-find-replace.png"))));
#endif
  editReplace->setObjectName(QLatin1String("edit_replace"));
  m_shortcutsModel->registerAction(editReplace, menuTitle);
  connect(editReplace, &QAction::triggered,
    impl(), &BaseMainWindowImpl::findReplace);
  editMenu->addAction(editReplace);

  menuTitle = tr("&Tools");
  QMenu* toolsMenu = menubar->addMenu(menuTitle);
  auto toolsApplyFilenameFormat = new QAction(this);
  toolsApplyFilenameFormat->setStatusTip(tr("Apply Filename Format"));
  toolsApplyFilenameFormat->setText(tr("Apply &Filename Format"));
  toolsApplyFilenameFormat->setObjectName(
        QLatin1String("apply_filename_format"));
  m_shortcutsModel->registerAction(toolsApplyFilenameFormat, menuTitle);
  connect(toolsApplyFilenameFormat, &QAction::triggered,
    app(), &Kid3Application::applyFilenameFormat);
  toolsMenu->addAction(toolsApplyFilenameFormat);

  auto toolsApplyTagFormat = new QAction(this);
  toolsApplyTagFormat->setStatusTip(tr("Apply Tag Format"));
  toolsApplyTagFormat->setText(tr("Apply &Tag Format"));
  toolsApplyTagFormat->setObjectName(QLatin1String("apply_id3_format"));
  m_shortcutsModel->registerAction(toolsApplyTagFormat, menuTitle);
  connect(toolsApplyTagFormat, &QAction::triggered,
    app(), &Kid3Application::applyTagFormat);
  toolsMenu->addAction(toolsApplyTagFormat);

  auto toolsApplyTextEncoding = new QAction(this);
  toolsApplyTextEncoding->setStatusTip(tr("Apply Text Encoding"));
  toolsApplyTextEncoding->setText(tr("Apply Text &Encoding"));
  toolsApplyTextEncoding->setObjectName(QLatin1String("apply_text_encoding"));
  m_shortcutsModel->registerAction(toolsApplyTextEncoding, menuTitle);
  connect(toolsApplyTextEncoding, &QAction::triggered,
    app(), &Kid3Application::applyTextEncoding);
  toolsMenu->addAction(toolsApplyTextEncoding);
  toolsMenu->addSeparator();

  auto toolsRenameDirectory = new QAction(this);
  toolsRenameDirectory->setStatusTip(tr("Rename Directory"));
  toolsRenameDirectory->setText(tr("&Rename Directory..."));
  toolsRenameDirectory->setObjectName(QLatin1String("rename_directory"));
  m_shortcutsModel->registerAction(toolsRenameDirectory, menuTitle);
  connect(toolsRenameDirectory, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotRenameDirectory);
  toolsMenu->addAction(toolsRenameDirectory);

  auto toolsNumberTracks = new QAction(this);
  toolsNumberTracks->setStatusTip(tr("Number Tracks"));
  toolsNumberTracks->setText(tr("&Number Tracks..."));
  toolsNumberTracks->setObjectName(QLatin1String("number_tracks"));
  m_shortcutsModel->registerAction(toolsNumberTracks, menuTitle);
  connect(toolsNumberTracks, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotNumberTracks);
  toolsMenu->addAction(toolsNumberTracks);

  auto toolsFilter = new QAction(this);
  toolsFilter->setStatusTip(tr("Filter"));
  toolsFilter->setText(tr("F&ilter..."));
  toolsFilter->setObjectName(QLatin1String("filter"));
  m_shortcutsModel->registerAction(toolsFilter, menuTitle);
  connect(toolsFilter, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotFilter);
  toolsMenu->addAction(toolsFilter);
  toolsMenu->addSeparator();

  const TagConfig& tagCfg = TagConfig::instance();
  if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v24) {
    auto toolsConvertToId3v24 = new QAction(this);
    toolsConvertToId3v24->setStatusTip(tr("Convert ID3v2.3 to ID3v2.4"));
    toolsConvertToId3v24->setText(tr("Convert ID3v2.3 to ID3v2.&4"));
    toolsConvertToId3v24->setObjectName(QLatin1String("convert_to_id3v24"));
    m_shortcutsModel->registerAction(toolsConvertToId3v24, menuTitle);
    connect(toolsConvertToId3v24, &QAction::triggered,
      app(), &Kid3Application::convertToId3v24);
    toolsMenu->addAction(toolsConvertToId3v24);

    if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v23) {
      auto toolsConvertToId3v23 = new QAction(this);
      toolsConvertToId3v23->setStatusTip(tr("Convert ID3v2.4 to ID3v2.3"));
      toolsConvertToId3v23->setText(tr("Convert ID3v2.4 to ID3v2.&3"));
      toolsConvertToId3v23->setObjectName(QLatin1String("convert_to_id3v23"));
      m_shortcutsModel->registerAction(toolsConvertToId3v23, menuTitle);
      connect(toolsConvertToId3v23, &QAction::triggered,
        app(), &Kid3Application::convertToId3v23);
      toolsMenu->addAction(toolsConvertToId3v23);
    }
  }

  toolsMenu->addSeparator();
  auto toolsPlay = new QAction(this);
  toolsPlay->setStatusTip(tr("Play"));
  toolsPlay->setText(tr("&Play"));
  toolsPlay->setIcon(QIcon(style()->standardIcon(QStyle::SP_MediaPlay)));
  toolsPlay->setObjectName(QLatin1String("play"));
  m_shortcutsModel->registerAction(toolsPlay, menuTitle);
  connect(toolsPlay, &QAction::triggered,
    app(), &Kid3Application::playAudio);
  toolsMenu->addAction(toolsPlay);
  toolBar->addAction(toolsPlay);

  menuTitle = tr("&Settings");
  QMenu* settingsMenu = menubar->addMenu(menuTitle);
  m_viewToolBar = toolBar->toggleViewAction();
  if (m_viewToolBar) {
    m_viewToolBar->setStatusTip(tr("Enables/disables the toolbar"));
    m_viewToolBar->setText(tr("Show &Toolbar"));
    m_viewToolBar->setObjectName(QLatin1String("options_configure_toolbars"));
    m_shortcutsModel->registerAction(m_viewToolBar, menuTitle);
    m_viewToolBar->setChecked(!MainWindowConfig::instance().hideToolBar());
    settingsMenu->addAction(m_viewToolBar);
  }
  if (MainWindowConfig::instance().hideToolBar())
    toolBar->hide();

  m_viewStatusBar = new QAction(this);
  m_viewStatusBar->setStatusTip(tr("Enables/disables the statusbar"));
  m_viewStatusBar->setText(tr("Show St&atusbar"));
  m_viewStatusBar->setCheckable(true);
  m_viewStatusBar->setObjectName(QLatin1String("options_show_statusbar"));
  m_shortcutsModel->registerAction(m_viewStatusBar, menuTitle);
  connect(m_viewStatusBar, &QAction::triggered,
    this, &Kid3MainWindow::slotViewStatusBar);
  settingsMenu->addAction(m_viewStatusBar);

  m_settingsShowHidePicture = new QAction(this);
  m_settingsShowHidePicture->setStatusTip(tr("Show Picture"));
  m_settingsShowHidePicture->setText(tr("Show &Picture"));
  m_settingsShowHidePicture->setCheckable(true);
  m_settingsShowHidePicture->setObjectName(QLatin1String("hide_picture"));
  m_shortcutsModel->registerAction(m_settingsShowHidePicture, menuTitle);
  connect(m_settingsShowHidePicture, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotSettingsShowHidePicture);
  settingsMenu->addAction(m_settingsShowHidePicture);

  m_settingsAutoHideTags = new QAction(this);
  m_settingsAutoHideTags->setStatusTip(tr("Auto Hide Tags"));
  m_settingsAutoHideTags->setText(tr("Auto &Hide Tags"));
  m_settingsAutoHideTags->setCheckable(true);
  m_settingsAutoHideTags->setObjectName(QLatin1String("auto_hide_tags"));
  m_shortcutsModel->registerAction(m_settingsAutoHideTags, menuTitle);
  connect(m_settingsAutoHideTags, &QAction::triggered,
    impl(), &BaseMainWindowImpl::slotSettingsAutoHideTags);
  settingsMenu->addAction(m_settingsAutoHideTags);

  auto settingsConfigure = new QAction(this);
  settingsConfigure->setStatusTip(tr("Configure Kid3"));
  settingsConfigure->setText(tr("&Configure Kid3..."));
  settingsConfigure->setIcon(QIcon::fromTheme(
      QLatin1String("preferences-system"),
      QIcon(QLatin1String(":/images/preferences-system.png"))));
  settingsConfigure->setShortcut(QKeySequence::Preferences);
  settingsConfigure->setMenuRole(QAction::PreferencesRole);
  settingsConfigure->setObjectName(QLatin1String("options_configure"));
  m_shortcutsModel->registerAction(settingsConfigure, menuTitle);
  connect(settingsConfigure, &QAction::triggered,
    this, &Kid3MainWindow::slotSettingsConfigure);
  settingsMenu->addSeparator();
  settingsMenu->addAction(settingsConfigure);
  toolBar->addAction(settingsConfigure);

  menuTitle = tr("&Help");
  QMenu* helpMenu = menubar->addMenu(menuTitle);
  auto helpHandbook = new QAction(this);
  helpHandbook->setStatusTip(tr("Kid3 Handbook"));
  helpHandbook->setText(tr("Kid3 &Handbook"));
  helpHandbook->setIcon(QIcon::fromTheme(QLatin1String("help-contents"),
      QIcon(QLatin1String(":/images/help-contents.png"))));
  helpHandbook->setShortcut(QKeySequence::HelpContents);
  helpHandbook->setObjectName(QLatin1String("help_contents"));
  m_shortcutsModel->registerAction(helpHandbook, menuTitle);
  connect(helpHandbook, &QAction::triggered,
    this, &Kid3MainWindow::slotHelpHandbook);
  helpMenu->addAction(helpHandbook);

  auto helpAbout = new QAction(this);
  helpAbout->setStatusTip(tr("About Kid3"));
  helpAbout->setText(tr("&About Kid3"));
#ifndef Q_OS_MAC
  helpAbout->setIcon(QIcon::fromTheme(QLatin1String("help-about"),
      QIcon(QLatin1String(":/images/help-about.png"))));
#endif
  helpAbout->setMenuRole(QAction::AboutRole);
  helpAbout->setObjectName(QLatin1String("help_about_app"));
  m_shortcutsModel->registerAction(helpAbout, menuTitle);
  connect(helpAbout, &QAction::triggered,
    this, &Kid3MainWindow::slotHelpAbout);
  helpMenu->addAction(helpAbout);

  auto helpAboutQt = new QAction(this);
  helpAboutQt->setStatusTip(tr("About Qt"));
  helpAboutQt->setText(tr("About &Qt"));
  helpAboutQt->setMenuRole(QAction::AboutQtRole);
  helpAboutQt->setObjectName(QLatin1String("help_about_qt"));
  m_shortcutsModel->registerAction(helpAboutQt, menuTitle);
  connect(helpAboutQt, &QAction::triggered,
    this, &Kid3MainWindow::slotHelpAboutQt);
  helpMenu->addAction(helpAboutQt);

  addToolBar(toolBar);

  updateWindowCaption();

  initFormActions();

  FileList* fileList = form()->getFileList();
  connect(fileList, &FileList::userActionAdded,
          this, &Kid3MainWindow::onUserActionAdded);
  connect(fileList, &FileList::userActionRemoved,
          this, &Kid3MainWindow::onUserActionRemoved);
  fileList->initUserActions();
  const UserActionsConfig& userActionsCfg = UserActionsConfig::instance();
  connect(&userActionsCfg, &UserActionsConfig::contextMenuCommandsChanged,
          fileList, &FileList::initUserActions);
}

/**
 * Init actions of form.
 */
void Kid3MainWindow::initFormActions()
{
  QString ctx(tr("Filename"));
  FOR_ALL_TAGS(tagNr) {
    QString tagStr = Frame::tagNumberToString(tagNr);
    connect(addFormAction(tr("From Tag %1").arg(tagStr),
                          QLatin1String("filename_from_v") + tagStr, ctx),
            &QAction::triggered,
            app()->tag(tagNr), &Kid3ApplicationTagContext::getFilenameFromTags);
  }
  connect(addFormAction(tr("Focus"), QLatin1String("filename_focus"), ctx),
          &QAction::triggered,
          form(), &Kid3Form::setFocusFilename);
  FOR_ALL_TAGS(tagNr) {
    Frame::TagNumber otherTagNr = tagNr == Frame::Tag_1
        ? Frame::Tag_2
        : tagNr == Frame::Tag_2 ? Frame::Tag_1 : Frame::Tag_NumValues;
    QString tagStr = Frame::tagNumberToString(tagNr);
    Kid3ApplicationTagContext* appTag = app()->tag(tagNr);
    Kid3FormTagContext* formTag = form()->tag(tagNr);
    ctx = tr("Tag %1").arg(tagStr);
    tagStr = QLatin1Char('v') + tagStr + QLatin1Char('_');
    connect(addFormAction(tr("From Filename"),
                          tagStr + QLatin1String("from_filename"), ctx),
            &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::getTagsFromFilename);
    if (otherTagNr < Frame::Tag_NumValues) {
      QString otherTagStr = Frame::tagNumberToString(otherTagNr);
      connect(addFormAction(tr("From Tag %1").arg(otherTagStr),
                            tagStr + QLatin1String("from_v") + otherTagStr, ctx),
              &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::copyToOtherTag);
    }
    connect(addFormAction(tr("Copy"), tagStr + QLatin1String("copy"), ctx),
            &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::copyTags);
    connect(addFormAction(tr("Paste"), tagStr + QLatin1String("paste"), ctx),
            &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::pasteTags);
    connect(addFormAction(tr("Remove"), tagStr + QLatin1String("remove"), ctx),
            &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::removeTags);
    if (tagNr != Frame::Tag_Id3v1) {
      connect(addFormAction(tr("Edit"), tagStr + QLatin1String("frames_edit"), ctx),
              &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::editFrame);
      connect(addFormAction(tr("Add"), tagStr + QLatin1String("frames_add"), ctx),
              &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::addFrame);
      connect(addFormAction(tr("Delete"), tagStr + QLatin1String("frames_delete"), ctx),
              &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::deleteFrame);
    }
    connect(addFormAction(tr("Focus"), tagStr + QLatin1String("focus"), ctx),
            &QAction::triggered,
            formTag, &Kid3FormTagContext::setFocusTag);
  }
  ctx = tr("File List");
  connect(addFormAction(tr("Focus"), QLatin1String("filelist_focus"), ctx),
          &QAction::triggered,
          form(), &Kid3Form::setFocusFileList);
  QAction* renameAction = new QAction(tr("&Rename"), this);
  renameAction->setObjectName(QLatin1String("filelist_rename"));
  renameAction->setShortcut(QKeySequence(Qt::Key_F2));
  renameAction->setShortcutContext(Qt::WidgetShortcut);
  connect(renameAction, &QAction::triggered, impl(), &BaseMainWindowImpl::renameFile);
  form()->getFileList()->setRenameAction(renameAction);
  m_shortcutsModel->registerAction(renameAction, ctx);
  QAction* deleteAction = new QAction(tr("&Move to Trash"), this);
  deleteAction->setObjectName(QLatin1String("filelist_delete"));
  deleteAction->setShortcut(QKeySequence::Delete);
  deleteAction->setShortcutContext(Qt::WidgetShortcut);
  connect(deleteAction, &QAction::triggered, impl(), &BaseMainWindowImpl::deleteFile);
  form()->getFileList()->setDeleteAction(deleteAction);
  m_shortcutsModel->registerAction(deleteAction, ctx);
  ctx = tr("Directory List");
  connect(addFormAction(tr("Focus"), QLatin1String("dirlist_focus"), ctx),
          &QAction::triggered,
          form(), &Kid3Form::setFocusDirList);
}

/**
 * Add action of form.
 */
QAction* Kid3MainWindow::addFormAction(const QString& text, const QString& name,
                                       const QString& context)
{
  auto action = new QAction(form());
  action->setStatusTip(text);
  action->setText(text);
  action->setObjectName(name);
  m_shortcutsModel->registerAction(action, context);
  addAction(action);
  return action;
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
  const MainWindowConfig& mainWindowConfig = MainWindowConfig::instance();
  setStatusBarVisible(!mainWindowConfig.hideStatusBar());
  m_viewStatusBar->setChecked(!mainWindowConfig.hideStatusBar());
  m_settingsShowHidePicture->setChecked(!GuiConfig::instance().hidePicture());
  m_settingsAutoHideTags->setChecked(GuiConfig::instance().autoHideTags());
  m_fileOpenRecent->loadEntries(app()->getSettings());
  m_shortcutsModel->readFromConfig(app()->getSettings());
  if (!mainWindowConfig.geometry().isEmpty()) {
    restoreGeometry(mainWindowConfig.geometry());
  } else {
    resize(1000, 900);
  }
  if (!mainWindowConfig.windowState().isEmpty()) {
    restoreState(mainWindowConfig.windowState());
  }
}

/**
 * Store geometry and recent files in settings.
 */
void Kid3MainWindow::saveConfig()
{
  MainWindowConfig& mainWindowConfig = MainWindowConfig::instance();
  m_fileOpenRecent->saveEntries(app()->getSettings());
  m_shortcutsModel->writeToConfig(app()->getSettings());
  mainWindowConfig.setHideToolBar(!m_viewToolBar->isChecked());
  mainWindowConfig.setGeometry(saveGeometry());
  mainWindowConfig.setWindowState(saveState());
  mainWindowConfig.writeToConfig(app()->getSettings());
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
  const MainWindowConfig& mainWindowConfig = MainWindowConfig::instance();
  if (mainWindowConfig.useFont() &&
      !mainWindowConfig.fontFamily().isEmpty() &&
      mainWindowConfig.fontSize() > 0) {
    QApplication::setFont(QFont(mainWindowConfig.fontFamily(),
                                mainWindowConfig.fontSize()));
  }
  if (!mainWindowConfig.style().isEmpty()) {
    QApplication::setStyle(mainWindowConfig.style());
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
  MainWindowConfig::instance().setHideStatusBar(!m_viewStatusBar->isChecked());
  setStatusBarVisible(!MainWindowConfig::instance().hideStatusBar());
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
    QLatin1String("<big><b>Kid3 " VERSION
    "</b></big><br/><br/>") +
    tr("Audio Tag Editor") +
    QLatin1String("<br/><br/>(c) 2003-" RELEASE_YEAR
    " <a href=\"mailto:ufleisch@users.sourceforge.net\">Urs Fleisch</a>"
    "<br/><br/>"
    "<a href=\"http://kid3.sourceforge.net/\">http://kid3.sourceforge.net</a>"
    "<br/>") + tr("License") +
    QLatin1String(": <a href=\"http://www.gnu.org/licenses/licenses.html#GPL\">"
    "GNU General Public License</a><br/> "));
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
  auto dialog = new ConfigDialog(m_platformTools, this, caption,
                                          m_shortcutsModel);
  dialog->setConfig();
  if (dialog->exec() == QDialog::Accepted) {
    dialog->getConfig();
    impl()->applyChangedConfiguration();
  }
}

/**
 * Called when session manager wants application to commit its data.
 * @param manager session manager
 */
void Kid3MainWindow::onCommitDataRequest(QSessionManager& manager)
{
  // Make sure that current file is saved even if "load last opened file"
  // is not enabled.
  FileConfig& fileCfg = FileConfig::instance();
  if (!fileCfg.loadLastOpenedFile()) {
    fileCfg.setLastOpenedFile(
        app()->getFileProxyModel()->filePath(app()->currentOrRootIndex()));
  }

  // Ask user if there are unsaved data.
  if (manager.allowsInteraction()) {
    if (queryBeforeClosing()) {
      manager.release();
    } else {
      manager.cancel();
    }
  }
}

/**
 * Add user action to shortcuts.
 * @param name name of action
 * @param action action to add
 */
void Kid3MainWindow::onUserActionAdded(const QString& name, QAction* action)
{
  action->setObjectName(name);
  action->setShortcutContext(Qt::ApplicationShortcut);
  m_shortcutsModel->registerAction(action, tr("User Actions"));
  addAction(action);
}

/**
 * Remove user action from shortcuts.
 * @param name name of action
 * @param action action to remove
 */
void Kid3MainWindow::onUserActionRemoved(const QString& name, QAction* action)
{
  Q_UNUSED(name)
  m_shortcutsModel->unregisterAction(action, tr("User Actions"));
  removeAction(action);
}
