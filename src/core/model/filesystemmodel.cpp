/**
 * \file filesystemmodel.cpp
 * \cond
 * Taken from Qt Git, revision e73bd4a
 * qtbase/src/widgets/dialogs/qfilesystemmodel.cpp
 * Adapted for Kid3 with the following changes:
 * - Remove Q prefix from class names
 * - Remove QT_..._CONFIG, QT_..._NAMESPACE, Q_..._EXPORT...
 * - Allow compilation without Qt private headers (USE_QT_PRIVATE_HEADERS)
 * - Remove moc includes
 * - Remove dependencies to Qt5::Widgets
 * - Do not display a message box from setData(), this will crash without GUI
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

#include "filesystemmodel.h"
#include <qlocale.h>
#include <qmimedata.h>
#include <qurl.h>
#include <qdebug.h>
#include <qcoreevent.h>
#include <QtCore/qcollator.h>
#include <QRegularExpression>

#include <algorithm>
#include <memory>

#ifdef Q_OS_WIN
#  include <QtCore/QVarLengthArray>
#  include <qt_windows.h>
#endif

#include "abstractfiledecorationprovider.h"

/*!
    \enum FileSystemModel::Roles
    \value FileIconRole
    \value FilePathRole
    \value FileNameRole
    \value FilePermissions
*/

/*!
    \class FileSystemModel
    \since 4.4

    \brief The QFileSystemModel class provides a data model for the local filesystem.

    \ingroup model-view
    \inmodule QtWidgets

    This class provides access to the local filesystem, providing functions
    for renaming and removing files and directories, and for creating new
    directories. In the simplest case, it can be used with a suitable display
    widget as part of a browser or filter.

    QFileSystemModel can be accessed using the standard interface provided by
    QAbstractItemModel, but it also provides some convenience functions that are
    specific to a directory model.
    The fileInfo(), isDir(), fileName() and filePath() functions provide information
    about the underlying files and directories related to items in the model.
    Directories can be created and removed using mkdir(), rmdir().

    \note QFileSystemModel requires an instance of \l QApplication.

    \section1 Example Usage

    A directory model that displays the contents of a default directory
    is usually constructed with a parent object:

    \snippet shareddirmodel/main.cpp 2

    A tree view can be used to display the contents of the model

    \snippet shareddirmodel/main.cpp 4

    and the contents of a particular directory can be displayed by
    setting the tree view's root index:

    \snippet shareddirmodel/main.cpp 7

    The view's root index can be used to control how much of a
    hierarchical model is displayed. QFileSystemModel provides a convenience
    function that returns a suitable model index for a path to a
    directory within the model.

    \section1 Caching and Performance

    QFileSystemModel will not fetch any files or directories until setRootPath()
    is called.  This will prevent any unnecessary querying on the file system
    until that point such as listing the drives on Windows.

    Unlike QDirModel, QFileSystemModel uses a separate thread to populate
    itself so it will not cause the main thread to hang as the file system
    is being queried.  Calls to rowCount() will return 0 until the model
    populates a directory.

    QFileSystemModel keeps a cache with file information. The cache is
    automatically kept up to date using the QFileSystemWatcher.

    \sa {Model Classes}
*/

/*!
    \fn bool QFileSystemModel::rmdir(const QModelIndex &index)

    Removes the directory corresponding to the model item \a index in the
    file system model and \b{deletes the corresponding directory from the
    file system}, returning true if successful. If the directory cannot be
    removed, false is returned.

    \warning This function deletes directories from the file system; it does
    \b{not} move them to a location where they can be recovered.

    \sa remove()
*/

/*!
    \fn QIcon QFileSystemModel::fileName(const QModelIndex &index) const

    Returns the file name for the item stored in the model under the given
    \a index.
*/

/*!
    \fn QIcon QFileSystemModel::fileIcon(const QModelIndex &index) const

    Returns the icon for the item stored in the model under the given
    \a index.
*/

/*!
    \fn QFileInfo QFileSystemModel::fileInfo(const QModelIndex &index) const

    Returns the QFileInfo for the item stored in the model under the given
    \a index.
*/
QFileInfo FileSystemModel::fileInfo(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    return d->node(index)->fileInfo();
}

/*!
    \fn void QFileSystemModel::rootPathChanged(const QString &newPath);

    This signal is emitted whenever the root path has been changed to a \a newPath.
*/

/*!
    \fn void QFileSystemModel::fileRenamed(const QString &path, const QString &oldName, const QString &newName)

    This signal is emitted whenever a file with the \a oldName is successfully
    renamed to \a newName.  The file is located in the directory \a path.
*/

/*!
    \since 4.7
    \fn void QFileSystemModel::directoryLoaded(const QString &path)

    This signal is emitted when the gatherer thread has finished to load the \a path.

*/

/*!
    \fn bool QFileSystemModel::remove(const QModelIndex &index)

    Removes the model item \a index from the file system model and \b{deletes the
    corresponding file from the file system}, returning true if successful. If the
    item cannot be removed, false is returned.

    \warning This function deletes files from the file system; it does \b{not}
    move them to a location where they can be recovered.

    \sa rmdir()
*/

bool FileSystemModel::remove(const QModelIndex &aindex)
{
    Q_D(FileSystemModel);

    const QString path = d->filePath(aindex);
    const QFileInfo fileInfo(path);
    const bool success = fileInfo.isFile() || fileInfo.isSymLink()
            ? QFile::remove(path) : QDir(path).removeRecursively();
#ifndef QT_NO_FILESYSTEMWATCHER
    if (success) {
        auto dptr = const_cast<FileSystemModelPrivate*>(d_func());
        dptr->fileInfoGatherer.removePath(path);
    }
#endif
    return success;
}

/*!
  Constructs a file system model with the given \a parent.
*/
FileSystemModel::FileSystemModel(QObject *parent)
#ifdef USE_QT_PRIVATE_HEADERS
    : QAbstractItemModel(*new FileSystemModelPrivate, parent)
#else
    : QAbstractItemModel(parent),
      d_ptr(new FileSystemModelPrivate(this))
#endif
{
    Q_D(FileSystemModel);
    d->init();
}

/*!
    \internal
*/
FileSystemModel::FileSystemModel(FileSystemModelPrivate &dd, QObject *parent)
#if defined(USE_QT_PRIVATE_HEADERS)
    : QAbstractItemModel(dd, parent)
#else
    : QAbstractItemModel(parent),
      d_ptr(&dd)
#endif
{
    Q_D(FileSystemModel);
    d->init();
}

/*!
  Destroys this file system model.
*/
FileSystemModel::~FileSystemModel()
{
}

/*!
    \reimp
*/
QModelIndex FileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const FileSystemModel);
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    // get the parent node
    FileSystemModelPrivate::FileSystemNode *parentNode = d->indexValid(parent) ? d->node(parent) :
                                                   const_cast<FileSystemModelPrivate::FileSystemNode*>(&d->root);
    Q_ASSERT(parentNode);

    // now get the internal pointer for the index
    const int i = d->translateVisibleLocation(parentNode, row);
    if (i >= parentNode->visibleChildren.size())
        return QModelIndex();
    const QString &childName = parentNode->visibleChildren.at(i);
    const FileSystemModelPrivate::FileSystemNode *indexNode = parentNode->children.value(childName);
    Q_ASSERT(indexNode);

    return createIndex(row, column, const_cast<FileSystemModelPrivate::FileSystemNode*>(indexNode));
}

/*!
    \reimp
*/
QModelIndex FileSystemModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (row == idx.row() && column < FileSystemModelPrivate::NumColumns) {
        // cheap sibling operation: just adjust the column:
        return createIndex(row, column, idx.internalPointer());
    }
    // for anything else: call the default implementation
    // (this could probably be optimized, too):
    return QAbstractItemModel::sibling(row, column, idx);
}

/*!
    \overload

    Returns the model item index for the given \a path and \a column.
*/
QModelIndex FileSystemModel::index(const QString &path, int column) const
{
    Q_D(const FileSystemModel);
    FileSystemModelPrivate::FileSystemNode *node = d->node(path, false);
    return d->index(node, column);
}

