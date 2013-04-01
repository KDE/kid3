/**
 * \file kid3mainwindow.h
 * Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
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

#ifndef KID3MAINWINDOW_H
#define KID3MAINWINDOW_H

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kdeversion.h>
#include <kxmlguiwindow.h>
class KAction;
class KRecentFilesAction;
class KToggleAction;
/** Base class for main window. */
typedef KXmlGuiWindow Kid3MainWindowBaseClass;
#else
#include <QMainWindow>
class QAction;
class RecentFilesMenu;
/** Base class for main window. */
typedef QMainWindow Kid3MainWindowBaseClass;
#endif
#include <QDateTime>
#include "iframeeditor.h"
#include "trackdata.h"
#include "kid3api.h"

class KURL;
class KUrl;
class Kid3Form;
class Kid3Application;
class TaggedFile;
class ImportDialog;
class BatchImportDialog;
class ExportDialog;
class BrowseCoverArtDialog;
class RenDirDialog;
class NumberTracksDialog;
class RenDirDialog;
class FilterDialog;
class FileFilter;
class QImage;
class QProgressDialog;
class DownloadDialog;
class PlaylistDialog;
class PlaylistConfig;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
class PlayToolBar;
#endif
class ConfigStore;
class DirContents;
class QFileSystemModel;
class QModelIndex;
class FileProxyModel;
class DirProxyModel;
class TrackDataModel;
class IPlatformTools;

class BaseMainWindow;

/**
 * Implementation class for BaseMainWindow.
 * The reason for this implementation class is that a QObject is needed to
 * have slots. However, BaseMainWindow cannot inherit from QObject because it
 * is used with multiple inheritance together with another class which is a
 * QObject (actually a QMainWindow). Therefore the functionality of the main
 * put into this class which is then used as an implementation class by
 * BaseMainWindow.
 */
class BaseMainWindowImpl : public QObject, public IFrameEditor {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param mainWin main window widget
   * @param platformTools platform specific tools
   */
  BaseMainWindowImpl(QMainWindow* mainWin, IPlatformTools* platformTools);

  /**
   * Destructor.
   */
  virtual ~BaseMainWindowImpl();

  /**
   * Create dialog to edit a frame and update the fields
   * if Ok is returned.
   *
   * @param frame frame to edit
   * @param taggedFile tagged file where frame has to be set
   *
   * @return true if Ok selected in dialog.
   */
  virtual bool editFrameOfTaggedFile(Frame* frame, TaggedFile* taggedFile);

  /**
   * Let user select a frame type.
   *
   * @param frame is filled with the selected frame if true is returned
   * @param taggedFile tagged file for which frame has to be selected
   *
   * @return false if no frame selected.
   */
  virtual bool selectFrame(Frame* frame, const TaggedFile* taggedFile);

  /**
   * Set back pointer for implementation class.
   *
   * @param self back pointer
   */
  void setBackPointer(BaseMainWindow* self) { m_self = self; }

  /**
   * Initialize main window.
   * Shall be called at end of constructor body.
   */
  void init();

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
  bool queryBeforeClosing();

  /**
   * Open recent directory.
   *
   * @param dir directory to open
   */
  void openRecentDirectory(const QString& dir);

  /**
   * Set window title with information from directory, filter and modification
   * state.
   */
  void updateWindowCaption();

  /**
   * Apply configuration changes.
   */
  void applyChangedConfiguration();

  /**
   * Access to application.
   * @return application.
   */
  Kid3Application* app() { return m_app; }

  /**
   * Access to main form.
   * @return main form.
   */
  Kid3Form* form() { return m_form; }

public slots:
  /**
   * Open directory, user has to confirm if current directory modified.
   *
   * @param dir directory or file path
   */
  void confirmedOpenDirectory(const QString& dir);

  /**
   * Update the recent file list and the caption when a new directory
   * is opened.
   */
  void onDirectoryOpened();

