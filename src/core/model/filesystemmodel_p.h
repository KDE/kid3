/**
 * \file filesystemmodel_p.h
 * \cond
 * Taken from Qt Git, revision e73bd4a
 * qtbase/src/widgets/dialogs/qfilesystemmodel_p.h
 * Adapted for Kid3 with the following changes:
 * - Remove Q prefix from class names
 * - Remove QT_..._CONFIG, QT_..._NAMESPACE, Q_..._EXPORT...
 * - Allow compilation without Qt private headers (USE_QT_PRIVATE_HEADERS)
 * - Allow compilation with Qt versions < 5.7
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

#ifdef USE_QT_PRIVATE_HEADERS
#include <private/qabstractitemmodel_p.h>
#endif
#include <qabstractitemmodel.h>
#include "filesystemmodel.h"
#include "fileinfogatherer_p.h"
#include <qpair.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qhash.h>
#include "abstractfiledecorationprovider.h"

class ExtendedInformation;
class FileSystemModelPrivate;
class AbstractFileDecorationProvider;

#if defined(Q_OS_WIN)
class FileSystemModelNodePathKey : public QString
{
public:
    FileSystemModelNodePathKey() {}
    FileSystemModelNodePathKey(const QString &other) : QString(other) {}
    FileSystemModelNodePathKey(const FileSystemModelNodePathKey &other) : QString(other) {}
    bool operator==(const FileSystemModelNodePathKey &other) const { return !compare(other, Qt::CaseInsensitive); }
};

Q_DECLARE_TYPEINFO(FileSystemModelNodePathKey, Q_MOVABLE_TYPE);

inline uint qHash(const FileSystemModelNodePathKey &key) { return qHash(key.toCaseFolded()); }
#else // Q_OS_WIN
typedef QString FileSystemModelNodePathKey;
#endif

#ifdef USE_QT_PRIVATE_HEADERS
class FileSystemModelPrivate : public QAbstractItemModelPrivate
#else
class FileSystemModelPrivate
#endif
{
    Q_DECLARE_PUBLIC(FileSystemModel)

public:
    enum { NumColumns = 4 };

    class FileSystemNode
    {
    public:
        explicit FileSystemNode(const QString &filename = QString(), FileSystemNode *p = 0)
            : fileName(filename), populatedChildren(false), isVisible(false), dirtyChildrenIndex(-1), parent(p), info(0) {}
        ~FileSystemNode() {
            qDeleteAll(children);
            delete info;
            info = 0;
            parent = 0;
        }
        void clear() {
            fileName.clear();
            populatedChildren = false;
            isVisible = false;
            qDeleteAll(children);
            children.clear();
            visibleChildren.clear();
            dirtyChildrenIndex = -1;
            parent = Q_NULLPTR;
            delete info;
            info = Q_NULLPTR;
        }

        QString fileName;
#if defined(Q_OS_WIN)
        QString volumeName;
#endif

        inline qint64 size() const { if (info && !info->isDir()) return info->size(); return 0; }
        inline QString type() const { if (info) return info->displayType; return QLatin1String(""); }
        inline QDateTime lastModified() const { if (info) return info->lastModified(); return QDateTime(); }
        inline QFile::Permissions permissions() const { if (info) return info->permissions(); return {}; }
        inline bool isReadable() const { return ((permissions() & QFile::ReadUser) != 0); }
        inline bool isWritable() const { return ((permissions() & QFile::WriteUser) != 0); }
        inline bool isExecutable() const { return ((permissions() & QFile::ExeUser) != 0); }
        inline bool isDir() const {
            if (info)
                return info->isDir();
            if (children.count() > 0)
                return true;
            return false;
        }
        inline QFileInfo fileInfo() const { if (info) return info->fileInfo(); return QFileInfo(); }
        inline bool isFile() const { if (info) return info->isFile(); return true; }
        inline bool isSystem() const { if (info) return info->isSystem(); return true; }
        inline bool isHidden() const { if (info) return info->isHidden(); return false; }
        inline bool isSymLink(bool ignoreNtfsSymLinks = false) const { return info && info->isSymLink(ignoreNtfsSymLinks); }
        inline bool caseSensitive() const { if (info) return info->isCaseSensitive(); return false; }
        inline QVariant icon() const { if (info) return info->icon; return QVariant(); }

        inline bool operator <(const FileSystemNode &node) const {
            if (caseSensitive() || node.caseSensitive())
                return fileName < node.fileName;
            return QString::compare(fileName, node.fileName, Qt::CaseInsensitive) < 0;
        }
        inline bool operator >(const QString &name) const {
            if (caseSensitive())
                return fileName > name;
            return QString::compare(fileName, name, Qt::CaseInsensitive) > 0;
        }
        inline bool operator <(const QString &name) const {
            if (caseSensitive())
                return fileName < name;
            return QString::compare(fileName, name, Qt::CaseInsensitive) < 0;
        }
        inline bool operator !=(const ExtendedInformation &fileInfo) const {
            return !operator==(fileInfo);
        }
        bool operator ==(const QString &name) const {
            if (caseSensitive())
                return fileName == name;
            return QString::compare(fileName, name, Qt::CaseInsensitive) == 0;
        }
        bool operator ==(const ExtendedInformation &fileInfo) const {
            return info && (*info == fileInfo);
        }

        inline bool hasInformation() const { return info != 0; }

        void populate(const ExtendedInformation &fileInfo) {
            if (!info)
                info = new ExtendedInformation(fileInfo.fileInfo());
            (*info) = fileInfo;
        }

        // children shouldn't normally be accessed directly, use node()
        inline int visibleLocation(const QString &childName) {
            return visibleChildren.indexOf(childName);
        }
        void updateIcon(AbstractFileDecorationProvider *iconProvider, const QString &path) {
            if (!iconProvider)
                return;
            if (info)
                info->icon = iconProvider->decoration(QFileInfo(path));
#if QT_VERSION >= 0x050700
            for (FileSystemNode *child : qAsConst(children)) {
#else
            const auto constChildren = children;
            for (FileSystemNode *child : constChildren) {
#endif
                //On windows the root (My computer) has no path so we don't want to add a / for nothing (e.g. /C:/)
                if (!path.isEmpty()) {
                    if (path.endsWith(QLatin1Char('/')))
                        child->updateIcon(iconProvider, path + child->fileName);
                    else
                        child->updateIcon(iconProvider, path + QLatin1Char('/') + child->fileName);
                } else
                    child->updateIcon(iconProvider, child->fileName);
            }
        }

        void retranslateStrings(AbstractFileDecorationProvider *iconProvider, const QString &path) {
            if (!iconProvider)
                return;
            if (info)
                info->displayType = iconProvider->type(QFileInfo(path));
#if QT_VERSION >= 0x050700
            for (FileSystemNode *child : qAsConst(children)) {
#else
            const auto constChildren = children;
            for (FileSystemNode *child : constChildren) {
#endif
                //On windows the root (My computer) has no path so we don't want to add a / for nothing (e.g. /C:/)
                if (!path.isEmpty()) {
                    if (path.endsWith(QLatin1Char('/')))
                        child->retranslateStrings(iconProvider, path + child->fileName);
                    else
                        child->retranslateStrings(iconProvider, path + QLatin1Char('/') + child->fileName);
                } else
                    child->retranslateStrings(iconProvider, child->fileName);
            }
        }

        bool populatedChildren;
        bool isVisible;
        QHash<FileSystemModelNodePathKey, FileSystemNode *> children;
        QList<QString> visibleChildren;
        int dirtyChildrenIndex;
        FileSystemNode *parent;


        ExtendedInformation *info;

    };

    FileSystemModelPrivate(
#ifndef USE_QT_PRIVATE_HEADERS
            QObject *q
#endif
            ) :
            forceSort(true),
            sortColumn(0),
            sortOrder(Qt::AscendingOrder),
            readOnly(true),
            setRootPath(false),
            filters(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs),
            nameFilterDisables(true), // false on windows, true on mac and unix
            disableRecursiveSort(false),
            sortIgnoringPunctuation(false)
#ifndef USE_QT_PRIVATE_HEADERS
            , q_ptr(q)
#endif
    {
        delayedSortTimer.setSingleShot(true);
    }

    void clear();
    void init();
    /*
      \internal

      Return true if index which is owned by node is hidden by the filter.
    */
    inline bool isHiddenByFilter(FileSystemNode *indexNode, const QModelIndex &index) const
    {
       return (indexNode != &root && !index.isValid());
    }
    FileSystemNode *node(const QModelIndex &index) const;
    FileSystemNode *node(const QString &path, bool fetch = true) const;
    inline QModelIndex index(const QString &path, int column = 0) { return index(node(path), column); }
    QModelIndex index(const FileSystemNode *node, int column = 0) const;
    bool filtersAcceptsNode(const FileSystemNode *node) const;
    bool passNameFilters(const FileSystemNode *node) const;
    void removeNode(FileSystemNode *parentNode, const QString &name);
    FileSystemNode* addNode(FileSystemNode *parentNode, const QString &fileName, const QFileInfo &info);
    void addVisibleFiles(FileSystemNode *parentNode, const QStringList &newFiles);
    void removeVisibleFile(FileSystemNode *parentNode, int vLocation);
    void sortChildren(int column, const QModelIndex &parent);

    inline int translateVisibleLocation(FileSystemNode *parent, int row) const {
        if (sortOrder != Qt::AscendingOrder) {
            if (parent->dirtyChildrenIndex == -1)
                return parent->visibleChildren.count() - row - 1;

            if (row < parent->dirtyChildrenIndex)
                return parent->dirtyChildrenIndex - row - 1;
        }

        return row;
    }

    inline static QString myComputer() {
        // ### TODO We should query the system to find out what the string should be
        // XP == "My Computer",
        // Vista == "Computer",
        // OS X == "Computer" (sometime user generated) "Benjamin's PowerBook G4"
#ifdef Q_OS_WIN
        return FileSystemModel::tr("My Computer");
#else
        return FileSystemModel::tr("Computer");
#endif
    }

    inline void delayedSort() {
        if (!delayedSortTimer.isActive())
            delayedSortTimer.start(0);
    }

    QVariant icon(const QModelIndex &index) const;
    QString name(const QModelIndex &index) const;
    QString displayName(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;
    QString size(const QModelIndex &index) const;
    static QString size(qint64 bytes);
    QString type(const QModelIndex &index) const;
    QString time(const QModelIndex &index) const;

    void _q_directoryChanged(const QString &directory, const QStringList &files);
    void _q_performDelayedSort();
    void _q_fileSystemChanged(const QString &path, const QVector<QPair<QString, QFileInfo> > &);
    void _q_resolvedName(const QString &fileName, const QString &resolvedName);

    static int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs);

