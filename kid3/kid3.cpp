/**
 * \file kid3.cpp
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "autoconf.h"
// include files for QT
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qurl.h>
#include <qtextstream.h>

#ifdef CONFIG_USE_KDE
// include files for KDE
#include <kapp.h>
#include <kurl.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#else
#include <qapplication.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

// application specific includes
#include "kid3.h"
#include "id3form.h"
#include "genres.h"
#include "framelist.h"

/**
 * Constructor.
 *
 * @param name name
 */

#ifdef CONFIG_USE_KDE
Kid3App::Kid3App(QWidget* , const char* name):KMainWindow(0, name)
#else
Kid3App::Kid3App(QWidget* , const char* name):QMainWindow(0, name)
#endif
{
	initStatusBar();
	initActions();
	setModified(false);
	doc_dir = QString::null;
	initView();
	filelist.setListBox(view->mp3ListBox);
	framelist.setListBox(view->framesListBox);

#ifdef CONFIG_USE_KDE
	config=kapp->config();
	readOptions();
#endif
}

/**
 * Destructor.
 */

Kid3App::~Kid3App()
{
	delete view;
	view = 0;
}

/**
 * Init menu and toolbar actions.
 */

void Kid3App::initActions()
{
#ifdef CONFIG_USE_KDE
	fileOpen = KStdAction::open(
	    this, SLOT(slotFileOpen()), actionCollection());
	fileOpenRecent = KStdAction::openRecent(
	    this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
	fileRevert = KStdAction::revert(
	    this, SLOT(slotFileRevert()), actionCollection());
	fileSave = KStdAction::save(
	    this, SLOT(slotFileSave()), actionCollection());
	fileQuit = KStdAction::quit(
	    this, SLOT(slotFileQuit()), actionCollection());
	viewToolBar = KStdAction::showToolbar(
	    this, SLOT(slotViewToolBar()), actionCollection());
	viewStatusBar = KStdAction::showStatusbar(
	    this, SLOT(slotViewStatusBar()), actionCollection());

	fileOpen->setStatusText(i18n("Opens a directory"));
	fileOpenRecent->setStatusText(i18n("Opens a recently used directory"));
	fileRevert->setStatusText(
	    i18n("Reverts the changes of all or the selected files"));
	fileSave->setStatusText(i18n("Saves the changed files"));
	fileQuit->setStatusText(i18n("Quits the application"));
	viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

	new KAction(i18n("&Create Playlist"), 0, this,
		    SLOT(slotCreatePlaylist()), actionCollection(),
		    "create_playlist");

	createGUI();

#else
	fileOpen = new QAction(this);
	if (fileOpen) {
		fileOpen->setText(i18n("Opens a directory"));
		fileOpen->setMenuText(i18n("&Open..."));
		fileOpen->setAccel(CTRL + Key_O);
		connect(fileOpen, SIGNAL(activated()),
			this, SLOT(slotFileOpen()));
	}
	fileSave = new QAction(this);
	if (fileSave) {
		fileSave->setText(i18n("Saves the changed files"));
		fileSave->setMenuText(i18n("&Save"));
		fileSave->setAccel(CTRL + Key_S);
		connect(fileSave, SIGNAL(activated()),
			this, SLOT(slotFileSave()));
	}
	fileRevert = new QAction(this);
	if (fileRevert) {
		fileRevert->setText(
		    i18n("Reverts the changes of all or the selected files"));
		fileRevert->setMenuText(i18n("Re&vert"));
		connect(fileRevert, SIGNAL(activated()),
			this, SLOT(slotFileRevert()));
	}
	fileCreatePlaylist = new QAction(this);
	if (fileCreatePlaylist) {
		fileCreatePlaylist->setText(i18n("Create M3U Playlist"));
		fileCreatePlaylist->setMenuText(i18n("&Create Playlist"));
		connect(fileCreatePlaylist, SIGNAL(activated()),
			this, SLOT(slotCreatePlaylist()));
	}
	fileQuit = new QAction(this);
	if (fileQuit) {
		fileQuit->setText(i18n("Quits the application"));
		fileQuit->setMenuText(i18n("&Quit"));
		fileQuit->setAccel(CTRL + Key_Q);
		connect(fileQuit, SIGNAL(activated()),
			this, SLOT(slotFileQuit()));
	}
	helpAbout = new QAction(this);
	if (helpAbout) {
		helpAbout->setText(i18n("About Kid3"));
		helpAbout->setMenuText(i18n("&About Kid3"));
		connect(helpAbout, SIGNAL(activated()),
			this, SLOT(slotHelpAbout()));
	}
	helpAboutQt = new QAction(this);
	if (helpAboutQt) {
		helpAboutQt->setText(i18n("About Qt"));
		helpAboutQt->setMenuText(i18n("About &Qt"));
		connect(helpAboutQt, SIGNAL(activated()),
			this, SLOT(slotHelpAboutQt()));
	}
	menubar = new QMenuBar(this);
	fileMenu = new QPopupMenu(this); 
	helpMenu = new QPopupMenu(this); 
	if (menubar && fileMenu && helpMenu) {
		fileOpen->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileSave->addTo(fileMenu);
		fileRevert->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileCreatePlaylist->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileQuit->addTo(fileMenu);
		menubar->insertItem((i18n("&File")), fileMenu);

		helpAbout->addTo(helpMenu);
		helpAboutQt->addTo(helpMenu);
		menubar->insertItem(i18n("&Help"), helpMenu);
	}
	setCaption("Kid3");
#endif
}

/**
 * Init status bar.
 */

void Kid3App::initStatusBar()
{
	statusBar()->message(i18n("Ready."));
}

/**
 * Init GUI.
 */

void Kid3App::initView()
{ 
	view = new id3Form(this);
	if (view) {
		setCentralWidget(view);	
		view->genreV1ComboBox->insertStrList(Genres::strList);
		view->genreV2ComboBox->insertStrList(Genres::strList);
		view->formatComboBox->setEditable(TRUE);
		view->formatComboBox->insertStrList(Mp3File::fnFmtList);
	}
}

/**
 * Open directory.
 *
 * @param dir directory or file path
 */

void Kid3App::openDirectory(QString dir)
{
	if (dir.isNull() || dir.isEmpty()) {
		return;
	}
	QFileInfo file(dir);
	if (!file.isDir()) {
		dir = file.dirPath(TRUE);
	}

	slotStatusMsg(i18n("Opening directory..."));
	if (filelist.readDir(dir)) {
		setModified(false);
#ifdef CONFIG_USE_KDE
		KURL url;
		url.setPath(dir);
		fileOpenRecent->addURL(url);
		setCaption(dir, false);
#else
		setCaption(dir + " - Kid3");
#endif
		doc_dir = dir;
	}
	slotStatusMsg(i18n("Ready."));
}

#ifdef CONFIG_USE_KDE
/**
 * Save application options.
 */

void Kid3App::saveOptions()
{	
	fileOpenRecent->saveEntries(config, "Recent Files");
	config->setGroup("General Options");
	config->writeEntry("NameFilter", filelist.getNameFilter());
	config->writeEntry("FormatItem", view->formatComboBox->currentItem());
	config->writeEntry("FormatText", view->formatComboBox->currentText());
}

/**
 * Load application options.
 */

void Kid3App::readOptions()
{
	setAutoSaveSettings();
	fileOpenRecent->loadEntries(config,"Recent Files");
	viewToolBar->setChecked(!toolBar("mainToolBar")->isHidden());
	viewStatusBar->setChecked(!statusBar()->isHidden());
	config->setGroup("General Options");
	filelist.setNameFilter(
	    config->readEntry("NameFilter", FileList::defaultNameFilter));
	view->formatComboBox->setCurrentItem(
	    config->readNumEntry("FormatItem", 0));
#if QT_VERSION >= 300
	view->formatComboBox->setCurrentText(
	    config->readEntry("FormatText", Mp3File::fnFmtList[0]));
#endif
}

/**
 * Saves the window properties to the session config file.
 *
 * @param _cfg application configuration
 */

void Kid3App::saveProperties(KConfig *_cfg)
{
	_cfg->writeEntry("dirname", doc_dir);
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param _cfg application configuration
 */

void Kid3App::readProperties(KConfig* _cfg)
{
	openDirectory(_cfg->readEntry("dirname", ""));
}
#else
/**
 * Window is closed.
 *
 * @param ce close event
 */

void Kid3App::closeEvent(QCloseEvent *ce)
{
	if (queryClose()) {
		ce->accept();
	}
	else {
		ce->ignore();
	}
}
#endif

/**
 * Save all changed files.
 *
 * @return true
 */

bool Kid3App::saveDirectory(void)
{
	bool renamed = FALSE;
	Mp3File *mp3file = filelist.first();
	while (mp3file != 0) {
		renamed = mp3file->writeTags(FALSE) || renamed;
		mp3file = filelist.next();
	}
	if (renamed) {
		filelist.readDir(doc_dir);
		setModified(false);
	}
	else {
		updateModificationState();
	}
	return true;
}

/**
 * If anything was modified, save after asking user.
 *
 * @return FALSE if user canceled.
 */

bool Kid3App::saveModified()
{
	bool completed=true;

	if(isModified() && !doc_dir.isEmpty())
	{
		Kid3App *win=(Kid3App *) parent();
#ifdef CONFIG_USE_KDE
		const int Yes = KMessageBox::Yes;
		const int No = KMessageBox::No;
		const int Cancel = KMessageBox::Cancel;
		int want_save = KMessageBox::warningYesNoCancel(
		    win,
		    i18n("The current directory has been modified.\n"
			 "Do you want to save it?"),
		    i18n("Warning"));
#else
		const int Yes = QMessageBox::Yes;
		const int No = QMessageBox::No;
		const int Cancel = QMessageBox::Cancel;
		int want_save = QMessageBox::warning(
			win,
			i18n("Warning - Kid3"),
			i18n("The current directory has been modified.\n"
			     "Do you want to save it?"),
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape);
#endif
		switch(want_save)
		{
		case Yes:
			saveDirectory();
			completed=true;
			break;

		case No:
			setModified(false);
			completed=true;
			break;

		case Cancel:
			completed=false;
			break;

		default:
			completed=false;
			break;
		}
	}

	return completed;
}

/**
 * Update modification state before closing.
 * Called on closeEvent() of window.
 * If anything was modified, save after asking user.
 *
 * @return FALSE if user canceled.
 */

bool Kid3App::queryClose()
{
	updateCurrentSelection();
	return saveModified();
}

/**
 * Save options before closing.
 * queryExit() is called when the last window of the application is
 * going to be closed during the closeEvent().
 *
 * @return TRUE.
 */

bool Kid3App::queryExit()
{
#ifdef CONFIG_USE_KDE
	saveOptions();
#endif
	return true;
}

/**
 * Request new directory and open it.
 */

void Kid3App::slotFileOpen()
{
	updateCurrentSelection();
	if(saveModified()) {
		QString dir, filter;
#ifdef CONFIG_USE_KDE
		KFileDialog diag(
		    QString::null,
		    i18n("*.mp3 *.MP3|MP3 (*.mp3 *.MP3)\n*|All Files (*)"),
		    this, "filedialog", TRUE);
		diag.setCaption(i18n("Open"));
		(void)diag.exec();
		dir = diag.selectedFile();
		filter = diag.currentFilter();
#else
#if QT_VERSION < 300
		filter = "MP3 (*.mp3 *.MP3);;All Files (*)";
		dir = QFileDialog::getOpenFileName(
		    QString::null, i18n(filter), this, 0);
#else
		dir = QFileDialog::getOpenFileName(
		    QString::null, i18n("MP3 (*.mp3 *.MP3);;All Files (*)"),
		    this, 0, QString::null, &filter);
#endif
#endif
		int start = filter.find('('), end = filter.find(')');
		if (start != -1 && end != -1 && end > start) {
			filter = filter.mid(start + 1, end - start - 1);
		}
		filelist.setNameFilter(filter);
		openDirectory(dir);
	}
}

#ifdef CONFIG_USE_KDE
/**
 * Open recent directory.
 *
 * @param url URL of directory to open
 */

void Kid3App::slotFileOpenRecent(const KURL& url)
{
	updateCurrentSelection();
	if(saveModified()) {
		QString dir = url.path();
		openDirectory(dir);
	}
}
#endif

/**
 * Revert file modifications.
 * Acts on selected files or all files if no file is selected.
 */

void Kid3App::slotFileRevert()
{
	Mp3File *mp3file = filelist.first();
	bool no_selection = view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->readTags(TRUE);
		}
		mp3file = filelist.next();
	}
	if (!no_selection) {
		StandardTags st; // empty
		view->setStandardTagsV1(&st);
		view->setStandardTagsV2(&st);
		view->nameLineEdit->setEnabled(FALSE);
		fileSelected();
	}
	else {
		updateModificationState();
	}
}

