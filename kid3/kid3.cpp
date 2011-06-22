/**
 * \file kid3.cpp
 * Kid3 application.
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

#include "kid3.h"
#include <QDir>
#include <QPrinter>
#include <QPainter>
#include <QUrl>
#include <QTextStream>
#include <QCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QProgressBar>
#include <QImage>
#include <QTextCodec>
#include "qtcompatmac.h"
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QIcon>
#include <QToolBar>
#include <QFileSystemModel>
#include "config.h"
#ifdef HAVE_QTDBUS
#include <QDBusConnection>
#include "scriptinterface.h"
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
#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kshortcutsdialog.h>
#include <krecentfilesaction.h>
#include <ktoolinvocation.h>
#include <kactioncollection.h>
#include <kedittoolbar.h>
#include <kconfigskeleton.h>
#else
#include <QApplication>
#include <QMenuBar>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#endif

#include "id3form.h"
#include "genres.h"
#include "framelist.h"
#include "frametablemodel.h"
#include "frametable.h"
#include "configdialog.h"
#include "importdialog.h"
#include "browsecoverartdialog.h"
#include "exportdialog.h"
#include "numbertracksdialog.h"
#include "filterdialog.h"
#include "rendirdialog.h"
#include "downloaddialog.h"
#include "playlistdialog.h"
#include "playlistcreator.h"
#include "fileproxymodel.h"
#include "dirproxymodel.h"
#include "modeliterator.h"
#include "trackdatamodel.h"
#include "dirlist.h"
#include "pictureframe.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.hpp"
#endif
#ifdef HAVE_FLAC
#include "flacfile.hpp"
#endif
#ifdef HAVE_MP4V2
#include "m4afile.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif
#ifdef HAVE_PHONON
#include "playtoolbar.h"
#endif

#ifndef CONFIG_USE_KDE
#include "recentfilesmenu.h"
#include "browserdialog.h"

BrowserDialog* Kid3App::s_helpBrowser = 0;
#endif

MiscConfig Kid3App::s_miscCfg("General Options");
ImportConfig Kid3App::s_genCfg("General Options");
FormatConfig Kid3App::s_fnFormatCfg("FilenameFormat");
FormatConfig Kid3App::s_id3FormatCfg("Id3Format");
FreedbConfig Kid3App::s_freedbCfg("Freedb");
FreedbConfig Kid3App::s_trackTypeCfg("TrackType");
DiscogsConfig Kid3App::s_discogsCfg("Discogs");
AmazonConfig Kid3App::s_amazonCfg("Amazon");
MusicBrainzConfig Kid3App::s_musicBrainzCfg("MusicBrainz");
FilterConfig Kid3App::s_filterCfg("Filter");
PlaylistConfig Kid3App::s_playlistCfg("Playlist");

/** Current directory */
QString Kid3App::s_dirName;

/**
 * Constructor.
 */
Kid3App::Kid3App() :
	m_fileSystemModel(new QFileSystemModel(this)),
	m_fileProxyModel(new FileProxyModel(this)),
	m_dirProxyModel(new DirProxyModel(this)),
	m_trackDataModel(new TrackDataModel(this)),
	m_downloadImageDest(ImageForSelectedFiles),
	m_importDialog(0), m_browseCoverArtDialog(0),
	m_exportDialog(0), m_renDirDialog(0),
	m_numberTracksDialog(0), m_filterDialog(0), m_downloadDialog(0),
	m_playlistDialog(0)
#ifdef HAVE_PHONON
	, m_playToolBar(0)
#endif
{
	m_fileSystemModel->setFilter(QDir::AllEntries | QDir::AllDirs);
	m_fileProxyModel->setSourceModel(m_fileSystemModel);
	m_dirProxyModel->setSourceModel(m_fileSystemModel);
#ifdef CONFIG_USE_KDE
	m_config = new KConfig;
#else
	m_config = new Kid3Settings(QSettings::UserScope, "kid3.sourceforge.net", "Kid3");
	m_config->beginGroup("/kid3");
#if !defined _WIN32 && !defined WIN32 && defined CFG_DATAROOTDIR
	QPixmap icon;
	if (icon.load(QString(CFG_DATAROOTDIR) +
#ifndef __APPLE__
								"/icons/hicolor/48x48/apps/kid3-qt.png"
#else
								"/kid3.png"
#endif
				)) {
		setWindowIcon(icon);
	}
#endif
	readFontAndStyleOptions();
#endif
#ifdef HAVE_QTDBUS
	if (QDBusConnection::sessionBus().isConnected()) {
		QString serviceName("net.sourceforge.kid3");
		QDBusConnection::sessionBus().registerService(serviceName);
#ifndef CONFIG_USE_KDE
		serviceName += '-';
		serviceName += QString::number(::getpid());
		QDBusConnection::sessionBus().registerService(serviceName);
#endif
		new ScriptInterface(this);
		if (!QDBusConnection::sessionBus().registerObject("/Kid3", this)) {
			qWarning("Registering D-Bus object failed");
		}
	} else {
		qWarning("Cannot connect to the D-BUS session bus.");
	}
#endif

	initFileTypes();
	initStatusBar();
	setModified(false);
	setFiltered(false);
	initView();
	initActions();
	s_fnFormatCfg.setAsFilenameFormatter();

	resize(sizeHint());

	readOptions();
}

/**
 * Destructor.
 */
Kid3App::~Kid3App()
{
	delete m_importDialog;
	delete m_renDirDialog;
	delete m_numberTracksDialog;
	delete m_filterDialog;
	delete m_downloadDialog;
	delete m_browseCoverArtDialog;
	delete m_playlistDialog;
#ifndef CONFIG_USE_KDE
	delete s_helpBrowser;
	s_helpBrowser = 0;
#endif
#ifdef HAVE_PHONON
	delete m_playToolBar;
#endif
}

/**
 * Init menu and toolbar actions.
 */
