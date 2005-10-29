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
typedef KMainWindow Kid3AppBaseClass;
#else
#include <qmainwindow.h>
#include "generalconfig.h" // Kid3Settings
class QAction;
typedef QMainWindow Kid3AppBaseClass;
#endif
class KURL;
class id3Form;
class TaggedFile;
class FormatConfig;
class ImportConfig;
class MiscConfig;
class FreedbConfig;
class StandardTags;
class FrameList;

#ifdef HAVE_TUNEPIMP
class MusicBrainzConfig;
#endif

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
	 * Update GUI controls from the tags in the files.
	 * The new selection is stored and the GUI controls and frame list
	 * updated accordingly (filtered for multiple selection).
	 */
	void updateGuiControls();

	/**
	 * Process change of selection.
	 * The files of the current selection are updated.
	 * The new selection is stored and the GUI controls and frame list
	 * updated accordingly (filtered for multiple selection).
	 */
	void fileSelected(void);

	/**
	 * Update files of current selection.
	 */
	void updateCurrentSelection(void);

	/**
	 * Copy a set of standard tags into copy buffer.
	 *
	 * @param st tags to copy
	 */
	void copyTags(const StandardTags *st);

	/**
	 * Paste from copy buffer to standard tags.
	 *
	 * @param st tags to fill from data in copy buffer.
	 */
	void pasteTags(StandardTags *st);

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
	void getTagsFromFilenameV1(void);

	/**
	 * Set ID3v2 tags according to filename.
	 * If a single file is selected the tags in the GUI controls
	 * are set, else the tags in the multiple selected files.
	 */
	void getTagsFromFilenameV2(void);

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
	void copyV1ToV2(void);

	/**
	 * Copy ID3v2 tags to ID3v1 tags of selected files.
	 */
	void copyV2ToV1(void);

	/**
	 * Remove ID3v1 tags in selected files.
	 */
	void removeTagsV1(void);

	/**
	 * Remove ID3v2 tags in selected files.
	 */
	void removeTagsV2(void);

	/**
	 * Open directory on drop.
	 *
	 * @param txt URL of directory or file in directory
	 */
	void openDrop(QString txt);

	/**
	 * Edit selected frame.
	 */
	void editFrame(void);

	/**
	 * Delete selected frame.
	 */
	void deleteFrame(void);

	/**
	 * Select a frame type and add such a frame to frame list.
	 */
	void addFrame(void);

	/** Filename format configuration */
	FormatConfig *fnFormatCfg;
	/** ID3 format configuration */
	FormatConfig *id3FormatCfg;
	/** Import configuration */
	ImportConfig *genCfg;
	/** Miscellaneous configuration */
	MiscConfig *miscCfg;
	/** Freedb configuration */
	FreedbConfig *freedbCfg;
#ifdef HAVE_TUNEPIMP
	/** MusicBrainz configuration */
	MusicBrainzConfig *m_musicBrainzCfg;
#endif

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
	 * @param _cfg application configuration
	 */
	virtual void saveProperties(KConfig *_cfg);

	/**
	 * Reads the session config file and restores the application's state.
	 *
	 * @param _cfg application configuration
	 */
	virtual void readProperties(KConfig *_cfg);

#else
	/**
	 * Window is closed.
	 *
	 * @param ce close event
	 */
	void closeEvent(QCloseEvent *ce);
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
	 * @param val TRUE if a file is modified
	 */
	void setModified(bool val) { modified = val; }

	/**
	 * Check modification state.
	 *
	 * @return TRUE if a file is modified.
	 */
	bool isModified(void) { return modified; }

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
	 * Select all files.
	 */
	void slotSelectAll();

	/**
	 * Select next file.
	 */
	void slotNextFile();

	/**
	 * Select previous file.
	 */
	void slotPreviousFile();

	/**
	 * Display handbook.
	 */
	void slotHelpHandbook(void);

	/**
	 * Display "About" dialog.
	 */
	void slotHelpAbout(void);

	/**
	 * Display "About Qt" dialog.
	 */
	void slotHelpAboutQt(void);

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
	void slotStatusMsg(const QString &text);

	/**
	 * Create playlist.
	 */
	void slotCreatePlaylist(void);

	/**
	 * Import.
	 */
	void slotImport(void);

	/**
	 * Preferences.
	 */
	void slotSettingsConfigure(void);

	/**
	 * Apply format.
	 */
	void slotApplyFormat(void);

	/**
	 * Rename directory.
	 */
	void slotRenameDirectory(void);

private:
	/**
	 * Save all changed files.
	 *
	 * @return true
	 */
	bool saveDirectory(void);

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
	void updateTags(TaggedFile *mp3file);

	/**
	 * Update modification state, caption and listbox entries.
	 */
	void updateModificationState(void);

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
	
	/** GUI with controls, created by Qt Designer */
	id3Form *view;
	/** TRUE if any file was modified */
	bool modified;
	/** Current directory */
	QString doc_dir;
	/** Copy buffer */
	StandardTags *copytags;

#ifdef CONFIG_USE_KDE
	/** the configuration object of the application */
	KConfig* config;
	/** Actions */
	KAction* fileOpen;
	KRecentFilesAction* fileOpenRecent;
	KAction* fileRevert;
	KAction* fileSave;
	KAction* fileQuit;
	KToggleAction* viewToolBar;
	KToggleAction* viewStatusBar;
	KAction* settingsShortcuts;
	KAction* settingsConfigure;
#else
	Kid3Settings* config;
	QAction* fileOpen;
	QAction* fileSave;
	QAction* fileRevert;
	QAction* fileCreatePlaylist;
	QAction* fileQuit;
	QAction* fileImport;
	QAction* toolsApplyFormat;
	QAction* toolsRenameDirectory;
	QAction* settingsConfigure;
	QAction* helpHandbook;
	QAction* helpAbout;
	QAction* helpAboutQt;
	QMenuBar* menubar;
	QPopupMenu* fileMenu;
	QPopupMenu* toolsMenu;
	QPopupMenu* settingsMenu;
	QPopupMenu* helpMenu;
#endif
};

#endif /* KID3_H */