/*!
    \internal

    Return the QFileSystemNode that goes to index.
  */
FileSystemModelPrivate::FileSystemNode *FileSystemModelPrivate::node(const QModelIndex &index) const
{
    if (!index.isValid())
        return const_cast<FileSystemNode*>(&root);
    auto indexNode = static_cast<FileSystemModelPrivate::FileSystemNode*>(index.internalPointer());
    Q_ASSERT(indexNode);
    return indexNode;
}

#ifdef Q_OS_WIN32
static QString qt_GetLongPathName(const QString &strShortPath)
{
    if (strShortPath.isEmpty()
        || strShortPath == QLatin1String(".") || strShortPath == QLatin1String(".."))
        return strShortPath;
    if (strShortPath.length() == 2 && strShortPath.endsWith(QLatin1Char(':')))
        return strShortPath.toUpper();
    const QString absPath = QDir(strShortPath).absolutePath();
    if (absPath.startsWith(QLatin1String("//"))
        || absPath.startsWith(QLatin1String("\\\\"))) // unc
        return QDir::fromNativeSeparators(absPath);
    if (absPath.startsWith(QLatin1Char('/')))
        return QString();
    const QString inputString = QLatin1String("\\\\?\\") + QDir::toNativeSeparators(absPath);
    // Kid3 WCHAR instead of TCHAR, GetLongPathNameW instead of GetLongPathName
    QVarLengthArray<WCHAR, MAX_PATH> buffer(MAX_PATH);
    DWORD result = ::GetLongPathNameW((wchar_t*)inputString.utf16(),
                                     buffer.data(),
                                     buffer.size());
    if (result > DWORD(buffer.size())) {
        buffer.resize(result);
        result = ::GetLongPathNameW((wchar_t*)inputString.utf16(),
                                   buffer.data(),
                                   buffer.size());
    }
    if (result > 4) {
        QString longPath = QString::fromWCharArray(buffer.data() + 4); // ignoring prefix
        longPath[0] = longPath.at(0).toUpper(); // capital drive letters
        return QDir::fromNativeSeparators(longPath);
    } else {
        return QDir::fromNativeSeparators(strShortPath);
    }
}
#endif

/*!
    \internal

    Given a path return the matching QFileSystemNode or &root if invalid
*/
FileSystemModelPrivate::FileSystemNode *FileSystemModelPrivate::node(const QString &path, bool fetch) const
{
    Q_Q(const FileSystemModel);
    Q_UNUSED(q);
    if (path.isEmpty() || path == myComputer() || path.startsWith(QLatin1Char(':')))
        return const_cast<FileSystemModelPrivate::FileSystemNode*>(&root);

    // Construct the nodes up to the new root path if they need to be built
    QString absolutePath;
#ifdef Q_OS_WIN32
    QString longPath = qt_GetLongPathName(path);
#else
    QString longPath = path;
#endif
    if (longPath == rootDir.path())
        absolutePath = rootDir.absolutePath();
    else
        absolutePath = QDir(longPath).absolutePath();

    // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
#if QT_VERSION >= 0x050e00
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
#else
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
#endif
    if (pathElements.isEmpty()
#if !defined(Q_OS_WIN)
        && QDir::fromNativeSeparators(longPath) != QLatin1String("/")
#endif
        )
        return const_cast<FileSystemModelPrivate::FileSystemNode*>(&root);
    auto index = QModelIndex(); // start with "My Computer"
    QString elementPath;
    QChar separator = QLatin1Char('/');
    QString trailingSeparator;
#if defined(Q_OS_WIN)
    if (absolutePath.startsWith(QLatin1String("//"))) { // UNC path
        QString host = QLatin1String("\\\\") + pathElements.constFirst();
        if (absolutePath == QDir::fromNativeSeparators(host))
            absolutePath.append(QLatin1Char('/'));
        if (longPath.endsWith(QLatin1Char('/')) && !absolutePath.endsWith(QLatin1Char('/')))
            absolutePath.append(QLatin1Char('/'));
        if (absolutePath.endsWith(QLatin1Char('/')))
            trailingSeparator = QLatin1String("\\");
        int r = 0;
        FileSystemModelPrivate::FileSystemNode *rootNode = const_cast<FileSystemModelPrivate::FileSystemNode*>(&root);
        if (!root.children.contains(host.toLower())) {
            if (pathElements.count() == 1 && !absolutePath.endsWith(QLatin1Char('/')))
                return rootNode;
            QFileInfo info(host);
            if (!info.exists())
                return rootNode;
            FileSystemModelPrivate *p = const_cast<FileSystemModelPrivate*>(this);
            p->addNode(rootNode, host,info);
            p->addVisibleFiles(rootNode, QStringList(host));
        }
        r = rootNode->visibleLocation(host);
        r = translateVisibleLocation(rootNode, r);
        index = q->index(r, 0, QModelIndex());
        pathElements.pop_front();
        separator = QLatin1Char('\\');
        elementPath = host;
        elementPath.append(separator);
    } else {
        if (!pathElements.at(0).contains(QLatin1Char(':'))) {
            QString rootPath = QDir(longPath).rootPath();
            pathElements.prepend(rootPath);
        }
        if (pathElements.at(0).endsWith(QLatin1Char('/')))
            pathElements[0].chop(1);
    }
#else
    // add the "/" item, since it is a valid path element on Unix
    if (absolutePath[0] == QLatin1Char('/'))
        pathElements.prepend(QLatin1String("/"));
#endif

    FileSystemModelPrivate::FileSystemNode *parent = node(index);

    for (int i = 0; i < pathElements.count(); ++i) {
        QString element = pathElements.at(i);
        if (i != 0)
            elementPath.append(separator);
        elementPath.append(element);
        if (i == pathElements.count() - 1)
            elementPath.append(trailingSeparator);
#ifdef Q_OS_WIN
        // On Windows, "filename    " and "filename" are equivalent and
        // "filename  .  " and "filename" are equivalent
        // "filename......." and "filename" are equivalent Task #133928
        // whereas "filename  .txt" is still "filename  .txt"
        // If after stripping the characters there is nothing left then we
        // just return the parent directory as it is assumed that the path
        // is referring to the parent
        while (element.endsWith(QLatin1Char('.')) || element.endsWith(QLatin1Char(' ')))
            element.chop(1);
        // Only filenames that can't possibly exist will be end up being empty
        if (element.isEmpty())
            return parent;
#endif
        bool alreadyExisted = parent->children.contains(element);

        // we couldn't find the path element, we create a new node since we
        // _know_ that the path is valid
        if (alreadyExisted) {
            if (parent->children.count() == 0
                || (parent->caseSensitive()
                    && parent->children.value(element)->fileName != element)
                || (!parent->caseSensitive()
                    && parent->children.value(element)->fileName.toLower() != element.toLower()))
                alreadyExisted = false;
        }

        FileSystemModelPrivate::FileSystemNode *node;
        if (!alreadyExisted) {
            // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
            // a path that doesn't exists, I.E. don't blindly create directories.
            QFileInfo info(elementPath);
            if (!info.exists())
                return const_cast<FileSystemModelPrivate::FileSystemNode*>(&root);
            auto p = const_cast<FileSystemModelPrivate*>(this);
            node = p->addNode(parent, element,info);
#ifndef QT_NO_FILESYSTEMWATCHER
            node->populate(fileInfoGatherer.getInfo(info));
#endif
        } else {
            node = parent->children.value(element);
        }

        Q_ASSERT(node);
        if (!node->isVisible) {
            // It has been filtered out
            if (alreadyExisted && node->hasInformation() && !fetch)
                return const_cast<FileSystemModelPrivate::FileSystemNode*>(&root);

            auto p = const_cast<FileSystemModelPrivate*>(this);
            p->addVisibleFiles(parent, QStringList(element));
            if (!p->bypassFilters.contains(node))
                p->bypassFilters[node] = true;
            QString dir = q->filePath(this->index(parent));
            if (!node->hasInformation() && fetch) {
                Fetching f = { std::move(dir), std::move(element), node };
                p->toFetch.append(std::move(f));
                p->fetchingTimer.start(0, const_cast<FileSystemModel*>(q));
            }
        }
        parent = node;
    }

    return parent;
}

