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
#include "iframeeditor.h"
#include "trackdata.h"

class KURL;
class KUrl;
class Kid3Form;
class Kid3Application;
class TaggedFile;
class FrameList;
class ImportDialog;
class ExportDialog;
class BrowseCoverArtDialog;
class RenDirDialog;
class NumberTracksDialog;
class RenDirDialog;
class FilterDialog;
class FileFilter;
class QImage;
class DownloadDialog;
class PlaylistDialog;
class PlaylistConfig;
#ifdef HAVE_PHONON
class PlayToolBar;
#endif
class ConfigStore;
class DirContents;
class QFileSystemModel;
class QModelIndex;
class FileProxyModel;
class DirProxyModel;
class TrackDataModel;

/** Kid3 main window */
class Kid3MainWindow : public Kid3MainWindowBaseClass, public IFrameEditor
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	Kid3MainWindow();

	/**
	 * Destructor.
	 */
	~Kid3MainWindow();

	/**
	 * Set the directory name from the tags.
	 * The directory must not have modified files.
	 *
	 * @param tagMask tag mask
	 * @param format  directory name format
	 * @param create  true to create, false to rename
	 * @param errStr  if not 0, a string describing the error is returned here
	 *
	 * @return true if ok.
	 */
	bool renameDirectory(TrackData::TagVersion tagMask, const QString& format,
											 bool create, QString* errStr);

	/**
	 * Number tracks in selected files of directory.
	 *
	 * @param nr start number
	 * @param total total number of tracks, used if >0
	 * @param destV1 true to set numbers in tag 1
	 * @param destV2 true to set numbers in tag 2
	 */
	void numberTracks(int nr, int total, bool destV1, bool destV2);

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

protected:
	/**
	 * Init menu and toolbar actions.
	 */
	void initActions();

	/**
	 * Init status bar.
	 */
	void initStatusBar();

	/**
	 * Init GUI.
	 */
	void initView();

	/**
	 * Free allocated resources.
	 * Our destructor may not be called, so cleanup is done here.
	 */
	void cleanup();

	/**
	 * Update modification state before closing.
	 * Called on closeEvent() of window.
	 * If anything was modified, save after asking user.
	 *
	 * @return FALSE if user canceled.
	 */
	virtual bool queryClose();

#ifdef CONFIG_USE_KDE
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

	/**
	 * Save application options.
	 */
	void saveOptions();

	/**
	 * Load application options.
	 */
	void readOptions();

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
	 * Open recent directory.
	 *
	 * @param url URL of directory to open
	 */
	void slotFileOpenRecentUrl(const KUrl& url);

	/**
	 * Open recent directory.
	 *
	 * @param dir directory to open
	 */
	void slotFileOpenRecentDirectory(const QString& dir);

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
	 * Import from freedb.org.
	 */
	void slotImportFreedb();

	/**
	 * Import from TrackType.org.
	 */
	void slotImportTrackType();

	/**
	 * Import from Discogs.
	 */
	void slotImportDiscogs();

	/**
	 * Import from Amazon.
	 */
	void slotImportAmazon();

	/**
	 * Import from MusicBrainz release database.
	 */
	void slotImportMusicBrainzRelease();

	/**
	 * Import from MusicBrainz.
	 */
	void slotImportMusicBrainz();

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
	 * Preferences.
	 */
	void slotSettingsConfigure();

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

	/**
	 * Play audio file.
	 */
	void slotPlayAudio();

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

private slots:
	/**
	 * Apply a file filter.
	 *
	 * @param fileFilter filter to apply.
	 */
	void applyFilter(FileFilter& fileFilter);

	/**
	 * Schedule actions to rename a directory.
	 */
	void scheduleRenameActions();

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

private:
	friend class ScriptInterface;

	/**
	 * Save all changed files.
	 *
	 * @param updateGui true to update GUI (controls, status, cursor)
	 */
	void saveDirectory(bool updateGui = false);

	/**
	 * If anything was modified, save after asking user.
	 *
	 * @return FALSE if user canceled.
	 */
	bool saveModified();

	/**
	 * Set window title with information from directory, filter and modification
	 * state.
	 */
	void updateWindowCaption();

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

	/**
	 * Apply a file filter to a directory.
	 *
	 * @param fileFilter filter to apply
	 * @param model the model to be filtered
	 * @param rootIndex model index of root directory
	 *
	 * @return true if ok, false if aborted.
	 */
	bool applyFilterToDir(FileFilter& fileFilter, FileProxyModel* model,
												const QModelIndex& rootIndex);

	/** GUI with controls */
	Kid3Form* m_form;
	/** Application logic */
	Kid3Application* m_app;
	/** Import dialog */
	ImportDialog* m_importDialog;
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
#ifdef HAVE_PHONON
	/** Play toolbar */
	PlayToolBar* m_playToolBar;
#endif
	/** Frame list */
	FrameList* m_framelist;

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
