/**
 * \file kdemainwindow.cpp
 * KDE Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2014  Urs Fleisch
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

#include "kdemainwindow.h"
#if QT_VERSION >= 0x050000
#include <KConfig>
#include <KToggleAction>
#include <KStandardAction>
#include <KShortcutsDialog>
#include <KRecentFilesAction>
#include <KActionCollection>
#include <KEditToolBar>
#include <KConfigSkeleton>
#include <QApplication>
#include <QUrl>
#include <QAction>

#define KUrl QUrl
#define KAction QAction
#define KShortcut QKeySequence
#define KIcon QIcon::fromTheme
#define KSharedConfig_openConfig KSharedConfig::openConfig

#else
#include <kapplication.h>
#include <kurl.h>
#include <kconfig.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kshortcutsdialog.h>
#include <krecentfilesaction.h>
#include <kactioncollection.h>
#include <kedittoolbar.h>
#include <kconfigskeleton.h>

/** Create object for configuration file. */
#define KSharedConfig_openConfig KGlobal::config

#endif
#include <QAction>
#include "config.h"
#include "qtcompatmac.h"
#include "kid3form.h"
#include "filelist.h"
#include "kid3application.h"
#include "kdeconfigdialog.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "serverimporter.h"
#include "servertrackimporter.h"

/**
 * Constructor.
 *
 * @param platformTools platform specific tools
 * @param app application context
 * @param parent parent widget
 */
KdeMainWindow::KdeMainWindow(IPlatformTools* platformTools,
                             Kid3Application* app, QWidget* parent) :
  KXmlGuiWindow(parent),
  BaseMainWindow(this, platformTools, app),
  m_fileOpenRecent(0),
  m_settingsAutoHideTags(0), m_settingsShowHidePicture(0)
{
  init();
}

/**
 * Destructor.
 */
KdeMainWindow::~KdeMainWindow()
{
}

/** Only defined for generation of translation files */
#define MAIN_TOOLBAR_FOR_PO QT_TRANSLATE_NOOP("@default", "Main Toolbar")

/**
 * Init menu and toolbar actions.
 */
void KdeMainWindow::initActions()
{
  KAction* fileOpen = KStandardAction::open(
      impl(), SLOT(slotFileOpen()), actionCollection());
  m_fileOpenRecent = KStandardAction::openRecent(
      this,
#if QT_VERSION >= 0x050000
        SLOT(slotFileOpenRecentUrl(QUrl)),
#else
        SLOT(slotFileOpenRecentUrl(KUrl)),
#endif
      actionCollection());
  KAction* fileRevert = KStandardAction::revert(
      app(), SLOT(revertFileModifications()), actionCollection());
  KAction* fileSave = KStandardAction::save(
      impl(), SLOT(slotFileSave()), actionCollection());
  KAction* fileQuit = KStandardAction::quit(
      impl(), SLOT(slotFileQuit()), actionCollection());
  KAction* editSelectAll = KStandardAction::selectAll(
      form(), SLOT(selectAllFiles()), actionCollection());
  KAction* editDeselect = KStandardAction::deselect(
      form(), SLOT(deselectAllFiles()), actionCollection());
  KAction* editFind = KStandardAction::find(
      impl(), SLOT(find()), actionCollection());
  KAction* editReplace = KStandardAction::replace(
      impl(), SLOT(findReplace()), actionCollection());
  setStandardToolBarMenuEnabled(true);
  createStandardStatusBarAction();
  KAction* settingsShortcuts = KStandardAction::keyBindings(
    this, SLOT(slotSettingsShortcuts()), actionCollection());
  KAction* settingsToolbars = KStandardAction::configureToolbars(
    this, SLOT(slotSettingsToolbars()), actionCollection());
  KAction* settingsConfigure = KStandardAction::preferences(
      this, SLOT(slotSettingsConfigure()), actionCollection());

  fileOpen->setStatusTip(tr("Open files"));
  m_fileOpenRecent->setStatusTip(tr("Opens a recently used directory"));
  fileRevert->setStatusTip(
      tr("Reverts the changes of all or the selected files"));
#if QT_VERSION >= 0x050000
  actionCollection()->setDefaultShortcuts(fileRevert,
                          KStandardShortcut::shortcut(KStandardShortcut::Undo));
#else
  fileRevert->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Undo));
