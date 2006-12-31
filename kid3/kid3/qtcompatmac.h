/**
 * \file qtcompatmac.h
 * Qt compatibility macros.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 31 Oct 2006
 */

#ifndef QTCOMPATMAC_H
#define QTCOMPATMAC_H

#include <qglobal.h>
#include "config.h"

#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#if QT_VERSION >= 0x040000
#include <QCoreApplication>
#define i18n(s) QCoreApplication::translate("@default", s)
#else
#define i18n(s) tr(s)
#endif
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#if QT_VERSION >= 0x040000

#define QListBoxItem Q3ListBoxItem
#define QListViewItem Q3ListViewItem

#define QCM_WriteOnly QIODevice::WriteOnly
#define QCM_ReadOnly QIODevice::ReadOnly
#define QCM_NoButton Qt::NoButton

#define QCM_indexIn indexIn
#define QCM_addItems addItems

#else

#define Q3CString QCString
#define Q3CheckTableItem QCheckTableItem
#define Q3ComboTableItem QComboTableItem
#define Q3DragObject QDragObject
#define Q3GroupBox QGroupBox
#define Q3HBox QHBox
#define Q3Header QHeader
#define Q3ListBox QListBox
#define Q3ListBoxItem QListBoxItem
#define Q3ListBoxText QListBoxText
#define Q3ListView QListView
#define Q3ListViewItem QListViewItem
#define Q3ListViewItemIterator QListViewItemIterator
#define Q3MainWindow QMainWindow
#define Q3PopupMenu QPopupMenu
#define Q3Process QProcess
#define Q3ProgressBar QProgressBar
#define Q3PtrList QPtrList
#define Q3ScrollView QScrollView
#define Q3Socket QSocket
#define Q3TabDialog QTabDialog
#define Q3Table QTable
#define Q3TableItem QTableItem
#define Q3TextDrag QTextDrag
#define Q3Url QUrl
#define Q3VBox QVBox
#define Q3ValueList QValueList
#define Q3ValueVector QValueVector

#define QCM_WriteOnly IO_WriteOnly
#define QCM_ReadOnly IO_ReadOnly
#define QCM_NoButton QMessageBox::NoButton

#define QCM_indexIn search
#define QCM_addItems insertStringList

#endif

#endif // QTCOMPATMAC_H
