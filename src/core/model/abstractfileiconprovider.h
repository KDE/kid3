/**
 * \file abstractfileiconprovider.h
 * Indirection for QFileIconProvider to use it without Qt5::Widgets.
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

#pragma once

#include "kid3api.h"

class QString;
class QFileInfo;
class QIcon;

/**
 * Provides icons for the FileSystemModel.
 */
class KID3_CORE_EXPORT AbstractFileIconProvider
{
public:
    virtual ~AbstractFileIconProvider();

    /** Computer icon. */
    virtual QIcon computerIcon() const = 0;
    /** Folder icon. */
    virtual QIcon folderIcon() const = 0;
    /** File icon. */
    virtual QIcon fileIcon() const = 0;
    /** Icon for a file type. */
    virtual QIcon icon(const QFileInfo &info) const = 0;
    /** Description for a file type. */
    virtual QString type(const QFileInfo &info) const;

    /** Default implementation for type(). */
    static QString fileTypeDescription(const QFileInfo &info);
};
