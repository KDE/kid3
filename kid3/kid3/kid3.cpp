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
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#else
#include <qlayout.h>
#endif

#ifdef CONFIG_USE_KDE
#include <kapplication.h>
#include <kurl.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#if KDE_VERSION >= 0x035c00
#include <kstandardaction.h>
#include <kshortcutsdialog.h>
#include <krecentfilesaction.h>
#include <ktoolinvocation.h>
#include <kactioncollection.h>
#else
#include <kstdaction.h>
#include <kkeydialog.h>
#endif
#include <kedittoolbar.h>
#else
#include <qapplication.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#endif

#include "kid3.h"
#include "id3form.h"
#include "genres.h"
#include "framelist.h"
#include "frametable.h"
#include "configdialog.h"
#include "importdialog.h"
#include "exportdialog.h"
#include "numbertracksdialog.h"
#include "standardtags.h"
#include "rendirdialog.h"
#include "filelistitem.h"
#include "dirlist.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.h"
#endif
#ifdef HAVE_FLAC
#include "flacfile.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

#ifdef KID3_USE_KCONFIGDIALOG
#include <kconfigskeleton.h>
#endif

#ifndef CONFIG_USE_KDE
#include <qdialog.h>
#include <qtextbrowser.h>
#include <qlocale.h>

/**
 * Help browser.
 */
class BrowserDialog : public QDialog {
public:
	/**
	 * Constructor.
	 */
	BrowserDialog(QWidget* parent, QString& caption);

	/**
	 * Destructor.
	 */
	~BrowserDialog();

	/**
	 * Show context help.
	 * @param anchor name of anchor
	 */
	void goToAnchor(const QString& anchor);

private:
	QTextBrowser* m_textBrowser;
	QString m_filename;
};

