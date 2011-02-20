/**
 * \file qtcompatmac.h
 * Qt compatibility macros.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 31 Oct 2006
 *
 * Copyright (C) 2006-2009  Urs Fleisch
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

#ifndef QTCOMPATMAC_H
#define QTCOMPATMAC_H

#include <qglobal.h>
#include <qobject.h>
#include "config.h"

#ifdef CONFIG_USE_KDE
#include <klocale.h>
#include <kdeversion.h>
#if KDE_VERSION >= 0x035c00

#define KCM_ICON_document_open "document-open"
#define KCM_ICON_go_previous "go-previous"
#define KCM_ICON_go_next "go-next"
#define KCM_ICON_document_import "document-import"
#define KCM_ICON_document_export "document-export"
#define KCM_ICON_media_playlist "view-media-playlist"
#define KCM_ICON_preferences_tags "applications-multimedia"
#define KCM_ICON_preferences_files "document-save"
#define KCM_ICON_preferences_useractions "preferences-other"
#define KCM_ICON_preferences_network "preferences-system-network"
#define KCM_ICON_media_playback_start "media-playback-start"

#define KCM_setStatusTip setStatusTip
#define KCM_addUrl addUrl
#define KCM_KStandardAction KStandardAction
#define KCM_KActionShortcutIcon(var, cut, icon, text, rcvr, slot, parent, name) \
	KAction* var = new KAction(KIcon(icon), text, this); \
	var->setShortcut(cut); \
	parent->addAction(name, var); \
	connect(var, SIGNAL(triggered()), rcvr, slot)
#define KCM_KActionIcon(var, icon, text, rcvr, slot, parent, name) \
	KAction* var = new KAction(KIcon(icon), text, this); \
	parent->addAction(name, var); \
	connect(var, SIGNAL(triggered()), rcvr, slot)
#define KCM_KActionShortcut(var, cut, text, rcvr, slot, parent, name)	\
	KAction* var = new KAction(text, this); \
	var->setShortcut(cut); \
	parent->addAction(name, var); \
	connect(var, SIGNAL(triggered()), rcvr, slot)
#define KCM_KActionVar(var, text, rcvr, slot, parent, name)	\
	var = new KAction(text, this); \
	parent->addAction(name, var); \
	connect(var, SIGNAL(triggered()), rcvr, slot)
#define KCM_KAction(var, text, rcvr, slot, parent, name)	\
	KAction* var = new KAction(text, this); \
	parent->addAction(name, var); \
	connect(var, SIGNAL(triggered()), rcvr, slot)
#define KCM_KToggleActionVar(var, text, rcvr, slot, parent, name)	\
	var = new KToggleAction(text, this); \
	var->setCheckable(true); \
	parent->addAction(name, var); \
	connect(var, SIGNAL(triggered()), rcvr, slot)
#define KCM_KConfigGroup(var, cfgptr, name) KConfigGroup var = cfgptr->group(name)
#define KCM_readBoolEntry readEntry
#define KCM_readNumEntry readEntry
#define KCM_readListEntry(key) readEntry(key, QStringList())
#define KCM_readIntListEntry(key) readEntry(key, QList<int>())

#define KCM_i18n1(s, a1) i18n(s, a1)
#define KCM_i18n2(s, a1, a2) i18n(s, a1, a2)

#else

#define KCM_ICON_document_open "fileopen"
#define KCM_ICON_go_previous "previous"
#define KCM_ICON_go_next "next"
#define KCM_ICON_document_import "fileimport"
#define KCM_ICON_document_export "fileexport"
#define KCM_ICON_media_playlist "player_playlist"
#define KCM_ICON_preferences_tags "package_multimedia"
#define KCM_ICON_preferences_files "package_system"
#define KCM_ICON_preferences_useractions "package_utilities"
#define KCM_ICON_preferences_network "package_network"
#define KCM_ICON_media_playback_start "player_play"

#define KCM_setStatusTip setStatusText
#define KCM_addUrl addURL
#define KCM_KStandardAction KStdAction
#define KCM_KActionShortcutIcon(var, cut, icon, text, rcvr, slot, parent, name) \
	KAction* var = new KAction(text, cut, rcvr, slot, parent, name); \
	var->setIcon(icon)
#define KCM_KActionIcon(var, icon, text, rcvr, slot, parent, name) \
	KAction* var = new KAction(text, 0, rcvr, slot, parent, name); \
	var->setIcon(icon)
#define KCM_KActionShortcut(var, cut, text, rcvr, slot, parent, name)	\
	new KAction(text, cut, rcvr, slot, parent, name)
#define KCM_KActionVar(var, text, rcvr, slot, parent, name)	\
	var = new KAction(text, 0, rcvr, slot, parent, name)
#define KCM_KAction(var, text, rcvr, slot, parent, name)	\
	new KAction(text, 0, rcvr, slot, parent, name)
#define KCM_KToggleActionVar(var, text, rcvr, slot, parent, name)	\
	var = new KToggleAction(text, 0, rcvr, slot, parent, name)
#define KCM_KConfigGroup(var, cfgptr, name) KConfig& var = *cfgptr; cfgptr->setGroup(name)
#define KCM_readBoolEntry readBoolEntry
#define KCM_readNumEntry readNumEntry
#define KCM_readListEntry(key) readListEntry(key)
#define KCM_readIntListEntry(key) readIntListEntry(key)

#define KCM_i18n1(s, a1) i18n(s).arg(a1)
#define KCM_i18n2(s, a1, a2) i18n(s).arg(a1).arg(a2)

#endif
#define QCM_translate(s) i18n(s)
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TRANSLATE_NOOP("@default", s)
#define KCM_i18n1(s, a1) tr(s).arg(a1)
#define KCM_i18n2(s, a1, a2) tr(s).arg(a1).arg(a2)
#if QT_VERSION >= 0x040000
#include <QCoreApplication>
#define QCM_translate(s) QCoreApplication::translate("@default", s)
#else
#include <qapplication.h>
#define QCM_translate(s) qApp->translate("@default", s)
#endif
#endif

#if QT_VERSION >= 0x040000

#define QCM_WriteOnly QIODevice::WriteOnly
#define QCM_ReadOnly QIODevice::ReadOnly
#define QCM_NoButton Qt::NoButton
#define QCM_CaseInsensitive Qt::CaseInsensitive

#define QCM_indexIn indexIn
#define QCM_addItems addItems
#define QCM_addItem addItem
#define QCM_toLower toLower
#define QCM_toUpper toUpper
#define QCM_absoluteFilePath absoluteFilePath
#define QCM_absolutePath absolutePath
#define QCM_indexOf indexOf
#define QCM_lastIndexOf lastIndexOf
#define QCM_setWindowTitle setWindowTitle
#define QCM_setMaximum setMaximum
#define QCM_setMinimum setMinimum
#define QCM_currentPath currentPath
#define QCM_setWindowIcon setWindowIcon
#define QCM_setMenuText setText
#define QCM_setShortcut setShortcut
#define QCM_addAction(w, a) (w)->addAction(a)
#define QCM_setToolTip(w, t) (w)->setToolTip(t)
#define QCM_addSeparator addSeparator
#define QCM_showMessage showMessage
#define QCM_clearMessage clearMessage
#define QCM_trimmed trimmed
#define QCM_writeEntry(k, v) setValue(k, QVariant(v))
#define QCM_readEntry(k, d) value(k, d).toString()
#define QCM_readBoolEntry(k, d) value(k, d).toBool()
#define QCM_readNumEntry(k, d) value(k, d).toInt()
#define QCM_readListEntry(k) value(k).toStringList()
#define QCM_removeEntry remove
#define QCM_insertItem(i, t) insertItem(i, t)
#define QCM_currentIndex currentIndex
#define QCM_setCurrentIndex setCurrentIndex
#define QCM_cleanPath cleanPath
#define QCM_latin1() toLatin1().data()
#define QCM_toUtf8 toUtf8
#define QCM_toAscii toAscii
#define QCM_readBlock read
#define QCM_writeBlock write
#define QCM_toPlainText toPlainText
#define QCM_setPlainText setPlainText
#define QCM_insertPlainText insertPlainText
#define QCM_readRawData readRawData
#define QCM_writeRawData writeRawData
#define QCM_duplicate(t, d, s) t = QByteArray(d, s)
#define QCM_split(sep, str) str.split(sep)
#define QCM_setScheme setScheme
#define QCM_setPath setPath
#define QCM_SIGNAL_triggered SIGNAL(triggered())
#define QCM_SIGNAL_editTextChanged SIGNAL(editTextChanged(const QString&))
#define QCM_SIGNAL_readyReadStandardOutput SIGNAL(readyReadStandardOutput())
#define QCM_SIGNAL_readyReadStandardError SIGNAL(readyReadStandardError())
#define QCM_getText(parent, title, label, mode, text, ok) getText(parent, title, label, mode, text, ok)
#define QCM_getItem(parent, title, label, list, current, editable, ok) getItem(parent, title, label, list, current, editable, ok)
#define QCM_getSaveFileName(parent, dir) getSaveFileName(parent, QString(), dir)
#define QCM_getOpenFileName(parent, dir) getOpenFileName(parent, QString(), dir)
#define QCM_QUrl_encode(u) u = QUrl::toPercentEncoding(u)
#define QCM_QUrl_decode(u) u = QUrl::fromPercentEncoding(u.toUtf8())
#define QCM_setTextFormat_PlainText() setAcceptRichText(false)
#define QCM_readAllStandardOutput readAllStandardOutput
#define QCM_readAllStandardError readAllStandardError
#define QCM_readAll readAll
#define QCM_setIcon setIcon
#define QCM_QCString QByteArray
#define QCM_setIconSet setIcon

#else

#define QTreeWidgetItem QListViewItem
#define QListWidgetItem QListBoxItem
namespace QAbstractSocket { enum SocketError {}; }

#define QCM_WriteOnly IO_WriteOnly
#define QCM_ReadOnly IO_ReadOnly
#define QCM_NoButton QMessageBox::NoButton
#define QCM_CaseInsensitive false

#define QCM_indexIn search
#define QCM_addItems insertStringList
#define QCM_addItem insertItem
#define QCM_toLower lower
#define QCM_toUpper upper
#define QCM_absoluteFilePath absFilePath
#define QCM_absolutePath absPath
#define QCM_indexOf find
#define QCM_lastIndexOf findRev
#define QCM_setWindowTitle setCaption
#define QCM_setMaximum setMaxValue
#define QCM_setMinimum setMinValue
#define QCM_currentPath currentDirPath
#define QCM_setWindowIcon setIcon
#define QCM_setMenuText setMenuText
#define QCM_setShortcut setAccel
#define QCM_addAction(w, a) (a)->addTo(w)
#define QCM_setToolTip(w, t) QToolTip::add(w, t)
#define QCM_addSeparator insertSeparator
#define QCM_showMessage message
#define QCM_clearMessage clear
#define QCM_trimmed stripWhiteSpace
#define QCM_writeEntry writeEntry
#define QCM_readEntry readEntry
#define QCM_readBoolEntry readBoolEntry
#define QCM_readNumEntry readNumEntry
#define QCM_readListEntry readListEntry
#define QCM_removeEntry removeEntry
#define QCM_insertItem(i, t) insertItem(t, i)
#define QCM_currentIndex currentItem
#define QCM_setCurrentIndex setCurrentItem
#define QCM_cleanPath cleanDirPath
#define QCM_latin1 latin1
#define QCM_toUtf8 utf8
#define QCM_toAscii ascii
#define QCM_readBlock readBlock
#define QCM_writeBlock writeBlock
#define QCM_toPlainText text
#define QCM_setPlainText setText
#define QCM_insertPlainText insert
#define QCM_readRawData readRawBytes
#define QCM_writeRawData writeRawBytes
#define QCM_duplicate(t, d, s) t.duplicate(d, s)
#define QCM_split(sep, str) QStringList::split(sep, str)
#define QCM_setScheme setProtocol
#define QCM_setPath setFileName
#define QCM_SIGNAL_triggered SIGNAL(activated())
#define QCM_SIGNAL_editTextChanged SIGNAL(textChanged(const QString&))
#define QCM_SIGNAL_readyReadStandardOutput SIGNAL(readyReadStdout())
#define QCM_SIGNAL_readyReadStandardError SIGNAL(readyReadStderr())
#define QCM_getText(parent, title, label, mode, text, ok) getText(title, label, mode, text, ok, parent)
#define QCM_getItem(parent, title, label, list, current, editable, ok) getItem(title, label, list, current, editable, ok, parent)
#define QCM_getSaveFileName(parent, dir) getSaveFileName(dir, QString::null, parent)
#define QCM_getOpenFileName(parent, dir) getOpenFileName(dir, QString::null, parent)
#define QCM_QUrl_encode(u) QUrl::encode(u)
#define QCM_QUrl_decode(u) QUrl::decode(u)
#define QCM_setTextFormat_PlainText() setTextFormat(Qt::PlainText)
#define QCM_readAllStandardOutput readStdout
#define QCM_readAllStandardError readStderr
#define QCM_readAll read
#define QCM_setIcon setPixmap
#define QCM_QCString QCString
#define QCM_setIconSet setIconSet

#endif

#endif // QTCOMPATMAC_H
