/**
 * \file coreplatformtools.h
 * Core platform specific tools for Qt.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
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

#ifndef COREPLATFORMTOOLS_H
#define COREPLATFORMTOOLS_H

#include "icoreplatformtools.h"

class QSettings;

/**
 * Core platform specific tools for Qt.
 */
class KID3_CORE_EXPORT CorePlatformTools : public ICorePlatformTools {
public:
  /**
   * Constructor.
   */
  CorePlatformTools();

  /**
   * Destructor.
   */
  virtual ~CorePlatformTools();

  /**
   * Get application settings.
   * @return settings instance.
   */
  virtual ISettings* applicationSettings();

  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  virtual bool moveToTrash(const QString& path) const;

  /**
   * Construct a name filter string suitable for file dialogs.
   * @param nameFilters list of description, filter pairs, e.g.
   * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
   * @return name filter string.
   */
  virtual QString fileDialogNameFilter(
      const QList<QPair<QString, QString> >& nameFilters) const;

  /**
   * Get file pattern part of m_nameFilter.
   * @param nameFilter name filter string
   * @return file patterns, e.g. "*.mp3".
   */
  virtual QString getNameFilterPatterns(const QString& nameFilter) const;

#if !defined Q_OS_WIN32 && !defined Q_OS_MAC
  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  static bool moveFileToTrash(const QString& path);
#endif

private:
  QSettings* m_settings;
  ISettings* m_config;
};

#endif // COREPLATFORMTOOLS_H
