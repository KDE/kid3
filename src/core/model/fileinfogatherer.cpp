/**
 * \file fileinfogatherer.cpp
 * \cond
 * Taken from Qt Git, revision e73bd4a
 * qtbase/src/widgets/dialogs/qfileinfogatherer.cpp
 * Adapted for Kid3 with the following changes:
 * - Remove Q prefix from class names
 * - Remove QT_..._CONFIG, QT_..._NAMESPACE, Q_..._EXPORT...
 * - Allow compilation with Qt versions < 5.7
 * - Remove moc includes
 * - Remove dependencies to Qt5::Widgets
 */
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "fileinfogatherer_p.h"
#include <qdebug.h>
#include <qdiriterator.h>
#ifndef Q_OS_WIN
#  include <unistd.h>
#  include <sys/types.h>
#endif
#if defined(Q_OS_VXWORKS)
#  include "qplatformdefs.h"
#endif
#include "abstractfiledecorationprovider.h"

#ifdef Q_OS_WIN
#include <windows.h>

/**
 * Check if path is a drive which could cause an insert disk dialog to pop up.
 *
 * This method should be used before calling QFileInfo::permissions(),
 * or QFileInfo::isReadable() on Windows.
 * The bug has been reported for Windows 7 32-bit and could be reproduced with
 * Windows XP. To trigger the bug, a CD has to be inserted and then removed once
 * before fetching the root directory with a file system model. See
 * https://forum.qt.io/topic/34799/checking-is-a-drive-is-readable-in-qt-pops-up-a-no-disk-error-in-windows-7
 *
 * @param path drive path, e.g. "D:/"
 * @return true if path is for a drive and getting volume information fails.
 */
bool ExtendedInformation::isInvalidDrive(const QString &path)
{
    // Windows drive nodes are queried with paths like "D:/", check if path is
    // a drive letter followed by a colon.
    const int pathLen = path.length();
    if (pathLen < 2 || pathLen > 3 || path.at(1) != QLatin1Char(':') ||
        !path.at(0).isLetter())
        return false;

    const DWORD VOLUME_NAME_SIZE = 255;
    const DWORD FILE_SYSTEM_NAME_SIZE = 255;
    LPCWSTR rootPathName = (LPCWSTR)path.utf16();
    UCHAR fileSystemNameBuffer[255], volumeNameBuffer[255];
    DWORD volumeSerialNumber, maximumComponentLength, fileSystemFlags;

    BOOL bSuccess = ::GetVolumeInformationW(
        rootPathName,
        (LPWSTR)volumeNameBuffer,
        VOLUME_NAME_SIZE,
        &volumeSerialNumber,
        &maximumComponentLength,
        &fileSystemFlags,
        (LPWSTR)fileSystemNameBuffer,
        FILE_SYSTEM_NAME_SIZE
    );

    return !bSuccess;
}
#endif // Q_OS_WIN

#ifdef QT_BUILD_INTERNAL
static QBasicAtomicInt fetchedRoot = Q_BASIC_ATOMIC_INITIALIZER(false);
void qt_test_resetFetchedRoot()
{
#if QT_VERSION >= 0x050e00
    fetchedRoot.storeRelaxed(false);
#else
    fetchedRoot.store(false);
#endif
}

bool qt_test_isFetchedRoot()
{
#if QT_VERSION >= 0x050e00
    return fetchedRoot.loadRelaxed();
#else
    return fetchedRoot.load();
#endif
}
#endif

static QString translateDriveName(const QFileInfo &drive)
{
    QString driveName = drive.absoluteFilePath();
#ifdef Q_OS_WIN
    if (driveName.startsWith(QLatin1Char('/'))) // UNC host
        return drive.fileName();
    if (driveName.endsWith(QLatin1Char('/')))
        driveName.chop(1);
#endif // Q_OS_WIN
    return driveName;
}

/*!
    Creates thread
*/
FileInfoGatherer::FileInfoGatherer(QObject *parent)
    : QThread(parent), holdOffOnUpdates(false), abort(false),
#ifndef QT_NO_FILESYSTEMWATCHER
      watcher(nullptr),
#endif
#ifdef Q_OS_WIN
      m_resolveSymlinks(true),
#endif
      m_decorationProvider(Q_NULLPTR)
{
#ifndef QT_NO_FILESYSTEMWATCHER
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(list(QString)));
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(updateFile(QString)));

#  if defined(Q_OS_WIN) && !defined(Q_OS_WINRT)
    const QVariant listener = watcher->property("_q_driveListener");
    if (listener.canConvert<QObject *>()) {
        if (QObject *driveListener = listener.value<QObject *>()) {
            connect(driveListener, SIGNAL(driveAdded()), this, SLOT(driveAdded()));
            connect(driveListener, SIGNAL(driveRemoved()), this, SLOT(driveRemoved()));
        }
    }
#  endif // Q_OS_WIN && !Q_OS_WINRT
#endif
    start(LowPriority);
}