/*!
    \reimp
*/
void FileSystemModel::timerEvent(QTimerEvent *event)
{
    Q_D(FileSystemModel);
    if (event->timerId() == d->fetchingTimer.timerId()) {
        d->fetchingTimer.stop();
#ifndef QT_NO_FILESYSTEMWATCHER
        for (int i = 0; i < d->toFetch.count(); ++i) {
            if (const FileSystemModelPrivate::FileSystemNode *node = d->toFetch.at(i).node;
                !node->hasInformation()) {
                d->fileInfoGatherer.fetchExtendedInformation(d->toFetch.at(i).dir,
                                                 QStringList(d->toFetch.at(i).file));
            } else {
                // qDebug("yah!, you saved a little gerbil soul");
            }
        }
#endif
        d->toFetch.clear();
    }
}

/*!
    Returns \c true if the model item \a index represents a directory;
    otherwise returns \c false.
*/
bool FileSystemModel::isDir(const QModelIndex &index) const
{
    // This function is for public usage only because it could create a file info
    Q_D(const FileSystemModel);
    if (!index.isValid())
        return true;
    if (FileSystemModelPrivate::FileSystemNode *n = d->node(index);
        n->hasInformation())
        return n->isDir();
    return fileInfo(index).isDir();
}

/*!
    Returns the size in bytes of \a index. If the file does not exist, 0 is returned.
  */
qint64 FileSystemModel::size(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    if (!index.isValid())
        return 0;
    return d->node(index)->size();
}

/*!
    Returns the type of file \a index such as "Directory" or "JPEG file".
  */
QString FileSystemModel::type(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    if (!index.isValid())
        return QString();
    return d->node(index)->type();
}

/*!
    Returns the date and time when \a index was last modified.
 */
QDateTime FileSystemModel::lastModified(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    if (!index.isValid())
        return QDateTime();
    return d->node(index)->lastModified();
}

/*!
    \reimp
*/
QModelIndex FileSystemModel::parent(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    if (!d->indexValid(index))
        return QModelIndex();

    FileSystemModelPrivate::FileSystemNode *indexNode = d->node(index);
    Q_ASSERT(indexNode != nullptr);
    FileSystemModelPrivate::FileSystemNode *parentNode = indexNode->parent;
    if (parentNode == nullptr || parentNode == &d->root)
        return QModelIndex();

    // get the parent's row
    FileSystemModelPrivate::FileSystemNode *grandParentNode = parentNode->parent;
    Q_ASSERT(grandParentNode->children.contains(parentNode->fileName));
    int visualRow = d->translateVisibleLocation(grandParentNode, grandParentNode->visibleLocation(grandParentNode->children.value(parentNode->fileName)->fileName));
    if (visualRow == -1)
        return QModelIndex();
    return createIndex(visualRow, 0, parentNode);
}

/*
    \internal

    return the index for node
*/
QModelIndex FileSystemModelPrivate::index(const FileSystemModelPrivate::FileSystemNode *node, int column) const
{
    Q_Q(const FileSystemModel);
    FileSystemModelPrivate::FileSystemNode *parentNode = node ? node->parent : nullptr;
    if (node == &root || !parentNode)
        return QModelIndex();

    // get the parent's row
    Q_ASSERT(node);
    if (!node->isVisible)
        return QModelIndex();

    int visualRow = translateVisibleLocation(parentNode, parentNode->visibleLocation(node->fileName));
    return q->createIndex(visualRow, column, const_cast<FileSystemNode*>(node));
}

/*!
    \reimp
*/
bool FileSystemModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const FileSystemModel);
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) // drives
        return true;

    const FileSystemModelPrivate::FileSystemNode *indexNode = d->node(parent);
    Q_ASSERT(indexNode);
    return indexNode->isDir();
}

/*!
    \reimp
 */
bool FileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    Q_D(const FileSystemModel);
    const FileSystemModelPrivate::FileSystemNode *indexNode = d->node(parent);
    return !indexNode->populatedChildren;
}

/*!
    \reimp
 */
void FileSystemModel::fetchMore(const QModelIndex &parent)
{
    Q_D(FileSystemModel);
    if (!d->setRootPath)
        return;
    FileSystemModelPrivate::FileSystemNode *indexNode = d->node(parent);
    if (indexNode->populatedChildren)
        return;
    indexNode->populatedChildren = true;
#ifndef QT_NO_FILESYSTEMWATCHER
    d->fileInfoGatherer.list(filePath(parent));
#endif
}

/*!
    \reimp
*/
int FileSystemModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const FileSystemModel);
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        return d->root.visibleChildren.count();

    const FileSystemModelPrivate::FileSystemNode *parentNode = d->node(parent);
    return parentNode->visibleChildren.count();
}

/*!
    \reimp
*/
int FileSystemModel::columnCount(const QModelIndex &parent) const
{
    return parent.column() > 0 ? 0 : FileSystemModelPrivate::NumColumns;
}

/*!
    Returns the data stored under the given \a role for the item "My Computer".

    \sa Qt::ItemDataRole
 */
QVariant FileSystemModel::myComputer(int role) const
{
#ifndef QT_NO_FILESYSTEMWATCHER
    Q_D(const FileSystemModel);
#endif
    switch (role) {
    case Qt::DisplayRole:
        return FileSystemModelPrivate::myComputer();
#ifndef QT_NO_FILESYSTEMWATCHER
    case Qt::DecorationRole:
        return d->fileInfoGatherer.decorationProvider()
                ? d->fileInfoGatherer.decorationProvider()->computerDecoration()
                : QVariant();
#endif
    }
    return QVariant();
}

/*!
    \reimp
*/
QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    Q_D(const FileSystemModel);
    if (!index.isValid() || index.model() != this)
        return QVariant();

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return d->displayName(index);
        case 1: return d->size(index);
        case 2: return d->type(index);
        case 3: return d->time(index);
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case FilePathRole:
        return filePath(index);
    case FileNameRole:
        return d->name(index);
    case Qt::DecorationRole:
        if (index.column() == 0) {
            QVariant icon = d->icon(index);
#ifndef QT_NO_FILESYSTEMWATCHER
            if (icon.isNull()) {
                if (d->node(index)->isDir())
                    icon = d->fileInfoGatherer.decorationProvider()
                            ? d->fileInfoGatherer.decorationProvider()->folderDecoration()
                            : QVariant();
                else
                    icon = d->fileInfoGatherer.decorationProvider()
                            ? d->fileInfoGatherer.decorationProvider()->fileDecoration()
                            : QVariant();
            }
#endif // QT_NO_FILESYSTEMWATCHER
            return icon;
        }
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 1)
            return QVariant(Qt::AlignTrailing | Qt::AlignVCenter);
        break;
    case FilePermissions:
        int p = permissions(index);
        return p;
    }

    return QVariant();
}

/*!
    \internal
*/
QString FileSystemModelPrivate::size(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    const FileSystemNode *n = node(index);
    if (n->isDir()) {
#ifdef Q_OS_MAC
        return QLatin1String("--");
#else
        return QLatin1String("");
#endif
    // Windows   - ""
    // OS X      - "--"
    // Konqueror - "4 KB"
    // Nautilus  - "9 items" (the number of children)
    }
    return size(n->size());
}

