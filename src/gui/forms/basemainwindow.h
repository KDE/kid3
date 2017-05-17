/**
 * \file basemainwindow.h
 * Base class for main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2017  Urs Fleisch
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

#ifndef BASEMAINWINDOW_H
#define BASEMAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include "config.h"
#include "iframeeditor.h"
#include "trackdata.h"
#include "kid3api.h"

class QProgressBar;
class QToolButton;
class QItemSelection;
class ProgressWidget;
class Kid3Form;
class Kid3Application;
class TaggedFile;
class ImportDialog;
class BatchImportDialog;
class ExportDialog;
class FindReplaceDialog;
class BrowseCoverArtDialog;
class RenDirDialog;
class NumberTracksDialog;
class RenDirDialog;
class FilterDialog;
class FileFilter;
class DownloadDialog;
class PlaylistDialog;
class PlaylistConfig;
class EditFrameFieldsDialog;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
class PlayToolBar;
#endif
class DirContents;
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
class KID3_GUI_EXPORT BaseMainWindowImpl : public QObject, public IFrameEditor {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param mainWin main window widget
   * @param platformTools platform specific tools
   */
  BaseMainWindowImpl(QMainWindow* mainWin, IPlatformTools* platformTools, Kid3Application *app);

  /**
   * Destructor.
   */
  virtual ~BaseMainWindowImpl();

  /**
   * Create dialog to edit a frame and update the fields
   * if Ok is returned.
   * frameEdited() is emitted when the edit dialog is closed with the edited
   * frame as a parameter if it was accepted.
   *
   * @param frame frame to edit
   * @param taggedFile tagged file where frame has to be set
   */
  virtual void editFrameOfTaggedFile(const Frame* frame, TaggedFile* taggedFile);

  /**
   * Let user select a frame type.
   * frameSelected() is emitted when the edit dialog is closed with the selected
   * frame as a parameter if a frame is selected.
   *
   * @param frame is filled with the selected frame
   * @param taggedFile tagged file for which frame has to be selected
   */
  virtual void selectFrame(Frame* frame, const TaggedFile* taggedFile);

  /**
   * Return object which emits frameSelected(), frameEdited() signals.
   *
   * @return object which emits signals.
   */
  virtual QObject* qobject();

  /**
   * Get the tag number of the edited frame.
   * @return tag number.
   */
  virtual Frame::TagNumber tagNumber() const;

  /**
   * Set the tag number of the edited frame.
   * @param tagNr tag number
   */
  virtual void setTagNumber(Frame::TagNumber tagNr);

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
   * Set window title with information from directory, filter and modification
   * state.
   */
  void updateWindowCaption();

  /**
   * Open directory, user has to confirm if current directory modified.
   *
   * @param paths directory or file paths
   */
  void confirmedOpenDirectory(const QStringList& paths);

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
   * Find in tags of files.
   */
  void find() { findReplace(true); }

  /**
   * Find and replace in tags of files.
   * @param findOnly true to display only find part of dialog
   */
  void findReplace(bool findOnly = false);

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
   * Apply selection change and update GUI controls.
   * The new selection is stored and the GUI controls and frame list
   * updated accordingly (filtered for multiple selection).
   * @param selected selected items
   * @param deselected deselected items
   */
  void applySelectionChange(const QItemSelection& selected,
                            const QItemSelection& deselected);

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

signals:
  /**
   * Emitted when the dialog to add and edit a frame is closed.
   * @param tagNr tag number
   * @param frame edited frame if dialog was accepted, else 0
   */
  void frameEdited(Frame::TagNumber tagNr, const Frame* frame);

  /**
   * Emitted when the dialog to select a frame is closed.
   * @param tagNr tag number
   * @param frame selected frame if dialog was accepted, else 0
   */
  void frameSelected(Frame::TagNumber tagNr, const Frame* frame);

private slots:
  /**
   * Update ID3v2 tags in GUI controls from file displayed in frame list.
   *
   * @param taggedFile the selected file
   * @param tagNr tag number
   */
  void updateAfterFrameModification(TaggedFile* taggedFile,
                                    Frame::TagNumber tagNr);

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

  /**
   * Show filter operation progress.
   * @param type filter event type
   * @param fileName name of file processed
   * @param passed number of files which passed the filter
   * @param total total number of files checked
   */
  void filterProgress(int type, const QString& fileName, int passed, int total);

  /**
   * Set tagged files of directory from imported track data model.
   */
  void applyImportedTrackData();

  /**
   * Called when the edit fram dialog is finished.
   * @param result dialog result
   */
  void onEditFrameDialogFinished(int result);

  /**
   * Toggle expanded state of directory in the file list.
   * @param index index of directory
   */
  void toggleExpanded(const QModelIndex& index);

  /**
   * Deactivate showing of find replace results.
   */
  void deactivateFindReplace();

  /**
   * Ensure that found text is made visible in the GUI.
   */
  void showFoundText();

  /**
   * Update GUI controls after text has been replaced.
   */
  void updateReplacedText();

  /**
   * Show progress of long running operation in status bar.
   * @param name name of operation
   * @param done amount of work done
   * @param total total amount of work
   * @param abort if not 0, can be set to true to abort operation
   */
  void showOperationProgress(const QString& name, int done, int total,
                             bool* abort);

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
   * Write playlist according to playlist configuration.
   *
   * @param cfg playlist configuration to use
   *
   * @return true if ok.
   */
  bool writePlaylist(const PlaylistConfig& cfg);

  /**
   * Terminate expanding the file list.
   */
  void terminateExpandFileList();

  /**
   * Terminate filtering the file list.
   */
  void terminateFilter();

  /**
   * Update GUI controls from the current selection.
   */
  void updateGuiControlsFromSelection();

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
  void startProgressMonitoring(const QString& title, void (BaseMainWindowImpl::*terminationHandler)(), bool disconnectModel);

  /**
   * Stop monitoring the progress started with startProgressMonitoring().
   */
  void stopProgressMonitoring();

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
  void checkProgressMonitoring(int done, int total, const QString& text);

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
  /** Find and replace dialog */
  FindReplaceDialog* m_findReplaceDialog;
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
  ProgressWidget* m_progressWidget;
  QProgressBar* m_progressBar;
  QToolButton* m_progressAbortButton;
  /** Edit frame dialog */
  EditFrameFieldsDialog* m_editFrameDialog;
#if defined HAVE_PHONON || QT_VERSION >= 0x050000
  /** Play toolbar */
  PlayToolBar* m_playToolBar;
#endif
  Frame m_editFrame;
  TaggedFile* m_editFrameTaggedFile;
  Frame::TagNumber m_editFrameTagNr;
  QDateTime m_progressStartTime;
  QString m_progressTitle;
  void (BaseMainWindowImpl::*m_progressTerminationHandler)();
  bool m_progressDisconnected;
  bool m_findReplaceActive;
  bool m_expandNotificationNeeded;
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
class KID3_GUI_EXPORT BaseMainWindow {
public:
  /**
   * Constructor.
   *
   * @param mainWin main window
   * @param platformTools platform specific tools
   * @param app application context
   */
  BaseMainWindow(QMainWindow* mainWin, IPlatformTools* platformTools,
                 Kid3Application *app);

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
   * @param paths directory or file paths
   */
  void confirmedOpenDirectory(const QStringList& paths);

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

#endif /* BASEMAINWINDOW_H */
