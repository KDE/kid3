/**
 * \file coreplatformtools.h
 * Core platform specific tools for Qt.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Aug 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#include <QScopedPointer>
#include "icoreplatformtools.h"
#include "isettings.h"
#include "coretaggedfileiconprovider.h"

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
  ~CorePlatformTools() override;

  /**
   * Get application settings.
   * @return settings instance.
   */
  ISettings* applicationSettings() override;

  /**
   * Get icon provider for tagged files.
   * @return icon provider.
   */
  CoreTaggedFileIconProvider* iconProvider() override;

  /**
   * Write text to clipboard.
   * @param text text to write
   * @return true if operation is supported.
   */
  bool writeToClipboard(const QString& text) const override;

  /**
   * Read text from clipboard.
   * @return text, null if operation not supported.
   */
  QString readFromClipboard() const override;

  /**
   * Create an audio player instance.
   * @param app application context
   * @param dbusEnabled true to enable MPRIS D-Bus interface
   * @return audio player, nullptr if not supported.
   */
  QObject* createAudioPlayer(Kid3Application* app,
                             bool dbusEnabled) const override;

  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  bool moveToTrash(const QString& path) const override;

  /**
   * Construct a name filter string suitable for file dialogs.
   * @param nameFilters list of description, filter pairs, e.g.
   * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
   * @return name filter string.
   */
  QString fileDialogNameFilter(
      const QList<QPair<QString, QString> >& nameFilters) const override;

  /**
   * Get file pattern part of m_nameFilter.
   * @param nameFilter name filter string
   * @return file patterns, e.g. "*.mp3".
   */
  QString getNameFilterPatterns(const QString& nameFilter) const override;

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
  QScopedPointer<ISettings> m_config;
  QScopedPointer<CoreTaggedFileIconProvider> m_iconProvider;
};