BrowserDialog::BrowserDialog(QWidget* parent, QString& caption)
	: QDialog(parent)
{
	QCM_setWindowTitle(caption);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QString lang(QLocale::system().name().left(2));
	QStringList docPaths;
#ifdef CFG_DOCDIR
	docPaths += QString(CFG_DOCDIR) + "/kid3_" + lang + ".html";
	docPaths += QString(CFG_DOCDIR) + "/kid3_en.html";
#endif
	docPaths += QDir::QCM_currentPath() + "/kid3_" + lang + ".html";
	docPaths += QDir::QCM_currentPath() + "/kid3_en.html";
	for (QStringList::const_iterator it = docPaths.begin();
			 it != docPaths.end();
			 ++it) {
		m_filename = *it;
		if (QFile::exists(m_filename)) break;
	}
	m_textBrowser = new QTextBrowser(this);
#if QT_VERSION >= 0x040000
	m_textBrowser->setSource(QUrl::fromLocalFile(m_filename));
#else
	m_textBrowser->setSource(m_filename);
#endif
	vlayout->addWidget(m_textBrowser);

	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* backButton = new QPushButton(i18n("&Back"), this);
	QPushButton* forwardButton = new QPushButton(i18n("&Forward"), this);
	QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
	if (hlayout && backButton && forwardButton && closeButton) {
		hlayout->addWidget(backButton);
		hlayout->addWidget(forwardButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(closeButton);
		closeButton->setDefault(true);
		backButton->setEnabled(false);
		forwardButton->setEnabled(false);
		connect(backButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
		connect(forwardButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
		connect(m_textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
		connect(m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));
		connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
		vlayout->addLayout(hlayout);
	}
	resize(500, 500);
}

BrowserDialog::~BrowserDialog()
{}

void BrowserDialog::goToAnchor(const QString& anchor)
{
#if QT_VERSION >= 0x040000
	QUrl url = QUrl::fromLocalFile(m_filename);
	url.setFragment(anchor);
	m_textBrowser->setSource(url);
#else
	if (!anchor.isEmpty()) {
		m_textBrowser->setSource(m_filename + '#' + anchor);
	} else {
		m_textBrowser->setSource(m_filename);
	}
#endif
}

BrowserDialog* Kid3App::s_helpBrowser = 0;
#endif


MiscConfig Kid3App::s_miscCfg("General Options");
ImportConfig Kid3App::s_genCfg("General Options");
FormatConfig Kid3App::s_fnFormatCfg("FilenameFormat");
FormatConfig Kid3App::s_id3FormatCfg("Id3Format");
FreedbConfig Kid3App::s_freedbCfg("Freedb");
FreedbConfig Kid3App::s_trackTypeCfg("TrackType");
DiscogsConfig Kid3App::s_discogsCfg("Discogs");
MusicBrainzConfig Kid3App::s_musicBrainzCfg("MusicBrainz");

/** Current directory */
QString Kid3App::s_dirName;

/**
 * Constructor.
 *
 * @param name name
 */
Kid3App::Kid3App() :
	m_importDialog(0), m_exportDialog(0), m_numberTracksDialog(0)
{
	initStatusBar();
	setModified(false);
	initView();
	initActions();
	s_fnFormatCfg.setAsFilenameFormatter();

	resize(sizeHint());
#ifdef CONFIG_USE_KDE
#if KDE_VERSION >= 0x035c00
	m_config = new KConfig;
#else
	m_config = kapp->config();
#endif
#else
#if QT_VERSION >= 0x040000
	m_config = new Kid3Settings(QSettings::UserScope, "kid3.sourceforge.net", "Kid3");
#else
	m_config = new Kid3Settings();
	m_config->setPath("kid3.sourceforge.net", "Kid3", Kid3Settings::User);
#endif
	m_config->beginGroup("/kid3");
#if !defined _WIN32 && !defined WIN32 && defined CFG_DATAROOTDIR
	QPixmap icon;
	if (icon.load(QString(CFG_DATAROOTDIR) +
								"/icons/hicolor/48x48/apps/kid3-qt.png")) {
		QCM_setWindowIcon(icon);
	}
#endif
#endif
	readOptions();
}

/**
 * Destructor.
 */
Kid3App::~Kid3App()
{
	delete m_importDialog;
	delete m_numberTracksDialog;
#ifndef CONFIG_USE_KDE
	delete s_helpBrowser;
	s_helpBrowser = 0;
#endif
}

/**
 * Init menu and toolbar actions.
 */
void Kid3App::initActions()
{
#ifdef CONFIG_USE_KDE
	KAction* fileOpen = KCM_KStandardAction::open(
	    this, SLOT(slotFileOpen()), actionCollection());
	m_fileOpenRecent = KCM_KStandardAction::openRecent(
	    this,
#if KDE_VERSION >= 0x035c00
			SLOT(slotFileOpenRecentUrl(const KUrl&)),
#else
			SLOT(slotFileOpenRecent(const KURL&)),
#endif
			actionCollection());
	KAction* fileRevert = KCM_KStandardAction::revert(
	    this, SLOT(slotFileRevert()), actionCollection());
	KAction* fileSave = KCM_KStandardAction::save(
	    this, SLOT(slotFileSave()), actionCollection());
	KAction* fileQuit = KCM_KStandardAction::quit(
	    this, SLOT(slotFileQuit()), actionCollection());
	KAction* editSelectAll = KCM_KStandardAction::selectAll(
	    m_view, SLOT(selectAllFiles()), actionCollection());
	KAction* editDeselect = KCM_KStandardAction::deselect(
	    m_view, SLOT(deselectAllFiles()), actionCollection());
#if KDE_VERSION < 0x30200
	m_viewToolBar = KCM_KStandardAction::showToolbar(
	    this, SLOT(slotViewToolBar()), actionCollection());
	m_viewStatusBar = KCM_KStandardAction::showStatusbar(
	    this, SLOT(slotViewStatusBar()), actionCollection());
	m_viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	m_viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
#else
	setStandardToolBarMenuEnabled(true);
	createStandardStatusBarAction();
#endif
	KAction* settingsShortcuts = KCM_KStandardAction::keyBindings(
		this, SLOT(slotSettingsShortcuts()), actionCollection());
	KAction* settingsToolbars = KCM_KStandardAction::configureToolbars(
		this, SLOT(slotSettingsToolbars()), actionCollection());
	KAction* settingsConfigure = KCM_KStandardAction::preferences(
	    this, SLOT(slotSettingsConfigure()), actionCollection());

	fileOpen->KCM_setStatusTip(i18n("Opens a directory"));
	m_fileOpenRecent->KCM_setStatusTip(i18n("Opens a recently used directory"));
	fileRevert->KCM_setStatusTip(
	    i18n("Reverts the changes of all or the selected files"));
	fileSave->KCM_setStatusTip(i18n("Saves the changed files"));
	fileQuit->KCM_setStatusTip(i18n("Quits the application"));
	editSelectAll->KCM_setStatusTip(i18n("Select all files"));
	editSelectAll->setShortcut(KShortcut("Alt+A"));
	editDeselect->KCM_setStatusTip(i18n("Deselect all files"));
	settingsShortcuts->KCM_setStatusTip(i18n("Configure Shortcuts"));
	settingsToolbars->KCM_setStatusTip(i18n("Configure Toolbars"));
	settingsConfigure->KCM_setStatusTip(i18n("Preferences dialog"));

	KCM_KActionShortcutIcon(fileOpenDirectory, KShortcut("Ctrl+D"), "fileopen",
		    i18n("O&pen Directory..."), this,
		    SLOT(slotFileOpenDirectory()), actionCollection(),
		    "open_directory");
	KCM_KActionIcon(fileImport, "fileimport",
		    i18n("&Import..."), this,
		    SLOT(slotImport()), actionCollection(),
		    "import");
	KCM_KAction(fileImportFreedb,
		    i18n("Import from &gnudb.org..."), this,
		    SLOT(slotImportFreedb()), actionCollection(),
		    "import_freedb");
	KCM_KAction(fileImportTrackType,
		    i18n("Import from &TrackType.org..."), this,
		    SLOT(slotImportTrackType()), actionCollection(),
		    "import_tracktype");
	KCM_KAction(fileImportDiscogs,
		    i18n("Import from &Discogs..."), this,
		    SLOT(slotImportDiscogs()), actionCollection(),
		    "import_discogs");
	KCM_KAction(fileImportMusicBrainzRelease,
		    i18n("Import from MusicBrainz &Release..."), this,
		    SLOT(slotImportMusicBrainzRelease()), actionCollection(),
		    "import_musicbrainzrelease");
#ifdef HAVE_TUNEPIMP
	KCM_KAction(fileImportMusicBrainz,
		    i18n("Import from &MusicBrainz Fingerprint..."), this,
		    SLOT(slotImportMusicBrainz()), actionCollection(),
		    "import_musicbrainz");
#endif
	KCM_KActionIcon(fileExport, "fileexport",
		    i18n("&Export..."), this,
		    SLOT(slotExport()), actionCollection(),
		    "export");
	KCM_KActionIcon(fileCreatePlaylist, "player_playlist",
		    i18n("&Create Playlist"), this,
				SLOT(slotCreatePlaylist()), actionCollection(),
				"create_playlist");
	KCM_KAction(toolsApplyFilenameFormat,
		    i18n("Apply &Filename Format"), this,
		    SLOT(slotApplyFilenameFormat()), actionCollection(),
		    "apply_filename_format");
	KCM_KAction(toolsApplyId3Format,
		    i18n("Apply &Tag Format"), this,
		    SLOT(slotApplyId3Format()), actionCollection(),
		    "apply_id3_format");
	KCM_KAction(toolsRenameDirectory,
		    i18n("&Rename Directory..."), this,
		    SLOT(slotRenameDirectory()), actionCollection(),
		    "rename_directory");
	KCM_KAction(toolsNumberTracks,
		    i18n("&Number Tracks..."), this,
		    SLOT(slotNumberTracks()), actionCollection(),
		    "number_tracks");
#ifdef HAVE_TAGLIB
	KCM_KAction(toolsConvertToId3v24,
		    i18n("Convert ID3v2.3 to ID3v2.&4"), this,
		    SLOT(slotConvertToId3v24()), actionCollection(),
		    "convert_to_id3v24");
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	KCM_KAction(toolsConvertToId3v23,
		    i18n("Convert ID3v2.4 to ID3v2.&3"), this,
		    SLOT(slotConvertToId3v23()), actionCollection(),
		    "convert_to_id3v23");
#endif
	KCM_KActionVar(m_settingsShowHideV1,
		            i18n("Hide Tag &1"), this,
								SLOT(slotSettingsShowHideV1()), actionCollection(),
								"hide_v1");
	KCM_KActionVar(m_settingsShowHideV2,
		            i18n("Hide Tag &2"), this,
								SLOT(slotSettingsShowHideV2()), actionCollection(),
								"hide_v2");

	KCM_KActionShortcutIcon(editPreviousFile, KShortcut("Alt+Up"), "previous",
		    i18n("&Previous File"), m_view,
		    SLOT(selectPreviousFile()), actionCollection(),
		    "previous_file");
	KCM_KActionShortcutIcon(editNextFile, KShortcut("Alt+Down"), "next",
		    i18n("&Next File"), m_view,
		    SLOT(selectNextFile()), actionCollection(),
		    "next_file");
	KCM_KAction(actionV1FromFilename,
				i18n("Tag 1") + ": " + i18n("From Filename"), m_view, SLOT(fromFilenameV1()),
				actionCollection(), "v1_from_filename");
	KCM_KAction(actionV1FromV2,
				i18n("Tag 1") + ": " + i18n("From Tag 2"), m_view, SLOT(fromID3V1()),
				actionCollection(), "v1_from_v2");
	KCM_KAction(actionV1Copy,
				i18n("Tag 1") + ": " + i18n("Copy"), m_view, SLOT(copyV1()),
				actionCollection(), "v1_copy");
	KCM_KAction(actionV1Paste,
				i18n("Tag 1") + ": " + i18n("Paste"), m_view, SLOT(pasteV1()),
				actionCollection(), "v1_paste");
	KCM_KAction(actionV1Remove,
				i18n("Tag 1") + ": " + i18n("Remove"), m_view, SLOT(removeV1()),
				actionCollection(), "v1_remove");
	KCM_KAction(actionV2FromFilename,
				i18n("Tag 2") + ": " + i18n("From Filename"), m_view, SLOT(fromFilenameV2()),
				actionCollection(), "v2_from_filename");
	KCM_KAction(actionV2FromV1,
				i18n("Tag 2") + ": " + i18n("From Tag 1"), m_view, SLOT(fromID3V2()),
				actionCollection(), "v2_from_v1");
	KCM_KAction(actionV2Copy,
				i18n("Tag 2") + ": " + i18n("Copy"), m_view, SLOT(copyV2()),
				actionCollection(), "v2_copy");
	KCM_KAction(actionV2Paste,
				i18n("Tag 2") + ": " + i18n("Paste"), m_view, SLOT(pasteV2()),
				actionCollection(), "v2_paste");
	KCM_KAction(actionV2Remove,
				i18n("Tag 2") + ": " + i18n("Remove"), m_view, SLOT(removeV2()),
				actionCollection(), "v2_remove");
	KCM_KAction(actionFramesEdit,
				i18n("Frames:") + " " + i18n("Edit"), m_view, SLOT(editFrame()),
				actionCollection(), "frames_edit");
	KCM_KAction(actionFramesAdd,
				i18n("Frames:") + " " + i18n("Add"), m_view, SLOT(addFrame()),
				actionCollection(), "frames_add");
	KCM_KAction(actionFramesDelete,
				i18n("Frames:") + " " + i18n("Delete"), m_view, SLOT(deleteFrame()),
				actionCollection(), "frames_delete");
	KCM_KAction(actionFilenameFromV1,
				i18n("Filename") + ": " + i18n("From Tag 1"), m_view, SLOT(fnFromID3V1()),
				actionCollection(), "filename_from_v1");
	KCM_KAction(actionFilenameFromV2,
				i18n("Filename") + ": " + i18n("From Tag 2"), m_view, SLOT(fnFromID3V2()),
				actionCollection(), "filename_from_v2");
	KCM_KAction(actionFilenameFocus,
				i18n("Filename") + ": " + i18n("Focus"), m_view, SLOT(setFocusFilename()),
				actionCollection(), "filename_focus");
	KCM_KAction(actionV1Focus,
				i18n("Tag 1") + ": " + i18n("Focus"), m_view, SLOT(setFocusV1()),
				actionCollection(), "v1_focus");
	KCM_KAction(actionV2Focus,
				i18n("Tag 2") + ": " + i18n("Focus"), m_view, SLOT(setFocusV2()),
				actionCollection(), "v2_focus");

	createGUI();

#else
	QAction* fileOpen = new QAction(this);
	if (fileOpen) {
		fileOpen->setStatusTip(i18n("Opens a directory"));
		fileOpen->QCM_setMenuText(i18n("&Open..."));
		fileOpen->QCM_setShortcut(Qt::CTRL + Qt::Key_O);
		connect(fileOpen, QCM_SIGNAL_triggered,
			this, SLOT(slotFileOpen()));
	}
	QAction* fileOpenDirectory = new QAction(this);
	if (fileOpenDirectory) {
		fileOpenDirectory->setStatusTip(i18n("Opens a directory"));
		fileOpenDirectory->QCM_setMenuText(i18n("O&pen Directory..."));
		fileOpenDirectory->QCM_setShortcut(Qt::CTRL + Qt::Key_D);
		connect(fileOpenDirectory, QCM_SIGNAL_triggered,
			this, SLOT(slotFileOpenDirectory()));
	}
	QAction* fileSave = new QAction(this);
	if (fileSave) {
		fileSave->setStatusTip(i18n("Saves the changed files"));
		fileSave->QCM_setMenuText(i18n("&Save"));
		fileSave->QCM_setShortcut(Qt::CTRL + Qt::Key_S);
		connect(fileSave, QCM_SIGNAL_triggered,
			this, SLOT(slotFileSave()));
	}
	QAction* fileRevert = new QAction(this);
	if (fileRevert) {
		fileRevert->setStatusTip(
		    i18n("Reverts the changes of all or the selected files"));
		fileRevert->QCM_setMenuText(i18n("Re&vert"));
		connect(fileRevert, QCM_SIGNAL_triggered,
			this, SLOT(slotFileRevert()));
	}
	QAction* fileImport = new QAction(this);
	if (fileImport) {
		fileImport->setStatusTip(i18n("Import from file or clipboard"));
		fileImport->QCM_setMenuText(i18n("&Import..."));
		connect(fileImport, QCM_SIGNAL_triggered,
			this, SLOT(slotImport()));
	}
	QAction* fileImportFreedb = new QAction(this);
	if (fileImportFreedb) {
		fileImportFreedb->setStatusTip(i18n("Import from gnudb.org"));
		fileImportFreedb->QCM_setMenuText(i18n("Import from &gnudb.org..."));
		connect(fileImportFreedb, QCM_SIGNAL_triggered,
			this, SLOT(slotImportFreedb()));
	}
	QAction* fileImportTrackType = new QAction(this);
	if (fileImportTrackType) {
		fileImportTrackType->setStatusTip(i18n("Import from TrackType.org"));
		fileImportTrackType->QCM_setMenuText(i18n("Import from &TrackType.org..."));
		connect(fileImportTrackType, QCM_SIGNAL_triggered,
			this, SLOT(slotImportTrackType()));
	}
	QAction* fileImportDiscogs = new QAction(this);
	if (fileImportDiscogs) {
		fileImportDiscogs->setStatusTip(i18n("Import from Discogs"));
		fileImportDiscogs->QCM_setMenuText(i18n("Import from &Discogs..."));
		connect(fileImportDiscogs, QCM_SIGNAL_triggered,
			this, SLOT(slotImportDiscogs()));
	}
	QAction* fileImportMusicBrainzRelease = new QAction(this);
	if (fileImportMusicBrainzRelease) {
		fileImportMusicBrainzRelease->setStatusTip(i18n("Import from MusicBrainz Release"));
		fileImportMusicBrainzRelease->QCM_setMenuText(i18n("Import from MusicBrainz &Release..."));
		connect(fileImportMusicBrainzRelease, QCM_SIGNAL_triggered,
			this, SLOT(slotImportMusicBrainzRelease()));
	}
#ifdef HAVE_TUNEPIMP
	QAction* fileImportMusicBrainz = new QAction(this);
	if (fileImportMusicBrainz) {
		fileImportMusicBrainz->setStatusTip(i18n("Import from MusicBrainz Fingerprint"));
		fileImportMusicBrainz->QCM_setMenuText(i18n("Import from &MusicBrainz Fingerprint..."));
		connect(fileImportMusicBrainz, QCM_SIGNAL_triggered,
			this, SLOT(slotImportMusicBrainz()));
	}
#endif
	QAction* fileExport = new QAction(this);
	if (fileExport) {
		fileExport->setStatusTip(i18n("Export to file or clipboard"));
		fileExport->QCM_setMenuText(i18n("&Export..."));
		connect(fileExport, QCM_SIGNAL_triggered,
			this, SLOT(slotExport()));
	}
	QAction* fileCreatePlaylist = new QAction(this);
	if (fileCreatePlaylist) {
		fileCreatePlaylist->setStatusTip(i18n("Create M3U Playlist"));
		fileCreatePlaylist->QCM_setMenuText(i18n("&Create Playlist"));
		connect(fileCreatePlaylist, QCM_SIGNAL_triggered,
			this, SLOT(slotCreatePlaylist()));
	}
	QAction* fileQuit = new QAction(this);
	if (fileQuit) {
		fileQuit->setStatusTip(i18n("Quits the application"));
		fileQuit->QCM_setMenuText(i18n("&Quit"));
		fileQuit->QCM_setShortcut(Qt::CTRL + Qt::Key_Q);
		connect(fileQuit, QCM_SIGNAL_triggered,
			this, SLOT(slotFileQuit()));
	}
	QAction* editSelectAll = new QAction(this);
	if (editSelectAll) {
		editSelectAll->setStatusTip(i18n("Select all files"));
		editSelectAll->QCM_setMenuText(i18n("Select &All"));
		editSelectAll->QCM_setShortcut(Qt::ALT + Qt::Key_A);
		connect(editSelectAll, QCM_SIGNAL_triggered,
			m_view, SLOT(selectAllFiles()));
	}
	QAction* editDeselect = new QAction(this);
	if (editDeselect) {
		editDeselect->setStatusTip(i18n("Deselect all files"));
		editDeselect->QCM_setMenuText(i18n("Dese&lect"));
		editDeselect->QCM_setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);
		connect(editDeselect, QCM_SIGNAL_triggered,
			m_view, SLOT(deselectAllFiles()));
	}
	QAction* editPreviousFile = new QAction(this);
	if (editPreviousFile) {
		editPreviousFile->setStatusTip(i18n("Select previous file"));
		editPreviousFile->QCM_setMenuText(i18n("&Previous File"));
		editPreviousFile->QCM_setShortcut(Qt::ALT + Qt::Key_Up);
		connect(editPreviousFile, QCM_SIGNAL_triggered,
			m_view, SLOT(selectPreviousFile()));
	}
	QAction* editNextFile = new QAction(this);
	if (editNextFile) {
		editNextFile->setStatusTip(i18n("Select next file"));
		editNextFile->QCM_setMenuText(i18n("&Next File"));
		editNextFile->QCM_setShortcut(Qt::ALT + Qt::Key_Down);
		connect(editNextFile, QCM_SIGNAL_triggered,
			m_view, SLOT(selectNextFile()));
	}
	QAction* helpHandbook = new QAction(this);
	if (helpHandbook) {
		helpHandbook->setStatusTip(i18n("Kid3 Handbook"));
		helpHandbook->QCM_setMenuText(i18n("Kid3 &Handbook"));
		connect(helpHandbook, QCM_SIGNAL_triggered,
			this, SLOT(slotHelpHandbook()));
	}
	QAction* helpAbout = new QAction(this);
	if (helpAbout) {
		helpAbout->setStatusTip(i18n("About Kid3"));
		helpAbout->QCM_setMenuText(i18n("&About Kid3"));
		connect(helpAbout, QCM_SIGNAL_triggered,
			this, SLOT(slotHelpAbout()));
	}
	QAction* helpAboutQt = new QAction(this);
	if (helpAboutQt) {
		helpAboutQt->setStatusTip(i18n("About Qt"));
		helpAboutQt->QCM_setMenuText(i18n("About &Qt"));
		connect(helpAboutQt, QCM_SIGNAL_triggered,
			this, SLOT(slotHelpAboutQt()));
	}
	QAction* toolsApplyFilenameFormat = new QAction(this);
	if (toolsApplyFilenameFormat) {
		toolsApplyFilenameFormat->setStatusTip(i18n("Apply Filename Format"));
		toolsApplyFilenameFormat->QCM_setMenuText(i18n("Apply &Filename Format"));
		connect(toolsApplyFilenameFormat, QCM_SIGNAL_triggered,
			this, SLOT(slotApplyFilenameFormat()));
	}
	QAction* toolsApplyId3Format = new QAction(this);
	if (toolsApplyId3Format) {
		toolsApplyId3Format->setStatusTip(i18n("Apply Tag Format"));
		toolsApplyId3Format->QCM_setMenuText(i18n("Apply &Tag Format"));
		connect(toolsApplyId3Format, QCM_SIGNAL_triggered,
			this, SLOT(slotApplyId3Format()));
	}
	QAction* toolsRenameDirectory = new QAction(this);
	if (toolsRenameDirectory) {
		toolsRenameDirectory->setStatusTip(i18n("Rename Directory"));
		toolsRenameDirectory->QCM_setMenuText(i18n("&Rename Directory..."));
		connect(toolsRenameDirectory, QCM_SIGNAL_triggered,
			this, SLOT(slotRenameDirectory()));
	}
	QAction* toolsNumberTracks = new QAction(this);
	if (toolsNumberTracks) {
		toolsNumberTracks->setStatusTip(i18n("Number Tracks"));
		toolsNumberTracks->QCM_setMenuText(i18n("&Number Tracks..."));
		connect(toolsNumberTracks, QCM_SIGNAL_triggered,
			this, SLOT(slotNumberTracks()));
	}
#ifdef HAVE_TAGLIB
	QAction* toolsConvertToId3v24 = new QAction(this);
	if (toolsConvertToId3v24) {
		toolsConvertToId3v24->setStatusTip(i18n("Convert ID3v2.3 to ID3v2.4"));
		toolsConvertToId3v24->QCM_setMenuText(i18n("Convert ID3v2.3 to ID3v2.&4"));
		connect(toolsConvertToId3v24, QCM_SIGNAL_triggered,
			this, SLOT(slotConvertToId3v24()));
	}
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	QAction* toolsConvertToId3v23 = new QAction(this);
	if (toolsConvertToId3v23) {
		toolsConvertToId3v23->setStatusTip(i18n("Convert ID3v2.4 to ID3v2.3"));
		toolsConvertToId3v23->QCM_setMenuText(i18n("Convert ID3v2.4 to ID3v2.&3"));
		connect(toolsConvertToId3v23, QCM_SIGNAL_triggered,
			this, SLOT(slotConvertToId3v23()));
	}
#endif
	m_settingsShowHideV1 = new QAction(this);
	if (m_settingsShowHideV1) {
		m_settingsShowHideV1->setStatusTip(i18n("Hide Tag 1"));
		m_settingsShowHideV1->QCM_setMenuText(i18n("Hide Tag &1"));
		connect(m_settingsShowHideV1, QCM_SIGNAL_triggered,
			this, SLOT(slotSettingsShowHideV1()));
	}
	m_settingsShowHideV2 = new QAction(this);
	if (m_settingsShowHideV2) {
		m_settingsShowHideV2->setStatusTip(i18n("Hide Tag 2"));
		m_settingsShowHideV2->QCM_setMenuText(i18n("Hide Tag &2"));
		connect(m_settingsShowHideV2, QCM_SIGNAL_triggered,
			this, SLOT(slotSettingsShowHideV2()));
	}
	QAction* settingsConfigure = new QAction(this);
	if (settingsConfigure) {
		settingsConfigure->setStatusTip(i18n("Configure Kid3"));
		settingsConfigure->QCM_setMenuText(i18n("&Configure Kid3..."));
		connect(settingsConfigure, QCM_SIGNAL_triggered,
			this, SLOT(slotSettingsConfigure()));
	}
#if QT_VERSION >= 0x040000
	QMenuBar* menubar = menuBar();
	QMenu* fileMenu = menubar->addMenu(i18n("&File"));
	QMenu* editMenu = menubar->addMenu(i18n("&Edit"));
	QMenu* toolsMenu = menubar->addMenu(i18n("&Tools"));
	QMenu* settingsMenu = menubar->addMenu(i18n("&Settings"));
	QMenu* helpMenu = menubar->addMenu(i18n("&Help"));
#else
	QMenuBar* menubar = new QMenuBar(this);
	QPopupMenu* fileMenu = new QPopupMenu(this);
	menubar->insertItem((i18n("&File")), fileMenu);
	QPopupMenu* editMenu = new QPopupMenu(this);
	menubar->insertItem((i18n("&Edit")), editMenu);
	QPopupMenu* toolsMenu = new QPopupMenu(this);
	menubar->insertItem((i18n("&Tools")), toolsMenu);
	QPopupMenu* settingsMenu = new QPopupMenu(this);
	menubar->insertItem(i18n("&Settings"), settingsMenu);
	QPopupMenu* helpMenu = new QPopupMenu(this);
	menubar->insertItem(i18n("&Help"), helpMenu);
#endif
	if (fileMenu && editMenu && toolsMenu && settingsMenu && helpMenu) {
		QCM_addAction(fileMenu, fileOpen);
		QCM_addAction(fileMenu, fileOpenDirectory);
		fileMenu->QCM_addSeparator();
		QCM_addAction(fileMenu, fileSave);
		QCM_addAction(fileMenu, fileRevert);
		fileMenu->QCM_addSeparator();
		QCM_addAction(fileMenu, fileImport);
		QCM_addAction(fileMenu, fileImportFreedb);
		QCM_addAction(fileMenu, fileImportTrackType);
		QCM_addAction(fileMenu, fileImportDiscogs);
		QCM_addAction(fileMenu, fileImportMusicBrainzRelease);
#ifdef HAVE_TUNEPIMP
		QCM_addAction(fileMenu, fileImportMusicBrainz);
#endif
		QCM_addAction(fileMenu, fileExport);
		QCM_addAction(fileMenu, fileCreatePlaylist);
		fileMenu->QCM_addSeparator();
		QCM_addAction(fileMenu, fileQuit);

		QCM_addAction(editMenu, editSelectAll);
		QCM_addAction(editMenu, editDeselect);
		QCM_addAction(editMenu, editPreviousFile);
		QCM_addAction(editMenu, editNextFile);

		QCM_addAction(toolsMenu, toolsApplyFilenameFormat);
		QCM_addAction(toolsMenu, toolsApplyId3Format);
		QCM_addAction(toolsMenu, toolsRenameDirectory);
		QCM_addAction(toolsMenu, toolsNumberTracks);
#ifdef HAVE_TAGLIB
		QCM_addAction(toolsMenu, toolsConvertToId3v24);
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
		QCM_addAction(toolsMenu, toolsConvertToId3v23);
#endif

		QCM_addAction(settingsMenu, m_settingsShowHideV1);
		QCM_addAction(settingsMenu, m_settingsShowHideV2);
		settingsMenu->QCM_addSeparator();
		QCM_addAction(settingsMenu, settingsConfigure);

		QCM_addAction(helpMenu, helpHandbook);
		QCM_addAction(helpMenu, helpAbout);
		QCM_addAction(helpMenu, helpAboutQt);
	}
	QCM_setWindowTitle("Kid3");
#endif
}

/**
 * Init status bar.
 */
void Kid3App::initStatusBar()
{
	statusBar()->QCM_showMessage(i18n("Ready."));
}

/**
 * Init GUI.
 */
void Kid3App::initView()
{ 
	m_view = new Id3Form(this);
	if (m_view) {
		setCentralWidget(m_view);
		m_view->initView();
		m_framelist = m_view->getFrameList();
		connect(m_view, SIGNAL(selectedFilesRenamed()),
						this, SLOT(updateGuiControls()));
	}
}

/**
 * Open directory.
 *
 * @param dir     directory or file path
 * @param confirm if true ask if there are unsaved changes
 */
void Kid3App::openDirectory(QString dir, bool confirm)
{
	if (confirm && !saveModified()) {
		return;
	}
	if (dir.isNull() || dir.isEmpty()) {
		return;
	}
	QFileInfo file(dir);
	if (!file.isDir()) {
#if QT_VERSION >= 0x040000
		dir = file.dir().path();
#else
		dir = file.dirPath(true);
#endif
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Opening directory..."));
	if (m_view->readFileList(dir)) {
		m_view->readDirectoryList(dir);
		setModified(false);
#ifdef CONFIG_USE_KDE
#if KDE_VERSION >= 0x035c00
		KUrl url;
#else
		KURL url;
#endif
		url.setPath(dir);
		m_fileOpenRecent->KCM_addUrl(url);
		setCaption(dir, false);
#else
		QCM_setWindowTitle(dir + " - Kid3");
#endif
		s_dirName = dir;
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
#if KDE_VERSION >= 0x035c00
	m_fileOpenRecent->saveEntries(KConfigGroup(m_config, "Recent Files"));
#else
	m_fileOpenRecent->saveEntries(m_config, "Recent Files");
#endif
#else
	s_miscCfg.m_windowWidth = size().width();
	s_miscCfg.m_windowHeight = size().height();
#endif
	m_view->saveConfig();

	s_miscCfg.writeToConfig(m_config);
	s_fnFormatCfg.writeToConfig(m_config);
	s_id3FormatCfg.writeToConfig(m_config);
	s_genCfg.writeToConfig(m_config);
	s_freedbCfg.writeToConfig(m_config);
	s_trackTypeCfg.writeToConfig(m_config);
	s_discogsCfg.writeToConfig(m_config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.writeToConfig(m_config);
#endif
}

/**
 * Load application options.
 */
void Kid3App::readOptions()
{
	s_miscCfg.readFromConfig(m_config);
	s_fnFormatCfg.readFromConfig(m_config);
	s_id3FormatCfg.readFromConfig(m_config);
	s_genCfg.readFromConfig(m_config);
	s_freedbCfg.readFromConfig(m_config);
	if (s_freedbCfg.m_server == "freedb2.org:80") {
		s_freedbCfg.m_server = "www.gnudb.org:80"; // replace old default
	}
	s_trackTypeCfg.readFromConfig(m_config);
	if (s_trackTypeCfg.m_server == "gnudb.gnudb.org:80") {
		s_trackTypeCfg.m_server = "tracktype.org:80"; // replace default
	}
	s_discogsCfg.readFromConfig(m_config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.readFromConfig(m_config);
#endif
	updateHideV1();
	updateHideV2();
#ifdef CONFIG_USE_KDE
	setAutoSaveSettings();
#if KDE_VERSION >= 0x035c00
	m_fileOpenRecent->loadEntries(KConfigGroup(m_config,"Recent Files"));
#else
	m_fileOpenRecent->loadEntries(m_config,"Recent Files");
#endif
#if KDE_VERSION < 0x30200
	m_viewToolBar->setChecked(!toolBar("mainToolBar")->isHidden());
	m_viewStatusBar->setChecked(!statusBar()->isHidden());
#endif
#else
	if (s_miscCfg.m_windowWidth != -1 && s_miscCfg.m_windowHeight != -1) {
		resize(s_miscCfg.m_windowWidth, s_miscCfg.m_windowHeight);
	}
#endif
	m_view->readConfig();
}

#ifdef CONFIG_USE_KDE
#if KDE_VERSION >= 0x035c00
/**
 * Saves the window properties to the session config file.
 *
 * @param cfg application configuration
 */
void Kid3App::saveProperties(KConfigGroup& cfg)
{
	cfg.writeEntry("dirname", s_dirName);
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param cfg application configuration
 */
void Kid3App::readProperties(KConfigGroup& cfg)
{
	openDirectory(cfg.readEntry("dirname", ""));
}
#else
/**
 * Saves the window properties to the session config file.
 *
 * @param cfg application configuration
 */
void Kid3App::saveProperties(KConfig* cfg)
{
	if (cfg) { // otherwise KDE 3.0 compiled program crashes with KDE 3.1
		cfg->writeEntry("dirname", s_dirName);
	}
}

/**
 * Reads the session config file and restores the application's state.
 *
 * @param cfg application configuration
 */
void Kid3App::readProperties(KConfig* cfg)
{
	openDirectory(cfg->readEntry("dirname", ""));
}
#endif

#else /* CONFIG_USE_KDE */

/**
 * Window is closed.
 *
 * @param ce close event
 */
void Kid3App::closeEvent(QCloseEvent* ce)
{
	if (queryClose()) {
		ce->accept();
	}
	else {
		ce->ignore();
	}
}
#endif /* CONFIG_USE_KDE */

/**
 * Save all changed files.
 *
 * @return true
 */
bool Kid3App::saveDirectory()
{
	QStringList errorFiles;
	int numFiles = 0, totalFiles = 0;
	FileListItem* mp3file = m_view->firstFile();
	// Get number of files to be saved to display correct progressbar
	while (mp3file != 0) {
		if (mp3file->getFile()->isChanged()) {
			++totalFiles;
		}
		mp3file = m_view->nextFile();
	}
	QProgressBar* progress = new QProgressBar();
#if QT_VERSION >= 0x040000
	statusBar()->addPermanentWidget(progress);
	progress->setMinimum(0);
	progress->setMaximum(totalFiles);
	progress->setValue(numFiles);
#else
	statusBar()->addWidget(progress, 0, true);
	progress->setTotalSteps(totalFiles);
	progress->setProgress(numFiles);
#endif
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
	mp3file = m_view->firstFile();
	while (mp3file != 0) {
		bool renamed = false;
		if (!mp3file->getFile()->writeTags(false, &renamed, s_miscCfg.m_preserveTime)) {
			errorFiles.push_back(mp3file->getFile()->getFilename());
		}
		if (renamed) {
			mp3file->updateText();
		}
		mp3file = m_view->nextFile();
		++numFiles;
#if QT_VERSION >= 0x040000
		progress->setValue(numFiles);
#else
		progress->setProgress(numFiles);
#endif
	}
	statusBar()->removeWidget(progress);
	delete progress;
	updateModificationState();
	if (!errorFiles.empty()) {
#ifdef CONFIG_USE_KDE
		KMessageBox::errorList(
			0, i18n("Error while writing file:\n"),
			errorFiles,
			i18n("File Error"));
#else
		QMessageBox::warning(
			0, i18n("File Error"),
			i18n("Error while writing file:\n") +
			errorFiles.join("\n"),
			QMessageBox::Ok, QCM_NoButton);
#endif
	}
	return true;
}

/**
 * If anything was modified, save after asking user.
 *
 * @return false if user canceled.
 */
bool Kid3App::saveModified()
{
	bool completed=true;

	if(isModified() && !s_dirName.isEmpty())
	{
		Kid3App* win=(Kid3App *) parent();
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
#ifdef _MSC_VER
	// A _BLOCK_TYPE_IS_VALID assertion pops up if config is deleted
	// on Windows, MSVC 2005, Qt 4.1.2
	m_config->sync();
#else
	delete m_config;
#endif
#elif KDE_VERSION >= 0x035c00
	m_config->sync();
	delete m_config;
#endif
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
		QString dir, filter, flt;
#ifdef CONFIG_USE_KDE
		flt = "*.mp3 *.ogg *.flac *.mpc *.MP3 *.OGG *.FLAC *.MPC *.Mp3 *.Ogg *.Flac *.Mpc *.mP3 *.ogG *.oGg *.oGG *.OgG *.OGg *.flaC *.flAc *.flAC *.FlaC *.FlAc *.mpC *.mPc *.mPC *.MpC *.MPc|MP3, OGG, FLAC, MPC (*.mp3, *.ogg, *.flac *.mpc)\n";
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
		flt += "*.mp3 *.MP3 *.Mp3 *.mP3|MP3 (*.mp3)\n";
#endif
#if defined HAVE_VORBIS || defined HAVE_TAGLIB
		flt += "*.ogg *.OGG *.Ogg *.ogG *.oGg *.oGG *.OgG *.OGg|OGG (*.ogg)\n";
#endif
#if defined HAVE_FLAC || defined HAVE_TAGLIB
		flt += "*.flac *.FLAC *.Flac *.flaC *.flAc *.flAC *.FlaC *.FlAc|FLAC (*.flac)\n";
#endif
#ifdef HAVE_TAGLIB
		flt += "*.mpc *.MPC *.Mpc *.mpC *.mPc *.mPC *.MpC *.MPc|MPC (*.mpc)\n";
#endif
		flt += ("*|All Files (*)");
#if KDE_VERSION >= 0x035c00
		KFileDialog diag(s_dirName, flt, this);
#else
		KFileDialog diag(
		    s_dirName,
		    flt,
		    this, "filedialog", true);
#endif
		diag.QCM_setWindowTitle(i18n("Open"));
		if (diag.exec() == QDialog::Accepted) {
			dir = diag.selectedFile();
			filter = diag.currentFilter();
		}
#else
		flt = "MP3, OGG, FLAC, MPC (*.mp3 *.ogg *.flac *.mpc *.MP3 *.OGG *.FLAC *.MPC *.Mp3 *.Ogg *.Flac *.Mpc *.mP3 *.ogG *.oGg *.oGG *.OgG *.OGg *.flaC *.flAc *.flAC *.FlaC *.FlAc *.mpC *.mPc *.mPC *.MpC *.MPc);;";
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
		flt += "MP3 (*.mp3 *.MP3 *.Mp3 *.mP3);;";
#endif
#if defined HAVE_VORBIS || defined HAVE_TAGLIB
		flt += "OGG (*.ogg *.OGG *.Ogg *.ogG *.oGg *.oGG *.OgG *.OGg);;";
#endif
#if defined HAVE_FLAC || defined HAVE_TAGLIB
		flt += "FLAC (*.flac *.FLAC *.Flac *.flaC *.flAc *.flAC *.FlaC *.FlAc);;";
#endif
#ifdef HAVE_TAGLIB
		flt += "MPC (*.mpc *.MPC *.Mpc *.mpC *.mPc *.mPC *.MpC *.MPc);;";
#endif
		flt += i18n("All Files (*)");
#if QT_VERSION >= 0x040000
		dir = QFileDialog::getOpenFileName(
			this, QString(), s_dirName, flt, &filter);
#else
		dir = QFileDialog::getOpenFileName(
		    s_dirName, flt,
		    this, 0, QString::null, &filter);
#endif
#endif
		if (!dir.isEmpty()) {
			int start = filter.QCM_indexOf('('), end = filter.QCM_indexOf(')');
			if (start != -1 && end != -1 && end > start) {
				filter = filter.mid(start + 1, end - start - 1);
			}
			s_miscCfg.m_nameFilter = filter;
			openDirectory(dir);
		}
	}
}

/**
 * Request new directory and open it.
 */
void Kid3App::slotFileOpenDirectory()
{
	updateCurrentSelection();
	if(saveModified()) {
		QString dir;
#ifdef CONFIG_USE_KDE
		dir = KFileDialog::getExistingDirectory(s_dirName, this);
#else
#if QT_VERSION >= 0x040000
		dir = QFileDialog::getExistingDirectory(this, QString(), s_dirName);
#else
		dir = QFileDialog::getExistingDirectory(s_dirName, this);
#endif
#endif
		if (!dir.isEmpty()) {
			openDirectory(dir);
		}
	}
}

#ifdef CONFIG_USE_KDE
/**
 * Open recent directory.
 *
 * @param url URL of directory to open
 */
#if KDE_VERSION >= 0x035c00
void Kid3App::slotFileOpenRecentUrl(const KUrl& url)
{
	updateCurrentSelection();
	QString dir = url.path();
	openDirectory(dir, true);
}

void Kid3App::slotFileOpenRecent(const KURL&) {}
#else
void Kid3App::slotFileOpenRecent(const KURL& url)
{
	updateCurrentSelection();
	QString dir = url.path();
	openDirectory(dir, true);
}

void Kid3App::slotFileOpenRecentUrl(const KUrl&) {}
#endif
#else /* CONFIG_USE_KDE */
void Kid3App::slotFileOpenRecent(const KURL&) {}
void Kid3App::slotFileOpenRecentUrl(const KUrl&) {}
#endif /* CONFIG_USE_KDE */

/**
 * Revert file modifications.
 * Acts on selected files or all files if no file is selected.
 */
void Kid3App::slotFileRevert()
{
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(true);
		}
		mp3file = m_view->nextFile();
	}
	if (!no_selection) {
		m_view->frameTableV1()->frames().clear();
		m_view->frameTableV1()->framesToTable();
		m_view->frameTableV2()->frames().clear();
		m_view->frameTableV2()->framesToTable();
		m_view->setFilenameEditEnabled(false);
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
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Saving directory..."));

	saveDirectory();
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
	updateGuiControls();
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
#if KDE_VERSION < 0x30200
/**
 * Turn tool bar on or off.
 */
void Kid3App::slotViewToolBar()
{
	slotStatusMsg(i18n("Toggling toolbar..."));
	if(!m_viewToolBar->isChecked()) {
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
	if(!m_viewStatusBar->isChecked()) {
		statusBar()->hide();
	}
	else {
		statusBar()->show();
	}
	slotStatusMsg(i18n("Ready."));
}
#else

void Kid3App::slotViewToolBar() {}
void Kid3App::slotViewStatusBar() {}

#endif

/**
 * Shortcuts configuration.
 */
void Kid3App::slotSettingsShortcuts()
{
#if KDE_VERSION >= 0x035c00
	KShortcutsDialog::configure(
		actionCollection(),
		KShortcutsEditor::LetterShortcutsDisallowed, this);
#else
	KKeyDialog::configure(actionCollection(), this);
#endif
}

/**
 * Toolbars configuration.
 */
void Kid3App::slotSettingsToolbars()
{
#if KDE_VERSION >= 0x035c00
	KEditToolBar dlg(actionCollection());
#else
	KEditToolbar dlg(actionCollection());
#endif
	if (dlg.exec()) {
		createGUI();
	}
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void Kid3App::displayHelp(const QString& anchor)
{
#if KDE_VERSION >= 0x035c00
	KToolInvocation::invokeHelp(anchor);
#else
	kapp->invokeHelp(anchor, QString::null, "");
#endif
}

void Kid3App::slotHelpHandbook() {}
void Kid3App::slotHelpAbout() {}
void Kid3App::slotHelpAboutQt() {}

#else /* CONFIG_USE_KDE */

void Kid3App::slotViewToolBar() {}
void Kid3App::slotViewStatusBar() {}
void Kid3App::slotSettingsShortcuts() {}
void Kid3App::slotSettingsToolbars() {}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void Kid3App::displayHelp(const QString& anchor)
{
	if (!s_helpBrowser) {
		QString caption(i18n("Kid3 Handbook"));
		s_helpBrowser =
			new BrowserDialog(NULL, caption);
	}
	if (s_helpBrowser) { 
		s_helpBrowser->goToAnchor(anchor);
		s_helpBrowser->setModal(!anchor.isEmpty());
		if (s_helpBrowser->isHidden()) {
			s_helpBrowser->show();
		}
	}
}

/**
 * Display handbook.
 */
void Kid3App::slotHelpHandbook()
{
	displayHelp();
}

/**
 * Display "About" dialog.
 */
void Kid3App::slotHelpAbout()
{
	QMessageBox::about(
		(Kid3App*)parent(), "Kid3",
		"Kid3 " VERSION
		"\n(c) 2003-2007 Urs Fleisch\nufleisch@users.sourceforge.net");
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
void Kid3App::slotStatusMsg(const QString& text)
{
	statusBar()->QCM_showMessage(text);
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
void Kid3App::slotCreatePlaylist()
{
	FileListItem* mp3file = m_view->firstFileInDir();
	if (!(mp3file && mp3file->getFile())) return;
	QDir dir(mp3file->getFile()->getDirname());
	QString dirname = dir.QCM_absolutePath();
	QString fn = dirname + QDir::separator() + dir.dirName() + ".m3u";
	QFile file(fn);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Creating playlist..."));
	if (file.open(QCM_WriteOnly)) {
		QTextStream stream(&file);
		while (mp3file != 0) {
			stream << mp3file->getFile()->getFilename() << "\n";
			mp3file = m_view->nextFileInDir();
		}
		file.close();
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
}

/**
 * Update track data and create import dialog.
 */
void Kid3App::setupImportDialog()
{
	m_trackDataList.clear();
	FileListItem* mp3file = m_view->firstFileInDir();
	bool firstTrack = true;
	while (mp3file != 0) {
		mp3file->getFile()->readTags(false);
		if (firstTrack) {
			StandardTags st;
			mp3file->getFile()->getStandardTagsV2(&st);
			if (st.artist.isEmpty() && st.album.isEmpty()) {
				mp3file->getFile()->getStandardTagsV1(&st);
			}
			m_trackDataList.m_artist = st.artist;
			m_trackDataList.m_album = st.album;
			firstTrack = false;
		}
		m_trackDataList.push_back(ImportTrackData(mp3file->getFile()->getAbsFilename(),
                                            mp3file->getFile()->getDuration()));
		mp3file = m_view->nextFileInDir();
	}

	if (!m_importDialog) {
		QString caption(i18n("Import"));
		m_importDialog =
			new ImportDialog(NULL, caption, m_trackDataList);
	}
	if (m_importDialog) {
		m_importDialog->clear();
	}
}

/**
 * Execute the import dialog.
 */
void Kid3App::execImportDialog()
{
	if (m_importDialog &&
			m_importDialog->exec() == QDialog::Accepted) {
		slotStatusMsg(i18n("Import..."));
		ImportTrackDataVector::const_iterator it = m_trackDataList.begin();
		StandardTags st1, st2;
		bool destV1 = m_importDialog->getDestination() == ImportConfig::DestV1 ||
		              m_importDialog->getDestination() == ImportConfig::DestV1V2;
		bool destV2 = m_importDialog->getDestination() == ImportConfig::DestV2 ||
		              m_importDialog->getDestination() == ImportConfig::DestV1V2;
		StandardTagsFilter flt(destV1 ?
													 m_view->getFilterFromID3V1() :
													 m_view->getFilterFromID3V2());
		bool no_selection = m_view->numFilesSelected() == 0;
		FileListItem* mp3file = m_view->firstFileInDir();
		while (mp3file != 0) {
			mp3file->getFile()->readTags(false);
			if (destV1) mp3file->getFile()->getStandardTagsV1(&st1);
			if (destV2) mp3file->getFile()->getStandardTagsV2(&st2);
			if (it != m_trackDataList.end()) {
				if (destV1) (*it).copyActiveTags(st1);
				if (destV2) (*it).copyActiveTags(st2);
				++it;
			} else {
				break;
			}
			if (destV1) {
				formatStandardTagsIfEnabled(&st1);
				mp3file->getFile()->setStandardTagsV1(&st1, flt);
			}
			if (destV2) {
				formatStandardTagsIfEnabled(&st2);
				mp3file->getFile()->setStandardTagsV2(&st2, flt);
			}
			mp3file = m_view->nextFileInDir();
		}
		if (!no_selection) {
			m_view->frameTableV1()->frames().clear();
			m_view->frameTableV1()->framesToTable();
			m_view->frameTableV2()->frames().clear();
			m_view->frameTableV2()->framesToTable();
			m_view->setFilenameEditEnabled(false);
			fileSelected();
		}
		else {
			updateModificationState();
		}
		slotStatusMsg(i18n("Ready."));
		QApplication::restoreOverrideCursor();
	}
}

/**
 * Import.
 */
void Kid3App::slotImport()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_None);
		execImportDialog();
	}
}

/**
 * Import from freedb.org.
 */
void Kid3App::slotImportFreedb()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Freedb);
		execImportDialog();
	}
}

/**
 * Import from TrackType.org.
 */
void Kid3App::slotImportTrackType()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_TrackType);
		execImportDialog();
	}
}

/**
 * Import from Discogs.
 */
void Kid3App::slotImportDiscogs()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Discogs);
		execImportDialog();
	}
}

