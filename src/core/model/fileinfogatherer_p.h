/**
 * \file fileinfogatherer_p.h
 * \cond
 * Taken from Qt Git, revision e73bd4a
 * qtbase/src/widgets/dialogs/qfileinfogatherer_p.h
 * Adapted for Kid3 with the following changes:
 * - Remove Q prefix from class names
 * - Remove QT_..._CONFIG, QT_..._NAMESPACE, Q_..._EXPORT...
 * - Allow compilation without Qt private headers (USE_QT_PRIVATE_HEADERS)
 * - Replace include guards by #pragma once
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

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qfilesystemwatcher.h>
#include <qvariant.h>
#include <qpair.h>
#include <qstack.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qelapsedtimer.h>

#ifdef USE_QT_PRIVATE_HEADERS
#include <private/qfilesystemengine_p.h>
#endif

class ExtendedInformation {
public:
    enum Type { Dir, File, System };

    ExtendedInformation() {}
    explicit ExtendedInformation(const QFileInfo &info) : mFileInfo(info) {}

    inline bool isDir() { return type() == Dir; }
    inline bool isFile() { return type() == File; }
    inline bool isSystem() { return type() == System; }

    bool operator ==(const ExtendedInformation &fileInfo) const {
       return mFileInfo == fileInfo.mFileInfo
       && displayType == fileInfo.displayType
       && permissions() == fileInfo.permissions()
       && lastModified() == fileInfo.lastModified();
    }

#ifndef QT_NO_FSFILEENGINE
    bool isCaseSensitive() const {
#ifdef USE_QT_PRIVATE_HEADERS
        return QFileSystemEngine::isCaseSensitive();
#else
        return false;
#endif
    }
#endif

    QFile::Permissions permissions() const {
#ifdef Q_OS_WIN
        if (isInvalidDrive(mFileInfo.filePath())) {
            return {};
        }
#endif
        return mFileInfo.permissions();
    }

    Type type() const {
        if (mFileInfo.isDir()) {
            return ExtendedInformation::Dir;
        }
        if (mFileInfo.isFile()) {
            return ExtendedInformation::File;
        }
        if (!mFileInfo.exists() && mFileInfo.isSymLink()) {
            return ExtendedInformation::System;
        }
        return ExtendedInformation::System;
    }

    bool isSymLink(bool ignoreNtfsSymLinks = false) const
    {
        if (ignoreNtfsSymLinks) {
#ifdef Q_OS_WIN
            return !mFileInfo.suffix().compare(QLatin1String("lnk"), Qt::CaseInsensitive);
#endif
        }
        return mFileInfo.isSymLink();
    }

    bool isHidden() const {
        return mFileInfo.isHidden();
    }

    QFileInfo fileInfo() const {
        return mFileInfo;
    }

    QDateTime lastModified() const {
        return mFileInfo.lastModified();
    }

    qint64 size() const {
        qint64 size = -1;
        if (type() == ExtendedInformation::Dir)
            size = 0;
        if (type() == ExtendedInformation::File)
            size = mFileInfo.size();
        if (!mFileInfo.exists() && !mFileInfo.isSymLink())
            size = -1;
        return size;
    }

    QString displayType;
    QVariant icon;

private :
#ifdef Q_OS_WIN
    static bool isInvalidDrive(const QString &path);
#endif
    QFileInfo mFileInfo;
};

class AbstractFileDecorationProvider;

class FileInfoGatherer : public QThread
{
Q_OBJECT

Q_SIGNALS:
    void updates(const QString &directory, const QVector<QPair<QString, QFileInfo> > &updates);
    void newListOfFiles(const QString &directory, const QStringList &listOfFiles) const;
    void nameResolved(const QString &fileName, const QString &resolvedName) const;
    void directoryLoaded(const QString &path);

public:
    explicit FileInfoGatherer(QObject *parent = 0);
    ~FileInfoGatherer();

    // only callable from this->thread():
    void clear();
    void addPath(const QString &path);
    void removePath(const QString &path);
    ExtendedInformation getInfo(const QFileInfo &fileInfo) const;
    AbstractFileDecorationProvider *decorationProvider() const;
    bool resolveSymlinks() const;

public Q_SLOTS:
    void list(const QString &directoryPath);
    void fetchExtendedInformation(const QString &path, const QStringList &files);
    void updateFile(const QString &filePath);
    void setResolveSymlinks(bool enable);
    void setDecorationProvider(AbstractFileDecorationProvider *provider);

private Q_SLOTS:
    void driveAdded();
    void driveRemoved();

private:
    void run() Q_DECL_OVERRIDE;
    // called by run():
    void getFileInfos(const QString &path, const QStringList &files);
    void fetch(const QFileInfo &fileInfo, QElapsedTimer &base, bool &firstTime, QVector<QPair<QString, QFileInfo> > &updatedFiles, const QString &path);

private:
    mutable QMutex mutex;
    // begin protected by mutex
    QWaitCondition condition;
    QStack<QString> path;
    QStack<QStringList> files;
    // end protected by mutex
    QAtomicInt abort;

#ifndef QT_NO_FILESYSTEMWATCHER
    QFileSystemWatcher *watcher;
#endif
#ifdef Q_OS_WIN
    bool m_resolveSymlinks; // not accessed by run()
#endif
    AbstractFileDecorationProvider *m_decorationProvider;
};
/** \endcond */