#endif
  fileSave->setStatusTip(tr("Saves the changed files"));
  fileQuit->setStatusTip(tr("Quits the application"));
  editSelectAll->setStatusTip(tr("Select all files"));
  editSelectAll->setShortcut(KShortcut(QLatin1String("Alt+Shift+A")));
  editDeselect->setStatusTip(tr("Deselect all files"));
  editFind->setStatusTip(tr("Find"));
  editReplace->setStatusTip(tr("Find and replace"));
  settingsShortcuts->setStatusTip(tr("Configure Shortcuts"));
  settingsToolbars->setStatusTip(tr("Configure Toolbars"));
  settingsConfigure->setStatusTip(tr("Preferences dialog"));

  KAction* fileOpenDirectory = new KAction(KIcon(QLatin1String("document-open")), tr("O&pen Directory..."), this);
  fileOpenDirectory->setStatusTip(tr("Opens a directory"));
  fileOpenDirectory->setShortcut(KShortcut(QLatin1String("Ctrl+D")));
  actionCollection()->addAction(QLatin1String("open_directory"), fileOpenDirectory);
  connect(fileOpenDirectory, SIGNAL(triggered()), impl(), SLOT(slotFileOpenDirectory()));
  KAction* fileImport = new KAction(KIcon(QLatin1String("document-import")), tr("&Import..."), this);
  fileImport->setStatusTip(tr("Import from file or clipboard"));
  fileImport->setData(-1);
  actionCollection()->addAction(QLatin1String("import"), fileImport);
  connect(fileImport, SIGNAL(triggered()), impl(), SLOT(slotImport()));

  int importerIdx = 0;
  foreach (const ServerImporter* si, app()->getServerImporters()) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower().remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    KAction* fileImportServer =
        new KAction(tr("Import from %1...").arg(serverName), this);
    fileImportServer->setData(importerIdx);
    fileImportServer->setStatusTip(tr("Import from %1").arg(serverName));
    actionCollection()->addAction(actionName, fileImportServer);
    connect(fileImportServer, SIGNAL(triggered()), impl(), SLOT(slotImport()));
    ++importerIdx;
  }

  foreach (const ServerTrackImporter* si, app()->getServerTrackImporters()) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower().remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    KAction* fileImportServer =
        new KAction(tr("Import from %1...").arg(serverName), this);
    fileImportServer->setStatusTip(tr("Import from %1").arg(serverName));
    fileImportServer->setData(importerIdx);
    actionCollection()->addAction(actionName, fileImportServer);
    connect(fileImportServer, SIGNAL(triggered()), impl(), SLOT(slotImport()));
    ++importerIdx;
  }

  KAction* fileBatchImport = new KAction(tr("Automatic I&mport..."), this);
  fileBatchImport->setStatusTip(tr("Automatic import"));
  actionCollection()->addAction(QLatin1String("batch_import"), fileBatchImport);
  connect(fileBatchImport, SIGNAL(triggered()), impl(), SLOT(slotBatchImport()));

  KAction* fileBrowseCoverArt = new KAction(tr("&Browse Cover Art..."), this);
  fileBrowseCoverArt->setStatusTip(tr("Browse album cover artwork"));
  actionCollection()->addAction(QLatin1String("browse_cover_art"), fileBrowseCoverArt);
  connect(fileBrowseCoverArt, SIGNAL(triggered()), impl(), SLOT(slotBrowseCoverArt()));
  KAction* fileExport = new KAction(KIcon(QLatin1String("document-export")), tr("&Export..."), this);
  fileExport->setStatusTip(tr("Export to file or clipboard"));
  actionCollection()->addAction(QLatin1String("export"), fileExport);
  connect(fileExport, SIGNAL(triggered()), impl(), SLOT(slotExport()));
  KAction* fileCreatePlaylist = new KAction(KIcon(QLatin1String("view-media-playlist")), tr("&Create Playlist..."), this);
  fileCreatePlaylist->setStatusTip(tr("Create M3U Playlist"));
  actionCollection()->addAction(QLatin1String("create_playlist"), fileCreatePlaylist);
  connect(fileCreatePlaylist, SIGNAL(triggered()), impl(), SLOT(slotPlaylistDialog()));
  KAction* toolsApplyFilenameFormat = new KAction(tr("Apply &Filename Format"), this);
  toolsApplyFilenameFormat->setStatusTip(tr("Apply Filename Format"));
  actionCollection()->addAction(QLatin1String("apply_filename_format"), toolsApplyFilenameFormat);
  connect(toolsApplyFilenameFormat, SIGNAL(triggered()), app(), SLOT(applyFilenameFormat()));
  KAction* toolsApplyTagFormat = new KAction(tr("Apply &Tag Format"), this);
  toolsApplyTagFormat->setStatusTip(tr("Apply Tag Format"));
  actionCollection()->addAction(QLatin1String("apply_id3_format"), toolsApplyTagFormat);
  connect(toolsApplyTagFormat, SIGNAL(triggered()), app(), SLOT(applyTagFormat()));
  KAction* toolsApplyTextEncoding = new KAction(tr("Apply Text &Encoding"), this);
  toolsApplyTextEncoding->setStatusTip(tr("Apply Text Encoding"));
  actionCollection()->addAction(QLatin1String("apply_text_encoding"), toolsApplyTextEncoding);
  connect(toolsApplyTextEncoding, SIGNAL(triggered()), app(), SLOT(applyTextEncoding()));
  KAction* toolsRenameDirectory = new KAction(tr("&Rename Directory..."), this);
  toolsRenameDirectory->setStatusTip(tr("Rename Directory"));
  actionCollection()->addAction(QLatin1String("rename_directory"), toolsRenameDirectory);
  connect(toolsRenameDirectory, SIGNAL(triggered()), impl(), SLOT(slotRenameDirectory()));
  KAction* toolsNumberTracks = new KAction(tr("&Number Tracks..."), this);
  toolsNumberTracks->setStatusTip(tr("Number Tracks"));
  actionCollection()->addAction(QLatin1String("number_tracks"), toolsNumberTracks);
  connect(toolsNumberTracks, SIGNAL(triggered()), impl(), SLOT(slotNumberTracks()));
  KAction* toolsFilter = new KAction(tr("F&ilter..."), this);
  toolsFilter->setStatusTip(tr("Filter"));
  actionCollection()->addAction(QLatin1String("filter"), toolsFilter);
  connect(toolsFilter, SIGNAL(triggered()), impl(), SLOT(slotFilter()));
  const TagConfig& tagCfg = TagConfig::instance();
  if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v24) {
    KAction* toolsConvertToId3v24 = new KAction(tr("Convert ID3v2.3 to ID3v2.&4"), this);
    toolsConvertToId3v24->setStatusTip(tr("Convert ID3v2.3 to ID3v2.4"));
    actionCollection()->addAction(QLatin1String("convert_to_id3v24"), toolsConvertToId3v24);
    connect(toolsConvertToId3v24, SIGNAL(triggered()), app(), SLOT(convertToId3v24()));
    if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v23) {
      KAction* toolsConvertToId3v23 = new KAction(tr("Convert ID3v2.4 to ID3v2.&3"), this);
      toolsConvertToId3v23->setStatusTip(tr("Convert ID3v2.4 to ID3v2.3"));
      actionCollection()->addAction(QLatin1String("convert_to_id3v23"), toolsConvertToId3v23);
      connect(toolsConvertToId3v23, SIGNAL(triggered()), app(), SLOT(convertToId3v23()));
    }
  }
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  KAction* toolsPlay = new KAction(KIcon(QLatin1String("media-playback-start")), tr("&Play"), this);
  toolsPlay->setStatusTip(tr("Play"));
  actionCollection()->addAction(QLatin1String("play"), toolsPlay);
  connect(toolsPlay, SIGNAL(triggered()), app(), SLOT(playAudio()));
