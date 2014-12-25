/**
 * \file kdemainwindow.h
 * KDE Kid3 main window.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
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

#ifndef KDEMAINWINDOW_H
#define KDEMAINWINDOW_H

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <KXmlGuiWindow>
#else
#include <kxmlguiwindow.h>
#endif
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
                         Kid3Application* app, QWidget* parent = 0);

  /**
   * Destructor.
   */
  virtual ~KdeMainWindow();

  /**
   * Init menu and toolbar actions.
   */
  virtual void initActions();

  /**
   * Add directory to recent files list.
   *
   * @param dirName path to directory
   */
  virtual void addDirectoryToRecentFiles(const QString& dirName);

  /**
   * Read settings from the configuration.
   */
  virtual void readConfig();

  /**
   * Store geometry and recent files in settings.
   */
  virtual void saveConfig();

  /**
   * Get action for Settings/Auto Hide Tags.
   * @return action.
   */
  virtual QAction* autoHideTagsAction();

  /**
   * Get action for Settings/Hide Picture.
   * @return action.
   */
  virtual QAction* showHidePictureAction();

  /**
   * Set main window caption.
   *
   * @param caption caption without application name
   * @param modified true if any file is modified
   */
  virtual void setWindowCaption(const QString& caption, bool modified);

protected:
  /**
   * Update modification state before closing.
   * Called on closeEvent() of window.
   * If anything was modified, save after asking user.
   *
   * @return FALSE if user canceled.
   */
  virtual bool queryClose();

  /**
   * Saves the window properties for each open window during session end
   * to the session config file.
   *
   * @param cfg application configuration
   */
  virtual void saveProperties(KConfigGroup& cfg);

  /**
   * Reads the session config file and restores the application's state.
   *
   * @param cfg application configuration
   */
  virtual void readProperties(const KConfigGroup& cfg);

private slots:
  /**
   * Open recent directory.
   *
   * @param url URL of directory to open
   */
#if QT_VERSION >= 0x050000
  void slotFileOpenRecentUrl(const QUrl& url);
#else
  void slotFileOpenRecentUrl(const KUrl& url);
#endif

  /**
   * Shortcuts configuration.
   */
  void slotSettingsShortcuts();

  /**
   * Toolbars configuration.
   */
  void slotSettingsToolbars();

  /**
   * Preferences.
   */
  void slotSettingsConfigure();

private:
  /** Actions */
  KRecentFilesAction* m_fileOpenRecent;
  KToggleAction* m_viewToolBar;
  KToggleAction* m_viewStatusBar;
  KToggleAction* m_settingsAutoHideTags;
  KToggleAction* m_settingsShowHidePicture;
};

#endif /* KDEMAINWINDOW_H */
