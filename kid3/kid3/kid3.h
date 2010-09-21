/**
 * \file kid3.h
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2009  Urs Fleisch
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

#ifndef KID3_H
#define KID3_H

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kdeversion.h>
class KAction;
class KRecentFilesAction;
class KToggleAction;
/** Base class for main window. */
#if KDE_VERSION >= 0x035c00
#include <kxmlguiwindow.h>
typedef KXmlGuiWindow Kid3AppBaseClass;
#else
#include <kmainwindow.h>
typedef KMainWindow Kid3AppBaseClass;
#endif
#else
#include "qtcompatmac.h"
#include <qmainwindow.h>
#include "generalconfig.h" // Kid3Settings
class QAction;
class BrowserDialog;
class RecentFilesMenu;
/** Base class for main window. */
typedef QMainWindow Kid3AppBaseClass;
#endif
#include "importtrackdata.h"
#include "formatconfig.h"
#include "importconfig.h"
#include "miscconfig.h"
#include "freedbconfig.h"
#include "discogsconfig.h"
#include "amazonconfig.h"
#include "musicbrainzconfig.h"
#include "filterconfig.h"
#include "playlistconfig.h"
#include "frame.h"

class KURL;
class KUrl;
class Id3Form;
class TaggedFile;
class FileListItem;
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
#ifdef HAVE_PHONON
class PlayToolBar;
#endif
class DirContents;

/** Kid3 application */
class Kid3App : public Kid3AppBaseClass
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 */
	Kid3App();

	/**
	 * Destructor.
	 */
	~Kid3App();

	/**
	 * Open directory.
	 *
	 * @param dir       directory or file path
	 * @param confirm   if true ask if there are unsaved changes
	 * @param fileCheck if true and dir in not directory, only open directory
	 *                  if dir is a valid file path
	 *
	 * @return true if ok.
	 */
	bool openDirectory(QString dir, bool confirm = false, bool fileCheck = false);

	/**
	 * Process change of selection.
	 * The files of the current selection are updated.
	 * The new selection is stored and the GUI controls and frame list
	 * updated accordingly (filtered for multiple selection).
	 */
	void fileSelected();

	/**
	 * Update files of current selection.
	 */
	void updateCurrentSelection();

	/**
	 * Copy tags 1 into copy buffer.
	 */
	void copyTagsV1();

	/**
	 * Copy tags 2 into copy buffer.
	 */
	void copyTagsV2();

	/**
	 * Paste from copy buffer to ID3v1 tags.
	 */
	void pasteTagsV1();

	/**
	 * Paste from copy buffer to ID3v2 tags.
	 */
	void pasteTagsV2();

	/**
	 * Set ID3v1 tags according to filename.
	 * If a single file is selected the tags in the GUI controls
	 * are set, else the tags in the multiple selected files.
	 */
	void getTagsFromFilenameV1();

	/**
	 * Set ID3v2 tags according to filename.
	 * If a single file is selected the tags in the GUI controls
	 * are set, else the tags in the multiple selected files.
	 */
	void getTagsFromFilenameV2();

	/**
	 * Set filename according to tags.
	 * If a single file is selected the tags in the GUI controls
	 * are used, else the tags in the multiple selected files.
	 *
	 * @param tag_version 1=ID3v1, 2=ID3v2
	 */
	void getFilenameFromTags(int tag_version);

	/**
	 * Copy ID3v1 tags to ID3v2 tags of selected files.
	 */
	void copyV1ToV2();

	/**
	 * Copy ID3v2 tags to ID3v1 tags of selected files.
	 */
	void copyV2ToV1();

	/**
	 * Remove ID3v1 tags in selected files.
	 */
	void removeTagsV1();

	/**
	 * Remove ID3v2 tags in selected files.
	 */
	void removeTagsV2();

	/**
	 * Open directory on drop.
	 *
	 * @param txt URL of directory or file in directory
	 */
	void openDrop(QString txt);

	/**
	 * Add picture on drop.
	 *
	 * @param image dropped image.
	 */
	void dropImage(const QImage& image);

	/**
	 * Download an image file.
	 *
	 * @param url           URL of image
	 * @param allFilesInDir true to add the image to all files in the directory
	 */
	void downloadImage(const QString& url, bool allFilesInDir);

	/**
	 * Handle URL on drop.
	 *
	 * @param txt dropped URL.
	 */
	void dropUrl(const QString& txt);

	/**
	 * Edit selected frame.
	 */
	void editFrame();

	/**
	 * Delete selected frame.
	 *
	 * @param frameName name of frame to delete, empty to delete selected frame
	 */
	void deleteFrame(const QString& frameName = QString::null);

	/**
	 * Select a frame type and add such a frame to frame list.
	 *
	 * @param frame frame to add, if 0 the user has to select and edit the frame
	 * @param edit  if frame is set and edit is true, the user can edit the frame
	 *              before it is added
	 */
	void addFrame(const Frame* frame = 0, bool edit = false);

	/**
	 * Set the directory name from the tags.
	 * The directory must not have modified files.
	 *
	 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
	 * @param format  directory name format
	 * @param create  true to create, false to rename
	 * @param errStr  if not 0, a string describing the error is returned here
	 *
	 * @return true if ok.
	 */
	bool renameDirectory(int tagMask, const QString& format,
											 bool create, QString* errStr);

	/**
	 * Number tracks in selected files of directory.
	 *
	 * @param nr start number
	 * @param destV1 true to set numbers in tag 1
	 * @param destV2 true to set numbers in tag 2
	 */
	void numberTracks(int nr, bool destV1, bool destV2);

	/**
	 * Export.
	 *
	 * @param tagNr  tag number (1 or 2)
	 * @param path   path of file
	 * @param fmtIdx index of format
	 *
	 * @return true if ok.
	 */
	bool exportTags(int tagNr, const QString& path, int fmtIdx);

	/**
	 * Edit a picture frame if one exists or add a new one.
	 */
	void editOrAddPicture();

	/**
	 * Display help for a topic.
	 *
	 * @param anchor anchor in help document
	 */
	static void displayHelp(const QString& anchor = QString::null);

	/**
	 * Get directory name.
	 * @return directory.
	 */
	static QString getDirName() { return s_dirName; }

	/**
	 * Read file with TagLib if it has an ID3v2.4 tag.
	 *
	 * @param item       file list item, can be updated if not null
	 * @param taggedFile tagged file
	 *
	 * @return tagged file (can be new TagLibFile).
	 */
	static TaggedFile* readWithTagLibIfId3V24(FileListItem* item,
																						TaggedFile* taggedFile);

	/** Filename format configuration */
	static FormatConfig s_fnFormatCfg;
	/** ID3 format configuration */
	static FormatConfig s_id3FormatCfg;
	/** Import configuration */
	static ImportConfig s_genCfg;
	/** Miscellaneous configuration */
	static MiscConfig s_miscCfg;
	/** Freedb configuration */
	static FreedbConfig s_freedbCfg;
	/** TrackType configuration */
	static FreedbConfig s_trackTypeCfg;
	/** Discogs configuration */
	static DiscogsConfig s_discogsCfg;
	/** Amazon configuration */
	static AmazonConfig s_amazonCfg;
	/** MusicBrainz configuration */
	static MusicBrainzConfig s_musicBrainzCfg;
	/** Filter configuration */
	static FilterConfig s_filterCfg;
	/** Playlist configuration */
	static PlaylistConfig s_playlistCfg;