/**
 * Save modified files.
 */

void Kid3App::slotFileSave()
{
	updateCurrentSelection();
	slotStatusMsg(i18n("Saving directory..."));
	
	saveDirectory();
	slotStatusMsg(i18n("Ready."));
}

/**
 * Quit application.
 */

void Kid3App::slotFileQuit()
{
	updateCurrentSelection();
	slotStatusMsg(i18n("Exiting..."));
#ifdef CONFIG_USE_KDE
	saveOptions();
#endif
	close();
}

#ifdef CONFIG_USE_KDE
/**
 * Turn tool bar on or off.
 */

void Kid3App::slotViewToolBar()
{
	slotStatusMsg(i18n("Toggling toolbar..."));
	if(!viewToolBar->isChecked()) {
		toolBar("mainToolBar")->hide();
	}
	else {
		toolBar("mainToolBar")->show();
	}		
	slotStatusMsg(i18n("Ready."));
}

/**
 * Turn status bar on or off.
 */

void Kid3App::slotViewStatusBar()
{
	slotStatusMsg(i18n("Toggle the statusbar..."));
	if(!viewStatusBar->isChecked()) {
		statusBar()->hide();
	}
	else {
		statusBar()->show();
	}
	slotStatusMsg(i18n("Ready."));
}

#else /* CONFIG_USE_KDE */
/**
 * Display "About" dialog.
 */