/**
 * Import from MusicBrainz release database.
 */
void Kid3App::slotImportMusicBrainzRelease()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_MusicBrainzRelease);
		execImportDialog();
	}
}

/**
 * Import from MusicBrainz.
 */
void Kid3App::slotImportMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_MusicBrainz);
		execImportDialog();
	}
#endif
}

/**
 * Set data to be exported.
 *
 * @param src ExportDialog::SrcV1 to export ID3v1,
 *            ExportDialog::SrcV2 to export ID3v2
 */
void Kid3App::setExportData(int src)
{
	if (m_exportDialog) {
		ImportTrackDataVector trackDataVector;
		FileListItem* mp3file = m_view->firstFileInDir();
		bool firstTrack = true;
		while (mp3file != 0) {
			mp3file->getFile()->readTags(false);
			ImportTrackData trackData(mp3file->getFile()->getAbsFilename(),
																mp3file->getFile()->getDuration());
			if (src == ExportDialog::SrcV1) {
				mp3file->getFile()->getStandardTagsV1(&trackData);
			} else {
				mp3file->getFile()->getStandardTagsV2(&trackData);
			}
			if (firstTrack) {
				trackDataVector.m_artist = trackData.artist;
				trackDataVector.m_album = trackData.album;
				firstTrack = false;
			}
			trackDataVector.push_back(trackData);
			mp3file = m_view->nextFileInDir();
		}
		m_exportDialog->setExportData(trackDataVector);
	}
}

