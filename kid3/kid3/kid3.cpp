/**
 * \file kid3.cpp
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qurl.h>
#include <qtextstream.h>
#include <qcursor.h>
#include <qprogressbar.h>
#include <qmessagebox.h>

#ifdef CONFIG_USE_KDE
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
#include <kkeydialog.h>
#else
#include <qapplication.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#if QT_VERSION >= 300
#include <qsettings.h>
#endif
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include "kid3.h"
#include "id3form.h"
#include "genres.h"
#include "framelist.h"
#include "configdialog.h"
#include "importdialog.h"
#include "formatconfig.h"
#include "importconfig.h"
#include "miscconfig.h"
#include "freedbconfig.h"
#include "standardtags.h"
#include "rendirdialog.h"

#ifndef CONFIG_USE_KDE
#include <qdialog.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>

class BrowserDialog : public QDialog {
public:
	BrowserDialog(QWidget *parent, QString &caption);
	~BrowserDialog();
};

BrowserDialog::BrowserDialog(QWidget *parent, QString &caption)
	: QDialog(parent, "browser", true)
{
	setCaption(caption);
	QVBoxLayout *vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QTextBrowser *textBrowser = new QTextBrowser(this, "textBrowser");
	QString fn(QDir::currentDirPath() + QDir::separator() + "kid3_");
	QString lang((QString(QTextCodec::locale())).left(2));
	if (!QFile::exists(fn + lang + ".html")) {
		lang = "en";
	}
	fn += lang + ".html";
	textBrowser->setSource(fn);
	vlayout->addWidget(textBrowser);

	QHBoxLayout *hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton *backButton = new QPushButton(i18n("Back"), this);
	QPushButton *forwardButton = new QPushButton(i18n("Forward"), this);
	QPushButton *closeButton = new QPushButton(i18n("Close"), this);
	if (hlayout && backButton && forwardButton && closeButton) {
		hlayout->addWidget(backButton);
		hlayout->addWidget(forwardButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(closeButton);
		closeButton->setDefault(true);
		backButton->setEnabled(false);
		forwardButton->setEnabled(false);
		connect(backButton, SIGNAL(clicked()), textBrowser, SLOT(backward()));
		connect(forwardButton, SIGNAL(clicked()), textBrowser, SLOT(forward()));
		connect(textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
		connect(textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));
		connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
	}
	resize(500, 500);
}

BrowserDialog::~BrowserDialog()
{}
#endif

/**
 * Constructor.
 *
 * @param name name
 */

Kid3App::Kid3App(QWidget* , const char* name):
#ifdef CONFIG_USE_KDE
KMainWindow(0, name)
#else
QMainWindow(0, name)
#endif
{
	miscCfg = new MiscConfig("General Options");
	genCfg = new ImportConfig("General Options");
	fnFormatCfg = new FormatConfig("FilenameFormat");
	id3FormatCfg = new FormatConfig("Id3Format");
	freedbCfg = new FreedbConfig("Freedb");
	framelist = new FrameList();
	copytags = new StandardTags();
	initStatusBar();
	setModified(false);
	doc_dir = QString::null;
	initView();
	initActions();
	framelist->setListBox(view->framesListBox);
	fnFormatCfg->setAsFilenameFormatter();

	resize(sizeHint());
#ifdef CONFIG_USE_KDE
	config=kapp->config();
#else
	config = new QSettings();
    config->setPath("kid3.sourceforge.net", "Kid3", QSettings::User);
	config->beginGroup("/kid3");
#endif
	readOptions();
}

/**
 * Destructor.
 */

