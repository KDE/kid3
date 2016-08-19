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
  m_platformTools(platformTools), m_fileOpenRecent(0),
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
  KActionCollection* collection = actionCollection();
  KAction* action = KStandardAction::open(
      impl(), SLOT(slotFileOpen()), collection);
  action->setStatusTip(tr("Open files"));
  m_fileOpenRecent = KStandardAction::openRecent(
      this,
#if QT_VERSION >= 0x050000
        SLOT(slotFileOpenRecentUrl(QUrl)),
#else
        SLOT(slotFileOpenRecentUrl(KUrl)),
#endif
      collection);
  m_fileOpenRecent->setStatusTip(tr("Opens a recently used directory"));
  action = KStandardAction::revert(
      app(), SLOT(revertFileModifications()), collection);
  action->setStatusTip(
      tr("Reverts the changes of all or the selected files"));
#if QT_VERSION >= 0x050000
  collection->setDefaultShortcuts(action,
                          KStandardShortcut::shortcut(KStandardShortcut::Undo));
#else
  action->setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Undo));
#endif
  action = KStandardAction::save(
      impl(), SLOT(slotFileSave()), collection);
  action->setStatusTip(tr("Saves the changed files"));
  action = KStandardAction::quit(
      impl(), SLOT(slotFileQuit()), collection);
  action->setStatusTip(tr("Quits the application"));
  action = KStandardAction::selectAll(
      form(), SLOT(selectAllFiles()), collection);
  action->setStatusTip(tr("Select all files"));
  action->setShortcut(KShortcut(QLatin1String("Alt+Shift+A")));
  action = KStandardAction::deselect(
      form(), SLOT(deselectAllFiles()), collection);
  action->setStatusTip(tr("Deselect all files"));
  action = KStandardAction::find(
      impl(), SLOT(find()), collection);
  action->setStatusTip(tr("Find"));
  action = KStandardAction::replace(
      impl(), SLOT(findReplace()), collection);
  action->setStatusTip(tr("Find and replace"));
  setStandardToolBarMenuEnabled(true);
  createStandardStatusBarAction();
  action = KStandardAction::keyBindings(
    this, SLOT(slotSettingsShortcuts()), collection);
  action->setStatusTip(tr("Configure Shortcuts"));
  action = KStandardAction::configureToolbars(
    this, SLOT(slotSettingsToolbars()), collection);
  action->setStatusTip(tr("Configure Toolbars"));
  action = KStandardAction::preferences(
      this, SLOT(slotSettingsConfigure()), collection);
  action->setStatusTip(tr("Preferences dialog"));

  action = new KAction(KIcon(QLatin1String("document-open")),
                       tr("O&pen Directory..."), this);
  action->setStatusTip(tr("Opens a directory"));
  action->setShortcut(KShortcut(QLatin1String("Ctrl+D")));
  collection->addAction(QLatin1String("open_directory"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotFileOpenDirectory()));
  action = new KAction(KIcon(QLatin1String("document-import")),
                       tr("&Import..."), this);
  action->setStatusTip(tr("Import from file or clipboard"));
  action->setData(-1);
  collection->addAction(QLatin1String("import"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotImport()));

  int importerIdx = 0;
  foreach (const ServerImporter* si, app()->getServerImporters()) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower().
        remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    action = new KAction(tr("Import from %1...").arg(serverName), this);
    action->setData(importerIdx);
    action->setStatusTip(tr("Import from %1").arg(serverName));
    collection->addAction(actionName, action);
    connect(action, SIGNAL(triggered()), impl(), SLOT(slotImport()));
    ++importerIdx;
  }

  foreach (const ServerTrackImporter* si, app()->getServerTrackImporters()) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower().
        remove(QLatin1Char(' '));
    int dotPos = actionName.indexOf(QLatin1Char('.'));
    if (dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    action = new KAction(tr("Import from %1...").arg(serverName), this);
    action->setStatusTip(tr("Import from %1").arg(serverName));
    action->setData(importerIdx);
    collection->addAction(actionName, action);
    connect(action, SIGNAL(triggered()), impl(), SLOT(slotImport()));
    ++importerIdx;
  }

  action = new KAction(tr("Automatic I&mport..."), this);
  action->setStatusTip(tr("Automatic import"));
  collection->addAction(QLatin1String("batch_import"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotBatchImport()));

  action = new KAction(tr("&Browse Cover Art..."), this);
  action->setStatusTip(tr("Browse album cover artwork"));
  collection->addAction(QLatin1String("browse_cover_art"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotBrowseCoverArt()));
  action = new KAction(KIcon(QLatin1String("document-export")), tr("&Export..."), this);
  action->setStatusTip(tr("Export to file or clipboard"));
  collection->addAction(QLatin1String("export"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotExport()));
  action = new KAction(KIcon(QLatin1String("view-media-playlist")), tr("&Create Playlist..."), this);
  action->setStatusTip(tr("Create M3U Playlist"));
  collection->addAction(QLatin1String("create_playlist"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotPlaylistDialog()));
  action = new KAction(tr("Apply &Filename Format"), this);
  action->setStatusTip(tr("Apply Filename Format"));
  collection->addAction(QLatin1String("apply_filename_format"), action);
  connect(action, SIGNAL(triggered()), app(), SLOT(applyFilenameFormat()));
  action = new KAction(tr("Apply &Tag Format"), this);
  action->setStatusTip(tr("Apply Tag Format"));
  collection->addAction(QLatin1String("apply_id3_format"), action);
  connect(action, SIGNAL(triggered()), app(), SLOT(applyTagFormat()));
  action = new KAction(tr("Apply Text &Encoding"), this);
  action->setStatusTip(tr("Apply Text Encoding"));
  collection->addAction(QLatin1String("apply_text_encoding"), action);
  connect(action, SIGNAL(triggered()), app(), SLOT(applyTextEncoding()));
  action = new KAction(tr("&Rename Directory..."), this);
  action->setStatusTip(tr("Rename Directory"));
  collection->addAction(QLatin1String("rename_directory"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotRenameDirectory()));
  action = new KAction(tr("&Number Tracks..."), this);
  action->setStatusTip(tr("Number Tracks"));
  collection->addAction(QLatin1String("number_tracks"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotNumberTracks()));
  action = new KAction(tr("F&ilter..."), this);
  action->setStatusTip(tr("Filter"));
  collection->addAction(QLatin1String("filter"), action);
  connect(action, SIGNAL(triggered()), impl(), SLOT(slotFilter()));
  const TagConfig& tagCfg = TagConfig::instance();
  if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v24) {
    action = new KAction(tr("Convert ID3v2.3 to ID3v2.&4"), this);
    action->setStatusTip(tr("Convert ID3v2.3 to ID3v2.4"));
    collection->addAction(QLatin1String("convert_to_id3v24"), action);
    connect(action, SIGNAL(triggered()), app(), SLOT(convertToId3v24()));
    if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v23) {
      action = new KAction(tr("Convert ID3v2.4 to ID3v2.&3"), this);
      action->setStatusTip(tr("Convert ID3v2.4 to ID3v2.3"));
      collection->addAction(QLatin1String("convert_to_id3v23"), action);
      connect(action, SIGNAL(triggered()), app(), SLOT(convertToId3v23()));
    }
  }
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  action = new KAction(KIcon(QLatin1String("media-playback-start")), tr("&Play"), this);
  action->setStatusTip(tr("Play"));
  collection->addAction(QLatin1String("play"), action);
  connect(action, SIGNAL(triggered()), app(), SLOT(playAudio()));
