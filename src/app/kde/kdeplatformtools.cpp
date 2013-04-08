/**
 * \file kdeplatformtools.cpp
 * KDE platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "kdeplatformtools.h"
#include <kurl.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <ktoolinvocation.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <QCoreApplication>
#include "kdesettings.h"

/**
 * Constructor.
 */
KdePlatformTools::KdePlatformTools() :
  m_settings(0), m_config(0)
{
}

/**
 * Destructor.
 */
KdePlatformTools::~KdePlatformTools()
{
  delete m_config;
  delete m_settings;
}

/**
 * Get application settings.
 * @return settings instance.
 */
ISettings* KdePlatformTools::applicationSettings()
{
  if (!m_config) {
    m_settings = new KConfig;
    m_config = new KdeSettings(m_settings);
  }
  return m_config;
}

/**
 * Move file or directory to trash.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool KdePlatformTools::moveToTrash(const QString& path) const
{
  KUrl src;
  src.setPath(path);
  KIO::Job* job = KIO::trash(src);
  return KIO::NetAccess::synchronousRun(job, 0);
}

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void KdePlatformTools::displayHelp(const QString& anchor)
{
  KToolInvocation::invokeHelp(anchor);
}

/**
 * Get a themed icon by name.
 * @param name name of icon
 * @return icon.
 */
QIcon KdePlatformTools::iconFromTheme(const QString& name) const
{
  return KIcon(name);
}

/**
 * Construct a name filter string suitable for file dialogs.
 * @param nameFilters list of description, filter pairs, e.g.
 * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
 * @return name filter string.
 */
QString KdePlatformTools::fileDialogNameFilter(
    const QList<QPair<QString, QString> >& nameFilters) const
{
  QString filter;
  for (QList<QPair<QString, QString> >::const_iterator it =
       nameFilters.constBegin();
       it != nameFilters.constEnd();
       ++it) {
    if (!filter.isEmpty()) {
      filter += QLatin1Char('\n');
    }
    filter += it->second;
    filter += QLatin1Char('|');
    filter += it->first;
    if (it->second.length() < 60) {
      filter += QLatin1String(" (");
      filter += it->second;
      filter += QLatin1Char(')');
    }
  }
  return filter;
}

/**
 * Get file pattern part of m_nameFilter.
 * @param nameFilter name filter string
 * @return file patterns, e.g. "*.mp3".
 */
QString KdePlatformTools::getNameFilterPatterns(const QString& nameFilter) const
{
  if (nameFilter.startsWith(QLatin1Char('*'))) {
    int end = nameFilter.indexOf(QLatin1Char('|'));
    return end != -1 ? nameFilter.left(end) : nameFilter;
  } else {
    return QString();
  }
}

/**
 * Display error dialog with item list.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 */
void KdePlatformTools::errorList(QWidget* parent, const QString& text,
    const QStringList& strlist, const QString& caption)
{
  KMessageBox::errorList(parent, text, strlist, caption);
}

/**
 * Display warning dialog with yes, no, cancel buttons.
 * @param parent parent widget
 * @param text text
 * @param caption caption
 * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
 */
int KdePlatformTools::warningYesNoCancel(QWidget* parent, const QString& text,
    const QString& caption)
{
  int rc = KMessageBox::warningYesNoCancel(parent, text, caption);
  switch (rc) {
  case KMessageBox::Ok:
    return QMessageBox::Ok;
  case KMessageBox::Cancel:
    return QMessageBox::Cancel;
  case KMessageBox::Yes:
    return QMessageBox::Yes;
  case KMessageBox::No:
    return QMessageBox::No;
  case KMessageBox::Continue:
  default:
    return QMessageBox::Ignore;
  }
}

/**
 * Display dialog to select an existing file.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected file, empty if canceled.
 */
QString KdePlatformTools::getOpenFileName(QWidget* parent,
    const QString& caption, const QString& dir, const QString& filter,
    QString* selectedFilter)
{
  if (selectedFilter) {
    QString selectedFile;
    KFileDialog diag(dir, filter, parent);
    diag.setWindowTitle(caption.isEmpty()
                        ? QCoreApplication::translate("@default",
                              QT_TRANSLATE_NOOP("@default", "Open"))
                        : caption);
    if (diag.exec() == QDialog::Accepted) {
      selectedFile = diag.selectedFile();
      *selectedFilter = diag.currentFilter();
    }
    return selectedFile;
  } else {
    return KFileDialog::getOpenFileName(dir, filter, parent, caption);
  }
}


/**
 * Display dialog to select a file to save.
 * @param parent parent widget
 * @param caption caption
 * @param dir directory
 * @param filter filter
 * @param selectedFilter the selected filter is returned here
 * @return selected file, empty if canceled.
 */
QString KdePlatformTools::getSaveFileName(QWidget* parent,
    const QString& caption, const QString& dir, const QString& filter,
    QString* selectedFilter)
{
  if (selectedFilter) {
    QString selectedFile;
    KFileDialog diag(dir, filter, parent);
    diag.setOperationMode(KFileDialog::Saving);
    diag.setWindowTitle(caption.isEmpty()
                        ? QCoreApplication::translate("@default",
                              QT_TRANSLATE_NOOP("@default", "Save As"))
                        : caption);
    if (diag.exec() == QDialog::Accepted) {
      selectedFile = diag.selectedFile();
      *selectedFilter = diag.currentFilter();
    }
    return selectedFile;
  } else {
    return KFileDialog::getSaveFileName(dir, filter, parent, caption);
  }
}

/**
 * Display dialog to select an existing directory.
 * @param parent parent widget
 * @param caption caption
 * @param startDir start directory
 * @return selected directory, empty if canceled.
 */
QString KdePlatformTools::getExistingDirectory(QWidget* parent,
    const QString& caption, const QString& startDir)
{
  return KFileDialog::getExistingDirectory(startDir, parent, caption);
}

/**
 * Display warning dialog.
 * @param parent parent widget
 * @param text text
 * @param details detailed message
 * @param caption caption
 */
void KdePlatformTools::warningDialog(QWidget* parent,
    const QString& text, const QString& details, const QString& caption)
{
  KMessageBox::error(parent, text + details, caption);
}

/**
 * Display warning dialog with options to continue or cancel.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 * @return true if continue was selected.
 */
bool KdePlatformTools::warningContinueCancelList(QWidget* parent,
    const QString& text, const QStringList& strlist, const QString& caption)
{
  return KMessageBox::warningContinueCancelList(parent, text, strlist, caption,
      KStandardGuiItem::ok(), KStandardGuiItem::cancel(), QString(),
      KMessageBox::Dangerous) == KMessageBox::Continue;
}
