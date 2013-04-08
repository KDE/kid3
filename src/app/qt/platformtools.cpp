/**
 * \file platformtools.cpp
 * Platform specific tools.
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

#include "platformtools.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include "config.h"
#include "browserdialog.h"
#include "messagedialog.h"
#include "kid3settings.h"
#include "mainwindowconfig.h"

/**
 * Constructor.
 */
PlatformTools::PlatformTools() :
  m_mainWindowConfig(0), m_settings(0), m_config(0), m_helpBrowser(0)
{
}

/**
 * Destructor.
 */
PlatformTools::~PlatformTools()
{
  delete m_helpBrowser;
  delete m_config;
}

/**
 * Get application settings.
 * @return settings instance.
 */
ISettings* PlatformTools::applicationSettings()
{
  if (!m_config) {
    m_settings = new QSettings(
          QSettings::UserScope, QLatin1String("kid3.sourceforge.net"),
          QLatin1String("Kid3"), qApp);
    m_settings->beginGroup(QLatin1String("/kid3"));
    m_config = new Kid3Settings(m_settings);
  }
  return m_config;
}

#ifdef Q_OS_WIN32

#include <QVector>
#ifdef Q_CC_MSVC
#include <windows.h>
#else
#include <windef.h>
#include <winbase.h>
#include <shellapi.h>
#endif

bool PlatformTools::moveToTrash(const QString& path) const
{
  typedef int (WINAPI *SHFileOperationW_t)(LPSHFILEOPSTRUCTW);
  HMODULE hshell32 = GetModuleHandleA("shell32.dll");
  SHFileOperationW_t pSHFileOperationW = reinterpret_cast<SHFileOperationW_t>(
        GetProcAddress(hshell32, "SHFileOperationW"));
  if (!pSHFileOperationW) {
    // SHFileOperationW is only available since Windows XP.
    return false;
  }

  QFileInfo fi(path);
  const QString absPath(fi.absoluteFilePath());

  QVector<WCHAR> from(absPath.length() + 2);
  int i;
  for (i = 0; i < absPath.length(); i++) {
    from[i] = absPath.at(i).unicode();
  }
  from[i++] = 0;
  from[i++] = 0;

  SHFILEOPSTRUCTW fileOp;
  fileOp.hwnd = 0;
  fileOp.wFunc = FO_DELETE;
  fileOp.pFrom = from.data();
  fileOp.pTo = 0;
  fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
  fileOp.fAnyOperationsAborted = 0;
  fileOp.hNameMappings = 0;
  fileOp.lpszProgressTitle = 0;
  return pSHFileOperationW(&fileOp) == 0;
}

#elif defined Q_OS_MAC

#include <CoreServices/CoreServices.h>

bool PlatformTools::moveToTrash(const QString& path) const
{
  QFileInfo fi(path);
  const QString absPath(fi.absoluteFilePath());
  FSRef fsRef;
  OSErr err = FSPathMakeRefWithOptions(
    reinterpret_cast<const UInt8*>(
      QFile::encodeName(absPath).constData()),
    kFSPathMakeRefDoNotFollowLeafSymlink, &fsRef, 0);
  if (err != noErr)
    return false;

  return FSMoveObjectToTrashSync(&fsRef, 0, kFSFileOperationDefaultOptions) == noErr;
}

#else

/*
 * Implemented according to Desktop Trash Can Specification at
 * http://www.freedesktop.org/wiki/Specifications/trash-spec
 */

#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

