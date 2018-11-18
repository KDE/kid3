/**
 * \file abstractfiledecorationprovider.cpp
 * Indirection for QFileIconProvider to use it without Gui and Widgets.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Nov 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

#include "abstractfiledecorationprovider.h"
#include <QString>
#include <QDir>
#include <QCoreApplication>

AbstractFileDecorationProvider::~AbstractFileDecorationProvider()
{
}

QString AbstractFileDecorationProvider::type(const QFileInfo &info) const
{
    return fileTypeDescription(info);
}

/* Taken from QFileIconProvider, simplified. */
#if defined(Q_OS_WIN)
static bool isUncRoot(const QString &server)
{
    QString localPath = QDir::toNativeSeparators(server);
    if (!localPath.startsWith(QLatin1String("\\\\")))
        return false;

    int idx = localPath.indexOf(QLatin1Char('\\'), 2);
    if (idx == -1 || idx + 1 == localPath.length())
        return true;

    return localPath.rightRef(localPath.length() - idx - 1).trimmed().isEmpty();
}

static bool isDriveRootPath(const QString &path)
{
#ifndef Q_OS_WINRT
    return (path.length() == 3
           && path.at(0).isLetter() && path.at(1) == QLatin1Char(':')
           && path.at(2) == QLatin1Char('/'));
#else // !Q_OS_WINRT
    return path == QDir::rootPath();
#endif // !Q_OS_WINRT
}
#endif // Q_OS_WIN

static bool isRootPath(const QString &path)
{
    if (path == QLatin1String("/")
#if defined(Q_OS_WIN)
            || isDriveRootPath(path)
            || isUncRoot(path)
#endif
            )
        return true;

    return false;
}

QString AbstractFileDecorationProvider::fileTypeDescription(const QFileInfo &info)
{
    if (isRootPath(info.absoluteFilePath())) {
        const char* const driveStr = QT_TRANSLATE_NOOP("@default", "Drive");
        return QCoreApplication::translate("@default", driveStr);
    }
    if (info.isFile()) {
        if (!info.suffix().isEmpty()) {
            //: %1 is a file name suffix, for example txt
            const char* const suffixStr = QT_TRANSLATE_NOOP("@default", "%1 File");
            return QCoreApplication::translate("@default", suffixStr).arg(info.suffix());
        }
        const char* const fileStr = QT_TRANSLATE_NOOP("@default", "File");
        return QCoreApplication::translate("@default", fileStr);
    }

    if (info.isDir()) {
        const char* const folderStr = QT_TRANSLATE_NOOP("@default", "Folder");
        return QCoreApplication::translate("@default", folderStr);
    }
    if (info.isSymLink()) {
        const char* const shortcutStr = QT_TRANSLATE_NOOP("@default", "Shortcut");
        return QCoreApplication::translate("@default", shortcutStr);
    }
    const char* const unknownStr = QT_TRANSLATE_NOOP("@default", "Unknown");
    return QCoreApplication::translate("@default", unknownStr);
}