QString FileSystemModelPrivate::size(qint64 bytes)
{
    // According to the Si standard KB is 1000 bytes, KiB is 1024
    // but on windows sizes are calculated by dividing by 1024 so we do what they do.
    constexpr qint64 kb = 1024;
    constexpr qint64 mb = 1024 * kb;
    constexpr qint64 gb = 1024 * mb;
    constexpr qint64 tb = 1024 * gb;
    if (bytes >= tb)
        return FileSystemModel::tr("%1 TB").arg(QLocale().toString(static_cast<qreal>(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return FileSystemModel::tr("%1 GB").arg(QLocale().toString(static_cast<qreal>(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return FileSystemModel::tr("%1 MB").arg(QLocale().toString(static_cast<qreal>(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return FileSystemModel::tr("%1 KB").arg(QLocale().toString(bytes / kb));
    return FileSystemModel::tr("%1 bytes").arg(QLocale().toString(bytes));
}

/*!
    \internal
*/
QString FileSystemModelPrivate::time(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
#ifndef QT_NO_DATESTRING
    return QLocale::system().toString(node(index)->lastModified(), QLocale::ShortFormat);
#else
    Q_UNUSED(index);
    return QString();
#endif
}

/*
    \internal
*/
QString FileSystemModelPrivate::type(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    return node(index)->type();
}

/*!
    \internal
*/
QString FileSystemModelPrivate::name(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();
    FileSystemNode *dirNode = node(index);
    if (
#ifndef QT_NO_FILESYSTEMWATCHER
        fileInfoGatherer.resolveSymlinks() &&
#endif
        !resolvedSymLinks.isEmpty() && dirNode->isSymLink(/* ignoreNtfsSymLinks = */ true)) {
        QString fullPath = QDir::fromNativeSeparators(filePath(index));
        return resolvedSymLinks.value(fullPath, dirNode->fileName);
    }
    return dirNode->fileName;
}

/*!
    \internal
*/
QString FileSystemModelPrivate::displayName(const QModelIndex &index) const
{
#if defined(Q_OS_WIN)
    FileSystemNode *dirNode = node(index);
    if (!dirNode->volumeName.isNull())
        return dirNode->volumeName + QLatin1String(" (") + name(index) + QLatin1Char(')');
#endif
    return name(index);
}

/*!
    \internal
*/
QVariant FileSystemModelPrivate::icon(const QModelIndex &index) const
{
    if (!index.isValid())
        return QVariant();
    return node(index)->icon();
}

/*!
    \reimp
*/
bool FileSystemModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    Q_D(FileSystemModel);
    if (!idx.isValid()
        || idx.column() != 0
        || role != Qt::EditRole
        || (flags(idx) & Qt::ItemIsEditable) == 0) {
        return false;
    }

    QString newName = value.toString();
    QString oldName = idx.data().toString();
    if (newName == oldName)
        return true;

    if (const QString parentPath = filePath(parent(idx));
        newName.isEmpty()
        || QDir::toNativeSeparators(newName).contains(QDir::separator())
        || !QDir(parentPath).rename(oldName, newName)) {
        emit fileRenameFailed(parentPath, oldName, newName);
        return false;
    } else {
        /*
            *After re-naming something we don't want the selection to change*
            - can't remove rows and later insert
            - can't quickly remove and insert
            - index pointer can't change because treeview doesn't use persistent index's

            - if this get any more complicated think of changing it to just
              use layoutChanged
         */

        FileSystemModelPrivate::FileSystemNode *indexNode = d->node(idx);
        FileSystemModelPrivate::FileSystemNode *parentNode = indexNode->parent;
        int visibleLocation = parentNode->visibleLocation(parentNode->children.value(indexNode->fileName)->fileName);

        parentNode->visibleChildren.removeAt(visibleLocation);
        std::unique_ptr<FileSystemModelPrivate::FileSystemNode> nodeToRename(parentNode->children.take(oldName));
        nodeToRename->fileName = newName;
        nodeToRename->parent = parentNode;
#ifndef QT_NO_FILESYSTEMWATCHER
        nodeToRename->populate(d->fileInfoGatherer.getInfo(QFileInfo(parentPath, newName)));
        if (nodeToRename->isDir()) {
            // Update watcher when directory is renamed
            d->fileInfoGatherer.removePath(parentPath + QLatin1Char('/') + oldName);
            d->fileInfoGatherer.addPath(parentPath + QLatin1Char('/') + newName);
        }
#endif
        nodeToRename->isVisible = true;
        parentNode->children[newName] = nodeToRename.release();
        parentNode->visibleChildren.insert(visibleLocation, newName);

        d->delayedSort();
        emit fileRenamed(parentPath, oldName, newName);
    }
    return true;
}

/*!
    \reimp
*/
QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        if (section == 0) {
            Q_D(const FileSystemModel);
            return d->fileInfoGatherer.decorationProvider()
                    ? d->fileInfoGatherer.decorationProvider()->headerDecoration()
                    : QVariant();
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section) {
    case 0: returnValue = tr("Name");
            break;
    case 1: returnValue = tr("Size");
            break;
    case 2: returnValue =
#ifdef Q_OS_MAC
                   tr("Kind", "Match OS X Finder");
#else
                   tr("Type", "All other platforms");
#endif
           break;
    // Windows   - Type
    // OS X      - Kind
    // Konqueror - File Type
    // Nautilus  - Type
    case 3: returnValue = tr("Date Modified");
            break;
    default: return QVariant();
    }
    return returnValue;
}

/*!
    \reimp
*/
Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    FileSystemModelPrivate::FileSystemNode *indexNode = d->node(index);
    if (d->nameFilterDisables && !d->passNameFilters(indexNode)) {
        flags &= ~Qt::ItemIsEnabled;
        // ### TODO you shouldn't be able to set this as the current item, task 119433
        return flags;
    }

    flags |= Qt::ItemIsDragEnabled;
    if (d->readOnly)
        return flags;
    if (index.column() == 0 && indexNode->permissions() & QFile::WriteUser) {
        flags |= Qt::ItemIsEditable;
        if (indexNode->isDir())
            flags |= Qt::ItemIsDropEnabled;
        else
            flags |= Qt::ItemNeverHasChildren;
    }
    return flags;
}

/*!
    \internal
*/
void FileSystemModelPrivate::_q_performDelayedSort()
{
    Q_Q(FileSystemModel);
    q->sort(sortColumn, sortOrder);
}


/*
    \internal
    Helper functor used by sort()
*/
class FileSystemModelSorter
{
public:
    explicit FileSystemModelSorter(int column, bool ignorePunctuation = false) : sortColumn(column)
    {
        naturalCompare.setIgnorePunctuation(ignorePunctuation);
        naturalCompare.setNumericMode(true);
        naturalCompare.setCaseSensitivity(Qt::CaseInsensitive);
    }

    bool compareNodes(const FileSystemModelPrivate::FileSystemNode *l,
                    const FileSystemModelPrivate::FileSystemNode *r) const
    {
        switch (sortColumn) {
        case 0: {
#ifndef Q_OS_MAC
            // place directories before files
            bool left = l->isDir();
            bool right = r->isDir();
            if (left ^ right)
                return left;
#endif
            return naturalCompare.compare(l->fileName, r->fileName) < 0;
                }
        case 1:
        {
            // Directories go first
            bool left = l->isDir();
            bool right = r->isDir();
            if (left ^ right)
                return left;

            qint64 sizeDifference = l->size() - r->size();
            if (sizeDifference == 0)
                return naturalCompare.compare(l->fileName, r->fileName) < 0;

            return sizeDifference < 0;
        }
        case 2:
        {
            int compare = naturalCompare.compare(l->type(), r->type());
            if (compare == 0)
                return naturalCompare.compare(l->fileName, r->fileName) < 0;

            return compare < 0;
        }
        case 3:
        {
            if (l->lastModified() == r->lastModified())
                return naturalCompare.compare(l->fileName, r->fileName) < 0;

            return l->lastModified() < r->lastModified();
        }
        }
        Q_ASSERT(false);
        return false;
    }

    bool operator()(const FileSystemModelPrivate::FileSystemNode *l,
                    const FileSystemModelPrivate::FileSystemNode *r) const
    {
        return compareNodes(l, r);
    }


private:
    QCollator naturalCompare;
    int sortColumn;
};

/*
    \internal

    Sort all of the children of parent
*/
void FileSystemModelPrivate::sortChildren(int column, const QModelIndex &parent)
{
    Q_Q(FileSystemModel);
    FileSystemModelPrivate::FileSystemNode *indexNode = node(parent);
    if (indexNode->children.count() == 0)
        return;

    QVector<FileSystemModelPrivate::FileSystemNode*> values;

    for (auto iterator = indexNode->children.constBegin(), cend = indexNode->children.constEnd(); iterator != cend; ++iterator) {
        if (filtersAcceptsNode(iterator.value())) {
            values.append(iterator.value());
        } else {
            iterator.value()->isVisible = false;
        }
    }
    FileSystemModelSorter ms(column, sortIgnoringPunctuation);
    std::sort(values.begin(), values.end(), ms);
    // First update the new visible list
    indexNode->visibleChildren.clear();
    //No more dirty item we reset our internal dirty index
    indexNode->dirtyChildrenIndex = -1;
    const int numValues = values.count();
    indexNode->visibleChildren.reserve(numValues);
    for (int i = 0; i < numValues; ++i) {
        indexNode->visibleChildren.append(values.at(i)->fileName);
        values.at(i)->isVisible = true;
    }

    if (!disableRecursiveSort) {
        for (int i = 0; i < q->rowCount(parent); ++i) {
            const QModelIndex childIndex = q->index(i, 0, parent);
            //Only do a recursive sort on visible nodes
            if (FileSystemModelPrivate::FileSystemNode *childIndexNode = node(childIndex);
                childIndexNode->isVisible)
                sortChildren(column, childIndex);
        }
    }
}

/*!
    \reimp
*/
void FileSystemModel::sort(int column, Qt::SortOrder order)
{
    Q_D(FileSystemModel);
    if (d->sortOrder == order && d->sortColumn == column && !d->forceSort)
        return;

    emit layoutAboutToBeChanged();
    QModelIndexList oldList = persistentIndexList();
    QVector<QPair<FileSystemModelPrivate::FileSystemNode*, int> > oldNodes;
    const int nodeCount = oldList.count();
    oldNodes.reserve(nodeCount);
    for (int i = 0; i < nodeCount; ++i) {
        const QModelIndex &oldNode = oldList.at(i);
        QPair<FileSystemModelPrivate::FileSystemNode*, int> pair(d->node(oldNode), oldNode.column());
        oldNodes.append(pair);
    }

    if (!(d->sortColumn == column && d->sortOrder != order && !d->forceSort)) {
        //we sort only from where we are, don't need to sort all the model
        d->sortChildren(column, index(rootPath()));
        d->sortColumn = column;
        d->forceSort = false;
    }
    d->sortOrder = order;

    QModelIndexList newList;
    const int numOldNodes = oldNodes.size();
    newList.reserve(numOldNodes);
    for (int i = 0; i < numOldNodes; ++i) {
        const QPair<FileSystemModelPrivate::FileSystemNode*, int> &oldNode = oldNodes.at(i);
        newList.append(d->index(oldNode.first, oldNode.second));
    }
    changePersistentIndexList(oldList, newList);
    emit layoutChanged();
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/
QStringList FileSystemModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

/*!
    Returns an object that contains a serialized description of the specified
    \a indexes. The format used to describe the items corresponding to the
    indexes is obtained from the mimeTypes() function.

    If the list of indexes is empty, 0 is returned rather than a serialized
    empty list.
*/
QMimeData *FileSystemModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    for (QList<QModelIndex>::const_iterator it = indexes.begin();
         it != indexes.end(); ++it)
        if (it->column() == 0)
            urls << QUrl::fromLocalFile(filePath(*it));
    auto data = new QMimeData();
    data->setUrls(urls);
    return data;
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action over the row in the model specified by the \a row and
    \a column and by the \a parent index.

    \sa supportedDropActions()
*/
bool FileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!parent.isValid() || isReadOnly())
        return false;

    bool success = true;
    QString to = filePath(parent) + QDir::separator();

    QList<QUrl> urls = data->urls();
    QList<QUrl>::const_iterator it = urls.constBegin();

    switch (action) {
    case Qt::CopyAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = it->toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::LinkAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = it->toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    case Qt::MoveAction:
        for (; it != urls.constEnd(); ++it) {
            QString path = it->toLocalFile();
            success = QFile::rename(path, to + QFileInfo(path).fileName()) && success;
        }
        break;
    default:
        return false;
    }

    return success;
}

/*!
    \reimp
*/
Qt::DropActions FileSystemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

/*!
    Returns the path of the item stored in the model under the
    \a index given.
*/
QString FileSystemModel::filePath(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    QString fullPath = d->filePath(index);
    if (FileSystemModelPrivate::FileSystemNode *dirNode = d->node(index);
        dirNode->isSymLink()
#ifndef QT_NO_FILESYSTEMWATCHER
        && d->fileInfoGatherer.resolveSymlinks()
#endif
        && d->resolvedSymLinks.contains(fullPath)
        && dirNode->isDir()) {
        QFileInfo resolvedInfo(fullPath);
        resolvedInfo = QFileInfo(resolvedInfo.canonicalFilePath());
        if (resolvedInfo.exists())
            return resolvedInfo.filePath();
    }
    return fullPath;
}

QString FileSystemModelPrivate::filePath(const QModelIndex &index) const
{
    Q_Q(const FileSystemModel);
    Q_UNUSED(q);
    if (!index.isValid())
        return QString();
    Q_ASSERT(index.model() == q);

    QStringList path;
    QModelIndex idx = index;
    while (idx.isValid()) {
        if (FileSystemModelPrivate::FileSystemNode *dirNode = node(idx))
            path.prepend(dirNode->fileName);
        idx = idx.parent();
    }
    QString fullPath = QDir::fromNativeSeparators(path.join(QDir::separator()));
#if !defined(Q_OS_WIN)
    if (fullPath.length() > 2 && fullPath[0] == QLatin1Char('/') && fullPath[1] == QLatin1Char('/'))
        fullPath = fullPath.mid(1);
#else
    if (fullPath.length() == 2 && fullPath.endsWith(QLatin1Char(':')))
        fullPath.append(QLatin1Char('/'));
#endif
    return fullPath;
}

/*!
    Create a directory with the \a name in the \a parent model index.
*/
QModelIndex FileSystemModel::mkdir(const QModelIndex &parent, const QString &name)
{
    Q_D(FileSystemModel);
    if (!parent.isValid())
        return parent;

    QDir dir(filePath(parent));
    if (!dir.mkdir(name))
        return QModelIndex();
    FileSystemModelPrivate::FileSystemNode *parentNode = d->node(parent);
    d->addNode(parentNode, name, QFileInfo());
    Q_ASSERT(parentNode->children.contains(name));
    FileSystemModelPrivate::FileSystemNode *node = parentNode->children[name];
#ifndef QT_NO_FILESYSTEMWATCHER
    node->populate(d->fileInfoGatherer.getInfo(QFileInfo(dir.absolutePath() + QDir::separator() + name)));
#endif
    d->addVisibleFiles(parentNode, QStringList(name));
    return d->index(node);
}

/*!
    Returns the complete OR-ed together combination of QFile::Permission for the \a index.
 */
QFile::Permissions FileSystemModel::permissions(const QModelIndex &index) const
{
    Q_D(const FileSystemModel);
    return d->node(index)->permissions();
}

/*!
    Free resources used by file system model and reset root path to default.
    The model state will be as after creation, but all properties except the
    root path remain the same.
 */
void FileSystemModel::clear()
{
    Q_D(FileSystemModel);
    beginResetModel();
    d->clear();
    endResetModel();
}

/*!
    Sets the directory that is being watched by the model to \a newPath by
    installing a \l{QFileSystemWatcher}{file system watcher} on it. Any
    changes to files and directories within this directory will be
    reflected in the model.

    If the path is changed, the rootPathChanged() signal will be emitted.

    \note This function does not change the structure of the model or
    modify the data available to views. In other words, the "root" of
    the model is \e not changed to include only files and directories
    within the directory specified by \a newPath in the file system.
  */
QModelIndex FileSystemModel::setRootPath(const QString &newPath)
{
    Q_D(FileSystemModel);
#ifdef Q_OS_WIN
#ifdef Q_OS_WIN32
    QString longNewPath = qt_GetLongPathName(newPath);
#else
    QString longNewPath = QDir::fromNativeSeparators(newPath);
#endif
#else
    QString longNewPath = newPath;
#endif
    QDir newPathDir(longNewPath);
    //we remove .. and . from the given path if exist
    if (!newPath.isEmpty()) {
        longNewPath = QDir::cleanPath(longNewPath);
        newPathDir.setPath(longNewPath);
    }

    d->setRootPath = true;

    //user don't ask for the root path ("") but the conversion failed
    if (!newPath.isEmpty() && longNewPath.isEmpty())
        return d->index(rootPath());

    if (d->rootDir.path() == longNewPath)
        return d->index(rootPath());

    bool showDrives = longNewPath.isEmpty() || longNewPath == d->myComputer();
    if (!showDrives && !newPathDir.exists())
        return d->index(rootPath());

    //We remove the watcher on the previous path
    if (!rootPath().isEmpty() && rootPath() != QLatin1String(".")) {
        //This remove the watcher for the old rootPath
#ifndef QT_NO_FILESYSTEMWATCHER
        d->fileInfoGatherer.removePath(rootPath());
#endif
        //This line "marks" the node as dirty, so the next fetchMore
        //call on the path will ask the gatherer to install a watcher again
        //But it doesn't re-fetch everything
        d->node(rootPath())->populatedChildren = false;
    }

    // We have a new valid root path
    d->rootDir = newPathDir;
    QModelIndex newRootIndex;
    if (showDrives) {
        // otherwise dir will become '.'
        d->rootDir.setPath(QLatin1String(""));
    } else {
        newRootIndex = d->index(newPathDir.path());
    }
    fetchMore(newRootIndex);
    emit rootPathChanged(longNewPath);
    d->forceSort = true;
    d->delayedSort();
    return newRootIndex;
}

/*!
    The currently set root path

    \sa rootDirectory()
*/
QString FileSystemModel::rootPath() const
{
    Q_D(const FileSystemModel);
    return d->rootDir.path();
}

/*!
    The currently set directory

    \sa rootPath()
*/
QDir FileSystemModel::rootDirectory() const
{
    Q_D(const FileSystemModel);
    QDir dir(d->rootDir);
    dir.setNameFilters(nameFilters());
    dir.setFilter(filter());
    return dir;
}

/*!
    Sets the \a provider of file icons for the directory model.
*/
void FileSystemModel::setDecorationProvider(AbstractFileDecorationProvider *provider)
{
    Q_D(FileSystemModel);
#ifndef QT_NO_FILESYSTEMWATCHER
    d->fileInfoGatherer.setDecorationProvider(provider);
#endif
    d->root.updateIcon(provider, QString());
}

/*!
    Returns the file icon provider for this directory model.
*/
AbstractFileDecorationProvider *FileSystemModel::decorationProvider() const
{
#ifndef QT_NO_FILESYSTEMWATCHER
    Q_D(const FileSystemModel);
    return d->fileInfoGatherer.decorationProvider();
#else
    return 0;
#endif
}

/*!
    Sets the directory model's filter to that specified by \a filters.

    Note that the filter you set should always include the QDir::AllDirs enum value,
    otherwise QFileSystemModel won't be able to read the directory structure.

    \sa QDir::Filters
*/
void FileSystemModel::setFilter(QDir::Filters filters)
{
    Q_D(FileSystemModel);
    if (d->filters == filters)
        return;
    d->filters = filters;
    // CaseSensitivity might have changed
    setNameFilters(nameFilters());
    d->forceSort = true;
    d->delayedSort();
}

/*!
    Returns the filter specified for the directory model.

    If a filter has not been set, the default filter is QDir::AllEntries |
    QDir::NoDotAndDotDot | QDir::AllDirs.

    \sa QDir::Filters
*/
QDir::Filters FileSystemModel::filter() const
{
    Q_D(const FileSystemModel);
    return d->filters;
}

/*!
    \property QFileSystemModel::resolveSymlinks
    \brief Whether the directory model should resolve symbolic links

    This is only relevant on Windows.

    By default, this property is \c true.
*/
void FileSystemModel::setResolveSymlinks(bool enable)
{
#ifndef QT_NO_FILESYSTEMWATCHER
    Q_D(FileSystemModel);
    d->fileInfoGatherer.setResolveSymlinks(enable);
#else
    Q_UNUSED(enable)
#endif
}

bool FileSystemModel::resolveSymlinks() const
{
#ifndef QT_NO_FILESYSTEMWATCHER
    Q_D(const FileSystemModel);
    return d->fileInfoGatherer.resolveSymlinks();
#else
    return false;
#endif
}

/*!
    \property QFileSystemModel::readOnly
    \brief Whether the directory model allows writing to the file system

    If this property is set to false, the directory model will allow renaming, copying
    and deleting of files and directories.

    This property is \c true by default
*/
void FileSystemModel::setReadOnly(bool enable)
{
    Q_D(FileSystemModel);
    d->readOnly = enable;
}

bool FileSystemModel::isReadOnly() const
{
    Q_D(const FileSystemModel);
    return d->readOnly;
}

/*!
    \property QFileSystemModel::nameFilterDisables
    \brief Whether files that don't pass the name filter are hidden or disabled

    This property is \c true by default
*/
void FileSystemModel::setNameFilterDisables(bool enable)
{
    Q_D(FileSystemModel);
    if (d->nameFilterDisables == enable)
        return;
    d->nameFilterDisables = enable;
    d->forceSort = true;
    d->delayedSort();
}

bool FileSystemModel::nameFilterDisables() const
{
    Q_D(const FileSystemModel);
    return d->nameFilterDisables;
}

/*!
    Sets the name \a filters to apply against the existing files.
*/
void FileSystemModel::setNameFilters(const QStringList &filters)
{
    // Prep the regexp's ahead of time
#ifndef QT_NO_REGEXP
    Q_D(FileSystemModel);

    if (!d->bypassFilters.isEmpty()) {
        // update the bypass filter to only bypass the stuff that must be kept around
        d->bypassFilters.clear();
        // We guarantee that rootPath will stick around
        QPersistentModelIndex root(index(rootPath()));
        const QModelIndexList persistentList = persistentIndexList();
        for (const auto &persistentIndex : persistentList) {
            FileSystemModelPrivate::FileSystemNode *node = d->node(persistentIndex);
            while (node) {
                if (d->bypassFilters.contains(node))
                    break;
                if (node->isDir())
                    d->bypassFilters[node] = true;
                node = node->parent;
            }
        }
    }

    d->nameFilters = filters;
    d->forceSort = true;
    d->delayedSort();
#endif
}

/*!
    Returns a list of filters applied to the names in the model.
*/
QStringList FileSystemModel::nameFilters() const
{
#ifndef QT_NO_REGEXP
    Q_D(const FileSystemModel);
    return d->nameFilters;
#else
    return QStringList();
#endif
}

/*!
    \brief Whether punctuation characters and symbols are ignored when sorting,
    \c false by default.
    \sa QCollator::setIgnorePunctuation
*/
void FileSystemModel::setSortIgnoringPunctuation(bool ignore)
{
    Q_D(FileSystemModel);
    d->sortIgnoringPunctuation = ignore;
}

bool FileSystemModel::sortIgnoringPunctuation() const
{
    Q_D(const FileSystemModel);
    return d->sortIgnoringPunctuation;
}

/*!
    \reimp
*/
bool FileSystemModel::event(QEvent *event)
{
#ifndef QT_NO_FILESYSTEMWATCHER
    Q_D(FileSystemModel);
    if (event->type() == QEvent::LanguageChange) {
        d->root.retranslateStrings(d->fileInfoGatherer.decorationProvider(), QString());
        return true;
    }
#endif
    return QAbstractItemModel::event(event);
}

bool FileSystemModel::rmdir(const QModelIndex &aindex)
{
    QString path = filePath(aindex);
    const bool success = QDir().rmdir(path);
#ifndef QT_NO_FILESYSTEMWATCHER
    if (success) {
        auto d = const_cast<FileSystemModelPrivate*>(d_func());
        d->fileInfoGatherer.removePath(path);
    }
#endif
    return success;
}

/*!
     \internal

    Performed quick listing and see if any files have been added or removed,
    then fetch more information on visible files.
 */
void FileSystemModelPrivate::_q_directoryChanged(const QString &directory, const QStringList &files)
{
    FileSystemModelPrivate::FileSystemNode *parentNode = node(directory, false);
    if (parentNode->children.count() == 0)
        return;
    QStringList toRemove;
    QStringList newFiles = files;
    std::sort(newFiles.begin(), newFiles.end());
    for (auto i = parentNode->children.constBegin(), cend = parentNode->children.constEnd(); i != cend; ++i) {
        if (QStringList::iterator iterator = std::lower_bound(
              newFiles.begin(), newFiles.end(), i.value()->fileName);
            iterator == newFiles.end() || i.value()->fileName < *iterator)
            toRemove.append(i.value()->fileName);
    }
    for (int i = 0 ; i < toRemove.count() ; ++i )
        removeNode(parentNode, toRemove[i]);
}

/*!
    \internal

    Adds a new file to the children of parentNode

    *WARNING* this will change the count of children
*/
FileSystemModelPrivate::FileSystemNode* FileSystemModelPrivate::addNode(FileSystemNode *parentNode, const QString &fileName, const QFileInfo& info)
{
    // In the common case, itemLocation == count() so check there first
    auto node = new FileSystemModelPrivate::FileSystemNode(fileName, parentNode);
#ifndef QT_NO_FILESYSTEMWATCHER
    node->populate(ExtendedInformation(info));
#else
    Q_UNUSED(info)
#endif
#if defined(Q_OS_WIN) && !defined(Q_OS_WINRT)
    //The parentNode is "" so we are listing the drives
    if (parentNode->fileName.isEmpty()) {
        wchar_t name[MAX_PATH + 1];
        //GetVolumeInformation requires to add trailing backslash
        const QString nodeName = fileName + QLatin1String("\\");
        // Kid3 GetVolumeInformationW instead of GetVolumeInformation
        BOOL success = ::GetVolumeInformationW((wchar_t *)(nodeName.utf16()),
                name, MAX_PATH + 1, NULL, 0, NULL, NULL, 0);
        if (success && name[0])
            node->volumeName = QString::fromWCharArray(name);
    }
#endif
    Q_ASSERT(!parentNode->children.contains(fileName));
    parentNode->children.insert(fileName, node);
    return node;
}

/*!
    \internal

    File at parentNode->children(itemLocation) has been removed, remove from the lists
    and emit signals if necessary

    *WARNING* this will change the count of children and could change visibleChildren
 */
void FileSystemModelPrivate::removeNode(FileSystemModelPrivate::FileSystemNode *parentNode, const QString& name)
{
    Q_Q(FileSystemModel);
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);

    int vLocation = parentNode->visibleLocation(name);
    if (vLocation >= 0 && !indexHidden)
        q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                                       translateVisibleLocation(parentNode, vLocation));
    FileSystemNode * node = parentNode->children.take(name);
#ifndef QT_NO_FILESYSTEMWATCHER
    if (node->info && node->info->isDir()) {
        // Remove watched path when directory is removed or renamed
        fileInfoGatherer.removePath(node->info->fileInfo().filePath());
    }
#endif
    delete node;
    // cleanup sort files after removing rather then re-sorting which is O(n)
    if (vLocation >= 0)
        parentNode->visibleChildren.removeAt(vLocation);
    if (vLocation >= 0 && !indexHidden)
        q->endRemoveRows();
}

/*!
    \internal

    File at parentNode->children(itemLocation) was not visible before, but now should be
    and emit signals if necessary.

    *WARNING* this will change the visible count
 */
void FileSystemModelPrivate::addVisibleFiles(FileSystemNode *parentNode, const QStringList &newFiles)
{
    Q_Q(FileSystemModel);
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);
    if (!indexHidden) {
        q->beginInsertRows(parent, parentNode->visibleChildren.count() , parentNode->visibleChildren.count() + newFiles.count() - 1);
    }

    if (parentNode->dirtyChildrenIndex == -1)
        parentNode->dirtyChildrenIndex = parentNode->visibleChildren.count();

    for (const auto &newFile : newFiles) {
        parentNode->visibleChildren.append(newFile);
        parentNode->children.value(newFile)->isVisible = true;
    }
    if (!indexHidden)
      q->endInsertRows();
}

/*!
    \internal

    File was visible before, but now should NOT be

    *WARNING* this will change the visible count
 */
void FileSystemModelPrivate::removeVisibleFile(FileSystemNode *parentNode, int vLocation)
{
    Q_Q(FileSystemModel);
    if (vLocation == -1)
        return;
    QModelIndex parent = index(parentNode);
    bool indexHidden = isHiddenByFilter(parentNode, parent);
    if (!indexHidden)
        q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                                       translateVisibleLocation(parentNode, vLocation));
    parentNode->children.value(parentNode->visibleChildren.at(vLocation))->isVisible = false;
    parentNode->visibleChildren.removeAt(vLocation);
    if (!indexHidden)
        q->endRemoveRows();
}

/*!
    \internal

    The thread has received new information about files,
    update and emit dataChanged if it has actually changed.
 */
void FileSystemModelPrivate::_q_fileSystemChanged(const QString &path, const QVector<QPair<QString, QFileInfo> > &updates)
{
#ifndef QT_NO_FILESYSTEMWATCHER
    Q_Q(FileSystemModel);
    QVector<QString> rowsToUpdate;
    QStringList newFiles;
    FileSystemModelPrivate::FileSystemNode *parentNode = node(path, false);
    QModelIndex parentIndex = index(parentNode);
    for (const auto &update : updates) {
        QString fileName = update.first;
        Q_ASSERT(!fileName.isEmpty());
        ExtendedInformation info = fileInfoGatherer.getInfo(update.second);
        if (bool previouslyHere = parentNode->children.contains(fileName);
            !previouslyHere) {
            addNode(parentNode, fileName, info.fileInfo());
        }
        FileSystemModelPrivate::FileSystemNode * node = parentNode->children.value(fileName);
        bool isCaseSensitive = parentNode->caseSensitive();
        if (isCaseSensitive) {
            if (node->fileName != fileName)
                continue;
        } else {
            if (QString::compare(node->fileName,fileName,Qt::CaseInsensitive) != 0)
                continue;
        }
        if (isCaseSensitive) {
            Q_ASSERT(node->fileName == fileName);
        } else {
            node->fileName = fileName;
        }

        if (*node != info ) {
            node->populate(info);
            bypassFilters.remove(node);
            // brand new information.
            if (filtersAcceptsNode(node)) {
                if (!node->isVisible) {
                    newFiles.append(fileName);
                } else {
                    rowsToUpdate.append(fileName);
                }
            } else {
                if (node->isVisible) {
                    int visibleLocation = parentNode->visibleLocation(fileName);
                    removeVisibleFile(parentNode, visibleLocation);
                } else {
                    // The file is not visible, don't do anything
                }
            }
        }
    }

    // bundle up all of the changed signals into as few as possible.
    std::sort(rowsToUpdate.begin(), rowsToUpdate.end());
    QString min;
    QString max;
    for (int i = 0; i < rowsToUpdate.count(); ++i) {
        QString value = rowsToUpdate.at(i);
        //##TODO is there a way to bundle signals with QString as the content of the list?
        /*if (min.isEmpty()) {
            min = value;
            if (i != rowsToUpdate.count() - 1)
                continue;
        }
        if (i != rowsToUpdate.count() - 1) {
            if ((value == min + 1 && max.isEmpty()) || value == max + 1) {
                max = value;
                continue;
            }
        }*/
        max = value;
        min = value;
        int visibleMin = parentNode->visibleLocation(min);
        int visibleMax = parentNode->visibleLocation(max);
        if (visibleMin >= 0
            && visibleMin < parentNode->visibleChildren.count()
            && parentNode->visibleChildren.at(visibleMin) == min
            && visibleMax >= 0) {
            QModelIndex bottom = q->index(translateVisibleLocation(parentNode, visibleMin), 0, parentIndex);
            QModelIndex top = q->index(translateVisibleLocation(parentNode, visibleMax), 3, parentIndex);
            emit q->dataChanged(bottom, top);
        }

        /*min = QString();
        max = QString();*/
    }

    if (newFiles.count() > 0) {
        addVisibleFiles(parentNode, newFiles);
    }

    if (newFiles.count() > 0 || (sortColumn != 0 && rowsToUpdate.count() > 0)) {
        forceSort = true;
        delayedSort();
    }
#else
    Q_UNUSED(path)
    Q_UNUSED(updates)
#endif // !QT_NO_FILESYSTEMWATCHER
}

/*!
    \internal
*/
void FileSystemModelPrivate::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
    resolvedSymLinks[fileName] = resolvedName;
}