/**
 * Export.
 */
void Kid3App::slotExport()
{
	m_exportDialog = new ExportDialog(0);
	if (m_exportDialog) {
		m_exportDialog->readConfig();
		setExportData(s_genCfg.m_exportSrcV1 ?
									ExportDialog::SrcV1 : ExportDialog::SrcV2);
		connect(m_exportDialog, SIGNAL(exportDataRequested(int)),
						this, SLOT(setExportData(int)));
		m_exportDialog->exec();
		delete m_exportDialog;
		m_exportDialog = 0;
	}
}

/**
 * Show or hide the ID3V1.1 controls according to the settings and
 * set the menu entries appropriately.
 */
void Kid3App::updateHideV1()
{
	m_view->hideV1(s_miscCfg.m_hideV1);
	if (s_miscCfg.m_hideV1) {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV1->setText(i18n("Show Tag &1"));
#else
		m_settingsShowHideV1->setStatusTip(i18n("Show Tag 1"));
		m_settingsShowHideV1->QCM_setMenuText(i18n("Show Tag &1"));
#endif

	} else {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV1->setText(i18n("Hide Tag &1"));
#else
		m_settingsShowHideV1->setStatusTip(i18n("Hide Tag 1"));
		m_settingsShowHideV1->QCM_setMenuText(i18n("Hide Tag &1"));
#endif
	}
#if QT_VERSION >= 0x040000
	m_view->adjustRightHalfBoxSize();
#endif
}