protected:
	/**
	 * Init menu and toolbar actions.
	 */
	void initActions();

	/**
	 * Init file types.
	 */
	void initFileTypes();

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
	 * Create a filter string for the file dialog.
	 * The filter string contains entries for all supported types.
	 *
	 * @param defaultNameFilter if not 0, return default name filter here
	 *
	 * @return filter string.
	 */
	QString createFilterString(QString* defaultNameFilter = 0) const;

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
	virtual void saveProperties(
#if KDE_VERSION >= 0x035c00
		KConfigGroup& cfg
#else
		KConfig* cfg
#endif
		);

	/**
	 * Reads the session config file and restores the application's state.
	 *
	 * @param cfg application configuration
	 */
	virtual void readProperties(
#if KDE_VERSION >= 0x035c00
		const KConfigGroup& cfg
#else
		KConfig* cfg
#endif
		);

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

	/**
	 * Set modification state.
	 *
	 * @param val true if a file is modified
	 */
	void setModified(bool val) { m_modified = val; }

	/**
	 * Check modification state.
	 *
	 * @return true if a file is modified.
	 */
	bool isModified() { return m_modified; }

public slots:
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
	void slotFileOpenRecent(const KURL& url);

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
	 * Quit application.
	 */
	void slotViewToolBar();

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
	 * Revert file modifications.
	 * Acts on selected files or all files if no file is selected.
	 */
	void slotFileRevert();

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
	 * Apply filename format.
	 */
	void slotApplyFilenameFormat();

	/**
	 * Apply ID3 format.
	 */
	void slotApplyId3Format();

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
	 * Convert ID3v2.3 to ID3v2.4 tags.
	 */
	void slotConvertToId3v24();

	/**
	 * Convert ID3v2.4 to ID3v2.3 tags.
	 */
	void slotConvertToId3v23();

	/**
	 * Play audio file.
	 */
	void slotPlayAudio();

	/**
	 * Update GUI controls from the tags in the files.
	 * The new selection is stored and the GUI controls and frame list
	 * updated accordingly (filtered for multiple selection).
	 */
	void updateGuiControls();

