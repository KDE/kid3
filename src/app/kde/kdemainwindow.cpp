/**
 * \file kdemainwindow.cpp
 * KDE Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
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

#include "kdemainwindow.h"
#include <kconfigwidgets_version.h>
#include <kconfig_version.h>
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
#include "config.h"
#include "kid3form.h"
#include "filelist.h"
#include "sectionactions.h"
#include "kid3application.h"
#include "kdeconfigdialog.h"
#include "guiconfig.h"
#include "tagconfig.h"
#include "useractionsconfig.h"
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
                             Kid3Application* app, QWidget* parent)
  : KXmlGuiWindow(parent),
    BaseMainWindow(this, platformTools, app),
    m_platformTools(platformTools), m_fileOpenRecent(nullptr),
    m_settingsShowStatusbar(nullptr),
    m_settingsAutoHideTags(nullptr), m_settingsShowHidePicture(nullptr)
{
  init();
}

/** Only defined for generation of translation files */
#define MAIN_TOOLBAR_FOR_PO QT_TRANSLATE_NOOP("@default", "Main Toolbar")

/**
 * Init menu and toolbar actions.
 */
void KdeMainWindow::initActions()
{
  KActionCollection* collection = actionCollection();
#if KCONFIGWIDGETS_VERSION >= 0x051700
  QAction* action = KStandardAction::open(
      impl(), &BaseMainWindowImpl::slotFileOpen, collection);
#else
  QAction* action = KStandardAction::open(
      impl(), SLOT(slotFileOpen()), collection);
#endif
  action->setStatusTip(tr("Open files"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  m_fileOpenRecent = KStandardAction::openRecent(
      this, &KdeMainWindow::slotFileOpenRecentUrl, collection);
#else
  m_fileOpenRecent = KStandardAction::openRecent(
      this, SLOT(slotFileOpenRecentUrl(QUrl)), collection);
#endif
  m_fileOpenRecent->setStatusTip(tr("Opens a recently used folder"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::revert(
      app(), &Kid3Application::revertFileModifications, collection);
#else
  action = KStandardAction::revert(
      app(), SLOT(revertFileModifications()), collection);
#endif
  action->setStatusTip(
      tr("Reverts the changes of all or the selected files"));
  collection->setDefaultShortcuts(action,
                          KStandardShortcut::shortcut(KStandardShortcut::Undo));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::save(
      impl(), &BaseMainWindowImpl::slotFileSave, collection);
#else
  action = KStandardAction::save(
      impl(), SLOT(slotFileSave()), collection);
#endif
  action->setStatusTip(tr("Saves the changed files"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::quit(
      impl(), &BaseMainWindowImpl::slotFileQuit, collection);
#else
  action = KStandardAction::quit(
      impl(), SLOT(slotFileQuit()), collection);
#endif
  action->setStatusTip(tr("Quits the application"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::selectAll(
      form(), &Kid3Form::selectAllFiles, collection);
#else
  action = KStandardAction::selectAll(
      form(), SLOT(selectAllFiles()), collection);
#endif
  action->setStatusTip(tr("Select all files"));
  action->setShortcut(QKeySequence(QLatin1String("Alt+Shift+A")));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::deselect(
      form(), &Kid3Form::deselectAllFiles, collection);
#else
  action = KStandardAction::deselect(
      form(), SLOT(deselectAllFiles()), collection);
#endif
  action->setStatusTip(tr("Deselect all files"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::find(
      impl(), &BaseMainWindowImpl::find, collection);
#else
  action = KStandardAction::find(
      impl(), SLOT(find()), collection);
#endif
  action->setStatusTip(tr("Find"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::replace(
      impl(), &BaseMainWindowImpl::findReplace, collection);
#else
  action = KStandardAction::replace(
      impl(), SLOT(findReplace()), collection);
#endif
  action->setStatusTip(tr("Find and replace"));
  setStandardToolBarMenuEnabled(true);
  createStandardStatusBarAction();
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::keyBindings(
    this, &KdeMainWindow::slotSettingsShortcuts, collection);
#else
  action = KStandardAction::keyBindings(
    this, SLOT(slotSettingsShortcuts()), collection);
#endif
  action->setStatusTip(tr("Configure Shortcuts"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::configureToolbars(
    this, &KdeMainWindow::slotSettingsToolbars, collection);
#else
  action = KStandardAction::configureToolbars(
    this, SLOT(slotSettingsToolbars()), collection);
#endif
  action->setStatusTip(tr("Configure Toolbars"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  m_settingsShowStatusbar = KStandardAction::showStatusbar(
    this, &KdeMainWindow::slotSettingsShowStatusbar, collection);
#else
  m_settingsShowStatusbar = KStandardAction::showStatusbar(
    this, SLOT(slotSettingsShowStatusbar()), collection);
#endif
  m_settingsShowStatusbar->setStatusTip(tr("Enables/disables the statusbar"));
#if KCONFIGWIDGETS_VERSION >= 0x051700
  action = KStandardAction::preferences(
      this, &KdeMainWindow::slotSettingsConfigure, collection);
#else
  action = KStandardAction::preferences(
      this, SLOT(slotSettingsConfigure()), collection);
#endif
  action->setStatusTip(tr("Preferences dialog"));

  action = new QAction(QIcon::fromTheme(QLatin1String("document-open")),
                       tr("O&pen Folder..."), this);
  action->setStatusTip(tr("Opens a folder"));
  action->setShortcut(QKeySequence(QLatin1String("Ctrl+D")));
  collection->addAction(QLatin1String("open_directory"), action);
  connect(action, &QAction::triggered,
          impl(), &BaseMainWindowImpl::slotFileOpenDirectory);

  action = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                       tr("Re&load"), this);
  action->setStatusTip(tr("Reload folder"));
  // When using the KDE version on GNOME, a dialog appears "There are two
  // actions (Replace..., Reload) that want to use the same shortcut (Ctrl+R)".
  // Avoid this by assigning Qt::Key_F5 instead of QKeySequence::Refresh.
  // The section "Standard Shortcuts" in the QKeySequence documentation lists
  // F5 as a key for "Refresh" on all platforms.
  action->setShortcut(Qt::Key_F5);
  collection->addAction(QLatin1String("reload"), action);
  connect(action, &QAction::triggered,
          impl(), &BaseMainWindowImpl::slotFileReload);

  action = new QAction(tr("Unload"), this);
  collection->addAction(QLatin1String("unload"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::unloadAllTags);

  action = new QAction(QIcon::fromTheme(QLatin1String("document-import")),
                       tr("&Import..."), this);
  action->setStatusTip(tr("Import from file or clipboard"));
  action->setData(-1);
  collection->addAction(QLatin1String("import"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotImport);

  int importerIdx = 0;
  const auto sis = app()->getServerImporters();
  for (const ServerImporter* si : sis) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower()
        .remove(QLatin1Char(' '));
    if (int dotPos = actionName.indexOf(QLatin1Char('.')); dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    action = new QAction(tr("Import from %1...").arg(serverName), this);
    action->setData(importerIdx);
    action->setStatusTip(tr("Import from %1").arg(serverName));
    collection->addAction(actionName, action);
    connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotImport);
    ++importerIdx;
  }

  const auto stis = app()->getServerTrackImporters();
  for (const ServerTrackImporter* si : stis) {
    QString serverName(QCoreApplication::translate("@default", si->name()));
    QString actionName = QString::fromLatin1(si->name()).toLower()
        .remove(QLatin1Char(' '));
    if (int dotPos = actionName.indexOf(QLatin1Char('.')); dotPos != -1)
      actionName.truncate(dotPos);
    actionName = QLatin1String("import_") + actionName;
    action = new QAction(tr("Import from %1...").arg(serverName), this);
    action->setStatusTip(tr("Import from %1").arg(serverName));
    action->setData(importerIdx);
    collection->addAction(actionName, action);
    connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotImport);
    ++importerIdx;
  }

  action = new QAction(tr("Import from Tags..."), this);
  action->setStatusTip(tr("Import from Tags"));
  collection->addAction(QLatin1String("import_tags"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotTagImport);

  action = new QAction(tr("Automatic I&mport..."), this);
  action->setStatusTip(tr("Automatic import"));
  collection->addAction(QLatin1String("batch_import"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotBatchImport);

  action = new QAction(tr("&Browse Cover Art..."), this);
  action->setStatusTip(tr("Browse album cover artwork"));
  collection->addAction(QLatin1String("browse_cover_art"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotBrowseCoverArt);
  action = new QAction(QIcon::fromTheme(QLatin1String("document-export")),
                       tr("&Export..."), this);
  action->setStatusTip(tr("Export to file or clipboard"));
  collection->addAction(QLatin1String("export"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotExport);
  action = new QAction(QIcon::fromTheme(QLatin1String("view-media-playlist")),
                       tr("&Create Playlist..."), this);
  action->setStatusTip(tr("Create M3U Playlist"));
  collection->addAction(QLatin1String("create_playlist"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotPlaylistDialog);
  action = new QAction(tr("Apply &Filename Format"), this);
  action->setStatusTip(tr("Apply Filename Format"));
  collection->addAction(QLatin1String("apply_filename_format"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::applyFilenameFormat);
  action = new QAction(tr("Apply &Tag Format"), this);
  action->setStatusTip(tr("Apply Tag Format"));
  collection->addAction(QLatin1String("apply_id3_format"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::applyTagFormat);
  action = new QAction(tr("Apply Text &Encoding"), this);
  action->setStatusTip(tr("Apply Text Encoding"));
  collection->addAction(QLatin1String("apply_text_encoding"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::applyTextEncoding);
  action = new QAction(tr("&Rename Folder..."), this);
  action->setStatusTip(tr("Rename Folder"));
  collection->addAction(QLatin1String("rename_directory"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotRenameDirectory);
  action = new QAction(tr("&Number Tracks..."), this);
  action->setStatusTip(tr("Number Tracks"));
  collection->addAction(QLatin1String("number_tracks"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotNumberTracks);
  action = new QAction(tr("F&ilter..."), this);
  action->setStatusTip(tr("Filter"));
  collection->addAction(QLatin1String("filter"), action);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::slotFilter);
  if (const TagConfig& tagCfg = TagConfig::instance();
      tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v24) {
    action = new QAction(tr("Convert ID3v2.3 to ID3v2.&4"), this);
    action->setStatusTip(tr("Convert ID3v2.3 to ID3v2.4"));
    collection->addAction(QLatin1String("convert_to_id3v24"), action);
    connect(action, &QAction::triggered, app(), &Kid3Application::convertToId3v24);
    if (tagCfg.taggedFileFeatures() & TaggedFile::TF_ID3v23) {
      action = new QAction(tr("Convert ID3v2.4 to ID3v2.&3"), this);
      action->setStatusTip(tr("Convert ID3v2.4 to ID3v2.3"));
      collection->addAction(QLatin1String("convert_to_id3v23"), action);
      connect(action, &QAction::triggered, app(), &Kid3Application::convertToId3v23);
    }
  }
#ifdef HAVE_QTMULTIMEDIA
  action = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")),
                       tr("&Play"), this);
  action->setStatusTip(tr("Play"));
  collection->addAction(QLatin1String("play"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::playAudio);
#endif
  m_settingsShowHidePicture = new KToggleAction(tr("Show &Picture"), this);
  m_settingsShowHidePicture->setStatusTip(tr("Show Picture"));
  m_settingsShowHidePicture->setCheckable(true);
  collection->addAction(QLatin1String("hide_picture"), m_settingsShowHidePicture);
  connect(m_settingsShowHidePicture, &QAction::triggered,
          impl(), &BaseMainWindowImpl::slotSettingsShowHidePicture);
  m_settingsAutoHideTags = new KToggleAction(tr("Auto &Hide Tags"), this);
  m_settingsAutoHideTags->setStatusTip(tr("Auto Hide Tags"));
  m_settingsAutoHideTags->setCheckable(true);
  collection->addAction(QLatin1String("auto_hide_tags"), m_settingsAutoHideTags);
  connect(m_settingsAutoHideTags, &QAction::triggered,
          impl(), &BaseMainWindowImpl::slotSettingsAutoHideTags);
  action = new QAction(tr("Select All in &Folder"), this);
  action->setStatusTip(tr("Select all files in the current folder"));
  collection->addAction(QLatin1String("select_all_in_directory"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::selectAllInDirectory);
  action = new QAction(tr("&Invert Selection"), this);
  collection->addAction(QLatin1String("invert_selection"), action);
  connect(action, &QAction::triggered, app(), &Kid3Application::invertSelection);
  action = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),
                       tr("&Previous File"), this);
  action->setStatusTip(tr("Select previous file"));
  collection->setDefaultShortcuts(action,
                         KStandardShortcut::shortcut(KStandardShortcut::Prior));
  collection->addAction(QLatin1String("previous_file"), action);
  connect(action, &QAction::triggered, form(), &Kid3Form::selectPreviousTaggedFile);
  action = new QAction(QIcon::fromTheme(QLatin1String("go-next")),
                       tr("&Next File"), this);
  action->setStatusTip(tr("Select next file"));
  collection->setDefaultShortcuts(action,
                         KStandardShortcut::shortcut(KStandardShortcut::Next));
  collection->addAction(QLatin1String("next_file"), action);
  connect(action, &QAction::triggered, form(), &Kid3Form::selectNextTaggedFile);
  FOR_ALL_TAGS(tagNr) {
    Frame::TagNumber otherTagNr = tagNr == Frame::Tag_1
        ? Frame::Tag_2
        : tagNr == Frame::Tag_2 ? Frame::Tag_1 : Frame::Tag_NumValues;
    QString tagStr = Frame::tagNumberToString(tagNr);
    Kid3ApplicationTagContext* appTag = app()->tag(tagNr);
    Kid3FormTagContext* formTag = form()->tag(tagNr);
    QString actionPrefix = tr("Tag %1").arg(tagStr) +
        QLatin1String(": ");
    action = new QAction(tr("Filename") + QLatin1String(": ") +
                         tr("From Tag %1").arg(tagStr), this);
    collection->addAction(QLatin1String("filename_from_v") + tagStr, action);
    connect(action, &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::getFilenameFromTags);
    tagStr = QLatin1Char('v') + tagStr + QLatin1Char('_');
    action = new QAction(actionPrefix + tr("From Filename"), this);
    collection->addAction(tagStr + QLatin1String("from_filename"), action);
    connect(action, &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::getTagsFromFilename);
    if (otherTagNr < Frame::Tag_NumValues) {
      QString otherTagStr = Frame::tagNumberToString(otherTagNr);
      action = new QAction(actionPrefix + tr("From Tag %1").arg(otherTagStr),
                           this);
      collection->addAction(tagStr + QLatin1String("from_v") + otherTagStr,
                            action);
      connect(action, &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::copyToOtherTag);
    }
    action = new QAction(actionPrefix + tr("Copy"), this);
    collection->addAction(tagStr + QLatin1String("copy"), action);
    connect(action, &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::copyTags);
    action = new QAction(actionPrefix + tr("Paste"), this);
    collection->addAction(tagStr + QLatin1String("paste"), action);
    connect(action, &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::pasteTags);
    action = new QAction(actionPrefix + tr("Remove"), this);
    collection->addAction(tagStr + QLatin1String("remove"), action);
    connect(action, &QAction::triggered,
            appTag, &Kid3ApplicationTagContext::removeTags);
    action = new QAction(actionPrefix + tr("Focus"), this);
    collection->addAction(tagStr + QLatin1String("focus"), action);
    connect(action, &QAction::triggered,
            formTag, &Kid3FormTagContext::setFocusTag);
    if (tagNr != Frame::Tag_Id3v1) {
      actionPrefix += tr("Frames:") + QLatin1Char(' ');
      action = new QAction(actionPrefix + tr("Edit"), this);
      collection->addAction(tagStr + QLatin1String("frames_edit"), action);
      connect(action, &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::editFrame);
      action = new QAction(actionPrefix + tr("Add"), this);
      collection->addAction(tagStr + QLatin1String("frames_add"), action);
      connect(action, &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::addFrame);
      action = new QAction(actionPrefix + tr("Delete"), this);
      collection->addAction(tagStr + QLatin1String("frames_delete"), action);
      connect(action, &QAction::triggered,
              appTag, &Kid3ApplicationTagContext::deleteFrame);
    }
  }

  action = new QAction(tr("Filename") + QLatin1String(": ") + tr("Focus"),
                       this);
  collection->addAction(QLatin1String("filename_focus"), action);
  connect(action, &QAction::triggered, form(), &Kid3Form::setFocusFilename);

  action = new QAction(tr("File List") + QLatin1String(": ") + tr("Focus"),
                       this);
  collection->addAction(QLatin1String("filelist_focus"), action);
  connect(action, &QAction::triggered, form(), &Kid3Form::setFocusFileList);
  action = new QAction(tr("&Rename"), this);
  action->setShortcut(QKeySequence(Qt::Key_F2));
  action->setShortcutContext(Qt::WidgetShortcut);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::renameFile);
  // This action is not made configurable because its shortcut F2 conflicts
  // with a section shortcut and there seems to be no way to avoid it with
  // KShortcutsDialog. The same applies to the shortcut with the Delete key.
  // collection->addAction(QLatin1String("filelist_rename"), action);
  form()->getFileList()->setRenameAction(action);
  action = new QAction(tr("&Move to Trash"), this);
  action->setShortcut(QKeySequence::Delete);
  action->setShortcutContext(Qt::WidgetShortcut);
  connect(action, &QAction::triggered, impl(), &BaseMainWindowImpl::deleteFile);
  // collection->addAction(QLatin1String("filelist_delete"), action);
  form()->getFileList()->setDeleteAction(action);
  action = new QAction(tr("Folder List") + QLatin1String(": ") + tr("Focus"),
                       this);
  collection->addAction(QLatin1String("dirlist_focus"), action);
  connect(action, &QAction::triggered, form(), &Kid3Form::setFocusDirList);

  FileList* fileList = form()->getFileList();
  // Do not support user action keyboard shortcuts with KDE 4, it would only
  // print "Attempt to use QAction (..) with KXMLGUIFactory!" warnings.
  connect(fileList, &FileList::userActionAdded,
          this, &KdeMainWindow::onUserActionAdded);
  connect(fileList, &FileList::userActionRemoved,
          this, &KdeMainWindow::onUserActionRemoved);
  fileList->initUserActions();
  const UserActionsConfig& userActionsCfg = UserActionsConfig::instance();
  connect(&userActionsCfg, &UserActionsConfig::contextMenuCommandsChanged,
          fileList, &FileList::initUserActions);

  const auto sectionShortcuts = SectionActions::defaultShortcuts();
  QString actionPrefix = tr("Section") + QLatin1String(": ");
  for (auto it = sectionShortcuts.constBegin();
       it != sectionShortcuts.constEnd();
       ++it) {
    const auto& tpl = *it;
    action = new QAction(actionPrefix + std::get<1>(tpl), this);
    action->setShortcutContext(Qt::WidgetShortcut);
    // The action is only used to configure the shortcuts. Disabling it will
    // also avoid "that want to use the same shortcut" error dialogs.
    action->setEnabled(false);
    collection->setDefaultShortcut(action, std::get<2>(tpl));
    collection->addAction(std::get<0>(tpl), action);
  }

  actionPrefix = tr("Player") + QLatin1String(": ");
  const auto actions = impl()->mediaActions();
  for (QAction* mediaAction : actions) {
    mediaAction->setText(actionPrefix + mediaAction->text());
    collection->addAction(mediaAction->objectName(), mediaAction);
  }

  createGUI();
}

/**
 * Get keyboard shortcuts.
 * @return mapping of action names to key sequences.
 */
QMap<QString, QKeySequence> KdeMainWindow::shortcutsMap() const
{
  QMap<QString, QKeySequence> map;
  if (KActionCollection* collection = actionCollection()) {
    const auto actions = collection->actions();
    for (QAction* action : actions) {
      if (action) {
        if (QString name = action->objectName(); !name.isEmpty()) {
          map.insert(name, action->shortcut());
        }
      }
    }
  }
  return map;
}

/**
 * Add directory to recent files list.
 *
 * @param dirName path to directory
 */
void KdeMainWindow::addDirectoryToRecentFiles(const QString& dirName)
{
  QUrl url;
  url.setPath(dirName);
  m_fileOpenRecent->addUrl(url);
}

/**
 * Read settings from the configuration.
 */
void KdeMainWindow::readConfig()
{
  auto cfg = KSharedConfig::openConfig();
#if KCONFIG_VERSION >= 0x054300
  auto stateCfg = KSharedConfig::openStateConfig();
#else
  auto stateCfg = cfg;
#endif
  setAutoSaveSettings(stateCfg->group("MainWindow"));
  m_settingsShowHidePicture->setChecked(!GuiConfig::instance().hidePicture());
  m_settingsAutoHideTags->setChecked(GuiConfig::instance().autoHideTags());
  m_fileOpenRecent->loadEntries(stateCfg->group("Recent Files"));

  QString entry = cfg->group("MainWindow").readEntry("StatusBar", "Enabled");
  bool statusBarVisible = entry != QLatin1String("Disabled");
  if (m_settingsShowStatusbar) {
    m_settingsShowStatusbar->setChecked(statusBarVisible);
  }
  setStatusBarVisible(statusBarVisible);
}

/**
 * Store geometry and recent files in settings.
 */
void KdeMainWindow::saveConfig()
{
#if KCONFIG_VERSION >= 0x054300
  auto stateCfg = KSharedConfig::openStateConfig();
#else
  auto stateCfg = KSharedConfig::openConfig();
#endif
  m_fileOpenRecent->saveEntries(stateCfg->group("Recent Files"));
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
  app()->openDirectory({cfg.readEntry("dirname", "")});
}

/**
 * Open recent directory.
 *
 * @param url URL of directory to open
 */
void KdeMainWindow::slotFileOpenRecentUrl(const QUrl& url)
{
  openRecentDirectory(url.path());
}

/**
 * Shortcuts configuration.
 */
void KdeMainWindow::slotSettingsShortcuts()
{
#if KCONFIGWIDGETS_VERSION >= 0x05f000
  KShortcutsDialog::showDialog(
        actionCollection(),
        KShortcutsEditor::LetterShortcutsAllowed, this);
  impl()->applyChangedShortcuts();
#else
  if (KShortcutsDialog::configure(
        actionCollection(),
        KShortcutsEditor::LetterShortcutsAllowed, this) ==
      QDialog::Accepted) {
    impl()->applyChangedShortcuts();
  }
#endif
}

/**
 * Toolbars configuration.
 */
void KdeMainWindow::slotSettingsToolbars()
{
  if (KEditToolBar dlg(actionCollection()); dlg.exec()) {
    createGUI();
  }
}

/**
 * Statusbar configuration.
 */
void KdeMainWindow::slotSettingsShowStatusbar()
{
  setStatusBarVisible(m_settingsShowStatusbar->isChecked());
  setSettingsDirty();
}

/**
 * Preferences.
 */
void KdeMainWindow::slotSettingsConfigure()
{
  QString caption(tr("Configure - Kid3"));
  auto configSkeleton = new KConfigSkeleton;
  auto dialog = new KdeConfigDialog(m_platformTools, this, caption,
                                                configSkeleton);
  dialog->setConfig();
  if (dialog->exec() == QDialog::Accepted) {
    dialog->getConfig();
    impl()->applyChangedConfiguration();
  }
  delete configSkeleton;
}

/**
 * Add user action to collection.
 * @param name name of action
 * @param action action to add
 */
void KdeMainWindow::onUserActionAdded(const QString& name, QAction* action)
{
  KActionCollection* collection = actionCollection();
  collection->addAction(name, action);
}

/**
 * Remove user action from collection.
 * @param name name of action
 * @param action action to remove
 */
void KdeMainWindow::onUserActionRemoved(const QString& name, QAction* action)
{
  Q_UNUSED(name)
  KActionCollection* collection = actionCollection();
  collection->takeAction(action);
}