/**
 * Show or hide the ID3V2.3 controls according to the settings and
 * set the menu entries appropriately.
 */
void Kid3App::updateHideV2()
{
	m_view->hideV2(s_miscCfg.m_hideV2);
	if (s_miscCfg.m_hideV2) {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV2->setText(i18n("Show Tag &2"));
#else
		m_settingsShowHideV2->setStatusTip(i18n("Show Tag 2"));
		m_settingsShowHideV2->QCM_setMenuText(i18n("Show Tag &2"));
#endif

	} else {
#ifdef CONFIG_USE_KDE
		m_settingsShowHideV2->setText(i18n("Hide Tag &2"));
#else
		m_settingsShowHideV2->setStatusTip(i18n("Hide Tag 2"));
		m_settingsShowHideV2->QCM_setMenuText(i18n("Hide Tag &2"));
#endif
	}
#if QT_VERSION >= 0x040000
	m_view->adjustRightHalfBoxSize();
#endif
}

/**
 * Show or hide ID3v1.1 controls.
 */
void Kid3App::slotSettingsShowHideV1()
{
	s_miscCfg.m_hideV1 = !s_miscCfg.m_hideV1;
	updateHideV1();
}

/**
 * Show or hide ID3v2.3 controls.
 */
void Kid3App::slotSettingsShowHideV2()
{
	s_miscCfg.m_hideV2 = !s_miscCfg.m_hideV2;
	updateHideV2();
}