void Kid3App::initActions()
{
#ifdef CONFIG_USE_KDE
	KAction* fileOpen = KStandardAction::open(
	    this, SLOT(slotFileOpen()), actionCollection());
	m_fileOpenRecent = KStandardAction::openRecent(
	    this,
			SLOT(slotFileOpenRecentUrl(const KUrl&)),
			actionCollection());
	KAction* fileRevert = KStandardAction::revert(
	    this, SLOT(slotFileRevert()), actionCollection());
	KAction* fileSave = KStandardAction::save(
	    this, SLOT(slotFileSave()), actionCollection());
	KAction* fileQuit = KStandardAction::quit(
	    this, SLOT(slotFileQuit()), actionCollection());
	KAction* editSelectAll = KStandardAction::selectAll(
	    m_view, SLOT(selectAllFiles()), actionCollection());
	KAction* editDeselect = KStandardAction::deselect(
	    m_view, SLOT(deselectAllFiles()), actionCollection());
	setStandardToolBarMenuEnabled(true);
	createStandardStatusBarAction();
	KAction* settingsShortcuts = KStandardAction::keyBindings(
		this, SLOT(slotSettingsShortcuts()), actionCollection());
	KAction* settingsToolbars = KStandardAction::configureToolbars(
		this, SLOT(slotSettingsToolbars()), actionCollection());
	KAction* settingsConfigure = KStandardAction::preferences(
	    this, SLOT(slotSettingsConfigure()), actionCollection());

	fileOpen->setStatusTip(i18n("Opens a directory"));
	m_fileOpenRecent->setStatusTip(i18n("Opens a recently used directory"));
	fileRevert->setStatusTip(
	    i18n("Reverts the changes of all or the selected files"));
	fileSave->setStatusTip(i18n("Saves the changed files"));
	fileQuit->setStatusTip(i18n("Quits the application"));
	editSelectAll->setStatusTip(i18n("Select all files"));
	editSelectAll->setShortcut(KShortcut("Alt+Shift+A"));
	editDeselect->setStatusTip(i18n("Deselect all files"));
	settingsShortcuts->setStatusTip(i18n("Configure Shortcuts"));
	settingsToolbars->setStatusTip(i18n("Configure Toolbars"));
	settingsConfigure->setStatusTip(i18n("Preferences dialog"));

	KAction* fileOpenDirectory = new KAction(KIcon("document-open"), i18n("O&pen Directory..."), this);
	fileOpenDirectory->setShortcut(KShortcut("Ctrl+D"));
	actionCollection()->addAction("open_directory", fileOpenDirectory);
	connect(fileOpenDirectory, SIGNAL(triggered()), this, SLOT(slotFileOpenDirectory()));
	KAction* fileImport = new KAction(KIcon("document-import"), i18n("&Import..."), this);
	actionCollection()->addAction("import", fileImport);
	connect(fileImport, SIGNAL(triggered()), this, SLOT(slotImport()));
	KAction* fileImportFreedb = new KAction(i18n("Import from &gnudb.org..."), this);
	actionCollection()->addAction("import_freedb", fileImportFreedb);
	connect(fileImportFreedb, SIGNAL(triggered()), this, SLOT(slotImportFreedb()));
	KAction* fileImportTrackType = new KAction(i18n("Import from &TrackType.org..."), this);
	actionCollection()->addAction("import_tracktype", fileImportTrackType);
	connect(fileImportTrackType, SIGNAL(triggered()), this, SLOT(slotImportTrackType()));
	KAction* fileImportDiscogs = new KAction(i18n("Import from &Discogs..."), this);
	actionCollection()->addAction("import_discogs", fileImportDiscogs);
	connect(fileImportDiscogs, SIGNAL(triggered()), this, SLOT(slotImportDiscogs()));
	KAction* fileImportAmazon = new KAction(i18n("Import from &Amazon..."), this);
	actionCollection()->addAction("import_amazon", fileImportAmazon);
	connect(fileImportAmazon, SIGNAL(triggered()), this, SLOT(slotImportAmazon()));
	KAction* fileImportMusicBrainzRelease = new KAction(i18n("Import from MusicBrainz &Release..."), this);
	actionCollection()->addAction("import_musicbrainzrelease", fileImportMusicBrainzRelease);
	connect(fileImportMusicBrainzRelease, SIGNAL(triggered()), this, SLOT(slotImportMusicBrainzRelease()));
#ifdef HAVE_TUNEPIMP
	KAction* fileImportMusicBrainz = new KAction(i18n("Import from &MusicBrainz Fingerprint..."), this);
	actionCollection()->addAction("import_musicbrainz", fileImportMusicBrainz);
	connect(fileImportMusicBrainz, SIGNAL(triggered()), this, SLOT(slotImportMusicBrainz()));
#endif
	KAction* fileBrowseCoverArt = new KAction(i18n("&Browse Cover Art..."), this);
	actionCollection()->addAction("browse_cover_art", fileBrowseCoverArt);
	connect(fileBrowseCoverArt, SIGNAL(triggered()), this, SLOT(slotBrowseCoverArt()));
	KAction* fileExport = new KAction(KIcon("document-export"), i18n("&Export..."), this);
	actionCollection()->addAction("export", fileExport);
	connect(fileExport, SIGNAL(triggered()), this, SLOT(slotExport()));
	KAction* fileCreatePlaylist = new KAction(KIcon("view-media-playlist"), i18n("&Create Playlist..."), this);
	actionCollection()->addAction("create_playlist", fileCreatePlaylist);
	connect(fileCreatePlaylist, SIGNAL(triggered()), this, SLOT(slotPlaylistDialog()));
	KAction* toolsApplyFilenameFormat = new KAction(i18n("Apply &Filename Format"), this);
	actionCollection()->addAction("apply_filename_format", toolsApplyFilenameFormat);
	connect(toolsApplyFilenameFormat, SIGNAL(triggered()), this, SLOT(slotApplyFilenameFormat()));
	KAction* toolsApplyId3Format = new KAction(i18n("Apply &Tag Format"), this);
	actionCollection()->addAction("apply_id3_format", toolsApplyId3Format);
	connect(toolsApplyId3Format, SIGNAL(triggered()), this, SLOT(slotApplyId3Format()));
	KAction* toolsRenameDirectory = new KAction(i18n("&Rename Directory..."), this);
	actionCollection()->addAction("rename_directory", toolsRenameDirectory);
	connect(toolsRenameDirectory, SIGNAL(triggered()), this, SLOT(slotRenameDirectory()));
	KAction* toolsNumberTracks = new KAction(i18n("&Number Tracks..."), this);
	actionCollection()->addAction("number_tracks", toolsNumberTracks);
	connect(toolsNumberTracks, SIGNAL(triggered()), this, SLOT(slotNumberTracks()));
	KAction* toolsFilter = new KAction(i18n("F&ilter..."), this);
	actionCollection()->addAction("filter", toolsFilter);
	connect(toolsFilter, SIGNAL(triggered()), this, SLOT(slotFilter()));
#ifdef HAVE_TAGLIB
	KAction* toolsConvertToId3v24 = new KAction(i18n("Convert ID3v2.3 to ID3v2.&4"), this);
	actionCollection()->addAction("convert_to_id3v24", toolsConvertToId3v24);
	connect(toolsConvertToId3v24, SIGNAL(triggered()), this, SLOT(slotConvertToId3v24()));
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	KAction* toolsConvertToId3v23 = new KAction(i18n("Convert ID3v2.4 to ID3v2.&3"), this);
	actionCollection()->addAction("convert_to_id3v23", toolsConvertToId3v23);
	connect(toolsConvertToId3v23, SIGNAL(triggered()), this, SLOT(slotConvertToId3v23()));
#endif
#ifdef HAVE_PHONON
	KAction* toolsPlay = new KAction(KIcon("media-playback-start"), i18n("&Play"), this);
	actionCollection()->addAction("play", toolsPlay);
	connect(toolsPlay, SIGNAL(triggered()), this, SLOT(slotPlayAudio()));
#endif
	m_settingsShowHidePicture = new KToggleAction(i18n("Show &Picture"), this);
	m_settingsShowHidePicture->setCheckable(true);
	actionCollection()->addAction("hide_picture", m_settingsShowHidePicture);
	connect(m_settingsShowHidePicture, SIGNAL(triggered()), this, SLOT(slotSettingsShowHidePicture()));
	m_settingsAutoHideTags = new KToggleAction(i18n("Auto &Hide Tags"), this);
	m_settingsAutoHideTags->setCheckable(true);
	actionCollection()->addAction("auto_hide_tags", m_settingsAutoHideTags);
	connect(m_settingsAutoHideTags, SIGNAL(triggered()), this, SLOT(slotSettingsAutoHideTags()));
	KAction* editPreviousFile = new KAction(KIcon("go-previous"), i18n("&Previous File"), this);
	editPreviousFile->setShortcut(KShortcut("Alt+Up"));
	actionCollection()->addAction("previous_file", editPreviousFile);
	connect(editPreviousFile, SIGNAL(triggered()), m_view, SLOT(selectPreviousFile()));
	KAction* editNextFile = new KAction(KIcon("go-next"), i18n("&Next File"), this);
	editNextFile->setShortcut(KShortcut("Alt+Down"));
	actionCollection()->addAction("next_file", editNextFile);
	connect(editNextFile, SIGNAL(triggered()), m_view, SLOT(selectNextFile()));
	KAction* actionV1FromFilename = new KAction(i18n("Tag 1") + ": " + i18n("From Filename"), this);
	actionCollection()->addAction("v1_from_filename", actionV1FromFilename);
	connect(actionV1FromFilename, SIGNAL(triggered()), m_view, SLOT(fromFilenameV1()));
	KAction* actionV1FromV2 = new KAction(i18n("Tag 1") + ": " + i18n("From Tag 2"), this);
	actionCollection()->addAction("v1_from_v2", actionV1FromV2);
	connect(actionV1FromV2, SIGNAL(triggered()), m_view, SLOT(fromID3V1()));
	KAction* actionV1Copy = new KAction(i18n("Tag 1") + ": " + i18n("Copy"), this);
	actionCollection()->addAction("v1_copy", actionV1Copy);
	connect(actionV1Copy, SIGNAL(triggered()), m_view, SLOT(copyV1()));
	KAction* actionV1Paste = new KAction(i18n("Tag 1") + ": " + i18n("Paste"), this);
	actionCollection()->addAction("v1_paste", actionV1Paste);
	connect(actionV1Paste, SIGNAL(triggered()), m_view, SLOT(pasteV1()));
	KAction* actionV1Remove = new KAction(i18n("Tag 1") + ": " + i18n("Remove"), this);
	actionCollection()->addAction("v1_remove", actionV1Remove);
	connect(actionV1Remove, SIGNAL(triggered()), m_view, SLOT(removeV1()));
	KAction* actionV2FromFilename = new KAction(i18n("Tag 2") + ": " + i18n("From Filename"), this);
	actionCollection()->addAction("v2_from_filename", actionV2FromFilename);
	connect(actionV2FromFilename, SIGNAL(triggered()), m_view, SLOT(fromFilenameV2()));
	KAction* actionV2FromV1 = new KAction(i18n("Tag 2") + ": " + i18n("From Tag 1"), this);
	actionCollection()->addAction("v2_from_v1", actionV2FromV1);
	connect(actionV2FromV1, SIGNAL(triggered()), m_view, SLOT(fromID3V2()));
	KAction* actionV2Copy = new KAction(i18n("Tag 2") + ": " + i18n("Copy"), this);
	actionCollection()->addAction("v2_copy", actionV2Copy);
	connect(actionV2Copy, SIGNAL(triggered()), m_view, SLOT(copyV2()));
	KAction* actionV2Paste = new KAction(i18n("Tag 2") + ": " + i18n("Paste"), this);
	actionCollection()->addAction("v2_paste", actionV2Paste);
	connect(actionV2Paste, SIGNAL(triggered()), m_view, SLOT(pasteV2()));
	KAction* actionV2Remove = new KAction(i18n("Tag 2") + ": " + i18n("Remove"), this);
	actionCollection()->addAction("v2_remove", actionV2Remove);
	connect(actionV2Remove, SIGNAL(triggered()), m_view, SLOT(removeV2()));
	KAction* actionFramesEdit = new KAction(i18n("Frames:") + " " + i18n("Edit"), this);
	actionCollection()->addAction("frames_edit", actionFramesEdit);
	connect(actionFramesEdit, SIGNAL(triggered()), m_view, SLOT(editFrame()));
	KAction* actionFramesAdd = new KAction(i18n("Frames:") + " " + i18n("Add"), this);
	actionCollection()->addAction("frames_add", actionFramesAdd);
	connect(actionFramesAdd, SIGNAL(triggered()), m_view, SLOT(addFrame()));
	KAction* actionFramesDelete = new KAction(i18n("Frames:") + " " + i18n("Delete"), this);
	actionCollection()->addAction("frames_delete", actionFramesDelete);
	connect(actionFramesDelete, SIGNAL(triggered()), m_view, SLOT(deleteFrame()));
	KAction* actionFilenameFromV1 = new KAction(i18n("Filename") + ": " + i18n("From Tag 1"), this);
	actionCollection()->addAction("filename_from_v1", actionFilenameFromV1);
	connect(actionFilenameFromV1, SIGNAL(triggered()), m_view, SLOT(fnFromID3V1()));
	KAction* actionFilenameFromV2 = new KAction(i18n("Filename") + ": " + i18n("From Tag 2"), this);
	actionCollection()->addAction("filename_from_v2", actionFilenameFromV2);
	connect(actionFilenameFromV2, SIGNAL(triggered()), m_view, SLOT(fnFromID3V2()));
	KAction* actionFilenameFocus = new KAction(i18n("Filename") + ": " + i18n("Focus"), this);
	actionCollection()->addAction("filename_focus", actionFilenameFocus);
	connect(actionFilenameFocus, SIGNAL(triggered()), m_view, SLOT(setFocusFilename()));
	KAction* actionV1Focus = new KAction(i18n("Tag 1") + ": " + i18n("Focus"), this);
	actionCollection()->addAction("v1_focus", actionV1Focus);
	connect(actionV1Focus, SIGNAL(triggered()), m_view, SLOT(setFocusV1()));
	KAction* actionV2Focus = new KAction(i18n("Tag 2") + ": " + i18n("Focus"), this);
	actionCollection()->addAction("v2_focus", actionV2Focus);
	connect(actionV2Focus, SIGNAL(triggered()), m_view, SLOT(setFocusV2()));

	createGUI();

#else
	QAction* fileOpen = new QAction(this);
	if (fileOpen) {
		fileOpen->setStatusTip(i18n("Opens a directory"));
		fileOpen->setText(i18n("&Open..."));
		fileOpen->setShortcut(Qt::CTRL + Qt::Key_O);
		fileOpen->setIcon(QIcon(":/images/document-open.png"));
		connect(fileOpen, SIGNAL(triggered()),
			this, SLOT(slotFileOpen()));
	}
	QAction* fileOpenDirectory = new QAction(this);
	if (fileOpenDirectory) {
		fileOpenDirectory->setStatusTip(i18n("Opens a directory"));
		fileOpenDirectory->setText(i18n("O&pen Directory..."));
		fileOpenDirectory->setShortcut(Qt::CTRL + Qt::Key_D);
		fileOpenDirectory->setIcon(QIcon(":/images/document-open.png"));
		connect(fileOpenDirectory, SIGNAL(triggered()),
			this, SLOT(slotFileOpenDirectory()));
	}
	QAction* fileSave = new QAction(this);
	if (fileSave) {
		fileSave->setStatusTip(i18n("Saves the changed files"));
		fileSave->setText(i18n("&Save"));
		fileSave->setShortcut(Qt::CTRL + Qt::Key_S);
		fileSave->setIcon(QIcon(":/images/document-save.png"));
		connect(fileSave, SIGNAL(triggered()),
			this, SLOT(slotFileSave()));
	}
	QAction* fileRevert = new QAction(this);
	if (fileRevert) {
		fileRevert->setStatusTip(
		    i18n("Reverts the changes of all or the selected files"));
		fileRevert->setText(i18n("Re&vert"));
		fileRevert->setIcon(QIcon(":/images/document-revert.png"));
		connect(fileRevert, SIGNAL(triggered()),
			this, SLOT(slotFileRevert()));
	}
	QAction* fileImport = new QAction(this);
	if (fileImport) {
		fileImport->setStatusTip(i18n("Import from file or clipboard"));
		fileImport->setText(i18n("&Import..."));
		fileImport->setIcon(QIcon(":/images/document-import.png"));
		connect(fileImport, SIGNAL(triggered()),
			this, SLOT(slotImport()));
	}
	QAction* fileImportFreedb = new QAction(this);
	if (fileImportFreedb) {
		fileImportFreedb->setStatusTip(i18n("Import from gnudb.org"));
		fileImportFreedb->setText(i18n("Import from &gnudb.org..."));
		connect(fileImportFreedb, SIGNAL(triggered()),
			this, SLOT(slotImportFreedb()));
	}
	QAction* fileImportTrackType = new QAction(this);
	if (fileImportTrackType) {
		fileImportTrackType->setStatusTip(i18n("Import from TrackType.org"));
		fileImportTrackType->setText(i18n("Import from &TrackType.org..."));
		connect(fileImportTrackType, SIGNAL(triggered()),
			this, SLOT(slotImportTrackType()));
	}
	QAction* fileImportDiscogs = new QAction(this);
	if (fileImportDiscogs) {
		fileImportDiscogs->setStatusTip(i18n("Import from Discogs"));
		fileImportDiscogs->setText(i18n("Import from &Discogs..."));
		connect(fileImportDiscogs, SIGNAL(triggered()),
			this, SLOT(slotImportDiscogs()));
	}
	QAction* fileImportAmazon = new QAction(this);
	if (fileImportAmazon) {
		fileImportAmazon->setStatusTip(i18n("Import from Amazon"));
		fileImportAmazon->setText(i18n("Import from &Amazon..."));
		connect(fileImportAmazon, SIGNAL(triggered()),
			this, SLOT(slotImportAmazon()));
	}
	QAction* fileImportMusicBrainzRelease = new QAction(this);
	if (fileImportMusicBrainzRelease) {
		fileImportMusicBrainzRelease->setStatusTip(i18n("Import from MusicBrainz Release"));
		fileImportMusicBrainzRelease->setText(i18n("Import from MusicBrainz &Release..."));
		connect(fileImportMusicBrainzRelease, SIGNAL(triggered()),
			this, SLOT(slotImportMusicBrainzRelease()));
	}
#ifdef HAVE_TUNEPIMP
	QAction* fileImportMusicBrainz = new QAction(this);
	if (fileImportMusicBrainz) {
		fileImportMusicBrainz->setStatusTip(i18n("Import from MusicBrainz Fingerprint"));
		fileImportMusicBrainz->setText(i18n("Import from &MusicBrainz Fingerprint..."));
		connect(fileImportMusicBrainz, SIGNAL(triggered()),
			this, SLOT(slotImportMusicBrainz()));
	}
#endif
	QAction* fileBrowseCoverArt = new QAction(this);
	if (fileBrowseCoverArt) {
		fileBrowseCoverArt->setStatusTip(i18n("Browse album cover artwork"));
		fileBrowseCoverArt->setText(i18n("&Browse Cover Art..."));
		connect(fileBrowseCoverArt, SIGNAL(triggered()),
			this, SLOT(slotBrowseCoverArt()));
	}
	QAction* fileExport = new QAction(this);
	if (fileExport) {
		fileExport->setStatusTip(i18n("Export to file or clipboard"));
		fileExport->setText(i18n("&Export..."));
		fileExport->setIcon(QIcon(":/images/document-export.png"));
		connect(fileExport, SIGNAL(triggered()),
			this, SLOT(slotExport()));
	}
	QAction* fileCreatePlaylist = new QAction(this);
	if (fileCreatePlaylist) {
		fileCreatePlaylist->setStatusTip(i18n("Create M3U Playlist"));
		fileCreatePlaylist->setText(i18n("&Create Playlist..."));
		fileCreatePlaylist->setIcon(QIcon(":/images/view-media-playlist.png"));
		connect(fileCreatePlaylist, SIGNAL(triggered()),
			this, SLOT(slotPlaylistDialog()));
	}
	QAction* fileQuit = new QAction(this);
	if (fileQuit) {
		fileQuit->setStatusTip(i18n("Quits the application"));
		fileQuit->setText(i18n("&Quit"));
		fileQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
		fileQuit->setIcon(QIcon(":/images/application-exit.png"));
		connect(fileQuit, SIGNAL(triggered()),
			this, SLOT(slotFileQuit()));
	}
	QAction* editSelectAll = new QAction(this);
	if (editSelectAll) {
		editSelectAll->setStatusTip(i18n("Select all files"));
		editSelectAll->setText(i18n("Select &All"));
		editSelectAll->setShortcut(Qt::ALT + Qt::Key_A);
		editSelectAll->setIcon(QIcon(":/images/edit-select-all.png"));
		connect(editSelectAll, SIGNAL(triggered()),
			m_view, SLOT(selectAllFiles()));
	}
	QAction* editDeselect = new QAction(this);
	if (editDeselect) {
		editDeselect->setStatusTip(i18n("Deselect all files"));
		editDeselect->setText(i18n("Dese&lect"));
		editDeselect->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_A);
		connect(editDeselect, SIGNAL(triggered()),
			m_view, SLOT(deselectAllFiles()));
	}
	QAction* editPreviousFile = new QAction(this);
	if (editPreviousFile) {
		editPreviousFile->setStatusTip(i18n("Select previous file"));
		editPreviousFile->setText(i18n("&Previous File"));
		editPreviousFile->setShortcut(Qt::ALT + Qt::Key_Up);
		editPreviousFile->setIcon(QIcon(":/images/go-previous.png"));
		connect(editPreviousFile, SIGNAL(triggered()),
			m_view, SLOT(selectPreviousFile()));
	}
	QAction* editNextFile = new QAction(this);
	if (editNextFile) {
		editNextFile->setStatusTip(i18n("Select next file"));
		editNextFile->setText(i18n("&Next File"));
		editNextFile->setShortcut(Qt::ALT + Qt::Key_Down);
		editNextFile->setIcon(QIcon(":/images/go-next.png"));
		connect(editNextFile, SIGNAL(triggered()),
			m_view, SLOT(selectNextFile()));
	}
	QAction* helpHandbook = new QAction(this);
	if (helpHandbook) {
		helpHandbook->setStatusTip(i18n("Kid3 Handbook"));
		helpHandbook->setText(i18n("Kid3 &Handbook"));
		helpHandbook->setIcon(QIcon(":/images/help-contents.png"));
		connect(helpHandbook, SIGNAL(triggered()),
			this, SLOT(slotHelpHandbook()));
	}
	QAction* helpAbout = new QAction(this);
	if (helpAbout) {
		helpAbout->setStatusTip(i18n("About Kid3"));
		helpAbout->setText(i18n("&About Kid3"));
		connect(helpAbout, SIGNAL(triggered()),
			this, SLOT(slotHelpAbout()));
	}
	QAction* helpAboutQt = new QAction(this);
	if (helpAboutQt) {
		helpAboutQt->setStatusTip(i18n("About Qt"));
		helpAboutQt->setText(i18n("About &Qt"));
		connect(helpAboutQt, SIGNAL(triggered()),
			this, SLOT(slotHelpAboutQt()));
	}
	QAction* toolsApplyFilenameFormat = new QAction(this);
	if (toolsApplyFilenameFormat) {
		toolsApplyFilenameFormat->setStatusTip(i18n("Apply Filename Format"));
		toolsApplyFilenameFormat->setText(i18n("Apply &Filename Format"));
		connect(toolsApplyFilenameFormat, SIGNAL(triggered()),
			this, SLOT(slotApplyFilenameFormat()));
	}
	QAction* toolsApplyId3Format = new QAction(this);
	if (toolsApplyId3Format) {
		toolsApplyId3Format->setStatusTip(i18n("Apply Tag Format"));
		toolsApplyId3Format->setText(i18n("Apply &Tag Format"));
		connect(toolsApplyId3Format, SIGNAL(triggered()),
			this, SLOT(slotApplyId3Format()));
	}
	QAction* toolsRenameDirectory = new QAction(this);
	if (toolsRenameDirectory) {
		toolsRenameDirectory->setStatusTip(i18n("Rename Directory"));
		toolsRenameDirectory->setText(i18n("&Rename Directory..."));
		connect(toolsRenameDirectory, SIGNAL(triggered()),
			this, SLOT(slotRenameDirectory()));
	}
	QAction* toolsNumberTracks = new QAction(this);
	if (toolsNumberTracks) {
		toolsNumberTracks->setStatusTip(i18n("Number Tracks"));
		toolsNumberTracks->setText(i18n("&Number Tracks..."));
		connect(toolsNumberTracks, SIGNAL(triggered()),
			this, SLOT(slotNumberTracks()));
	}
	QAction* toolsFilter = new QAction(this);
	if (toolsFilter) {
		toolsFilter->setStatusTip(i18n("Filter"));
		toolsFilter->setText(i18n("F&ilter..."));
		connect(toolsFilter, SIGNAL(triggered()),
			this, SLOT(slotFilter()));
	}