private slots:
	/**
	 * Set data to be exported.
	 *
	 * @param src ExportDialog::SrcV1 to export ID3v1,
	 *            ExportDialog::SrcV2 to export ID3v2
	 */
	void setExportData(int src);

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
	 * Add a downloaded image.
	 *
	 * @param data     HTTP response of download
	 * @param mimeType MIME type of data
	 * @param url      URL of downloaded data
	 */
	void imageDownloaded(const QByteArray& data,
	                     const QString& mimeType, const QString& url);

private:
	friend class ScriptInterface;

	/**
	 * Save all changed files.
	 *
	 * @param updateGui true to update GUI (controls, status, cursor)
	 * @param errStr    if not 0, the error string is returned here and
	 *                  no dialog is displayed
	 *
	 * @return true if ok.
	 */
	bool saveDirectory(bool updateGui = false, QString* errStr = 0);

	/**
	 * If anything was modified, save after asking user.
	 *
	 * @return FALSE if user canceled.
	 */
	bool saveModified();

	/**
	 * Update modification state, caption and listbox entries.
	 */
	void updateModificationState();

	/**
	 * Update ID3v2 tags in GUI controls from file displayed in frame list.
	 *
	 * @param taggedFile the selected file
	 */
	void updateAfterFrameModification(TaggedFile* taggedFile);

	/**
	 * Get the selected file.
	 *
	 * @return the selected file,
	 *         0 if not exactly one file is selected
	 */
	TaggedFile* getSelectedFile();
	
	/**
	 * Format a filename if format while editing is switched on.
	 *
	 * @param taggedFile file to modify
	 */
	void formatFileNameIfEnabled(TaggedFile* taggedFile) const;

	/**
	 * Format frames if format while editing is switched on.
	 *
	 * @param frames frames
	 */
	void formatFramesIfEnabled(FrameCollection& frames) const;

	/**
	 * Update track data and create import dialog.
	 */
	void setupImportDialog();

	/**
	 * Import tags from the import dialog.
	 *
	 * @param destV1 true to set tag 1
	 * @param destV2 true to set tag 2
	 */
	void getTagsFromImportDialog(bool destV1, bool destV2);

	/**
	 * Execute the import dialog.
	 */
	void execImportDialog();

	/**
	 * Import.
	 *
	 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
	 * @param path    path of file
	 * @param fmtIdx  index of format
	 *
	 * @return true if ok.
	 */
	bool importTags(int tagMask, const QString& path, int fmtIdx);

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
	 * @param dirContents directory contents, will be filled with results
	 *
	 * @return true if ok, false if aborted.
	 */
	bool applyFilterToDir(FileFilter& fileFilter, DirContents* dirContents);

	/**
	 * Set filter state.
	 *
	 * @param val true if list is filtered
	 */
	void setFiltered(bool val) { m_filtered = val; }

	/**
	 * Check filter state.
	 *
	 * @return true if list is filtered.
	 */
	bool isFiltered() { return m_filtered; }

	/** GUI with controls, created by Qt Designer */
	Id3Form* m_view;
	/** true if any file was modified */
	bool m_modified;
	/** true if list is filtered */
	bool m_filtered;
	/** true to add frame to all files in directory in imageDownloaded() */
	bool m_downloadToAllFilesInDir;
	/** Copy buffer */
	FrameCollection m_copyTags;
	/** Import dialog */
	ImportDialog* m_importDialog;
	/** Track data list */
	ImportTrackDataVector m_trackDataList;
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
	/** the configuration object of the application */
	KConfig* m_config;
	/** Actions */
	KRecentFilesAction* m_fileOpenRecent;
	KToggleAction* m_viewToolBar;
	KToggleAction* m_viewStatusBar;
	KToggleAction* m_settingsAutoHideTags;
	KToggleAction* m_settingsShowHidePicture;
#else
	Kid3Settings* m_config;
	RecentFilesMenu* m_fileOpenRecent;
#if QT_VERSION >= 0x040000
	QAction* m_viewToolBar;
#endif
	QAction* m_viewStatusBar;
	QAction* m_settingsAutoHideTags;
	QAction* m_settingsShowHidePicture;

	static BrowserDialog* s_helpBrowser;
#endif

	/** Current directory */
	static QString s_dirName;
};

#endif /* KID3_H */
