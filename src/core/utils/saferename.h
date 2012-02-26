/**
 * \file saferename.h
 * Safely rename a file.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Feb 2012
 *
 * Copyright (C) 2012  Urs Fleisch
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

#ifndef SAFERENAME_H
#define SAFERENAME_H

#include "kid3api.h"

class QString;

namespace Utils {

/**
 * Check if file name has illegal characters.
 *
 * @param fileName file name
 *
 * @return true if file name contains illegal characters.
 */
bool KID3_CORE_EXPORT hasIllegalFileNameCharacters(const QString& fileName);

/**
 * Rename a file.
 * Renames the file using QDir::rename() if @a newName does not contain
 * illegal characters.
 *
 * @param oldName old file name
 * @param newName new file name
 *
 * @return true if ok.
 */
bool KID3_CORE_EXPORT safeRename(const QString& oldName, const QString& newName);

/**
 * Rename a file.
 * Renames the file using QDir::rename() if @a newName does not contain
 * illegal characters.
 *
 * @param dirPath directory path
 * @param oldName old file name
 * @param newName new file name
 *
 * @return true if ok.
 */
bool KID3_CORE_EXPORT safeRename(const QString& dirPath,
                const QString& oldName, const QString& newName);

}

#endif // SAFERENAME_H