#endif
  m_settingsShowHidePicture = new KToggleAction(tr("Show &Picture"), this);
  m_settingsShowHidePicture->setStatusTip(tr("Show Picture"));
  m_settingsShowHidePicture->setCheckable(true);
  actionCollection()->addAction(QLatin1String("hide_picture"), m_settingsShowHidePicture);
  connect(m_settingsShowHidePicture, SIGNAL(triggered()), impl(), SLOT(slotSettingsShowHidePicture()));
  m_settingsAutoHideTags = new KToggleAction(tr("Auto &Hide Tags"), this);
  m_settingsAutoHideTags->setStatusTip(tr("Auto Hide Tags"));
  m_settingsAutoHideTags->setCheckable(true);
  actionCollection()->addAction(QLatin1String("auto_hide_tags"), m_settingsAutoHideTags);
  connect(m_settingsAutoHideTags, SIGNAL(triggered()), impl(), SLOT(slotSettingsAutoHideTags()));
  KAction* editSelectAllInDir = new KAction(tr("Select All in &Directory"), this);
  editSelectAllInDir->setStatusTip(tr("Select all files in the current directory"));
  actionCollection()->addAction(QLatin1String("select_all_in_directory"), editSelectAllInDir);
  connect(editSelectAllInDir, SIGNAL(triggered()), app(), SLOT(selectAllInDirectory()));
  KAction* editPreviousFile = new KAction(KIcon(QLatin1String("go-previous")), tr("&Previous File"), this);
  editPreviousFile->setStatusTip(tr("Select previous file"));