  /**
   * Request new directory and open it.
   */
  void slotFileOpen();

  /**
   * Request new directory and open it.
   */
  void slotFileOpenDirectory();

  /**
   * Save modified files.
   */
  void slotFileSave();

  /**
   * Quit application.
   */
  void slotFileQuit();

  /**
   * Change status message.
   *
   * @param text message
   */
  void slotStatusMsg(const QString& text);

  /**
   * Show playlist dialog.
   */
  void slotPlaylistDialog();

  /**
   * Create playlist.
   *
   * @return true if ok.
   */
  bool slotCreatePlaylist();

  /**
   * Import.
   */
  void slotImport();

  /**
   * Batch import.
   */
  void slotBatchImport();

  /**
   * Browse album cover artwork.
   */
  void slotBrowseCoverArt();

  /**
   * Export.
   */
  void slotExport();

  /**
   * Toggle auto hiding of tags.
   */
  void slotSettingsAutoHideTags();

  /**
   * Show or hide picture.
   */
  void slotSettingsShowHidePicture();

  /**
   * Rename directory.
   */
  void slotRenameDirectory();

  /**
   * Number tracks.
   */
  void slotNumberTracks();

  /**
   * Filter.
   */
  void slotFilter();

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /**
   * Play audio file.
   */
  void slotPlayAudio();
#endif

  /**
   * Update files of current selection.
   */
  void updateCurrentSelection();

  /**
   * Update GUI controls from the tags in the files.
   * The new selection is stored and the GUI controls and frame list
   * updated accordingly (filtered for multiple selection).
   */
  void updateGuiControls();

  /**
   * Rename the selected file(s).
   */
  void renameFile();

  /**
   * Delete the selected file(s).
   */
  void deleteFile();

  /**
   * Expand the file list.
   */
  void expandFileList();

private slots:
  /**
   * Update ID3v2 tags in GUI controls from file displayed in frame list.
   *
   * @param taggedFile the selected file
   */
  void updateAfterFrameModification(TaggedFile* taggedFile);

  /**
   * Update modification state, caption and listbox entries.
   */
  void updateModificationState();

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /**
   * Show play tool bar.
   */
  void showPlayToolBar();
#endif

  /**
   * Expand item if it is a directory.
   *
   * @param index index of file in file proxy model
   */
  void expandNextDirectory(const QPersistentModelIndex& index);

private:
  /**
   * Free allocated resources.
   * Our destructor may not be called, so cleanup is done here.
   */
  void cleanup();

  /**
   * Save application options.
   */
  void saveOptions();

  /**
   * Load application options.
   */
  void readOptions();

  /**
   * Save all changed files.
   *
   * @param updateGui true to update GUI (controls, status, cursor)
   */
  void saveDirectory(bool updateGui = false);

  /**
   * If anything was modified, save after asking user.
   *
   * @param doNotRevert if true, modifications are not reverted, this can be
   * used to skip the possibly long process if the application is not be closed
   *
   * @return false if user canceled.
   */
  bool saveModified(bool doNotRevert = false);

  /**
   * Update track data and create import dialog.
   */
  void setupImportDialog();

  /**
   * Execute the import dialog.
   */
  void execImportDialog();

  /**
   * Write playlist according to playlist configuration.
   *
   * @param cfg playlist configuration to use
   *
   * @return true if ok.
   */
  bool writePlaylist(const PlaylistConfig& cfg);

  IPlatformTools* m_platformTools;
  QMainWindow* m_w;
  BaseMainWindow* m_self;