#ifdef HAVE_TAGLIB
	QAction* toolsConvertToId3v24 = new QAction(this);
	if (toolsConvertToId3v24) {
		toolsConvertToId3v24->setStatusTip(i18n("Convert ID3v2.3 to ID3v2.4"));
		toolsConvertToId3v24->setText(i18n("Convert ID3v2.3 to ID3v2.&4"));
		connect(toolsConvertToId3v24, SIGNAL(triggered()),
			this, SLOT(slotConvertToId3v24()));
	}
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
	QAction* toolsConvertToId3v23 = new QAction(this);
	if (toolsConvertToId3v23) {
		toolsConvertToId3v23->setStatusTip(i18n("Convert ID3v2.4 to ID3v2.3"));
		toolsConvertToId3v23->setText(i18n("Convert ID3v2.4 to ID3v2.&3"));
		connect(toolsConvertToId3v23, SIGNAL(triggered()),
			this, SLOT(slotConvertToId3v23()));
	}
#endif
#ifdef HAVE_PHONON
	QAction* toolsPlay = new QAction(this);
	if (toolsPlay) {
		toolsPlay->setStatusTip(i18n("Play"));
		toolsPlay->setText(i18n("&Play"));
		toolsPlay->setIcon(QIcon(style()->standardIcon(QStyle::SP_MediaPlay)));
		connect(toolsPlay, SIGNAL(triggered()),
			this, SLOT(slotPlayAudio()));
	}
#endif
	m_viewStatusBar = new QAction(this);
	if (m_viewStatusBar) {
		m_viewStatusBar->setStatusTip(i18n("Enables/disables the statusbar"));
		m_viewStatusBar->setText(i18n("Show St&atusbar"));
		m_viewStatusBar->setCheckable(true);
		connect(m_viewStatusBar, SIGNAL(triggered()),
			this, SLOT(slotViewStatusBar()));
	}
	m_settingsShowHidePicture = new QAction(this);
	if (m_settingsShowHidePicture) {
		m_settingsShowHidePicture->setStatusTip(i18n("Show Picture"));
		m_settingsShowHidePicture->setText(i18n("Show &Picture"));
		m_settingsShowHidePicture->setCheckable(true);
		connect(m_settingsShowHidePicture, SIGNAL(triggered()),
			this, SLOT(slotSettingsShowHidePicture()));
	}
	m_settingsAutoHideTags = new QAction(this);
	if (m_settingsAutoHideTags) {
		m_settingsAutoHideTags->setStatusTip(i18n("Auto Hide Tags"));
		m_settingsAutoHideTags->setText(i18n("Auto &Hide Tags"));
		m_settingsAutoHideTags->setCheckable(true);
		connect(m_settingsAutoHideTags, SIGNAL(triggered()),
			this, SLOT(slotSettingsAutoHideTags()));
	}
	QAction* settingsConfigure = new QAction(this);
	if (settingsConfigure) {
		settingsConfigure->setStatusTip(i18n("Configure Kid3"));
		settingsConfigure->setText(i18n("&Configure Kid3..."));
		settingsConfigure->setIcon(QIcon(":/images/configure.png"));
		connect(settingsConfigure, SIGNAL(triggered()),
			this, SLOT(slotSettingsConfigure()));
	}
	QToolBar* toolBar = new QToolBar(this);
	toolBar->setObjectName("MainToolbar");
	toolBar->addAction(fileOpen);
	toolBar->addAction(fileSave);
	toolBar->addAction(fileRevert);
	toolBar->addAction(fileCreatePlaylist);
	toolBar->addAction(editPreviousFile);
	toolBar->addAction(editNextFile);
#ifdef HAVE_PHONON
	toolBar->addAction(toolsPlay);
#endif
	toolBar->addAction(settingsConfigure);
	addToolBar(toolBar);
	m_viewToolBar = toolBar->toggleViewAction();
	if (m_viewToolBar) {
		m_viewToolBar->setStatusTip(i18n("Enables/disables the toolbar"));
		m_viewToolBar->setText(i18n("Show &Toolbar"));
	}
	if (s_miscCfg.m_hideToolBar)
		toolBar->hide();
	m_viewToolBar->setChecked(!s_miscCfg.m_hideToolBar);

	QMenuBar* menubar = menuBar();
	QMenu* fileMenu = menubar->addMenu(i18n("&File"));
	QMenu* editMenu = menubar->addMenu(i18n("&Edit"));
	QMenu* toolsMenu = menubar->addMenu(i18n("&Tools"));
	QMenu* settingsMenu = menubar->addMenu(i18n("&Settings"));
	QMenu* helpMenu = menubar->addMenu(i18n("&Help"));
	if (fileMenu && editMenu && toolsMenu && settingsMenu && helpMenu) {
		fileMenu->addAction(fileOpen);
		m_fileOpenRecent = new RecentFilesMenu(fileMenu);
		if (m_fileOpenRecent) {
			connect(m_fileOpenRecent, SIGNAL(loadFile(const QString&)),
							this, SLOT(slotFileOpenRecentDirectory(const QString&)));
			m_fileOpenRecent->setStatusTip(i18n("Opens a recently used directory"));
			m_fileOpenRecent->setTitle(i18n("Open &Recent"));
			m_fileOpenRecent->setIcon(QIcon(":/images/document-open-recent.png"));
			fileMenu->addMenu(m_fileOpenRecent);
		}
		fileMenu->addAction(fileOpenDirectory);
		fileMenu->addSeparator();
		fileMenu->addAction(fileSave);
		fileMenu->addAction(fileRevert);
		fileMenu->addSeparator();
		fileMenu->addAction(fileImport);
		fileMenu->addAction(fileImportFreedb);
		fileMenu->addAction(fileImportTrackType);
		fileMenu->addAction(fileImportDiscogs);
		fileMenu->addAction(fileImportAmazon);
		fileMenu->addAction(fileImportMusicBrainzRelease);
#ifdef HAVE_TUNEPIMP
		fileMenu->addAction(fileImportMusicBrainz);
#endif
		fileMenu->addAction(fileBrowseCoverArt);
		fileMenu->addAction(fileExport);
		fileMenu->addAction(fileCreatePlaylist);
		fileMenu->addSeparator();
		fileMenu->addAction(fileQuit);

		editMenu->addAction(editSelectAll);
		editMenu->addAction(editDeselect);
		editMenu->addAction(editPreviousFile);
		editMenu->addAction(editNextFile);

		toolsMenu->addAction(toolsApplyFilenameFormat);
		toolsMenu->addAction(toolsApplyId3Format);
		toolsMenu->addAction(toolsRenameDirectory);
		toolsMenu->addAction(toolsNumberTracks);
		toolsMenu->addAction(toolsFilter);
#ifdef HAVE_TAGLIB
		toolsMenu->addAction(toolsConvertToId3v24);
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
		toolsMenu->addAction(toolsConvertToId3v23);
#endif
#ifdef HAVE_PHONON
		toolsMenu->addAction(toolsPlay);
#endif

		settingsMenu->addAction(m_viewToolBar);

		settingsMenu->addAction(m_viewStatusBar);
		settingsMenu->addAction(m_settingsShowHidePicture);
		settingsMenu->addAction(m_settingsAutoHideTags);
		settingsMenu->addSeparator();
		settingsMenu->addAction(settingsConfigure);

		helpMenu->addAction(helpHandbook);
		helpMenu->addAction(helpAbout);
		helpMenu->addAction(helpAboutQt);
	}
	updateWindowCaption();