namespace {

bool moveToTrashDir(const QFileInfo& fi, const QString& trashDir)
{
  QString absPath(fi.absoluteFilePath());
  QString fileName(fi.fileName());
  QString filesPath(trashDir + QLatin1String("/files"));
  QString infoPath(trashDir + QLatin1String("/info"));
  QString baseName(fi.baseName());
  QString suffix(fi.completeSuffix());
  QString destName(fileName);
  int counter = 1;
  while (QFile::exists(filesPath + QLatin1Char('/') + destName) ||
         QFile::exists(infoPath + QLatin1Char('/') + destName + QLatin1String(".trashinfo"))) {
    ++counter;
    destName = QString(QLatin1String("%1.%2.%3")).arg(baseName).arg(counter).arg(suffix);
  }
  if (!(QDir(filesPath).exists() ||
        QDir().mkpath(filesPath)) ||
      !(QDir(infoPath).exists() ||
        QDir().mkpath(infoPath)))
    return false;

  QFile file(infoPath + QLatin1Char('/') + destName + QLatin1String(".trashinfo"));
  if (!file.open(QIODevice::WriteOnly))
    return false;
  QTextStream stream(&file);
  stream << QString(QLatin1String("[Trash Info]\nPath=%1\nDeletionDate=%2\n")).
    arg(absPath).
    arg(QDateTime::currentDateTime().toString(Qt::ISODate));
  file.close();
  return QDir().rename(absPath, filesPath + QLatin1Char('/') + destName);
}

bool findMountPoint(dev_t dev, QString& mountPoint)
{
#ifdef HAVE_MNTENT_H
  if (FILE* fp = ::setmntent("/proc/mounts", "r")) {
    struct stat st;
    struct mntent* mnt;
    while ((mnt = ::getmntent(fp)) != 0) {
      if (::stat(mnt->mnt_dir, &st) != 0) {
        continue;
      }

      if (st.st_dev == dev) {
        ::endmntent(fp);
        mountPoint = QString::fromLatin1(mnt->mnt_dir);
        return true;
      }
    }
    ::endmntent(fp);
  }
#endif
  return false;
}

bool findExtVolumeTrash(const QString& volumeRoot, QString& trashDir)
{
  struct stat st;
  trashDir = volumeRoot + QLatin1String("/.Trash");
  uid_t uid = ::getuid();
  if (QDir(trashDir).exists() &&
      ::lstat(trashDir.toLocal8Bit().data(), &st) == 0 &&
      (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode) && (st.st_mode & S_ISVTX))) {
    trashDir += QString(QLatin1String("/%1")).arg(uid);
  } else {
    trashDir += QString(QLatin1String("-%1")).arg(uid);
  }
  if (QDir(trashDir).exists() ||
      QDir().mkpath(trashDir)) {
    return true;
  }
  return false;
}

} // anonymous namespace

bool PlatformTools::moveToTrash(const QString& path) const
{
  QFileInfo fi(path);
  const QString absPath(fi.absoluteFilePath());

  if (!fi.exists() || !fi.isWritable())
    return false;

  struct stat pathStat;
  struct stat trashStat;
  if (::lstat(QFile::encodeName(absPath).constData(), &pathStat) != 0 ||
      ::lstat(QFile::encodeName(QDir::homePath()).constData(), &trashStat) != 0)
    return false;

  QString topDir;
  QString trashDir;
  if (pathStat.st_dev == trashStat.st_dev) {
    char* xdhEnv = ::getenv("XDG_DATA_HOME");
    topDir = xdhEnv ? QString::fromLatin1(xdhEnv) : QDir::homePath() + QLatin1String("/.local/share");
    trashDir = topDir + QLatin1String("/Trash");
  } else if (!(findMountPoint(pathStat.st_dev, topDir) &&
               findExtVolumeTrash(topDir, trashDir))) {
    return false;
  }
  return moveToTrashDir(fi, trashDir);
}

#endif

/**
 * Display help for a topic.
 *
 * @param anchor anchor in help document
 */
void PlatformTools::displayHelp(const QString& anchor)
{
  if (!m_helpBrowser) {
    QString caption(QCoreApplication::translate("@default",
                    QT_TRANSLATE_NOOP("@default", "Kid3 Handbook")));
    m_helpBrowser = new BrowserDialog(0, caption);
  }
  m_helpBrowser->goToAnchor(anchor);
  m_helpBrowser->setModal(!anchor.isEmpty());
  if (m_helpBrowser->isHidden()) {
    m_helpBrowser->show();
  }
}

/**
 * Get a themed icon by name.
 * @param name name of icon
 * @return icon.
 */
QIcon PlatformTools::iconFromTheme(const QString& name) const
{
#if QT_VERSION >= 0x040600
  return QIcon::fromTheme(name,
      QIcon(QLatin1String(":/images/") + name + QLatin1String(".png")));
#else
  return QIcon(QLatin1String(":/images/") + name + QLatin1String(".png"));
#endif
}