#endif
  m_settingsShowHidePicture = new KToggleAction(tr("Show &Picture"), this);
  m_settingsShowHidePicture->setStatusTip(tr("Show Picture"));
  m_settingsShowHidePicture->setCheckable(true);
  collection->addAction(QLatin1String("hide_picture"), m_settingsShowHidePicture);
  connect(m_settingsShowHidePicture, SIGNAL(triggered()), impl(), SLOT(slotSettingsShowHidePicture()));
  m_settingsAutoHideTags = new KToggleAction(tr("Auto &Hide Tags"), this);
  m_settingsAutoHideTags->setStatusTip(tr("Auto Hide Tags"));
  m_settingsAutoHideTags->setCheckable(true);
  collection->addAction(QLatin1String("auto_hide_tags"), m_settingsAutoHideTags);
  connect(m_settingsAutoHideTags, SIGNAL(triggered()), impl(), SLOT(slotSettingsAutoHideTags()));
  action = new KAction(tr("Select All in &Directory"), this);
  action->setStatusTip(tr("Select all files in the current directory"));
  collection->addAction(QLatin1String("select_all_in_directory"), action);
  connect(action, SIGNAL(triggered()), app(), SLOT(selectAllInDirectory()));
  action = new KAction(KIcon(QLatin1String("go-previous")), tr("&Previous File"), this);
  action->setStatusTip(tr("Select previous file"));
#if QT_VERSION >= 0x050000
  collection->setDefaultShortcuts(action,
                         KStandardShortcut::shortcut(KStandardShortcut::Prior));
#else
  action->setShortcut(KShortcut(QLatin1String("Alt+Up")));
#endif
  collection->addAction(QLatin1String("previous_file"), action);
  connect(action, SIGNAL(triggered()), form(), SLOT(previousFile()));
  action = new KAction(KIcon(QLatin1String("go-next")), tr("&Next File"), this);
  action->setStatusTip(tr("Select next file"));
#if QT_VERSION >= 0x050000
  collection->setDefaultShortcuts(action,
                         KStandardShortcut::shortcut(KStandardShortcut::Next));
#else
  action->setShortcut(KShortcut(QLatin1String("Alt+Down")));