void FileSystemModelPrivate::clear()
{
    forceSort = true;
    setRootPath = false;
    rootDir.setPath(QString::fromLatin1("."));
#ifndef QT_NO_FILESYSTEMWATCHER
    fileInfoGatherer.clear();
#endif
    delayedSortTimer.stop();
    bypassFilters.clear();
    resolvedSymLinks.clear();
    root.clear();
    fetchingTimer.stop();
    toFetch.clear();
}

/*!
    \internal
*/
void FileSystemModelPrivate::init()
{
    Q_Q(FileSystemModel);
    qRegisterMetaType<QVector<QPair<QString,QFileInfo> > >();
#ifndef QT_NO_FILESYSTEMWATCHER
    q->connect(&fileInfoGatherer, SIGNAL(newListOfFiles(QString,QStringList)),
               q, SLOT(_q_directoryChanged(QString,QStringList)));
    q->connect(&fileInfoGatherer, SIGNAL(updates(QString,QVector<QPair<QString,QFileInfo> >)),
            q, SLOT(_q_fileSystemChanged(QString,QVector<QPair<QString,QFileInfo> >)));
    q->connect(&fileInfoGatherer, SIGNAL(nameResolved(QString,QString)),
            q, SLOT(_q_resolvedName(QString,QString)));
    q->connect(&fileInfoGatherer, SIGNAL(directoryLoaded(QString)),
               q, SIGNAL(directoryLoaded(QString)));
#endif // !QT_NO_FILESYSTEMWATCHER
    q->connect(&delayedSortTimer, SIGNAL(timeout()), q, SLOT(_q_performDelayedSort()), Qt::QueuedConnection);

#if QT_VERSION >= 0x050f00
    roleNames.insert(FileSystemModel::FileIconRole, QByteArrayLiteral("fileIcon")); // == Qt::decoration
#else
    roleNames.insertMulti(FileSystemModel::FileIconRole, QByteArrayLiteral("fileIcon")); // == Qt::decoration
#endif
    roleNames.insert(FileSystemModel::FilePathRole, QByteArrayLiteral("filePath"));
    roleNames.insert(FileSystemModel::FileNameRole, QByteArrayLiteral("fileName"));
    roleNames.insert(FileSystemModel::FilePermissions, QByteArrayLiteral("filePermissions"));
}

