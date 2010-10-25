/**
 * \file kid3.cpp
 * Kid3 application.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2010  Urs Fleisch
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
#include <qimage.h>
#include <qtextcodec.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QIcon>
#include <QToolBar>
/** Set action icon for Qt 4 */
#define QACTION_SET_ICON(a, i) a->setIcon(QIcon(i))
#else
#include <qlayout.h>
/** Do not set an action icon for Qt 3 */
#define QACTION_SET_ICON(a, i) 
#endif
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
#if KDE_VERSION >= 0x035c00
#include <ktoggleaction.h>
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
#include "browsecoverartdialog.h"
#include "exportdialog.h"
#include "numbertracksdialog.h"
#include "filterdialog.h"
#include "rendirdialog.h"
#include "downloaddialog.h"
#include "playlistdialog.h"
#include "playlistcreator.h"
#include "filelistitem.h"
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

#ifdef KID3_USE_KCONFIGDIALOG
#include <kconfigskeleton.h>
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
	m_downloadToAllFilesInDir(false),
	m_importDialog(0), m_browseCoverArtDialog(0),
	m_exportDialog(0), m_renDirDialog(0),
	m_numberTracksDialog(0), m_filterDialog(0), m_downloadDialog(0),
	m_playlistDialog(0)
#ifdef HAVE_PHONON
	, m_playToolBar(0)
