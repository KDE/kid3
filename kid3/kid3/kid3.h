/**
 * \file kid3.h
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef KID3_H
#define KID3_H

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kmainwindow.h>
class KAction;
class KRecentFilesAction;
class KToggleAction;
/** Base class for main window. */
typedef KMainWindow Kid3AppBaseClass;
#else
#include "qtcompatmac.h"
#include <qmainwindow.h>
#include "generalconfig.h" // Kid3Settings
class QAction;
class BrowserDialog;
#if QT_VERSION >= 0x040000
class QMenu;
#else
class QPopupMenu;
#endif
/** Base class for main window. */
typedef QMainWindow Kid3AppBaseClass;
#endif
#include "importtrackdata.h"
#include "formatconfig.h"
#include "importconfig.h"
#include "miscconfig.h"
#include "freedbconfig.h"
#include "discogsconfig.h"
#include "musicbrainzconfig.h"

class KURL;
class Id3Form;
class TaggedFile;
class StandardTags;
class FrameList;
class ImportDialog;
class ExportDialog;
class NumberTracksDialog;

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
	 * @param dir     directory or file path
 	 * @param confirm if true ask if there are unsaved changes
	 */
	void openDirectory(QString dir, bool confirm = false);

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
	 * Copy a set of standard tags into copy buffer.
	 *
	 * @param st tags to copy
	 */
	void copyTags(const StandardTags* st);

	/**
	 * Paste from copy buffer to standard tags.
	 *
	 * @param st tags to fill from data in copy buffer.
	 */
	void pasteTags(StandardTags* st);

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
	 * Edit selected frame.
	 */
	void editFrame();

	/**
	 * Delete selected frame.
	 */
	void deleteFrame();

	/**
	 * Select a frame type and add such a frame to frame list.
	 */
	void addFrame();

	/**
	 * Display help for a topic.
	 *
	 * @param anchor anchor in help document
	 */
	static void displayHelp(const QString& anchor = QString::null);

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
	/** MusicBrainz configuration */
	static MusicBrainzConfig s_musicBrainzCfg;

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
	virtual void saveProperties(KConfig* cfg);

	/**
	 * Reads the session config file and restores the application's state.
	 *
	 * @param cfg application configuration
	 */
	virtual void readProperties(KConfig* cfg);

#else
	/**
	 * Window is closed.
	 *
	 * @param ce close event
	 */
	void closeEvent(QCloseEvent* ce);
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
	 * Open recent directory.
	 *
	 * @param url URL of directory to open
	 */
	void slotFileOpenRecent(const KURL& url);

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
	 * Create playlist.
	 */
	void slotCreatePlaylist();

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
	 * Import from MusicBrainz release database.
	 */
	void slotImportMusicBrainzRelease();

	/**
	 * Import from MusicBrainz.
	 */
	void slotImportMusicBrainz();

	/**
	 * Export.
	 */
	void slotExport();

	/**
	 * Show or hide ID3v1.1 controls.
	 */
	void slotSettingsShowHideV1();

	/**
	 * Show or hide ID3v2.3 controls.
	 */
	void slotSettingsShowHideV2();

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
	 * Convert ID3v2.3 to ID3v2.4 tags.
	 */
	void slotConvertToId3v24();

	/**
	 * Convert ID3v2.4 to ID3v2.3 tags.
	 */
	void slotConvertToId3v23();

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

