/**
 * \file kid3.h
 * Kid3 application for KDE
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#ifndef KID3_H
#define KID3_H

#include "autoconf.h"
#include "filelist.h"
#include "standardtags.h"
#include "framelist.h"

// forward declaration of the Kid3 classes
#include <kmainwindow.h>
class KAction;
class KRecentFilesAction;
class KToggleAction;
class KURL;
class id3Form;
class Mp3File;

/** Kid3 application */
class Kid3App : public KMainWindow
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param name name
	 */
	Kid3App(QWidget* parent=0, const char* name=0);
	/**
	 * Destructor.
	 */
	~Kid3App();
	/**
	 * Open directory.
	 *
	 * @param dir directory or file path
	 */
	void openDirectory(QString dir);
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
	 * Remove ID3v1 tags in selected files.
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
	 * Update modification state before closing.
	 * Called on closeEvent() of window.
	 * If anything was modified, save after asking user.
	 *
	 * @return FALSE if user canceled.
	 */
	virtual bool queryClose();
	/**
	 * Save options before closing.
	 * queryExit() is called when the last window of the application is
	 * going to be closed during the closeEvent().
	 *
	 * @return TRUE.
	 */
	virtual bool queryExit();

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
	void updateTags(Mp3File *mp3file);
	/**
	 * Update modification state, caption and listbox entries.
	 */
	void updateModificationState(void);
	/**
	 * Update ID3v2 tags in GUI controls from file displayed in frame list.
	 */
	void updateAfterFrameModification(void);

	/** GUI with controls, created by Qt Designer */
	id3Form *view;

	/** TRUE if any file was modified */
	bool modified;
	/** Current directory */
	QString doc_dir;
	/** Frame list */
	FrameList framelist;
	/** Copy buffer */
	StandardTags copytags;


	/** the configuration object of the application */
	KConfig *config;
	/** Actions */
	KAction* fileOpen;
	KRecentFilesAction* fileOpenRecent;
	KAction* fileRevert;
	KAction* fileSave;
	KAction* fileQuit;
	KToggleAction* viewToolBar;
	KToggleAction* viewStatusBar;
};
#endif