void Kid3App::slotHelpAbout()
{
	QMessageBox::about(
		(Kid3App*)parent(), "Kid3",
		"Kid3 " CONFIG_VERSION
		"\n(c) 2003, Urs Fleisch\nufleisch@users.sourceforge.net");
}

/**
 * Display "About Qt" dialog.
 */

void Kid3App::slotHelpAboutQt()
{
	QMessageBox::aboutQt((Kid3App*)parent(), "Kid3");
}
#endif /* CONFIG_USE_KDE */

/**
 * Change status message.
 *
 * @param text message
 */

void Kid3App::slotStatusMsg(const QString &text)
{
	statusBar()->message(text);
	// processEvents() is necessary to make the change of the status bar
	// visible when it is changed back again in the same function,
	// i.e. in the same call from the Qt main event loop.
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
}

/**
 * Create playlist.
 */

void Kid3App::slotCreatePlaylist(void)
{
	QDir dir(filelist.getAbsDirname());
	QString fn = filelist.getAbsDirname() + QDir::separator() + dir.dirName() + ".m3u";
	QFile file(fn);
	slotStatusMsg(i18n("Creating playlist..."));
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		Mp3File *mp3file = filelist.first();
		while (mp3file != 0) {
			stream << mp3file->getFilename() << "\n";
			mp3file = filelist.next();
		}
		file.close();
	}
	slotStatusMsg(i18n("Ready."));
}