#endif
{
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
#ifndef __APPLE__
								"/icons/hicolor/48x48/apps/kid3-qt.png"
#else
								"/kid3.png"
#endif
				)) {
		QCM_setWindowIcon(icon);
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
#if KDE_VERSION >= 0x035c00
	editSelectAll->setShortcut(KShortcut("Alt+Shift+A"));
#else
	editSelectAll->setShortcut(KShortcut("Alt+A"));
#endif
	editDeselect->KCM_setStatusTip(i18n("Deselect all files"));
	settingsShortcuts->KCM_setStatusTip(i18n("Configure Shortcuts"));
	settingsToolbars->KCM_setStatusTip(i18n("Configure Toolbars"));
	settingsConfigure->KCM_setStatusTip(i18n("Preferences dialog"));

	KCM_KActionShortcutIcon(fileOpenDirectory, KShortcut("Ctrl+D"), KCM_ICON_document_open,
		    i18n("O&pen Directory..."), this,
		    SLOT(slotFileOpenDirectory()), actionCollection(),
		    "open_directory");
	KCM_KActionIcon(fileImport, KCM_ICON_document_import,
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
	KCM_KAction(fileImportAmazon,
		    i18n("Import from &Amazon..."), this,
		    SLOT(slotImportAmazon()), actionCollection(),
		    "import_amazon");
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
	KCM_KAction(fileBrowseCoverArt,
		    i18n("&Browse Cover Art..."), this,
		    SLOT(slotBrowseCoverArt()), actionCollection(),
		    "browse_cover_art");
	KCM_KActionIcon(fileExport, KCM_ICON_document_export,
		    i18n("&Export..."), this,
		    SLOT(slotExport()), actionCollection(),
		    "export");
	KCM_KActionIcon(fileCreatePlaylist, KCM_ICON_media_playlist,
		    i18n("&Create Playlist..."), this,
				SLOT(slotPlaylistDialog()), actionCollection(),
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
	KCM_KAction(toolsFilter,
		    i18n("F&ilter..."), this,
		    SLOT(slotFilter()), actionCollection(),
		    "filter");
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
#ifdef HAVE_PHONON
	KCM_KActionIcon(toolsPlay, KCM_ICON_media_playback_start,
		    i18n("&Play"), this,
		    SLOT(slotPlayAudio()), actionCollection(),
		    "play");
#endif
	KCM_KToggleActionVar(m_settingsShowHidePicture,
		    i18n("Show &Picture"), this,
		    SLOT(slotSettingsShowHidePicture()),
		    actionCollection(), "hide_picture");
	KCM_KToggleActionVar(m_settingsAutoHideTags,
		    i18n("Auto &Hide Tags"), this,
		    SLOT(slotSettingsAutoHideTags()),
		    actionCollection(), "auto_hide_tags");
	KCM_KActionShortcutIcon(editPreviousFile, KShortcut("Alt+Up"), KCM_ICON_go_previous,
		    i18n("&Previous File"), m_view,
		    SLOT(selectPreviousFile()), actionCollection(),
		    "previous_file");
	KCM_KActionShortcutIcon(editNextFile, KShortcut("Alt+Down"), KCM_ICON_go_next,
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
		QACTION_SET_ICON(fileOpen, ":/images/document-open.png");
		connect(fileOpen, QCM_SIGNAL_triggered,
			this, SLOT(slotFileOpen()));
	}
	QAction* fileOpenDirectory = new QAction(this);
	if (fileOpenDirectory) {
		fileOpenDirectory->setStatusTip(i18n("Opens a directory"));
		fileOpenDirectory->QCM_setMenuText(i18n("O&pen Directory..."));
		fileOpenDirectory->QCM_setShortcut(Qt::CTRL + Qt::Key_D);
		QACTION_SET_ICON(fileOpenDirectory, ":/images/document-open.png");
		connect(fileOpenDirectory, QCM_SIGNAL_triggered,
			this, SLOT(slotFileOpenDirectory()));
	}
	QAction* fileSave = new QAction(this);
	if (fileSave) {
		fileSave->setStatusTip(i18n("Saves the changed files"));
		fileSave->QCM_setMenuText(i18n("&Save"));
		fileSave->QCM_setShortcut(Qt::CTRL + Qt::Key_S);
		QACTION_SET_ICON(fileSave, ":/images/document-save.png");
		connect(fileSave, QCM_SIGNAL_triggered,
			this, SLOT(slotFileSave()));
	}
	QAction* fileRevert = new QAction(this);
	if (fileRevert) {
		fileRevert->setStatusTip(
		    i18n("Reverts the changes of all or the selected files"));
		fileRevert->QCM_setMenuText(i18n("Re&vert"));
		QACTION_SET_ICON(fileRevert, ":/images/document-revert.png");
		connect(fileRevert, QCM_SIGNAL_triggered,
			this, SLOT(slotFileRevert()));
	}
	QAction* fileImport = new QAction(this);
	if (fileImport) {
		fileImport->setStatusTip(i18n("Import from file or clipboard"));
		fileImport->QCM_setMenuText(i18n("&Import..."));
		QACTION_SET_ICON(fileImport, ":/images/document-import.png");
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
	QAction* fileImportAmazon = new QAction(this);
	if (fileImportAmazon) {
		fileImportAmazon->setStatusTip(i18n("Import from Amazon"));
		fileImportAmazon->QCM_setMenuText(i18n("Import from &Amazon..."));
		connect(fileImportAmazon, QCM_SIGNAL_triggered,
			this, SLOT(slotImportAmazon()));
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
	QAction* fileBrowseCoverArt = new QAction(this);
	if (fileBrowseCoverArt) {
		fileBrowseCoverArt->setStatusTip(i18n("Browse album cover artwork"));
		fileBrowseCoverArt->QCM_setMenuText(i18n("&Browse Cover Art..."));
		connect(fileBrowseCoverArt, QCM_SIGNAL_triggered,
			this, SLOT(slotBrowseCoverArt()));
	}
	QAction* fileExport = new QAction(this);
	if (fileExport) {
		fileExport->setStatusTip(i18n("Export to file or clipboard"));
		fileExport->QCM_setMenuText(i18n("&Export..."));
		QACTION_SET_ICON(fileExport, ":/images/document-export.png");
		connect(fileExport, QCM_SIGNAL_triggered,
			this, SLOT(slotExport()));
	}
	QAction* fileCreatePlaylist = new QAction(this);
	if (fileCreatePlaylist) {
		fileCreatePlaylist->setStatusTip(i18n("Create M3U Playlist"));
		fileCreatePlaylist->QCM_setMenuText(i18n("&Create Playlist..."));
		QACTION_SET_ICON(fileCreatePlaylist, ":/images/view-media-playlist.png");
		connect(fileCreatePlaylist, QCM_SIGNAL_triggered,
			this, SLOT(slotPlaylistDialog()));
	}
	QAction* fileQuit = new QAction(this);
	if (fileQuit) {
		fileQuit->setStatusTip(i18n("Quits the application"));
		fileQuit->QCM_setMenuText(i18n("&Quit"));
		fileQuit->QCM_setShortcut(Qt::CTRL + Qt::Key_Q);
		QACTION_SET_ICON(fileQuit, ":/images/application-exit.png");
		connect(fileQuit, QCM_SIGNAL_triggered,
			this, SLOT(slotFileQuit()));
	}
	QAction* editSelectAll = new QAction(this);
	if (editSelectAll) {
		editSelectAll->setStatusTip(i18n("Select all files"));
		editSelectAll->QCM_setMenuText(i18n("Select &All"));
		editSelectAll->QCM_setShortcut(Qt::ALT + Qt::Key_A);
		QACTION_SET_ICON(editSelectAll, ":/images/edit-select-all.png");
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
		QACTION_SET_ICON(editPreviousFile, ":/images/go-previous.png");
		connect(editPreviousFile, QCM_SIGNAL_triggered,
			m_view, SLOT(selectPreviousFile()));
	}
	QAction* editNextFile = new QAction(this);
	if (editNextFile) {
		editNextFile->setStatusTip(i18n("Select next file"));
		editNextFile->QCM_setMenuText(i18n("&Next File"));
		editNextFile->QCM_setShortcut(Qt::ALT + Qt::Key_Down);
		QACTION_SET_ICON(editNextFile, ":/images/go-next.png");
		connect(editNextFile, QCM_SIGNAL_triggered,
			m_view, SLOT(selectNextFile()));
	}
	QAction* helpHandbook = new QAction(this);
	if (helpHandbook) {
		helpHandbook->setStatusTip(i18n("Kid3 Handbook"));
		helpHandbook->QCM_setMenuText(i18n("Kid3 &Handbook"));
		QACTION_SET_ICON(helpHandbook, ":/images/help-contents.png");
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
	QAction* toolsFilter = new QAction(this);
	if (toolsFilter) {
		toolsFilter->setStatusTip(i18n("Filter"));
		toolsFilter->QCM_setMenuText(i18n("F&ilter..."));
		connect(toolsFilter, QCM_SIGNAL_triggered,
			this, SLOT(slotFilter()));
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
#ifdef HAVE_PHONON
	QAction* toolsPlay = new QAction(this);
	if (toolsPlay) {
		toolsPlay->setStatusTip(i18n("Play"));
		toolsPlay->QCM_setMenuText(i18n("&Play"));
		QACTION_SET_ICON(toolsPlay, style()->standardIcon(QStyle::SP_MediaPlay));
		connect(toolsPlay, QCM_SIGNAL_triggered,
			this, SLOT(slotPlayAudio()));
	}
#endif
	m_viewStatusBar = new QAction(this);
	if (m_viewStatusBar) {
		m_viewStatusBar->setStatusTip(i18n("Enables/disables the statusbar"));
		m_viewStatusBar->QCM_setMenuText(i18n("Show St&atusbar"));
#if QT_VERSION >= 0x040000
		m_viewStatusBar->setCheckable(true);
#else
		m_viewStatusBar->setToggleAction(true);
#endif
		connect(m_viewStatusBar, QCM_SIGNAL_triggered,
			this, SLOT(slotViewStatusBar()));
	}
	m_settingsShowHidePicture = new QAction(this);
	if (m_settingsShowHidePicture) {
		m_settingsShowHidePicture->setStatusTip(i18n("Show Picture"));
		m_settingsShowHidePicture->QCM_setMenuText(i18n("Show &Picture"));
#if QT_VERSION >= 0x040000
		m_settingsShowHidePicture->setCheckable(true);
#else
		m_settingsShowHidePicture->setToggleAction(true);
#endif
		connect(m_settingsShowHidePicture, QCM_SIGNAL_triggered,
			this, SLOT(slotSettingsShowHidePicture()));
	}
	m_settingsAutoHideTags = new QAction(this);
	if (m_settingsAutoHideTags) {
		m_settingsAutoHideTags->setStatusTip(i18n("Auto Hide Tags"));
		m_settingsAutoHideTags->QCM_setMenuText(i18n("Auto &Hide Tags"));
#if QT_VERSION >= 0x040000
		m_settingsAutoHideTags->setCheckable(true);
#else
		m_settingsAutoHideTags->setToggleAction(true);
#endif
		connect(m_settingsAutoHideTags, QCM_SIGNAL_triggered,
			this, SLOT(slotSettingsAutoHideTags()));
	}
	QAction* settingsConfigure = new QAction(this);
	if (settingsConfigure) {
		settingsConfigure->setStatusTip(i18n("Configure Kid3"));
		settingsConfigure->QCM_setMenuText(i18n("&Configure Kid3..."));
		QACTION_SET_ICON(settingsConfigure, ":/images/configure.png");
		connect(settingsConfigure, QCM_SIGNAL_triggered,
			this, SLOT(slotSettingsConfigure()));
	}
#if QT_VERSION >= 0x040000
	QToolBar* toolBar = new QToolBar(this);
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
		m_fileOpenRecent = new RecentFilesMenu(fileMenu);
		if (m_fileOpenRecent) {
			connect(m_fileOpenRecent, SIGNAL(loadFile(const QString&)),
							this, SLOT(slotFileOpenRecentDirectory(const QString&)));
#if QT_VERSION >= 0x040000
			m_fileOpenRecent->setStatusTip(i18n("Opens a recently used directory"));
			m_fileOpenRecent->setTitle(i18n("Open &Recent"));
			m_fileOpenRecent->setIcon(QIcon(":/images/document-open-recent.png"));
			fileMenu->addMenu(m_fileOpenRecent);
#else
			fileMenu->insertItem(i18n("Open &Recent"), m_fileOpenRecent);
#endif
		}
		QCM_addAction(fileMenu, fileOpenDirectory);
		fileMenu->QCM_addSeparator();
		QCM_addAction(fileMenu, fileSave);
		QCM_addAction(fileMenu, fileRevert);
		fileMenu->QCM_addSeparator();
		QCM_addAction(fileMenu, fileImport);
		QCM_addAction(fileMenu, fileImportFreedb);
		QCM_addAction(fileMenu, fileImportTrackType);
		QCM_addAction(fileMenu, fileImportDiscogs);
		QCM_addAction(fileMenu, fileImportAmazon);
		QCM_addAction(fileMenu, fileImportMusicBrainzRelease);
#ifdef HAVE_TUNEPIMP
		QCM_addAction(fileMenu, fileImportMusicBrainz);
#endif
		QCM_addAction(fileMenu, fileBrowseCoverArt);
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
		QCM_addAction(toolsMenu, toolsFilter);
#ifdef HAVE_TAGLIB
		QCM_addAction(toolsMenu, toolsConvertToId3v24);
#endif
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
		QCM_addAction(toolsMenu, toolsConvertToId3v23);
#endif
#ifdef HAVE_PHONON
		QCM_addAction(toolsMenu, toolsPlay);
#endif

#if QT_VERSION >= 0x040000
		QCM_addAction(settingsMenu, m_viewToolBar);
#endif

		QCM_addAction(settingsMenu, m_viewStatusBar);
		QCM_addAction(settingsMenu, m_settingsShowHidePicture);
		QCM_addAction(settingsMenu, m_settingsAutoHideTags);
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
	if (dir.isNull() || dir.isEmpty()) {
		return false;
	}
	QFileInfo file(dir);
	QString fileName;
	if (!file.isDir()) {
		if (fileCheck && !file.isFile()) {
			return false;
		}
#if QT_VERSION >= 0x040000
		dir = file.dir().path();
#else
		dir = file.dirPath(true);
#endif
		fileName = file.fileName();
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Opening directory..."));
	bool ok = m_view->readFileList(dir, fileName);
	if (ok) {
		m_view->readDirectoryList(dir);
		setModified(false);
		setFiltered(false);
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
		m_fileOpenRecent->addDirectory(dir);
		QCM_setWindowTitle(dir + " - Kid3");
#endif
		s_dirName = dir;
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
#if KDE_VERSION >= 0x035c00
	m_fileOpenRecent->saveEntries(KConfigGroup(m_config, "Recent Files"));
#else
	m_fileOpenRecent->saveEntries(m_config, "Recent Files");
#endif
#else
	m_fileOpenRecent->saveEntries(m_config);
#if QT_VERSION >= 0x040000
	s_miscCfg.m_hideToolBar = !m_viewToolBar->isChecked();
#endif
#if QT_VERSION >= 0x040200
	s_miscCfg.m_geometry = saveGeometry();
	s_miscCfg.m_windowState = saveState();
#else
	s_miscCfg.m_windowX = x();
	s_miscCfg.m_windowY = y();
	s_miscCfg.m_windowWidth = size().width();
	s_miscCfg.m_windowHeight = size().height();
#endif
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
		QTextCodec::codecForName(Kid3App::s_miscCfg.m_textEncodingV1.QCM_latin1()) : 0;
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
	if (s_miscCfg.m_hideStatusBar)
		statusBar()->hide();
#if QT_VERSION >= 0x040000
	m_viewStatusBar->setChecked(!s_miscCfg.m_hideStatusBar);
	m_settingsShowHidePicture->setChecked(!s_miscCfg.m_hidePicture);
	m_settingsAutoHideTags->setChecked(s_miscCfg.m_autoHideTags);
#else
	m_viewStatusBar->setOn(!s_miscCfg.m_hideStatusBar);
	m_settingsShowHidePicture->setOn(!s_miscCfg.m_hidePicture);
	m_settingsAutoHideTags->setOn(s_miscCfg.m_autoHideTags);
#endif
	m_fileOpenRecent->loadEntries(m_config);
#if QT_VERSION >= 0x040200
	restoreGeometry(s_miscCfg.m_geometry);
	restoreState(s_miscCfg.m_windowState);
#else
	if (s_miscCfg.m_windowWidth != -1 && s_miscCfg.m_windowHeight != -1) {
		resize(s_miscCfg.m_windowWidth, s_miscCfg.m_windowHeight);
	}
	if (s_miscCfg.m_windowX != -1 && s_miscCfg.m_windowY != -1) {
		move(s_miscCfg.m_windowX, s_miscCfg.m_windowY);
	}
#endif
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
void Kid3App::readProperties(const KConfigGroup& cfg)
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

/**
 * Read font and style options.
 */
void Kid3App::readFontAndStyleOptions()
{
	s_miscCfg.readFromConfig(m_config);
	if (s_miscCfg.m_useFont &&
			!s_miscCfg.m_fontFamily.isEmpty() && s_miscCfg.m_fontSize > 0) {
		QApplication::setFont(QFont(s_miscCfg.m_fontFamily, s_miscCfg.m_fontSize)
#if QT_VERSION < 0x040000
													, true
#endif
			);
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
				QMessageBox::Ok, QCM_NoButton);
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
	QString lc(str.QCM_toLower());
	QString uc(str.QCM_toUpper());

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
		QString text = (*it).mid(1).QCM_toUpper();
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
void Kid3App::slotFileOpenRecentDirectory(const QString&) {}
#else
void Kid3App::slotFileOpenRecent(const KURL& url)
{
	updateCurrentSelection();
	QString dir = url.path();
	openDirectory(dir, true);
}

void Kid3App::slotFileOpenRecentUrl(const KUrl&) {}
void Kid3App::slotFileOpenRecentDirectory(const QString&) {}
#endif
#else /* CONFIG_USE_KDE */
void Kid3App::slotFileOpenRecent(const KURL&) {}
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
void Kid3App::slotSettingsShortcuts() {}
void Kid3App::slotSettingsToolbars() {}

/**
 * Turn status bar on or off.
 */
void Kid3App::slotViewStatusBar()
{
#if QT_VERSION >= 0x040000
	s_miscCfg.m_hideStatusBar = !m_viewStatusBar->isChecked();
#else
	s_miscCfg.m_hideStatusBar = !m_viewStatusBar->isOn();
#endif
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
		"\n(c) 2003-2010 Urs Fleisch\nufleisch@users.sourceforge.net");
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
	PlaylistCreator plCtr(m_view->getDirInfo()->getDirname(), cfg);
	QString selectedDirPrefix;
	FileListItem* item = cfg.m_location == PlaylistConfig::PL_CurrentDirectory ?
		m_view->firstFileInDir() : m_view->firstFileOrDir();
	bool noSelection = !cfg.m_onlySelectedFiles ||
		m_view->numFilesOrDirsSelected() == 0;
	bool ok = true;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	slotStatusMsg(i18n("Creating playlist..."));
	while (item != 0) {
		PlaylistCreator::Item plItem(item, plCtr);
		bool inSelectedDir = false;
		if (cfg.m_location != PlaylistConfig::PL_CurrentDirectory &&
				plItem.isDir()) {
			if (!selectedDirPrefix.isEmpty()) {
				if (plItem.getDirName().startsWith(selectedDirPrefix)) {
					inSelectedDir = true;
				} else {
					selectedDirPrefix = "";
				}
			}
			if (inSelectedDir || noSelection || item->isSelected()) {
				// if directory is selected, all its files are selected
#if QT_VERSION >= 0x040000
				item->setExpanded(true);
#else
				item->setOpen(true);
#endif
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
			if (inSelectedDir || noSelection || item->isSelected()) {
				ok = plItem.add() && ok;
			}
		}
		item = cfg.m_location == PlaylistConfig::PL_CurrentDirectory ?
			m_view->nextFileInDir() : m_view->nextFileOrDir();
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
	m_trackDataList.clearData();
	FileListItem* mp3file = m_view->firstFileInDir();
	bool firstTrack = true;
	bool tag1Supported = true;
	while (mp3file != 0) {
		mp3file->getFile()->readTags(false);
		if (firstTrack) {
			FrameCollection frames;
			mp3file->getFile()->getAllFramesV2(frames);
			QString artist = frames.getArtist();
			QString album = frames.getAlbum();
			if (artist.isEmpty() && album.isEmpty()) {
				mp3file->getFile()->getAllFramesV1(frames);
				artist = frames.getArtist();
				album = frames.getAlbum();
			}
			m_trackDataList.setArtist(artist);
			m_trackDataList.setAlbum(album);
			firstTrack = false;
			tag1Supported = mp3file->getFile()->isTagV1Supported();
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
		if (!tag1Supported &&
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
	ImportTrackDataVector::iterator it = m_trackDataList.begin();
	FrameFilter flt(destV1 ?
	                m_view->frameTableV1()->getEnabledFrameFilter(true) :
	                m_view->frameTableV2()->getEnabledFrameFilter(true));
	bool no_selection = m_view->numFilesSelected() == 0;
	FileListItem* mp3file = m_view->firstFileInDir();
	while (mp3file != 0) {
		TaggedFile* taggedFile = mp3file->getFile();
		taggedFile->readTags(false);
		if (it != m_trackDataList.end()) {
			it->removeDisabledFrames(flt);
			formatFramesIfEnabled(*it);
			if (destV1) taggedFile->setFramesV1(*it, false);
			if (destV2) taggedFile->setFramesV2(*it, false);
			++it;
		} else {
			break;
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

	if (destV2 && flt.isEnabled(Frame::FT_Picture) &&
	    !m_trackDataList.getCoverArtUrl().isEmpty()) {
		downloadImage(m_trackDataList.getCoverArtUrl(), true);
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
 *
 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
 * @param path    path of file
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool Kid3App::importTags(int tagMask, const QString& path, int fmtIdx)
{
	setupImportDialog();
	if (m_importDialog) {
		m_importDialog->setAutoStartSubDialog(ImportDialog::ASD_None);
		m_importDialog->setFormatLineEdit(fmtIdx);
		if (m_importDialog->importFromFile(path)) {
			getTagsFromImportDialog((tagMask & 1) != 0, (tagMask & 2) != 0);
			return true;
		}
	}
	return false;
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
		FileListItem* item;
		TaggedFile* taggedFile;
		if ((item = m_view->currentFile()) != 0 &&
				(taggedFile = item->getFile()) != 0) {
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

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
/**
 * Read file with TagLib if it has an ID3v2.4 tag.
 *
 * @param item       file list item, can be updated if not null
 * @param taggedFile tagged file
 *
 * @return tagged file (can be new TagLibFile).
 */
TaggedFile* Kid3App::readWithTagLibIfId3V24(FileListItem* item,
																					TaggedFile* taggedFile)
{
	if (dynamic_cast<Mp3File*>(taggedFile) != 0 &&
			!taggedFile->isChanged() &&
			taggedFile->isTagInformationRead() && taggedFile->hasTagV2() &&
			taggedFile->getTagFormatV2() == QString::null) {
		TagLibFile* tagLibFile;
		if ((tagLibFile = new TagLibFile(
					 taggedFile->getDirInfo(),
					 taggedFile->getFilename())) != 0) {
			if (item)
				item->setFile(tagLibFile);
			taggedFile = tagLibFile;
			taggedFile->readTags(false);
		}
	}
	return taggedFile;
}
#endif

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
		while (mp3file != 0) {
			TaggedFile* taggedFile = mp3file->getFile();
			if (taggedFile) {
				taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				taggedFile = readWithTagLibIfId3V24(mp3file, taggedFile);
#endif
				ImportTrackData trackData(taggedFile->getAbsFilename(),
																	taggedFile->getDuration());
				trackData.setFileExtension(taggedFile->getFileExtension());
				trackData.setTagFormatV1(taggedFile->getTagFormatV1());
				trackData.setTagFormatV2(taggedFile->getTagFormatV2());
				TaggedFile::DetailInfo info;
				taggedFile->getDetailInfo(info);
				trackData.setDetailInfo(info);
				if (src == ExportDialog::SrcV1) {
					taggedFile->getAllFramesV1(trackData);
				} else {
					taggedFile->getAllFramesV2(trackData);
				}
				trackDataVector.push_back(trackData);
			}
			mp3file = m_view->nextFileInDir();
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
#if QT_VERSION >= 0x040000
	s_miscCfg.m_autoHideTags = m_settingsAutoHideTags->isChecked();
#else
	s_miscCfg.m_autoHideTags = m_settingsAutoHideTags->isOn();
#endif
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
#if QT_VERSION >= 0x040000
	s_miscCfg.m_hidePicture = !m_settingsShowHidePicture->isChecked();
#else
	s_miscCfg.m_hidePicture = !m_settingsShowHidePicture->isOn();
#endif
#endif

	m_view->hidePicture(s_miscCfg.m_hidePicture);
#if QT_VERSION >= 0x040000
	// In Qt3 the picture is displayed too small if Kid3 is started with picture
	// hidden, and then "Show Picture" is triggered while a file with a picture
	// is selected. Thus updating the controls is only done for Qt4, in Qt3 the
	// file has to be selected again for the picture to be shown.
	if (!s_miscCfg.m_hidePicture) {
		updateGuiControls();
	}
#endif
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
			if (!s_miscCfg.m_markChanges) {
				m_view->frameTableV1()->markChangedFrames(0);
				m_view->frameTableV2()->markChangedFrames(0);
				m_view->markChangedFilename(false);
			}
			setTextEncodings();
#if QT_VERSION < 0x040000
			m_view->frameTableV1()->triggerUpdateGenres();
			m_view->frameTableV2()->triggerUpdateGenres();
#endif
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
	updateCurrentSelection();
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
	FrameCollection frames;
	updateCurrentSelection();
	FrameFilter fltV1(m_view->frameTableV1()->getEnabledFrameFilter(true));
	FrameFilter fltV2(m_view->frameTableV2()->getEnabledFrameFilter(true));
	FileListItem* mp3file = m_view->firstFile();
	bool no_selection = m_view->numFilesSelected() == 0;
	while (mp3file != 0) {
		if (no_selection || mp3file->isInSelection()) {
			TaggedFile* taggedFile = mp3file->getFile();
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
		mp3file = m_view->nextFile();
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
		TaggedFile* taggedFile;
		FileListItem* item = m_view->firstFileOrDir();
		while (item) {
			if (item->getDirInfo()) {
#if QT_VERSION >= 0x040000
				item->setExpanded(true);
#else
				item->setOpen(true);
#endif
			} else if ((taggedFile = item->getFile()) != 0) {
				taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				taggedFile = readWithTagLibIfId3V24(item, taggedFile);
#endif
				m_renDirDialog->scheduleAction(taggedFile);
			}
			item = m_view->nextFileOrDir();
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
	if (!isModified() && m_view->firstFileInDir()) {
		if (!m_renDirDialog) {
			m_renDirDialog = new RenDirDialog(0);
			connect(m_renDirDialog, SIGNAL(actionSchedulingRequested()),
							this, SLOT(scheduleRenameActions()));
		}
		if (m_renDirDialog) {
			m_renDirDialog->startDialog(m_view->firstFileInDir()->getFile());
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
			FileListItem* item = m_view->currentFile();
			if (item && item->isSelected()) {
				TaggedFile* taggedFile;
				const DirInfo* dirInfo = item->getDirInfo();
				if (dirInfo) {
#if QT_VERSION >= 0x040000
					item->setExpanded(true);
#else
					item->setOpen(true);
#endif
				} else if ((taggedFile = item->getFile()) != 0) {
					dirInfo = taggedFile->getDirInfo();
				}
				if (dirInfo) {
					openDirectory(dirInfo->getDirname());
				}
			}
			item = m_view->firstFileInDir();
			if (item) {
				m_renDirDialog->startDialog(item->getFile());
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
															 QMessageBox::Ok, QCM_NoButton);
				}
			}

		}
	}
}

/**
 * Number tracks in selected files of directory.
 *
 * @param nr start number
 * @param destV1 true to set numbers in tag 1
 * @param destV2 true to set numbers in tag 2
 */
void Kid3App::numberTracks(int nr, bool destV1, bool destV2)
{
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
			numberTracks(nr, destV1, destV2);
		}
	}
}

/**
 * Apply a file filter to a directory.
 *
 * @param fileFilter filter to apply
 * @param dirContents directory contents, will be filled with results
 *
 * @return true if ok, false if aborted.
 */
bool Kid3App::applyFilterToDir(FileFilter& fileFilter, DirContents* dirContents)
{
	bool ok = true;
	int numFiles = 0;
	QString dirname = dirContents->getDirname();
	QDir dir(dirname);
#if QT_VERSION >= 0x040000
	const QDir::Filters dirFilters =
		QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files;
	QStringList nameFilters(Kid3App::s_miscCfg.m_nameFilter.split(' '));
	QStringList dirEntries = dir.entryList(
		nameFilters, dirFilters, QDir::DirsFirst | QDir::IgnoreCase);
#else
	QStringList dirEntries = dir.entryList(QDir::Dirs) +
		dir.entryList(Kid3App::s_miscCfg.m_nameFilter, QDir::Files);
#endif
	for (QStringList::Iterator it = dirEntries.begin();
			 it != dirEntries.end(); ++it) {
		QString filename = dirname + QDir::separator() + *it;
		if (!QFileInfo(filename).isDir()) {
			TaggedFile* taggedFile = TaggedFile::createFile(dirContents, *it);
			if (taggedFile) {
				taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				taggedFile = readWithTagLibIfId3V24(0, taggedFile);
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
						(pass ? QString("+\t") : QString("-\t")) + *it);
				}
				if (pass) {
					dirContents->files().append(*it);
				}
				++numFiles;
				delete taggedFile;
			}
		} else {
			if (*it != "." && *it != "..") {
				DirContents* subDirContents = new DirContents(filename);
				ok = applyFilterToDir(fileFilter, subDirContents);
				if (subDirContents->getFiles().size() > 0 ||
						subDirContents->getDirs().size() > 0) {
					dirContents->dirs().append(subDirContents);
				} else {
					delete subDirContents; 
				}
				if (!ok) {
					break;
				}
			}
		}
	}
	dirContents->setNumFiles(numFiles);

#ifdef CONFIG_USE_KDE
	kapp->processEvents();
#else
	qApp->processEvents();
#endif
	return ok && !(m_filterDialog && m_filterDialog->getAbortFlag());
}

/**
 * Apply a file filter.
 *
 * @param fileFilter filter to apply.
 */
void Kid3App::applyFilter(FileFilter& fileFilter)
{
	const DirInfo* dirInfo = m_view->getDirInfo();
	if (!dirInfo) return;
	QString dirname = dirInfo->getDirname();

	if (isFiltered()) {
		m_view->readFileList(dirname);
		setFiltered(false);
	}

	if (m_filterDialog) {
		m_filterDialog->clearAbortFlag();
	}

	DirContents dirContents(dirname);
	applyFilterToDir(fileFilter, &dirContents);

	m_view->getFileList()->setFromDirContents(dirContents);
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
						FrameCollection frames;
						taggedFile->getAllFramesV2(frames);
						FrameFilter flt;
						flt.enableAll();
						taggedFile->deleteFramesV2(flt);

						// The file has to be read with TagLib to write ID3v2.4 tags
						TagLibFile* tagLibFile;
						if ((tagLibFile = new TagLibFile(
									 taggedFile->getDirInfo(),
									 taggedFile->getFilename())) != 0) {
							item->setFile(tagLibFile);
							taggedFile = tagLibFile;
							taggedFile->readTags(false);
						}

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
	updateCurrentSelection();
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
						FrameCollection frames;
						taggedFile->getAllFramesV2(frames);
						FrameFilter flt;
						flt.enableAll();
						taggedFile->deleteFramesV2(flt);

						// The file has to be read with id3lib to write ID3v2.3 tags
						Mp3File* id3libFile;
						if ((id3libFile = new Mp3File(
									 taggedFile->getDirInfo(),
									 taggedFile->getFilename())) != 0) {
							item->setFile(id3libFile);
							taggedFile = id3libFile;
							taggedFile->readTags(false);
						}

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
		item = m_view->nextFile();
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
	FileListItem* item = m_view->firstFile();

	if (m_view->numFilesSelected() > 1) {
		// play only the selected files if more than one is selected
		while (item != 0) {
			if (item->isInSelection()) {
				files.push_back(item->getFile()->getAbsFilename());
			}
			item = m_view->nextFile();
		}
	} else {
		// play all files if none or only one is selected
		int idx = 0;
		while (item != 0) {
			files.push_back(item->getFile()->getAbsFilename());
			if (item->isInSelection()) {
				fileNr = idx;
			}
			item = m_view->nextFile();
			++idx;
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
	int lfPos = txt.QCM_indexOf('\n');
	if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
		txt.truncate(lfPos + 1);
	}
	QUrl url(txt);
	if (!url.path().isEmpty()) {
#if defined _WIN32 || defined WIN32
		QString dir = url.toString();
#else
		QString dir = url.path().QCM_trimmed();
#endif
		if (dir.endsWith(".jpg", QCM_CaseInsensitive) ||
				dir.endsWith(".jpeg", QCM_CaseInsensitive) ||
				dir.endsWith(".png", QCM_CaseInsensitive)) {
			PictureFrame frame;
			if (PictureFrame::setDataFromFile(frame, dir)) {
				QString fileName(dir);
				int slashPos = fileName.QCM_lastIndexOf('/');
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
 * @param url           URL of image
 * @param allFilesInDir true to add the image to all files in the directory
 */
void Kid3App::downloadImage(const QString& url, bool allFilesInDir)
{
	QString imgurl(BrowseCoverArtDialog::getImageUrl(url));
	if (!imgurl.isEmpty()) {
		if (!m_downloadDialog) {
			m_downloadDialog = new DownloadDialog(0, i18n("Download"));
			connect(m_downloadDialog, SIGNAL(downloadFinished(const QByteArray&, const QString&, const QString&)),
							this, SLOT(imageDownloaded(const QByteArray&, const QString&, const QString&)));
		}
		if (m_downloadDialog) {
			int hostPos = imgurl.QCM_indexOf("://");
			if (hostPos > 0) {
				int pathPos = imgurl.QCM_indexOf("/", hostPos + 3);
				if (pathPos > hostPos) {
					m_downloadToAllFilesInDir = allFilesInDir;
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
	downloadImage(txt, false);
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
		if (m_downloadToAllFilesInDir) {
			FileListItem* mp3file = m_view->firstFileInDir();
			while (mp3file != 0) {
				TaggedFile* taggedFile = mp3file->getFile();
				taggedFile->readTags(false);
				taggedFile->addFrameV2(frame);
				mp3file = m_view->nextFileInDir();
			}
			m_downloadToAllFilesInDir = false;
		} else {
			addFrame(&frame);
		}
		updateGuiControls();
	}
}

/**
 * Update modification state, caption and listbox entries.
 */
void Kid3App::updateModificationState()
{
	setModified(m_view->updateModificationState());
	QString cap(s_dirName);
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
	QCM_setWindowTitle(cap);
#endif
}

/**
 * Update files of current selection.
 */
void Kid3App::updateCurrentSelection()
{
#if QT_VERSION >= 0x040000
	const QList<QTreeWidgetItem*>& selItems =
		m_view->getFileList()->getCurrentSelection();
	int numFiles = selItems.size();
	if (numFiles > 0) {
		m_view->frameTableV1()->tableToFrames(numFiles > 1);
		m_view->frameTableV2()->tableToFrames(numFiles > 1);
		for (QList<QTreeWidgetItem*>::const_iterator it = selItems.begin();
				 it != selItems.end();
				 ++it) {
			FileListItem* item = dynamic_cast<FileListItem*>(*it);
			if (item) {
				TaggedFile* taggedFile = item->getFile();
				if (taggedFile) {
					taggedFile->setFramesV1(m_view->frameTableV1()->frames());
					taggedFile->setFramesV2(m_view->frameTableV2()->frames());
					if (m_view->isFilenameEditEnabled()) {
						taggedFile->setFilename(m_view->getFilename());
					}
				}
			}
		}
	}
	updateModificationState();
#else
	int numFiles = 0;
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (++numFiles > 1) {
				// we are only interested if 0, 1 or multiple files are selected
				break;
			}
		}
		mp3file = m_view->nextFile();
	}
	if (numFiles > 0) {
		m_view->frameTableV1()->tableToFrames(numFiles > 1);
		m_view->frameTableV2()->tableToFrames(numFiles > 1);
		mp3file = m_view->firstFile();
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				TaggedFile* taggedFile = mp3file->getFile();
				taggedFile->setFramesV1(m_view->frameTableV1()->frames());
				taggedFile->setFramesV2(m_view->frameTableV2()->frames());
				if (m_view->isFilenameEditEnabled()) {
					taggedFile->setFilename(m_view->getFilename());
				}
			}
			mp3file = m_view->nextFile();
		}
	}
	updateModificationState();
#endif
}

/**
 * Update GUI controls from the tags in the files.
 * The new selection is stored and the GUI controls and frame list
 * updated accordingly (filtered for multiple selection).
 */
void Kid3App::updateGuiControls()
{
#if QT_VERSION >= 0x040000
	TaggedFile* single_v2_file = 0;
	int num_v1_selected = 0;
	int num_v2_selected = 0;
	bool tagV1Supported = false;
	bool hasTagV1 = false;
	bool hasTagV2 = false;

	m_view->getFileList()->updateCurrentSelection();
	const QList<QTreeWidgetItem*>& selItems =
		m_view->getFileList()->getCurrentSelection();

	for (QList<QTreeWidgetItem*>::const_iterator it = selItems.begin();
			 it != selItems.end();
			 ++it) {
		FileListItem* mp3file = dynamic_cast<FileListItem*>(*it);
		if (mp3file) {
			TaggedFile* taggedFile = mp3file->getFile();
			if (taggedFile) {
				taggedFile->readTags(false);

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				taggedFile = readWithTagLibIfId3V24(mp3file, taggedFile);
#endif

				if (taggedFile->isTagV1Supported()) {
					if (num_v1_selected == 0) {
						taggedFile->getAllFramesV1(m_view->frameTableV1()->frames());
					}
					else {
						FrameCollection fileFrames;
						taggedFile->getAllFramesV1(fileFrames);
						m_view->frameTableV1()->frames().filterDifferent(fileFrames);
					}
					++num_v1_selected;
					tagV1Supported = true;
				}
				if (num_v2_selected == 0) {
					taggedFile->getAllFramesV2(m_view->frameTableV2()->frames());
					single_v2_file = taggedFile;
				}
				else {
					FrameCollection fileFrames;
					taggedFile->getAllFramesV2(fileFrames);
					m_view->frameTableV2()->frames().filterDifferent(fileFrames);
					single_v2_file = 0;
				}
				++num_v2_selected;

				hasTagV1 = hasTagV1 || taggedFile->hasTagV1();
				hasTagV2 = hasTagV2 || taggedFile->hasTagV2();
			}
		}
	}
#else
	FileListItem* mp3file = m_view->firstFile();
	TaggedFile* single_v2_file = 0;
	int num_v1_selected = 0;
	int num_v2_selected = 0;
	bool tagV1Supported = false;
	bool hasTagV1 = false;
	bool hasTagV2 = false;

	while (mp3file != 0) {
		if (mp3file->isSelected()) {
			mp3file->setInSelection(true);
			TaggedFile* taggedFile = mp3file->getFile();
			if (taggedFile) {
				taggedFile->readTags(false);

#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
				taggedFile = readWithTagLibIfId3V24(mp3file, taggedFile);
#endif

				if (taggedFile->isTagV1Supported()) {
					if (num_v1_selected == 0) {
						taggedFile->getAllFramesV1(m_view->frameTableV1()->frames());
					}
					else {
						FrameCollection fileFrames;
						taggedFile->getAllFramesV1(fileFrames);
						m_view->frameTableV1()->frames().filterDifferent(fileFrames);
					}
					++num_v1_selected;
					tagV1Supported = true;
				}
				if (num_v2_selected == 0) {
					taggedFile->getAllFramesV2(m_view->frameTableV2()->frames());
					single_v2_file = taggedFile;
				}
				else {
					FrameCollection fileFrames;
					taggedFile->getAllFramesV2(fileFrames);
					m_view->frameTableV2()->frames().filterDifferent(fileFrames);
					single_v2_file = 0;
				}
				++num_v2_selected;

				hasTagV1 = hasTagV1 || taggedFile->hasTagV1();
				hasTagV2 = hasTagV2 || taggedFile->hasTagV2();
			}
		}
		else {
			mp3file->setInSelection(false);
		}
		mp3file = m_view->nextFile();
	}
#endif

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
			m_view->frameTableV1()->markRows(single_v2_file->getTruncationFlags());
		}
		if (s_miscCfg.m_markChanges) {
			m_view->frameTableV1()->markChangedFrames(
				single_v2_file->getChangedFramesV1());
			m_view->frameTableV2()->markChangedFrames(
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
			m_view->frameTableV1()->markRows(0);
		}
		if (s_miscCfg.m_markChanges) {
			m_view->frameTableV1()->markChangedFrames(0);
			m_view->frameTableV2()->markChangedFrames(0);
			m_view->markChangedFilename(false);
		}
	}
	if (!s_miscCfg.m_hidePicture) {
		FrameCollection::const_iterator it =
			m_view->frameTableV2()->frames().find(Frame(Frame::FT_Picture, "", "", -1));
		if (it == m_view->frameTableV2()->frames().end() ||
				it->isInactive()) {
			m_view->setPictureData(0);
		} else {
			QByteArray data;
			m_view->setPictureData(PictureFrame::getData(*it, data) ? &data : 0);
		}
	}
	m_view->frameTableV1()->setAllCheckBoxes(num_v1_selected == 1);
	m_view->frameTableV1()->framesToTable();
	m_view->frameTableV2()->setAllCheckBoxes(num_v2_selected == 1);
	m_view->frameTableV2()->framesToTable();
	updateModificationState();

	if (num_v1_selected == 0 && num_v2_selected == 0) {
		tagV1Supported = true;
	}
	m_view->enableControlsV1(tagV1Supported);

	if (s_miscCfg.m_autoHideTags) {
		// If a tag is supposed to be absent, make sure that there is really no
		// unsaved data in the tag.
		if (!hasTagV1 && tagV1Supported) {
			FrameCollection& frames = m_view->frameTableV1()->frames();
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
			FrameCollection& frames = m_view->frameTableV2()->frames();
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
	m_copyTags = m_view->frameTableV1()->frames().copyEnabledFrames(
		m_view->frameTableV1()->getEnabledFrameFilter(true));
}

/**
 * Copy tags 2 into copy buffer.
 */
void Kid3App::copyTagsV2()
{
	updateCurrentSelection();
	m_copyTags = m_view->frameTableV2()->frames().copyEnabledFrames(
		m_view->frameTableV2()->getEnabledFrameFilter(true));
}

/**
 * Paste from copy buffer to ID3v1 tags.
 */
void Kid3App::pasteTagsV1()
{
	updateCurrentSelection();
	FrameCollection frames(m_copyTags.copyEnabledFrames(
													 m_view->frameTableV1()->getEnabledFrameFilter(true)));
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
													 m_view->frameTableV2()->getEnabledFrameFilter(true)));
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
	FrameCollection frames;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	FrameFilter flt(m_view->frameTableV1()->getEnabledFrameFilter(true));
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && m_view->isFilenameEditEnabled()) {
				mp3file->getFile()->setFilename(
					m_view->getFilename());
			}
			mp3file->getFile()->getAllFramesV1(frames);
			mp3file->getFile()->getTagsFromFilename(frames,
			                                        m_view->getFromFilenameFormat());
			frames.removeDisabledFrames(flt);
			formatFramesIfEnabled(frames);
			mp3file->getFile()->setFramesV1(frames);
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
	FrameCollection frames;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	FrameFilter flt(m_view->frameTableV2()->getEnabledFrameFilter(true));
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (!multiselect && m_view->isFilenameEditEnabled()) {
				mp3file->getFile()->setFilename(
					m_view->getFilename());
			}
			mp3file->getFile()->getAllFramesV2(frames);
			mp3file->getFile()->getTagsFromFilename(frames,
			                                        m_view->getFromFilenameFormat());
			frames.removeDisabledFrames(flt);
			formatFramesIfEnabled(frames);
			mp3file->getFile()->setFramesV2(frames);
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
	FrameCollection frames;
	FileListItem* mp3file = m_view->firstFile();
	bool multiselect = m_view->numFilesSelected() > 1;
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			if (tag_version == 2) {
				mp3file->getFile()->getAllFramesV2(frames);
			}
			else {
				mp3file->getFile()->getAllFramesV1(frames);
			}
			if (!frames.isEmptyOrInactive()) {
				mp3file->getFile()->getFilenameFromTags(
					frames, m_view->getFilenameFormat());
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
	FrameCollection frames;
	FrameFilter flt(m_view->frameTableV2()->getEnabledFrameFilter(true));
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getAllFramesV1(frames);
			frames.removeDisabledFrames(flt);
			formatFramesIfEnabled(frames);
			mp3file->getFile()->setFramesV2(frames, false);
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
	FrameCollection frames;
	FrameFilter flt(m_view->frameTableV1()->getEnabledFrameFilter(true));
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->getAllFramesV2(frames);
			frames.removeDisabledFrames(flt);
			formatFramesIfEnabled(frames);
			mp3file->getFile()->setFramesV1(frames, false);
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
	FrameFilter flt(m_view->frameTableV1()->getEnabledFrameFilter(true));
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->deleteFramesV1(flt);
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
	FrameFilter flt(m_view->frameTableV2()->getEnabledFrameFilter(true));
	FileListItem* mp3file = m_view->firstFile();
	while (mp3file != 0) {
		if (mp3file->isInSelection()) {
			mp3file->getFile()->deleteFramesV2(flt);
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
		TaggedFile* currentFile;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				currentFile = mp3file->getFile();
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
			mp3file = m_view->nextFile();
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
		FileListItem* mp3file = m_view->firstFile();
		bool firstFile = true;
		QString name;
		TaggedFile* currentFile;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				currentFile = mp3file->getFile();
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
			mp3file = m_view->nextFile();
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
		FileListItem* mp3file = m_view->firstFile();
		bool firstFile = true;
		int frameId = -1;
		while (mp3file != 0) {
			if (mp3file->isInSelection()) {
				if (firstFile) {
					firstFile = false;
					taggedFile = mp3file->getFile();
					m_framelist->setTags(mp3file->getFile());
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