#endif
}

/**
 * Init file types.
 */
void Kid3App::initFileTypes()
{
#ifdef HAVE_ID3LIB
	TaggedFile::addResolver(new Mp3File::Resolver);
#endif
#ifdef HAVE_VORBIS
	TaggedFile::addResolver(new OggFile::Resolver);
#endif
#ifdef HAVE_FLAC
	TaggedFile::addResolver(new FlacFile::Resolver);
#endif
#ifdef HAVE_MP4V2
	TaggedFile::addResolver(new M4aFile::Resolver);
#endif
#ifdef HAVE_TAGLIB
	TagLibFile::staticInit();
	TaggedFile::addResolver(new TagLibFile::Resolver);
#endif
}

/**
 * Init status bar.
 */
void Kid3App::initStatusBar()
{
	statusBar()->showMessage(i18n("Ready."));
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
	}
}

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
bool Kid3App::openDirectory(QString dir, bool confirm, bool fileCheck)
{
	if (confirm && !saveModified()) {
		return false;
	}
	if (dir.isEmpty()) {
		return false;
	}
	QFileInfo file(dir);
	QString filePath;
	if (!file.isDir()) {
		if (fileCheck && !file.isFile()) {
			return false;
		}
		dir = file.absolutePath();
		filePath = file.absoluteFilePath();
	} else {
		dir = QDir(dir).absolutePath();
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Opening directory..."));

	QStringList nameFilters(s_miscCfg.m_nameFilter.split(' '));
	m_fileProxyModel->setNameFilters(nameFilters);
	QModelIndex rootIndex = m_fileSystemModel->setRootPath(dir);
	QModelIndex fileIndex = m_fileSystemModel->index(filePath);
	bool ok = m_view->readFileList(rootIndex, fileIndex);
	if (ok) {
		m_view->readDirectoryList(rootIndex);
		setModified(false);
		setFiltered(false);
#ifdef CONFIG_USE_KDE
		KUrl url;
		url.setPath(dir);
		m_fileOpenRecent->addUrl(url);
#else
		m_fileOpenRecent->addDirectory(dir);
#endif
		s_dirName = dir;
		updateWindowCaption();
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
	return ok;
}

/**
 * Save application options.
 */
void Kid3App::saveOptions()
{
#ifdef CONFIG_USE_KDE
	m_fileOpenRecent->saveEntries(KConfigGroup(m_config, "Recent Files"));
#else
	m_fileOpenRecent->saveEntries(m_config);
	s_miscCfg.m_hideToolBar = !m_viewToolBar->isChecked();
	s_miscCfg.m_geometry = saveGeometry();
	s_miscCfg.m_windowState = saveState();
#endif
	m_view->saveConfig();

	s_miscCfg.writeToConfig(m_config);
	s_fnFormatCfg.writeToConfig(m_config);
	s_id3FormatCfg.writeToConfig(m_config);
	s_genCfg.writeToConfig(m_config);
	s_freedbCfg.writeToConfig(m_config);
	s_trackTypeCfg.writeToConfig(m_config);
	s_discogsCfg.writeToConfig(m_config);
	s_amazonCfg.writeToConfig(m_config);
	s_filterCfg.writeToConfig(m_config);
	s_playlistCfg.writeToConfig(m_config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.writeToConfig(m_config);
#endif
}

/**
 * Set the ID3v1 and ID3v2 text encodings from the configuration.
 */
static void setTextEncodings()
{
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
	const QTextCodec* id3v1TextCodec =
		Kid3App::s_miscCfg.m_textEncodingV1 != "ISO-8859-1" ?
		QTextCodec::codecForName(Kid3App::s_miscCfg.m_textEncodingV1.toLatin1().data()) : 0;
#endif
#ifdef HAVE_ID3LIB
	Mp3File::setDefaultTextEncoding(
		static_cast<MiscConfig::TextEncoding>(Kid3App::s_miscCfg.m_textEncoding));
	Mp3File::setTextCodecV1(id3v1TextCodec);
#endif
#ifdef HAVE_TAGLIB
	TagLibFile::setDefaultTextEncoding(
		static_cast<MiscConfig::TextEncoding>(Kid3App::s_miscCfg.m_textEncoding));
	TagLibFile::setTextCodecV1(id3v1TextCodec);
#endif
}

/**
 * Load application options.
 */
void Kid3App::readOptions()
{
	s_miscCfg.readFromConfig(m_config);
	if (s_miscCfg.m_nameFilter.isEmpty()) {
		createFilterString(&s_miscCfg.m_nameFilter);
	}
	setTextEncodings();
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
	s_amazonCfg.readFromConfig(m_config);
	s_filterCfg.readFromConfig(m_config);
	s_playlistCfg.readFromConfig(m_config);
#ifdef HAVE_TUNEPIMP
	s_musicBrainzCfg.readFromConfig(m_config);
#endif
#ifdef CONFIG_USE_KDE
	setAutoSaveSettings();
	m_settingsShowHidePicture->setChecked(!s_miscCfg.m_hidePicture);
	m_settingsAutoHideTags->setChecked(s_miscCfg.m_autoHideTags);
	m_fileOpenRecent->loadEntries(KConfigGroup(m_config,"Recent Files"));
#else
	if (s_miscCfg.m_hideStatusBar)
		statusBar()->hide();
	m_viewStatusBar->setChecked(!s_miscCfg.m_hideStatusBar);
	m_settingsShowHidePicture->setChecked(!s_miscCfg.m_hidePicture);
	m_settingsAutoHideTags->setChecked(s_miscCfg.m_autoHideTags);
	m_fileOpenRecent->loadEntries(m_config);
	restoreGeometry(s_miscCfg.m_geometry);
	restoreState(s_miscCfg.m_windowState);
#endif
	m_view->readConfig();
}

#ifdef CONFIG_USE_KDE
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
void Kid3App::readProperties(const KConfigGroup& cfg)
{
	openDirectory(cfg.readEntry("dirname", ""));
}

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

/**
 * Read font and style options.
 */
void Kid3App::readFontAndStyleOptions()
{
	s_miscCfg.readFromConfig(m_config);
	if (s_miscCfg.m_useFont &&
			!s_miscCfg.m_fontFamily.isEmpty() && s_miscCfg.m_fontSize > 0) {
		QApplication::setFont(QFont(s_miscCfg.m_fontFamily, s_miscCfg.m_fontSize));
	}
	if (!s_miscCfg.m_style.isEmpty()) {
		QApplication::setStyle(s_miscCfg.m_style);
	}
}

#endif /* CONFIG_USE_KDE */

/**
 * Save all changed files.
 *
 * @param updateGui true to update GUI (controls, status, cursor)
 * @param errStr    if not 0, the error string is returned here and
 *                  no dialog is displayed
 *
 * @return true if ok.
 */
bool Kid3App::saveDirectory(bool updateGui, QString* errStr)
{
	if (updateGui) {
		updateCurrentSelection();
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		slotStatusMsg(i18n("Saving directory..."));
	}

	QStringList errorFiles;
	int numFiles = 0, totalFiles = 0;
	// Get number of files to be saved to display correct progressbar
	TaggedFileIterator countIt(m_view->getFileList()->rootIndex());
	while (countIt.hasNext()) {
		if (countIt.next()->isChanged()) {
			++totalFiles;
		}
	}

	QProgressBar* progress = new QProgressBar();
	statusBar()->addPermanentWidget(progress);
	progress->setMinimum(0);
	progress->setMaximum(totalFiles);
	progress->setValue(numFiles);
#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
	TaggedFileIterator it(m_view->getFileList()->rootIndex());
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		bool renamed = false;
		if (!taggedFile->writeTags(false, &renamed, s_miscCfg.m_preserveTime)) {
			errorFiles.push_back(taggedFile->getFilename());
		}
		++numFiles;
		progress->setValue(numFiles);
	}
	statusBar()->removeWidget(progress);
	delete progress;
	updateModificationState();
	if (!errorFiles.empty()) {
		if (errStr) {
			*errStr = errorFiles.join("\n");
		} else {
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
				QMessageBox::Ok, Qt::NoButton);
#endif
		}
	}

	if (updateGui) {
		slotStatusMsg(i18n("Ready."));
		QApplication::restoreOverrideCursor();
		updateGuiControls();
	}
	return errorFiles.empty();
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
			if (m_view->getFileList()->selectionModel())
				m_view->getFileList()->selectionModel()->clearSelection();
			slotFileRevert();
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
	m_config->sync();
#else
	m_config->sync();
#endif
	TaggedFile::staticCleanup();
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

#ifndef WIN32
/**
 * Get all combinations with lower- and uppercase characters.
 *
 * @param str original string
 *
 * @return string with all combinations, separated by spaces.
 */
static QString lowerUpperCaseCombinations(const QString& str)
{
	QString result;
	QString lc(str.toLower());
	QString uc(str.toUpper());

	// get a mask of all alphabetic characters in the string
	unsigned char numChars = 0, charMask = 0, posMask = 1;
	int numPos = lc.length();
	if (numPos > 8) numPos = 8;
	for (int pos = 0; pos < numPos; ++pos, posMask <<= 1) {
		if (lc[pos] >= 'a' && lc[pos] <= 'z') {
			charMask |= posMask;
			++numChars;
		}
	}

	int numCombinations = 1 << numChars;
	for (int comb = 0; comb < numCombinations; ++comb) {
		posMask = 1;
		int combMask = 1;
		if (!result.isEmpty()) {
			result += ' ';
		}
		for (int pos = 0; pos < numPos; ++pos, posMask <<= 1) {
			if (charMask & posMask) {
				if (comb & combMask) {
					result += uc[pos];
				} else {
					result += lc[pos];
				}
				combMask <<= 1;
			} else {
				result += lc[pos];
			}
		}
	}

	return result;
}
#endif

/**
 * Create a filter string for the file dialog.
 * The filter string contains entries for all supported types.
 *
 * @param defaultNameFilter if not 0, return default name filter here
 *
 * @return filter string.
 */
QString Kid3App::createFilterString(QString* defaultNameFilter) const
{
	QStringList extensions = TaggedFile::getSupportedFileExtensions();
	QString result, allCombinations;
	for (QStringList::const_iterator it = extensions.begin();
			 it != extensions.end();
			 ++it) {
		QString text = (*it).mid(1).toUpper();
		QString lowerExt = '*' + *it;
#ifdef WIN32
		const QString& combinations = lowerExt;
#else
		QString combinations = lowerUpperCaseCombinations(lowerExt);
#endif
		if (!allCombinations.isEmpty()) {
			allCombinations += ' ';
		}
		allCombinations += combinations;
#ifdef CONFIG_USE_KDE
		result += combinations;
		result += '|';
		result += text;
		result += " (";
		result += lowerExt;
		result += ")\n";
#else
		result += text;
		result += " (";
		result += combinations;
		result += ");;";
#endif
	}

#ifdef CONFIG_USE_KDE
	QString allExt = allCombinations;
	allExt += '|';
	allExt += i18n("All Supported Files");
	allExt += '\n';
	result = allExt + result + "*|" + i18n("All Files (*)");
#else
	QString allExt = i18n("All Supported Files");
	allExt += " (";
	allExt += allCombinations;
	allExt += ");;";
	result = allExt + result + i18n("All Files (*)");
#endif

	if (defaultNameFilter) {
		*defaultNameFilter = allCombinations;
	}

	return result;
}

/**
 * Request new directory and open it.
 */
void Kid3App::slotFileOpen()
{
	updateCurrentSelection();
	if(saveModified()) {
		static QString flt = createFilterString();
		QString dir, filter;
#ifdef CONFIG_USE_KDE
		KFileDialog diag(s_dirName, flt, this);
		diag.setWindowTitle(i18n("Open"));
		if (diag.exec() == QDialog::Accepted) {
			dir = diag.selectedFile();
			filter = diag.currentFilter();
		}
#else
		dir = QFileDialog::getOpenFileName(
			this, QString(), s_dirName, flt, &filter
#if !defined Q_OS_WIN32 && !defined Q_OS_MAC
			// filter does not work with the KDE style file dialog
			, QFileDialog::DontUseNativeDialog
#endif
			);
#endif
		if (!dir.isEmpty()) {
			int start = filter.indexOf('('), end = filter.indexOf(')');
			if (start != -1 && end != -1 && end > start) {
				filter = filter.mid(start + 1, end - start - 1);
			}
			if (!filter.isEmpty()) {
				s_miscCfg.m_nameFilter = filter;
			}
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
		dir = QFileDialog::getExistingDirectory(this, QString(), s_dirName
#if !defined Q_OS_WIN32 && !defined Q_OS_MAC
			, QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog
#endif
			);
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
void Kid3App::slotFileOpenRecentUrl(const KUrl& url)
{
	updateCurrentSelection();
	QString dir = url.path();
	openDirectory(dir, true);
}

void Kid3App::slotFileOpenRecentDirectory(const QString&) {}
#else /* CONFIG_USE_KDE */
void Kid3App::slotFileOpenRecentUrl(const KUrl&) {}

void Kid3App::slotFileOpenRecentDirectory(const QString& dir)
{
	updateCurrentSelection();
	openDirectory(dir, true);
}
#endif /* CONFIG_USE_KDE */

/**
 * Revert file modifications.
 * Acts on selected files or all files if no file is selected.
 */
void Kid3App::slotFileRevert()
{
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																true);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(true);
		// update icon
		m_view->getFileList()->dataChanged(taggedFile->getIndex(),
																			 taggedFile->getIndex());
	}
	if (!it.hasNoSelection()) {
		m_view->frameModelV1()->clearFrames();
		m_view->frameModelV2()->clearFrames();
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
	saveDirectory(true);
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

void Kid3App::slotViewStatusBar() {}

/**
 * Shortcuts configuration.
 */
void Kid3App::slotSettingsShortcuts()
{
	KShortcutsDialog::configure(
		actionCollection(),
		KShortcutsEditor::LetterShortcutsDisallowed, this);
}

/**
 * Toolbars configuration.
 */
void Kid3App::slotSettingsToolbars()
{
	KEditToolBar dlg(actionCollection());
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
	KToolInvocation::invokeHelp(anchor);
}

void Kid3App::slotHelpHandbook() {}
void Kid3App::slotHelpAbout() {}
void Kid3App::slotHelpAboutQt() {}

#else /* CONFIG_USE_KDE */

void Kid3App::slotSettingsShortcuts() {}
void Kid3App::slotSettingsToolbars() {}

/**
 * Turn status bar on or off.
 */
void Kid3App::slotViewStatusBar()
{
	s_miscCfg.m_hideStatusBar = !m_viewStatusBar->isChecked();
	slotStatusMsg(i18n("Toggle the statusbar..."));
	if(s_miscCfg.m_hideStatusBar) {
		statusBar()->hide();
	}
	else {
		statusBar()->show();
	}
	slotStatusMsg(i18n("Ready."));
}

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
		"\n(c) 2003-2011 Urs Fleisch\nufleisch@users.sourceforge.net");
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
	statusBar()->showMessage(text);
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
 * Show playlist dialog.
 */
void Kid3App::slotPlaylistDialog()
{
	if (!m_playlistDialog) {
		m_playlistDialog = new PlaylistDialog(0);
	}
	if (m_playlistDialog) {
		m_playlistDialog->readConfig();
		if (m_playlistDialog->exec() == QDialog::Accepted) {
			PlaylistConfig cfg;
			m_playlistDialog->getCurrentConfig(cfg);
			writePlaylist(cfg);
		}
	}
}

/**
 * Write playlist according to playlist configuration.
 *
 * @param cfg playlist configuration to use
 *
 * @return true if ok.
 */
bool Kid3App::writePlaylist(const PlaylistConfig& cfg)
{
	PlaylistCreator plCtr(m_view->getDirPath(), cfg);
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	bool noSelection = !cfg.m_onlySelectedFiles || !selectModel ||
										 !selectModel->hasSelection();
	bool ok = true;
	QModelIndex rootIndex;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Creating playlist..."));

	if (cfg.m_location == PlaylistConfig::PL_CurrentDirectory) {
		// Get first child of parent of current index.
		rootIndex = m_view->getFileList()->currentOrRootIndex();
		if (rootIndex.model() && rootIndex.model()->rowCount(rootIndex) <= 0)
			rootIndex = rootIndex.parent();
		if (const QAbstractItemModel* model = rootIndex.model()) {
			for (int row = 0; row < model->rowCount(rootIndex); ++row) {
				QModelIndex index = model->index(row, 0, rootIndex);
				PlaylistCreator::Item plItem(index, plCtr);
				if (plItem.isFile() &&
						(noSelection || selectModel->isSelected(index))) {
					ok = plItem.add() && ok;
				}
			}
		}
	} else {
		QString selectedDirPrefix;
		rootIndex = m_view->getFileList()->rootIndex();
		ModelIterator it(rootIndex);
		while (it.hasNext()) {
			QModelIndex index = it.next();
			PlaylistCreator::Item plItem(index, plCtr);
			bool inSelectedDir = false;
			if (plItem.isDir()) {
				if (!selectedDirPrefix.isEmpty()) {
					if (plItem.getDirName().startsWith(selectedDirPrefix)) {
						inSelectedDir = true;
					} else {
						selectedDirPrefix = "";
					}
				}
				if (inSelectedDir || noSelection || selectModel->isSelected(index)) {
					// if directory is selected, all its files are selected
					if (!inSelectedDir) {
						selectedDirPrefix = plItem.getDirName();
					}
				}
			} else if (plItem.isFile()) {
				QString dirName = plItem.getDirName();
				if (!selectedDirPrefix.isEmpty()) {
					if (dirName.startsWith(selectedDirPrefix)) {
						inSelectedDir = true;
					} else {
						selectedDirPrefix = "";
					}
				}
				if (inSelectedDir || noSelection || selectModel->isSelected(index)) {
					ok = plItem.add() && ok;
				}
			}
		}
	}

	ok = plCtr.write() && ok;
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();
	return ok;
}

/**
 * Create playlist.
 *
 * @return true if ok.
 */
bool Kid3App::slotCreatePlaylist()
{
	return writePlaylist(s_playlistCfg);
}

/**
 * Update track data and create import dialog.
 */
void Kid3App::setupImportDialog()
{
	TrackData::TagVersion tagVersion = TrackData::TagNone;
	switch (s_genCfg.m_importDest) {
	case ImportConfig::DestV1:
		tagVersion = TrackData::TagV1;
		break;
	case ImportConfig::DestV2:
		tagVersion = TrackData::TagV2;
		break;
	case ImportConfig::DestV1V2:
		tagVersion = TrackData::TagV2V1;
	}

	ImportTrackDataVector trackDataList;
	TaggedFileOfDirectoryIterator it(m_view->getFileList()->currentOrRootIndex());
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
		taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
		trackDataList.push_back(ImportTrackData(*taggedFile, tagVersion));
	}
	m_trackDataModel->setTrackData(trackDataList);

	if (!m_importDialog) {
		QString caption(i18n("Import"));
		m_importDialog =
			new ImportDialog(NULL, caption, m_trackDataModel);
	}
	if (m_importDialog) {
		m_importDialog->clear();
		if (!trackDataList.isTagV1Supported() &&
				m_importDialog->getDestination() == ImportConfig::DestV1) {
			m_importDialog->setDestination(ImportConfig::DestV2);
		}
	}
}

/**
 * Import tags from the import dialog.
 *
 * @param destV1 true to set tag 1
 * @param destV2 true to set tag 2
 */
void Kid3App::getTagsFromImportDialog(bool destV1, bool destV2)
{
	slotStatusMsg(i18n("Import..."));
	ImportTrackDataVector trackDataList(m_trackDataModel->getTrackData());
	ImportTrackDataVector::iterator it = trackDataList.begin();
	FrameFilter flt(destV1 ?
									m_view->frameModelV1()->getEnabledFrameFilter(true) :
									m_view->frameModelV2()->getEnabledFrameFilter(true));
	TaggedFileOfDirectoryIterator tfit(
			m_view->getFileList()->currentOrRootIndex());
	while (tfit.hasNext()) {
		TaggedFile* taggedFile = tfit.next();
		taggedFile->readTags(false);
		if (it != trackDataList.end()) {
			it->removeDisabledFrames(flt);
			formatFramesIfEnabled(*it);
			if (destV1) taggedFile->setFramesV1(*it, false);
			if (destV2) taggedFile->setFramesV2(*it, false);
			++it;
		} else {
			break;
		}
	}
	if (m_view->getFileList()->selectionModel() &&
			m_view->getFileList()->selectionModel()->hasSelection()) {
		m_view->frameModelV1()->clearFrames();
		m_view->frameModelV2()->clearFrames();
		m_view->setFilenameEditEnabled(false);
		fileSelected();
	}
	else {
		updateModificationState();
	}
	slotStatusMsg(i18n("Ready."));
	QApplication::restoreOverrideCursor();

	if (destV2 && flt.isEnabled(Frame::FT_Picture) &&
			!trackDataList.getCoverArtUrl().isEmpty()) {
		downloadImage(trackDataList.getCoverArtUrl(), ImageForImportTrackData);
	}
}

/**
 * Execute the import dialog.
 */
void Kid3App::execImportDialog()
{
	if (m_importDialog &&
			m_importDialog->exec() == QDialog::Accepted) {
		bool destV1 = m_importDialog->getDestination() == ImportConfig::DestV1 ||
		              m_importDialog->getDestination() == ImportConfig::DestV1V2;
		bool destV2 = m_importDialog->getDestination() == ImportConfig::DestV2 ||
		              m_importDialog->getDestination() == ImportConfig::DestV1V2;
		getTagsFromImportDialog(destV1, destV2);
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
 * Import from Amazon.
 */
void Kid3App::slotImportAmazon()
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_Amazon);
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
 * Browse album cover artwork.
 */
void Kid3App::slotBrowseCoverArt()
{
	if (!m_browseCoverArtDialog) {
		m_browseCoverArtDialog = new BrowseCoverArtDialog(0);
	}
	if (m_browseCoverArtDialog) {
		FrameCollection frames2;
		QModelIndex index = m_view->getFileList()->currentIndex();
		if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
			taggedFile->readTags(false);
			FrameCollection frames1;
			taggedFile->getAllFramesV1(frames1);
			taggedFile->getAllFramesV2(frames2);
			frames2.merge(frames1);
		}

		m_browseCoverArtDialog->readConfig();
		m_browseCoverArtDialog->setFrames(frames2);
		m_browseCoverArtDialog->exec();
	}
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
		TaggedFileOfDirectoryIterator it(
				m_view->getFileList()->currentOrRootIndex());
		while (it.hasNext()) {
			TaggedFile* taggedFile = it.next();
			taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
			taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
			trackDataVector.push_back(
				ImportTrackData(
					*taggedFile,
					src == ExportDialog::SrcV1 ? ImportTrackData::TagV1
																		 : ImportTrackData::TagV2));
		}
		m_exportDialog->setExportData(trackDataVector);
	}
}

/**
 * Export.
 *
 * @param tagNr  tag number (1 or 2)
 * @param path   path of file
 * @param fmtIdx index of format
 *
 * @return true if ok.
 */
bool Kid3App::exportTags(int tagNr, const QString& path, int fmtIdx)
{
	bool ok = false;
	m_exportDialog = new ExportDialog(0);
	if (m_exportDialog) {
		m_exportDialog->readConfig();
		m_exportDialog->setFormatLineEdit(fmtIdx);
		setExportData(tagNr == 2 ?
									ExportDialog::SrcV2 : ExportDialog::SrcV1);
		connect(m_exportDialog, SIGNAL(exportDataRequested(int)),
						this, SLOT(setExportData(int)));
		ok = m_exportDialog->exportToFile(path);
		delete m_exportDialog;
		m_exportDialog = 0;
	}
	return ok;
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
 * Toggle auto hiding of tags.
 */
void Kid3App::slotSettingsAutoHideTags()
{
#ifdef CONFIG_USE_KDE
	s_miscCfg.m_autoHideTags = m_settingsAutoHideTags->isChecked();
#else
	s_miscCfg.m_autoHideTags = m_settingsAutoHideTags->isChecked();
#endif
	updateCurrentSelection();
	updateGuiControls();
}

/**
 * Show or hide picture.
 */
void Kid3App::slotSettingsShowHidePicture()
{
#ifdef CONFIG_USE_KDE
	s_miscCfg.m_hidePicture = !m_settingsShowHidePicture->isChecked();
#else
	s_miscCfg.m_hidePicture = !m_settingsShowHidePicture->isChecked();
#endif

	m_view->hidePicture(s_miscCfg.m_hidePicture);
	// In Qt3 the picture is displayed too small if Kid3 is started with picture
	// hidden, and then "Show Picture" is triggered while a file with a picture
	// is selected. Thus updating the controls is only done for Qt4, in Qt3 the
	// file has to be selected again for the picture to be shown.
	if (!s_miscCfg.m_hidePicture) {
		updateGuiControls();
	}
}

/**
 * Preferences.
 */
void Kid3App::slotSettingsConfigure()
{
	QString caption(i18n("Configure - Kid3"));
#ifdef CONFIG_USE_KDE
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
				m_view->frameModelV1()->markRows(0);
			}
			if (!s_miscCfg.m_markChanges) {
				m_view->frameModelV1()->markChangedFrames(0);
				m_view->frameModelV2()->markChangedFrames(0);
				m_view->markChangedFilename(false);
			}
			setTextEncodings();
		}
	}
#ifdef CONFIG_USE_KDE
	delete configSkeleton;
#endif
}