#if QT_VERSION >= 0x050000
  actionCollection()->setDefaultShortcuts(editPreviousFile,
                         KStandardShortcut::shortcut(KStandardShortcut::Prior));
#else
  editPreviousFile->setShortcut(KShortcut(QLatin1String("Alt+Up")));
#endif
  actionCollection()->addAction(QLatin1String("previous_file"), editPreviousFile);
  connect(editPreviousFile, SIGNAL(triggered()), form(), SLOT(previousFile()));
  KAction* editNextFile = new KAction(KIcon(QLatin1String("go-next")), tr("&Next File"), this);
  editNextFile->setStatusTip(tr("Select next file"));
#if QT_VERSION >= 0x050000
  actionCollection()->setDefaultShortcuts(editNextFile,
                         KStandardShortcut::shortcut(KStandardShortcut::Next));
#else
  editNextFile->setShortcut(KShortcut(QLatin1String("Alt+Down")));
#endif
  actionCollection()->addAction(QLatin1String("next_file"), editNextFile);
  connect(editNextFile, SIGNAL(triggered()), form(), SLOT(nextFile()));
  KAction* actionV1FromFilename = new KAction(tr("Tag 1") + QLatin1String(": ") + tr("From Filename"), this);
  actionCollection()->addAction(QLatin1String("v1_from_filename"), actionV1FromFilename);
  connect(actionV1FromFilename, SIGNAL(triggered()), app(), SLOT(getTagsFromFilenameV1()));
  KAction* actionV1FromV2 = new KAction(tr("Tag 1") + QLatin1String(": ") + tr("From Tag 2"), this);
  actionCollection()->addAction(QLatin1String("v1_from_v2"), actionV1FromV2);
  connect(actionV1FromV2, SIGNAL(triggered()), app(), SLOT(copyV2ToV1()));
  KAction* actionV1Copy = new KAction(tr("Tag 1") + QLatin1String(": ") + tr("Copy"), this);
  actionCollection()->addAction(QLatin1String("v1_copy"), actionV1Copy);
  connect(actionV1Copy, SIGNAL(triggered()), app(), SLOT(copyTagsV1()));
  KAction* actionV1Paste = new KAction(tr("Tag 1") + QLatin1String(": ") + tr("Paste"), this);
  actionCollection()->addAction(QLatin1String("v1_paste"), actionV1Paste);
  connect(actionV1Paste, SIGNAL(triggered()), app(), SLOT(pasteTagsV1()));
  KAction* actionV1Remove = new KAction(tr("Tag 1") + QLatin1String(": ") + tr("Remove"), this);
  actionCollection()->addAction(QLatin1String("v1_remove"), actionV1Remove);
  connect(actionV1Remove, SIGNAL(triggered()), app(), SLOT(removeTagsV1()));
  KAction* actionV2FromFilename = new KAction(tr("Tag 2") + QLatin1String(": ") + tr("From Filename"), this);
  actionCollection()->addAction(QLatin1String("v2_from_filename"), actionV2FromFilename);
  connect(actionV2FromFilename, SIGNAL(triggered()), app(), SLOT(getTagsFromFilenameV2()));
  KAction* actionV2FromV1 = new KAction(tr("Tag 2") + QLatin1String(": ") + tr("From Tag 1"), this);
  actionCollection()->addAction(QLatin1String("v2_from_v1"), actionV2FromV1);
  connect(actionV2FromV1, SIGNAL(triggered()), app(), SLOT(copyV1ToV2()));
  KAction* actionV2Copy = new KAction(tr("Tag 2") + QLatin1String(": ") + tr("Copy"), this);
  actionCollection()->addAction(QLatin1String("v2_copy"), actionV2Copy);
  connect(actionV2Copy, SIGNAL(triggered()), app(), SLOT(copyTagsV2()));
  KAction* actionV2Paste = new KAction(tr("Tag 2") + QLatin1String(": ") + tr("Paste"), this);
  actionCollection()->addAction(QLatin1String("v2_paste"), actionV2Paste);
  connect(actionV2Paste, SIGNAL(triggered()), app(), SLOT(pasteTagsV2()));
  KAction* actionV2Remove = new KAction(tr("Tag 2") + QLatin1String(": ") + tr("Remove"), this);
  actionCollection()->addAction(QLatin1String("v2_remove"), actionV2Remove);
  connect(actionV2Remove, SIGNAL(triggered()), app(), SLOT(removeTagsV2()));
  KAction* actionFramesEdit = new KAction(tr("Frames:") + QLatin1Char(' ') + tr("Edit"), this);
  actionCollection()->addAction(QLatin1String("frames_edit"), actionFramesEdit);
  connect(actionFramesEdit, SIGNAL(triggered()), form(), SLOT(editFrame()));
  KAction* actionFramesAdd = new KAction(tr("Frames:") + QLatin1Char(' ') + tr("Add"), this);
  actionCollection()->addAction(QLatin1String("frames_add"), actionFramesAdd);
  connect(actionFramesAdd, SIGNAL(triggered()), form(), SLOT(addFrame()));
  KAction* actionFramesDelete = new KAction(tr("Frames:") + QLatin1Char(' ') + tr("Delete"), this);
  actionCollection()->addAction(QLatin1String("frames_delete"), actionFramesDelete);
  connect(actionFramesDelete, SIGNAL(triggered()), form(), SLOT(deleteFrame()));
  KAction* actionFilenameFromV1 = new KAction(tr("Filename") + QLatin1String(": ") + tr("From Tag 1"), this);
  actionCollection()->addAction(QLatin1String("filename_from_v1"), actionFilenameFromV1);
  connect(actionFilenameFromV1, SIGNAL(triggered()), form(), SLOT(fnFromID3V1()));
  KAction* actionFilenameFromV2 = new KAction(tr("Filename") + QLatin1String(": ") + tr("From Tag 2"), this);
  actionCollection()->addAction(QLatin1String("filename_from_v2"), actionFilenameFromV2);
  connect(actionFilenameFromV2, SIGNAL(triggered()), form(), SLOT(fnFromID3V2()));
  KAction* actionFilenameFocus = new KAction(tr("Filename") + QLatin1String(": ") + tr("Focus"), this);
  actionCollection()->addAction(QLatin1String("filename_focus"), actionFilenameFocus);
  connect(actionFilenameFocus, SIGNAL(triggered()), form(), SLOT(setFocusFilename()));
  KAction* actionV1Focus = new KAction(tr("Tag 1") + QLatin1String(": ") + tr("Focus"), this);
  actionCollection()->addAction(QLatin1String("v1_focus"), actionV1Focus);
  connect(actionV1Focus, SIGNAL(triggered()), form(), SLOT(setFocusV1()));
  KAction* actionV2Focus = new KAction(tr("Tag 2") + QLatin1String(": ") + tr("Focus"), this);
  actionCollection()->addAction(QLatin1String("v2_focus"), actionV2Focus);
  connect(actionV2Focus, SIGNAL(triggered()), form(), SLOT(setFocusV2()));
  KAction* actionFileListFocus = new KAction(tr("File List") + QLatin1String(": ") + tr("Focus"), this);
  actionCollection()->addAction(QLatin1String("filelist_focus"), actionFileListFocus);
  connect(actionFileListFocus, SIGNAL(triggered()), form(), SLOT(setFocusFileList()));
  KAction* actionFileListRename = new KAction(tr("&Rename"), this);
  actionFileListRename->setShortcut(QKeySequence(Qt::Key_F2));
  actionFileListRename->setShortcutContext(Qt::WidgetShortcut);
  connect(actionFileListRename, SIGNAL(triggered()), impl(), SLOT(renameFile()));
  actionCollection()->addAction(QLatin1String("filelist_rename"), actionFileListRename);
  form()->getFileList()->setRenameAction(actionFileListRename);
  KAction* actionFileListDelete = new KAction(tr("&Move to Trash"), this);
  actionFileListDelete->setShortcut(QKeySequence::Delete);
  actionFileListDelete->setShortcutContext(Qt::WidgetShortcut);
  connect(actionFileListDelete, SIGNAL(triggered()), impl(), SLOT(deleteFile()));
  actionCollection()->addAction(QLatin1String("filelist_delete"), actionFileListDelete);
  form()->getFileList()->setDeleteAction(actionFileListDelete);
  KAction* actionDirListFocus = new KAction(tr("Directory List") + QLatin1String(": ") + tr("Focus"), this);
  actionCollection()->addAction(QLatin1String("dirlist_focus"), actionDirListFocus);
  connect(actionDirListFocus, SIGNAL(triggered()), form(), SLOT(setFocusDirList()));
  createGUI();
}

