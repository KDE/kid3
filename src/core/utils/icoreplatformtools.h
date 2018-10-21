/**
 * \file icoreplatformtools.h
 * Interface for GUI independent platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Apr 2013
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

#pragma once

#include <QList>
#include <QPair>
#include "kid3api.h"

class QString;
class ISettings;

/**
 * Interface for GUI independent platform specific tools.
 */
class KID3_CORE_EXPORT ICorePlatformTools {
public:
  /**
   * Destructor.
   */
  virtual ~ICorePlatformTools();

  /**
   * Get application settings.
   * @return settings instance.
   */
  virtual ISettings* applicationSettings() = 0;

  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  virtual bool moveToTrash(const QString& path) const = 0;

  /**
   * Construct a name filter string suitable for file dialogs.
   * @param nameFilters list of description, filter pairs, e.g.
   * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
   * @return name filter string.
   */
  virtual QString fileDialogNameFilter(
      const QList<QPair<QString, QString> >& nameFilters) const = 0;

  /**
   * Get file pattern part of m_nameFilter.
   * @param nameFilter name filter string
   * @return file patterns, e.g. "*.mp3".
   */
  virtual QString getNameFilterPatterns(const QString& nameFilter) const = 0;

protected:
  /**
   * Construct a name filter string suitable for file dialogs.
   * This function can be used to implement fileDialogNameFilter()
   * for QFileDialog.
   * @param nameFilters list of description, filter pairs, e.g.
   * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
   * @return name filter string.
   */
  static QString qtFileDialogNameFilter(
      const QList<QPair<QString, QString> >& nameFilters);

  /**
   * Get file pattern part of m_nameFilter.
   * This function can be used to implement getNameFilterPatterns()
   * for QFileDialog.
   * @param nameFilter name filter string
   * @return file patterns, e.g. "*.mp3".
   */
  static QString qtNameFilterPatterns(const QString& nameFilter);
};