/**
 * Apply filename format.
 */
void Kid3App::slotApplyFilenameFormat()
{
	updateCurrentSelection();
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																true);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(false);
		QString fn = taggedFile->getFilename();
		s_fnFormatCfg.formatString(fn);
		taggedFile->setFilename(fn);
	}
	updateGuiControls();
}

/**
 * Apply ID3 format.
 */
void Kid3App::slotApplyId3Format()
{
	FrameCollection frames;
	updateCurrentSelection();
	FrameFilter fltV1(m_view->frameModelV1()->getEnabledFrameFilter(true));
	FrameFilter fltV2(m_view->frameModelV2()->getEnabledFrameFilter(true));
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																true);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(false);
		taggedFile->getAllFramesV1(frames);
		frames.removeDisabledFrames(fltV1);
		s_id3FormatCfg.formatFrames(frames);
		taggedFile->setFramesV1(frames);
		taggedFile->getAllFramesV2(frames);
		frames.removeDisabledFrames(fltV2);
		s_id3FormatCfg.formatFrames(frames);
		taggedFile->setFramesV2(frames);
	}
	updateGuiControls();
}

/**
 * Schedule actions to rename a directory.
 */
void Kid3App::scheduleRenameActions()
{
	if (m_renDirDialog) {
		m_renDirDialog->clearActions();
		TaggedFileIterator it(m_view->getFileList()->rootIndex());
		while (it.hasNext()) {
			TaggedFile* taggedFile = it.next();
			taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
			taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
			m_renDirDialog->scheduleAction(taggedFile);
#ifdef CONFIG_USE_KDE
			kapp->processEvents();
#else
			qApp->processEvents();
#endif
			if (m_renDirDialog->getAbortFlag()) {
				break;
			}
		}
	}
}

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
bool Kid3App::renameDirectory(int tagMask, const QString& format,
															bool create, QString* errStr)
{
	bool ok = false;
	TaggedFile* taggedFile =
		TaggedFileOfDirectoryIterator::first(
				m_view->getFileList()->currentOrRootIndex());
	if (!isModified() && taggedFile) {
		if (!m_renDirDialog) {
			m_renDirDialog = new RenDirDialog(0);
			connect(m_renDirDialog, SIGNAL(actionSchedulingRequested()),
							this, SLOT(scheduleRenameActions()));
		}
		if (m_renDirDialog) {
			m_renDirDialog->startDialog(taggedFile);
			m_renDirDialog->setTagSource(tagMask);
			m_renDirDialog->setDirectoryFormat(format);
			m_renDirDialog->setAction(create);
			scheduleRenameActions();
			openDirectory(getDirName());
			QString errorMsg;
			m_renDirDialog->performActions(&errorMsg);
			openDirectory(m_renDirDialog->getNewDirname());
			ok = errorMsg.isEmpty();
			if (errStr) {
				*errStr = errorMsg;
			}
		}
	}
	return ok;
}