/*!
    Destroys thread
*/
FileInfoGatherer::~FileInfoGatherer()
{
#if QT_VERSION >= 0x050e00
  abort.storeRelaxed(true);
#else
  abort.store(true);
#endif
    QMutexLocker locker(&mutex);
    condition.wakeAll();
    locker.unlock();
    wait();
}

void FileInfoGatherer::setResolveSymlinks(bool enable)
{
    Q_UNUSED(enable)
#ifdef Q_OS_WIN
    m_resolveSymlinks = enable;
#endif
}

void FileInfoGatherer::driveAdded()
{
    fetchExtendedInformation(QString(), QStringList());
}

void FileInfoGatherer::driveRemoved()
{
    QStringList drives;
    const QFileInfoList driveInfoList = QDir::drives();
    for (const QFileInfo &fi : driveInfoList)
        drives.append(translateDriveName(fi));
    newListOfFiles(QString(), drives);
}

bool FileInfoGatherer::resolveSymlinks() const
{
#ifdef Q_OS_WIN
    return m_resolveSymlinks;
#else
    return false;
#endif
}

void FileInfoGatherer::setDecorationProvider(AbstractFileDecorationProvider *provider)
{
    m_decorationProvider = provider;
}

AbstractFileDecorationProvider *FileInfoGatherer::decorationProvider() const
{
    return m_decorationProvider;
}

/*!
    Fetch extended information for all \a files in \a path

    \sa updateFile(), update(), resolvedName()
*/
void FileInfoGatherer::fetchExtendedInformation(const QString &path, const QStringList &files)
{
    QMutexLocker locker(&mutex);
    // See if we already have this dir/file in our queue
    int loc = this->path.lastIndexOf(path);
    while (loc > 0)  {
        if (this->files.at(loc) == files) {
            return;
        }
        loc = this->path.lastIndexOf(path, loc - 1);
    }
    this->path.push(path);
    this->files.push(files);
    condition.wakeAll();

#ifndef QT_NO_FILESYSTEMWATCHER
    if (files.isEmpty()
        && !path.isEmpty()
        && !path.startsWith(QLatin1String("//")) /*don't watch UNC path*/) {
        if (!watcher->directories().contains(path))
            watcher->addPath(path);
    }
#endif
}

bool FileInfoGatherer::setHoldOffOnUpdates(bool holdoff)
{
    QMutexLocker locker(&mutex);
    bool previous = holdOffOnUpdates;
    holdOffOnUpdates = holdoff;
    if (!holdoff) {
        condition.wakeAll();
    }
    return previous;
}

/*!
    Fetch extended information for all \a filePath

    \sa fetchExtendedInformation()
*/
void FileInfoGatherer::updateFile(const QString &filePath)
{
    QString dir = filePath.mid(0, filePath.lastIndexOf(QLatin1Char('/')));
    QString fileName = filePath.mid(dir.length() + 1);
    fetchExtendedInformation(dir, QStringList(fileName));
}

/*
    List all files in \a directoryPath

    \sa listed()
*/
void FileInfoGatherer::clear()
{
#ifndef QT_NO_FILESYSTEMWATCHER
    QMutexLocker locker(&mutex);
    watcher->removePaths(watcher->files());
    watcher->removePaths(watcher->directories());
#endif

    path.clear();
    files.clear();
}

/*
    Add a \a path to the watcher
*/
void FileInfoGatherer::addPath(const QString &path)
{
#ifndef QT_NO_FILESYSTEMWATCHER
    QMutexLocker locker(&mutex);
    watcher->addPath(path);
#else
    Q_UNUSED(path);
#endif
}

/*
    Remove a \a path from the watcher

    \sa listed()
*/
void FileInfoGatherer::removePath(const QString &path)
{
#ifndef QT_NO_FILESYSTEMWATCHER
    QMutexLocker locker(&mutex);
    watcher->removePath(path);
#else
    Q_UNUSED(path);
#endif
}

/*
    List all files in \a directoryPath

    \sa listed()
*/
void FileInfoGatherer::list(const QString &directoryPath)
{
    fetchExtendedInformation(directoryPath, QStringList());
}

/*
    Until aborted wait to fetch a directory or files
*/
void FileInfoGatherer::run()
{
    forever {
        QMutexLocker locker(&mutex);
#if QT_VERSION >= 0x050e00
        while (!abort.loadRelaxed() && (path.isEmpty() || holdOffOnUpdates))
            condition.wait(&mutex);
        if (abort.loadRelaxed())
            return;
#else
        while (!abort.load() && (path.isEmpty() || holdOffOnUpdates))
            condition.wait(&mutex);
        if (abort.load())
            return;
#endif
#if QT_VERSION >= 0x050700
        const QString thisPath = std::as_const(path).front();
#else
        const auto constPath = path;
        const QString thisPath = constPath.front();
#endif
        path.pop_front();
#if QT_VERSION >= 0x050700
        const QStringList thisList = std::as_const(files).front();
#else
        const auto constFiles = files;
        const QStringList thisList = constFiles.front();
#endif
        files.pop_front();
        locker.unlock();

        getFileInfos(thisPath, thisList);
    }
}