private:
	/**
	 * Save all changed files.
	 *
	 * @return true
	 */
	bool saveDirectory();

	/**
	 * If anything was modified, save after asking user.
	 *
	 * @return FALSE if user canceled.
	 */
	bool saveModified();

	/**
	 * Set tags in file to tags in GUI controls.
	 *
	 * @param mp3file file
	 */
	void updateTags(TaggedFile* mp3file);

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
	 * Get the selected file together with its frame list.
	 * If multiple files are selected, 0 is returned for both parameters.
	 *
	 * @param taggedFile the file is returned here,
	 *                   0 if not exactly one file is selected
	 * @param framelist  the frame list is returned here,
	 *                   0 if not exactly one file is selected
	 */
	void getSelectedFileWithFrameList(
		TaggedFile*& taggedFile, FrameList*& framelist);
	
	/**
	 * Format a filename if format while editing is switched on.
	 *
	 * @param taggedFile file to modify
	 */
	void formatFileNameIfEnabled(TaggedFile* taggedFile) const;

	/**
	 * Format tags if format while editing is switched on.
	 *
	 * @param st standard tags
	 */
	void formatStandardTagsIfEnabled(StandardTags* st) const;

	/**
	 * Update track data and create import dialog.
	 */
	void setupImportDialog();

	/**
	 * Execute the import dialog.
	 */
	void execImportDialog();

	/**
	 * Show or hide the ID3V1.1 controls according to the settings and
	 * set the menu entries appropriately.
	 */
	void updateHideV1();

	/**
	 * Show or hide the ID3V2.3 controls according to the settings and
	 * set the menu entries appropriately.
	 */
	void updateHideV2();

	/** GUI with controls, created by Qt Designer */
	Id3Form* m_view;
	/** true if any file was modified */
	bool m_modified;
	/** Current directory */
	QString m_dirName;
	/** Copy buffer */
	StandardTags* m_copyTags;
	/** Import dialog */
	ImportDialog* m_importDialog;
	/** Track data list */
	ImportTrackDataVector m_trackDataList;
	/** Export dialog */
	ExportDialog* m_exportDialog;
	/** Number tracks dialog */
	NumberTracksDialog* m_numberTracksDialog;

#ifdef CONFIG_USE_KDE
	/** the configuration object of the application */
	KConfig* m_config;
	/** Actions */
	KAction* m_fileOpen;
	KRecentFilesAction* m_fileOpenRecent;
	KAction* m_fileRevert;
	KAction* m_fileSave;
	KAction* m_fileQuit;
	KToggleAction* m_viewToolBar;
	KToggleAction* m_viewStatusBar;
	KAction* settingsShortcuts;
	KAction* m_settingsConfigure;
	KAction* m_settingsShowHideV1;
	KAction* m_settingsShowHideV2;
#else
	Kid3Settings* m_config;
	QAction* m_fileOpen;
	QAction* m_fileSave;
	QAction* m_fileRevert;
	QAction* m_fileCreatePlaylist;
	QAction* m_fileQuit;
	QAction* m_fileImport;
	QAction* m_fileImportFreedb;
	QAction* m_fileImportTrackType;
	QAction* m_fileImportDiscogs;
	QAction* m_fileImportMusicBrainzRelease;
#ifdef HAVE_TUNEPIMP
	QAction* m_fileImportMusicBrainz;
#endif
	QAction* m_fileExport;
	QAction* m_toolsApplyFilenameFormat;
	QAction* m_toolsApplyId3Format;
	QAction* m_toolsRenameDirectory;
	QAction* m_toolsNumberTracks;
#ifdef HAVE_TAGLIB
	QAction* m_toolsConvertToId3v24;
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	QAction* m_toolsConvertToId3v23;
#endif
	QAction* m_settingsShowHideV1;
	QAction* m_settingsShowHideV2;
	QAction* m_settingsConfigure;
	QAction* m_helpHandbook;
	QAction* m_helpAbout;
	QAction* m_helpAboutQt;
	QMenuBar* m_menubar;
#if QT_VERSION >= 0x040000
	QMenu* m_fileMenu;
	QMenu* m_toolsMenu;
	QMenu* m_settingsMenu;
	QMenu* m_helpMenu;
#else
	QPopupMenu* m_fileMenu;
	QPopupMenu* m_toolsMenu;
	QPopupMenu* m_settingsMenu;
	QPopupMenu* m_helpMenu;
#endif

	static BrowserDialog* s_helpBrowser;
#endif
};

#endif /* KID3_H */
