/**
 * \file kdemainwindow.h
 * KDE Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#include <QtGlobal>
#include <KXmlGuiWindow>
#include "basemainwindow.h"

class QAction;
class KRecentFilesAction;
class KToggleAction;
class KUrl;
class IPlatformTools;

/**
 * KDE Kid3 main window.
 */
class KdeMainWindow : public KXmlGuiWindow, public BaseMainWindow {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform specific tools
   * @param app application context
   * @param parent parent widget
   */
  explicit KdeMainWindow(IPlatformTools* platformTools,
                         Kid3Application* app, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~KdeMainWindow() override = default;

  /**
   * Init menu and toolbar actions.
   */
  virtual void initActions() override;

  /**
   * Add directory to recent files list.
   *
   * @param dirName path to directory
   */
  virtual void addDirectoryToRecentFiles(const QString& dirName) override;

  /**
   * Read settings from the configuration.
   */
  virtual void readConfig() override;

  /**
   * Store geometry and recent files in settings.
   */
  virtual void saveConfig() override;

  /**
   * Get action for Settings/Auto Hide Tags.
   * @return action.
   */
  virtual QAction* autoHideTagsAction() override;

  /**
   * Get action for Settings/Hide Picture.
   * @return action.
   */
  virtual QAction* showHidePictureAction() override;

  /**
   * Set main window caption.
   *
   * @param caption caption without application name
   * @param modified true if any file is modified
   */
  virtual void setWindowCaption(const QString& caption, bool modified) override;

protected:
  /**
   * Update modification state before closing.
   * Called on closeEvent() of window.
   * If anything was modified, save after asking user.
   *
   * @return FALSE if user canceled.
   */
  virtual bool queryClose() override;

  /**
   * Saves the window properties for each open window during session end
   * to the session config file.
   *
   * @param cfg application configuration
   */
  virtual void saveProperties(KConfigGroup& cfg) override;

  /**
   * Reads the session config file and restores the application's state.
   *
   * @param cfg application configuration
   */
  virtual void readProperties(const KConfigGroup& cfg) override;

private slots:
  /**
   * Open recent directory.
   *
   * @param url URL of directory to open
   */
  void slotFileOpenRecentUrl(const QUrl& url);

  /**
   * Shortcuts configuration.
   */
  void slotSettingsShortcuts();

  /**
   * Toolbars configuration.
   */
  void slotSettingsToolbars();

  /**
   * Statusbar configuration.
   */
  void slotSettingsShowStatusbar();

  /**
   * Preferences.
   */
  void slotSettingsConfigure();

  /**
   * Add user action to collection.
   * @param name name of action
   * @param action action to add
   */
  void onUserActionAdded(const QString& name, QAction* action);

  /**
   * Remove user action from collection.
   * @param name name of action
   * @param action action to remove
   */
  void onUserActionRemoved(const QString& name, QAction* action);

private:
  IPlatformTools* m_platformTools;
  /** Actions */
  KRecentFilesAction* m_fileOpenRecent;
  KToggleAction* m_settingsShowStatusbar;
  KToggleAction* m_settingsAutoHideTags;
  KToggleAction* m_settingsShowHidePicture;
};