/**
 * Preferences.
 */
void Kid3App::slotSettingsConfigure()
{
	QString caption(i18n("Configure - Kid3"));
#ifdef KID3_USE_KCONFIGDIALOG
	KConfigSkeleton* configSkeleton = new KConfigSkeleton;
	ConfigDialog* dialog =
		new ConfigDialog(NULL, caption, configSkeleton);
#else
	ConfigDialog* dialog =
		new ConfigDialog(NULL, caption);
#endif
	if (dialog) {
		dialog->setConfig(&s_fnFormatCfg, &s_id3FormatCfg, &s_miscCfg);
		if (dialog->exec() == QDialog::Accepted) {
			dialog->getConfig(&s_fnFormatCfg, &s_id3FormatCfg, &s_miscCfg);
			s_fnFormatCfg.writeToConfig(m_config);
			s_id3FormatCfg.writeToConfig(m_config);
			s_miscCfg.writeToConfig(m_config);
#ifdef CONFIG_USE_KDE
			m_config->sync();
#endif
			if (!s_miscCfg.m_markTruncations) {
				m_view->frameTableV1()->markRows(0);
			}
		}
	}
#ifdef KID3_USE_KCONFIGDIALOG
	delete configSkeleton;
#endif
}

/**
 * Apply filename format.
 */