#ifndef USE_QT_PRIVATE_HEADERS
QHash<int, QByteArray> FileSystemModel::roleNames() const
{
    Q_D(const FileSystemModel);
    return d->roleNames;
}
#endif

#if QT_VERSION < 0x050f00
QString FileSystemModel::wildcardToRegularExpression(const QString &pattern)
{
    const int wclen = pattern.size();
    QString rx;
    rx.reserve(wclen + wclen / 16);
    int i = 0;
    const QChar *wc = pattern.data();

#ifdef Q_OS_WIN
    const QLatin1Char nativePathSeparator('\\');
    const QLatin1String starEscape("[^/\\\\]*");
    const QLatin1String questionMarkEscape("[^/\\\\]");
#else
    const QLatin1Char nativePathSeparator('/');
    const QLatin1String starEscape("[^/]*");
    const QLatin1String questionMarkEscape("[^/]");
#endif

    while (i < wclen) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
            rx += starEscape;
            break;
        case '?':
            rx += questionMarkEscape;
            break;
        case '\\':
#ifdef Q_OS_WIN
        case '/':
            rx += QLatin1String("[/\\\\]");
            break;
#endif
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        case '[':
            rx += c;
            // Support for the [!abc] or [!a-c] syntax
            if (i < wclen) {
                if (wc[i] == QLatin1Char('!')) {
                    rx += QLatin1Char('^');
                    ++i;
                }

                if (i < wclen && wc[i] == QLatin1Char(']'))
                    rx += wc[i++];

                while (i < wclen && wc[i] != QLatin1Char(']')) {
                    // The '/' appearing in a character class invalidates the
                    // regular expression parsing. It also concerns '\\' on
                    // Windows OS types.
                    if (wc[i] == QLatin1Char('/') || wc[i] == nativePathSeparator)
                        return rx;
                    if (wc[i] == QLatin1Char('\\'))
                        rx += QLatin1Char('\\');
                    rx += wc[i++];
                }
            }
            break;
        default:
            rx += c;
            break;
        }
    }

    return rx;
}
#endif