/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */

void Kid3App::openDrop(QString txt)
{
	QUrl url(txt);
	if (url.hasPath()) {
		QString dir = url.path().stripWhiteSpace();
#if defined _WIN32 || defined WIN32
		// There seems to be problems with filenames on Win32,
		// so correct
		if (dir[0] == '/' && dir[1] == '/' && dir[3] == '|') {
			dir[3] = ':';
			dir.remove(0, 2);
		}
#endif
		updateCurrentSelection();
		if(saveModified()) {
			openDirectory(dir);
		}
	}
}

/**
 * Set tags in file to tags in GUI controls.
 *
 * @param mp3file file
 */

void Kid3App::updateTags(Mp3File *mp3file)
{
	StandardTags st;
	view->getStandardTagsV1(&st);
	mp3file->setStandardTagsV1(&st);
	view->getStandardTagsV2(&st);
	mp3file->setStandardTagsV2(&st);
	if (view->nameLineEdit->isEnabled()) {
		mp3file->setFilename(view->nameLineEdit->text());
	}
}

/**
 * Update modification state, caption and listbox entries.
 */

void Kid3App::updateModificationState(void)
{
	setModified(filelist.updateModificationState());
#ifdef CONFIG_USE_KDE
	setCaption(doc_dir, isModified());
#else
	QString cap(doc_dir);
	if (isModified()) {
		cap += i18n(" [modified]");
	}
	if (!cap.isEmpty()) {
		cap += " - ";
	}
	cap += "Kid3";
	setCaption(cap);
#endif
}