void Kid3App::slotApplyFilenameFormat()
{
	StandardTags st;
	if (m_view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(false);
			QString str;
			str = mp3file->getFile()->getFilename();
			s_fnFormatCfg.formatString(str);
			mp3file->getFile()->setFilename(str);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Apply ID3 format.
 */
void Kid3App::slotApplyId3Format()
{
	StandardTags st;
	if (m_view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	StandardTagsFilter fltV1(m_view->getFilterFromID3V1());
	StandardTagsFilter fltV2(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			mp3file->getFile()->readTags(false);
			mp3file->getFile()->getStandardTagsV1(&st);
			s_id3FormatCfg.formatStandardTags(st);
			mp3file->getFile()->setStandardTagsV1(&st, fltV1);
			mp3file->getFile()->getStandardTagsV2(&st);
			s_id3FormatCfg.formatStandardTags(st);
			mp3file->getFile()->setStandardTagsV2(&st, fltV2);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Rename directory.
 */
void Kid3App::slotRenameDirectory()
{
	if (saveModified() && m_view->firstFileInDir()) {
		QString caption(i18n("Rename Directory"));
		RenDirDialog* dialog =
			new RenDirDialog(NULL, caption, m_view->firstFileInDir()->getFile());
		if (dialog) {
			if (dialog->exec() == QDialog::Accepted) {
				FileListItem* mp3file = m_view->firstFileInDir();
				QString errorMsg;
				bool again = false;
				while (mp3file &&
					   dialog->performAction(mp3file->getFile(), again, &errorMsg)) {
					mp3file = m_view->nextFileInDir();
				}
				openDirectory(dialog->getNewDirname());
				if (again) {
					mp3file = m_view->firstFileInDir();
					while (mp3file &&
						   dialog->performAction(mp3file->getFile(), again, &errorMsg)) {
						mp3file = m_view->nextFileInDir();
					}
					openDirectory(dialog->getNewDirname());
				}
				if (!errorMsg.isEmpty()) {
					QMessageBox::warning(0, i18n("File Error"),
										 i18n("Error while renaming:\n") +
										 errorMsg,
										 QMessageBox::Ok, QCM_NoButton);
				}
			}
		}
	}
}

/**
 * Number tracks.
 */
void Kid3App::slotNumberTracks()
{
	if (!m_numberTracksDialog) {
		m_numberTracksDialog = new NumberTracksDialog(0);
	}
	if (m_numberTracksDialog) {
		if (m_numberTracksDialog->exec() == QDialog::Accepted) {
			int nr = m_numberTracksDialog->getStartNumber();
			bool destV1 =
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV1 ||
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV1V2;
			bool destV2 =
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV2 ||
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV1V2;

			updateCurrentSelection();
			FileListItem* mp3file = m_view->firstFileInDir();
			bool no_selection = m_view->numFilesSelected() == 0;
			while (mp3file != 0) {
				if (no_selection || mp3file->isInSelection()) {
					mp3file->getFile()->readTags(false);
					if (destV1) {
						int oldnr = mp3file->getFile()->getTrackNumV1();
						if (nr != oldnr) {
							mp3file->getFile()->setTrackNumV1(nr);
						}
					}
					if (destV2) {
						int oldnr = mp3file->getFile()->getTrackNumV2();
						if (nr != oldnr) {
							mp3file->getFile()->setTrackNumV2(nr);
						}
					}
					++nr;
				}
				mp3file = m_view->nextFileInDir();
			}
			updateGuiControls();
		}
	}
}

/**
 * Convert ID3v2.3 to ID3v2.4 tags.
 */
void Kid3App::slotConvertToId3v24()
{
#ifdef HAVE_TAGLIB
	if (m_view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	FileListItem* item = m_view->firstFile();
	while (item != 0) {
		TaggedFile* taggedFile;
		if (item->isInSelection() &&
				(taggedFile = item->getFile()) != 0) {
			taggedFile->readTags(false);
			if (taggedFile->hasTagV2() && !taggedFile->isChanged()) {
				QString tagFmt = taggedFile->getTagFormatV2();
				if (tagFmt.length() >= 7 && tagFmt.startsWith("ID3v2.") && tagFmt[6] < '4') {
#ifdef HAVE_ID3LIB
					if (dynamic_cast<Mp3File*>(taggedFile) != 0) {
						// The file has to be read with TagLib to write ID3v2.4 tags
						TagLibFile* tagLibFile;
						if ((tagLibFile = new TagLibFile(
									 taggedFile->getDirInfo(),
									 taggedFile->getFilename())) != 0) {
							item->setFile(tagLibFile);
							taggedFile = tagLibFile;
							taggedFile->readTags(false);
						}
					}
#endif
					// Write the file with TagLib, it always writes ID3v2.4 tags
					bool renamed;
					taggedFile->writeTags(true, &renamed, s_miscCfg.m_preserveTime);
				}
			}
		}
		item = m_view->nextFile();
	}
	updateGuiControls();
#endif
}

/**
 * Convert ID3v2.4 to ID3v2.3 tags.
 */
void Kid3App::slotConvertToId3v23()
{
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	if (m_view->numFilesSelected() == 1) {
		updateCurrentSelection();
	}
	FileListItem* item = m_view->firstFile();
	while (item != 0) {
		TaggedFile* taggedFile;
		if (item->isInSelection() &&
				(taggedFile = item->getFile()) != 0) {
			taggedFile->readTags(false);
			if (taggedFile->hasTagV2() && !taggedFile->isChanged()) {
				QString tagFmt = taggedFile->getTagFormatV2();
				if (tagFmt.length() >= 7 && tagFmt.startsWith("ID3v2.") && tagFmt[6] > '3') {
					if (dynamic_cast<TagLibFile*>(taggedFile) != 0) {
						// Read the standard ID3v2.4 tags, other frames will be discarded!
						StandardTags st;
						taggedFile->getStandardTagsV2(&st);
						StandardTagsFilter flt;
						flt.setAllTrue();
						taggedFile->removeTagsV2(flt);

						// The file has to be read with id3lib to write ID3v2.3 tags
						Mp3File* id3libFile;
						if ((id3libFile = new Mp3File(
									 taggedFile->getDirInfo(),
									 taggedFile->getFilename())) != 0) {
							item->setFile(id3libFile);
							taggedFile = id3libFile;
							taggedFile->readTags(false);
						}

						// Restore the standard tags
						taggedFile->setStandardTagsV2(&st, flt);
					}

					// Write the file with id3lib, it always writes ID3v2.3 tags
					bool renamed;
					taggedFile->writeTags(true, &renamed, s_miscCfg.m_preserveTime);
					taggedFile->readTags(true);
				}
			}
		}
		item = m_view->nextFile();
	}
	updateGuiControls();
#endif
}

/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */
void Kid3App::openDrop(QString txt)
{
	int lfPos = txt.QCM_indexOf('\n');
	if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
		txt.truncate(lfPos + 1);
	}
	QUrl url(txt);
	if (!url.path().isEmpty()) {
		QString dir = url.path().QCM_trimmed();
#if defined _WIN32 || defined WIN32
		// There seems to be problems with filenames on Win32,
		// so correct
		if (dir[0] == '/' && dir[1] == '/' && dir[3] == '|') {
			dir[3] = ':';
			dir.remove(0, 2);
		} else if (dir[0] == '/' && dir[2] == ':') {
			dir.remove(0, 1);
		}
#endif
		updateCurrentSelection();
		openDirectory(dir, true);
	}
}

/**
 * Set tags in file to tags in GUI controls.
 *
 * @param mp3file file
 */
void Kid3App::updateTags(TaggedFile* mp3file)
{
	m_view->frameTableV1()->tableToFrames();
	mp3file->setFramesV1(m_view->frameTableV1()->frames());
	m_view->frameTableV2()->tableToFrames();
	mp3file->setFramesV2(m_view->frameTableV2()->frames());
	if (m_view->isFilenameEditEnabled()) {
		mp3file->setFilename(m_view->getFilename());
	}
}

/**
 * Update modification state, caption and listbox entries.
 */
void Kid3App::updateModificationState()
{
	setModified(m_view->updateModificationState());
#ifdef CONFIG_USE_KDE
	setCaption(s_dirName, isModified());
#else
	QString cap(s_dirName);
	if (isModified()) {
		cap += i18n(" [modified]");
	}
	if (!cap.isEmpty()) {
		cap += " - ";
	}
	cap += "Kid3";
	QCM_setWindowTitle(cap);
#endif
}

/**
 * Update files of current selection.
 */
void Kid3App::updateCurrentSelection()
{
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			updateTags(mp3file->getFile());
		}
		mp3file = m_view->nextFile();
	}
	updateModificationState();
}

/**
 * Update GUI controls from the tags in the files.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::updateGuiControls()
{
	FileListItem* mp3file = m_view->firstFile();
	FileListItem* singleItem = 0;
	TaggedFile* single_v2_file = 0;
	TaggedFile* firstMp3File = 0;
	int num_files_selected = 0;
	bool tagV1Supported = false;

	while (mp3file != 0) {
		if (mp3file->isSelected()) {
			mp3file->setInSelection(true);
			TaggedFile* taggedFile = mp3file->getFile();
			if (taggedFile) {
				taggedFile->readTags(false);

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				if (dynamic_cast<Mp3File*>(taggedFile) != 0 &&
						!taggedFile->isChanged() &&
						taggedFile->isTagInformationRead() && taggedFile->hasTagV2() &&
						taggedFile->getTagFormatV2() == QString::null) {
					TagLibFile* tagLibFile;
					if ((tagLibFile = new TagLibFile(
								 taggedFile->getDirInfo(),
								 taggedFile->getFilename())) != 0) {
						mp3file->setFile(tagLibFile);
						taggedFile = tagLibFile;
						taggedFile->readTags(false);
					}
				}
#endif

				if (num_files_selected == 0) {
					taggedFile->getAllFramesV1(m_view->frameTableV1()->frames());
					taggedFile->getAllFramesV2(m_view->frameTableV2()->frames());
					single_v2_file = taggedFile;
					singleItem = mp3file;
					firstMp3File = taggedFile;
				}
				else {
					FrameCollection fileFrames;
					taggedFile->getAllFramesV1(fileFrames);
					m_view->frameTableV1()->frames().filterDifferent(fileFrames);
					fileFrames.clear();
					taggedFile->getAllFramesV2(fileFrames);
					m_view->frameTableV2()->frames().filterDifferent(fileFrames);
					single_v2_file = 0;
					singleItem = 0;
				}
				++num_files_selected;

				if (taggedFile->isTagV1Supported()) {
					tagV1Supported = true;
				}
			}
		}
		else {
			mp3file->setInSelection(false);
		}
		mp3file = m_view->nextFile();
	}

	if (single_v2_file) {
		m_framelist->setTags(single_v2_file);
		m_view->setFilenameEditEnabled(true);
		m_view->setFilename(single_v2_file->getFilename());
		m_view->setDetailInfo(single_v2_file->getDetailInfo());
		m_view->setTagFormatV1(single_v2_file->getTagFormatV1());
		m_view->setTagFormatV2(single_v2_file->getTagFormatV2());

		if (s_miscCfg.m_markTruncations) {
			m_view->frameTableV1()->markRows(single_v2_file->getTruncationFlags());
		}
	}
	else {
		m_view->setFilenameEditEnabled(false);
		m_view->setDetailInfo("");
		m_view->setTagFormatV1(QString::null);
		m_view->setTagFormatV2(QString::null);

		if (s_miscCfg.m_markTruncations) {
			m_view->frameTableV1()->markRows(0);
		}
	}
	m_view->frameTableV1()->setAllCheckBoxes(num_files_selected == 1);
	m_view->frameTableV1()->framesToTable();
	m_view->frameTableV2()->setAllCheckBoxes(num_files_selected == 1);
	m_view->frameTableV2()->framesToTable();
	updateModificationState();

	if (num_files_selected == 0) {
		tagV1Supported = true;
	}
	m_view->enableControlsV1(tagV1Supported);
}

/**
 * Process change of selection.
 * The files of the current selection are updated.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::fileSelected()
{
	updateCurrentSelection();
	updateGuiControls();
}

/**
 * Copy tags 1 into copy buffer.
 */
void Kid3App::copyTagsV1()
{
	m_copyTags = m_view->frameTableV1()->frames().copyEnabledFrames(
		m_view->frameTableV1()->getEnabledFrameFilter());
}

/**
 * Copy tags 2 into copy buffer.
 */
void Kid3App::copyTagsV2()
{
	m_copyTags = m_view->frameTableV2()->frames().copyEnabledFrames(
		m_view->frameTableV2()->getEnabledFrameFilter());
}

/**
 * Paste from copy buffer to ID3v1 tags.
 */
void Kid3App::pasteTagsV1()
{
	updateCurrentSelection();
	FrameCollection frames(m_copyTags.copyEnabledFrames(
													 m_view->frameTableV1()->getEnabledFrameFilter()));
	formatFramesIfEnabled(frames);
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->setFramesV1(frames, false);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Paste from copy buffer to ID3v2 tags.
 */
void Kid3App::pasteTagsV2()
{
	updateCurrentSelection();
	FrameCollection frames(m_copyTags.copyEnabledFrames(
													 m_view->frameTableV2()->getEnabledFrameFilter()));
	formatFramesIfEnabled(frames);
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->setFramesV2(frames, false);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set ID3v1 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */
void Kid3App::getTagsFromFilenameV1()
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && m_view->isFilenameEditEnabled()) {
				mp3file->getFile()->setFilename(
					m_view->getFilename());
			}
			mp3file->getFile()->getStandardTagsV1(&st);
			mp3file->getFile()->getTagsFromFilename(&st, m_view->getFilenameFormat());
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Set ID3v2 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */
void Kid3App::getTagsFromFilenameV2()
{
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && m_view->isFilenameEditEnabled()) {
				mp3file->getFile()->setFilename(
					m_view->getFilename());
			}
			mp3file->getFile()->getStandardTagsV2(&st);
			mp3file->getFile()->getTagsFromFilename(&st, m_view->getFilenameFormat());
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
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
	updateCurrentSelection();
	StandardTags st;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (tag_version == 2) {
				mp3file->getFile()->getStandardTagsV2(&st);
			}
			else {
				mp3file->getFile()->getStandardTagsV1(&st);
			}
			if (!st.isEmptyOrInactive()) {
				mp3file->getFile()->getFilenameFromTags(
					&st, m_view->getFilenameFormat());
				formatFileNameIfEnabled(mp3file->getFile());
				if (!multiselect) {
					m_view->setFilename(
						mp3file->getFile()->getFilename());
				}
			}
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Copy ID3v1 tags to ID3v2 tags of selected files.
 */
void Kid3App::copyV1ToV2()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV1(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV2(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Copy ID3v2 tags to ID3v1 tags of selected files.
 */
void Kid3App::copyV2ToV1()
{
	updateCurrentSelection();
	StandardTags st;
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getStandardTagsV2(&st);
			formatStandardTagsIfEnabled(&st);
			mp3file->getFile()->setStandardTagsV1(&st, flt);
		}
		mp3file = m_view->nextFile();
	}
	// update controls with filtered data
	updateGuiControls();
}

/**
 * Remove ID3v1 tags in selected files.
 */
void Kid3App::removeTagsV1()
{
	updateCurrentSelection();
	StandardTagsFilter flt(m_view->getFilterFromID3V1());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->removeTagsV1(flt);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Remove ID3v2 tags in selected files.
 */
void Kid3App::removeTagsV2()
{
	updateCurrentSelection();
	StandardTagsFilter flt(m_view->getFilterFromID3V2());
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->removeTagsV2(flt);
		}
		mp3file = m_view->nextFile();
	}
	updateGuiControls();
}

/**
 * Update ID3v2 tags in GUI controls from file displayed in frame list.
 *
 * @param taggedFile the selected file
 */
void Kid3App::updateAfterFrameModification(TaggedFile* taggedFile)
{
	if (taggedFile) {
		taggedFile->getAllFramesV2(m_view->frameTableV2()->frames());
		m_view->frameTableV2()->framesToTable();
		updateModificationState();
	}
}

/**
 * Get the selected file.
 *
 * @return the selected file,
 *         0 if not exactly one file is selected
 */
TaggedFile* Kid3App::getSelectedFile()
{
	TaggedFile* taggedFile = 0;
	if (m_view->numFilesSelected() == 1) {
		FileListItem* mp3file = m_view->firstFile();
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				taggedFile = mp3file->getFile();
				break;
			}
			mp3file = m_view->nextFile();
		}
	}
	return taggedFile;
}

/**
 * Edit selected frame.
 */
void Kid3App::editFrame()
{
	updateCurrentSelection();
	TaggedFile* taggedFile = getSelectedFile();
	m_framelist->reloadTags();
	if (taggedFile && m_framelist->editFrame()) {
		updateAfterFrameModification(taggedFile);
	} else if (!taggedFile) {
		// multiple files selected
		FileListItem* mp3file = m_view->firstFile();
		bool firstFile = true;
		QString name;
		int frameId = -1;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				if (firstFile) {
					firstFile = false;
					taggedFile = mp3file->getFile();
					name = m_framelist->getSelectedName();
					if (!name.isEmpty() && m_framelist->editFrame()) {
						frameId = m_framelist->getSelectedId();
					} else {
						break;
					}
				} else {
					m_framelist->setTags(mp3file->getFile());
					if (m_framelist->selectByName(name) &&
							m_framelist->deleteFrame()) {
						m_framelist->pasteFrame();
					}
				}
			}
			mp3file = m_view->nextFile();
		}
		m_framelist->setTags(taggedFile);
		if (frameId != -1) {
			m_framelist->setSelectedId(frameId);
		}
		updateModificationState();
	}
}