/**
 * Construct a name filter string suitable for file dialogs.
 * @param nameFilters list of description, filter pairs, e.g.
 * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
 * @return name filter string.
 */
QString PlatformTools::fileDialogNameFilter(
    const QList<QPair<QString, QString> >& nameFilters) const
{
  QString filter;
  for (QList<QPair<QString, QString> >::const_iterator it =
       nameFilters.constBegin();
       it != nameFilters.constEnd();
       ++it) {
    if (!filter.isEmpty()) {
      filter += QLatin1String(";;");
    }
    filter += it->first;
    filter += QLatin1String(" (");
    filter += it->second;
    filter += QLatin1Char(')');
  }
  return filter;
}

/**
 * Get file pattern part of m_nameFilter.
 * @param nameFilter name filter string
 * @return file patterns, e.g. "*.mp3".
 */
QString PlatformTools::getNameFilterPatterns(const QString& nameFilter) const
{
  int start = nameFilter.indexOf(QLatin1Char('(')),
      end = nameFilter.indexOf(QLatin1Char(')'));
  return start != -1 && end != -1 && end > start
      ? nameFilter.mid(start + 1, end - start - 1)
      : QString();
}


/**
 * Display error dialog with item list.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 */
void PlatformTools::errorList(QWidget* parent, const QString& text,
    const QStringList& strlist, const QString& caption)
{
  MessageDialog::warningList(parent, caption, text, strlist, QMessageBox::Ok);
}

/**
 * Display warning dialog with yes, no, cancel buttons.
 * @param parent parent widget
 * @param text text
 * @param caption caption
 * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
 */
int PlatformTools::warningYesNoCancel(QWidget* parent, const QString& text,
    const QString& caption)
{
  return QMessageBox::warning(parent, caption, text,
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No,
                              QMessageBox::Cancel | QMessageBox::Escape);
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
QString PlatformTools::getOpenFileName(QWidget* parent, const QString& caption,
    const QString& dir, const QString& filter, QString* selectedFilter)
{
  return QFileDialog::getOpenFileName(
        parent, caption, dir, filter, selectedFilter,
        m_mainWindowConfig && m_mainWindowConfig->m_dontUseNativeDialogs
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0));
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
QString PlatformTools::getSaveFileName(QWidget* parent, const QString& caption,
    const QString& dir, const QString& filter, QString* selectedFilter)
{
  return QFileDialog::getSaveFileName(
        parent, caption, dir, filter, selectedFilter,
        m_mainWindowConfig && m_mainWindowConfig->m_dontUseNativeDialogs
        ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0));
}

/**
 * Display dialog to select an existing directory.
 * @param parent parent widget
 * @param caption caption
 * @param startDir start directory
 * @return selected directory, empty if canceled.
 */
QString PlatformTools::getExistingDirectory(QWidget* parent,
    const QString& caption, const QString& startDir)
{
  return QFileDialog::getExistingDirectory(parent, caption, startDir,
      m_mainWindowConfig && m_mainWindowConfig->m_dontUseNativeDialogs
      ? QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog
      : QFileDialog::ShowDirsOnly);
}

/**
 * Display warning dialog.
 * @param parent parent widget
 * @param text text
 * @param details detailed message
 * @param caption caption
 */
void PlatformTools::warningDialog(QWidget* parent,
    const QString& text, const QString& details, const QString& caption)
{
  MessageDialog dialog(parent);
  dialog.setWindowTitle(caption);
  dialog.setText(text);
  dialog.setInformativeText(details);
  dialog.setIcon(QMessageBox::Warning);
  dialog.exec();
}

/**
 * Display warning dialog with options to continue or cancel.
 * @param parent parent widget
 * @param text text
 * @param strlist list of items
 * @param caption caption
 * @return true if continue was selected.
 */
bool PlatformTools::warningContinueCancelList(QWidget* parent,
    const QString& text, const QStringList& strlist, const QString& caption)
{
  return MessageDialog::warningList(parent, caption, text, strlist) ==
      QMessageBox::Ok;
}