/**
 * Rename directory.
 */
void Kid3App::slotRenameDirectory()
{
	if (saveModified()) {
		if (!m_renDirDialog) {
			m_renDirDialog = new RenDirDialog(0);
			connect(m_renDirDialog, SIGNAL(actionSchedulingRequested()),
							this, SLOT(scheduleRenameActions()));
		}
		if (m_renDirDialog) {
			QModelIndex index = m_view->getFileList()->currentOrRootIndex();
			QString dirName = FileProxyModel::getPathIfIndexOfDir(index);
			if (!dirName.isNull()) {
				m_view->getFileList()->expand(index);
			} else if (TaggedFile* taggedFile =
								 FileProxyModel::getTaggedFileOfIndex(index)) {
				dirName = taggedFile->getDirname();
			}
			if (!dirName.isEmpty()) {
				openDirectory(dirName);
			}
			if (TaggedFile* taggedFile = TaggedFileOfDirectoryIterator::first(index)) {
				m_renDirDialog->startDialog(taggedFile);
			} else {
				m_renDirDialog->startDialog(0, getDirName());
			}
			if (m_renDirDialog->exec() == QDialog::Accepted) {
				openDirectory(getDirName());
				QString errorMsg;
				m_renDirDialog->performActions(&errorMsg);
				openDirectory(m_renDirDialog->getNewDirname());
				if (!errorMsg.isEmpty()) {
					QMessageBox::warning(0, i18n("File Error"),
															 i18n("Error while renaming:\n") +
															 errorMsg,
															 QMessageBox::Ok, Qt::NoButton);
				}
			}

		}
	}
}

/**
 * Get number of tracks in current directory.
 *
 * @return number of tracks, 0 if not found.
 */
int Kid3App::getTotalNumberOfTracksInDir()
{
	if (TaggedFile* taggedFile = TaggedFileOfDirectoryIterator::first(
			m_view->getFileList()->currentOrRootIndex())) {
		return taggedFile->getTotalNumberOfTracksInDir();
	}
	return 0;
}

/**
 * Number tracks in selected files of directory.
 *
 * @param nr start number
 * @param total total number of tracks, used if >0
 * @param destV1 true to set numbers in tag 1
 * @param destV2 true to set numbers in tag 2
 */
void Kid3App::numberTracks(int nr, int total, bool destV1, bool destV2)
{
	updateCurrentSelection();
	int numDigits = Kid3App::s_miscCfg.m_trackNumberDigits;
	if (numDigits < 1 || numDigits > 5)
		numDigits = 1;

	SelectedTaggedFileOfDirectoryIterator it(
			m_view->getFileList()->currentOrRootIndex(),
			m_view->getFileList()->selectionModel(),
			true);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(false);
		if (destV1) {
			int oldnr = taggedFile->getTrackNumV1();
			if (nr != oldnr) {
				taggedFile->setTrackNumV1(nr);
			}
		}
		if (destV2) {
			// For tag 2 the frame is written, so that we have control over the
			// format and the total number of tracks, and it is possible to change
			// the format even if the numbers stay the same.
			QString value;
			if (total > 0) {
				value.sprintf("%0*d/%0*d", numDigits, nr, numDigits, total);
			} else {
				value.sprintf("%0*d", numDigits, nr);
			}
			FrameCollection frames;
			taggedFile->getAllFramesV2(frames);
			Frame frame(Frame::FT_Track, "", "", -1);
			FrameCollection::const_iterator it = frames.find(frame);
			if (it != frames.end()) {
				frame = *it;
				frame.setValueIfChanged(value);
				if (frame.isValueChanged()) {
					taggedFile->setFrameV2(frame);
				}
			} else {
				frame.setValue(value);
				frame.setInternalName(Frame::getNameFromType(Frame::FT_Track));
				taggedFile->setFrameV2(frame);
			}
		}
		++nr;
	}
	updateGuiControls();
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
		m_numberTracksDialog->setTotalNumberOfTracks(
			getTotalNumberOfTracksInDir(),
			Kid3App::s_miscCfg.m_enableTotalNumberOfTracks);
		if (m_numberTracksDialog->exec() == QDialog::Accepted) {
			int nr = m_numberTracksDialog->getStartNumber();
			bool destV1 =
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV1 ||
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV1V2;
			bool destV2 =
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV2 ||
				m_numberTracksDialog->getDestination() == NumberTracksDialog::DestV1V2;
			bool totalEnabled;
			int total = m_numberTracksDialog->getTotalNumberOfTracks(&totalEnabled);
			if (!totalEnabled)
				total = 0;
			Kid3App::s_miscCfg.m_enableTotalNumberOfTracks = totalEnabled;
			numberTracks(nr, total, destV1, destV2);
		}
	}
}

/**
 * Apply a file filter to a directory.
 *
 * @param fileFilter filter to apply
 * @param model the model to be filtered
 * @param rootIndex model index of root directory
 *
 * @return true if ok, false if aborted.
 */
bool Kid3App::applyFilterToDir(FileFilter& fileFilter, FileProxyModel* model,
																 const QModelIndex& rootIndex)
{
	if (!model)
		return false;
	bool ok = true;
	unsigned numFiles = 0;
	TaggedFileIterator it(rootIndex);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();

		taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
		taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
		bool pass = fileFilter.filter(*taggedFile, &ok);
		if (!ok) {
			if (m_filterDialog) {
				m_filterDialog->showInformation("parse error");
			}
			break;
		}
		if (m_filterDialog) {
			m_filterDialog->showInformation(
				(pass ? QString("+\t") : QString("-\t")) + taggedFile->getFilename());
		}
		if (!pass)
			model->filterOutIndex(taggedFile->getIndex());

		if (++numFiles == 8) {
			numFiles = 0;
#ifdef CONFIG_USE_KDE
			kapp->processEvents();
#else
			qApp->processEvents();
#endif
			if (m_filterDialog && m_filterDialog->getAbortFlag())
				break;
		}
	}
	return false;
}

/**
 * Apply a file filter.
 *
 * @param fileFilter filter to apply.
 */
void Kid3App::applyFilter(FileFilter& fileFilter)
{
	QModelIndex rootIndex(m_view->getFileList()->rootIndex());
	FileProxyModel* model =
			qobject_cast<FileProxyModel*>(m_view->getFileList()->model());
	if (!rootIndex.isValid() || !model)
		return;

	model->disableFilteringOutIndexes();
	setFiltered(false);

	if (m_filterDialog) {
		m_filterDialog->clearAbortFlag();
	}

	applyFilterToDir(fileFilter, model, rootIndex);

	model->applyFilteringOutIndexes();
	setFiltered(!fileFilter.isEmptyFilterExpression());
	updateModificationState();
}

/**
 * Filter.
 */
void Kid3App::slotFilter()
{
	if (saveModified()) {
		if (!m_filterDialog) {
			m_filterDialog = new FilterDialog(0);
			connect(m_filterDialog, SIGNAL(apply(FileFilter&)),
							this, SLOT(applyFilter(FileFilter&)));
		}
		if (m_filterDialog) {
			s_filterCfg.setFilenameFormat(m_view->getFilenameFormat());
			m_filterDialog->readConfig();
			m_filterDialog->exec();
		}
	}
}

/**
 * Convert ID3v2.3 to ID3v2.4 tags.
 */
void Kid3App::slotConvertToId3v24()
{
#ifdef HAVE_TAGLIB
	updateCurrentSelection();
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(false);
		if (taggedFile->hasTagV2() && !taggedFile->isChanged()) {
			QString tagFmt = taggedFile->getTagFormatV2();
			if (tagFmt.length() >= 7 && tagFmt.startsWith("ID3v2.") && tagFmt[6] < '4') {
#ifdef HAVE_ID3LIB
				if (dynamic_cast<Mp3File*>(taggedFile) != 0) {
					FrameCollection frames;
					taggedFile->getAllFramesV2(frames);
					FrameFilter flt;
					flt.enableAll();
					taggedFile->deleteFramesV2(flt);

					// The file has to be read with TagLib to write ID3v2.4 tags
					taggedFile = FileProxyModel::readWithTagLib(taggedFile);

					// Restore the frames
					FrameFilter frameFlt;
					frameFlt.enableAll();
					taggedFile->setFramesV2(frames.copyEnabledFrames(frameFlt), false);
				}
#endif

				// Write the file with TagLib, it always writes ID3v2.4 tags
				bool renamed;
				taggedFile->writeTags(true, &renamed, s_miscCfg.m_preserveTime);
				taggedFile->readTags(true);
			}
		}
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
	updateCurrentSelection();
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->readTags(false);
		if (taggedFile->hasTagV2() && !taggedFile->isChanged()) {
			QString tagFmt = taggedFile->getTagFormatV2();
			if (tagFmt.length() >= 7 && tagFmt.startsWith("ID3v2.") && tagFmt[6] > '3') {
				if (dynamic_cast<TagLibFile*>(taggedFile) != 0) {
					FrameCollection frames;
					taggedFile->getAllFramesV2(frames);
					FrameFilter flt;
					flt.enableAll();
					taggedFile->deleteFramesV2(flt);

					// The file has to be read with id3lib to write ID3v2.3 tags
					taggedFile = FileProxyModel::readWithId3Lib(taggedFile);

					// Restore the frames
					FrameFilter frameFlt;
					frameFlt.enableAll();
					taggedFile->setFramesV2(frames.copyEnabledFrames(frameFlt), false);
				}

				// Write the file with id3lib, it always writes ID3v2.3 tags
				bool renamed;
				taggedFile->writeTags(true, &renamed, s_miscCfg.m_preserveTime);
				taggedFile->readTags(true);
			}
		}
	}
	updateGuiControls();
#endif
}

/**
 * Play audio file.
 */