  /** GUI with controls */
  Kid3Form* m_form;
  /** Application logic */
  Kid3Application* m_app;
  /** Import dialog */
  ImportDialog* m_importDialog;
  /** Batch import dialog */
  BatchImportDialog* m_batchImportDialog;
  /** Browse cover art dialog */
  BrowseCoverArtDialog* m_browseCoverArtDialog;
  /** Export dialog */
  ExportDialog* m_exportDialog;
  /** Rename directory dialog */
  RenDirDialog* m_renDirDialog;
  /** Number tracks dialog */
  NumberTracksDialog* m_numberTracksDialog;
  /** Filter dialog */
  FilterDialog* m_filterDialog;
  /** Download dialog */
  DownloadDialog* m_downloadDialog;
  /** Playlist dialog */
  PlaylistDialog* m_playlistDialog;
  /** Progress dialog */
  QProgressDialog* m_progressDialog;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /** Play toolbar */
  PlayToolBar* m_playToolBar;
#endif
  QDateTime m_expandFileListStartTime;
};


/**
 * Base class for the main window.
 * The main window classes for Qt (QMainWindow) and KDE (KXmlGuiWindow)
 * have common functionality. The actual Kid3 main window can inherit from both
 * the platform dependent main window class and this base class. Differences
 * between the platforms can be handled by implementing the pure virtual methods
 * of this class. Because this class cannot be a QObject (QMainWindow is
 * already a QObject), most of its functionality is delegated to a QObject
 * implementation class.
 */
class KID3_GUI_EXPORT BaseMainWindow : public IFrameEditor {
public:
  /**
   * Constructor.
   *
   * @param mainWin main window instance
   * @param platformTools platform specific tools
   */
  BaseMainWindow(QMainWindow* mainWin, IPlatformTools* platformTools);

  /**
   * Destructor.
   */
  virtual ~BaseMainWindow();

  /**
   * Init menu and toolbar actions.
   */
  virtual void initActions() = 0;

  /**
   * Add directory to recent files list.
   *
   * @param dirName path to directory
   */
  virtual void addDirectoryToRecentFiles(const QString& dirName) = 0;

  /**
   * Read settings from the configuration.
   */
  virtual void readConfig() = 0;

  /**
   * Store geometry and recent files in settings.
   */
  virtual void saveConfig() = 0;

  /**
   * Get action for Settings/Auto Hide Tags.
   * @return action.
   */
  virtual QAction* autoHideTagsAction() = 0;

  /**
   * Get action for Settings/Hide Picture.
   * @return action.
   */
  virtual QAction* showHidePictureAction() = 0;

  /**
   * Set main window caption.
   *
   * @param caption caption without application name
   * @param modified true if any file is modified
   */
  virtual void setWindowCaption(const QString& caption, bool modified) = 0;

  /**
   * Create dialog to edit a frame and update the fields
   * if Ok is returned.
   *
   * @param frame frame to edit
   * @param taggedFile tagged file where frame has to be set
   *
   * @return true if Ok selected in dialog.
   */
  virtual bool editFrameOfTaggedFile(Frame* frame, TaggedFile* taggedFile);

  /**
   * Let user select a frame type.
   *
   * @param frame is filled with the selected frame if true is returned
   * @param taggedFile tagged file for which frame has to be selected
   *
   * @return false if no frame selected.
   */
  virtual bool selectFrame(Frame* frame, const TaggedFile* taggedFile);

#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /**
   * Play audio file.
   */
  void slotPlayAudio();
#endif

  /**
   * Update files of current selection.
   */
  void updateCurrentSelection();

  /**
   * Open directory, user has to confirm if current directory modified.
   *
   * @param dir directory or file path
   */
  void confirmedOpenDirectory(const QString& dir);

  /**
   * Access to implementation object.
   * @return implementation object.
   */
  BaseMainWindowImpl* impl() { return m_impl; }

protected:
  /**
   * Initialize main window.
   * Shall be called at end of constructor body in derived classes.
   */
  void init();

  /**
   * Change status message.
   *
   * @param text message
   */
  void slotStatusMsg(const QString& text);

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
  bool queryBeforeClosing();

  /**
   * Open recent directory.
   *
   * @param dir directory to open
   */
  void openRecentDirectory(const QString& dir);