#ifndef USE_QT_PRIVATE_HEADERS
    inline bool indexValid(const QModelIndex &index) const {
         return (index.row() >= 0) && (index.column() >= 0) && (index.model() == q_func());
    }

    QHash<int, QByteArray> roleNames;
#endif

    QDir rootDir;
#ifndef QT_NO_FILESYSTEMWATCHER
    FileInfoGatherer fileInfoGatherer;
#endif
    QTimer delayedSortTimer;
    bool forceSort;
    int sortColumn;
    Qt::SortOrder sortOrder;
    bool readOnly;
    bool setRootPath;
    QDir::Filters filters;
    QHash<const FileSystemNode*, bool> bypassFilters;
    bool nameFilterDisables;
    //This flag is an optimization for the QFileDialog
    //It enable a sort which is not recursive, it means
    //we sort only what we see.
    bool disableRecursiveSort;
    bool sortIgnoringPunctuation;
#ifndef QT_NO_REGEXP
    QStringList nameFilters;
#endif
    QHash<QString, QString> resolvedSymLinks;

    FileSystemNode root;

    QBasicTimer fetchingTimer;
    struct Fetching {
        QString dir;
        QString file;
        const FileSystemNode *node;
    };
    QVector<Fetching> toFetch;

#ifndef USE_QT_PRIVATE_HEADERS
    QObject * const q_ptr;
#endif
};
Q_DECLARE_TYPEINFO(FileSystemModelPrivate::Fetching, Q_MOVABLE_TYPE);
/** \endcond */