/**
 * Update files of current selection.
 */

void Kid3App::updateCurrentSelection(void)
{
	Mp3File *mp3file = filelist.first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			updateTags(mp3file);
		}
		mp3file = filelist.next();
	}
	updateModificationState();
}

/**
 * Process change of selection.
 * The files of the current selection are updated.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */

void Kid3App::fileSelected(void)
{
	StandardTags tags_v1, tags_v2;
	Mp3File *mp3file = filelist.first(), *single_v2_file = NULL;
	int num_files_selected = 0;

	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			// file was selected -> update
			updateTags(mp3file);
		}
		if (
#if QT_VERSION < 300
			mp3file->QListBoxItem::selected()
#else
			mp3file->isSelected()
#endif
			) {
			StandardTags filetags;
			mp3file->setInSelection(TRUE);
			mp3file->readTags(FALSE);
			mp3file->getStandardTagsV1(&filetags);
			if (num_files_selected == 0) {
				tags_v1 = filetags;
			}
			else {
				tags_v1.filterDifferent(filetags);
			}
			mp3file->getStandardTagsV2(&filetags);
			if (num_files_selected == 0) {
				tags_v2 = filetags;
				single_v2_file = mp3file;
			}
			else {
				tags_v2.filterDifferent(filetags);
				single_v2_file = NULL;
			}
			++num_files_selected;
		}
		else {
			mp3file->setInSelection(FALSE);
		}
		mp3file = filelist.next();
	}
	view->setStandardTagsV1(&tags_v1);
	view->setStandardTagsV2(&tags_v2);
	view->setAllCheckBoxes(num_files_selected == 1);
	updateModificationState();
	if (single_v2_file) {
		framelist.setTags(single_v2_file);
		view->nameLineEdit->setEnabled(TRUE);
		view->nameLineEdit->setText(single_v2_file->getFilename());
	}
	else {
		framelist.clear();
		view->nameLineEdit->setEnabled(FALSE);
	}
}

/**
 * Copy a set of standard tags into copy buffer.
 *
 * @param st tags to copy
 */

void Kid3App::copyTags(const StandardTags *st)
{
	copytags = *st;
}

/**
 * Paste from copy buffer to standard tags.
 *
 * @param st tags to fill from data in copy buffer.
 */

void Kid3App::pasteTags(StandardTags *st)
{
	if (!copytags.title.isNull())
		st->title = copytags.title;
	if (!copytags.artist.isNull())
		st->artist = copytags.artist;
	if (!copytags.album.isNull())
		st->album = copytags.album;
	if (!copytags.album.isNull())
		st->album = copytags.album;
	if (copytags.year >= 0)
		st->year = copytags.year;
	if (copytags.track >= 0)
		st->track = copytags.track;
	if (copytags.genre >= 0)
		st->genre = copytags.genre;
}

/**
 * Set ID3v1 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */

void Kid3App::getTagsFromFilenameV1(void)
{
	StandardTags st;
	Mp3File *mp3file = filelist.first();
	bool multiselect = view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (multiselect) {
				mp3file->getStandardTagsV1(&st);
				mp3file->getTagsFromFilename(&st);
				mp3file->setStandardTagsV1(&st);
			}
			else {
				if (view->nameLineEdit->isEnabled()) {
					mp3file->setFilename(
					    view->nameLineEdit->text());
				}
				view->getStandardTagsV1(&st);
				mp3file->getTagsFromFilename(&st);
				view->setStandardTagsV1(&st);
			}
		}
		mp3file = filelist.next();
	}
}

/**
 * Set ID3v2 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */

void Kid3App::getTagsFromFilenameV2(void)
{
	StandardTags st;
	Mp3File *mp3file = filelist.first();
	bool multiselect = view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (multiselect) {
				mp3file->getStandardTagsV2(&st);
				mp3file->getTagsFromFilename(&st);
				mp3file->setStandardTagsV2(&st);
			}
			else {
				if (view->nameLineEdit->isEnabled()) {
					mp3file->setFilename(
					    view->nameLineEdit->text());
				}
				view->getStandardTagsV2(&st);
				mp3file->getTagsFromFilename(&st);
				view->setStandardTagsV2(&st);
			}
		}
		mp3file = filelist.next();
	}
}

/**
 * Set filename according to tags.
 * If a single file is selected the tags in the GUI controls
 * are used, else the tags in the multiple selected files.
 *
 * @param tag_version 1=ID3v1, 2=ID3v2
 */

void Kid3App::getFilenameFromTags(int tag_version)
{
	StandardTags st;
	Mp3File *mp3file = filelist.first();
	bool multiselect = view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (multiselect) {
				if (tag_version == 2) {
					mp3file->getStandardTagsV2(&st);
				}
				else {
					mp3file->getStandardTagsV1(&st);
				}
				mp3file->getFilenameFromTags(
				    &st, view->formatComboBox->currentText());
			}
			else {
				if (tag_version == 2) {
					view->getStandardTagsV2(&st);
				}
				else {
					view->getStandardTagsV1(&st);
				}
				mp3file->getFilenameFromTags(
				    &st, view->formatComboBox->currentText());
				view->nameLineEdit->setText(
				    mp3file->getFilename());
			}
		}
		mp3file = filelist.next();
	}
}

/**
 * Copy ID3v1 tags to ID3v2 tags of selected files.
 */

void Kid3App::copyV1ToV2(void)
{
	StandardTags st;
	if (view->numFilesSelected() > 1) {
		Mp3File *mp3file = filelist.first();
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				mp3file->getStandardTagsV1(&st);
				mp3file->setStandardTagsV2(&st);
			}
			mp3file = filelist.next();
		}
	}
	else {
		view->getStandardTagsV1(&st);
		view->setStandardTagsV2(&st);
	}
}

/**
 * Copy ID3v2 tags to ID3v1 tags of selected files.
 */

void Kid3App::copyV2ToV1(void)
{
	StandardTags st;
	if (view->numFilesSelected() > 1) {
		Mp3File *mp3file = filelist.first();
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				mp3file->getStandardTagsV2(&st);
				mp3file->setStandardTagsV1(&st);
			}
			mp3file = filelist.next();
		}
	}
	else {
		view->getStandardTagsV2(&st);
		view->setStandardTagsV1(&st);
	}
}

/**
 * Remove ID3v1 tags in selected files.
 */

void Kid3App::removeTagsV1(void)
{
	Mp3File *mp3file = filelist.first();
	int i = 0;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->removeTagsV1();
		}
		mp3file = filelist.next();
		++i;
	}
}

/**
 * Remove ID3v1 tags in selected files.
 */

void Kid3App::removeTagsV2(void)
{
	Mp3File *mp3file = filelist.first();
	int i = 0;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->removeTagsV2();
		}
		mp3file = filelist.next();
		++i;
	}
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 */

void Kid3App::updateAfterFrameModification(void)
{
	Mp3File *mp3file = framelist.getFile();
	if (mp3file) {
		StandardTags st;
		mp3file->getStandardTagsV2(&st);
		view->setStandardTagsV2(&st);
		updateModificationState();
	}
}

/**
 * Edit selected frame.
 */

void Kid3App::editFrame(void)
{
	if (framelist.editFrame()) {
		updateAfterFrameModification();
	}
}

/**
 * Delete selected frame.
 */

void Kid3App::deleteFrame(void)
{
	if (framelist.deleteFrame()) {
		updateAfterFrameModification();
	}
}

/**
 * Select a frame type and add such a frame to frame list.
 */

void Kid3App::addFrame(void)
{
	ID3_FrameID id = FrameList::selectFrameId();
	if (id != ID3FID_NOFRAME &&
		framelist.addFrame(id)) {
		updateAfterFrameModification();
	}
}