#endif
  collection->addAction(QLatin1String("next_file"), action);
  connect(action, SIGNAL(triggered()), form(), SLOT(nextFile()));
  FOR_ALL_TAGS(tagNr) {
    Frame::TagNumber otherTagNr = tagNr == Frame::Tag_1 ? Frame::Tag_2 :
          tagNr == Frame::Tag_2 ? Frame::Tag_1 : Frame::Tag_NumValues;
    QString tagStr = Frame::tagNumberToString(tagNr);
    Kid3ApplicationTagContext* appTag = app()->tag(tagNr);
    Kid3FormTagContext* formTag = form()->tag(tagNr);
    QString actionPrefix = tr("Tag %1").arg(tagStr) +
        QLatin1String(": ");
    action = new KAction(tr("Filename") + QLatin1String(": ") +
                         tr("From Tag %1").arg(tagStr), this);
    collection->addAction(QLatin1String("filename_from_v") + tagStr, action);
    connect(action, SIGNAL(triggered()), appTag, SLOT(getFilenameFromTags()));
    tagStr = QLatin1Char('v') + tagStr + QLatin1Char('_');
    action = new KAction(actionPrefix + tr("From Filename"), this);
    collection->addAction(tagStr + QLatin1String("from_filename"), action);
    connect(action, SIGNAL(triggered()), appTag, SLOT(getTagsFromFilename()));
    if (otherTagNr < Frame::Tag_NumValues) {
      QString otherTagStr = Frame::tagNumberToString(otherTagNr);
      action = new KAction(actionPrefix + tr("From Tag %1").arg(otherTagStr),
                           this);
      collection->addAction(tagStr + QLatin1String("from_v") + otherTagStr,
                            action);
      connect(action, SIGNAL(triggered()), appTag, SLOT(copyToOtherTag()));
    }
    action = new KAction(actionPrefix + tr("Copy"), this);
    collection->addAction(tagStr + QLatin1String("copy"), action);
    connect(action, SIGNAL(triggered()), appTag, SLOT(copyTags()));
    action = new KAction(actionPrefix + tr("Paste"), this);
    collection->addAction(tagStr + QLatin1String("paste"), action);
    connect(action, SIGNAL(triggered()), appTag, SLOT(pasteTags()));
    action = new KAction(actionPrefix + tr("Remove"), this);
    collection->addAction(tagStr + QLatin1String("remove"), action);
    connect(action, SIGNAL(triggered()), appTag, SLOT(removeTags()));
    action = new KAction(actionPrefix + tr("Focus"), this);
    collection->addAction(tagStr + QLatin1String("focus"), action);
    connect(action, SIGNAL(triggered()), formTag, SLOT(setFocusTag()));
    if (tagNr != Frame::Tag_Id3v1) {
      actionPrefix += tr("Frames:") + QLatin1Char(' ');
      action = new KAction(actionPrefix + tr("Edit"), this);
      collection->addAction(tagStr + QLatin1String("frames_edit"), action);
      connect(action, SIGNAL(triggered()), appTag, SLOT(editFrame()));
      action = new KAction(actionPrefix + tr("Add"), this);
      collection->addAction(tagStr + QLatin1String("frames_add"), action);
      connect(action, SIGNAL(triggered()), appTag, SLOT(addFrame()));
      action = new KAction(actionPrefix + tr("Delete"), this);
      collection->addAction(tagStr + QLatin1String("frames_delete"), action);
      connect(action, SIGNAL(triggered()), appTag, SLOT(deleteFrame()));
    }
  }

  action = new KAction(tr("Filename") + QLatin1String(": ") + tr("Focus"),
                       this);
  collection->addAction(QLatin1String("filename_focus"), action);
  connect(action, SIGNAL(triggered()), form(), SLOT(setFocusFilename()));

  action = new KAction(tr("File List") + QLatin1String(": ") + tr("Focus"),
                       this);
  collection->addAction(QLatin1String("filelist_focus"), action);
  connect(action, SIGNAL(triggered()), form(), SLOT(setFocusFileList()));
  action = new KAction(tr("&Rename"), this);
  action->setShortcut(QKeySequence(Qt::Key_F2));
  action->setShortcutContext(Qt::WidgetShortcut);
  connect(action, SIGNAL(triggered()), impl(), SLOT(renameFile()));
  collection->addAction(QLatin1String("filelist_rename"), action);
  form()->getFileList()->setRenameAction(action);
  action = new KAction(tr("&Move to Trash"), this);
  action->setShortcut(QKeySequence::Delete);
  action->setShortcutContext(Qt::WidgetShortcut);
  connect(action, SIGNAL(triggered()), impl(), SLOT(deleteFile()));
  collection->addAction(QLatin1String("filelist_delete"), action);
  form()->getFileList()->setDeleteAction(action);
  action = new KAction(tr("Directory List") + QLatin1String(": ") + tr("Focus"),
                       this);
  collection->addAction(QLatin1String("dirlist_focus"), action);
  connect(action, SIGNAL(triggered()), form(), SLOT(setFocusDirList()));
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
  KdeConfigDialog* dialog = new KdeConfigDialog(m_platformTools, this, caption,
                                                configSkeleton);
  dialog->setConfig();
  if (dialog->exec() == QDialog::Accepted) {
    dialog->getConfig();
    impl()->applyChangedConfiguration();
  }
  delete configSkeleton;
}