/**
 * Delete selected frame.
 */
void Kid3App::deleteFrame()
{
	updateCurrentSelection();
	TaggedFile* taggedFile = getSelectedFile();
	m_framelist->reloadTags();
	if (taggedFile && m_framelist->deleteFrame()) {
		updateAfterFrameModification(taggedFile);
	} else if (!taggedFile) {
		// multiple files selected
		FileListItem* mp3file = m_view->firstFile();
		bool firstFile = true;
		QString name;
		int frameId = -1;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				if (firstFile) {
					firstFile = false;
					taggedFile = mp3file->getFile();
					name = m_framelist->getSelectedName();
					if (!name.isEmpty() && m_framelist->deleteFrame()) {
						frameId = m_framelist->getSelectedId();
					} else {
						break;
					}
				} else {
					m_framelist->setTags(mp3file->getFile());
					if (m_framelist->selectByName(name)) {
						m_framelist->deleteFrame();
					}
				}
			}
			mp3file = m_view->nextFile();
		}
		m_framelist->setTags(taggedFile);
		if (frameId != -1) {
			m_framelist->setSelectedId(frameId);
		}
		updateModificationState();
	}
}

/**
 * Select a frame type and add such a frame to frame list.
 */
void Kid3App::addFrame()
{
	updateCurrentSelection();
	TaggedFile* taggedFile = getSelectedFile();
	if (taggedFile &&
			m_framelist->selectFrame() &&
			m_framelist->addFrame(true)) {
		updateAfterFrameModification(taggedFile);
	} else if (!taggedFile) {
		// multiple files selected
		FileListItem* mp3file = m_view->firstFile();
		bool firstFile = true;
		int frameId = -1;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				if (firstFile) {
					firstFile = false;
					taggedFile = mp3file->getFile();
					if (m_framelist->selectFrame() &&
							m_framelist->addFrame(true)) {
						frameId = m_framelist->getSelectedId();
					} else {
						break;
					}
				} else {
					m_framelist->setTags(mp3file->getFile());
					m_framelist->pasteFrame();
				}
			}
			mp3file = m_view->nextFile();
		}
		m_framelist->setTags(taggedFile);
		if (frameId != -1) {
			m_framelist->setSelectedId(frameId);
		}
		updateModificationState();
	}
}

/**
 * Format a filename if format while editing is switched on.
 *
 * @param taggedFile file to modify
 */
void Kid3App::formatFileNameIfEnabled(TaggedFile* taggedFile) const
{
	if (s_fnFormatCfg.m_formatWhileEditing) {
		QString fn(taggedFile->getFilename());
		s_fnFormatCfg.formatString(fn);
		taggedFile->setFilename(fn);
	}
}

/**
 * Format tags if format while editing is switched on.
 *
 * @param st standard tags
 */
void Kid3App::formatStandardTagsIfEnabled(StandardTags* st) const
{
	if (s_id3FormatCfg.m_formatWhileEditing) {
		s_id3FormatCfg.formatStandardTags(*st);
	}
}

/**
 * Format frames if format while editing is switched on.
 *
 * @param frames frames
 */
void Kid3App::formatFramesIfEnabled(FrameCollection& frames) const
{
	if (s_id3FormatCfg.m_formatWhileEditing) {
		s_id3FormatCfg.formatFrames(frames);
	}
}