Kid3App::~Kid3App()
{
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
    settingsShortcuts = KStdAction::keyBindings(
		this, SLOT(slotSettingsShortcuts()), actionCollection());
	settingsConfigure = KStdAction::preferences(
	    this, SLOT(slotSettingsConfigure()), actionCollection());

	fileOpen->setStatusText(i18n("Opens a directory"));
	fileOpenRecent->setStatusText(i18n("Opens a recently used directory"));
	fileRevert->setStatusText(
	    i18n("Reverts the changes of all or the selected files"));
	fileSave->setStatusText(i18n("Saves the changed files"));
	fileQuit->setStatusText(i18n("Quits the application"));
	viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
	settingsShortcuts->setStatusText(i18n("Configure Shortcuts"));
	settingsConfigure->setStatusText(i18n("Preferences dialog"));

	new KAction(i18n("&Import..."), 0, this,
		    SLOT(slotImport()), actionCollection(),
		    "import");
	new KAction(i18n("&Create Playlist"), 0, this,
		    SLOT(slotCreatePlaylist()), actionCollection(),
		    "create_playlist");
	new KAction(i18n("&Apply Format"), 0, this,
		    SLOT(slotApplyFormat()), actionCollection(),
		    "apply_format");
	new KAction(i18n("&Rename Directory..."), 0, this,
		    SLOT(slotRenameDirectory()), actionCollection(),
		    "rename_directory");

	new KAction(i18n("Select &All"), KShortcut("Alt+A"), this,
		    SLOT(slotSelectAll()), actionCollection(),
		    "select_all");
	new KAction(i18n("&Next File"), KShortcut("Alt+Down"), this,
		    SLOT(slotNextFile()), actionCollection(),
		    "next_file");
	new KAction(i18n("&Previous File"), KShortcut("Alt+Up"), this,
		    SLOT(slotPreviousFile()), actionCollection(),
		    "previous_file");
	new KAction("ID3v1: " + i18n("From Filename"), 0, view, SLOT(fromFilenameV1()),
				actionCollection(), "v1_from_filename");
	new KAction("ID3v1: " + i18n("From ID3v2"), 0, view, SLOT(fromID3V1()),
				actionCollection(), "v1_from_v2");
	new KAction("ID3v1: " + i18n("Copy"), 0, view, SLOT(copyV1()),
				actionCollection(), "v1_copy");
	new KAction("ID3v1: " + i18n("Paste"), 0, view, SLOT(pasteV1()),
				actionCollection(), "v1_paste");
	new KAction("ID3v1: " + i18n("Remove"), 0, view, SLOT(removeV1()),
				actionCollection(), "v1_remove");
	new KAction("ID3v2: " + i18n("From Filename"), 0, view, SLOT(fromFilenameV2()),
				actionCollection(), "v2_from_filename");
	new KAction("ID3v2: " + i18n("From ID3v1"), 0, view, SLOT(fromID3V2()),
				actionCollection(), "v2_from_v1");
	new KAction("ID3v2: " + i18n("Copy"), 0, view, SLOT(copyV2()),
				actionCollection(), "v2_copy");
	new KAction("ID3v2: " + i18n("Paste"), 0, view, SLOT(pasteV2()),
				actionCollection(), "v2_paste");
	new KAction("ID3v2: " + i18n("Remove"), 0, view, SLOT(removeV2()),
				actionCollection(), "v2_remove");
	new KAction(i18n("Frames:") + " " + i18n("Edit"), 0, view, SLOT(editFrame()),
				actionCollection(), "frames_edit");
	new KAction(i18n("Frames:") + " " + i18n("Add"), 0, view, SLOT(addFrame()),
				actionCollection(), "frames_add");
	new KAction(i18n("Frames:") + " " + i18n("Delete"), 0, view, SLOT(deleteFrame()),
				actionCollection(), "frames_delete");
	new KAction(i18n("Filename") + ": " + i18n("From ID3v1"), 0, view, SLOT(fnFromID3V1()),
				actionCollection(), "filename_from_v1");
	new KAction(i18n("Filename") + ": " + i18n("From ID3v2"), 0, view, SLOT(fnFromID3V2()),
				actionCollection(), "filename_from_v2");
	new KAction(i18n("Filename") + ": " + i18n("Focus"), 0, view->nameLineEdit, SLOT(setFocus()),
				actionCollection(), "filename_focus");
	new KAction("ID3v1: " + i18n("Focus"), 0, view->titleV1LineEdit, SLOT(setFocus()),
				actionCollection(), "v1_focus");
	new KAction("ID3v2: " + i18n("Focus"), 0, view->titleV2LineEdit, SLOT(setFocus()),
				actionCollection(), "v2_focus");

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
	fileImport = new QAction(this);
	if (fileImport) {
		fileImport->setText(i18n("Import from file or clipboard"));
		fileImport->setMenuText(i18n("&Import..."));
		connect(fileImport, SIGNAL(activated()),
			this, SLOT(slotImport()));
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
	helpHandbook = new QAction(this);
	if (helpHandbook) {
		helpHandbook->setText(i18n("Kid3 Handbook"));
		helpHandbook->setMenuText(i18n("Kid3 &Handbook"));
		connect(helpHandbook, SIGNAL(activated()),
			this, SLOT(slotHelpHandbook()));
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
	toolsApplyFormat = new QAction(this);
	if (toolsApplyFormat) {
		toolsApplyFormat->setText(i18n("Apply Format"));
		toolsApplyFormat->setMenuText(i18n("&Apply Format"));
		connect(toolsApplyFormat, SIGNAL(activated()),
			this, SLOT(slotApplyFormat()));
	}
	toolsRenameDirectory = new QAction(this);
	if (toolsRenameDirectory) {
		toolsRenameDirectory->setText(i18n("Rename Directory"));
		toolsRenameDirectory->setMenuText(i18n("&Rename Directory..."));
		connect(toolsRenameDirectory, SIGNAL(activated()),
			this, SLOT(slotRenameDirectory()));
	}
	settingsConfigure = new QAction(this);
	if (settingsConfigure) {
		settingsConfigure->setText(i18n("Configure Kid3"));
		settingsConfigure->setMenuText(i18n("&Configure Kid3..."));
		connect(settingsConfigure, SIGNAL(activated()),
			this, SLOT(slotSettingsConfigure()));
	}
	menubar = new QMenuBar(this);
	fileMenu = new QPopupMenu(this);
	toolsMenu = new QPopupMenu(this);
	settingsMenu = new QPopupMenu(this);
	helpMenu = new QPopupMenu(this);
	if (menubar && fileMenu && toolsMenu && settingsMenu && helpMenu) {
		fileOpen->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileSave->addTo(fileMenu);
		fileRevert->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileImport->addTo(fileMenu);
		fileCreatePlaylist->addTo(fileMenu);
		fileMenu->insertSeparator();
		fileQuit->addTo(fileMenu);
		menubar->insertItem((i18n("&File")), fileMenu);

		toolsApplyFormat->addTo(toolsMenu);
		toolsRenameDirectory->addTo(toolsMenu);
		menubar->insertItem((i18n("&Tools")), toolsMenu);

		settingsConfigure->addTo(settingsMenu);
		menubar->insertItem(i18n("&Settings"), settingsMenu);

		helpHandbook->addTo(helpMenu);
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
	view = new id3Form(this, "id3Form");
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

#if QT_VERSION >= 300
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#else
	QApplication::setOverrideCursor(QCursor(WaitCursor));
#endif
	slotStatusMsg(i18n("Opening directory..."));
	if (view->mp3ListBox->readDir(dir)) {
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
	QApplication::restoreOverrideCursor();
}

/**
 * Save application options.
 */

void Kid3App::saveOptions()
{
#ifdef CONFIG_USE_KDE
	fileOpenRecent->saveEntries(config, "Recent Files");
#else
	miscCfg->windowWidth = size().width();
	miscCfg->windowHeight = size().height();
#endif
	miscCfg->splitterSizes = view->sizes();
	miscCfg->nameFilter = view->mp3ListBox->getNameFilter();
	miscCfg->formatItem = view->formatComboBox->currentItem();
	miscCfg->formatText = view->formatComboBox->currentText();

	miscCfg->writeToConfig(config);
	fnFormatCfg->writeToConfig(config);
	id3FormatCfg->writeToConfig(config);
	genCfg->writeToConfig(config);
	freedbCfg->writeToConfig(config);
}

/**
 * Load application options.
 */

void Kid3App::readOptions()
{
	miscCfg->readFromConfig(config);
	fnFormatCfg->readFromConfig(config);
	id3FormatCfg->readFromConfig(config);
	genCfg->readFromConfig(config);
	freedbCfg->readFromConfig(config);
#ifdef CONFIG_USE_KDE
	setAutoSaveSettings();
	fileOpenRecent->loadEntries(config,"Recent Files");
	viewToolBar->setChecked(!toolBar("mainToolBar")->isHidden());
	viewStatusBar->setChecked(!statusBar()->isHidden());
#else
	if (miscCfg->windowWidth != -1 && miscCfg->windowHeight != -1) {
		resize(miscCfg->windowWidth, miscCfg->windowHeight);
	}
#endif
	if (
#if QT_VERSION >= 300
		!miscCfg->splitterSizes.empty()
#else
		miscCfg->splitterSizes.count() > 0
#endif
		) {
		view->setSizes(miscCfg->splitterSizes);
	}
	view->mp3ListBox->setNameFilter(miscCfg->nameFilter);
	view->formatComboBox->setCurrentItem(miscCfg->formatItem);
#if QT_VERSION >= 300
	view->formatComboBox->setCurrentText(miscCfg->formatText);
#endif
}

#ifdef CONFIG_USE_KDE
/**
 * Saves the window properties to the session config file.
 *
 * @param _cfg application configuration
 */

void Kid3App::saveProperties(KConfig *_cfg)
{
	if (_cfg) { // otherwise KDE 3.0 compiled program crashes with KDE 3.1
		_cfg->writeEntry("dirname", doc_dir);
	}
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
	QString errorFiles;
	int numFiles = 0, totalFiles = 0;
	bool renamed = FALSE;
	Mp3File *mp3file = view->mp3ListBox->first();
	// Get number of files to be saved to display correct progressbar
	while (mp3file != 0) {
		if (mp3file->isChanged()) {
			++totalFiles;
		}
		mp3file = view->mp3ListBox->next();
	}
	QProgressBar *progress = new QProgressBar();
	statusBar()->addWidget(progress, 0, true);
	progress->setTotalSteps(totalFiles);
	progress->setProgress(numFiles);
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
	mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (!mp3file->writeTags(FALSE, &renamed)) {
			errorFiles.append(mp3file->getFilename());
			errorFiles.append('\n');
		}
		mp3file = view->mp3ListBox->next();
		++numFiles;
		progress->setProgress(numFiles);
	}
	statusBar()->removeWidget(progress);
	delete progress;
	if (renamed) {
		view->mp3ListBox->readDir(doc_dir);
		setModified(false);
	}
	else {
		updateModificationState();
	}
	if (!errorFiles.isEmpty()) {
		QMessageBox::warning(0, i18n("File Error"),
							 i18n("Error while writing file:\n") +
							 errorFiles,
							 QMessageBox::Ok, QMessageBox::NoButton);
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
 * Free allocated resources.
 * Our destructor may not be called, so cleanup is done here.
 */
void Kid3App::cleanup()
{
#ifndef CONFIG_USE_KDE
		delete config;
#endif
		delete freedbCfg;
		delete genCfg;
		delete miscCfg;
		delete fnFormatCfg;
		delete id3FormatCfg;
		delete framelist;
		delete copytags;
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

bool Kid3App::queryClose()
{
	updateCurrentSelection();
	if (saveModified()) {
		saveOptions();
		cleanup();
		return true;
	}
	return false;
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
		view->mp3ListBox->setNameFilter(filter);
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
	Mp3File *mp3file = view->mp3ListBox->first();
	bool no_selection = view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->readTags(TRUE);
		}
		mp3file = view->mp3ListBox->next();
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
#if QT_VERSION >= 300
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#else
	QApplication::setOverrideCursor(QCursor(WaitCursor));
#endif
	slotStatusMsg(i18n("Saving directory..."));

	saveDirectory();
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Quit application.
 */

void Kid3App::slotFileQuit()
{
	slotStatusMsg(i18n("Exiting..."));
	close(); /* this will lead to call of closeEvent(), queryClose() */
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

/**
 * Shortcuts configuration.
 */
void Kid3App::slotSettingsShortcuts()
{
	KKeyDialog::configure(actionCollection(), this);
}

/**
 * Select all files.
 */
void Kid3App::slotSelectAll()
{
	view->mp3ListBox->selectAll(true);
}

/**
 * Select next file.
 */
void Kid3App::slotNextFile()
{
	int ci = view->mp3ListBox->currentItem();
	if (ci >= 0 && ci < (int)view->mp3ListBox->count() - 1) {
		++ci;
		view->mp3ListBox->clearSelection();
		view->mp3ListBox->setCurrentItem(ci);
		view->mp3ListBox->setSelected(ci, true);
	}
}

/**
 * Select previous file.
 */
void Kid3App::slotPreviousFile()
{
	int ci = view->mp3ListBox->currentItem();
	if (ci > 0) {
		--ci;
		view->mp3ListBox->clearSelection();
		view->mp3ListBox->setCurrentItem(ci);
		view->mp3ListBox->setSelected(ci, true);
	}
}

#else /* CONFIG_USE_KDE */
/**
 * Display handbook.
 */

void Kid3App::slotHelpHandbook()
{
	QString caption(i18n("Kid3 Handbook"));
	BrowserDialog *dialog =
		new BrowserDialog(NULL, caption);
	if (dialog) {
		(void)dialog->exec();
	}
}

/**
 * Display "About" dialog.
 */

void Kid3App::slotHelpAbout()
{
	QMessageBox::about(
		(Kid3App*)parent(), "Kid3",
		"Kid3 " VERSION
		"\n(c) 2003-2004 Urs Fleisch\nufleisch@users.sourceforge.net");
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
	QDir dir(view->mp3ListBox->getAbsDirname());
	QString fn = view->mp3ListBox->getAbsDirname() + QDir::separator() + dir.dirName() + ".m3u";
	QFile file(fn);
#if QT_VERSION >= 300
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#else
	QApplication::setOverrideCursor(QCursor(WaitCursor));
#endif
	slotStatusMsg(i18n("Creating playlist..."));
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		Mp3File *mp3file = view->mp3ListBox->first();
		while (mp3file != 0) {
			stream << mp3file->getFilename() << "\n";
			mp3file = view->mp3ListBox->next();
		}
		file.close();
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Check if the duration of the files corresponds to the duration of the
 * imported tracks.
 *
 * @param trackDuration list with durations of imported tracks
 * @param maxDiff       the maximum difference in seconds allowed
 *
 * @return true if ok or user agreed with import,
 *         false if user canceled import.
 */
bool Kid3App::checkDuration(QValueList<int>* trackDuration,
							unsigned maxDiff)
{
	if (!trackDuration) {
		return true;
	}
	QString warningMsg;
	int numFiles = 0;
	int numTracks = 0;
#if QT_VERSION >= 300
	QValueList<int>::iterator
#else
	QValueListIterator<int>
#endif
	it = trackDuration->begin();
	Mp3File *mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (it != trackDuration->end()) {
			mp3file->readTags(false);
			unsigned fileLen = mp3file->getDuration();
			unsigned trackLen = *it;
			if (fileLen != 0 && trackLen != 0) {
				unsigned diff = fileLen > trackLen ?
					fileLen - trackLen : trackLen - fileLen;
				if (diff > maxDiff) {
					warningMsg.append(mp3file->getFilename());
					warningMsg.append(": ");
					warningMsg.append(Mp3File::formatTime(fileLen));
					warningMsg.append(" != ");
					warningMsg.append(Mp3File::formatTime(trackLen));
					warningMsg.append('\n');
				}
			}
			++it;
			++numTracks;
		}
		mp3file = view->mp3ListBox->next();
		++numFiles;
	}
	if (numFiles != 0 && numTracks != 0 &&
		numFiles != numTracks) {
		warningMsg.append(i18n("Number of files is not equal to number of imported tracks"));
		warningMsg.append(QString(": %1 != %2\n").arg(numFiles).arg(numTracks));
	}
	if (!warningMsg.isEmpty()) {
		return (QMessageBox::warning(0, i18n("Import Mismatch"),
									 warningMsg + i18n("Continue import?"),
									 QMessageBox::Ok, QMessageBox::Cancel) ==
				QMessageBox::Ok);
	}
	return true;
}

/**
 * Import.
 */

void Kid3App::slotImport(void)
{
	QString caption(i18n("Import"));
	ImportDialog *dialog =
		new ImportDialog(NULL, caption);
	if (dialog) {
		dialog->setDestV1(genCfg->importDestV1);
		dialog->setImportFormat(genCfg->importFormatNames,
								genCfg->importFormatHeaders,
								genCfg->importFormatTracks,
								genCfg->importFormatIdx);
		dialog->setTimeDifferenceCheck(genCfg->enableTimeDifferenceCheck,
									   genCfg->maxTimeDifference);
		dialog->setFreedbConfig(freedbCfg);
		if (dialog->exec() == QDialog::Accepted) {
			genCfg->importDestV1 = dialog->getDestV1();
			QString name, header, track;
			genCfg->importFormatIdx = dialog->getImportFormat(name, header, track);
			genCfg->importFormatNames[genCfg->importFormatIdx] = name;
			genCfg->importFormatHeaders[genCfg->importFormatIdx] = header;
			genCfg->importFormatTracks[genCfg->importFormatIdx] = track;
			dialog->getTimeDifferenceCheck(genCfg->enableTimeDifferenceCheck,
										   genCfg->maxTimeDifference);
			dialog->getFreedbConfig(freedbCfg);
			StandardTags st_hdr;
			st_hdr.setInactive();
			(void)dialog->parseHeader(st_hdr);
			if (!genCfg->enableTimeDifferenceCheck ||
				checkDuration(dialog->getTrackDurations(), genCfg->maxTimeDifference)) {
#if QT_VERSION >= 300
				QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#else
				QApplication::setOverrideCursor(QCursor(WaitCursor));
#endif
				slotStatusMsg(i18n("Import..."));
				StandardTags st;
				bool start = true;
				bool no_selection = view->numFilesSelected() == 0;
				Mp3File *mp3file = view->mp3ListBox->first();
				while (mp3file != 0) {
					mp3file->readTags(false);
					if (genCfg->importDestV1) {
						mp3file->getStandardTagsV1(&st);
					} else {
						mp3file->getStandardTagsV2(&st);
					}
					st_hdr.copyActiveTags(st);
					if (!dialog->getNextTags(st, start))
						break;
					start = false;
					if (genCfg->importDestV1) {
						mp3file->setStandardTagsV1(&st);
					} else {
						mp3file->setStandardTagsV2(&st);
					}
					mp3file = view->mp3ListBox->next();
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
				slotStatusMsg(i18n("Ready."));
				QApplication::restoreOverrideCursor();
			}
		}
		delete dialog;
	}
}

/**
 * Preferences.
 */

void Kid3App::slotSettingsConfigure(void)
{
	QString caption(i18n("Configure - Kid3"));
	ConfigDialog *dialog =
		new ConfigDialog(NULL, caption);
	if (dialog) {
		dialog->setConfig(fnFormatCfg, id3FormatCfg, miscCfg);
		if (dialog->exec() == QDialog::Accepted) {
			dialog->getConfig(fnFormatCfg, id3FormatCfg, miscCfg);
#if defined CONFIG_USE_KDE || QT_VERSION >= 300
			fnFormatCfg->writeToConfig(config);
			id3FormatCfg->writeToConfig(config);
			genCfg->writeToConfig(config);
#endif
#ifdef CONFIG_USE_KDE
			config->sync();
#endif
		}
	}
}

/**
 * Apply format.
 */
void Kid3App::slotApplyFormat(void)
{
	StandardTags st;
	updateCurrentSelection();
	Mp3File *mp3file = view->mp3ListBox->first();
	bool no_selection = view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			QString str;
			str = mp3file->getFilename();
			fnFormatCfg->formatString(str);
			mp3file->setFilename(str);
			mp3file->getStandardTagsV1(&st);
			id3FormatCfg->formatStandardTags(st);
			mp3file->setStandardTagsV1(&st);
			mp3file->getStandardTagsV2(&st);
			id3FormatCfg->formatStandardTags(st);
			mp3file->setStandardTagsV2(&st);
		}
		mp3file = view->mp3ListBox->next();
	}
	if (!no_selection) {
		StandardTags st; // empty
		view->setStandardTagsV1(&st);
		view->setStandardTagsV2(&st);
		view->nameLineEdit->setEnabled(FALSE);
		fileSelected();
	} else {
		updateModificationState();
	}
}

/**
 * Rename directory.
 */
void Kid3App::slotRenameDirectory(void)
{
	if (saveModified()) {
		QString caption(i18n("Rename Directory"));
		RenDirDialog *dialog =
			new RenDirDialog(NULL, caption, view->mp3ListBox->first(),
							 miscCfg->dirFormatItem, miscCfg->dirFormatText);
		if (dialog) {
			if (dialog->exec() == QDialog::Accepted) {
				Mp3File *mp3file = view->mp3ListBox->first();
				QString errorMsg;
				bool again = false;
				while (mp3file &&
					   dialog->performAction(mp3file, again, &errorMsg)) {
					mp3file = view->mp3ListBox->next();
				}
				openDirectory(dialog->getNewDirname());
				if (again) {
					mp3file = view->mp3ListBox->first();
					while (mp3file &&
						   dialog->performAction(mp3file, again, &errorMsg)) {
						mp3file = view->mp3ListBox->next();
					}
					openDirectory(dialog->getNewDirname());
				}
				miscCfg->dirFormatItem = dialog->getFormatItem();
				miscCfg->dirFormatText = dialog->getFormatText();
				if (!errorMsg.isEmpty()) {
					QMessageBox::warning(0, i18n("File Error"),
										 i18n("Error while renaming:\n") +
										 errorMsg,
										 QMessageBox::Ok, QMessageBox::NoButton);
				}
			}
		}
	}
}

/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */

void Kid3App::openDrop(QString txt)
{
	int lfPos = txt.find('\n');
	if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
		txt.truncate(lfPos + 1);
	}
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
	setModified(view->mp3ListBox->updateModificationState());
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
	Mp3File *mp3file = view->mp3ListBox->first();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			updateTags(mp3file);
		}
		mp3file = view->mp3ListBox->next();
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
	Mp3File *mp3file = view->mp3ListBox->first(), *single_v2_file = NULL;
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
		mp3file = view->mp3ListBox->next();
	}
	view->setStandardTagsV1(&tags_v1);
	view->setStandardTagsV2(&tags_v2);
	view->setAllCheckBoxes(num_files_selected == 1);
	updateModificationState();
	if (single_v2_file) {
		framelist->setTags(single_v2_file);
		view->nameLineEdit->setEnabled(TRUE);
		view->nameLineEdit->setText(single_v2_file->getFilename());
		view->detailsLabel->setText(single_v2_file->getDetailInfo());
	}
	else {
		framelist->clear();
		view->nameLineEdit->setEnabled(FALSE);
		view->detailsLabel->setText("");
	}
}

/**
 * Copy a set of standard tags into copy buffer.
 *
 * @param st tags to copy
 */

void Kid3App::copyTags(const StandardTags *st)
{
	*copytags = *st;
}

/**
 * Paste from copy buffer to standard tags.
 *
 * @param st tags to fill from data in copy buffer.
 */

void Kid3App::pasteTags(StandardTags *st)
{
	if (!copytags->title.isNull())
		st->title = copytags->title;
	if (!copytags->artist.isNull())
		st->artist = copytags->artist;
	if (!copytags->album.isNull())
		st->album = copytags->album;
	if (!copytags->album.isNull())
		st->album = copytags->album;
	if (copytags->year >= 0)
		st->year = copytags->year;
	if (copytags->track >= 0)
		st->track = copytags->track;
	if (copytags->genre >= 0)
		st->genre = copytags->genre;
}

/**
 * Set ID3v1 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */

void Kid3App::getTagsFromFilenameV1(void)
{
	StandardTags st;
	Mp3File *mp3file = view->mp3ListBox->first();
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
		mp3file = view->mp3ListBox->next();
	}
	if (multiselect) {
		// update controls with filtered data
		fileSelected();
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
	Mp3File *mp3file = view->mp3ListBox->first();
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
		mp3file = view->mp3ListBox->next();
	}
	if (multiselect) {
		// update controls with filtered data
		fileSelected();
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
	Mp3File *mp3file = view->mp3ListBox->first();
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
		mp3file = view->mp3ListBox->next();
	}
}

/**
 * Copy ID3v1 tags to ID3v2 tags of selected files.
 */

void Kid3App::copyV1ToV2(void)
{
	StandardTags st;
	if (view->numFilesSelected() > 1) {
		Mp3File *mp3file = view->mp3ListBox->first();
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				mp3file->getStandardTagsV1(&st);
				mp3file->setStandardTagsV2(&st);
			}
			mp3file = view->mp3ListBox->next();
		}
		// update controls with filtered data
		fileSelected();
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
		Mp3File *mp3file = view->mp3ListBox->first();
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				mp3file->getStandardTagsV2(&st);
				mp3file->setStandardTagsV1(&st);
			}
			mp3file = view->mp3ListBox->next();
		}
		// update controls with filtered data
		fileSelected();
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
	Mp3File *mp3file = view->mp3ListBox->first();
	int i = 0;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->removeTagsV1();
		}
		mp3file = view->mp3ListBox->next();
		++i;
	}
}

/**
 * Remove ID3v1 tags in selected files.
 */

void Kid3App::removeTagsV2(void)
{
	Mp3File *mp3file = view->mp3ListBox->first();
	int i = 0;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->removeTagsV2();
		}
		mp3file = view->mp3ListBox->next();
		++i;
	}
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 */

void Kid3App::updateAfterFrameModification(void)
{
	Mp3File *mp3file = framelist->getFile();
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
	if (framelist->editFrame()) {
		updateAfterFrameModification();
	}
}

/**
 * Delete selected frame.
 */

void Kid3App::deleteFrame(void)
{
	if (framelist->deleteFrame()) {
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
		framelist->addFrame(id)) {
		updateAfterFrameModification();
	}
}