void Kid3App::slotPlayAudio()
{
#ifdef HAVE_PHONON
	QStringList files;
	int fileNr = 0;
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	if (selectModel && selectModel->selectedIndexes().size() > 1) {
		// play only the selected files if more than one is selected
		SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																	m_view->getFileList()->selectionModel(),
																	false);
		while (it.hasNext()) {
			files.append(it.next()->getAbsFilename());
		}
	} else {
		// play all files if none or only one is selected
		int idx = 0;
		QModelIndex rootIndex(m_view->getFileList()->rootIndex());
		ModelIterator it(rootIndex);
		while (it.hasNext()) {
			QModelIndex index = it.next();
			if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
				files.append(taggedFile->getAbsFilename());
				if (selectModel && selectModel->isSelected(index)) {
					fileNr = idx;
				}
				++idx;
			}
		}
	}

	if (!m_playToolBar) {
		m_playToolBar = new PlayToolBar(this);
		m_playToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
		addToolBar(Qt::BottomToolBarArea, m_playToolBar);
		connect(m_playToolBar, SIGNAL(errorMessage(const QString&)),
						this, SLOT(slotStatusMsg(const QString&)));
	}
	m_playToolBar->setFiles(files, fileNr);
	m_playToolBar->show();
#endif
}


/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */
void Kid3App::openDrop(QString txt)
{
	int lfPos = txt.indexOf('\n');
	if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
		txt.truncate(lfPos + 1);
	}
	QUrl url(txt);
	if (!url.path().isEmpty()) {
#if defined _WIN32 || defined WIN32
		QString dir = url.toString();
#else
		QString dir = url.path().trimmed();
#endif
		if (dir.endsWith(".jpg", Qt::CaseInsensitive) ||
				dir.endsWith(".jpeg", Qt::CaseInsensitive) ||
				dir.endsWith(".png", Qt::CaseInsensitive)) {
			PictureFrame frame;
			if (PictureFrame::setDataFromFile(frame, dir)) {
				QString fileName(dir);
				int slashPos = fileName.lastIndexOf('/');
				if (slashPos != -1) {
					fileName = fileName.mid(slashPos + 1);
				}
				PictureFrame::setMimeTypeFromFileName(frame, fileName);
				PictureFrame::setDescription(frame, fileName);
				addFrame(&frame);
				updateGuiControls();
			}
		} else {
			updateCurrentSelection();
			openDirectory(dir, true);
		}
	}
}

/**
 * Add picture on drop.
 *
 * @param image dropped image.
 */
void Kid3App::dropImage(const QImage& image)
{
	if (!image.isNull()) {
		PictureFrame frame;
		if (PictureFrame::setDataFromImage(frame, image)) {
			addFrame(&frame);
			updateGuiControls();
		}
	}
}

/**
 * Download an image file.
 *
 * @param url  URL of image
 * @param dest specifies affected files
 */
void Kid3App::downloadImage(const QString& url, DownloadImageDestination dest)
{
	QString imgurl(BrowseCoverArtDialog::getImageUrl(url));
	if (!imgurl.isEmpty()) {
		if (!m_downloadDialog) {
			m_downloadDialog = new DownloadDialog(0, i18n("Download"));
			connect(m_downloadDialog, SIGNAL(downloadFinished(const QByteArray&, const QString&, const QString&)),
							this, SLOT(imageDownloaded(const QByteArray&, const QString&, const QString&)));
		}
		if (m_downloadDialog) {
			int hostPos = imgurl.indexOf("://");
			if (hostPos > 0) {
				int pathPos = imgurl.indexOf("/", hostPos + 3);
				if (pathPos > hostPos) {
					m_downloadImageDest = dest;
					m_downloadDialog->startDownload(
						imgurl.mid(hostPos + 3, pathPos - hostPos - 3),
						imgurl.mid(pathPos));
					m_downloadDialog->show();
				}
			}
		}
	}
}

/**
 * Handle URL on drop.
 *
 * @param txt dropped URL.
 */
void Kid3App::dropUrl(const QString& txt)
{
	downloadImage(txt, ImageForSelectedFiles);
}

/**
 * Add a downloaded image.
 *
 * @param data     HTTP response of download
 * @param mimeType MIME type of data
 * @param url      URL of downloaded data
 */
void Kid3App::imageDownloaded(const QByteArray& data,
                              const QString& mimeType, const QString& url)
{
	if (mimeType.startsWith("image")) {
		PictureFrame frame(data, url, PictureFrame::PT_CoverFront, mimeType);
		if (m_downloadImageDest == ImageForAllFilesInDirectory) {
			TaggedFileOfDirectoryIterator it(
					m_view->getFileList()->currentOrRootIndex());
			while (it.hasNext()) {
				TaggedFile* taggedFile = it.next();
				taggedFile->readTags(false);
				taggedFile->addFrameV2(frame);
			}
		} else if (m_downloadImageDest == ImageForImportTrackData) {
			const ImportTrackDataVector& trackDataVector(
						m_trackDataModel->trackData());
			for (ImportTrackDataVector::const_iterator it =
					 trackDataVector.constBegin();
					 it != trackDataVector.constEnd();
					 ++it) {
				TaggedFile* taggedFile;
				if (it->isEnabled() && (taggedFile = it->getTaggedFile()) != 0) {
					taggedFile->readTags(false);
					taggedFile->addFrameV2(frame);
				}
			}
		} else {
			addFrame(&frame);
		}
		m_downloadImageDest = ImageForSelectedFiles;
		updateGuiControls();
	}
}

/**
 * Update modification state, caption and listbox entries.
 */
void Kid3App::updateModificationState()
{
	bool modified = false;
	TaggedFileIterator it(m_view->getFileList()->rootIndex());
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		if (taggedFile->isChanged()) {
			modified = true;
			m_view->getFileList()->dataChanged(taggedFile->getIndex(),
																				 taggedFile->getIndex());
		}
	}
	setModified(modified);
	updateWindowCaption();
}

/**
 * Set window title with information from directory, filter and modification
 * state.
 */
void Kid3App::updateWindowCaption()
{
	QString cap(QDir(s_dirName).dirName());
	if (isFiltered()) {
		cap += i18n(" [filtered]");
	}
#ifdef CONFIG_USE_KDE
	setCaption(cap, isModified());
#else
	if (isModified()) {
		cap += i18n(" [modified]");
	}
	if (!cap.isEmpty()) {
		cap += " - ";
	}
	cap += "Kid3";
	setWindowTitle(cap);
#endif
}

/**
 * Update files of current selection.
 */
void Kid3App::updateCurrentSelection()
{
	const QList<QPersistentModelIndex>& selItems =
		m_view->getFileList()->getCurrentSelection();
	int numFiles = selItems.size();
	if (numFiles > 0) {
		m_view->frameTableV1()->acceptEdit();
		m_view->frameTableV2()->acceptEdit();
		for (QList<QPersistentModelIndex>::const_iterator it = selItems.begin();
				 it != selItems.end();
				 ++it) {
			if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(*it)) {
				taggedFile->setFramesV1(m_view->frameModelV1()->frames());
				taggedFile->setFramesV2(m_view->frameModelV2()->frames());
				if (m_view->isFilenameEditEnabled()) {
					taggedFile->setFilename(m_view->getFilename());
				}
			}
		}
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
	TaggedFile* single_v2_file = 0;
	int num_v1_selected = 0;
	int num_v2_selected = 0;
	bool tagV1Supported = false;
	bool hasTagV1 = false;
	bool hasTagV2 = false;

	m_view->getFileList()->updateCurrentSelection();
	const QList<QPersistentModelIndex>& selItems =
			m_view->getFileList()->getCurrentSelection();

	for (QList<QPersistentModelIndex>::const_iterator it = selItems.begin();
			 it != selItems.end();
			 ++it) {
		TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(*it);
		if (taggedFile) {
			taggedFile->readTags(false);

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
			taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif

			if (taggedFile->isTagV1Supported()) {
				if (num_v1_selected == 0) {
					FrameCollection frames;
					taggedFile->getAllFramesV1(frames);
					m_view->frameModelV1()->transferFrames(frames);
				}
				else {
					FrameCollection fileFrames;
					taggedFile->getAllFramesV1(fileFrames);
					m_view->frameModelV1()->filterDifferent(fileFrames);
				}
				++num_v1_selected;
				tagV1Supported = true;
			}
			if (num_v2_selected == 0) {
				FrameCollection frames;
				taggedFile->getAllFramesV2(frames);
				m_view->frameModelV2()->transferFrames(frames);
				single_v2_file = taggedFile;
			}
			else {
				FrameCollection fileFrames;
				taggedFile->getAllFramesV2(fileFrames);
				m_view->frameModelV2()->filterDifferent(fileFrames);
				single_v2_file = 0;
			}
			++num_v2_selected;

			hasTagV1 = hasTagV1 || taggedFile->hasTagV1();
			hasTagV2 = hasTagV2 || taggedFile->hasTagV2();
		}
	}

	TaggedFile::DetailInfo info;
	if (single_v2_file) {
		m_framelist->setTags(single_v2_file);
		m_view->setFilenameEditEnabled(true);
		m_view->setFilename(single_v2_file->getFilename());
		single_v2_file->getDetailInfo(info);
		m_view->setDetailInfo(info);
		m_view->setTagFormatV1(single_v2_file->getTagFormatV1());
		m_view->setTagFormatV2(single_v2_file->getTagFormatV2());

		if (s_miscCfg.m_markTruncations) {
			m_view->frameModelV1()->markRows(single_v2_file->getTruncationFlags());
		}
		if (s_miscCfg.m_markChanges) {
			m_view->frameModelV1()->markChangedFrames(
				single_v2_file->getChangedFramesV1());
			m_view->frameModelV2()->markChangedFrames(
				single_v2_file->getChangedFramesV2());
			m_view->markChangedFilename(single_v2_file->isFilenameChanged());
		}
	}
	else {
		if (num_v2_selected > 1) {
			m_view->setFilenameEditEnabled(false);
		}
		m_view->setDetailInfo(info);
		m_view->setTagFormatV1(QString::null);
		m_view->setTagFormatV2(QString::null);

		if (s_miscCfg.m_markTruncations) {
			m_view->frameModelV1()->markRows(0);
		}
		if (s_miscCfg.m_markChanges) {
			m_view->frameModelV1()->markChangedFrames(0);
			m_view->frameModelV2()->markChangedFrames(0);
			m_view->markChangedFilename(false);
		}
	}
	if (!s_miscCfg.m_hidePicture) {
		FrameCollection::const_iterator it =
			m_view->frameModelV2()->frames().find(Frame(Frame::FT_Picture, "", "", -1));
		if (it == m_view->frameModelV2()->frames().end() ||
				it->isInactive()) {
			m_view->setPictureData(0);
		} else {
			QByteArray data;
			m_view->setPictureData(PictureFrame::getData(*it, data) ? &data : 0);
		}
	}
	m_view->frameModelV1()->setAllCheckStates(num_v1_selected == 1);
	m_view->frameModelV2()->setAllCheckStates(num_v2_selected == 1);
	updateModificationState();

	if (num_v1_selected == 0 && num_v2_selected == 0) {
		tagV1Supported = true;
	}
	m_view->enableControlsV1(tagV1Supported);

	if (s_miscCfg.m_autoHideTags) {
		// If a tag is supposed to be absent, make sure that there is really no
		// unsaved data in the tag.
		if (!hasTagV1 && tagV1Supported) {
			const FrameCollection& frames = m_view->frameModelV1()->frames();
			for (FrameCollection::iterator it = frames.begin();
					 it != frames.end();
					 ++it) {
				if (!(*it).getValue().isEmpty()) {
					hasTagV1 = true;
					break;
				}
			}
		}
		if (!hasTagV2) {
			const FrameCollection& frames = m_view->frameModelV2()->frames();
			for (FrameCollection::iterator it = frames.begin();
					 it != frames.end();
					 ++it) {
				if (!(*it).getValue().isEmpty()) {
					hasTagV2 = true;
					break;
				}
			}
		}
		m_view->hideV1(!hasTagV1);
		m_view->hideV2(!hasTagV2);
	}
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
	updateCurrentSelection();
	m_copyTags = m_view->frameModelV1()->frames().copyEnabledFrames(
		m_view->frameModelV1()->getEnabledFrameFilter(true));
}

/**
 * Copy tags 2 into copy buffer.
 */
void Kid3App::copyTagsV2()
{
	updateCurrentSelection();
	m_copyTags = m_view->frameModelV2()->frames().copyEnabledFrames(
		m_view->frameModelV2()->getEnabledFrameFilter(true));
}

/**
 * Paste from copy buffer to ID3v1 tags.
 */