  /**
   * Set window title with information from directory, filter and modification
   * state.
   */
  void updateWindowCaption();

  /**
   * Access to application.
   * @return application.
   */
  Kid3Application* app();

  /**
   * Access to main form.
   * @return main form.
   */
  Kid3Form* form();

private:
  BaseMainWindowImpl* m_impl;
};


/** Kid3 main window */
class KID3_GUI_EXPORT Kid3MainWindow :
    public Kid3MainWindowBaseClass, public BaseMainWindow {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform specific tools
   */
  explicit Kid3MainWindow(IPlatformTools* platformTools);

  /**
   * Destructor.
   */
  ~Kid3MainWindow();

  /**
   * Init menu and toolbar actions.
   */
  virtual void initActions();

  /**
   * Add directory to recent files list.
   *
   * @param dirName path to directory
   */
  virtual void addDirectoryToRecentFiles(const QString& dirName);

  /**
   * Read settings from the configuration.
   */
  virtual void readConfig();

  /**
   * Store geometry and recent files in settings.
   */
  virtual void saveConfig();

  /**
   * Get action for Settings/Auto Hide Tags.
   * @return action.
   */
  virtual QAction* autoHideTagsAction();

  /**
   * Get action for Settings/Hide Picture.
   * @return action.
   */
  virtual QAction* showHidePictureAction();

  /**
   * Set main window caption.
   *
   * @param caption caption without application name
   * @param modified true if any file is modified
   */
  virtual void setWindowCaption(const QString& caption, bool modified);

protected:
#ifdef CONFIG_USE_KDE
  /**
   * Update modification state before closing.
   * Called on closeEvent() of window.
   * If anything was modified, save after asking user.
   *
   * @return FALSE if user canceled.
   */
  virtual bool queryClose();

  /**
   * Saves the window properties for each open window during session end
   * to the session config file.
   *
   * @param cfg application configuration
   */
  virtual void saveProperties(KConfigGroup& cfg);

  /**
   * Reads the session config file and restores the application's state.
   *
   * @param cfg application configuration
   */
  virtual void readProperties(const KConfigGroup& cfg);

#else
  /**
   * Window is closed.
   *
   * @param ce close event
   */
  void closeEvent(QCloseEvent* ce);

  /**
   * Read font and style options.
   */
  void readFontAndStyleOptions();
#endif

public slots:
#ifdef CONFIG_USE_KDE
  /**
   * Open recent directory.
   *
   * @param url URL of directory to open
   */
  void slotFileOpenRecentUrl(const KUrl& url);
#else
  /**
   * Open recent directory.
   *
   * @param dir directory to open
   */
  void slotFileOpenRecentDirectory(const QString& dir);
#endif

  /**
   * Turn status bar on or off.
   */
  void slotViewStatusBar();

  /**
   * Shortcuts configuration.
   */
  void slotSettingsShortcuts();

  /**
   * Toolbars configuration.
   */
  void slotSettingsToolbars();

  /**
   * Preferences.
   */
  void slotSettingsConfigure();

  /**
   * Display handbook.
   */
  void slotHelpHandbook();

  /**
   * Display "About" dialog.
   */
  void slotHelpAbout();

  /**
   * Display "About Qt" dialog.
   */
  void slotHelpAboutQt();

private:
#ifdef CONFIG_USE_KDE
  /** Actions */
  KRecentFilesAction* m_fileOpenRecent;
  KToggleAction* m_viewToolBar;
  KToggleAction* m_viewStatusBar;
  KToggleAction* m_settingsAutoHideTags;
  KToggleAction* m_settingsShowHidePicture;
#else
  RecentFilesMenu* m_fileOpenRecent;
  QAction* m_viewToolBar;
  QAction* m_viewStatusBar;
  QAction* m_settingsAutoHideTags;
  QAction* m_settingsShowHidePicture;
#endif
};

#endif /* KID3MAINWINDOW_H */
