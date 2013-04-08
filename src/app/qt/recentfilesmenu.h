/**
 * \file recentfilesmenu.h
 * Menu to open recent files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15-Aug-2010
 *
 * Copyright (C) 2010-2013  Urs Fleisch
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

#ifndef RECENTFILESMENU_H
#define RECENTFILESMENU_H

#include <QStringList>
#include <QMenu>
#include "generalconfig.h"

/**
 * Menu to open recent files.
 */
class RecentFilesMenu : public QMenu {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit RecentFilesMenu(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~RecentFilesMenu();

  /**
   * Add directory to list of recent files.
   *
   * @param dir path to directory
   */
  void addDirectory(const QString& dir);

  /**
   * Saves the current recent files entries to a given configuration.
   *
   * @param config configuration settings
   */
  void saveEntries(ISettings* config);

  /**
   * Loads the recent files entries from a given configuration.
   *
   * @param config configuration settings
   */
  void loadEntries(ISettings* config);

signals:
  /**
   * Emitted when a recent file has to be loaded.
   * Parameter: path to file or directory
   */
  void loadFile(const QString&);

private slots:
  /**
   * Emit a load file signal when a recent file has to be loaded.
   */
  void openRecentFile();

  /**
   * Clear the list of recent files.
   */
  void clearList();

private:
  /**
   * Update the recent file actions.
   */
  void updateRecentFileActions();

  QStringList m_files;
};

#endif // RECENTFILESMENU_H