/**
 * Add directory to recent files list.
 *
 * @param dirName path to directory
 */
void KdeMainWindow::addDirectoryToRecentFiles(const QString& dirName)
{
  KUrl url;
  url.setPath(dirName);
  m_fileOpenRecent->addUrl(url);
}

/**
 * Read settings from the configuration.
 */
void KdeMainWindow::readConfig()
{
  setAutoSaveSettings();
  m_settingsShowHidePicture->setChecked(!GuiConfig::instance().hidePicture());
  m_settingsAutoHideTags->setChecked(GuiConfig::instance().autoHideTags());
  m_fileOpenRecent->loadEntries(KSharedConfig_openConfig()->group("Recent Files"));
}

/**
 * Store geometry and recent files in settings.
 */
void KdeMainWindow::saveConfig()
{
  m_fileOpenRecent->saveEntries(KSharedConfig_openConfig()->group("Recent Files"));
}

/**
 * Set main window caption.
 *
 * @param caption caption without application name
 * @param modified true if any file is modified
 */
void KdeMainWindow::setWindowCaption(const QString& caption, bool modified)
{
  setCaption(caption, modified);
}

/**
 * Get action for Settings/Auto Hide Tags.
 * @return action.
 */
QAction* KdeMainWindow::autoHideTagsAction()
{
  return m_settingsAutoHideTags;
}