ExtendedInformation FileInfoGatherer::getInfo(const QFileInfo &fileInfo) const
{
    ExtendedInformation info(fileInfo);
    if (m_decorationProvider) {
        info.icon = m_decorationProvider->decoration(fileInfo);
        info.displayType = m_decorationProvider->type(fileInfo);
    } else {
        info.icon = QVariant();
        info.displayType = AbstractFileDecorationProvider::fileTypeDescription(fileInfo);
    }
#ifndef QT_NO_FILESYSTEMWATCHER
    // ### Not ready to listen all modifications by default
    static const bool watchFiles = qEnvironmentVariableIsSet("QT_FILESYSTEMMODEL_WATCH_FILES");
    if (watchFiles) {
        if (!fileInfo.exists() && !fileInfo.isSymLink()) {
            watcher->removePath(fileInfo.absoluteFilePath());
        } else {
            if (const QString filePath = fileInfo.absoluteFilePath();
                !filePath.isEmpty() && fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable()
                && !watcher->files().contains(filePath)) {
                watcher->addPath(filePath);
            }
        }
    }
#endif

#ifdef Q_OS_WIN
    if (m_resolveSymlinks && info.isSymLink(/* ignoreNtfsSymLinks = */ true)) {
        QFileInfo resolvedInfo(fileInfo.symLinkTarget());
        resolvedInfo = QFileInfo(resolvedInfo.canonicalFilePath());
        if (resolvedInfo.exists()) {
            emit nameResolved(fileInfo.filePath(), resolvedInfo.fileName());
        }
    }
#endif
    return info;
}

/*
    Get specific file info's, batch the files so update when we have 100
    items and every 200ms after that
 */
void FileInfoGatherer::getFileInfos(const QString &path, const QStringList &files)
{
    // List drives
    if (path.isEmpty()) {
#ifdef QT_BUILD_INTERNAL
#if QT_VERSION >= 0x050e00
        fetchedRoot.storeRelaxed(true);
#else
        fetchedRoot.store(true);
#endif
#endif
        QFileInfoList infoList;
        if (files.isEmpty()) {
            infoList = QDir::drives();
        } else {
            infoList.reserve(files.count());
            for (const auto &file : files)
                infoList << QFileInfo(file);
        }
        for (int i = infoList.count() - 1; i >= 0; --i) {
            QString driveName = translateDriveName(infoList.at(i));
            QVector<QPair<QString,QFileInfo> > updatedFiles;
            updatedFiles.append(QPair<QString,QFileInfo>(driveName, infoList.at(i)));
            emit updates(path, updatedFiles);
        }
        return;
    }

    QElapsedTimer base;
    base.start();
    QFileInfo fileInfo;
    bool firstTime = true;
    QVector<QPair<QString, QFileInfo> > updatedFiles;
    QStringList filesToCheck = files;

    QStringList allFiles;
    if (files.isEmpty()) {
        QDirIterator dirIt(path, QDir::AllEntries | QDir::System | QDir::Hidden);
#if QT_VERSION >= 0x050e00
        while (!abort.loadRelaxed() && dirIt.hasNext())
#else
        while (!abort.load() && dirIt.hasNext())
#endif
        {
            dirIt.next();
            fileInfo = dirIt.fileInfo();
            allFiles.append(fileInfo.fileName());
            fetch(fileInfo, base, firstTime, updatedFiles, path);
        }
    }
    if (!allFiles.isEmpty())
        emit newListOfFiles(path, allFiles);

    QStringList::const_iterator filesIt = filesToCheck.constBegin();
#if QT_VERSION >= 0x050e00
    while (!abort.loadRelaxed() && filesIt != filesToCheck.constEnd())
#else
    while (!abort.load() && filesIt != filesToCheck.constEnd())
#endif
    {
        fileInfo.setFile(path + QDir::separator() + *filesIt);
        ++filesIt;
        fetch(fileInfo, base, firstTime, updatedFiles, path);
    }
    if (!updatedFiles.isEmpty())
        emit updates(path, updatedFiles);
    emit directoryLoaded(path);
}

void FileInfoGatherer::fetch(const QFileInfo &fileInfo, QElapsedTimer &base, bool &firstTime, QVector<QPair<QString, QFileInfo> > &updatedFiles, const QString &path) {
    updatedFiles.append(QPair<QString, QFileInfo>(fileInfo.fileName(), fileInfo));
    QElapsedTimer current;
    current.start();
    if ((firstTime && updatedFiles.count() > 100) || base.msecsTo(current) > 1000) {
        emit updates(path, updatedFiles);
        updatedFiles.clear();
        base = current;
        firstTime = false;
    }
}
/** \endcond */