void Kid3App::pasteTagsV1()
{
	updateCurrentSelection();
	FrameCollection frames(m_copyTags.copyEnabledFrames(
													 m_view->frameModelV1()->getEnabledFrameFilter(true)));
	formatFramesIfEnabled(frames);
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		it.next()->setFramesV1(frames, false);
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
													 m_view->frameModelV2()->getEnabledFrameFilter(true)));
	formatFramesIfEnabled(frames);
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		it.next()->setFramesV2(frames, false);
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
	FrameCollection frames;
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																selectModel,
																false);
	bool multiselect = selectModel && selectModel->selectedIndexes().size() > 1;
	FrameFilter flt(m_view->frameModelV1()->getEnabledFrameFilter(true));
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		if (!multiselect && m_view->isFilenameEditEnabled()) {
			taggedFile->setFilename(m_view->getFilename());
		}
		taggedFile->getAllFramesV1(frames);
		taggedFile->getTagsFromFilename(frames,
																		m_view->getFromFilenameFormat());
		frames.removeDisabledFrames(flt);
		formatFramesIfEnabled(frames);
		taggedFile->setFramesV1(frames);
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
	FrameCollection frames;
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																selectModel,
																false);
	bool multiselect = selectModel && selectModel->selectedIndexes().size() > 1;
	FrameFilter flt(m_view->frameModelV2()->getEnabledFrameFilter(true));
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		if (!multiselect && m_view->isFilenameEditEnabled()) {
			taggedFile->setFilename(m_view->getFilename());
		}
		taggedFile->getAllFramesV2(frames);
		taggedFile->getTagsFromFilename(frames,
																		m_view->getFromFilenameFormat());
		frames.removeDisabledFrames(flt);
		formatFramesIfEnabled(frames);
		taggedFile->setFramesV2(frames);
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
	FrameCollection frames;
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																selectModel,
																false);
	bool multiselect = selectModel && selectModel->selectedIndexes().size() > 1;
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		if (tag_version == 2) {
			taggedFile->getAllFramesV2(frames);
		} else {
			taggedFile->getAllFramesV1(frames);
		}
		if (!frames.isEmptyOrInactive()) {
			taggedFile->getFilenameFromTags(frames, m_view->getFilenameFormat());
			formatFileNameIfEnabled(taggedFile);
			if (!multiselect) {
				m_view->setFilename(taggedFile->getFilename());
			}
		}
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
	FrameCollection frames;
	FrameFilter flt(m_view->frameModelV2()->getEnabledFrameFilter(true));
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->getAllFramesV1(frames);
		frames.removeDisabledFrames(flt);
		formatFramesIfEnabled(frames);
		taggedFile->setFramesV2(frames, false);
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
	FrameCollection frames;
	FrameFilter flt(m_view->frameModelV1()->getEnabledFrameFilter(true));
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		TaggedFile* taggedFile = it.next();
		taggedFile->getAllFramesV2(frames);
		frames.removeDisabledFrames(flt);
		formatFramesIfEnabled(frames);
		taggedFile->setFramesV1(frames, false);
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
	FrameFilter flt(m_view->frameModelV1()->getEnabledFrameFilter(true));
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		it.next()->deleteFramesV1(flt);
	}
	updateGuiControls();
}

/**
 * Remove ID3v2 tags in selected files.
 */
void Kid3App::removeTagsV2()
{
	updateCurrentSelection();
	FrameFilter flt(m_view->frameModelV2()->getEnabledFrameFilter(true));
	SelectedTaggedFileIterator it(m_view->getFileList()->rootIndex(),
																m_view->getFileList()->selectionModel(),
																false);
	while (it.hasNext()) {
		it.next()->deleteFramesV2(flt);
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
		FrameCollection frames;
		taggedFile->getAllFramesV2(frames);
		m_view->frameModelV2()->transferFrames(frames);
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
	if (!m_view->getFileList()->selectionModel())
		return 0;
	QModelIndexList selItems(
			m_view->getFileList()->selectionModel()->selectedIndexes());
	if (selItems.size() != 1)
		return 0;

	return FileProxyModel::getTaggedFileOfIndex(selItems.first());
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
		bool firstFile = true;
		QString name;
		SelectedTaggedFileIterator tfit(m_view->getFileList()->rootIndex(),
																		m_view->getFileList()->selectionModel(),
																		false);
		while (tfit.hasNext()) {
			TaggedFile* currentFile = tfit.next();
			if (firstFile) {
				firstFile = false;
				taggedFile = currentFile;
				m_framelist->setTags(taggedFile);
				name = m_framelist->getSelectedName();
				if (name.isEmpty() || !m_framelist->editFrame()) {
					break;
				}
			}
			FrameCollection frames;
			currentFile->getAllFramesV2(frames);
			for (FrameCollection::const_iterator it = frames.begin();
					 it != frames.end();
					 ++it) {
				if (it->getName() == name) {
					currentFile->deleteFrameV2(*it);
					m_framelist->setTags(currentFile);
					m_framelist->pasteFrame();
					break;
				}
			}
		}
		updateAfterFrameModification(taggedFile);
	}
}

/**
 * Delete selected frame.
 *
 * @param frameName name of frame to delete, empty to delete selected frame
 */
void Kid3App::deleteFrame(const QString& frameName)
{
	updateCurrentSelection();
	TaggedFile* taggedFile = getSelectedFile();
	m_framelist->reloadTags();
	if (taggedFile && frameName.isEmpty()) {
		// delete selected frame from single file
		if (!m_framelist->deleteFrame()) {
			// frame not found
			return;
		}
	} else {
		// multiple files selected or frame name specified
		bool firstFile = true;
		QString name;
		SelectedTaggedFileIterator tfit(m_view->getFileList()->rootIndex(),
																		m_view->getFileList()->selectionModel(),
																		false);
		while (tfit.hasNext()) {
			TaggedFile* currentFile = tfit.next();
			if (firstFile) {
				firstFile = false;
				taggedFile = currentFile;
				m_framelist->setTags(taggedFile);
				name = frameName.isEmpty() ? m_framelist->getSelectedName() :
					frameName;
			}
			FrameCollection frames;
			currentFile->getAllFramesV2(frames);
			for (FrameCollection::const_iterator it = frames.begin();
					 it != frames.end();
					 ++it) {
				if (it->getName() == name) {
					currentFile->deleteFrameV2(*it);
					break;
				}
			}
		}
	}
	updateAfterFrameModification(taggedFile);
}

/**
 * Select a frame type and add such a frame to frame list.
 *
 * @param frame frame to add, if 0 the user has to select and edit the frame
 * @param edit  if frame is set and edit is true, the user can edit the frame
 *              before it is added
 */
void Kid3App::addFrame(const Frame* frame, bool edit)
{
	updateCurrentSelection();
	TaggedFile* taggedFile = getSelectedFile();
	if (taggedFile) {
		bool frameAdded;
		if (!frame) {
			frameAdded = m_framelist->selectFrame() &&
				m_framelist->addFrame(true);
		} else if (edit) {
			m_framelist->setFrame(*frame);
			frameAdded = m_framelist->addFrame(true);
		} else {
			m_framelist->setFrame(*frame);
			frameAdded = m_framelist->pasteFrame();
		}
		if (frameAdded) {
			updateAfterFrameModification(taggedFile);
			if (m_framelist->isPictureFrame()) {
				// update preview picture
				updateGuiControls();
			}
		}
	} else {
		// multiple files selected
		bool firstFile = true;
		int frameId = -1;
		SelectedTaggedFileIterator tfit(m_view->getFileList()->rootIndex(),
																		m_view->getFileList()->selectionModel(),
																		false);
		while (tfit.hasNext()) {
			TaggedFile* currentFile = tfit.next();
			if (firstFile) {
				firstFile = false;
				taggedFile = currentFile;
				m_framelist->setTags(currentFile);
				if (!frame) {
					if (m_framelist->selectFrame() &&
							m_framelist->addFrame(true)) {
						frameId = m_framelist->getSelectedId();
					} else {
						break;
					}
				} else if (edit) {
					m_framelist->setFrame(*frame);
					if (m_framelist->addFrame(edit)) {
						frameId = m_framelist->getSelectedId();
					} else {
						break;
					}
				} else {
					m_framelist->setFrame(*frame);
					if (m_framelist->pasteFrame()) {
						frameId = m_framelist->getSelectedId();
					} else {
						break;
					}
				}
			} else {
				m_framelist->setTags(currentFile);
				m_framelist->pasteFrame();
			}
		}
		m_framelist->setTags(taggedFile);
		if (frameId != -1) {
			m_framelist->setSelectedId(frameId);
		}
		updateModificationState();
	}
}

/**
 * Edit a picture frame if one exists or add a new one.
 */
void Kid3App::editOrAddPicture()
{
	if (m_framelist->selectByName("Picture")) {
		editFrame();
	} else {
		PictureFrame frame;
		addFrame(&frame, true);
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

/**
 * Rename the selected file(s).
 */
void Kid3App::renameFile()
{
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	FileProxyModel* model =
			qobject_cast<FileProxyModel*>(m_view->getFileList()->model());
	if (!selectModel || !model)
		return;

	QList<QPersistentModelIndex> selItems;
	foreach (QModelIndex index, selectModel->selectedIndexes())
		selItems.append(index);
	foreach (QPersistentModelIndex index, selItems) {
		bool isDir = false;
		TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index);
		QString absFilename, dirName, fileName;
		if (taggedFile) {
			absFilename = taggedFile->getAbsFilename();
			dirName = taggedFile->getDirname();
			fileName = taggedFile->getFilename();
		} else {
			QFileInfo fi(model->fileInfo(index));
			absFilename = fi.filePath();
			dirName = fi.dir().path();
			fileName = fi.fileName();
			isDir = model->isDir(index);
		}
		bool ok;
		QString newFileName = QInputDialog::getText(
			this,
			i18n("Rename File"),
			i18n("Enter new file name:"),
			QLineEdit::Normal, fileName, &ok);
		if (ok && !newFileName.isEmpty() && newFileName != fileName) {
			if (taggedFile) {
				if (taggedFile->isChanged()) {
					taggedFile->setFilename(newFileName);
					if (selItems.size() == 1)
						m_view->setFilename(newFileName);
					continue;
				}
				// This will close the file.
				// The file must be closed before renaming on Windows.
				FileProxyModel::releaseTaggedFileOfIndex(index);
			}
			QString newPath = dirName + '/' + newFileName;
			if (!QDir().rename(absFilename, newPath)) {
				QMessageBox::warning(
					0, i18n("File Error"),
					i18n("Error while renaming:\n") +
					KCM_i18n2("Rename %1 to %2 failed\n", fileName, newFileName),
					QMessageBox::Ok, Qt::NoButton);
			}
		}
	}
}

/**
 * Delete the selected file(s).
 */
void Kid3App::deleteFile()
{
	QItemSelectionModel* selectModel = m_view->getFileList()->selectionModel();
	FileProxyModel* model =
			qobject_cast<FileProxyModel*>(m_view->getFileList()->model());
	if (!selectModel || !model)
		return;

	QStringList files;
	QList<QPersistentModelIndex> selItems;
	foreach (QModelIndex index, selectModel->selectedIndexes())
		selItems.append(index);
	foreach (QPersistentModelIndex index, selItems) {
		files.append(model->filePath(index));
	}

	unsigned numFiles = files.size();
	if (numFiles > 0) {
#ifdef CONFIG_USE_KDE
		if (KMessageBox::warningContinueCancelList(
					this,
					i18np("Do you really want to delete this item?",
								"Do you really want to delete these %1 items?", numFiles),
					files,
					i18n("Delete Files"),
					KStandardGuiItem::del(), KStandardGuiItem::cancel(), QString(),
					KMessageBox::Dangerous) == KMessageBox::Continue)
#else
		QString txt = numFiles > 1 ?
			KCM_i18n1("Do you really want to delete these %1 items?", numFiles) :
			i18n("Do you really want to delete this item?");
		txt += '\n';
		txt += files.join("\n");
		if (QMessageBox::question(
				this, i18n("Delete Files"), txt,
				QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
#endif
		{
			bool rmdirError = false;
			files.clear();
			foreach (QPersistentModelIndex index, selItems) {
				QString absFilename(model->filePath(index));
				if (model->isDir(index)) {
					if (!model->rmdir(index)) {
						rmdirError = true;
						files.append(absFilename);
					}
				} else {
					if (FileProxyModel::getTaggedFileOfIndex(index)) {
						// This will close the file.
						// The file must be closed before deleting on Windows.
						FileProxyModel::releaseTaggedFileOfIndex(index);
					}
					if (!model->remove(index)) {
						files.append(absFilename);
					}
				}
			}
			if (!files.isEmpty()) {
				QString txt;
				if (rmdirError)
					txt += i18n("Directory must be empty.\n");
#ifdef CONFIG_USE_KDE
				txt += i18np("Error while deleting this item:",
										 "Error while deleting these %1 items:", files.size());
				KMessageBox::errorList(0, txt, files, i18n("File Error"));
#else
				txt += files.size() > 1 ?
					KCM_i18n1("Error while deleting these %1 items:", files.size()) :
					i18n("Error while deleting this item:");
				txt += '\n';
				txt += files.join("\n");
				QMessageBox::warning(
					0, i18n("File Error"), txt,
					QMessageBox::Ok, Qt::NoButton);
#endif
			}
		}
	}
}