/*!
    \internal

    Returns \c false if node doesn't pass the filters otherwise true

    QDir::Modified is not supported
    QDir::Drives is not supported
*/
bool FileSystemModelPrivate::filtersAcceptsNode(const FileSystemNode *node) const
{
    // always accept drives
    if (node->parent == &root || bypassFilters.contains(node))
        return true;

    // If we don't know anything yet don't accept it
    if (!node->hasInformation())
        return false;

    const bool filterPermissions = ((filters & QDir::PermissionMask)
                                   && (filters & QDir::PermissionMask) != QDir::PermissionMask);
    const bool hideDirs          = !(filters & (QDir::Dirs | QDir::AllDirs));
    const bool hideFiles         = !(filters & QDir::Files);
    const bool hideReadable      = !(!filterPermissions || (filters & QDir::Readable));
    const bool hideWritable      = !(!filterPermissions || (filters & QDir::Writable));
    const bool hideExecutable    = !(!filterPermissions || (filters & QDir::Executable));
    const bool hideHidden        = !(filters & QDir::Hidden);
    const bool hideSystem        = !(filters & QDir::System);
    const bool hideSymlinks      = (filters & QDir::NoSymLinks);
    const bool hideDot           = (filters & QDir::NoDot);
    const bool hideDotDot        = (filters & QDir::NoDotDot);

    // Note that we match the behavior of entryList and not QFileInfo on this.
    bool isDot    = node->fileName == QLatin1String(".");
    bool isDotDot = node->fileName == QLatin1String("..");
    if (   (hideHidden && !(isDot || isDotDot) && node->isHidden())
        || (hideSystem && node->isSystem())
        || (hideDirs && node->isDir())
        || (hideFiles && node->isFile())
        || (hideSymlinks && node->isSymLink())
        || (hideReadable && node->isReadable())
        || (hideWritable && node->isWritable())
        || (hideExecutable && node->isExecutable())
        || (hideDot && isDot)
        || (hideDotDot && isDotDot))
        return false;

    return nameFilterDisables || passNameFilters(node);
}

/*
    \internal

    Returns \c true if node passes the name filters and should be visible.
 */
bool FileSystemModelPrivate::passNameFilters(const FileSystemNode *node) const
{
#ifndef QT_NO_REGEXP
    if (nameFilters.isEmpty())
        return true;

    // Check the name regularexpression filters
    if (!(node->isDir() && (filters & QDir::AllDirs))) {
        auto reOptions = filters & QDir::CaseSensitive
            ? QRegularExpression::NoPatternOption
            : QRegularExpression::CaseInsensitiveOption;
        for (const auto &nameFilter : nameFilters) {
#if QT_VERSION >= 0x050f00
            auto rx = QRegularExpression(
                  QRegularExpression::wildcardToRegularExpression(nameFilter),
                  reOptions);
#else
            auto rx = QRegularExpression(
                  FileSystemModel::wildcardToRegularExpression(nameFilter),
                  reOptions);
#endif
            if (auto match = rx.match(node->fileName); match.hasMatch())
                return true;
        }
        return false;
    }
#endif
    return true;
}
/** \endcond */