/**
 * Get action for Settings/Hide Picture.
 * @return action.
 */
QAction* KdeMainWindow::showHidePictureAction()
{
 return m_settingsShowHidePicture;
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
bool KdeMainWindow::queryClose()
{
  return queryBeforeClosing();
}

/**
 * Saves the window properties to the session config file.
 *
 * @param cfg application configuration
 */
void KdeMainWindow::saveProperties(KConfigGroup& cfg)
{
  cfg.writeEntry("dirname", app()->getDirName());
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param cfg application configuration
 */
void KdeMainWindow::readProperties(const KConfigGroup& cfg)
{
  app()->openDirectory(QStringList() << cfg.readEntry("dirname", ""));
}

/**
 * Open recent directory.
 *
 * @param dir directory to open
 */
void KdeMainWindow::slotFileOpenRecentUrl(const KUrl& url)
{
  openRecentDirectory(url.path());
}

/**
 * Shortcuts configuration.
 */
void KdeMainWindow::slotSettingsShortcuts()
{
  KShortcutsDialog::configure(
    actionCollection(),
    KShortcutsEditor::LetterShortcutsDisallowed, this);
}

/**
 * Toolbars configuration.
 */
void KdeMainWindow::slotSettingsToolbars()
{
  KEditToolBar dlg(actionCollection());
  if (dlg.exec()) {
    createGUI();
  }
}

/**
 * Preferences.
 */
void KdeMainWindow::slotSettingsConfigure()
{
  QString caption(tr("Configure - Kid3"));
  KConfigSkeleton* configSkeleton = new KConfigSkeleton;
  KdeConfigDialog* dialog = new KdeConfigDialog(this, caption, configSkeleton);
  dialog->setConfig();
  if (dialog->exec() == QDialog::Accepted) {
    dialog->getConfig();
    impl()->applyChangedConfiguration();
  }
  delete configSkeleton;
}
