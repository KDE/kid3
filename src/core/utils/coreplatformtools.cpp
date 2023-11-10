/**
 * \file coreplatformtools.cpp
 * Core platform specific tools for Qt.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "coreplatformtools.h"
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>
#include "config.h"
#include "kid3settings.h"
#include "coretaggedfileiconprovider.h"

/**
 * Constructor.
 */
CorePlatformTools::CorePlatformTools()
  : m_settings(nullptr)
{
}

/**
 * Destructor.
 */
CorePlatformTools::~CorePlatformTools()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Get application settings.
 * @return settings instance.
 */
ISettings* CorePlatformTools::applicationSettings()
{
  if (!m_config) {
    QByteArray configPath = qgetenv("KID3_CONFIG_FILE");
    if (configPath.isNull()) {
      m_settings = new QSettings(
            QSettings::UserScope, QLatin1String("Kid3"),
            QLatin1String("Kid3"), qApp);
    } else {
      m_settings = new QSettings(
            QFile::decodeName(configPath), QSettings::IniFormat, qApp);
    }
    m_config.reset(new Kid3Settings(m_settings));
  }
  return m_config.data();
}

/**
 * Get icon provider for tagged files.
 * @return icon provider.
 */
CoreTaggedFileIconProvider* CorePlatformTools::iconProvider()
{
  if (!m_iconProvider) {
    m_iconProvider.reset(new CoreTaggedFileIconProvider);
  }
  return m_iconProvider.data();
}

/**
 * Write text to clipboard.
 * @param text text to write
 * @return true if operation is supported.
 */
bool CorePlatformTools::writeToClipboard(const QString& text) const
{
  Q_UNUSED(text)
  return false;
}

/**
 * Read text from clipboard.
 * @return text, null if operation not supported.
 */
QString CorePlatformTools::readFromClipboard() const
{
  return QString();
}

/**
 * Create an audio player instance.
 * @param app application context
 * @param dbusEnabled true to enable MPRIS D-Bus interface
 * @return audio player, nullptr if not supported.
 */
QObject* CorePlatformTools::createAudioPlayer(Kid3Application* app,
                                              bool dbusEnabled) const
{
  Q_UNUSED(app)
  Q_UNUSED(dbusEnabled)
  return nullptr;
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

bool CorePlatformTools::moveToTrash(const QString& path) const
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

bool CorePlatformTools::moveToTrash(const QString& path) const
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
#include <QUrl>
#include <cstdlib>
#include <cstdio>
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
         QFile::exists(infoPath + QLatin1Char('/') + destName +
                       QLatin1String(".trashinfo"))) {
    ++counter;
    destName = QString(QLatin1String("%1.%2.%3"))
        .arg(baseName).arg(counter).arg(suffix);
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
  stream << QString(QLatin1String("[Trash Info]\nPath=%1\nDeletionDate=%2\n"))
    .arg(QString::fromLatin1(QUrl(absPath).toEncoded()),
         QDateTime::currentDateTime().toString(Qt::ISODate));
  file.close();
  return QDir().rename(absPath, filesPath + QLatin1Char('/') + destName);
}

bool findMountPoint(dev_t dev, QString& mountPoint)
{
#if defined HAVE_MNTENT_H && !defined Q_OS_ANDROID
  if (FILE* fp = ::setmntent("/proc/mounts", "r")) {
    struct stat st;
    struct mntent* mnt;
    while ((mnt = ::getmntent(fp)) != nullptr) {
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
#else
  Q_UNUSED(dev)
  Q_UNUSED(mountPoint)
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
  return QDir(trashDir).exists() || QDir().mkpath(trashDir);
}

} // anonymous namespace

/**
 * Move file or directory to trash.
 *
 * @param path path to file or directory
 *
 * @return true if ok.
 */
bool CorePlatformTools::moveFileToTrash(const QString& path)
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
    QByteArray xdhEnv = qgetenv("XDG_DATA_HOME");
    topDir = !xdhEnv.isEmpty() ? QString::fromLatin1(xdhEnv)
                               : QDir::homePath() + QLatin1String("/.local/share");
    trashDir = topDir + QLatin1String("/Trash");
  } else if (!(findMountPoint(pathStat.st_dev, topDir) &&
               findExtVolumeTrash(topDir, trashDir))) {
    return false;
  }
  return moveToTrashDir(fi, trashDir);
}

bool CorePlatformTools::moveToTrash(const QString& path) const
{
  return moveFileToTrash(path);
}

#endif

/**
 * Construct a name filter string suitable for file dialogs.
 * @param nameFilters list of description, filter pairs, e.g.
 * [("Images", "*.jpg *.jpeg *.png *.webp"), ("All Files", "*")].
 * @return name filter string.
 */
QString CorePlatformTools::fileDialogNameFilter(
    const QList<QPair<QString, QString> >& nameFilters) const
{
  return ICorePlatformTools::qtFileDialogNameFilter(nameFilters);
}

/**
 * Get file pattern part of m_nameFilter.
 * @param nameFilter name filter string
 * @return file patterns, e.g. "*.mp3".
 */
QString CorePlatformTools::getNameFilterPatterns(const QString& nameFilter) const
{
  return ICorePlatformTools::qtNameFilterPatterns(nameFilter);
}
